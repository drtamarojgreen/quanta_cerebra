#include "visualization/video_logic.h"
#include "core/data_parsing_hub.h"
#include "io/config.h"
#include "ai/ai.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#ifndef _WIN32
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include "ui/terminal_renderer.h"

static const char* INTENSITY_CHARS = " .:+X@";
static const char* BINARY_CHARS = "01";

char intensityToChar(double intensity, const std::string& map_type) {
    if (map_type == "binary") return BINARY_CHARS[intensity > 0.5];
    if (map_type == "default") return INTENSITY_CHARS[std::min(5, (int)(intensity * 6))];
    if (map_type == "grayscale") {
        if (intensity < 0.2) return ' ';
        if (intensity < 0.4) return '.';
        if (intensity < 0.6) return 'o';
        if (intensity < 0.8) return 'O';
        return '#';
    }
    if (intensity < 0.2) return ' ';
    if (intensity < 0.4) return '.';
    if (intensity < 0.6) return ':';
    if (intensity < 0.75) return '+';
    if (intensity < 0.9) return 'X';
    return '@';
}

std::string intensityToSymbol(double intensity, const std::string& map_type) {
    if (map_type == "dither") {
        static int flip = 0; flip = !flip;
        return (intensity > 0.5) ? (flip ? "█" : "▓") : (flip ? "▒" : "░");
    }
    if (map_type == "grayscale") {
        if (intensity < 0.2) return " ";
        if (intensity < 0.4) return "░";
        if (intensity < 0.6) return "▒";
        if (intensity < 0.8) return "▓";
        return "█";
    }
    std::string s;
    s += intensityToChar(intensity, map_type);
    return s;
}

std::string intensityToColor(double intensity, const std::string& theme) {
    if (theme == "monochrome") return "";
    if (theme == "dynamic") {
        int code = 16 + (int)(intensity * 216);
        return "\033[38;5;" + std::to_string(code) + "m";
    }
    if (theme == "ocean") {
        int colorCode = 17 + (int)(intensity * 4);
        return "\033[38;5;" + std::to_string(colorCode) + "m";
    }
    if (theme == "forest") {
        int colorCode = 22 + (int)(intensity * 24);
        return "\033[38;5;" + std::to_string(colorCode) + "m";
    }
    if (theme == "high-contrast") {
        return (intensity > 0.5) ? "\033[97;40m" : "\033[90;40m";
    }
    int colorCode = 232 + (int)(intensity * 23);
    return "\033[38;5;" + std::to_string(colorCode) + "m";
}

void renderRegion(std::ostringstream& oss, const cerebra::RegionState& region, int depth, const AppConfig& config) {
    for (int i = 0; i < depth; ++i) oss << "  ";
    oss << "[" << region.region << "] ";
    if (config.enable_color) oss << intensityToColor(region.intensity, config.theme);
    int max_width = cerebra::terminal_size().cols / 4;
    if (max_width < 10) max_width = 10;
    int barWidth = static_cast<int>(region.intensity * max_width);
    for (int i = 0; i < barWidth; ++i) oss << intensityToSymbol(region.intensity, config.intensity_map);
    if (config.enable_color) oss << "\033[0m";
    if (!region.intensity_history.empty()) {
        oss << "  ";
        const char* sparkline[] = {" ", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
        for (double val : region.intensity_history) {
            int idx = std::max(0, std::min(7, (int)(val * 8))); // Modified to val * 8
            oss << sparkline[idx];
        }


    }
    oss << "\n";
    for (const cerebra::RegionState& subregion : region.subregions) renderRegion(oss, subregion, depth + 1, config);
}

void renderGrid(std::ostringstream& oss, const cerebra::BrainFrame& frame, const AppConfig& config) {
    double grid[10][10]; bool occupied[10][10];
    for(int i=0; i<10; ++i) for(int j=0; j<10; ++j) { grid[i][j] = 0; occupied[i][j] = false; }
    for (const auto& region : frame.regions) {
        double zx = (region.x - 0.5 + config.offset_x) * config.zoom + 0.5;
        double zy = (region.y - 0.5 + config.offset_y) * config.zoom + 0.5;
        int gx = std::max(0, std::min(9, (int)(zx * 9)));
        int gy = std::max(0, std::min(9, (int)(zy * 9)));
        grid[gy][gx] = region.intensity; occupied[gy][gx] = true;
    }
    oss << "--- Grid View ---\n";
    for(int i=0; i<10; ++i) {
        for(int j=0; j<10; ++j) {
            if (occupied[i][j]) {
                if (config.enable_color) oss << intensityToColor(grid[i][j], config.theme);
                oss << intensityToSymbol(grid[i][j], config.intensity_map) << " ";
                if (config.enable_color) oss << "\033[0m";
            } else oss << "  ";
        }
        oss << "\n";
    }
}

void renderParticles(std::ostringstream& oss, const cerebra::BrainFrame& frame) {
    oss << "--- Particles ---\n";
    for(const auto& r : frame.regions) {
        int count = (int)(r.intensity * 20);
        for(int i=0; i<count; i++) oss << "*";
        oss << "\n";
    }
}

void render3D(std::ostringstream& oss, const cerebra::BrainFrame& frame, const AppConfig& config) {
    double canvas[20][40]; bool occupied[20][40];
    for(int i=0; i<20; ++i) for(int j=0; j<40; ++j) { canvas[i][j] = 0; occupied[i][j] = false; }
    for (const auto& region : frame.regions) {
        double zx = (region.x - 0.5 + config.offset_x) * config.zoom + 0.5;
        double zy = (region.y - 0.5 + config.offset_y) * config.zoom + 0.5;
        double zz = region.z * config.zoom;
        double xp = (zx - zz) * 15 + 20;
        double yp = (zy + (zx + zz) / 2.0) * 8 + 2;
        int ix = std::max(0, std::min(39, (int)xp));
        int iy = std::max(0, std::min(19, (int)yp));
        canvas[iy][ix] = region.intensity; occupied[iy][ix] = true;
    }
    oss << "--- 3D Greenhouse View ---\n";
    for(int i=0; i<20; ++i) {
        for(int j=0; j<40; ++j) {
            if (occupied[i][j]) {
                if (config.enable_color) oss << intensityToColor(canvas[i][j], config.theme);
                oss << intensityToSymbol(canvas[i][j], config.intensity_map);
                if (config.enable_color) oss << "\033[0m";
            } else oss << " ";
        }
        oss << "\n";
    }
}

std::vector<std::string> generateFrames(const std::vector<cerebra::BrainFrame>& input, const AppConfig& config) {
    std::vector<std::string> result;
    if (config.layout_mode == "quiet") return result;
    result.reserve(input.size());
    for (const cerebra::BrainFrame& frame : input) {
        std::ostringstream oss;
        oss << "[Time: " << frame.timestamp_ms << " ms] " << generatePoeticDescription(frame) << "\n";
        if (config.layout_mode == "grid") renderGrid(oss, frame, config);
        else if (config.layout_mode == "3d") render3D(oss, frame, config);
        else if (config.layout_mode == "particles") renderParticles(oss, frame);
        else for (const cerebra::RegionState& region : frame.regions) renderRegion(oss, region, 0, config);
        result.push_back(oss.str());
    }
    return result;
}

void applyASCIIShader(std::string& frame, const std::string& type) {
    if(type == "blur") {
        for(size_t i=1; i<frame.size()-1; i++) if(frame[i] == '@') frame[i] = 'X';
    } else if(type == "edge") {
        for(size_t i=1; i<frame.size()-1; i++) if(frame[i] != frame[i+1]) frame[i] = '|';
    }
}

void synthesizeRealTimeSound(const cerebra::BrainFrame& frame) {
    for(const auto& r : frame.regions) if(r.intensity > 0.95) std::cout << "\a" << std::flush;
}

std::string renderLargeText(const std::string& text) {
    std::string out;
    for(char c : text) {
        if (c == ' ') out += "  ";
        else out += std::string(1, c) + " ";
    }
    return out;
}

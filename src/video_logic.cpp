#include "video_logic.h"
#include "analytics.h"
#include "exporters.h"
#include "ai.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <random>
#include <algorithm>
#include <sys/ioctl.h>
#include <unistd.h>

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

void renderRegion(std::ostringstream& oss, const BrainRegion& region, int depth, const AppConfig& config) {
    for (int i = 0; i < depth; ++i) oss << "  ";
    oss << "[" << region.region_name << "] ";
    if (config.enable_color) oss << intensityToColor(region.intensity, config.theme);
    struct winsize w; int max_width = 20;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) max_width = std::max(10, w.ws_col / 4);
    int barWidth = static_cast<int>(region.intensity * max_width);
    for (int i = 0; i < barWidth; ++i) oss << intensityToSymbol(region.intensity, config.intensity_map);
    if (config.enable_color) oss << "\033[0m";
    if (!region.intensity_history.empty()) {
        oss << "  ";
        const char* sparkline[] = {" ", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
        for (double val : region.intensity_history) {
            int idx = std::max(0, std::min(7, (int)(val * 7)));
            oss << sparkline[idx];
        }
    }
    oss << "\n";
    for (const BrainRegion& subregion : region.subregions) renderRegion(oss, subregion, depth + 1, config);
}

void renderGrid(std::ostringstream& oss, const BrainFrame& frame, const AppConfig& config) {
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

void renderParticles(std::ostringstream& oss, const BrainFrame& frame) {
    oss << "--- Particles ---\n";
    for(const auto& r : frame.regions) {
        int count = (int)(r.intensity * 20);
        for(int i=0; i<count; i++) oss << "*";
        oss << "\n";
    }
}

void render3D(std::ostringstream& oss, const BrainFrame& frame, const AppConfig& config) {
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

std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input, const AppConfig& config) {
    std::vector<std::string> result;
    if (config.layout_mode == "quiet") return result;
    result.reserve(input.size());
    for (const BrainFrame& frame : input) {
        std::ostringstream oss;
        oss << "[Time: " << frame.timestamp_ms << " ms] " << generatePoeticDescription(frame) << "\n";
        if (config.layout_mode == "grid") renderGrid(oss, frame, config);
        else if (config.layout_mode == "3d") render3D(oss, frame, config);
        else if (config.layout_mode == "particles") renderParticles(oss, frame);
        else for (const BrainRegion& region : frame.regions) renderRegion(oss, region, 0, config);
        result.push_back(oss.str());
    }
    return result;
}

void smoothRegion(BrainRegion& region, int window_size) {
    region.intensity_history.push_back(region.intensity);
    if (region.intensity_history.size() > static_cast<size_t>(window_size)) region.intensity_history.pop_front();
    double sum = 0; for (double val : region.intensity_history) sum += val;
    region.intensity = sum / region.intensity_history.size();
    for (auto& sub : region.subregions) smoothRegion(sub, window_size);
}

void applyTemporalSmoothing(std::vector<BrainFrame>& frames, int window_size) {
    if (window_size <= 1) return;
    for (auto& frame : frames) for (auto& region : frame.regions) smoothRegion(region, window_size);
}

void decayRegion(BrainRegion& region, double decay_rate, int dt_ms) {
    region.intensity *= std::exp(-decay_rate * (dt_ms / 1000.0));
    for (auto& sub : region.subregions) decayRegion(sub, decay_rate, dt_ms);
}

void applyActivityDecayModel(std::vector<BrainFrame>& frames, double decay_rate) {
    if (decay_rate <= 0 || frames.size() < 2) return;
    for (size_t i = 1; i < frames.size(); ++i) {
        int dt = static_cast<int>(frames[i].timestamp_ms - frames[i-1].timestamp_ms);
        for (auto& region : frames[i].regions) decayRegion(region, decay_rate, dt);
    }
}

void delayRegion(BrainRegion& region, int delay_frames) {
    region.synaptic_buffer.push_back(region.intensity);
    if (region.synaptic_buffer.size() > static_cast<size_t>(delay_frames)) {
        region.intensity = region.synaptic_buffer.front(); region.synaptic_buffer.pop_front();
    } else region.intensity = 0;
    for (auto& sub : region.subregions) delayRegion(sub, delay_frames);
}

void applySynapticDelaySimulation(std::vector<BrainFrame>& frames, int delay_frames) {
    if (delay_frames <= 0) return;
    for (auto& frame : frames) for (auto& region : frame.regions) delayRegion(region, delay_frames);
}

void refractoryRegion(BrainRegion& region, int period_ms, int timestamp_ms) {
    if (region.intensity >= 0.9) region.last_peak_timestamp_ms = timestamp_ms;
    if (timestamp_ms - region.last_peak_timestamp_ms < period_ms) region.intensity *= 0.5;
    for (auto& sub : region.subregions) refractoryRegion(sub, period_ms, timestamp_ms);
}

void applyRefractoryPeriodLogic(std::vector<BrainFrame>& frames, int period_ms) {
    if (period_ms <= 0) return;
    for (auto& frame : frames) for (auto& region : frame.regions) refractoryRegion(region, period_ms, frame.timestamp_ms);
}

void applyStochasticModeling(std::vector<BrainFrame>& frames, double noise_amplitude) {
    if (noise_amplitude <= 0) return;
    std::random_device rd; std::mt19937 gen(rd()); std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (auto& frame : frames) {
        for (auto& region : frame.regions) {
            region.intensity = std::max(0.0, std::min(1.0, region.intensity + dist(gen) * noise_amplitude));
        }
    }
}

void transformRegion(BrainRegion& region, const std::string& transform) {
    if (transform == "sigmoid") region.intensity = 1.0 / (1.0 + std::exp(-10.0 * (region.intensity - 0.5)));
    else if (transform == "logarithmic") region.intensity = std::log(1.0 + region.intensity) / std::log(2.0);
    else if (transform == "square") region.intensity *= region.intensity;
    else if (transform == "sqrt") region.intensity = std::sqrt(region.intensity);
    region.intensity = std::max(0.0, std::min(1.0, region.intensity));
    for (auto& sub : region.subregions) transformRegion(sub, transform);
}

void applyCustomMathematicalFunctions(std::vector<BrainFrame>& frames, const std::string& transform) {
    if (transform == "linear" || transform.empty()) return;
    for (auto& frame : frames) for (auto& region : frame.regions) transformRegion(region, transform);
}

void neuroRegion(BrainRegion& region) {
    if (region.intensity > 0.7) { region.neurotransmitters["Glutamate"] = region.intensity; region.neurotransmitters["GABA"] *= 0.8; }
    else if (region.intensity < 0.3) { region.neurotransmitters["GABA"] = 1.0 - region.intensity; region.neurotransmitters["Glutamate"] *= 0.8; }
    for (auto& sub : region.subregions) neuroRegion(sub);
}

void applyNeurotransmitterSimulation(std::vector<BrainFrame>& frames) {
    for (auto& frame : frames) for (auto& region : frame.regions) neuroRegion(region);
}

void ltpRegion(BrainRegion& region, double threshold, double increment) {
    if (region.intensity > threshold) region.plasticity_factor += increment;
    region.intensity = std::min(1.0, region.intensity * region.plasticity_factor);
    for (auto& sub : region.subregions) ltpRegion(sub, threshold, increment);
}

void applyLongTermPotentiation(std::vector<BrainFrame>& frames, double threshold, double increment) {
    for (auto& frame : frames) for (auto& region : frame.regions) ltpRegion(region, threshold, increment);
}

std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor) {
    if (frames.empty() || factor <= 1) return frames;
    std::vector<BrainFrame> interpolated;
    for (size_t i = 0; i < frames.size() - 1; ++i) {
        interpolated.push_back(frames[i]);
        for (int j = 1; j < factor; ++j) {
            double t = static_cast<double>(j) / factor;
            BrainFrame f;
            f.timestamp_ms = frames[i].timestamp_ms + t * (frames[i+1].timestamp_ms - frames[i].timestamp_ms);
            for (size_t k = 0; k < frames[i].regions.size(); ++k) {
                BrainRegion r = frames[i].regions[k];
                r.intensity = frames[i].regions[k].intensity + t * (frames[i+1].regions[k].intensity - frames[i].regions[k].intensity);
                f.regions.push_back(r);
            }
            interpolated.push_back(f);
        }
    }
    interpolated.push_back(frames.back());
    return interpolated;
}

void applyASCIIShader(std::string& frame, const std::string& type) {
    if(type == "blur") {
        for(size_t i=1; i<frame.size()-1; i++) if(frame[i] == '@') frame[i] = 'X';
    } else if(type == "edge") {
        for(size_t i=1; i<frame.size()-1; i++) if(frame[i] != frame[i+1]) frame[i] = '|';
    }
}

void synthesizeRealTimeSound(const BrainFrame& frame) {
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

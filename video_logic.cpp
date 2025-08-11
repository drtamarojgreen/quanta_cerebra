#include "video_logic.h"
#include <sstream>
#include <iostream>

char intensityToChar(double intensity) {
    if (intensity < 0.2) return ' ';
    if (intensity < 0.4) return '.';
    if (intensity < 0.6) return ':';
    if (intensity < 0.75) return '+';
    if (intensity < 0.9) return 'X';
    return '@';
}

// Maps intensity value to a corresponding ANSI color code
const std::string& intensityToColor(double intensity) {
    if (intensity < 0.2) return colors::grey;
    if (intensity < 0.4) return colors::blue;
    if (intensity < 0.6) return colors::cyan;
    if (intensity < 0.75) return colors::green;
    if (intensity < 0.9) return colors::yellow;
    return colors::red;
}

// Recursively renders a brain region and its subregions
void renderRegion(std::ostringstream& oss, const BrainRegion& region, int depth, const AppConfig& config) {
    // Add indentation for hierarchy
    for (int i = 0; i < depth; ++i) {
        oss << "  ";
    }

    // Render the region name
    oss << "[" << region.region_name << "] ";

    // Apply color if enabled in config
    if (config.enable_color) {
        oss << intensityToColor(region.intensity);
    }

    // Render the intensity bar
    int barWidth = static_cast<int>(region.intensity * 10);
    for (int i = 0; i < barWidth; ++i) {
        oss << intensityToChar(region.intensity);
    }

    // Reset color if enabled
    if (config.enable_color) {
        oss << colors::reset;
    }
    oss << "\n";

    // Recursively render subregions
    for (const BrainRegion& subregion : region.subregions) {
        renderRegion(oss, subregion, depth + 1, config);
    }
}

// Generates ASCII video frames from brain activity data
std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input, const AppConfig& config) {
    std::vector<std::string> result;
    for (const BrainFrame& frame : input) {
        std::ostringstream oss;
        oss << "[Time: " << frame.timestamp_ms << " ms]\n";
        for (const BrainRegion& region : frame.regions) {
            renderRegion(oss, region, 0, config); // Start recursion with depth 0
        }
        result.push_back(oss.str());
    }
    return result;
}

// Interpolates frames to create smoother animations
std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor) {
    if (frames.empty() || factor <= 1) {
        return frames;
    }

    std::vector<BrainFrame> interpolated;
    for (size_t i = 0; i < frames.size() - 1; ++i) {
        const BrainFrame& currentFrame = frames[i];
        const BrainFrame& nextFrame = frames[i+1];
        interpolated.push_back(currentFrame);

        for (int j = 1; j < factor; ++j) {
            double t = static_cast<double>(j) / factor;
            BrainFrame newFrame;
            newFrame.timestamp_ms = static_cast<long long>(currentFrame.timestamp_ms + t * (nextFrame.timestamp_ms - currentFrame.timestamp_ms));

            for (const auto& currentRegion : currentFrame.regions) {
                for (const auto& nextRegion : nextFrame.regions) {
                    if (currentRegion.region_name == nextRegion.region_name) {
                        BrainRegion newRegion;
                        newRegion.region_name = currentRegion.region_name;
                        newRegion.intensity = currentRegion.intensity + t * (nextRegion.intensity - currentRegion.intensity);
                        // Note: Interpolation of subregions is not yet implemented
                        newFrame.regions.push_back(newRegion);
                        break;
                    }
                }
            }
            interpolated.push_back(newFrame);
        }
    }
    interpolated.push_back(frames.back());

    return interpolated;
}
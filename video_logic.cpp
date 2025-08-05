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

std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input) {
    std::vector<std::string> result;
    for (const BrainFrame& frame : input) {
        std::ostringstream oss;
        oss << "[Time: " << frame.timestamp_ms << " ms]\n";
        for (const BrainRegion& region : frame.regions) {
            oss << "[" << region.region_name << "] ";
            int barWidth = static_cast<int>(region.intensity * 10);
            for (int i = 0; i < barWidth; ++i) {
                oss << intensityToChar(region.intensity);
            }
            oss << "\n";
        }
        result.push_back(oss.str());
    }
    return result;
}

std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor) {
    std::cout << "Interpolating " << frames.size() << " frames with factor " << factor << std::endl;
    std::vector<BrainFrame> interpolated;
    if (frames.empty() || factor <= 1) {
        return frames;
    }

    for (size_t i = 0; i < frames.size() - 1; ++i) {
        std::cout << "Outer loop i = " << i << std::endl;
        const BrainFrame& currentFrame = frames[i];
        const BrainFrame& nextFrame = frames[i+1];
        interpolated.push_back(currentFrame);

        for (int j = 1; j < factor; ++j) {
            double t = static_cast<double>(j) / factor;
            BrainFrame newFrame;
            newFrame.timestamp_ms = static_cast<long long>(currentFrame.timestamp_ms + t * (nextFrame.timestamp_ms - currentFrame.timestamp_ms));
            std::cout << "Inner loop j = " << j << ", timestamp = " << newFrame.timestamp_ms << std::endl;

            for (const auto& currentRegion : currentFrame.regions) {
                for (const auto& nextRegion : nextFrame.regions) {
                    if (currentRegion.region_name == nextRegion.region_name) {
                        BrainRegion newRegion;
                        newRegion.region_name = currentRegion.region_name;
                        newRegion.intensity = currentRegion.intensity + t * (nextRegion.intensity - currentRegion.intensity);
                        newFrame.regions.push_back(newRegion);
                        break;
                    }
                }
            }
            std::cout << "Pushing back new frame" << std::endl;
            interpolated.push_back(newFrame);
        }
    }
    interpolated.push_back(frames.back());
    std::cout << "Interpolation complete, returning " << interpolated.size() << " frames" << std::endl;

    return interpolated;
}
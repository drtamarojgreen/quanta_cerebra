#include "video_logic.h"
#include <sstream>

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
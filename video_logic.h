#ifndef VIDEO_LOGIC_H
#define VIDEO_LOGIC_H
#include <vector>
#include <string>
#include "json_logic.h"
#include "config.h"

// ANSI color codes for terminal output
namespace colors {
    const std::string grey    = "\033[90m";
    const std::string blue    = "\033[94m";
    const std::string cyan    = "\033[96m";
    const std::string green   = "\033[92m";
    const std::string yellow  = "\033[93m";
    const std::string red     = "\033[91m";
    const std::string reset   = "\033[0m";
}

std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input, const AppConfig& config);
std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor);

#endif
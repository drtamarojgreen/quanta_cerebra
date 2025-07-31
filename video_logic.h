#ifndef VIDEO_LOGIC_H
#define VIDEO_LOGIC_H
#include <vector>
#include <string>
#include "json_logic.h"

std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input);
std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor);

#endif
#pragma once
#include "core/state_manager.h"
#include <vector>
#include <string>

namespace cerebra {
std::vector<BrainFrame> parse_yaml_frames(const std::string& yaml);
}

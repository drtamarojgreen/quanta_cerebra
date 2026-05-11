#pragma once
#include "core/state_manager.h"
#include <vector>
#include <string>

namespace cerebra {
std::vector<cerebra::BrainFrame> parse_csv_frames(const std::string& csv);
}

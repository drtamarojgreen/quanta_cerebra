#pragma once
#include "core/state_manager.h"
#include <vector>
#include <string>

namespace cerebra {
std::vector<BrainFrame> parse_xml_frames(const std::string& xml);
}

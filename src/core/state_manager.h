#pragma once

#include "core/atlas_region.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cerebra {

struct BrainFrame {
    std::int64_t timestamp_ms = 0;
    std::vector<RegionState> regions;
};

enum class BrainTemplate {
    Focused,
    Relaxed,
    Stressed,
    REMSleep,
};

const char* template_name(BrainTemplate t);
bool parse_template(std::string_view name, BrainTemplate& out);

std::string resolve_template_id(std::string_view input);
bool template_exists(std::string_view id);

BrainFrame template_frame(std::string_view template_id, std::int64_t timestamp_ms);
BrainFrame template_frame(BrainTemplate t, std::int64_t timestamp_ms);

} // namespace cerebra

#pragma once

#include "core/atlas_region.h"
#include "core/sample.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <map>

namespace cerebra {

class JsonValue;
class ActivityTimeline;

struct BrainFrame {
    std::int64_t timestamp_ms = 0;
    std::vector<RegionState> regions;

    double intensity_of(const std::string& region_key) const {
        for (const auto& r : regions) if (r.region == region_key) return r.intensity;
        return 0.0;
    }
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

struct BrainStateTemplate {
    std::string key;
    std::string display_name;
    std::string description;
    std::map<std::string, double> region_intensities;
    std::map<std::string, std::string> extra;
};

class BrainStateLibrary {
public:
    static const std::vector<BrainStateTemplate>& all();
    static const BrainStateTemplate* find(const std::string& key);
    static ActivityTimeline synthesize_timeline(const BrainStateTemplate& tmpl, int frames, std::int64_t step_ms);
    static void load_from_file(const std::string& path);
    static void load_from_json(const JsonValue& root);
    static void reset_to_defaults();
    static bool using_custom_catalog();
    static std::vector<std::string> keys();
};

} // namespace cerebra

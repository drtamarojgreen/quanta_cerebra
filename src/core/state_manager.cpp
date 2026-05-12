#include "core/state_manager.h"

#include "core/atlas_core.h"
#include "core/atlas_region.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace cerebra {

const char* template_name(BrainTemplate t) {
    switch (t) {
        case BrainTemplate::Focused:  return "focused";
        case BrainTemplate::Relaxed:  return "relaxed";
        case BrainTemplate::Stressed: return "stressed";
        case BrainTemplate::REMSleep: return "rem_sleep";
    }
    return "unknown";
}

namespace {

std::string to_lower(std::string_view s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

}

std::string resolve_template_id(std::string_view input) {
    std::string id = to_lower(input);
    if (id == "rem" || id == "rem-sleep") id = "rem_sleep";
    if (current_atlas().find_template(id)) return id;
    return {};
}

bool template_exists(std::string_view id) {
    return !resolve_template_id(id).empty();
}

bool parse_template(std::string_view name, BrainTemplate& out) {
    std::string id = resolve_template_id(name);
    if (id == "focused")    { out = BrainTemplate::Focused;  return true; }
    if (id == "relaxed")    { out = BrainTemplate::Relaxed;  return true; }
    if (id == "stressed")   { out = BrainTemplate::Stressed; return true; }
    if (id == "rem_sleep")  { out = BrainTemplate::REMSleep; return true; }
    return false;
}

cerebra::BrainFrame template_frame(std::string_view template_id, std::int64_t timestamp_ms) {
    cerebra::BrainFrame f;
    f.timestamp_ms = timestamp_ms;
    std::string canonical = resolve_template_id(template_id);
    if (canonical.empty()) return f;
    const TemplateDefinition* def = current_atlas().find_template(canonical);
    if (!def) return f;
    for (const auto& kv : def->intensities) {
        cerebra::RegionState rs;
        rs.region = kv.first;
        double inten = kv.second;
        if (inten < 0.0) inten = 0.0;
        if (inten > 1.0) inten = 1.0;
        rs.intensity = inten;
        rs.flows = default_flows_for(kv.first, inten);
        f.regions.push_back(std::move(rs));
    }
    return f;
}

cerebra::BrainFrame template_frame(BrainTemplate t, std::int64_t timestamp_ms) {
    return template_frame(template_name(t), timestamp_ms);
}

const std::vector<BrainStateTemplate>& BrainStateLibrary::all() {
    static std::vector<BrainStateTemplate> cache;
    cache.clear();
    for (const auto& t : current_atlas().templates()) {
        BrainStateTemplate bst;
        bst.key = t.id;
        bst.display_name = t.display_name;
        bst.region_intensities = t.intensities;
        cache.push_back(bst);
    }
    return cache;
}

const BrainStateTemplate* BrainStateLibrary::find(const std::string& key) {
    std::string id = resolve_template_id(key);
    if (id.empty()) return nullptr;
    const TemplateDefinition* def = current_atlas().find_template(id);
    if (!def) return nullptr;
    static BrainStateTemplate static_bst;
    static_bst.key = def->id;
    static_bst.display_name = def->display_name;
    static_bst.region_intensities = def->intensities;
    return &static_bst;
}

ActivityTimeline BrainStateLibrary::synthesize_timeline(const BrainStateTemplate& tmpl, int frames, std::int64_t step_ms) {
    std::vector<BrainActivitySample> samples;
    if (frames <= 0) frames = 1;
    for (int i = 0; i < frames; ++i) {
        BrainActivitySample s;
        s.timestamp_ms = i * step_ms;
        s.intensities = tmpl.region_intensities;
        samples.push_back(std::move(s));
    }
    return ActivityTimeline(std::move(samples));
}

void BrainStateLibrary::load_from_file(const std::string& /* path */) {}
void BrainStateLibrary::load_from_json(const JsonValue& /* root */) {}
void BrainStateLibrary::reset_to_defaults() { reset_current_atlas_to_builtin(); }
bool BrainStateLibrary::using_custom_catalog() { return false; }

std::vector<std::string> BrainStateLibrary::keys() {
    std::vector<std::string> k;
    for (const auto& t : current_atlas().templates()) k.push_back(t.id);
    return k;
}

}

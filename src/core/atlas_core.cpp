#include "core/atlas_core.h"
#include "io/json_parser.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <utility>

namespace cerebra {

namespace {

RegionAtlas build_builtin() {
    try {
        return load_atlas_file("data/builtin_atlas.json");
    } catch (...) {
        // Fallback to empty atlas if file missing or corrupt
        return RegionAtlas();
    }
}

RegionAtlas& current_atlas_storage() {
    static RegionAtlas atlas = build_builtin();
    return atlas;
}

}

const RegionAtlas& RegionAtlas::builtin() {
    static const RegionAtlas atlas = build_builtin();
    return atlas;
}

const cerebra::RegionDefinition* RegionAtlas::find(std::string_view id) const {
    for (const auto& r : regions_) if (r.id == id) return &r;
    return nullptr;
}

void RegionAtlas::add_or_replace(cerebra::RegionDefinition def) {
    for (auto& r : regions_) {
        if (r.id == def.id) { r = std::move(def); return; }
    }
    regions_.push_back(std::move(def));
}

bool RegionAtlas::remove(std::string_view id) {
    auto it = std::find_if(regions_.begin(), regions_.end(),
                           [&](const cerebra::RegionDefinition& r){ return r.id == id; });
    if (it == regions_.end()) return false;
    regions_.erase(it);
    return true;
}

const PathwayDefinition* RegionAtlas::find_pathway(std::string_view id) const {
    for (const auto& p : pathways_) if (p.id == id) return &p;
    return nullptr;
}

void RegionAtlas::add_or_replace_pathway(PathwayDefinition def) {
    for (auto& p : pathways_) {
        if (p.id == def.id) { p = std::move(def); return; }
    }
    pathways_.push_back(std::move(def));
}

bool RegionAtlas::remove_pathway(std::string_view id) {
    auto it = std::find_if(pathways_.begin(), pathways_.end(),
                           [&](const PathwayDefinition& p){ return p.id == id; });
    if (it == pathways_.end()) return false;
    pathways_.erase(it);
    return true;
}

const TemplateDefinition* RegionAtlas::find_template(std::string_view id) const {
    for (const auto& t : templates_) if (t.id == id) return &t;
    return nullptr;
}

void RegionAtlas::add_or_replace_template(TemplateDefinition def) {
    for (auto& t : templates_) {
        if (t.id == def.id) { t = std::move(def); return; }
    }
    templates_.push_back(std::move(def));
}

bool RegionAtlas::remove_template(std::string_view id) {
    auto it = std::find_if(templates_.begin(), templates_.end(),
                           [&](const TemplateDefinition& t){ return t.id == id; });
    if (it == templates_.end()) return false;
    templates_.erase(it);
    return true;
}

RegionAtlas RegionAtlas::from_json(std::string_view text) {
    return parse_atlas(text);
}

RegionAtlas RegionAtlas::from_file(const std::string& path) {
    return load_atlas_file(path);
}

const RegionAtlas& current_atlas() {
    return current_atlas_storage();
}

void set_current_atlas(RegionAtlas atlas) {
    current_atlas_storage() = std::move(atlas);
}

void reset_current_atlas_to_builtin() {
    current_atlas_storage() = RegionAtlas::builtin();
}

}

#pragma once

#include <iosfwd>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace cerebra {

// A baseline (intensity=1.0) neurotransmitter rate associated with a region.
struct AtlasFlow {
    std::string transmitter;
    double base_rate = 0.0;
};

// A single region in the atlas: identity, display metadata, where it lives in
// the 2D coronal slice, where it lives in the 3D projection, and the baseline
// neurotransmitter flows it drives.
struct RegionDefinition {
    std::string id;
    std::string key;
    std::string display_name;
    std::string abbreviation;
    std::string primary_transmitter;
    bool region_of_interest = false;

    double slice_x = 0.5;
    double slice_y = 0.5;
    double depth = 0.5;
    std::map<std::string, std::string> extra;

    int slice_row = 0;
    int slice_col = 0;
    int slice_w = 1;
    int slice_h = 1;

    double proj_x = 0.0;
    double proj_y = 0.0;
    double proj_z = 0.0;
    double proj_radius = 0.2;

    std::vector<AtlasFlow> flows;
};

// A directed neurological pathway through one or more regions.
struct PathwayDefinition {
    std::string id;
    std::string name;
    std::vector<std::string> nodes;
    std::string transmitter;
    double strength = 0.0;
    bool bidirectional = false;
};

// A named brain-state preset: an id, a display name, and a region-id to
// intensity map.
struct TemplateDefinition {
    std::string id;
    std::string display_name;
    std::map<std::string, double> intensities;
};

class AtlasError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// A pluggable collection of region definitions.
class RegionAtlas {
public:
    static const RegionAtlas& builtin();

    static RegionAtlas from_json(std::string_view text);
    static RegionAtlas from_file(const std::string& path);

    const std::vector<cerebra::RegionDefinition>& regions() const { return regions_; }
    const cerebra::RegionDefinition* find(std::string_view id) const;

    void add_or_replace(cerebra::RegionDefinition def);
    bool remove(std::string_view id);

    std::size_t size() const { return regions_.size(); }
    bool empty() const { return regions_.empty(); }

    const std::vector<PathwayDefinition>& pathways() const { return pathways_; }
    const PathwayDefinition* find_pathway(std::string_view id) const;
    void add_or_replace_pathway(PathwayDefinition def);
    bool remove_pathway(std::string_view id);

    const std::vector<TemplateDefinition>& templates() const { return templates_; }
    const TemplateDefinition* find_template(std::string_view id) const;
    void add_or_replace_template(TemplateDefinition def);
    bool remove_template(std::string_view id);

private:
    std::vector<cerebra::RegionDefinition> regions_;
    std::vector<PathwayDefinition> pathways_;
    std::vector<TemplateDefinition> templates_;
};

const RegionAtlas& current_atlas();
void set_current_atlas(RegionAtlas atlas);
void reset_current_atlas_to_builtin();

}

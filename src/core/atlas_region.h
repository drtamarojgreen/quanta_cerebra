#pragma once

#include "core/atlas_core.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace cerebra {

// A scaled neurotransmitter flow for a particular cerebra::RegionState. `type` is a
// free-form string (see AtlasFlow::transmitter) so atlases can introduce
// novel neurochemistry.
struct NeurotransmitterFlow {
    std::string type;
    double rate = 0.0;
};

using RegionInfo = cerebra::RegionDefinition;

// Convenience accessors that delegate to the *current* atlas. They keep the
// pre-atlas call sites working while still routing through one source of
// truth for region geometry and flows.
const std::vector<cerebra::RegionDefinition>& known_regions();
const cerebra::RegionDefinition* find_region(std::string_view id);
std::vector<NeurotransmitterFlow> default_flows_for(std::string_view region, double intensity);

class RegionCatalog {
public:
    static const std::vector<RegionInfo>& all();
    static std::optional<RegionInfo> find(const std::string& key);
    static std::string normalize_key(const std::string& raw);
    static bool is_region_of_interest(const std::string& key);
    static void load_from_file(const std::string& path);
};

// Per-frame snapshot of a single region.
struct RegionState {
    std::string region;
    double intensity = 0.0;
    std::vector<NeurotransmitterFlow> flows;
    // Arbitrary named statistics carried alongside intensity. Atlases and
    // upstream data sources can populate this freely; the display surface
    // renders whatever is present.
    std::map<std::string, double> metrics;

    // Advanced Modeling
    std::map<std::string, double> neurotransmitters;
    double plasticity_factor = 1.0;
    std::vector<RegionState> subregions;

    std::vector<double> intensity_history;
    std::vector<double> synaptic_buffer;
    std::vector<void*> synapses;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    RegionState() = default;
    RegionState(std::string r, double i) : region(std::move(r)), intensity(i) {}
};

}

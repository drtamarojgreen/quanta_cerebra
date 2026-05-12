#pragma once

#include "core/atlas_core.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace cerebra {

// A scaled neurotransmitter flow for a particular RegionState. `type` is a
// free-form string (see AtlasFlow::transmitter) so atlases can introduce
// novel neurochemistry.
struct NeurotransmitterFlow {
    std::string type;
    double rate = 0.0;
};

// Backwards-compatible alias for code that referred to the old RegionInfo
// struct (with slice_*/display_name/id). The atlas-driven RegionDefinition
// is a strict superset.
using RegionInfo = RegionDefinition;

// Convenience accessors that delegate to the *current* atlas. They keep the
// pre-atlas call sites working while still routing through one source of
// truth for region geometry and flows.
const std::vector<RegionDefinition>& known_regions();
const RegionDefinition* find_region(std::string_view id);
std::vector<NeurotransmitterFlow> default_flows_for(std::string_view region, double intensity);

// Per-frame snapshot of a single region.
struct RegionState {
    std::string region;
    double intensity = 0.0;
    std::vector<NeurotransmitterFlow> flows;
    // Arbitrary named statistics carried alongside intensity. Atlases and
    // upstream data sources can populate this freely; the display surface
    // renders whatever is present.
    std::map<std::string, double> metrics;
};

}

#include "core/bm_atlas_region.h"

#include "core/atlas_core.h"

namespace cerebra {

const std::vector<RegionDefinition>& known_regions() {
    return current_atlas().regions();
}

const RegionDefinition* find_region(std::string_view id) {
    return current_atlas().find(id);
}

std::vector<NeurotransmitterFlow> default_flows_for(std::string_view region,
                                                    double intensity) {
    std::vector<NeurotransmitterFlow> out;
    if (const RegionDefinition* def = current_atlas().find(region)) {
        out.reserve(def->flows.size());
        for (const auto& f : def->flows) {
            out.push_back({f.transmitter, f.base_rate * intensity});
        }
        return out;
    }
    // Unknown region: a single low-level glutamatergic fallback so display code
    // never has to special-case an empty flow list.
    out.push_back({"glutamate", 0.5 * intensity});
    return out;
}

}

#include "core/atlas_region.h"

#include "core/atlas_core.h"
#include <algorithm>

namespace cerebra {

const std::vector<cerebra::RegionDefinition>& known_regions() {
    return current_atlas().regions();
}

const cerebra::RegionDefinition* find_region(std::string_view id) {
    return current_atlas().find(id);
}

std::vector<NeurotransmitterFlow> default_flows_for(std::string_view region,
                                                    double intensity) {
    std::vector<NeurotransmitterFlow> out;
    if (const cerebra::RegionDefinition* def = current_atlas().find(region)) {
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

const std::vector<RegionInfo>& RegionCatalog::all() {
    return current_atlas().regions();
}

std::optional<RegionInfo> RegionCatalog::find(const std::string& key) {
    const auto* def = current_atlas().find(key);
    if (def) return *def;
    return std::nullopt;
}

std::string RegionCatalog::normalize_key(const std::string& raw) {
    std::string out = raw;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

bool RegionCatalog::is_region_of_interest(const std::string& /* key */) {
    return false;
}

void RegionCatalog::load_from_file(const std::string& /* path */) {}

}

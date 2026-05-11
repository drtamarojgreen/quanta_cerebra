#include "test_harness.hpp"
#include "core/atlas_region.h"

#include <set>
#include <string>

BM_CASE(all_required_regions_present) {
    std::set<std::string> ids;
    for (const auto& r : cerebra::known_regions()) ids.insert(r.id);
    for (const char* id : {
            "amygdala", "cuneus", "anterior_cingulate_cortex", "insula",
            "gyrus", "hippocampus", "thalamus", "entorhinal_cortex",
            "prefrontal_cortex", "occipital_lobe"}) {
        BM_REQUIRE(ids.count(id) == 1);
    }
}

BM_CASE(find_region_works) {
    BM_REQUIRE(cerebra::find_region("amygdala") != nullptr);
    BM_REQUIRE(cerebra::find_region("doesnt_exist") == nullptr);
}

BM_CASE(default_flows_scale_with_intensity) {
    auto low  = cerebra::default_flows_for("amygdala", 0.2);
    auto high = cerebra::default_flows_for("amygdala", 1.0);
    BM_REQUIRE(!low.empty());
    BM_REQUIRE_EQ(low.size(), high.size());
    BM_REQUIRE(low[0].rate < high[0].rate);
}

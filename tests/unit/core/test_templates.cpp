#include "../test_harness.h"
#include "core/state_manager.h"

#include <string>

static double intensity(const cerebra::BrainFrame& f, const std::string& id) {
    for (const auto& r : f.regions) if (r.region == id) return r.intensity;
    return -1;
}

BM_CASE(parse_template_names) {
    cerebra::BrainTemplate t;
    BM_REQUIRE(cerebra::parse_template("focused", t)  && t == cerebra::BrainTemplate::Focused);
    BM_REQUIRE(cerebra::parse_template("Relaxed", t)  && t == cerebra::BrainTemplate::Relaxed);
    BM_REQUIRE(cerebra::parse_template("stressed", t) && t == cerebra::BrainTemplate::Stressed);
    BM_REQUIRE(cerebra::parse_template("rem", t)      && t == cerebra::BrainTemplate::REMSleep);
    BM_REQUIRE(cerebra::parse_template("rem_sleep", t)&& t == cerebra::BrainTemplate::REMSleep);
    BM_REQUIRE(!cerebra::parse_template("nope", t));
}

BM_CASE(focused_pfc_high) {
    auto f = cerebra::template_frame(cerebra::BrainTemplate::Focused, 0);
    BM_REQUIRE(intensity(f, "prefrontal_cortex") > 0.7);
    BM_REQUIRE(intensity(f, "amygdala") < 0.4);
}

BM_CASE(stressed_amygdala_dominant) {
    auto f = cerebra::template_frame(cerebra::BrainTemplate::Stressed, 0);
    BM_REQUIRE(intensity(f, "amygdala") > 0.8);
    BM_REQUIRE(intensity(f, "insula") > 0.7);
}

BM_CASE(rem_sleep_pfc_low) {
    auto f = cerebra::template_frame(cerebra::BrainTemplate::REMSleep, 0);
    BM_REQUIRE(intensity(f, "prefrontal_cortex") < 0.4);
    BM_REQUIRE(intensity(f, "hippocampus") > 0.5);
}

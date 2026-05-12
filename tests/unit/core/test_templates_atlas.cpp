// Tests for the atlas-backed template system. test_templates.cpp still covers
// the legacy enum API; this file focuses on the atlas-driven entry points and
// the JSON pathway for adding/overriding templates without recompiling.

#include "../test_harness.h"

#include "core/atlas_core.h"
#include "core/atlas_region.h"
#include "core/state_manager.h"

#include <string>

static double intensity_of(const cerebra::BrainFrame& f, const std::string& id) {
    for (const auto& r : f.regions) if (r.region == id) return r.intensity;
    return -1.0;
}

BM_CASE(atlas_ships_four_builtin_templates) {
    cerebra::reset_current_atlas_to_builtin();
    const auto& ts = cerebra::current_atlas().templates();
    BM_REQUIRE_EQ(ts.size(), std::size_t(4));
    BM_REQUIRE(cerebra::current_atlas().find_template("focused")    != nullptr);
    BM_REQUIRE(cerebra::current_atlas().find_template("relaxed")    != nullptr);
    BM_REQUIRE(cerebra::current_atlas().find_template("stressed")   != nullptr);
    BM_REQUIRE(cerebra::current_atlas().find_template("rem_sleep")  != nullptr);
}

BM_CASE(builtin_template_intensities_match_legacy_values) {
    // These values must remain stable so behaviour is preserved across the
    // port from the hard-coded switch to the atlas.
    cerebra::reset_current_atlas_to_builtin();
    const auto* t = cerebra::current_atlas().find_template("focused");
    BM_REQUIRE_NEAR(t->intensities.at("prefrontal_cortex"), 0.90, 1e-9);
    BM_REQUIRE_NEAR(t->intensities.at("amygdala"),          0.20, 1e-9);

    t = cerebra::current_atlas().find_template("stressed");
    BM_REQUIRE_NEAR(t->intensities.at("amygdala"),          0.95, 1e-9);
    BM_REQUIRE_NEAR(t->intensities.at("insula"),            0.90, 1e-9);
}

BM_CASE(template_frame_string_form_matches_enum_form) {
    cerebra::reset_current_atlas_to_builtin();
    auto a = cerebra::template_frame("focused", 123);
    auto b = cerebra::template_frame(cerebra::BrainTemplate::Focused, 123);
    BM_REQUIRE_EQ(a.timestamp_ms, b.timestamp_ms);
    BM_REQUIRE_EQ(a.regions.size(), b.regions.size());
    BM_REQUIRE_NEAR(intensity_of(a, "prefrontal_cortex"),
                    intensity_of(b, "prefrontal_cortex"), 1e-12);
    BM_REQUIRE_NEAR(intensity_of(a, "amygdala"),
                    intensity_of(b, "amygdala"), 1e-12);
}

BM_CASE(resolve_template_id_handles_aliases_and_case) {
    cerebra::reset_current_atlas_to_builtin();
    BM_REQUIRE_EQ(cerebra::resolve_template_id("Focused"),    std::string("focused"));
    BM_REQUIRE_EQ(cerebra::resolve_template_id("STRESSED"),   std::string("stressed"));
    BM_REQUIRE_EQ(cerebra::resolve_template_id("rem"),        std::string("rem_sleep"));
    BM_REQUIRE_EQ(cerebra::resolve_template_id("rem-sleep"),  std::string("rem_sleep"));
    BM_REQUIRE(cerebra::resolve_template_id("unknown").empty());
}

BM_CASE(template_exists_reflects_resolve) {
    cerebra::reset_current_atlas_to_builtin();
    BM_REQUIRE(cerebra::template_exists("focused"));
    BM_REQUIRE(cerebra::template_exists("REM"));
    BM_REQUIRE(!cerebra::template_exists("flow_state"));
}

BM_CASE(template_frame_unknown_id_returns_empty_frame) {
    cerebra::reset_current_atlas_to_builtin();
    auto f = cerebra::template_frame("nope", 0);
    BM_REQUIRE(f.regions.empty());
    BM_REQUIRE_EQ(f.timestamp_ms, std::int64_t(0));
}

BM_CASE(from_json_loads_custom_template) {
    const char* src = R"({
        "extends_builtin": true,
        "templates": [
            {
                "id": "meditation",
                "display_name": "Meditation",
                "regions": {
                    "prefrontal_cortex": 0.6,
                    "anterior_cingulate_cortex": 0.7,
                    "insula": 0.6,
                    "amygdala": 0.1
                }
            }
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.templates().size(), std::size_t(5));  // 4 builtin + meditation
    const auto* t = a.find_template("meditation");
    BM_REQUIRE(t != nullptr);
    BM_REQUIRE_EQ(t->display_name, std::string("Meditation"));
    BM_REQUIRE_NEAR(t->intensities.at("amygdala"), 0.1, 1e-9);
}

BM_CASE(from_json_template_overrides_existing) {
    const char* src = R"({
        "extends_builtin": true,
        "templates": [
            { "id":"focused","display_name":"Hyper-focused",
              "regions": {"prefrontal_cortex": 1.0} }
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.templates().size(), std::size_t(4));
    const auto* t = a.find_template("focused");
    BM_REQUIRE_EQ(t->display_name, std::string("Hyper-focused"));
    BM_REQUIRE_NEAR(t->intensities.at("prefrontal_cortex"), 1.0, 1e-9);
    // Old keys are gone — override is a full replacement, not a merge.
    BM_REQUIRE_EQ(t->intensities.size(), std::size_t(1));
}

BM_CASE(from_json_template_clamps_intensities) {
    const char* src = R"({
        "templates": [
            { "id":"weird","regions":{"a": 1.7, "b": -0.5} }
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    const auto* t = a.find_template("weird");
    BM_REQUIRE_NEAR(t->intensities.at("a"), 1.0, 1e-9);
    BM_REQUIRE_NEAR(t->intensities.at("b"), 0.0, 1e-9);
}

BM_CASE(from_json_template_defaults_display_name_to_id) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"templates":[{"id":"x","regions":{"a":0.5}}]})");
    BM_REQUIRE_EQ(a.find_template("x")->display_name, std::string("x"));
}

BM_CASE(from_json_rejects_template_without_regions) {
    bool threw = false;
    try {
        cerebra::RegionAtlas::from_json(R"({"templates":[{"id":"x"}]})");
    } catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_json_rejects_non_numeric_intensity) {
    bool threw = false;
    try {
        cerebra::RegionAtlas::from_json(
            R"({"templates":[{"id":"x","regions":{"a":"hello"}}]})");
    } catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(template_frame_uses_custom_atlas_template) {
    cerebra::RegionAtlas a = cerebra::RegionAtlas::builtin();
    cerebra::TemplateDefinition def;
    def.id = "flow_state";
    def.display_name = "Flow State";
    def.intensities = {
        {"prefrontal_cortex", 0.85},
        {"amygdala",          0.15},
    };
    a.add_or_replace_template(def);
    cerebra::set_current_atlas(std::move(a));

    auto f = cerebra::template_frame("flow_state", 500);
    BM_REQUIRE_NEAR(intensity_of(f, "prefrontal_cortex"), 0.85, 1e-9);
    BM_REQUIRE_NEAR(intensity_of(f, "amygdala"),          0.15, 1e-9);
    BM_REQUIRE_EQ(f.timestamp_ms, std::int64_t(500));

    // Flow defaults come from the atlas, so the custom-template regions have
    // the atlas's baseline neurotransmitter profile applied.
    bool found_dopamine = false;
    for (const auto& r : f.regions) {
        if (r.region == "prefrontal_cortex") {
            for (const auto& nf : r.flows) if (nf.type == "dopamine") found_dopamine = true;
        }
    }
    BM_REQUIRE(found_dopamine);

    cerebra::reset_current_atlas_to_builtin();
}

BM_CASE(remove_template_returns_false_for_unknown) {
    cerebra::RegionAtlas a;
    BM_REQUIRE(!a.remove_template("nope"));
    cerebra::TemplateDefinition def;
    def.id = "x";
    def.intensities = {{"a", 0.5}};
    a.add_or_replace_template(def);
    BM_REQUIRE(a.remove_template("x"));
    BM_REQUIRE_EQ(a.templates().size(), std::size_t(0));
}

BM_CASE(atlas_with_only_templates_is_valid) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"templates":[{"id":"t","regions":{"a":0.5}}]})");
    BM_REQUIRE_EQ(a.templates().size(), std::size_t(1));
    BM_REQUIRE(a.regions().empty());
}

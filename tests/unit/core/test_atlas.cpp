#include "test_harness.hpp"

#include "core/atlas_core.h"
#include "core/atlas_region.h"
#include "io/atlas_json_parser.h"

#include <string>

BM_CASE(builtin_atlas_has_ten_regions) {
    const auto& atlas = cerebra::RegionAtlas::builtin();
    BM_REQUIRE_EQ(atlas.size(), std::size_t(10));
    BM_REQUIRE(atlas.find("amygdala") != nullptr);
}

BM_CASE(builtin_atlas_json_text_is_embedded_and_parses) {
    // The CMake-embedded data/builtin_atlas.json must be present and valid.
    const char* text = cerebra::builtin_atlas_json_text();
    BM_REQUIRE(text != nullptr);
    BM_REQUIRE(std::string(text).find("\"regions\")   != std::string::npos);
    BM_REQUIRE(std::string(text).find("\"pathways\")  != std::string::npos);
    BM_REQUIRE(std::string(text).find("\"templates\") != std::string::npos);

    auto atlas = cerebra::RegionAtlas::from_json(text);
    BM_REQUIRE_EQ(atlas.regions().size(),   std::size_t(10));
    BM_REQUIRE_EQ(atlas.pathways().size(),  std::size_t(4));
    BM_REQUIRE_EQ(atlas.templates().size(), std::size_t(4));
}

BM_CASE(builtin_atlas_round_trip_matches_runtime_builtin) {
    // Parsing the embedded JSON must produce an atlas equivalent to the one
    // RegionAtlas::builtin() hands out (which is itself parsed from the same
    // JSON, so this guards against drift if someone ever re-introduces inline
    // overrides).
    auto from_text = cerebra::RegionAtlas::from_json(cerebra::builtin_atlas_json_text());
    const auto& builtin = cerebra::RegionAtlas::builtin();
    BM_REQUIRE_EQ(from_text.regions().size(),   builtin.regions().size());
    BM_REQUIRE_EQ(from_text.pathways().size(),  builtin.pathways().size());
    BM_REQUIRE_EQ(from_text.templates().size(), builtin.templates().size());
    for (const auto& r : builtin.regions()) {
        const auto* m = from_text.find(r.id);
        BM_REQUIRE(m != nullptr);
        BM_REQUIRE_EQ(m->display_name, r.display_name);
        BM_REQUIRE_EQ(m->slice_row, r.slice_row);
        BM_REQUIRE_EQ(m->slice_col, r.slice_col);
        BM_REQUIRE_EQ(m->flows.size(), r.flows.size());
    }
    for (const auto& p : builtin.pathways()) {
        const auto* m = from_text.find_pathway(p.id);
        BM_REQUIRE(m != nullptr);
        BM_REQUIRE_EQ(m->nodes.size(), p.nodes.size());
        BM_REQUIRE_NEAR(m->strength, p.strength, 1e-9);
    }
    for (const auto& t : builtin.templates()) {
        const auto* m = from_text.find_template(t.id);
        BM_REQUIRE(m != nullptr);
        BM_REQUIRE_EQ(m->intensities.size(), t.intensities.size());
    }
}

BM_CASE(current_atlas_defaults_to_builtin) {
    cerebra::reset_current_atlas_to_builtin();
    BM_REQUIRE_EQ(cerebra::current_atlas().size(), std::size_t(10));
    BM_REQUIRE(cerebra::current_atlas().find("hippocampus") != nullptr);
}

BM_CASE(add_or_replace_inserts_then_updates) {
    cerebra::RegionAtlas a;
    cerebra::RegionDefinition def;
    def.id = "test_region";
    def.display_name = "Test";
    def.slice_row = 1; def.slice_col = 2; def.slice_w = 3; def.slice_h = 1;
    a.add_or_replace(def);
    BM_REQUIRE_EQ(a.size(), std::size_t(1));
    BM_REQUIRE(a.find("test_region") != nullptr);

    def.display_name = "Test v2";
    a.add_or_replace(def);
    BM_REQUIRE_EQ(a.size(), std::size_t(1));
    BM_REQUIRE_EQ(a.find("test_region")->display_name, std::string("Test v2"));
}

BM_CASE(remove_returns_false_for_unknown) {
    cerebra::RegionAtlas a;
    BM_REQUIRE(!a.remove("nope"));
    cerebra::RegionDefinition def;
    def.id = "x";
    a.add_or_replace(def);
    BM_REQUIRE(a.remove("x"));
    BM_REQUIRE_EQ(a.size(), std::size_t(0));
}

BM_CASE(from_json_loads_single_region) {
    const char* src = R"({
        "regions": [
            {
                "id": "cerebellum",
                "display_name": "Cerebellum",
                "slice": {"row":20,"col":14,"w":10,"h":2},
                "projection": {"x":0.1,"y":-0.5,"z":-0.5,"radius":0.2},
                "flows": {"glutamate":0.7,"GABA":0.6}
            }
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.size(), std::size_t(1));
    const auto* d = a.find("cerebellum");
    BM_REQUIRE(d != nullptr);
    BM_REQUIRE_EQ(d->display_name, std::string("Cerebellum"));
    BM_REQUIRE_EQ(d->slice_col, 14);
    BM_REQUIRE_NEAR(d->proj_radius, 0.2, 1e-9);
    BM_REQUIRE_EQ(d->flows.size(), std::size_t(2));
}

BM_CASE(from_json_extends_builtin) {
    const char* src = R"({
        "extends_builtin": true,
        "regions": [
            {
                "id": "cerebellum",
                "display_name": "Cerebellum",
                "slice": {"row":20,"col":14,"w":10,"h":2},
                "projection": {"x":0.1,"y":-0.5,"z":-0.5,"radius":0.2},
                "flows": {"glutamate":0.7}
            }
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.size(), std::size_t(11)); // 10 builtin + 1 new
    BM_REQUIRE(a.find("amygdala") != nullptr);
    BM_REQUIRE(a.find("cerebellum") != nullptr);
}

BM_CASE(from_json_overrides_existing) {
    const char* src = R"json({
        "extends_builtin": true,
        "regions": [
            {
                "id": "amygdala",
                "display_name": "Amygdala (lateral)",
                "slice": {"row":15,"col":6,"w":6,"h":2},
                "projection": {"x":-0.3,"y":-0.4,"z":-0.1,"radius":0.16},
                "flows": {"norepinephrine":0.99}
            }
        ]
    })json";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.size(), std::size_t(10));
    const auto* d = a.find("amygdala");
    BM_REQUIRE(d != nullptr);
    BM_REQUIRE_EQ(d->display_name, std::string("Amygdala (lateral)"));
    BM_REQUIRE_EQ(d->flows.size(), std::size_t(1));
    BM_REQUIRE_EQ(d->flows[0].transmitter, std::string("norepinephrine"));
}

BM_CASE(from_json_rejects_top_level_array) {
    bool threw = false;
    try { cerebra::RegionAtlas::from_json("[]"); }
    catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_json_requires_regions_array) {
    bool threw = false;
    try { cerebra::RegionAtlas::from_json("{}"); }
    catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_json_requires_region_id) {
    bool threw = false;
    try {
        cerebra::RegionAtlas::from_json(R"({"regions":[{"display_name":"x"}]})");
    } catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_file_missing_throws) {
    bool threw = false;
    try { cerebra::RegionAtlas::from_file("/no/such/atlas.json"); }
    catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(set_current_atlas_changes_known_regions) {
    cerebra::RegionAtlas a;
    cerebra::RegionDefinition def;
    def.id = "only";
    def.display_name = "Only Region";
    a.add_or_replace(def);
    cerebra::set_current_atlas(a);
    BM_REQUIRE_EQ(cerebra::known_regions().size(), std::size_t(1));
    BM_REQUIRE_EQ(cerebra::known_regions()[0].id, std::string("only"));
    cerebra::reset_current_atlas_to_builtin();
    BM_REQUIRE_EQ(cerebra::known_regions().size(), std::size_t(10));
}

BM_CASE(default_flows_uses_atlas_baseline) {
    cerebra::reset_current_atlas_to_builtin();
    auto flows = cerebra::default_flows_for("amygdala", 0.5);
    BM_REQUIRE(!flows.empty());
    // builtin amygdala has norepinephrine base_rate=0.9
    bool found = false;
    for (const auto& f : flows) {
        if (f.type == "norepinephrine") {
            BM_REQUIRE_NEAR(f.rate, 0.45, 1e-9);
            found = true;
        }
    }
    BM_REQUIRE(found);
}

BM_CASE(default_flows_picks_up_custom_transmitter) {
    cerebra::RegionAtlas a;
    cerebra::RegionDefinition def;
    def.id = "test_region";
    def.display_name = "Test";
    def.flows.push_back({"endocannabinoid", 0.8});
    a.add_or_replace(def);
    cerebra::set_current_atlas(a);

    auto flows = cerebra::default_flows_for("test_region", 0.5);
    cerebra::reset_current_atlas_to_builtin();

    BM_REQUIRE_EQ(flows.size(), std::size_t(1));
    BM_REQUIRE_EQ(flows[0].type, std::string("endocannabinoid"));
    BM_REQUIRE_NEAR(flows[0].rate, 0.4, 1e-9);
}

BM_CASE(default_flows_unknown_region_returns_fallback) {
    cerebra::reset_current_atlas_to_builtin();
    auto flows = cerebra::default_flows_for("not_in_atlas", 1.0);
    BM_REQUIRE_EQ(flows.size(), std::size_t(1));
    BM_REQUIRE_EQ(flows[0].type, std::string("glutamate"));
}

BM_CASE(json_input_parses_metrics_map) {
    cerebra::reset_current_atlas_to_builtin();
    auto frames = cerebra::parse_frames(R"([{
        "brain_activity":[{
            "region":"amygdala",
            "intensity":0.6,
            "metrics":{"bold":0.82,"alpha_power":0.31}
        }],
        "timestamp_ms":0
    }])");
    BM_REQUIRE_EQ(frames.size(), std::size_t(1));
    const auto& rs = frames[0].regions[0];
    BM_REQUIRE_EQ(rs.metrics.size(), std::size_t(2));
    BM_REQUIRE_NEAR(rs.metrics.at("bold"), 0.82, 1e-9);
    BM_REQUIRE_NEAR(rs.metrics.at("alpha_power"), 0.31, 1e-9);
}

BM_CASE(json_input_ignores_non_numeric_metrics) {
    cerebra::reset_current_atlas_to_builtin();
    auto frames = cerebra::parse_frames(R"([{
        "brain_activity":[{
            "region":"amygdala",
            "intensity":0.6,
            "metrics":{"bold":0.5,"label":"left","present":true}
        }],
        "timestamp_ms":0
    }])");
    const auto& rs = frames[0].regions[0];
    BM_REQUIRE_EQ(rs.metrics.size(), std::size_t(1));
    BM_REQUIRE(rs.metrics.count("bold") == 1);
}

BM_CASE(json_input_no_metrics_keeps_map_empty) {
    cerebra::reset_current_atlas_to_builtin();
    auto frames = cerebra::parse_frames(
        R"([{"brain_activity":[{"region":"amygdala","intensity":0.5}],"timestamp_ms":0}])");
    BM_REQUIRE(frames[0].regions[0].metrics.empty());
}

#include "../test_harness.h"

#include "core/atlas_core.h"
#include "core/atlas_region.h"
#include "core/state_manager.h"
#include "visualization/view_2d.h"
#include "ui/interactive_ui.h"
#include "core/simulation_engine.h"
#include "ui/visual_themes.h"

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

BM_CASE(builtin_atlas_has_four_pathways) {
    cerebra::reset_current_atlas_to_builtin();
    const auto& paths = cerebra::current_atlas().pathways();
    BM_REQUIRE_EQ(paths.size(), std::size_t(4));
    BM_REQUIRE(cerebra::current_atlas().find_pathway("fear_circuit") != nullptr);
    BM_REQUIRE(cerebra::current_atlas().find_pathway("papez_circuit") != nullptr);
    BM_REQUIRE(cerebra::current_atlas().find_pathway("salience_network") != nullptr);
    BM_REQUIRE(cerebra::current_atlas().find_pathway("visual_relay") != nullptr);
}

BM_CASE(papez_circuit_is_four_nodes_bidirectional) {
    cerebra::reset_current_atlas_to_builtin();
    const auto* p = cerebra::current_atlas().find_pathway("papez_circuit");
    BM_REQUIRE(p != nullptr);
    BM_REQUIRE_EQ(p->nodes.size(), std::size_t(4));
    BM_REQUIRE_EQ(p->nodes.front(), std::string("entorhinal_cortex"));
    BM_REQUIRE_EQ(p->nodes.back(), std::string("anterior_cingulate_cortex"));
    BM_REQUIRE(p->bidirectional);
}

BM_CASE(add_or_replace_pathway_inserts_then_updates) {
    cerebra::RegionAtlas a;
    cerebra::PathwayDefinition p;
    p.id = "test_path";
    p.name = "Test Path";
    p.nodes = {"a", "b"};
    p.strength = 0.5;
    a.add_or_replace_pathway(p);
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(1));

    p.strength = 0.9;
    a.add_or_replace_pathway(p);
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(1));
    BM_REQUIRE_NEAR(a.find_pathway("test_path")->strength, 0.9, 1e-9);
}

BM_CASE(remove_pathway_returns_false_for_unknown) {
    cerebra::RegionAtlas a;
    BM_REQUIRE(!a.remove_pathway("nope"));
}

BM_CASE(from_json_loads_pathway_with_nodes) {
    const char* src = R"({
        "regions": [
            {"id":"a","display_name":"A"},
            {"id":"b","display_name":"B"},
            {"id":"c","display_name":"C"}
        ],
        "pathways": [
            {"id":"abc","name":"A-B-C","nodes":["a","b","c"],
             "transmitter":"glutamate","strength":0.8,"bidirectional":true}
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(1));
    const auto* p = a.find_pathway("abc");
    BM_REQUIRE(p != nullptr);
    BM_REQUIRE_EQ(p->nodes.size(), std::size_t(3));
    BM_REQUIRE(p->bidirectional);
    BM_REQUIRE_NEAR(p->strength, 0.8, 1e-9);
}

BM_CASE(from_json_loads_pathway_with_from_to_shorthand) {
    const char* src = R"({
        "regions": [
            {"id":"a","display_name":"A"},
            {"id":"b","display_name":"B"}
        ],
        "pathways": [
            {"id":"ab","name":"A->B","from":"a","to":"b","strength":0.5}
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    const auto* p = a.find_pathway("ab");
    BM_REQUIRE(p != nullptr);
    BM_REQUIRE_EQ(p->nodes.size(), std::size_t(2));
    BM_REQUIRE_EQ(p->nodes[0], std::string("a"));
    BM_REQUIRE_EQ(p->nodes[1], std::string("b"));
}

BM_CASE(from_json_extends_builtin_pathways) {
    const char* src = R"({
        "extends_builtin": true,
        "pathways": [
            {"id":"custom","name":"Custom","nodes":["thalamus","insula"],"strength":0.4}
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    // 4 builtin pathways + 1 custom; regions unchanged (10 builtin)
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(5));
    BM_REQUIRE_EQ(a.regions().size(), std::size_t(10));
    BM_REQUIRE(a.find_pathway("custom") != nullptr);
    BM_REQUIRE(a.find_pathway("papez_circuit") != nullptr);
}

BM_CASE(from_json_overrides_existing_pathway) {
    const char* src = R"({
        "extends_builtin": true,
        "pathways": [
            {"id":"fear_circuit","name":"Renamed","nodes":["thalamus","amygdala"],"strength":0.99}
        ]
    })";
    auto a = cerebra::RegionAtlas::from_json(src);
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(4));
    const auto* p = a.find_pathway("fear_circuit");
    BM_REQUIRE_EQ(p->name, std::string("Renamed"));
    BM_REQUIRE_NEAR(p->strength, 0.99, 1e-9);
}

BM_CASE(from_json_rejects_pathway_without_nodes) {
    bool threw = false;
    try {
        cerebra::RegionAtlas::from_json(
            R"({"pathways":[{"id":"x","name":"X"}]})");
    } catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_json_rejects_pathway_with_one_node) {
    bool threw = false;
    try {
        cerebra::RegionAtlas::from_json(
            R"({"pathways":[{"id":"x","name":"X","nodes":["a"]}]})");
    } catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_json_rejects_non_string_node) {
    bool threw = false;
    try {
        cerebra::RegionAtlas::from_json(
            R"({"pathways":[{"id":"x","name":"X","nodes":["a",1]}]})");
    } catch (const cerebra::AtlasError&) { threw = true; }
    BM_REQUIRE(threw);
}

BM_CASE(from_json_accepts_pathways_only_no_regions) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"pathways":[{"id":"x","name":"X","nodes":["a","b"]}]})");
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(1));
    BM_REQUIRE(a.regions().empty());
}

BM_CASE(pathway_activation_is_mean_intensity_times_strength) {
    cerebra::BrainFrame f;
    f.regions.push_back({"a", 0.4, {}, {}});
    f.regions.push_back({"b", 0.8, {}, {}});

    cerebra::PathwayDefinition p;
    p.id = "ab"; p.name = "A-B"; p.nodes = {"a", "b"};
    p.strength = 0.5;
    // mean(0.4, 0.8) * 0.5 = 0.3
    BM_REQUIRE_NEAR(cerebra::pathway_activation(p, f), 0.30, 1e-9);
}

BM_CASE(pathway_activation_skips_missing_nodes) {
    cerebra::BrainFrame f;
    f.regions.push_back({"a", 1.0, {}, {}});

    cerebra::PathwayDefinition p;
    p.id = "ab"; p.name = "A-B"; p.nodes = {"a", "missing"};
    p.strength = 1.0;
    // Only "a" contributes; mean = 1.0; strength = 1.0; activation = 1.0
    BM_REQUIRE_NEAR(cerebra::pathway_activation(p, f), 1.0, 1e-9);
}

BM_CASE(pathway_activation_zero_when_no_nodes_present) {
    cerebra::BrainFrame f;
    cerebra::PathwayDefinition p;
    p.id = "x"; p.name = "X"; p.nodes = {"foo", "bar"};
    p.strength = 0.9;
    BM_REQUIRE_NEAR(cerebra::pathway_activation(p, f), 0.0, 1e-9);
}

BM_CASE(render_pathways_table_contains_all_builtin_pathway_names) {
    cerebra::reset_current_atlas_to_builtin();
    auto f = cerebra::template_frame(cerebra::BrainTemplate::Stressed, 0);
    const auto& theme = cerebra::theme_by_name("mono");
    std::string out = cerebra::render_pathways_table(f, 100, theme);
    BM_REQUIRE(out.find("Fear Circuit") != std::string::npos);
    BM_REQUIRE(out.find("Papez Circuit") != std::string::npos);
    BM_REQUIRE(out.find("Salience Network") != std::string::npos);
    BM_REQUIRE(out.find("Visual Relay") != std::string::npos);
    BM_REQUIRE(out.find("Thalamus") != std::string::npos);
}

BM_CASE(render_pathways_table_uses_arrow_or_double_arrow) {
    cerebra::reset_current_atlas_to_builtin();
    auto f = cerebra::template_frame(cerebra::BrainTemplate::Focused, 0);
    const auto& theme = cerebra::theme_by_name("mono");
    std::string out = cerebra::render_pathways_table(f, 100, theme);
    BM_REQUIRE(out.find(" -> ")  != std::string::npos); // unidirectional pathway
    BM_REQUIRE(out.find(" <-> ") != std::string::npos); // Papez is bidirectional
}

BM_CASE(render_pathways_table_empty_when_no_pathways) {
    cerebra::RegionAtlas a;
    cerebra::RegionDefinition r;
    r.id = "only"; r.display_name = "Only";
    a.add_or_replace(r);
    cerebra::set_current_atlas(std::move(a));

    auto f = cerebra::template_frame(cerebra::BrainTemplate::Focused, 0);
    const auto& theme = cerebra::theme_by_name("mono");
    std::string out = cerebra::render_pathways_table(f, 100, theme);
    cerebra::reset_current_atlas_to_builtin();

    BM_REQUIRE(out.empty());
}

BM_CASE(report_includes_pathway_section) {
    cerebra::reset_current_atlas_to_builtin();
    std::vector<cerebra::BrainFrame> frames = {
        cerebra::template_frame(cerebra::BrainTemplate::Stressed, 0)};
    cerebra::Simulation sim(std::move(frames));
    std::ostringstream out;
    cerebra::run_report(sim, "mono", out);
    BM_REQUIRE(out.str().find("Fear Circuit")     != std::string::npos);
    BM_REQUIRE(out.str().find("Salience Network") != std::string::npos);
}

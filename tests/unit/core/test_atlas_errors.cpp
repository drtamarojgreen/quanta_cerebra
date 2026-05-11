// Targeted error-path coverage for src/atlas.cpp. The happy paths and many
// errors are already in test_atlas.cpp and test_pathways.cpp; this file
// systematically walks every uncovered throw and every uncommon escape branch
// in the embedded MiniJson parser.

#include "test_harness.hpp"

#include "core/atlas_core.h"

#include <stdexcept>
#include <string>

static bool throws_atlas(const std::string& src) {
    try { cerebra::RegionAtlas::from_json(src); return false; }
    catch (const cerebra::AtlasError&) { return true; }
    catch (...) { return false; }
}

// ---------- MiniJson value / structural errors ----------

BM_CASE(atlas_minijson_rejects_unexpected_char_at_top_level) {
    BM_REQUIRE(throws_atlas("@"));
}

BM_CASE(atlas_minijson_rejects_unexpected_char_in_array) {
    BM_REQUIRE(throws_atlas(R"({"regions": [@]})"));
}

BM_CASE(atlas_minijson_rejects_missing_comma_in_object) {
    BM_REQUIRE(throws_atlas(R"({"regions":[{"id":"x" "display_name":"X"}]})"));
}

BM_CASE(atlas_minijson_rejects_missing_comma_in_array) {
    BM_REQUIRE(throws_atlas(R"({"regions":[1 2]})"));
}

BM_CASE(atlas_minijson_rejects_trailing_content) {
    BM_REQUIRE(throws_atlas(R"({"regions":[]} extra)"));
}

// ---------- MiniJson string-escape branches ----------
// These ARE accepted by the parser; testing them executes the case statements
// in MiniJson::str() that no happy-path atlas JSON triggers.

BM_CASE(atlas_minijson_accepts_b_escape) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"a\bb"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("a\bb"));
}

BM_CASE(atlas_minijson_accepts_f_escape) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"a\fb"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("a\fb"));
}

BM_CASE(atlas_minijson_accepts_r_escape) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"a\rb"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("a\rb"));
}

BM_CASE(atlas_minijson_accepts_t_escape) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"a\tb"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("a\tb"));
}

BM_CASE(atlas_minijson_accepts_n_escape) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"line1\nline2"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("line1\nline2"));
}

BM_CASE(atlas_minijson_accepts_quote_and_backslash) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"a\"\\b"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("a\"\\b"));
}

BM_CASE(atlas_minijson_accepts_forward_slash_escape) {
    auto a = cerebra::RegionAtlas::from_json(
        R"({"regions":[{"id":"x","display_name":"a\/b"}]})");
    BM_REQUIRE_EQ(a.find("x")->display_name, std::string("a/b"));
}

BM_CASE(atlas_minijson_rejects_unknown_escape) {
    BM_REQUIRE(throws_atlas(R"({"regions":[{"id":"x","display_name":"\q"}]})"));
}

BM_CASE(atlas_minijson_rejects_backslash_at_end_of_input) {
    // String opens but the very next char is a lone backslash with nothing
    // after it: pos_ >= s_.size() inside the escape handler.
    std::string src = "{\"regions\":[{\"id\":\"\\";
    BM_REQUIRE(throws_atlas(src));
}

BM_CASE(atlas_minijson_rejects_unterminated_string) {
    BM_REQUIRE(throws_atlas("{\"regions\":[{\"id\":\"unclosed"));
}

// ---------- MiniJson literal / number errors ----------

BM_CASE(atlas_minijson_rejects_partial_true) {
    BM_REQUIRE(throws_atlas(R"({"extends_builtin": tru, "regions": []})"));
}

BM_CASE(atlas_minijson_rejects_partial_false) {
    BM_REQUIRE(throws_atlas(R"({"extends_builtin": fals, "regions": []})"));
}

BM_CASE(atlas_minijson_rejects_partial_null) {
    BM_REQUIRE(throws_atlas(R"({"key": nul, "regions": []})"));
}

BM_CASE(atlas_minijson_rejects_lone_minus_number) {
    // '-' enters num(), advances pos_, while-loop exits immediately because
    // the next char is not a digit/'.'/'e'/etc. stod("-") then throws, which
    // is caught and reported via fail("bad number").
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"x","display_name":"x","slice":{"row":-,"col":1,"w":1,"h":1}}]})"));
}

// ---------- need_num field-type errors ----------

BM_CASE(atlas_need_num_rejects_string_for_slice_row) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"x","display_name":"X","slice":{"row":"oops","col":1,"w":1,"h":1}}]})"));
}

BM_CASE(atlas_need_num_rejects_missing_slice_col) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"x","display_name":"X","slice":{"row":1,"w":1,"h":1}}]})"));
}

// ---------- from_json structural / type-mismatch errors ----------

BM_CASE(atlas_rejects_top_level_array) {
    BM_REQUIRE(throws_atlas("[]"));
}

BM_CASE(atlas_rejects_top_level_number) {
    BM_REQUIRE(throws_atlas("42"));
}

BM_CASE(atlas_rejects_regions_wrong_type_when_pathways_valid) {
    // The pathways array is valid so the early "at least one of"-check passes;
    // then the regions-specific type check fires.
    BM_REQUIRE(throws_atlas(
        R"({"regions":{},"pathways":[{"id":"x","name":"X","nodes":["a","b"]}]})"));
}

BM_CASE(atlas_rejects_non_object_region_entry) {
    BM_REQUIRE(throws_atlas(R"({"regions":[42]})"));
}

BM_CASE(atlas_rejects_slice_wrong_type) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"x","display_name":"X","slice":42}]})"));
}

BM_CASE(atlas_rejects_projection_wrong_type) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"x","display_name":"X","projection":"nope"}]})"));
}

BM_CASE(atlas_rejects_flows_wrong_type) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"x","display_name":"X","flows":42}]})"));
}

BM_CASE(atlas_rejects_pathways_wrong_type_when_regions_valid) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"a","display_name":"A"}],"pathways":{}})"));
}

BM_CASE(atlas_rejects_non_object_pathway_entry) {
    BM_REQUIRE(throws_atlas(R"({"pathways":[42]})"));
}

BM_CASE(atlas_rejects_pathway_nodes_wrong_type) {
    BM_REQUIRE(throws_atlas(
        R"({"pathways":[{"id":"x","name":"X","nodes":"not an array"}]})"));
}

BM_CASE(atlas_rejects_pathway_transmitter_wrong_type) {
    BM_REQUIRE(throws_atlas(
        R"({"pathways":[{"id":"x","name":"X","nodes":["a","b"],"transmitter":42}]})"));
}

BM_CASE(atlas_rejects_pathway_bidirectional_wrong_type) {
    BM_REQUIRE(throws_atlas(
        R"({"pathways":[{"id":"x","name":"X","nodes":["a","b"],"bidirectional":"yes"}]})"));
}

BM_CASE(atlas_rejects_templates_wrong_type_when_regions_valid) {
    BM_REQUIRE(throws_atlas(
        R"({"regions":[{"id":"a","display_name":"A"}],"templates":{}})"));
}

BM_CASE(atlas_rejects_non_object_template_entry) {
    BM_REQUIRE(throws_atlas(R"({"templates":[42]})"));
}

// ---------- Lookup helper success / failure paths ----------

BM_CASE(find_pathway_returns_nullptr_for_unknown_id) {
    cerebra::RegionAtlas a;
    BM_REQUIRE(a.find_pathway("nope") == nullptr);
}

BM_CASE(find_template_returns_nullptr_for_unknown_id) {
    cerebra::RegionAtlas a;
    BM_REQUIRE(a.find_template("nope") == nullptr);
}

BM_CASE(remove_pathway_returns_true_when_present) {
    cerebra::RegionAtlas a;
    cerebra::PathwayDefinition p;
    p.id = "x"; p.name = "X"; p.nodes = {"a", "b"};
    a.add_or_replace_pathway(p);
    BM_REQUIRE(a.remove_pathway("x"));
    BM_REQUIRE_EQ(a.pathways().size(), std::size_t(0));
}

BM_CASE(remove_template_returns_true_when_present) {
    cerebra::RegionAtlas a;
    cerebra::TemplateDefinition t;
    t.id = "x";
    t.intensities = {{"a", 0.5}};
    a.add_or_replace_template(t);
    BM_REQUIRE(a.remove_template("x"));
    BM_REQUIRE_EQ(a.templates().size(), std::size_t(0));
}

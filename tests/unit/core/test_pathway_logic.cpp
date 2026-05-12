#include <cstdio>
#include <fstream>
#include <string>

#include "../test_framework.hpp"
#include "../../test_config.h"
#include "io/json_parser.h"
#include "core/pathway_logic.h"
#include "core/region.hpp"
#include "visualization/scene_renderer.h"
#include "core/sample.hpp"
#include "core/simulation_engine.h"

using namespace cerebra;

namespace {
// Restores both catalogs to their defaults on scope exit.
struct CatalogGuard {
  ~CatalogGuard() {
    PathwayCatalog::reset_to_defaults();
    RegionCatalog::reset_to_defaults();
  }
};
JsonValue json(const std::string& text) { return JsonValue::parse(text); }
const Pathway* find_pathway(const std::string& from, const std::string& to) {
  for (const auto& p : PathwayCatalog::all())
    if (p.from == from && p.to == to) return &p;
  return nullptr;
}
}  // namespace

TEST_CASE("pathway kind parsing accepts names and short forms") {
  PathwayKind k;
  CHECK(parse_pathway_kind("excitatory", k) && k == PathwayKind::Excitatory);
  CHECK(parse_pathway_kind("EXC", k) && k == PathwayKind::Excitatory);
  CHECK(parse_pathway_kind("inhibitory", k) && k == PathwayKind::Inhibitory);
  CHECK(parse_pathway_kind("-", k) && k == PathwayKind::Inhibitory);
  CHECK(parse_pathway_kind("Modulatory", k) && k == PathwayKind::Modulatory);
  CHECK(parse_pathway_kind("~", k) && k == PathwayKind::Modulatory);
  CHECK(!parse_pathway_kind("wobbly", k));
  CHECK_EQ(std::string(to_string(PathwayKind::Inhibitory)), std::string("inhibitory"));
}

TEST_CASE("pathway catalog is empty by default and loads from JSON") {
  CatalogGuard guard;
  CHECK(PathwayCatalog::all().empty());
  CHECK(!PathwayCatalog::using_custom_catalog());

  PathwayCatalog::load_from_json(json(R"([
    { "from": "thalamus", "to": "occipital_lobe", "weight": 0.8, "delay_ms": 12,
      "kind": "excitatory", "transmitter": "glutamate",
      "metadata": { "tract": "optic radiation" } },
    { "from": "PFC", "to": "amygdala", "directed": true, "weight": 0.5,
      "kind": "inhibitory", "transmitter": "gaba" },
    { "from": "hippocampus", "to": "entorhinal_cortex", "directed": false, "weight": 0.9 }
  ])"));

  CHECK(PathwayCatalog::using_custom_catalog());
  CHECK_EQ(PathwayCatalog::all().size(), static_cast<std::size_t>(3));

  // Endpoints are canonicalised (abbreviation "PFC" -> "prefrontal_cortex").
  const Pathway* p = find_pathway("prefrontal_cortex", "amygdala");
  CHECK(p != nullptr);
  CHECK(p->directed);
  CHECK_NEAR(p->weight, 0.5, 1e-9);
  CHECK(p->kind == PathwayKind::Inhibitory);
  CHECK_EQ(p->transmitter, std::string("gaba"));
  CHECK_EQ(p->label(), std::string("prefrontal_cortex -> amygdala"));

  const Pathway* t = find_pathway("thalamus", "occipital_lobe");
  CHECK(t != nullptr);
  CHECK_EQ(t->delay_ms, static_cast<std::int64_t>(12));
  CHECK_EQ(t->extra.at("tract"), std::string("optic radiation"));

  // Defaults: undirected, weight 0.5 / kind excitatory when absent.
  const Pathway* h = find_pathway("hippocampus", "entorhinal_cortex");
  CHECK(h != nullptr);
  CHECK(!h->directed);
  CHECK(h->kind == PathwayKind::Excitatory);
  CHECK_EQ(h->label(), std::string("hippocampus -- entorhinal_cortex"));
}

TEST_CASE("pathway lookups distinguish afferent, efferent and incident") {
  CatalogGuard guard;
  PathwayCatalog::load_from_json(json(R"([
    { "from": "thalamus", "to": "amygdala", "directed": true },
    { "from": "amygdala", "to": "prefrontal_cortex", "directed": true },
    { "from": "amygdala", "to": "insula", "directed": false }
  ])"));

  CHECK_EQ(PathwayCatalog::incident_to("amygdala").size(), static_cast<std::size_t>(3));
  // Afferent: thalamus->amygdala, plus the undirected amygdala--insula.
  CHECK_EQ(PathwayCatalog::afferent("amygdala").size(), static_cast<std::size_t>(2));
  // Efferent: amygdala->prefrontal_cortex, plus the undirected amygdala--insula.
  CHECK_EQ(PathwayCatalog::efferent("amygdala").size(), static_cast<std::size_t>(2));
  // The thalamus only has an outgoing edge.
  CHECK_EQ(PathwayCatalog::afferent("thalamus").size(), static_cast<std::size_t>(0));
  CHECK_EQ(PathwayCatalog::efferent("thalamus").size(), static_cast<std::size_t>(1));
  // Lookups accept aliases/abbreviations.
  CHECK_EQ(PathwayCatalog::incident_to("AMY").size(), static_cast<std::size_t>(3));
}

TEST_CASE("pathway catalog validation rejects malformed or invalid documents") {
  CatalogGuard guard;
  CHECK_THROWS(PathwayCatalog::load_from_json(json("{}")));               // not an array
  CHECK_THROWS(PathwayCatalog::load_from_json(json("[1]")));              // entry not an object
  CHECK_THROWS(PathwayCatalog::load_from_json(json("[{\"to\":\"amygdala\"}]")));   // missing from
  CHECK_THROWS(PathwayCatalog::load_from_json(json("[{\"from\":\"amygdala\"}]"))); // missing to
  CHECK_THROWS(PathwayCatalog::load_from_json(json(
      "[{\"from\":\"amygdala\",\"to\":\"nowhere_land\"}]")));             // unknown region
  CHECK_THROWS(PathwayCatalog::load_from_json(json(
      "[{\"from\":\"amygdala\",\"to\":\"thalamus\",\"kind\":\"sideways\"}]")));   // unknown kind
  CHECK_THROWS(PathwayCatalog::load_from_json(json(
      "[{\"from\":\"amygdala\",\"to\":\"thalamus\",\"transmitter\":\"phlogiston\"}]")));  // bad nt

  // A failed load leaves the previous catalog intact.
  PathwayCatalog::load_from_json(json("[{\"from\":\"amygdala\",\"to\":\"thalamus\"}]"));
  CHECK_EQ(PathwayCatalog::all().size(), static_cast<std::size_t>(1));
  CHECK_THROWS(PathwayCatalog::load_from_json(json("[1]")));
  CHECK_EQ(PathwayCatalog::all().size(), static_cast<std::size_t>(1));

  // An empty array is allowed (zero pathways) and counts as a custom catalog.
  PathwayCatalog::load_from_json(json("[]"));
  CHECK(PathwayCatalog::all().empty());
  CHECK(PathwayCatalog::using_custom_catalog());
}

TEST_CASE("pathway catalog load_from_file handles missing, malformed and valid files") {
  CatalogGuard guard;
  CHECK_THROWS(PathwayCatalog::load_from_file("/no/such/pathways-xyz.json"));

  std::string path = cerebra::test::temp_path("bm_test_pathways.json");
  { std::ofstream out(path, std::ios::trunc); out << "not json"; }
  CHECK_THROWS(PathwayCatalog::load_from_file(path));

  { std::ofstream out(path, std::ios::trunc);
    out << R"([{ "from": "amygdala", "to": "insula", "weight": 0.7, "kind": "excitatory" }])"; }
  PathwayCatalog::load_from_file(path);
  CHECK_EQ(PathwayCatalog::all().size(), static_cast<std::size_t>(1));
  CHECK(find_pathway("amygdala", "insula") != nullptr);
  std::remove(path.c_str());

  PathwayCatalog::reset_to_defaults();
  CHECK(PathwayCatalog::all().empty());
  CHECK(!PathwayCatalog::using_custom_catalog());
}

TEST_CASE("loaded pathways surface in the report and the brain view") {
  CatalogGuard guard;
  PathwayCatalog::load_from_json(json(R"([
    { "from": "amygdala", "to": "prefrontal_cortex", "kind": "modulatory", "weight": 0.7,
      "delay_ms": 15, "transmitter": "norepinephrine", "metadata": { "tract": "uncinate" } },
    { "from": "thalamus", "to": "occipital_lobe", "kind": "excitatory", "weight": 0.85 }
  ])"));

  auto tl = ActivityTimeline::from_json_text(
      "[{\"brain_activity\":[{\"region\":\"amygdala\",\"intensity\":0.7}],\"timestamp_ms\":0}]");
  Simulation sim(std::move(tl));
  sim.jump_to_end();

  std::string r = Renderer::render_report(sim, 90, ViewMode::Slice2D, true);
  CHECK(r.find("Pathways") != std::string::npos);
  CHECK(r.find("Amygdala -> Prefrontal Cortex") != std::string::npos);
  CHECK(r.find("modulatory") != std::string::npos);
  CHECK(r.find("tract=uncinate") != std::string::npos);

  // The slice can draw the overlay without crashing, and the region panel lists
  // the connections of the selected region.
  RenderOptions opt;
  opt.width = 100;
  opt.height = 28;
  opt.ascii_only = true;
  opt.show_pathways = true;
  Renderer rdr(opt);
  CHECK_EQ(rdr.render_brain_view(sim, 60, 20).size(), static_cast<std::size_t>(20));
  opt.view = ViewMode::Projection3D;
  Renderer rdr3(opt);
  CHECK_EQ(rdr3.render_brain_view(sim, 60, 20).size(), static_cast<std::size_t>(20));

  sim.select_region("amygdala");
  Renderer rframe(opt);
  auto frame = rframe.render_frame(sim);
  bool mentions_conn = false;
  for (const auto& l : frame)
    if (l.find("connections:") != std::string::npos) mentions_conn = true;
  CHECK(mentions_conn);
}

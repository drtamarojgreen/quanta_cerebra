#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "../test_framework.hpp"
#include "../../test_config.h"
#include "io/json_parser.h"
#include "core/region.hpp"
#include "visualization/scene_renderer.h"
#include "core/sample.hpp"
#include "core/simulation_engine.h"

using namespace cerebra;

namespace {
// Restores the built-in catalog when a test scope exits, even on failure.
struct CatalogGuard {
  ~CatalogGuard() { RegionCatalog::reset_to_defaults(); }
};

JsonValue json(const std::string& text) { return JsonValue::parse(text); }

const RegionInfo* find_in(const std::vector<RegionInfo>& v, const std::string& key) {
  for (const auto& r : v)
    if (r.key == key) return &r;
  return nullptr;
}
}  // namespace

TEST_CASE("region config: a custom catalog replaces the built-in one") {
  CatalogGuard guard;
  CHECK(!RegionCatalog::using_custom_catalog());
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(10));

  RegionCatalog::load_from_json(json(R"([
    { "key": "amygdala", "display_name": "Amygdala", "abbreviation": "AMY",
      "slice_x": 0.4, "slice_y": 0.6, "depth": 0.5,
      "primary_transmitter": "norepinephrine", "region_of_interest": true,
      "metadata": { "system": "limbic", "weight": 1.5 } },
    { "key": "synthetic nucleus", "primary_transmitter": "dopamine",
      "region_of_interest": false },
    { "key": "raphe_nuclei", "display_name": "Raphe Nuclei",
      "slice_y": 0.7, "primary_transmitter": "serotonin", "region_of_interest": true }
  ])"));

  CHECK(RegionCatalog::using_custom_catalog());
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(3));

  // Keys are slugified; missing fields get sensible defaults.
  auto syn = RegionCatalog::find("synthetic nucleus");
  CHECK(syn.has_value());
  CHECK_EQ(syn->key, std::string("synthetic_nucleus"));
  CHECK_EQ(syn->display_name, std::string("Synthetic Nucleus"));  // derived from key
  CHECK_EQ(syn->abbreviation, std::string("SYN"));                // first 3 alnum chars
  CHECK(!syn->region_of_interest);
  CHECK_NEAR(syn->slice_x, 0.5, 1e-9);                            // default

  // Metadata is parsed (numbers stringified).
  auto amy = RegionCatalog::find("amygdala");
  CHECK(amy.has_value());
  CHECK_EQ(amy->extra.at("system"), std::string("limbic"));
  CHECK_EQ(amy->extra.at("weight"), std::string("1.5"));
  CHECK(amy->region_of_interest);

  // ROI helpers reflect the custom catalog.
  CHECK(RegionCatalog::is_region_of_interest("raphe_nuclei"));
  CHECK(!RegionCatalog::is_region_of_interest("synthetic_nucleus"));
  CHECK_EQ(RegionCatalog::regions_of_interest().size(), static_cast<std::size_t>(2));

  // A built-in region that is no longer present does not resolve...
  CHECK(!RegionCatalog::find("hippocampus").has_value());
  // ...and an alias whose target is absent stays unresolved.
  CHECK(!RegionCatalog::find("acc").has_value());  // anterior_cingulate_cortex not in this catalog

  // Unknown names still slugify (so brain-activity input never breaks).
  CHECK_EQ(RegionCatalog::normalize_key("Some Other Area"), std::string("some_other_area"));

  // reset restores the defaults.
  RegionCatalog::reset_to_defaults();
  CHECK(!RegionCatalog::using_custom_catalog());
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(10));
  CHECK(RegionCatalog::find("hippocampus").has_value());
}

TEST_CASE("region config: validation rejects malformed or invalid documents") {
  CatalogGuard guard;
  CHECK_THROWS(RegionCatalog::load_from_json(json("{}")));                // not an array
  CHECK_THROWS(RegionCatalog::load_from_json(json("[]")));                // empty
  CHECK_THROWS(RegionCatalog::load_from_json(json("[1, 2]")));            // entries not objects
  CHECK_THROWS(RegionCatalog::load_from_json(json("[{}]")));              // missing key
  CHECK_THROWS(RegionCatalog::load_from_json(json("[{\"key\":\"  \"}]")));  // blank key after slugify
  CHECK_THROWS(RegionCatalog::load_from_json(json(
      "[{\"key\":\"a\"},{\"key\":\"a\"}]")));                            // duplicate key
  CHECK_THROWS(RegionCatalog::load_from_json(json(
      "[{\"key\":\"x\",\"primary_transmitter\":\"unobtanium\"}]")));      // unknown transmitter

  // A failed load leaves the previous catalog intact.
  CHECK(!RegionCatalog::using_custom_catalog());
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(10));
  RegionCatalog::load_from_json(json("[{\"key\":\"only_one\",\"primary_transmitter\":\"gaba\"}]"));
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(1));
  CHECK_THROWS(RegionCatalog::load_from_json(json("[]")));  // bad reload...
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(1));  // ...keeps the good one
}

TEST_CASE("region config: load_from_file handles missing, malformed and valid files") {
  CatalogGuard guard;
  CHECK_THROWS(RegionCatalog::load_from_file("/no/such/regions-xyz.json"));

  std::string path = cerebra::test::temp_path("bm_test_regions.json");
  {
    std::ofstream out(path, std::ios::trunc);
    out << "this is not json";
  }
  CHECK_THROWS(RegionCatalog::load_from_file(path));

  {
    std::ofstream out(path, std::ios::trunc);
    out << R"([{ "key": "test_area", "abbreviation": "TST",
                 "primary_transmitter": "glutamate", "region_of_interest": true,
                 "metadata": { "note": "hello" } }])";
  }
  RegionCatalog::load_from_file(path);
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(1));
  auto t = RegionCatalog::find("test_area");
  CHECK(t.has_value());
  CHECK_EQ(t->abbreviation, std::string("TST"));
  CHECK_EQ(t->extra.at("note"), std::string("hello"));
  std::remove(path.c_str());
}

TEST_CASE("region config: the loaded catalog flows through the renderer and report") {
  CatalogGuard guard;
  RegionCatalog::load_from_json(json(R"([
    { "key": "amygdala", "display_name": "Amygdala", "abbreviation": "AMY",
      "slice_x": 0.4, "slice_y": 0.6, "primary_transmitter": "norepinephrine",
      "region_of_interest": true, "metadata": { "function": "threat detection" } },
    { "key": "thalamus", "display_name": "Thalamus", "abbreviation": "THA",
      "slice_x": 0.5, "slice_y": 0.5, "primary_transmitter": "glutamate",
      "region_of_interest": true },
    { "key": "neocortex_demo", "display_name": "Neocortex Demo", "abbreviation": "NCX",
      "slice_x": 0.5, "slice_y": 0.2, "primary_transmitter": "dopamine",
      "region_of_interest": false }
  ])"));

  // Build a timeline that mentions one custom region by key.
  auto tl = ActivityTimeline::from_json_text(
      "[{\"brain_activity\":[{\"region\":\"amygdala\",\"intensity\":0.8},"
      "{\"region\":\"thalamus\",\"intensity\":0.4}],\"timestamp_ms\":0}]");
  Simulation sim(std::move(tl));
  sim.jump_to_end();

  std::string report = Renderer::render_report(sim, 80, ViewMode::Slice2D, true);
  CHECK(report.find("Amygdala") != std::string::npos);
  CHECK(report.find("Thalamus") != std::string::npos);
  CHECK(report.find("Neocortex Demo") != std::string::npos);
  // Built-in regions are gone.
  CHECK(report.find("Hippocampus") == std::string::npos);
  CHECK(report.find("Occipital Lobe") == std::string::npos);
  // The metadata section is emitted because a region carries metadata.
  CHECK(report.find("Region metadata") != std::string::npos);
  CHECK(report.find("function=threat detection") != std::string::npos);

  // The interactive frame uses the same catalog (region labels / hit testing).
  RenderOptions opt;
  opt.width = 90;
  opt.height = 26;
  opt.ascii_only = true;
  Renderer r(opt);
  sim.select_region("neocortex_demo");
  auto frame = r.render_frame(sim);
  CHECK(!frame.empty());
  // Region cycling now only walks the three configured regions.
  sim.clear_selection();
  sim.select_next_region();
  CHECK(find_in(RegionCatalog::all(), *sim.selected_region()) != nullptr);
  for (int i = 0; i < 3; ++i) sim.select_next_region();  // full loop + 1
  CHECK(sim.selected_region().has_value());
}

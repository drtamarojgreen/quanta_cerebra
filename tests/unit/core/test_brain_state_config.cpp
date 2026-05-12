#include <cstdio>
#include <fstream>
#include <string>

#include "../test_framework.hpp"
#include "../../test_config.h"
#include "core/state_manager.h"
#include "io/json_parser.h"
#include "core/region.hpp"

using namespace cerebra;

namespace {
struct CatalogGuard {
  ~CatalogGuard() { BrainStateLibrary::reset_to_defaults(); }
};
JsonValue json(const std::string& t) { return JsonValue::parse(t); }
}  // namespace

TEST_CASE("brain-state library: built-in by default, replaceable from JSON") {
  CatalogGuard guard;
  CHECK(!BrainStateLibrary::using_custom_catalog());
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(4));
  CHECK(BrainStateLibrary::find("focused") != nullptr);

  BrainStateLibrary::load_from_json(json(R"([
    { "key": "Deep Meditation", "description": "very calm",
      "region_intensities": { "prefrontal_cortex": 0.3, "Amygdala": 1.4, "cuneus": -0.2 },
      "metadata": { "tag": "contemplative" } },
    { "key": "seizure_demo",
      "region_intensities": [ {"region":"thalamus","intensity":0.95},
                              {"region":"occipital_lobe","intensity":0.9} ] }
  ])"));

  CHECK(BrainStateLibrary::using_custom_catalog());
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(2));
  // Built-in presets are gone.
  CHECK(BrainStateLibrary::find("focused") == nullptr);

  const BrainStateTemplate* dm = BrainStateLibrary::find("deep meditation");  // key slugified
  CHECK(dm != nullptr);
  CHECK_EQ(dm->key, std::string("deep_meditation"));
  CHECK_EQ(dm->display_name, std::string("Deep Meditation"));  // derived from key
  CHECK_EQ(dm->description, std::string("very calm"));
  CHECK_NEAR(dm->region_intensities.at("prefrontal_cortex"), 0.3, 1e-9);
  CHECK_NEAR(dm->region_intensities.at("amygdala"), 1.0, 1e-9);  // key normalized, clamped
  CHECK_NEAR(dm->region_intensities.at("cuneus"), 0.0, 1e-9);    // clamped
  CHECK_EQ(dm->extra.at("tag"), std::string("contemplative"));

  // The array form of region_intensities is also accepted.
  const BrainStateTemplate* sz = BrainStateLibrary::find("seizure_demo");
  CHECK(sz != nullptr);
  CHECK_NEAR(sz->region_intensities.at("thalamus"), 0.95, 1e-9);
  CHECK_NEAR(sz->region_intensities.at("occipital_lobe"), 0.9, 1e-9);

  // keys() reflects the loaded library; synthesize_timeline still works.
  auto k = BrainStateLibrary::keys();
  CHECK_EQ(k.size(), static_cast<std::size_t>(2));
  auto tl = BrainStateLibrary::synthesize_timeline(*dm, 20, 50);
  CHECK_EQ(tl.size(), static_cast<std::size_t>(20));
  for (const auto& s : tl.samples())
    for (const auto& kv : s.intensities) { CHECK(kv.second >= 0.0); CHECK(kv.second <= 1.0); }
  // Determinism is preserved.
  auto tl2 = BrainStateLibrary::synthesize_timeline(*dm, 20, 50);
  CHECK_NEAR(tl.at(11).intensity_of("amygdala"), tl2.at(11).intensity_of("amygdala"), 1e-12);

  BrainStateLibrary::reset_to_defaults();
  CHECK(!BrainStateLibrary::using_custom_catalog());
  CHECK(BrainStateLibrary::find("focused") != nullptr);
}

TEST_CASE("brain-state library: validation rejects malformed documents") {
  CatalogGuard guard;
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("{}")));            // not an array
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("[]")));            // empty
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("[1]")));           // entry not an object
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("[{}]")));          // missing key
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("[{\"key\":\"  \"}]")));            // blank key
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("[{\"key\":\"a\"},{\"key\":\"a\"}]")));  // dup
  CHECK_THROWS(BrainStateLibrary::load_from_json(json(
      "[{\"key\":\"x\",\"region_intensities\":7}]")));                   // wrong type for intensities
  CHECK_THROWS(BrainStateLibrary::load_from_json(json(
      "[{\"key\":\"x\",\"region_intensities\":{\"amygdala\":\"high\"}}]")));  // non-numeric value

  // A failed load leaves the previous library intact.
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(4));
  BrainStateLibrary::load_from_json(json("[{\"key\":\"only_state\"}]"));  // no intensities is OK
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(1));
  CHECK(BrainStateLibrary::find("only_state")->region_intensities.empty());
  CHECK_THROWS(BrainStateLibrary::load_from_json(json("[]")));
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(1));
}

TEST_CASE("brain-state library: load_from_file handles missing, malformed and valid files") {
  CatalogGuard guard;
  CHECK_THROWS(BrainStateLibrary::load_from_file("/no/such/states-xyz.json"));

  std::string path = cerebra::test::temp_path("bm_test_states.json");
  { std::ofstream out(path, std::ios::trunc); out << "definitely not json"; }
  CHECK_THROWS(BrainStateLibrary::load_from_file(path));

  { std::ofstream out(path, std::ios::trunc);
    out << R"([{ "key": "lab_default", "display_name": "Lab Default",
                 "region_intensities": { "amygdala": 0.5, "thalamus": 0.5 } }])"; }
  BrainStateLibrary::load_from_file(path);
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(1));
  const BrainStateTemplate* d = BrainStateLibrary::find("lab_default");
  CHECK(d != nullptr);
  CHECK_EQ(d->display_name, std::string("Lab Default"));
  CHECK_NEAR(d->region_intensities.at("amygdala"), 0.5, 1e-9);
  std::remove(path.c_str());

  BrainStateLibrary::reset_to_defaults();
  CHECK_EQ(BrainStateLibrary::all().size(), static_cast<std::size_t>(4));
}

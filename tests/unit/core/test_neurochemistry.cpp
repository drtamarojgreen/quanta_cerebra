#include <cstdio>
#include <fstream>
#include <string>

#include "../test_framework.hpp"
#include "io/json_utility.h"
#include "core/neurochemistry.h"
#include "core/pathway_logic.h"
#include "core/region.hpp"

using namespace cerebra;

namespace {
struct CatalogGuard {
  ~CatalogGuard() {
    PathwayCatalog::reset_to_defaults();
    RegionCatalog::reset_to_defaults();
    Neurochemistry::reset_to_defaults();
  }
};
JsonValue json(const std::string& t) { return JsonValue::parse(t); }
}  // namespace

TEST_CASE("neurotransmitter catalog: built-in by default, replaceable from JSON") {
  CatalogGuard guard;
  CHECK(!Neurochemistry::using_custom_catalog());
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(6));

  Neurochemistry::load_from_json(json(R"([
    { "key": "Dopamine", "symbol": "DA", "baseline": 0.3, "release_gain": 0.9,
      "reuptake_rate": 0.2, "metadata": { "class": "monoamine" } },
    { "key": "octopamine", "baseline": 1.5 },
    { "key": "glutamate", "primary": "ignored", "baseline": 0.4 }
  ])"));
  CHECK(Neurochemistry::using_custom_catalog());
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(3));

  // Keys are slugified; defaults fill in the rest; values are clamped.
  const NeurotransmitterInfo* da = Neurochemistry::find("dopamine");
  CHECK(da != nullptr);
  CHECK_EQ(da->display_name, std::string("Dopamine"));   // present
  CHECK_NEAR(da->baseline, 0.3, 1e-9);
  CHECK_EQ(da->extra.at("class"), std::string("monoamine"));

  const NeurotransmitterInfo* oct = Neurochemistry::find("octopamine");
  CHECK(oct != nullptr);
  CHECK_EQ(oct->display_name, std::string("Octopamine"));  // derived from key
  CHECK_EQ(oct->symbol, std::string("OCTO"));              // first 4 alnum chars
  CHECK_NEAR(oct->baseline, 1.0, 1e-9);                    // clamped from 1.5
  CHECK_NEAR(oct->release_gain, 0.8, 1e-9);                // default

  // baseline_state() reflects the loaded catalog.
  auto base = Neurochemistry::baseline_state();
  CHECK_EQ(base.size(), static_cast<std::size_t>(3));
  CHECK_NEAR(base.at("dopamine"), 0.3, 1e-9);
  CHECK(base.find("serotonin") == base.end());  // not in this catalog

  // step() still works against the custom catalog.
  std::map<std::string, double> active{{"prefrontal_cortex", 1.0}};  // built-in region -> dopamine
  auto s = Neurochemistry::baseline_state();
  for (int i = 0; i < 6; ++i) s = Neurochemistry::step(s, active);
  CHECK(s.at("dopamine") > base.at("dopamine") + 0.05);

  Neurochemistry::reset_to_defaults();
  CHECK(!Neurochemistry::using_custom_catalog());
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(6));
  CHECK(Neurochemistry::find("serotonin") != nullptr);
}

TEST_CASE("neurotransmitter catalog: validation rejects malformed documents") {
  CatalogGuard guard;
  CHECK_THROWS(Neurochemistry::load_from_json(json("{}")));            // not an array
  CHECK_THROWS(Neurochemistry::load_from_json(json("[]")));            // empty
  CHECK_THROWS(Neurochemistry::load_from_json(json("[1, 2]")));        // entries not objects
  CHECK_THROWS(Neurochemistry::load_from_json(json("[{}]")));          // missing key
  CHECK_THROWS(Neurochemistry::load_from_json(json("[{\"key\":\"  \"}]")));            // blank key
  CHECK_THROWS(Neurochemistry::load_from_json(json("[{\"key\":\"a\"},{\"key\":\"a\"}]")));  // dup

  // A failed load leaves the previous catalog intact.
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(6));
  Neurochemistry::load_from_json(json("[{\"key\":\"only_nt\"}]"));
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(1));
  CHECK_THROWS(Neurochemistry::load_from_json(json("[]")));
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(1));
}

TEST_CASE("neurotransmitter catalog: load_from_file and validation interplay with regions") {
  CatalogGuard guard;
  CHECK_THROWS(Neurochemistry::load_from_file("/no/such/nt-xyz.json"));

  std::string path = std::string(P_tmpdir ? P_tmpdir : "/tmp") + "/bm_test_nt.json";
  { std::ofstream out(path, std::ios::trunc); out << "not json"; }
  CHECK_THROWS(Neurochemistry::load_from_file(path));

  { std::ofstream out(path, std::ios::trunc);
    out << R"([{ "key": "histamine", "symbol": "HA", "baseline": 0.2 },
               { "key": "glutamate", "baseline": 0.35 }])"; }
  Neurochemistry::load_from_file(path);
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(2));
  std::remove(path.c_str());

  // A region referencing a transmitter only in the custom catalog now validates,
  // and one referencing a built-in transmitter that was dropped does not.
  CHECK_NOTHROW(RegionCatalog::load_from_json(json(
      "[{\"key\":\"tuberomammillary\",\"primary_transmitter\":\"histamine\"}]")));
  CHECK_THROWS(RegionCatalog::load_from_json(json(
      "[{\"key\":\"raphe\",\"primary_transmitter\":\"serotonin\"}]")));  // not in custom NT catalog

  // Likewise pathways validate transmitters against the active catalog.
  RegionCatalog::reset_to_defaults();  // restore the built-in regions for endpoints
  CHECK_NOTHROW(PathwayCatalog::load_from_json(json(
      "[{\"from\":\"amygdala\",\"to\":\"insula\",\"transmitter\":\"histamine\"}]")));
  CHECK_THROWS(PathwayCatalog::load_from_json(json(
      "[{\"from\":\"amygdala\",\"to\":\"insula\",\"transmitter\":\"dopamine\"}]")));  // dropped
}

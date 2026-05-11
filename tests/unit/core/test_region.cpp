#include "../test_framework.hpp"
#include "core/neurochemistry.h"
#include "core/region.hpp"

using namespace cerebra;

TEST_CASE("catalog contains all required regions of interest") {
  // The project spec calls out these eight regions of interest.
  const char* roi[] = {"amygdala", "cuneus", "anterior_cingulate_cortex", "insula",
                       "gyrus", "hippocampus", "thalamus", "entorhinal_cortex"};
  for (const char* key : roi) {
    auto info = RegionCatalog::find(key);
    CHECK_MSG(info.has_value(), std::string("missing region ") + key);
    CHECK_MSG(info->region_of_interest, std::string("region not flagged ROI: ") + key);
    CHECK(RegionCatalog::is_region_of_interest(key));
  }
  CHECK_EQ(RegionCatalog::regions_of_interest().size(), static_cast<std::size_t>(8));
}

TEST_CASE("region lookup tolerates aliases and formatting") {
  CHECK_EQ(RegionCatalog::normalize_key("Prefrontal Cortex"), std::string("prefrontal_cortex"));
  CHECK_EQ(RegionCatalog::normalize_key("anterior-cingulate-cortex"),
           std::string("anterior_cingulate_cortex"));
  CHECK(RegionCatalog::find("ACC").has_value());
  CHECK_EQ(RegionCatalog::find("ACC")->key, std::string("anterior_cingulate_cortex"));
  CHECK_EQ(RegionCatalog::normalize_key("unknown blob"), std::string("unknown_blob"));
}

TEST_CASE("every region maps to a known neurotransmitter") {
  for (const auto& r : RegionCatalog::all()) {
    CHECK_MSG(Neurochemistry::find(r.primary_transmitter) != nullptr,
              std::string("region ") + r.key + " -> unknown transmitter " + r.primary_transmitter);
  }
}

TEST_CASE("neurochemistry releases on activity and reuptakes toward baseline") {
  auto base = Neurochemistry::baseline_state();
  // Drive the amygdala (norepinephrine) hard for several steps.
  ChemicalState s = base;
  std::map<std::string, double> active{{"amygdala", 1.0}};
  for (int i = 0; i < 8; ++i) s = Neurochemistry::step(s, active);
  CHECK_MSG(s["norepinephrine"] > base["norepinephrine"] + 0.1,
            "NE should rise under sustained amygdala activity");

  // Now let it relax with no activity; it should fall back toward baseline.
  for (int i = 0; i < 30; ++i) s = Neurochemistry::step(s, {});
  CHECK_NEAR(s["norepinephrine"], base["norepinephrine"], 0.05);
}

TEST_CASE("neurochemistry flows attribute release to the driving region") {
  auto prev = Neurochemistry::baseline_state();
  std::map<std::string, double> active{{"hippocampus", 0.9}};  // acetylcholine
  auto cur = Neurochemistry::step(prev, active);
  auto flows = Neurochemistry::flows(prev, cur, active);
  bool found = false;
  for (const auto& f : flows) {
    if (f.transmitter == "acetylcholine") {
      CHECK(f.magnitude > 0);
      CHECK_EQ(f.source_region, std::string("hippocampus"));
      found = true;
    }
  }
  CHECK_MSG(found, "expected an acetylcholine release flow");
}

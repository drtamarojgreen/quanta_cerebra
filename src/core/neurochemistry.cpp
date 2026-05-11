#include "core/neurochemistry.h"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

#include "io/json_parser.h"
#include "core/region.hpp"
#include "io/config_util.hpp"

namespace cerebra {
namespace {

using config_util::clamp01;

const std::vector<NeurotransmitterInfo>& builtin_catalog() {
  static const std::vector<NeurotransmitterInfo> kTransmitters = {
      {"dopamine",       "Dopamine",       "DA",  0.25, 0.85, 0.22, {}},
      {"serotonin",      "Serotonin",      "5HT", 0.30, 0.55, 0.18, {}},
      {"norepinephrine", "Norepinephrine", "NE",  0.20, 0.95, 0.30, {}},
      {"acetylcholine",  "Acetylcholine",  "ACh", 0.22, 0.75, 0.25, {}},
      {"glutamate",      "Glutamate",      "Glu", 0.35, 0.90, 0.35, {}},
      {"gaba",           "GABA",           "GABA",0.40, 0.40, 0.20, {}},
  };
  return kTransmitters;
}

std::vector<NeurotransmitterInfo>& active_catalog() {
  static std::vector<NeurotransmitterInfo> active = builtin_catalog();
  return active;
}

bool& custom_flag() {
  static bool f = false;
  return f;
}

}  // namespace

const std::vector<NeurotransmitterInfo>& Neurochemistry::catalog() { return active_catalog(); }

const NeurotransmitterInfo* Neurochemistry::find(const std::string& key) {
  for (const auto& nt : catalog()) {
    if (nt.key == key) return &nt;
  }
  return nullptr;
}

ChemicalState Neurochemistry::baseline_state() {
  ChemicalState state;
  for (const auto& nt : catalog()) state[nt.key] = nt.baseline;
  return state;
}

ChemicalState Neurochemistry::step(const ChemicalState& previous,
                                   const std::map<std::string, double>& region_intensities) {
  ChemicalState next = previous.empty() ? baseline_state() : previous;
  // Make sure every transmitter currently in the catalog has an entry.
  for (const auto& nt : catalog()) {
    if (next.find(nt.key) == next.end()) next[nt.key] = nt.baseline;
  }

  // Aggregate release pressure per transmitter from active regions.
  std::map<std::string, double> release;
  for (const auto& kv : region_intensities) {
    auto info = RegionCatalog::find(kv.first);
    if (!info) continue;
    const NeurotransmitterInfo* nt = find(info->primary_transmitter);
    if (!nt) continue;
    release[nt->key] = std::max(release[nt->key], kv.second);
  }

  for (const auto& nt : catalog()) {
    double level = next.count(nt.key) ? next.at(nt.key) : nt.baseline;
    double target_release = release.count(nt.key) ? release.at(nt.key) : 0.0;
    // Release pushes the level up toward release_gain * pressure.
    double release_target = nt.baseline + nt.release_gain * target_release * (1.0 - nt.baseline);
    if (release_target > level) {
      level += 0.5 * (release_target - level);
    }
    // Reuptake always pulls toward baseline.
    level += nt.reuptake_rate * (nt.baseline - level);
    next[nt.key] = clamp01(level);
  }
  return next;
}

std::vector<Neurochemistry::Flow> Neurochemistry::flows(
    const ChemicalState& previous, const ChemicalState& current,
    const std::map<std::string, double>& region_intensities) {
  // Map each transmitter to the strongest region driving it (for arrow source).
  std::map<std::string, std::pair<std::string, double>> dominant_source;
  for (const auto& kv : region_intensities) {
    auto info = RegionCatalog::find(kv.first);
    if (!info) continue;
    auto& slot = dominant_source[info->primary_transmitter];
    if (kv.second > slot.second) slot = {info->key, kv.second};
  }

  std::vector<Flow> out;
  for (const auto& nt : catalog()) {
    double before = previous.count(nt.key) ? previous.at(nt.key) : nt.baseline;
    double after = current.count(nt.key) ? current.at(nt.key) : nt.baseline;
    double delta = after - before;
    if (std::abs(delta) < 1e-4) continue;
    Flow f;
    f.transmitter = nt.key;
    f.magnitude = delta;
    if (delta > 0 && dominant_source.count(nt.key)) {
      f.source_region = dominant_source.at(nt.key).first;
    }
    out.push_back(f);
  }
  std::sort(out.begin(), out.end(), [](const Flow& a, const Flow& b) {
    return std::abs(a.magnitude) > std::abs(b.magnitude);
  });
  return out;
}

void Neurochemistry::load_from_json(const JsonValue& root) {
  if (!root.is_array()) {
    throw std::runtime_error("neurotransmitter catalog must be a JSON array");
  }
  const auto& arr = root.as_array();
  if (arr.empty()) {
    throw std::runtime_error("neurotransmitter catalog must contain at least one transmitter");
  }
  std::vector<NeurotransmitterInfo> parsed;
  std::set<std::string> seen;
  for (const auto& elem : arr) {
    if (!elem.is_object()) {
      throw std::runtime_error("each neurotransmitter entry must be a JSON object");
    }
    NeurotransmitterInfo nt;
    nt.key = config_util::slugify(elem["key"].as_string());
    if (nt.key.empty()) {
      throw std::runtime_error("neurotransmitter entry is missing a non-empty 'key'");
    }
    if (!seen.insert(nt.key).second) {
      throw std::runtime_error("duplicate neurotransmitter key: " + nt.key);
    }
    nt.display_name = elem["display_name"].as_string();
    if (nt.display_name.empty()) nt.display_name = config_util::title_from_key(nt.key);
    nt.symbol = elem["symbol"].as_string();
    if (nt.symbol.empty()) nt.symbol = config_util::short_code(nt.key, 4, "NT");
    nt.baseline = clamp01(elem["baseline"].as_number(0.2));
    nt.release_gain = clamp01(elem["release_gain"].as_number(0.8));
    nt.reuptake_rate = clamp01(elem["reuptake_rate"].as_number(0.25));
    nt.extra = config_util::parse_metadata(elem["metadata"]);
    parsed.push_back(std::move(nt));
  }
  active_catalog() = std::move(parsed);
  custom_flag() = true;
}

void Neurochemistry::load_from_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("cannot open neurotransmitters file: " + path);
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  load_from_json(JsonValue::parse(buffer.str()));
}

void Neurochemistry::reset_to_defaults() {
  active_catalog() = builtin_catalog();
  custom_flag() = false;
}

bool Neurochemistry::using_custom_catalog() { return custom_flag(); }

}  // namespace cerebra

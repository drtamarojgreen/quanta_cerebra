#ifndef BRAIN_MODELER_NEUROTRANSMITTER_HPP
#define BRAIN_MODELER_NEUROTRANSMITTER_HPP

#include <map>
#include <string>
#include <vector>

namespace cerebra {

class JsonValue;  // defined in json_utility.h; referenced only by reference here

struct NeurotransmitterInfo {
  std::string key;
  std::string display_name;
  std::string symbol;       // short tag used in compact displays
  double baseline = 0.2;    // resting concentration in [0,1]
  double release_gain = 0.8;  // how strongly region activity drives release
  double reuptake_rate = 0.25;  // fraction of the gap to baseline removed/step
  std::map<std::string, std::string> extra;  // open metadata for future attributes
};

// A snapshot of every modelled neurotransmitter concentration at one timestep.
using ChemicalState = std::map<std::string, double>;

class Neurochemistry {
public:
  // The active list of modelled neurotransmitters. Built in by default; can be
  // replaced from a JSON file (see data/neurotransmitters.json).
  static const std::vector<NeurotransmitterInfo>& catalog();
  static const NeurotransmitterInfo* find(const std::string& key);

  // The resting state with every transmitter at its baseline.
  static ChemicalState baseline_state();

  // Advance the chemical state by one step given the current region intensities
  // (region key -> intensity in [0,1]). Each region drives release of its
  // primary transmitter; everything relaxes back toward baseline via reuptake.
  static ChemicalState step(const ChemicalState& previous,
                            const std::map<std::string, double>& region_intensities);

  // A directed description of where chemicals are "flowing" this step, useful
  // for the ASCII flow visualization. Magnitude is the change vs. the previous
  // state (positive = release, negative = reuptake).
  struct Flow {
    std::string transmitter;
    std::string source_region;  // empty when it is just reuptake
    double magnitude = 0.0;
  };
  static std::vector<Flow> flows(const ChemicalState& previous,
                                 const ChemicalState& current,
                                 const std::map<std::string, double>& region_intensities);

  // ---- configuration ------------------------------------------------------
  // Replace the active catalog with one parsed from a JSON array. Throws
  // std::runtime_error / JsonParseError on a malformed or invalid document; the
  // previous catalog is left intact on failure.
  static void load_from_json(const JsonValue& root);
  static void load_from_file(const std::string& path);
  static void reset_to_defaults();
  static bool using_custom_catalog();
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_NEUROTRANSMITTER_HPP

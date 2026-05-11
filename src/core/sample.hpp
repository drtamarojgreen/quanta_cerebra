#ifndef BRAIN_MODELER_SAMPLE_HPP
#define BRAIN_MODELER_SAMPLE_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "io/json_utility.h"

namespace cerebra {

// One observation of brain activity at a point in time. Intensities are keyed
// by canonical region key and clamped into [0,1].
struct BrainActivitySample {
  std::int64_t timestamp_ms = 0;
  std::map<std::string, double> intensities;  // region key -> intensity

  double intensity_of(const std::string& region_key) const {
    auto it = intensities.find(region_key);
    return it == intensities.end() ? 0.0 : it->second;
  }
};

// An ordered sequence of samples plus helpers to load/synthesize them.
class ActivityTimeline {
public:
  ActivityTimeline() = default;
  explicit ActivityTimeline(std::vector<BrainActivitySample> samples);

  bool empty() const { return samples_.empty(); }
  std::size_t size() const { return samples_.size(); }
  const std::vector<BrainActivitySample>& samples() const { return samples_; }
  const BrainActivitySample& at(std::size_t index) const { return samples_.at(index); }

  std::int64_t duration_ms() const;

  // Append a sample, keeping the timeline sorted by timestamp.
  void append(BrainActivitySample sample);

  // Parse the project's JSON array-of-frames format. Throws JsonParseError or
  // std::runtime_error on malformed input.
  static ActivityTimeline from_json(const JsonValue& root);
  static ActivityTimeline from_json_text(const std::string& text);
  static ActivityTimeline from_json_file(const std::string& path);

  // Build a single-frame timeline from a region->intensity map.
  static ActivityTimeline from_intensities(const std::map<std::string, double>& intensities,
                                           std::int64_t timestamp_ms = 0);

private:
  std::vector<BrainActivitySample> samples_;
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_SAMPLE_HPP

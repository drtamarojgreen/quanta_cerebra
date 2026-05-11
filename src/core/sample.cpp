#include "core/sample.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "core/region.hpp"

namespace cerebra {
namespace {

double clamp01(double v) { return std::max(0.0, std::min(1.0, v)); }

}  // namespace

ActivityTimeline::ActivityTimeline(std::vector<BrainActivitySample> samples)
    : samples_(std::move(samples)) {
  std::stable_sort(samples_.begin(), samples_.end(),
                   [](const BrainActivitySample& a, const BrainActivitySample& b) {
                     return a.timestamp_ms < b.timestamp_ms;
                   });
}

std::int64_t ActivityTimeline::duration_ms() const {
  if (samples_.empty()) return 0;
  return samples_.back().timestamp_ms - samples_.front().timestamp_ms;
}

void ActivityTimeline::append(BrainActivitySample sample) {
  auto it = std::upper_bound(samples_.begin(), samples_.end(), sample,
                             [](const BrainActivitySample& a, const BrainActivitySample& b) {
                               return a.timestamp_ms < b.timestamp_ms;
                             });
  samples_.insert(it, std::move(sample));
}

ActivityTimeline ActivityTimeline::from_json(const JsonValue& root) {
  if (!root.is_array()) {
    throw std::runtime_error("brain activity JSON must be an array of frames");
  }
  std::vector<BrainActivitySample> samples;
  for (const auto& frame : root.as_array()) {
    if (!frame.is_object()) {
      throw std::runtime_error("each frame must be a JSON object");
    }
    BrainActivitySample sample;
    sample.timestamp_ms = frame["timestamp_ms"].as_int(0);
    const JsonValue& activity = frame["brain_activity"];
    if (!activity.is_array()) {
      throw std::runtime_error("frame is missing a 'brain_activity' array");
    }
    for (const auto& entry : activity.as_array()) {
      if (!entry.is_object()) {
        throw std::runtime_error("brain_activity entries must be objects");
      }
      const std::string& raw_region = entry["region"].as_string();
      if (raw_region.empty()) continue;
      std::string key = RegionCatalog::normalize_key(raw_region);
      double intensity = clamp01(entry["intensity"].as_number(0.0));
      sample.intensities[key] = intensity;
    }
    samples.push_back(std::move(sample));
  }
  return ActivityTimeline(std::move(samples));
}

ActivityTimeline ActivityTimeline::from_json_text(const std::string& text) {
  return from_json(JsonValue::parse(text));
}

ActivityTimeline ActivityTimeline::from_json_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("cannot open input file: " + path);
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return from_json_text(buffer.str());
}

ActivityTimeline ActivityTimeline::from_intensities(
    const std::map<std::string, double>& intensities, std::int64_t timestamp_ms) {
  BrainActivitySample sample;
  sample.timestamp_ms = timestamp_ms;
  for (const auto& kv : intensities) {
    sample.intensities[RegionCatalog::normalize_key(kv.first)] = clamp01(kv.second);
  }
  return ActivityTimeline({sample});
}

}  // namespace cerebra

#include "io/simulated_device.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "core/bm_state_manager.h"
#include "io/json_utility.h"

namespace cerebra {
namespace {

std::string frame_to_json_line(const BrainActivitySample& s, std::int64_t timestamp_ms) {
  std::ostringstream os;
  os << "{\"brain_activity\":[";
  bool first = true;
  for (const auto& kv : s.intensities) {
    if (!first) os << ",";
    first = false;
    os << "{\"region\":\" << kv.first << "\",\"intensity\":" << kv.second << "}";
  }
  os << "],\"timestamp_ms\":" << timestamp_ms << "}\n";
  return os.str();
}

}  // namespace

SimulatedSerialDevice::SimulatedSerialDevice(MemorySerialPort& port,
                                             const std::string& initial_state,
                                             std::int64_t step_ms)
    : port_(port), step_ms_(std::max<std::int64_t>(1, step_ms)) {
  load_preset(initial_state);
}

void SimulatedSerialDevice::load_preset(const std::string& key) {
  const BrainStateTemplate* tmpl = BrainStateLibrary::find(key);
  if (!tmpl) tmpl = &BrainStateLibrary::all().front();
  state_key_ = tmpl->key;
  preset_timeline_ = BrainStateLibrary::synthesize_timeline(*tmpl, 240, step_ms_);
  cursor_ = 0;
}

int SimulatedSerialDevice::process_inbound() {
  std::string outbound = port_.take_sent();
  if (outbound.empty()) return 0;
  int handled = 0;
  std::istringstream lines(outbound);
  std::string line;
  while (std::getline(lines, line)) {
    std::size_t b = line.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) continue;
    std::size_t e = line.find_last_not_of(" \t\r\n");
    std::string trimmed = line.substr(b, e - b + 1);
    if (trimmed.empty() || trimmed[0] != '{') continue;
    try {
      JsonValue v = JsonValue::parse(trimmed);
      if (!v.is_object()) continue;
      if (v["command"].as_string() == "set_state") {
        const std::string& want = v["state"].as_string();
        if (BrainStateLibrary::find(want)) {
          load_preset(want);
          ++handled;
          ++commands_processed_;
        }
      }
    } catch (const std::exception&) {
      // Ignore malformed command lines, like real firmware would.
    }
  }
  return handled;
}

void SimulatedSerialDevice::emit_frames(int count) {
  if (preset_timeline_.empty()) return;
  for (int i = 0; i < std::max(0, count); ++i) {
    const auto& sample = preset_timeline_.at(cursor_ % preset_timeline_.size());
    std::int64_t ts = static_cast<std::int64_t>(frames_emitted_) * step_ms_;
    port_.feed(frame_to_json_line(sample, ts));
    ++cursor_;
    ++frames_emitted_;
  }
}

void SimulatedSerialDevice::tick(std::int64_t elapsed_ms) {
  process_inbound();
  if (elapsed_ms <= 0) return;
  frame_accumulator_ += static_cast<double>(elapsed_ms) / static_cast<double>(step_ms_);
  int whole = static_cast<int>(std::floor(frame_accumulator_));
  if (whole > 0) {
    frame_accumulator_ -= whole;
    emit_frames(whole);
  }
}

}  // namespace cerebra

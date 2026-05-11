#include <cstdio>
#include <fstream>
#include <string>

#include "../test_framework.hpp"
#include "io/input_source.hpp"
#include "io/json_utility.h"
#include "core/sample.hpp"

using namespace cerebra;

namespace {
std::string temp_dir() { return P_tmpdir ? std::string(P_tmpdir) : std::string("/tmp"); }
std::string write_temp(const std::string& name, const std::string& contents) {
  std::string path = temp_dir() + "/" + name;
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << contents;
  out.close();
  return path;
}
}  // namespace

TEST_CASE("timeline: from_json rejects documents of the wrong shape") {
  // Not an array at the top level.
  CHECK_THROWS(ActivityTimeline::from_json(JsonValue("nope")));
  CHECK_THROWS(ActivityTimeline::from_json(JsonValue(42)));
  CHECK_THROWS(ActivityTimeline::from_json_text("{}"));
  // Frame entries must be objects.
  CHECK_THROWS(ActivityTimeline::from_json_text("[1, 2, 3]"));
  CHECK_THROWS(ActivityTimeline::from_json_text("[\"x\"]"));
  // A frame missing the brain_activity array.
  CHECK_THROWS(ActivityTimeline::from_json_text("[{\"timestamp_ms\":0}]"));
  CHECK_THROWS(ActivityTimeline::from_json_text("[{\"brain_activity\":5}]"));
  // brain_activity entries must be objects.
  CHECK_THROWS(ActivityTimeline::from_json_text("[{\"brain_activity\":[1]}]"));
}

TEST_CASE("timeline: from_json tolerates entries with no usable region") {
  // An activity entry without a 'region' is skipped rather than rejected.
  auto tl = ActivityTimeline::from_json_text(
      "[{\"brain_activity\":[{\"intensity\":0.5},{\"region\":\"\",\"intensity\":0.9}],"
      "\"timestamp_ms\":7}]");
  CHECK_EQ(tl.size(), static_cast<std::size_t>(1));
  CHECK_EQ(tl.at(0).timestamp_ms, static_cast<std::int64_t>(7));
  CHECK(tl.at(0).intensities.empty());
}

TEST_CASE("timeline: from_json_file reports missing, empty and malformed files") {
  CHECK_THROWS(ActivityTimeline::from_json_file("/no/such/dir/brain-xyz.json"));

  std::string empty = write_temp("bm_test_empty.json", ");
  CHECK_THROWS(ActivityTimeline::from_json_file(empty));
  std::remove(empty.c_str());

  std::string junk = write_temp("bm_test_junk.json", "this is not json {{{");
  CHECK_THROWS(ActivityTimeline::from_json_file(junk));
  std::remove(junk.c_str());

  std::string ok = write_temp("bm_test_ok.json",
                              "[{\"brain_activity\":[{\"region\":\"amygdala\",\"intensity\":0.5}],"
                              "\"timestamp_ms\":0}]");
  ActivityTimeline tl;
  CHECK_NOTHROW(tl = ActivityTimeline::from_json_file(ok));
  CHECK_EQ(tl.size(), static_cast<std::size_t>(1));
  CHECK_NEAR(tl.at(0).intensity_of("amygdala"), 0.5, 1e-9);
  std::remove(ok.c_str());
}

TEST_CASE("timeline: from_intensities builds a normalized single frame") {
  auto tl = ActivityTimeline::from_intensities({{"Amygdala", 1.5}, {"prefrontal cortex", -0.3}}, 99);
  CHECK_EQ(tl.size(), static_cast<std::size_t>(1));
  CHECK_EQ(tl.at(0).timestamp_ms, static_cast<std::int64_t>(99));
  CHECK_NEAR(tl.at(0).intensity_of("amygdala"), 1.0, 1e-9);            // clamped & key-normalized
  CHECK_NEAR(tl.at(0).intensity_of("prefrontal_cortex"), 0.0, 1e-9);  // clamped
}

TEST_CASE("InputLoader: each kind produces a sensible starting timeline") {
  // None -> a resting baseline single frame.
  {
    InputSpec spec;
    spec.kind = InputKind::None;
    LoadedInput loaded = InputLoader::load(spec);
    CHECK(loaded.live_stream == nullptr);
    CHECK(loaded.timeline.empty());  // None yields a default (empty) timeline
  }
  // BrainState -> synthesized timeline of the requested length.
  {
    InputSpec spec;
    spec.kind = InputKind::BrainState;
    spec.state_key = "relaxed";
    spec.synth_frames = 17;
    spec.synth_step_ms = 25;
    LoadedInput loaded = InputLoader::load(spec);
    CHECK(loaded.live_stream == nullptr);
    CHECK_EQ(loaded.timeline.size(), static_cast<std::size_t>(17));
  }
  // BrainState with an unknown preset -> error.
  {
    InputSpec spec;
    spec.kind = InputKind::BrainState;
    spec.state_key = "no-such-preset";
    CHECK_THROWS(InputLoader::load(spec));
  }
  // JsonFile that does not exist -> error.
  {
    InputSpec spec;
    spec.kind = InputKind::JsonFile;
    spec.path = "/no/such/file-abc.json";
    CHECK_THROWS(InputLoader::load(spec));
  }
  // Serial with an in-memory port -> opens, seeds a baseline frame, no error.
  {
    InputSpec spec;
    spec.kind = InputKind::Serial;
    spec.use_memory_port = true;
    spec.serial.device = "memory0";
    spec.state_key = "focused";  // sends a STATE request to the device
    LoadedInput loaded = InputLoader::load(spec);
    CHECK(loaded.live_stream != nullptr);
    CHECK(loaded.live_stream->is_open());
    CHECK_EQ(loaded.timeline.size(), static_cast<std::size_t>(1));  // baseline seed
    // A JSON set_state command was written to the (memory) device.
    auto* mem = dynamic_cast<MemorySerialPort*>(&loaded.live_stream->port());
    CHECK(mem != nullptr);
    CHECK_EQ(mem->sent(), std::string("{\"command\":\"set_state\",\"state\":\"focused\"}\n"));
  }
}

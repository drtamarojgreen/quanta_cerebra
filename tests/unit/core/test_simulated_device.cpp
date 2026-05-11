#include <memory>
#include <string>
#include <vector>

#include "../test_framework.hpp"
#include "core/sample.hpp"
#include "io/serial_port.hpp"
#include "io/simulated_device.hpp"

using namespace cerebra;

namespace {
// Wires a SimulatedSerialDevice to the device end of a port and a
// SerialActivityStream to the application end (the same MemorySerialPort).
struct Link {
  MemorySerialPort* port = nullptr;
  std::unique_ptr<SerialActivityStream> stream;
  std::unique_ptr<SimulatedSerialDevice> device;
  Link(const std::string& preset, std::int64_t step_ms) {
    auto owned = std::make_unique<MemorySerialPort>();
    port = owned.get();
    stream = std::make_unique<SerialActivityStream>(std::move(owned));
    stream->open(SerialConfig{});
    device = std::make_unique<SimulatedSerialDevice>(*port, preset, step_ms);
  }
  std::vector<BrainActivitySample> drain() {
    std::vector<BrainActivitySample> all;
    for (int i = 0; i < 16; ++i) {
      auto f = stream->poll();
      all.insert(all.end(), f.begin(), f.end());
    }
    return all;
  }
};
}  // namespace

TEST_CASE("simulated device transmits decodable JSON activity frames") {
  Link link("focused", 100);
  CHECK_EQ(link.device->current_state(), std::string("focused"));
  link.device->emit_frames(5);
  CHECK_EQ(link.device->frames_emitted(), static_cast<std::size_t>(5));

  auto frames = link.drain();
  CHECK_EQ(frames.size(), static_cast<std::size_t>(5));
  CHECK_EQ(link.stream->frames_decoded(), static_cast<std::size_t>(5));
  CHECK_EQ(link.stream->parse_errors(), static_cast<std::size_t>(0));
  // Frames carry region intensities and monotonically increasing timestamps.
  for (const auto& f : frames) CHECK(!f.intensities.empty());
  CHECK_EQ(frames.front().timestamp_ms, static_cast<std::int64_t>(0));
  CHECK_EQ(frames.back().timestamp_ms, static_cast<std::int64_t>(400));  // 4 * 100ms
  // "focused" should keep the amygdala fairly quiet.
  CHECK(frames.front().intensity_of("amygdala") < 0.5);
}

TEST_CASE("simulated device acts on JSON set_state commands from the application") {
  Link link("focused", 100);
  CHECK_EQ(link.device->current_state(), std::string("focused"));

  // The application asks the device to switch presets.
  link.stream->request_state("stressed");
  int handled = link.device->process_inbound();
  CHECK_EQ(handled, 1);
  CHECK_EQ(link.device->commands_processed(), static_cast<std::size_t>(1));
  CHECK_EQ(link.device->current_state(), std::string("stressed"));

  link.device->emit_frames(3);
  auto frames = link.drain();
  CHECK_EQ(frames.size(), static_cast<std::size_t>(3));
  // "stressed" drives the amygdala hard.
  CHECK(frames.back().intensity_of("amygdala") > 0.6);

  // Unknown commands, invalid states and malformed lines are ignored, like
  // real firmware would.
  JsonValue::Object reboot;
  reboot.emplace("command", JsonValue("reboot"));
  link.stream->send_json(JsonValue(std::move(reboot)));
  JsonValue::Object bad_state;
  bad_state.emplace("command", JsonValue("set_state"));
  bad_state.emplace("state", JsonValue("banana"));
  link.stream->send_json(JsonValue(std::move(bad_state)));
  link.port->write_str("{not valid json at all\n");  // garbage on the wire
  CHECK_EQ(link.device->process_inbound(), 0);
  CHECK_EQ(link.device->commands_processed(), static_cast<std::size_t>(1));
  CHECK_EQ(link.device->current_state(), std::string("stressed"));
}

TEST_CASE("simulated device tick() converts elapsed time to emitted frames") {
  Link link("relaxed", 100);
  link.device->tick(350);   // 3 whole frames, 0.5 carried over
  link.device->tick(150);   // 0.5 + 1.5 = 2 whole frames
  CHECK_EQ(link.device->frames_emitted(), static_cast<std::size_t>(5));
  link.device->tick(0);     // no time -> no frames
  link.device->tick(-10);   // negative -> no frames
  CHECK_EQ(link.device->frames_emitted(), static_cast<std::size_t>(5));

  auto frames = link.drain();
  CHECK_EQ(frames.size(), static_cast<std::size_t>(5));
}

TEST_CASE("simulated device defaults to a valid preset for an unknown name") {
  Link link("not-a-real-preset", 100);
  // Falls back to the first library preset rather than crashing.
  CHECK(!link.device->current_state().empty());
  link.device->emit_frames(1);
  CHECK_EQ(link.drain().size(), static_cast<std::size_t>(1));
}

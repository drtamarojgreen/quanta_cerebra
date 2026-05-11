#ifndef BRAIN_MODELER_SIMULATED_DEVICE_HPP
#define BRAIN_MODELER_SIMULATED_DEVICE_HPP

#include <cstdint>
#include <string>

#include "core/sample.hpp"
#include "io/serial_port.hpp"

namespace cerebra {

// A fake "firmware" sitting on the device end of a MemorySerialPort, for
// testing the serial pipeline without hardware. It transmits JSON brain-activity
// frames toward the application, and consumes the JSON command payloads the
// application sends back (currently {"command":"set_state","state":"<preset>"}),
// switching the preset it emits accordingly.
class SimulatedSerialDevice {
public:
  // `port` is the application's end of the link; it must outlive this device.
  explicit SimulatedSerialDevice(MemorySerialPort& port,
                                 const std::string& initial_state = "relaxed",
                                 std::int64_t step_ms = 100);

  // Read and act on any JSON command lines the application has sent. Returns the
  // number of recognised commands handled.
  int process_inbound();

  // Transmit `count` JSON activity frames toward the application.
  void emit_frames(int count = 1);

  // Process inbound commands, then emit floor(elapsed_ms / step_ms) frames.
  void tick(std::int64_t elapsed_ms);

  const std::string& current_state() const { return state_key_; }
  std::size_t frames_emitted() const { return frames_emitted_; }
  std::size_t commands_processed() const { return commands_processed_; }
  std::int64_t step_ms() const { return step_ms_; }

private:
  void load_preset(const std::string& key);

  MemorySerialPort& port_;
  std::string state_key_;
  std::int64_t step_ms_ = 100;
  ActivityTimeline preset_timeline_;  // animated frames for the current preset
  std::size_t cursor_ = 0;            // position within preset_timeline_
  std::size_t frames_emitted_ = 0;
  std::size_t commands_processed_ = 0;
  double frame_accumulator_ = 0.0;
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_SIMULATED_DEVICE_HPP

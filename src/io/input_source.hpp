#ifndef BRAIN_MODELER_INPUT_SOURCE_HPP
#define BRAIN_MODELER_INPUT_SOURCE_HPP

#include <memory>
#include <string>

#include "core/sample.hpp"
#include "io/serial_port.hpp"

namespace cerebra {

// Where the simulation's activity data comes from.
enum class InputKind {
  None,         // empty (resting baseline) timeline
  JsonFile,     // a JSON array-of-frames document on disk
  BrainState,   // a synthesized timeline from a predefined preset
  Serial,       // a live serial stream from an experimental device
};

struct InputSpec {
  InputKind kind = InputKind::None;
  std::string path;            // for JsonFile
  std::string state_key;       // for BrainState / initial Serial preset
  SerialConfig serial;         // for Serial
  bool use_memory_port = false;  // for Serial: simulate a device (demo/testing)
  int synth_frames = 80;       // for BrainState
  std::int64_t synth_step_ms = 100;
};

// Resolves an InputSpec into a starting timeline plus, for serial sources, an
// open live stream the application can keep polling.
struct LoadedInput {
  ActivityTimeline timeline;
  std::unique_ptr<SerialActivityStream> live_stream;  // null unless serial
};

class InputLoader {
public:
  // Throws std::runtime_error on a fatal problem (missing file, bad JSON,
  // serial port that won't open, unknown preset).
  static LoadedInput load(const InputSpec& spec);

  // Build a serial stream using either the native port or, when
  // `use_memory_port` is true, an in-memory port (for tests/demos).
  static std::unique_ptr<SerialActivityStream> make_serial_stream(const SerialConfig& cfg,
                                                                  bool use_memory_port = false);
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_INPUT_SOURCE_HPP

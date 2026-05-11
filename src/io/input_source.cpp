#include "io/input_source.hpp"

#include <stdexcept>

#include "core/state_manager.h"

namespace cerebra {

std::unique_ptr<SerialActivityStream> InputLoader::make_serial_stream(const SerialConfig& cfg,
                                                                      bool use_memory_port) {
  std::unique_ptr<SerialPort> port =
      use_memory_port ? std::unique_ptr<SerialPort>(std::make_unique<MemorySerialPort>())
                      : SerialPort::create_native();
  auto stream = std::make_unique<SerialActivityStream>(std::move(port));
  if (!stream->open(cfg)) {
    throw std::runtime_error("could not open serial device: " + cfg.device);
  }
  return stream;
}

LoadedInput InputLoader::load(const InputSpec& spec) {
  LoadedInput out;
  switch (spec.kind) {
    case InputKind::None:
      out.timeline = ActivityTimeline();
      break;
    case InputKind::JsonFile:
      out.timeline = ActivityTimeline::from_json_file(spec.path);
      break;
    case InputKind::BrainState: {
      const BrainStateTemplate* tmpl = BrainStateLibrary::find(spec.state_key);
      if (!tmpl) throw std::runtime_error("unknown brain-state preset: " + spec.state_key);
      out.timeline = BrainStateLibrary::synthesize_timeline(*tmpl, spec.synth_frames,
                                                            spec.synth_step_ms);
      break;
    }
    case InputKind::Serial: {
      out.live_stream = make_serial_stream(spec.serial, spec.use_memory_port);
      if (!spec.state_key.empty()) out.live_stream->request_state(spec.state_key);
      // Seed with a baseline frame so the UI has something before data arrives.
      out.timeline = ActivityTimeline::from_intensities({}, 0);
      break;
    }
  }
  return out;
}

}  // namespace cerebra

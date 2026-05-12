#include "ui/report_ui.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

#include "visualization/scene_renderer.h"
#include "io/simulated_device.hpp"
#include "ui/terminal_renderer.h"

namespace cerebra {

std::string ReportUi::build(const Simulation& sim, const CliOptions& options) {
  int width = options.width_override.value_or(cerebra::terminal_size().cols);
  width = std::max(40, std::min(width, 200));
  // Reports are plain text so they paste cleanly into logs/notebooks.
  bool ascii = options.force_ascii;
  return Renderer::render_report(sim, width, options.view, ascii ? true : false);
}

int ReportUi::run(const CliOptions& options, std::ostream& out) {
  Simulation sim;
  try {
    LoadedInput loaded = InputLoader::load(options.input);
    // For a serial source in report mode, poll briefly to gather some frames.
    if (loaded.live_stream) {
      // If this is the simulated in-memory device, prime it with a burst of
      // JSON activity frames so the report has a real timeline to summarise.
      std::unique_ptr<SimulatedSerialDevice> demo_device;
      if (options.use_memory_serial) {
        if (auto* mem = dynamic_cast<MemorySerialPort*>(&loaded.live_stream->port())) {
          demo_device = std::make_unique<SimulatedSerialDevice>(
              *mem, options.input.state_key.empty() ? "relaxed" : options.input.state_key,
              std::max<std::int64_t>(1, options.input.synth_step_ms));
          demo_device->process_inbound();
          demo_device->emit_frames(80);
        }
      }
      for (int i = 0; i < 50; ++i) {
        auto frames = loaded.live_stream->poll();
        for (auto& f : frames) loaded.timeline.append(std::move(f));
        if (loaded.timeline.size() > 200) break;
      }
    }
    if (loaded.timeline.empty()) {
      // Fall back to a short resting-baseline timeline so the report isn't empty.
      loaded.timeline = ActivityTimeline::from_intensities({}, 0);
    }
    sim.set_timeline(std::move(loaded.timeline));
  } catch (const std::exception& e) {
    std::cerr << "brain_modeler: " << e.what() << "\n";
    return 1;
  }
  if (options.preselect_region) sim.select_region(*options.preselect_region);
  sim.jump_to_end();  // report summarizes the full run; show the last frame view
  out << build(sim, options);
  out.flush();
  return 0;
}

}  // namespace cerebra

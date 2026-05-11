// Step definitions for features/playback.feature.

#include <algorithm>
#include <vector>

#include "core/state_manager.h"
#include "gherkin.hpp"

GIVEN("a loaded simulation with {int} frames at {int} fps") {
  int frames = std::max(1, bdd::arg_int(args, 0));
  int fps = std::max(1, bdd::arg_int(args, 1));
  std::vector<cerebra::BrainActivitySample> samples;
  for (int i = 0; i < frames; ++i) {
    cerebra::BrainActivitySample s;
    s.timestamp_ms = static_cast<std::int64_t>(i) * 100;
    s.intensities["amygdala"] = static_cast<double>(i) / static_cast<double>(frames);
    samples.push_back(s);
  }
  world.sim = std::make_unique<cerebra::Simulation>(cerebra::ActivityTimeline(std::move(samples)));
  world.sim->set_frames_per_second(static_cast<double>(fps));
}

GIVEN("a simulation built from the {string} preset") {
  const cerebra::BrainStateTemplate* tmpl = cerebra::BrainStateLibrary::find(bdd::arg_str(args, 0));
  CHECK_MSG(tmpl != nullptr, "unknown brain-state preset: " + bdd::arg_str(args, 0));
  world.sim = std::make_unique<cerebra::Simulation>(
      cerebra::BrainStateLibrary::synthesize_timeline(*tmpl, 40, 50));
}

WHEN("the simulation is paused") {
  world.active_sim().pause();
}

WHEN("the user pauses and {int} ms of time passes") {
  world.active_sim().pause();
  world.active_sim().advance(bdd::arg_ll(args, 0));
}

WHEN("the user resumes and {int} ms of time passes") {
  world.active_sim().play();
  world.active_sim().advance(bdd::arg_ll(args, 0));
}

WHEN("{int} ms of time passes") {
  world.active_sim().advance(bdd::arg_ll(args, 0));
}

WHEN("the user fast-forwards {int} frames") {
  world.active_sim().fast_forward(static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

WHEN("the user rewinds {int} frames") {
  world.active_sim().rewind(static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

THEN("the chemistry is stable after seeking to the end") {
  cerebra::Simulation& sim = world.active_sim();
  CHECK(sim.frame_count() > 0);
  sim.jump_to_end();
  double a = sim.chemical_state().at("norepinephrine");
  sim.restart();
  sim.seek_to(sim.frame_count() - 1);
  CHECK_NEAR(sim.chemical_state().at("norepinephrine"), a, 1e-9);
}

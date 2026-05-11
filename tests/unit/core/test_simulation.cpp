#include "../test_framework.hpp"
#include "core/state_manager.h"
#include "core/sample.hpp"
#include "core/simulation_engine.h"

using namespace cerebra;

namespace {
ActivityTimeline make_ramp(int n) {
  std::vector<BrainActivitySample> s;
  for (int i = 0; i < n; ++i) {
    BrainActivitySample b;
    b.timestamp_ms = i * 100;
    b.intensities["amygdala"] = static_cast<double>(i) / std::max(1, n - 1);
    s.push_back(b);
  }
  return ActivityTimeline(std::move(s));
}
}  // namespace

TEST_CASE("timeline from_json parses and clamps intensities") {
  const char* doc = R"([
    {"brain_activity":[{"region":"amygdala","intensity":1.5},
                       {"region":"Insula","intensity":-0.2}],"timestamp_ms":50},
    {"brain_activity":[{"region":"amygdala","intensity":0.4}],"timestamp_ms":10}
  ])";
  auto tl = ActivityTimeline::from_json_text(doc);
  CHECK_EQ(tl.size(), static_cast<std::size_t>(2));
  // Sorted by timestamp.
  CHECK_EQ(tl.at(0).timestamp_ms, static_cast<std::int64_t>(10));
  CHECK_EQ(tl.at(1).timestamp_ms, static_cast<std::int64_t>(50));
  CHECK_NEAR(tl.at(1).intensity_of("amygdala"), 1.0, 1e-9);  // clamped to 1
  CHECK_NEAR(tl.at(1).intensity_of("insula"), 0.0, 1e-9);    // clamped to 0, key normalized
}

TEST_CASE("simulation steps, rewinds and seeks") {
  Simulation sim(make_ramp(10));
  CHECK_EQ(sim.frame_count(), static_cast<std::size_t>(10));
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
  sim.step_forward(3);
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(3));
  sim.fast_forward(100);  // clamps to end
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(9));
  sim.rewind(4);
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(5));
  sim.restart();
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
  sim.seek_to(7);
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(7));
  sim.jump_to_end();
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(9));
}

TEST_CASE("simulation advance respects fps and speed, stopping at the end") {
  Simulation sim(make_ramp(11));
  sim.set_frames_per_second(10.0);
  sim.set_speed(1.0);
  sim.play();
  sim.advance(1000);  // 10 fps * 1s = 10 frames
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(10));
  CHECK(!sim.playing());  // auto-paused at the end

  sim.restart();
  sim.play();
  sim.set_speed(0.5);
  sim.advance(1000);  // 5 frames
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(5));
}

TEST_CASE("simulation pause prevents advance") {
  Simulation sim(make_ramp(20));
  sim.set_frames_per_second(10.0);
  sim.pause();
  sim.advance(5000);
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
  sim.toggle_play_pause();
  CHECK(sim.playing());
  sim.advance(100);  // 1 frame
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(1));
}

TEST_CASE("negative speed plays backwards") {
  Simulation sim(make_ramp(20));
  sim.set_frames_per_second(10.0);
  sim.seek_to(15);
  sim.play();
  sim.set_speed(-1.0);
  sim.advance(500);  // 5 frames backwards
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(10));
}

TEST_CASE("region selection cycles through the catalog") {
  Simulation sim(make_ramp(3));
  CHECK(!sim.selected_region().has_value());
  sim.select_next_region();
  CHECK(sim.selected_region().has_value());
  auto first = *sim.selected_region();
  sim.select_previous_region();
  sim.select_next_region();
  CHECK_EQ(*sim.selected_region(), first);
  sim.select_region("Amygdala");
  CHECK_EQ(*sim.selected_region(), std::string("amygdala"));
  sim.clear_selection();
  CHECK(!sim.selected_region().has_value());
}

TEST_CASE("chemistry recomputes consistently after a seek") {
  Simulation sim(make_ramp(12));
  sim.jump_to_end();
  auto end_state = sim.chemical_state();
  sim.restart();
  sim.seek_to(11);
  CHECK_NEAR(sim.chemical_state().at("norepinephrine"), end_state.at("norepinephrine"), 1e-9);
}

TEST_CASE("simulation speed is clamped and the +/- helpers move sensibly") {
  Simulation sim(make_ramp(10));
  sim.set_speed(1000.0);
  CHECK_NEAR(sim.speed(), 16.0, 1e-9);
  sim.set_speed(-1000.0);
  CHECK_NEAR(sim.speed(), -16.0, 1e-9);

  sim.set_speed(1.0);
  sim.faster();
  CHECK_NEAR(sim.speed(), 2.0, 1e-9);
  sim.faster();
  CHECK_NEAR(sim.speed(), 4.0, 1e-9);

  sim.set_speed(1.0);
  sim.slower();
  CHECK_NEAR(sim.speed(), -1.0, 1e-9);  // flips into rewind
  sim.slower();
  CHECK_NEAR(sim.speed(), -2.0, 1e-9);  // faster rewind

  sim.set_speed(0.0);
  sim.faster();
  CHECK_NEAR(sim.speed(), 1.0, 1e-9);   // 0 -> forward
}

TEST_CASE("advance is a no-op when paused, empty, at speed zero or with non-positive time") {
  Simulation empty;
  CHECK(empty.empty());
  CHECK(!empty.advance(1000));

  Simulation sim(make_ramp(20));
  sim.set_frames_per_second(10.0);
  sim.pause();
  CHECK(!sim.advance(1000));
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));

  sim.play();
  sim.set_speed(0.0);
  CHECK(!sim.advance(1000));
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));

  sim.set_speed(1.0);
  CHECK(!sim.advance(0));
  CHECK(!sim.advance(-100));
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
  CHECK(sim.advance(100));  // finally something happens
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(1));
}

TEST_CASE("rewinding to the very start while playing pauses the simulation") {
  Simulation sim(make_ramp(20));
  sim.set_frames_per_second(10.0);
  sim.seek_to(3);
  sim.play();
  sim.set_speed(-1.0);
  sim.advance(1000);  // 10 frames of rewind, only 3 available
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
  CHECK(!sim.playing());
}

TEST_CASE("extend_timeline keeps the playback position when possible") {
  Simulation sim(make_ramp(5));
  sim.seek_to(3);
  // A longer timeline (e.g. more serial frames arrived): position is kept.
  sim.extend_timeline(make_ramp(12));
  CHECK_EQ(sim.frame_count(), static_cast<std::size_t>(12));
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(3));
  // A shorter timeline: position clamps to the new end.
  sim.seek_to(11);
  sim.extend_timeline(make_ramp(4));
  CHECK_EQ(sim.frame_count(), static_cast<std::size_t>(4));
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(3));
  // Extending with an empty timeline leaves an empty simulation.
  sim.extend_timeline(ActivityTimeline());
  CHECK(sim.empty());
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
}

TEST_CASE("an empty simulation has a zeroed current sample and baseline chemistry") {
  Simulation sim;
  CHECK(sim.empty());
  CHECK_EQ(sim.frame_count(), static_cast<std::size_t>(0));
  CHECK_EQ(sim.current_sample().timestamp_ms, static_cast<std::int64_t>(0));
  CHECK(sim.current_sample().intensities.empty());
  // Baseline chemistry: norepinephrine sits at its resting level.
  CHECK_NEAR(sim.chemical_state().at("norepinephrine"), 0.20, 1e-9);
  // Navigation on an empty timeline is harmless.
  sim.step_forward(5);
  sim.rewind(5);
  sim.jump_to_end();
  sim.restart();
  CHECK_EQ(sim.current_index(), static_cast<std::size_t>(0));
}

TEST_CASE("brain-state lookup resolves aliases and casing") {
  CHECK(BrainStateLibrary::find("REM") != nullptr);
  CHECK_EQ(BrainStateLibrary::find("REM")->key, std::string("rem_sleep"));
  CHECK_EQ(BrainStateLibrary::find("rem sleep")->key, std::string("rem_sleep"));
  CHECK_EQ(BrainStateLibrary::find("REM-SLEEP")->key, std::string("rem_sleep"));
  CHECK_EQ(BrainStateLibrary::find("Stressed")->key, std::string("stressed"));
  CHECK_EQ(BrainStateLibrary::find("FOCUSED")->key, std::string("focused"));
  CHECK(BrainStateLibrary::find(") == nullptr);
  CHECK(BrainStateLibrary::find("daydreaming") == nullptr);
  // synthesize_timeline never produces out-of-range intensities or zero frames.
  auto tl = BrainStateLibrary::synthesize_timeline(*BrainStateLibrary::find("stressed"), 0, 0);
  CHECK_EQ(tl.size(), static_cast<std::size_t>(1));
  auto rem = BrainStateLibrary::synthesize_timeline(*BrainStateLibrary::find("rem_sleep"), 30, 10);
  for (const auto& s : rem.samples()) {
    for (const auto& kv : s.intensities) {
      CHECK(kv.second >= 0.0);
      CHECK(kv.second <= 1.0);
    }
  }
}

TEST_CASE("brain-state presets exist and synthesize timelines") {
  for (const char* k : {"focused", "relaxed", "stressed", "rem_sleep"}) {
    const auto* tmpl = BrainStateLibrary::find(k);
    CHECK_MSG(tmpl != nullptr, std::string("missing preset ") + k);
    auto tl = BrainStateLibrary::synthesize_timeline(*tmpl, 24, 50);
    CHECK_EQ(tl.size(), static_cast<std::size_t>(24));
    // Synthesis is deterministic.
    auto tl2 = BrainStateLibrary::synthesize_timeline(*tmpl, 24, 50);
    CHECK_NEAR(tl.at(10).intensity_of("amygdala"), tl2.at(10).intensity_of("amygdala"), 1e-12);
  }
  CHECK(BrainStateLibrary::find("REM") != nullptr);  // alias
  CHECK(BrainStateLibrary::find("does-not-exist") == nullptr);
}

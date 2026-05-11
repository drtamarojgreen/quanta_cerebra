#include <sstream>
#include <string>
#include <vector>

#include "../test_framework.hpp"
#include "core/state_manager.h"
#include "ui/cli_handler.h"
#include "core/region.hpp"
#include "visualization/scene_renderer.h"
#include "ui/report_ui.hpp"
#include "core/sample.hpp"
#include "core/simulation_engine.h"

using namespace cerebra;

namespace {

bool contains(const std::string& h, const std::string& n) { return h.find(n) != std::string::npos; }
bool is_pure_ascii(const std::string& s) {
  for (unsigned char c : s)
    if (c >= 0x80) return false;
  return true;
}
std::vector<std::string> split_lines(const std::string& s) {
  std::vector<std::string> out;
  std::istringstream in(s);
  std::string line;
  while (std::getline(in, line)) out.push_back(line);
  return out;
}
std::string line_starting_with(const std::string& report, const std::string& prefix) {
  for (const auto& l : split_lines(report))
    if (l.rfind(prefix, 0) == 0) return l;
  return ";
}

// A small, hand-checkable timeline:
//   amygdala : 0.2, 0.8, 0.5   -> peak 0.80, mean 0.50, last 0.50
//   thalamus : 0.4, 0.4, 0.4   -> peak 0.40, mean 0.40, last 0.40
Simulation hand_timeline() {
  std::vector<BrainActivitySample> s(3);
  s[0].timestamp_ms = 0;   s[0].intensities = {{"amygdala", 0.2}, {"thalamus", 0.4}};
  s[1].timestamp_ms = 100; s[1].intensities = {{"amygdala", 0.8}, {"thalamus", 0.4}};
  s[2].timestamp_ms = 200; s[2].intensities = {{"amygdala", 0.5}, {"thalamus", 0.4}};
  Simulation sim(ActivityTimeline(std::move(s)));
  sim.jump_to_end();
  return sim;
}

}  // namespace

TEST_CASE("report: header block reflects the timeline metadata") {
  Simulation sim = hand_timeline();
  std::string r = Renderer::render_report(sim, 80, ViewMode::Slice2D, true);
  CHECK(contains(r, "Brain Modeler - simulation report"));
  CHECK(contains(r, "frames      : 3"));
  CHECK(contains(r, "duration    : 200 ms"));
  CHECK(contains(r, "start ts    : 0 ms"));
  CHECK(contains(r, "end ts      : 200 ms"));
  CHECK(contains(r, "view        : 2D slice"));
}

TEST_CASE("report: width controls the horizontal rule and brain-view widths") {
  Simulation sim = hand_timeline();
  for (int w : {40, 60, 80, 100, 132}) {
    std::string r = Renderer::render_report(sim, w, ViewMode::Slice2D, true);
    std::string rule = line_starting_with(r, "====");
    CHECK_MSG(!rule.empty(), "report missing a '===' rule at width " + std::to_string(w));
    CHECK_EQ(rule.size(), static_cast<std::size_t>(w));
    CHECK(rule == std::string(static_cast<std::size_t>(w), '='));

    // The brain view immediately follows the "Coronal slice" header + a rule;
    // in ASCII mode each of its rows is exactly `width` columns.
    auto lines = split_lines(r);
    int idx = -1;
    for (std::size_t i = 0; i < lines.size(); ++i) {
      if (lines[i].rfind("Coronal slice", 0) == 0) { idx = static_cast<int>(i); break; }
    }
    CHECK(idx >= 0);
    CHECK(idx + 3 < static_cast<int>(lines.size()));
    CHECK_EQ(lines[idx + 2].size(), static_cast<std::size_t>(w));
    CHECK_EQ(lines[idx + 3].size(), static_cast<std::size_t>(w));
  }
  // render_report clamps very small widths up to 40.
  std::string tiny = Renderer::render_report(sim, 5, ViewMode::Slice2D, true);
  CHECK_EQ(line_starting_with(tiny, "====").size(), static_cast<std::size_t>(40));
}

TEST_CASE("report: region table lists every region with correct ROI flags and stats") {
  Simulation sim = hand_timeline();
  std::string r = Renderer::render_report(sim, 90, ViewMode::Slice2D, true);
  CHECK(contains(r, "Region activity"));
  CHECK(contains(r, "ROI  peak  mean  last  trace"));

  // All ten modelled regions appear by display name.
  for (const auto& info : RegionCatalog::all())
    CHECK_MSG(contains(r, info.display_name), "report missing region: " + info.display_name);
  CHECK(contains(r, "Cingulate Gyrus"));   // 'gyrus' renders with its full name

  // The amygdala row: flagged ROI, hand-computed peak/mean/last.
  std::string amy = line_starting_with(r, "Amygdala");
  CHECK_MSG(!amy.empty(), "no Amygdala row");
  CHECK(contains(amy, "yes"));
  CHECK(contains(amy, "0.80"));
  CHECK(contains(amy, "0.50"));

  // The thalamus row: ROI, flat at 0.40.
  std::string tha = line_starting_with(r, "Thalamus");
  CHECK(contains(tha, "yes"));
  CHECK(contains(tha, "0.40"));

  // A non-ROI region with no activity in this timeline.
  std::string occ = line_starting_with(r, "Occipital Lobe");
  CHECK(contains(occ, " no "));
  CHECK(contains(occ, "0.00"));
  std::string pfc = line_starting_with(r, "Prefrontal Cortex");
  CHECK(contains(pfc, " no "));
  CHECK(contains(pfc, "0.00"));
}

TEST_CASE("report: neurochemistry section lists every transmitter") {
  Simulation sim = hand_timeline();
  std::string r = Renderer::render_report(sim, 80, ViewMode::Slice2D, true);
  CHECK(contains(r, "Neurochemistry (final frame)"));
  for (const char* nt : {"Dopamine", "Serotonin", "Norepinephrine", "Acetylcholine",
                         "Glutamate", "GABA"}) {
    CHECK_MSG(contains(r, nt), std::string("report missing transmitter: ") + nt);
  }
}

TEST_CASE("report: the brain-view section header and legend follow the view mode") {
  Simulation sim = hand_timeline();
  std::string r2 = Renderer::render_report(sim, 80, ViewMode::Slice2D, true);
  CHECK(contains(r2, "Coronal slice (final frame)"));
  CHECK(!contains(r2, "3D projection (final frame)"));

  std::string r3 = Renderer::render_report(sim, 80, ViewMode::Projection3D, true);
  CHECK(contains(r3, "3D projection (final frame)"));
  CHECK(contains(r3, "view        : 3D projection"));

  CHECK(contains(r2, "intensity legend"));
  CHECK(contains(r2, "(low -> high)"));
}

TEST_CASE("report: ascii mode is byte-for-byte ASCII; unicode mode is not") {
  Simulation sim(BrainStateLibrary::synthesize_timeline(*BrainStateLibrary::find("focused"), 12, 50));
  sim.jump_to_end();
  std::string ascii = Renderer::render_report(sim, 80, ViewMode::Slice2D, /*ascii_only=*/true);
  CHECK_MSG(is_pure_ascii(ascii), "ascii report contains a non-ASCII byte");
  // No ANSI escape sequences either (reports are always color-free).
  CHECK(ascii.find('\x1b') == std::string::npos);

  std::string uni = Renderer::render_report(sim, 80, ViewMode::Slice2D, /*ascii_only=*/false);
  CHECK_MSG(!is_pure_ascii(uni), "expected unicode block glyphs in the non-ascii report");
}

TEST_CASE("report: degenerate sizes and a single-frame timeline still render") {
  Simulation single(ActivityTimeline::from_intensities({{"amygdala", 0.7}, {"cuneus", 0.3}}, 0));
  single.jump_to_end();
  std::string r = Renderer::render_report(single, 41, ViewMode::Slice2D, true);
  CHECK(contains(r, "frames      : 1"));
  CHECK(contains(r, "duration    : 0 ms"));
  CHECK(contains(r, "Amygdala"));

  // An empty simulation: no metadata lines beyond the frame count, but the
  // structure (and a zeroed region table) must still be there.
  Simulation empty;
  CHECK(empty.empty());
  std::string e = Renderer::render_report(empty, 60, ViewMode::Projection3D, true);
  CHECK(contains(e, "frames      : 0"));
  CHECK(contains(e, "Region activity"));
  CHECK(contains(e, "3D projection (final frame)"));
  CHECK(contains(e, "Neurochemistry"));
  std::string amy = line_starting_with(e, "Amygdala");
  CHECK(contains(amy, "0.00"));
}

TEST_CASE("report: ReportUi::run wires options through and exits cleanly") {
  // Brain-state preset, 3D, ascii, custom width.
  CliOptions opts;
  opts.format = OutputFormat::Report;
  opts.input.kind = InputKind::BrainState;
  opts.input.state_key = "rem_sleep";
  opts.input.synth_frames = 9;
  opts.view = ViewMode::Projection3D;
  opts.force_ascii = true;
  opts.width_override = 72;
  std::ostringstream out;
  int rc = ReportUi::run(opts, out);
  CHECK_EQ(rc, 0);
  std::string r = out.str();
  CHECK(contains(r, "frames      : 9"));
  CHECK(contains(r, "3D projection"));
  CHECK(contains(r, "Hippocampus"));
  CHECK(is_pure_ascii(r));
  CHECK_EQ(line_starting_with(r, "====").size(), static_cast<std::size_t>(72));

  // A preselected region is honoured (the report still summarises everything).
  CliOptions opts2;
  opts2.format = OutputFormat::Report;
  opts2.input.kind = InputKind::None;  // resting baseline, single frame
  opts2.preselect_region = "amygdala";
  opts2.force_ascii = true;
  std::ostringstream out2;
  CHECK_EQ(ReportUi::run(opts2, out2), 0);
  CHECK(contains(out2.str(), "simulation report"));

  // A missing input file is a hard error.
  CliOptions bad;
  bad.format = OutputFormat::Report;
  bad.input.kind = InputKind::JsonFile;
  bad.input.path = "/no/such/path/brain-xyz.json";
  std::ostringstream out3;
  CHECK(ReportUi::run(bad, out3) != 0);
}

TEST_CASE("report: every line of an ascii report fits within the requested width") {
  Simulation sim(BrainStateLibrary::synthesize_timeline(*BrainStateLibrary::find("stressed"), 15, 40));
  sim.jump_to_end();
  // The region table fixes ~64 columns as a practical floor (name + flag +
  // peak/mean/last + a 12-wide sparkline); at or above that, every line fits.
  for (int w : {64, 80, 120}) {
    std::string r = Renderer::render_report(sim, w, ViewMode::Slice2D, true);
    for (const auto& l : split_lines(r)) {
      CHECK_MSG(l.size() <= static_cast<std::size_t>(w),
                "line wider than " + std::to_string(w) + ": '" + l + "'");
    }
  }
}

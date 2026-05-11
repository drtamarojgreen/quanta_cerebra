#include <algorithm>
#include <utility>
#include <vector>

#include "../test_framework.hpp"
#include "core/state_manager.h"
#include "visualization/scene_renderer.h"
#include "core/sample.hpp"
#include "core/simulation_engine.h"

using namespace cerebra;

namespace {
Simulation focused_sim() {
  const auto* tmpl = BrainStateLibrary::find("focused");
  Simulation sim(BrainStateLibrary::synthesize_timeline(*tmpl, 20, 50));
  return sim;
}

bool contains(const std::string& haystack, const std::string& needle) {
  return haystack.find(needle) != std::string::npos;
}
}  // namespace

TEST_CASE("intensity glyph ramps from blank to solid") {
  CHECK_EQ(Renderer::intensity_glyph(0.0, true), std::string(" "));
  CHECK_EQ(Renderer::intensity_glyph(1.0, true), std::string("@"));
  CHECK(Renderer::intensity_glyph(0.5, true) != std::string(" "));
  // Unicode variant is a single (multi-byte) shade glyph, never empty.
  CHECK(!Renderer::intensity_glyph(0.7, false).empty());
}

TEST_CASE("brain view returns exactly the requested number of rows") {
  Simulation sim = focused_sim();
  RenderOptions opt;
  opt.width = 70;
  opt.height = 24;
  opt.ascii_only = true;
  Renderer r(opt);
  auto slice = r.render_brain_view(sim, 60, 18);
  CHECK_EQ(slice.size(), static_cast<std::size_t>(18));
  // ASCII-only mode keeps one byte per cell, so each row is `width` chars.
  for (const auto& line : slice) CHECK_EQ(line.size(), static_cast<std::size_t>(60));

  opt.view = ViewMode::Projection3D;
  Renderer r3(opt);
  auto proj = r3.render_brain_view(sim, 60, 20);
  CHECK_EQ(proj.size(), static_cast<std::size_t>(20));
}

TEST_CASE("interactive frame fits within the configured height") {
  Simulation sim = focused_sim();
  sim.select_region("amygdala");
  RenderOptions opt;
  opt.width = 100;
  opt.height = 30;
  opt.ascii_only = true;
  Renderer r(opt);
  auto frame = r.render_frame(sim);
  CHECK(frame.size() <= static_cast<std::size_t>(30));
  CHECK(!frame.empty());
  // Title row mentions the app and the play state.
  CHECK(contains(frame.front(), "Brain Modeler"));
}

TEST_CASE("report contains all the expected sections and ROI flags") {
  auto tl = ActivityTimeline::from_json_text(
      R"([{"brain_activity":[{"region":"amygdala","intensity":0.9},
                            {"region":"thalamus","intensity":0.5}],"timestamp_ms":0},
          {"brain_activity":[{"region":"amygdala","intensity":0.7}],"timestamp_ms":100}])");
  Simulation sim(std::move(tl));
  sim.jump_to_end();
  std::string report = Renderer::render_report(sim, 80, ViewMode::Slice2D, true);
  CHECK(contains(report, "simulation report"));
  CHECK(contains(report, "Region activity"));
  CHECK(contains(report, "Neurochemistry"));
  CHECK(contains(report, "Amygdala"));
  CHECK(contains(report, "Thalamus"));
  CHECK(contains(report, "intensity legend"));
  // The amygdala row should be flagged as a region of interest.
  auto pos = report.find("Amygdala");
  CHECK(pos != std::string::npos);
  CHECK(contains(report.substr(pos, 40), "yes"));
}

TEST_CASE("report frame count reflects the timeline") {
  Simulation sim(BrainStateLibrary::synthesize_timeline(*BrainStateLibrary::find("relaxed"), 7, 10));
  std::string report = Renderer::render_report(sim, 60, ViewMode::Projection3D, true);
  CHECK(contains(report, "frames      : 7"));
  CHECK(contains(report, "3D projection"));
}

TEST_CASE("mouse hit testing maps clicks to regions") {
  Simulation sim = focused_sim();
  RenderOptions opt;
  opt.width = 81;
  opt.height = 25;
  opt.ascii_only = true;
  Renderer r(opt);
  // Cells computed from the same placement formula the renderer uses.
  auto amy = r.region_at(sim, 81, 25, 31, 14);
  CHECK(amy.has_value());
  CHECK_EQ(*amy, std::string("amygdala"));
  auto pfc = r.region_at(sim, 81, 25, 40, 4);
  CHECK(pfc.has_value());
  CHECK_EQ(*pfc, std::string("prefrontal_cortex"));
  // A corner click is outside the brain silhouette.
  CHECK(!r.region_at(sim, 81, 25, 0, 0).has_value());
}

TEST_CASE("frame layout is consistent with the rendered frame") {
  RenderOptions opt;
  opt.width = 90;
  opt.height = 28;
  opt.ascii_only = true;
  Renderer r(opt);
  auto layout = r.frame_layout();
  CHECK_EQ(layout.view_x, 1);
  CHECK_EQ(layout.view_y, 3);  // after title + separator
  CHECK(layout.view_width > 20);
  CHECK(layout.view_height >= 8);

  // Without the help bar the view is taller but still starts on row 3.
  opt.show_help_bar = false;
  Renderer r2(opt);
  auto l2 = r2.frame_layout();
  CHECK_EQ(l2.view_y, 3);
  CHECK(l2.view_height > layout.view_height);

  // Without the side panel the view spans the full width.
  opt.show_help_bar = true;
  opt.show_chemistry = false;
  Renderer r3(opt);
  auto l3 = r3.frame_layout();
  CHECK_EQ(l3.view_width, 90);
}

TEST_CASE("renderer copes with degenerate sizes and option combinations") {
  Simulation sim = focused_sim();
  // Tiny brain views must not crash and must return the requested rows.
  for (auto wh : std::vector<std::pair<int,int>>{{1, 1}, {3, 4}, {6, 6}, {12, 8}}) {
    RenderOptions o;
    o.ascii_only = true;
    Renderer r(o);
    auto slice = r.render_brain_view(sim, wh.first, wh.second);
    CHECK_EQ(slice.size(), static_cast<std::size_t>(std::max(1, wh.second)));
    RenderOptions o3 = o;
    o3.view = ViewMode::Projection3D;
    Renderer r3(o3);
    CHECK_EQ(r3.render_brain_view(sim, wh.first, wh.second).size(),
             static_cast<std::size_t>(std::max(1, wh.second)));
  }

  // A minimum-ish interactive frame.
  RenderOptions tiny;
  tiny.width = 40;
  tiny.height = 16;
  tiny.ascii_only = true;
  Renderer rt(tiny);
  auto frame = rt.render_frame(sim);
  CHECK(!frame.empty());
  CHECK(frame.size() <= static_cast<std::size_t>(16));

  // No help bar, no chemistry panel: still a valid frame.
  RenderOptions bare;
  bare.width = 70;
  bare.height = 22;
  bare.ascii_only = true;
  bare.show_help_bar = false;
  bare.show_chemistry = false;
  Renderer rb(bare);
  auto bframe = rb.render_frame(sim);
  CHECK(!bframe.empty());
  CHECK(bframe.size() <= static_cast<std::size_t>(22));
}

TEST_CASE("unicode mode actually emits block glyphs; ascii mode never does") {
  Simulation sim = focused_sim();
  auto pure_ascii = [](const std::vector<std::string>& lines) {
    for (const auto& l : lines)
      for (unsigned char c : l)
        if (c >= 0x80) return false;
    return true;
  };

  RenderOptions a;
  a.ascii_only = true;
  Renderer ra(a);
  CHECK(pure_ascii(ra.render_brain_view(sim, 40, 16)));
  CHECK(pure_ascii(ra.render_frame(sim)));

  RenderOptions u;
  u.ascii_only = false;
  Renderer ru(u);
  CHECK(!pure_ascii(ru.render_brain_view(sim, 40, 16)));  // shaded cells use block chars
}

TEST_CASE("region hit-testing rejects out-of-bounds and tiny views") {
  Simulation sim = focused_sim();
  RenderOptions o;
  o.ascii_only = true;
  Renderer r(o);
  CHECK(!r.region_at(sim, 60, 20, -1, 5).has_value());
  CHECK(!r.region_at(sim, 60, 20, 5, -1).has_value());
  CHECK(!r.region_at(sim, 60, 20, 60, 5).has_value());
  CHECK(!r.region_at(sim, 60, 20, 5, 20).has_value());
  CHECK(!r.region_at(sim, 3, 3, 1, 1).has_value());        // view too small
  // A click in the middle of a normal view lands on some region.
  CHECK(r.region_at(sim, 60, 20, 30, 10).has_value());
  // In 3D mode a central click also resolves to a region.
  RenderOptions o3 = o;
  o3.view = ViewMode::Projection3D;
  Renderer r3(o3);
  CHECK(r3.region_at(sim, 60, 20, 30, 10).has_value());
}

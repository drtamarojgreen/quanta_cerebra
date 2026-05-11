#include "gherkin_runner.hpp"

#include "core/atlas_core.h"
#include "core/atlas_region.h"
#include "core/state_manager.h"
#include "visualization/view_2d.h"
#include "ui/interactive_ui.h"
#include "io/atlas_json_parser.h"
#include "core/simulation_engine.h"
#include "ui/visual_themes.h"

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

GIVEN("the built-in atlas is active") {
    cerebra::reset_current_atlas_to_builtin();
}

WHEN("a custom atlas adds the {string} region with display name {string}") {
    cerebra::RegionAtlas a = cerebra::RegionAtlas::builtin();
    cerebra::RegionDefinition def;
    def.id = args[0];
    def.display_name = args[1];
    def.slice_row = 20; def.slice_col = 14; def.slice_w = 10; def.slice_h = 2;
    def.proj_x = 0.10; def.proj_y = -0.55; def.proj_z = -0.55; def.proj_radius = 0.20;
    def.flows.push_back({"glutamate", 0.7});
    a.add_or_replace(def);
    cerebra::set_current_atlas(std::move(a));

    cerebra::BrainFrame frame = cerebra::template_frame(cerebra::BrainTemplate::Focused, 0);
    // Add the new region to the frame at modest intensity so it surfaces in
    // both the table and the slice legend.
    cerebra::RegionState rs;
    rs.region = args[0];
    rs.intensity = 0.5;
    rs.flows = cerebra::default_flows_for(rs.region, rs.intensity);
    frame.regions.push_back(rs);

    const auto& theme = cerebra::theme_by_name("mono");
    ctx.set<std::string>("table", cerebra::render_region_table(frame, 100, theme));
    ctx.set<std::string>("slice", cerebra::render_2d_slice(frame, 100, theme));
}

WHEN("a custom atlas with only {string} is loaded") {
    cerebra::RegionAtlas a;
    cerebra::RegionDefinition def;
    def.id = args[0];
    def.display_name = args[0];
    a.add_or_replace(def);
    cerebra::set_current_atlas(std::move(a));
}

WHEN("a JSON frame supplies a {string} metric of {float} on the {string}") {
    std::ostringstream src;
    src << R"([{"brain_activity":[{"region":")" << args[2]
        << R"(","intensity":0.6,"metrics":{")" << args[0]
        << "\":" << args[1] << "}}],\"timestamp_ms\":0}]";
    auto frames = cerebra::parse_frames(src.str());
    cerebra::Simulation sim(std::move(frames));
    std::ostringstream out;
    cerebra::run_report(sim, "mono", out);
    ctx.set<std::string>("report", out.str());
}

THEN("{string} appears in the rendered region table") {
    const std::string& table = ctx.get<std::string>("table");
    if (table.find(args[0]) == std::string::npos) {
        throw std::runtime_error("missing in table: " + args[0]);
    }
}

THEN("{string} appears in the 2D slice legend") {
    const std::string& slice = ctx.get<std::string>("slice");
    if (slice.find(args[0]) == std::string::npos) {
        throw std::runtime_error("missing in slice legend: " + args[0]);
    }
}

THEN("the current atlas contains {int} region") {
    int n = std::stoi(args[0]);
    BDD_REQUIRE_EQ(static_cast<int>(cerebra::current_atlas().size()), n);
    cerebra::reset_current_atlas_to_builtin();
}

THEN("the rendered region table shows {string} next to {string}") {
    const std::string& report = ctx.get<std::string>("report");
    // Both pieces must be present, and the metric must appear on the same line
    // as the region label.
    std::size_t pos = 0;
    bool found = false;
    while ((pos = report.find(args[1], pos)) != std::string::npos) {
        std::size_t eol = report.find('\n', pos);
        std::string line = report.substr(pos, eol - pos);
        if (line.find(args[0]) != std::string::npos) { found = true; break; }
        pos = eol == std::string::npos ? report.size() : eol + 1;
    }
    if (!found) {
        throw std::runtime_error("metric '" + args[0] + "' not found on the line containing '" + args[1] + "'");
    }
}

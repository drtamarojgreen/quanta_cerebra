#include "gherkin_runner.hpp"

#include "core/atlas_region.h"
#include "core/state_manager.h"
#include "visualization/view_2d.h"
#include "visualization/view_3d.h"
#include "ui/visual_themes.h"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

GIVEN("a {string} template frame at timestamp {int}") {
    const std::string& tmpl = args[0];
    std::int64_t ts = std::stoll(args[1]);
    cerebra::BrainTemplate t;
    if (!cerebra::parse_template(tmpl, t)) {
        throw std::runtime_error("unknown template: " + tmpl);
    }
    ctx.set<cerebra::BrainFrame>("frame", cerebra::template_frame(t, ts));
}

GIVEN("the {string} theme is selected") {
    ctx.set<std::string>("theme_name", args[0]);
}

WHEN("the 2D slice is rendered at width {int}") {
    int width = std::stoi(args[0]);
    const auto& f = ctx.get<cerebra::BrainFrame>("frame");
    const auto& theme = cerebra::theme_by_name(ctx.get<std::string>("theme_name"));
    ctx.set<std::string>("slice", cerebra::render_2d_slice(f, width, theme));
    ctx.set<int>("slice_width", width);
}

WHEN("the 2D slice is rendered again at width {int}") {
    int width = std::stoi(args[0]);
    const auto& f = ctx.get<cerebra::BrainFrame>("frame");
    const auto& theme = cerebra::theme_by_name(ctx.get<std::string>("theme_name"));
    ctx.set<std::string>("slice2", cerebra::render_2d_slice(f, width, theme));
}

WHEN("the 3D projection is rendered at {int} by {int}") {
    int w = std::stoi(args[0]);
    int h = std::stoi(args[1]);
    const auto& f = ctx.get<cerebra::BrainFrame>("frame");
    const auto& theme = cerebra::theme_by_name(ctx.get<std::string>("theme_name"));
    ctx.set<std::string>("projection", cerebra::render_3d_projection(f, w, h, theme));
}

WHEN("the region table is rendered at width {int}") {
    int width = std::stoi(args[0]);
    const auto& f = ctx.get<cerebra::BrainFrame>("frame");
    const auto& theme = cerebra::theme_by_name(ctx.get<std::string>("theme_name"));
    ctx.set<std::string>("table", cerebra::render_region_table(f, width, theme));
}

THEN("the slice output contains every region display name") {
    const std::string& s = ctx.get<std::string>("slice");
    for (const auto& info : cerebra::known_regions()) {
        if (s.find(info.display_name) == std::string::npos) {
            throw std::runtime_error("Missing region label in 2D slice: " + info.display_name);
        }
    }
}

THEN("the projection canvas contains at least {int} characters") {
    int threshold = std::stoi(args[0]);
    const std::string& s = ctx.get<std::string>("projection");
    BDD_REQUIRE(static_cast<int>(s.size()) >= threshold);
}

THEN("the table output contains every region display name") {
    const std::string& s = ctx.get<std::string>("table");
    for (const auto& info : cerebra::known_regions()) {
        if (s.find(info.display_name) == std::string::npos) {
            throw std::runtime_error("Missing region label in table: " + info.display_name);
        }
    }
}

THEN("the wider rendering produces longer output") {
    const std::string& narrow = ctx.get<std::string>("slice");
    const std::string& wide = ctx.get<std::string>("slice2");
    if (wide.size() <= narrow.size()) {
        std::ostringstream oss;
        oss << "wide=" << wide.size() << " bytes, narrow=" << narrow.size() << " bytes";
        throw std::runtime_error(oss.str());
    }
}

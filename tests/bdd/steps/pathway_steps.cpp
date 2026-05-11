#include "gherkin_runner.hpp"

#include "core/atlas_core.h"
#include "core/state_manager.h"
#include "ui/interactive_ui.h"
#include "core/simulation_engine.h"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

WHEN("the simulation runs report mode on the {string} template") {
    cerebra::BrainTemplate t;
    if (!cerebra::parse_template(args[0], t)) {
        throw std::runtime_error("unknown template: " + args[0]);
    }
    std::vector<cerebra::BrainFrame> frames;
    for (int i = 0; i < 3; ++i) {
        frames.push_back(cerebra::template_frame(t, static_cast<std::int64_t>(i) * 100));
    }
    cerebra::Simulation sim(std::move(frames));
    std::ostringstream out;
    cerebra::run_report(sim, "mono", out);
    ctx.set<std::string>("report", out.str());
}

WHEN("a custom atlas adds the {string} pathway from {string} to {string}") {
    cerebra::RegionAtlas a = cerebra::RegionAtlas::builtin();
    cerebra::PathwayDefinition p;
    p.id = args[0];
    p.name = args[0];
    p.nodes = {args[1], args[2]};
    p.strength = 0.5;
    a.add_or_replace_pathway(p);
    cerebra::set_current_atlas(std::move(a));

    auto f = cerebra::template_frame(cerebra::BrainTemplate::Focused, 0);
    cerebra::Simulation sim(std::vector<cerebra::BrainFrame>{f});
    std::ostringstream out;
    cerebra::run_report(sim, "mono", out);
    cerebra::reset_current_atlas_to_builtin();
    ctx.set<std::string>("report", out.str());
}

THEN("the report contains {string}") {
    const std::string& s = ctx.get<std::string>("report");
    if (s.find(args[0]) == std::string::npos) {
        throw std::runtime_error("missing in report: " + args[0]);
    }
}

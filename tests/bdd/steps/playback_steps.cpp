#include "gherkin_runner.hpp"

#include "core/state_manager.h"
#include "core/simulation_engine.h"

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

GIVEN("a simulation with {int} frames loaded from the {string} template") {
    int n = std::stoi(args[0]);
    const std::string& tmpl = args[1];
    cerebra::BrainTemplate t;
    if (!cerebra::parse_template(tmpl, t)) {
        throw std::runtime_error("unknown template: " + tmpl);
    }
    std::vector<cerebra::BrainFrame> frames;
    frames.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        frames.push_back(cerebra::template_frame(t, static_cast<std::int64_t>(i) * 100));
    }
    ctx.set<cerebra::Simulation>("sim", cerebra::Simulation(std::move(frames)));
}

WHEN("the user pauses") {
    ctx.get<cerebra::Simulation>("sim").set_paused(true);
}

WHEN("the user resumes") {
    ctx.get<cerebra::Simulation>("sim").set_paused(false);
}

WHEN("the user fast-forwards by {int} frames") {
    ctx.get<cerebra::Simulation>("sim").advance(std::stoi(args[0]));
}

WHEN("the user rewinds by {int} frames") {
    ctx.get<cerebra::Simulation>("sim").advance(-std::stoi(args[0]));
}

THEN("the simulation is paused") {
    BDD_REQUIRE(ctx.get<cerebra::Simulation>("sim").paused());
}

THEN("the simulation is not paused") {
    BDD_REQUIRE(!ctx.get<cerebra::Simulation>("sim").paused());
}

THEN("the current frame index is {int}") {
    int expected = std::stoi(args[0]);
    std::size_t got = ctx.get<cerebra::Simulation>("sim").index();
    BDD_REQUIRE_EQ(got, static_cast<std::size_t>(expected));
}

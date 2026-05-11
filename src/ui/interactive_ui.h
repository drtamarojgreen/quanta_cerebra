#pragma once

#include "core/bm_simulation_engine.h"
#include "ui/visual_themes.h"

#include <iosfwd>
#include <string>

namespace cerebra {

struct InteractiveOptions {
    std::string initial_theme = "classic";
    bool show_3d = true;
    bool show_2d = true;
    bool mouse = true;
    int tick_ms = 250;
};

struct InteractiveSnapshot {
    std::size_t frame_index = 0;
    bool paused = false;
    int speed = 1;
    std::string theme;
    std::string highlight;
};

int run_interactive(Simulation& sim, const InteractiveOptions& opts);
int run_report(const Simulation& sim, const std::string& theme_name, std::ostream& out);

// Render a single full screen of UI (used by both modes and tests).
std::string render_frame(const Simulation& sim, const InteractiveSnapshot& state,
                         const Theme& theme, int cols, int rows);

}

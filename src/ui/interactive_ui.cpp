#include "ui/interactive_ui.h"
#include "core/atlas_core.h"
#include "visualization/view_2d.h"
#include "visualization/view_3d.h"
#include "ui/terminal_renderer.h"
#include "ui/guided_tour.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <thread>
#include <vector>

namespace cerebra {

namespace {

std::string format_header(const Theme& theme, const InteractiveSnapshot& s,
                          std::size_t frame_count, std::int64_t ts_ms) {
    std::ostringstream out;
    out << ansi(theme.title_color) << "Brain Modeler" << ansi_reset()
        << "  " << ansi(theme.accent_color) << "frame " << (s.frame_index + 1)
        << "/" << frame_count << ansi_reset()
        << "  t=" << std::fixed << std::setprecision(2) << (ts_ms / 1000.0) << "s"
        << "  " << ansi(theme.accent_color)
        << (s.paused ? "PAUSED" : "PLAYING") << ansi_reset()
        << "  speed=" << s.speed << "x"
        << "  theme=" << s.theme;
    if (!s.highlight.empty()) {
        out << "  " << ansi(theme.warn_color) << "[" << s.highlight << "]" << ansi_reset();
    }
    out << '\n';
    return out.str();
}

std::string format_footer(const Theme& theme) {
    std::ostringstream out;
    out << ansi(theme.label_color)
        << "[space] pause  [<-/->] step  [</>] jump  [+/-] speed  "
           "[1-9,0] region  [t] theme  [v] view  [r] tour  [q] quit"
        << ansi_reset() << '\n';
    return out.str();
}

}

std::string render_frame(const Simulation& sim, const InteractiveSnapshot& state,
                         const Theme& theme, int cols, int rows) {
    std::ostringstream out;
    const BrainFrame& f = sim.current();
    out << format_header(theme, state, sim.size(), f.timestamp_ms);
    out << render_2d_slice(f, cols, theme, state.highlight);
    int proj_h = std::max(12, rows - 24);
    int proj_w = std::max(40, cols - 4);
    out << render_3d_projection(f, proj_w, proj_h, theme, 35.0, state.highlight);
    out << render_region_table(f, cols, theme, state.highlight);
    out << render_pathways_table(f, cols, theme, state.highlight);
    out << format_footer(theme);
    return out.str();
}

int run_report(const Simulation& sim, const std::string& theme_name, std::ostream& out) {
    if (sim.empty()) {
        out << "(no frames)\n";
        return 0;
    }
    const Theme& theme = theme_by_name(theme_name);
    TerminalSize ts = terminal_size();
    for (std::size_t i = 0; i < sim.size(); ++i) {
        Simulation s = sim;
        s.set_index(i);
        const BrainFrame& f = s.current();
        out << ansi(theme.title_color) << "Brain Modeler" << ansi_reset()
            << "  " << ansi(theme.accent_color) << "frame " << (i + 1)
            << "/" << sim.size() << ansi_reset()
            << "  t=" << std::fixed << std::setprecision(2)
            << (f.timestamp_ms / 1000.0) << "s"
            << "  theme=" << theme.name << '\n';
        out << render_2d_slice(f, ts.cols, theme);
        int proj_h = std::max(12, ts.rows - 24);
        int proj_w = std::max(40, ts.cols - 4);
        out << render_3d_projection(f, proj_w, proj_h, theme);
        out << render_region_table(f, ts.cols, theme);
        out << render_pathways_table(f, ts.cols, theme);
        out << '\n';
    }
    return 0;
}

namespace {

std::string region_for_key(char c) {
    const auto& regions = current_atlas().regions();
    int idx = -1;
    if (c >= '1' && c <= '9') idx = c - '1';
    else if (c == '0') idx = 9;
    if (idx < 0 || idx >= static_cast<int>(regions.size())) return {};
    return regions[idx].id;
}

std::string region_at_pixel(int x, int y, int cols) {
    // The 2D slice begins on row 3 of output (after header). We approximate
    // by mapping x relative to the slice width back into base coordinates.
    int slice_w = cols - 4;
    if (slice_w <= 0) return {};
    double fx = (x - 2) / static_cast<double>(slice_w);
    double fy = (y - 4) / 22.0;
    if (fx < 0 || fx > 1 || fy < 0 || fy > 1) return {};
    int bx = static_cast<int>(fx * 38);
    int by = static_cast<int>(fy * 22);
    for (const auto& r : known_regions()) {
        if (by >= r.slice_row && by < r.slice_row + r.slice_h &&
            bx >= r.slice_col && bx < r.slice_col + r.slice_w) {
            return r.id;
        }
    }
    return {};
}

}

int run_interactive(Simulation& sim, const InteractiveOptions& opts) {
    if (sim.empty()) {
        std::cerr << "No frames to display.\n";
        return 1;
    }
    if (!stdin_is_tty() || !stdout_is_tty()) {
        std::cerr << "Interactive mode requires a TTY; falling back to report mode.\n";
        return run_report(sim, opts.initial_theme, std::cout);
    }

    RawTerminal raw;
    if (!raw.enable()) {
        std::cerr << "Failed to enter raw mode; using report mode.\n";
        return run_report(sim, opts.initial_theme, std::cout);
    }
    if (opts.mouse) raw.enable_mouse(true);
    std::cout << ansi_hide_cursor();

    InteractiveSnapshot snap;
    snap.theme = opts.initial_theme;
    snap.paused = false;
    snap.speed = 1;

    auto theme_index = [&]() {
        const auto& list = available_themes();
        for (std::size_t i = 0; i < list.size(); ++i) {
            if (list[i].name == snap.theme) return i;
        }
        return std::size_t(0);
    };

    auto last_tick = std::chrono::steady_clock::now();
    bool show_2d = opts.show_2d;
    bool show_3d = opts.show_3d;

    while (true) {
        TerminalSize ts = terminal_size();
        const Theme& theme = theme_by_name(snap.theme);
        sim.set_index(snap.frame_index);

        std::ostringstream out;
        out << ansi_clear_screen();
        out << format_header(theme, snap, sim.size(), sim.current().timestamp_ms);
        if (show_2d) out << render_2d_slice(sim.current(), ts.cols, theme, snap.highlight);
        if (show_3d) {
            int proj_h = std::max(12, ts.rows - (show_2d ? 26 : 8));
            int proj_w = std::max(40, ts.cols - 4);
            out << render_3d_projection(sim.current(), proj_w, proj_h, theme, 35.0, snap.highlight);
        }
        out << render_region_table(sim.current(), ts.cols, theme, snap.highlight);
        out << render_pathways_table(sim.current(), ts.cols, theme, snap.highlight);
        out << format_footer(theme);
        std::cout << out.str() << std::flush;

        InputEvent ev;
        if (raw.poll(ev, opts.tick_ms)) {
            if (ev.key == Key::Escape || (ev.key == Key::Char && (ev.ch == 'q' || ev.ch == 'Q'))) {
                break;
            }
            if (ev.key == Key::Space) snap.paused = !snap.paused;
            else if (ev.key == Key::Left)  sim.advance(-snap.speed);
            else if (ev.key == Key::Right) sim.advance( snap.speed);
            else if (ev.key == Key::Char) {
                char c = ev.ch;
                if (c == '<') sim.advance(-10);
                else if (c == '>') sim.advance(10);
                else if (c == '+' || c == '=') snap.speed = std::min(16, snap.speed + 1);
                else if (c == '-' || c == '_') snap.speed = std::max(1,  snap.speed - 1);
                else if (c == 't' || c == 'T') {
                    const auto& list = available_themes();
                    std::size_t i = (theme_index() + 1) % list.size();
                    snap.theme = list[i].name;
                }
                else if (c == 'v' || c == 'V') { show_3d = !show_3d; }
                else if (c == 's' || c == 'S') { show_2d = !show_2d; }
                else if (c == 'r' || c == 'R') {
                    raw.enable_mouse(false);
                    std::cout << ansi_clear_screen() << ansi_show_cursor();
                    run_tour(std::cout, theme);
                    std::cout << "\n(press any key to return)\n" << std::flush;
                    InputEvent dummy;
                    while (!raw.poll(dummy, 1000)) {}
                    std::cout << ansi_hide_cursor();
                    if (opts.mouse) raw.enable_mouse(true);
                }
                else {
                    std::string id = region_for_key(c);
                    if (!id.empty()) snap.highlight = (snap.highlight == id) ? std::string{} : id;
                }
            }
            else if (ev.key == Key::Mouse && ev.mouse_button == 1) {
                std::string id = region_at_pixel(ev.mouse_x, ev.mouse_y, ts.cols);
                if (!id.empty()) snap.highlight = id;
            }
            snap.frame_index = sim.index();
            continue;
        }

        if (!snap.paused) {
            auto now = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick);
            if (dt.count() >= opts.tick_ms / snap.speed) {
                if (snap.frame_index + 1 < sim.size()) ++snap.frame_index;
                last_tick = now;
            }
        }
    }

    raw.enable_mouse(false);
    std::cout << ansi_show_cursor() << ansi_reset() << "\n";
    return 0;
}

}

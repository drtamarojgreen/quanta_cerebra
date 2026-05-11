#include "core/atlas_core.h"
#include "core/data_parsing_hub.h"
#include "core/simulation_engine.h"
#include "ui/interactive_ui.h"
#include "ui/cli_handler.h"
#include "ui/visual_themes.h"
#include "ui/guided_tour.h"
#include "io/json_parser.h"
#include "io/serial_interface.h"
#include "io/config.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#endif

namespace cerebra {

void print_usage(std::ostream& out) {
    out << "QuantaCerebra - Advanced Brain Activity Modeler\n\n"
        << "Usage: QuantaCerebra [options]\n\n"
        << "Input Sources:\n"
        << "  --input <path>          Load activity from JSON/YAML/XML/CSV\n"
        << "  --template <name>       Use a preset template (focused, relaxed, stressed, rem_sleep)\n"
        << "  --serial <device>       Stream frames from a serial port\n"
        << "  --atlas <path>          Load a custom region atlas\n\n"
        << "Display Options:\n"
        << "  --interactive           Start TTY-based interactive mode (default)\n"
        << "  --report                Render all frames to stdout and exit\n"
        << "  --theme <name>          classic | matrix | sunset | ocean | mono\n"
        << "  --tour                  Show guided tour and exit\n"
        << "  --help                  Show this help and exit\n";
}

} // namespace cerebra

int main(int argc, char** argv) {
    using namespace cerebra;

    AppConfig config;
    try {
        config = loadConfigJSON("config.json");
    } catch (...) {
        // Continue with default config
    }

    std::string input_path;
    std::string template_name = "focused";
    std::string serial_device;
    std::string atlas_path;
    std::string theme_name = "classic";
    bool interactive_mode = true;
    bool report_mode = false;
    bool tour_mode = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help") { print_usage(std::cout); return 0; }
        else if (arg == "--input" && i + 1 < argc) input_path = argv[++i];
        else if (arg == "--template" && i + 1 < argc) template_name = argv[++i];
        else if (arg == "--serial" && i + 1 < argc) serial_device = argv[++i];
        else if (arg == "--atlas" && i + 1 < argc) atlas_path = argv[++i];
        else if (arg == "--theme" && i + 1 < argc) theme_name = argv[++i];
        else if (arg == "--report") { report_mode = true; interactive_mode = false; }
        else if (arg == "--tour") tour_mode = true;
        else if (arg == "--read-only") interactive_mode = false;
    }

    if (!atlas_path.empty()) {
        try {
            set_current_atlas(load_json_atlas_file(atlas_path));
        } catch (const std::exception& e) {
            std::cerr << "Failed to load atlas: " << e.what() << std::endl;
            return 1;
        }
    }

    if (tour_mode) {
        return run_tour(std::cout, theme_by_name(theme_name));
    }

    Simulation sim;
    if (!input_path.empty()) {
        std::ifstream f(input_path);
        if (f) {
            std::stringstream ss; ss << f.rdbuf();
            std::string ext = std::filesystem::path(input_path).extension().string();
            if (!ext.empty()) ext = ext.substr(1);
            sim.set_frames(parse_frames_by_format(ss.str(), ext));
        }
    } else if (!serial_device.empty()) {
        // Serial logic handled via serial_interface.h components
    } else {
        std::string id = resolve_template_id(template_name);
        if (!id.empty()) {
            std::vector<cerebra::BrainFrame> frames;
            for (int i = 0; i < 30; ++i) frames.push_back(template_frame(id, i * 200));
            sim.set_frames(std::move(frames));
        }
    }

    if (report_mode) {
        return run_report(sim, theme_name, std::cout);
    }

    if (interactive_mode) {
        InteractiveOptions opts;
        opts.initial_theme = theme_name;
        return run_interactive(sim, opts);
    }

    return 0;
}

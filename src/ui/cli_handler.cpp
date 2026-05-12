#include "ui/cli_handler.h"

#include <algorithm>
#include <sstream>

#include <stdexcept>

#include "core/state_manager.h"
#include "ui/color_theme.hpp"
#include "core/neurochemistry.h"
#include "core/pathway_logic.h"
#include "core/atlas_region.h"

namespace cerebra {
namespace {

std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}

bool parse_int(const std::string& s, int& out) {
  try {
    std::size_t idx = 0;
    int v = std::stoi(s, &idx);
    if (idx != s.size()) return false;
    out = v;
    return true;
  } catch (...) {
    return false;
  }
}

bool parse_double(const std::string& s, double& out) {
  try {
    std::size_t idx = 0;
    double v = std::stod(s, &idx);
    if (idx != s.size()) return false;
    out = v;
    return true;
  } catch (...) {
    return false;
  }
}

CliOptions make_exit(int code, std::string message) {
  CliOptions o;
  o.should_exit = true;
  o.exit_code = code;
  o.message = std::move(message);
  return o;
}

std::string list_states() {
  std::ostringstream os;
  os << "Brain-state presets:\n";
  for (const auto& s : BrainStateLibrary::all()) {
    os << "  " << s.key << "  - " << s.display_name;
    if (!s.description.empty()) os << ": " << s.description;
    for (const auto& kv : s.extra) os << "  " << kv.first << "=" << kv.second;
    os << "\n";
  }
  return os.str();
}

std::string list_transmitters() {
  std::ostringstream os;
  os << "Neurotransmitters:\n";
  for (const auto& nt : Neurochemistry::catalog()) {
    os << "  " << nt.key << " (" << nt.symbol << ")  - " << nt.display_name
       << "  baseline " << nt.baseline << "  release " << nt.release_gain
       << "  reuptake " << nt.reuptake_rate;
    for (const auto& kv : nt.extra) os << "  " << kv.first << "=" << kv.second;
    os << "\n";
  }
  return os.str();
}

std::string list_regions() {
  std::ostringstream os;
  os << "Modelled regions (ROI = region of interest):\n";
  for (const auto& r : RegionCatalog::all()) {
    os << "  " << r.key << " (" << r.abbreviation << ")"
       << (r.region_of_interest ? "  [ROI]" : "") << "  primary: " << r.primary_transmitter << "\n";
  }
  return os.str();
}

std::string list_themes() {
  std::ostringstream os;
  os << "Color themes:\n";
  for (const auto& t : ThemeLibrary::all(true)) {
    os << "  " << t.key() << "  - " << t.display_name() << "\n";
  }
  return os.str();
}

std::string list_pathways() {
  std::ostringstream os;
  if (PathwayCatalog::all().empty()) {
    os << "No pathways are loaded (use --pathways <file>; see data/pathways.json).\n";
    return os.str();
  }
  os << "Pathways:\n";
  for (const auto& p : PathwayCatalog::all()) {
    os << "  " << p.label() << "  weight " << p.weight << "  delay " << p.delay_ms << "ms  "
       << to_string(p.kind);
    if (!p.transmitter.empty()) os << "  (" << p.transmitter << ")";
    os << "\n";
  }
  return os.str();
}

}  // namespace

std::string Cli::version_string() { return "Brain Modeler 0.1.0"; }

std::string Cli::usage(const std::string& program_name) {
  std::ostringstream os;
  os << version_string() << "\n\n"
     << "Usage: " << program_name << " [options]\n\n"
     << "Input (choose at most one):\n"
     << "  -i, --input <file>        load a JSON array-of-frames document\n"
     << "      --state <preset>      synthesize a timeline from a brain-state preset\n"
     << "                            (focused, relaxed, stressed, rem_sleep)\n"
     << "      --serial <device>     stream live frames from a serial device\n"
     << "      --baud <rate>         serial baud rate (default 115200)\n"
     << "      --memory-serial       use a simulated in-memory device (demo/testing)\n\n"
     << "Configuration (load these in order if you use more than one):\n"
     << "      --neurotransmitters <file>  load the neurotransmitter catalog from JSON\n"
     << "                            (see data/neurotransmitters.json)\n"
     << "      --regions <file>      load the brain-region catalog from a JSON file\n"
     << "                            (see data/regions.json for the schema)\n"
     << "      --pathways <file>     load a pathway (connectivity) catalog from JSON\n"
     << "                            (see data/pathways.json for the schema)\n"
     << "      --states <file>       load brain-state presets from a JSON file\n"
     << "                            (see data/brain_states.json)\n\n"
     << "Display:\n"
     << "      --show-pathways       overlay configured pathways on the brain view\n"
     << "      --format <mode>       'interactive' (default on a TTY) or 'report'\n"
     << "      --view <2d|3d>        brain view: coronal slice or 3D projection\n"
     << "      --theme <name>        color theme (see --list-themes)\n"
     << "      --no-color            disable ANSI color\n"
     << "      --ascii               restrict glyphs to plain ASCII\n"
     << "      --fps <n>             playback frames per second at 1x speed (default 10)\n"
     << "      --region <name>       preselect a region for the stats panel\n"
     << "      --width <cols>        override detected terminal width\n"
     << "      --height <rows>       override detected terminal height\n"
     << "      --max-seconds <n>     auto-exit interactive mode after n seconds\n"
     << "      --tour                start the interactive guided tour\n\n"
     << "Info:\n"
     << "      --list-states         list brain-state presets and exit\n"
     << "      --list-regions        list modelled regions and exit\n"
     << "      --list-transmitters   list modelled neurotransmitters and exit\n"
     << "      --list-pathways       list loaded pathways and exit\n"
     << "      --list-themes         list color themes and exit\n"
     << "  -h, --help                show this help and exit\n"
     << "      --version             show version and exit\n";
  return os.str();
}

CliOptions Cli::parse(const std::vector<std::string>& argv, bool stdout_is_tty) {
  CliOptions opt;
  opt.format = stdout_is_tty ? OutputFormat::Interactive : OutputFormat::Report;

  // Tolerate a leading program name.
  std::vector<std::string> args = argv;
  std::string prog = "brain_modeler";
  if (!args.empty() && !args.front().empty() && args.front()[0] != '-') {
    // Heuristic: if it looks like a path/executable name, treat as prog name.
    // (Real subcommands aren't used by this tool.)
    if (args.front().find('/') != std::string::npos || args.front().find("brain") != std::string::npos) {
      prog = args.front();
      args.erase(args.begin());
    }
  }

  bool input_set = false;
  auto need_value = [&](std::size_t& i) -> std::optional<std::string> {
    if (i + 1 >= args.size()) return std::nullopt;
    return args[++i];
  };

  for (std::size_t i = 0; i < args.size(); ++i) {
    std::string a = args[i];
    auto eq = a.find('=');
    std::optional<std::string> inline_val;
    if (a.rfind("--", 0) == 0 && eq != std::string::npos) {
      inline_val = a.substr(eq + 1);
      a = a.substr(0, eq);
    }
    auto value = [&](const std::string&) -> std::optional<std::string> {
      if (inline_val) return inline_val;
      return need_value(i);
    };

    if (a == "-h" || a == "--help") return make_exit(0, usage(prog));
    if (a == "--version") return make_exit(0, version_string() + "\n");
    if (a == "--list-states") return make_exit(0, list_states());
    if (a == "--list-regions") return make_exit(0, list_regions());
    if (a == "--list-transmitters") return make_exit(0, list_transmitters());
    if (a == "--list-pathways") return make_exit(0, list_pathways());
    if (a == "--list-themes") return make_exit(0, list_themes());

    if (a == "--format") {
      auto v = value("--format");
      if (!v) return make_exit(2, "error: --format requires a value\n");
      std::string m = lower(*v);
      if (m == "interactive" || m == "tui") opt.format = OutputFormat::Interactive;
      else if (m == "report" || m == "text" || m == "batch") opt.format = OutputFormat::Report;
      else return make_exit(2, "error: --format must be 'interactive' or 'report'\n");
      opt.format_explicit = true;
    } else if (a == "-i" || a == "--input") {
      auto v = value("--input");
      if (!v) return make_exit(2, "error: --input requires a file path\n");
      if (input_set) return make_exit(2, "error: choose only one input source\n");
      opt.input.kind = InputKind::JsonFile;
      opt.input.path = *v;
      input_set = true;
    } else if (a == "--state") {
      auto v = value("--state");
      if (!v) return make_exit(2, "error: --state requires a preset name\n");
      if (!BrainStateLibrary::find(*v)) {
        return make_exit(2, "error: unknown brain-state preset '" + *v + "' (see --list-states)\n");
      }
      if (opt.input.kind == InputKind::Serial) {
        // With a (real or simulated) serial source, --state picks the preset
        // the device should emit / be asked for, rather than a new source.
        opt.input.state_key = *v;
      } else {
        if (input_set) return make_exit(2, "error: choose only one input source\n");
        opt.input.kind = InputKind::BrainState;
        opt.input.state_key = *v;
        input_set = true;
      }
    } else if (a == "--serial") {
      auto v = value("--serial");
      if (!v) return make_exit(2, "error: --serial requires a device path\n");
      if (input_set) return make_exit(2, "error: choose only one input source\n");
      opt.input.kind = InputKind::Serial;
      opt.input.serial.device = *v;
      input_set = true;
    } else if (a == "--baud") {
      auto v = value("--baud");
      int b = 0;
      if (!v || !parse_int(*v, b) || b <= 0) return make_exit(2, "error: --baud requires a positive integer\n");
      opt.input.serial.baud_rate = b;
    } else if (a == "--memory-serial") {
      opt.use_memory_serial = true;
      opt.input.use_memory_port = true;
      if (opt.input.kind != InputKind::Serial) {
        opt.input.kind = InputKind::Serial;
        if (opt.input.serial.device.empty()) opt.input.serial.device = "memory0";
        input_set = true;
      }
    } else if (a == "--serial-preset") {
      auto v = value("--serial-preset");
      if (!v) return make_exit(2, "error: --serial-preset requires a preset name\n");
      opt.input.state_key = *v;
    } else if (a == "--neurotransmitters") {
      auto v = value("--neurotransmitters");
      if (!v) return make_exit(2, "error: --neurotransmitters requires a file path\n");
      // Apply first so region/pathway validation sees the configured catalog.
      try {
        Neurochemistry::load_from_file(*v);
      } catch (const std::exception& e) {
        return make_exit(2, std::string("error: --neurotransmitters: ") + e.what() + "\n");
      }
      opt.neurotransmitters_file = *v;
    } else if (a == "--regions") {
      auto v = value("--regions");
      if (!v) return make_exit(2, "error: --regions requires a file path\n");
      // Apply immediately so the rest of this run (including a later
      // --list-regions, --pathways or --states) sees the configured catalog.
      try {
        RegionCatalog::load_from_file(*v);
      } catch (const std::exception& e) {
        return make_exit(2, std::string("error: --regions: ") + e.what() + "\n");
      }
      opt.regions_file = *v;
    } else if (a == "--pathways") {
      auto v = value("--pathways");
      if (!v) return make_exit(2, "error: --pathways requires a file path\n");
      try {
        PathwayCatalog::load_from_file(*v);
      } catch (const std::exception& e) {
        return make_exit(2, std::string("error: --pathways: ") + e.what() + "\n");
      }
      opt.pathways_file = *v;
      opt.show_pathways = true;  // loading pathways implies wanting to see them
    } else if (a == "--states") {
      auto v = value("--states");
      if (!v) return make_exit(2, "error: --states requires a file path\n");
      try {
        BrainStateLibrary::load_from_file(*v);
      } catch (const std::exception& e) {
        return make_exit(2, std::string("error: --states: ") + e.what() + "\n");
      }
      opt.states_file = *v;
    } else if (a == "--show-pathways") {
      opt.show_pathways = true;
    } else if (a == "--view") {
      auto v = value("--view");
      if (!v) return make_exit(2, "error: --view requires '2d' or '3d'\n");
      std::string m = lower(*v);
      if (m == "2d" || m == "slice" || m == "2") opt.view = ViewMode::Slice2D;
      else if (m == "3d" || m == "projection" || m == "3") opt.view = ViewMode::Projection3D;
      else return make_exit(2, "error: --view must be '2d' or '3d'\n");
    } else if (a == "--theme") {
      auto v = value("--theme");
      if (!v) return make_exit(2, "error: --theme requires a name\n");
      auto keys = ThemeLibrary::keys();
      if (std::find(keys.begin(), keys.end(), *v) == keys.end()) {
        return make_exit(2, "error: unknown theme '" + *v + "' (see --list-themes)\n");
      }
      opt.theme_key = *v;
    } else if (a == "--no-color") {
      opt.force_no_color = true;
    } else if (a == "--ascii") {
      opt.force_ascii = true;
    } else if (a == "--fps") {
      auto v = value("--fps");
      double f = 0;
      if (!v || !parse_double(*v, f) || f <= 0) return make_exit(2, "error: --fps requires a positive number\n");
      opt.fps = f;
      opt.input.synth_step_ms = static_cast<std::int64_t>(1000.0 / f);
      if (opt.input.synth_step_ms < 1) opt.input.synth_step_ms = 1;
    } else if (a == "--frames") {
      auto v = value("--frames");
      int n = 0;
      if (!v || !parse_int(*v, n) || n <= 0) return make_exit(2, "error: --frames requires a positive integer\n");
      opt.input.synth_frames = n;
    } else if (a == "--step-ms") {
      auto v = value("--step-ms");
      int n = 0;
      if (!v || !parse_int(*v, n) || n <= 0) return make_exit(2, "error: --step-ms requires a positive integer\n");
      opt.input.synth_step_ms = n;
    } else if (a == "--region") {
      auto v = value("--region");
      if (!v) return make_exit(2, "error: --region requires a name\n");
      opt.preselect_region = RegionCatalog::normalize_key(*v);
    } else if (a == "--width") {
      auto v = value("--width");
      int n = 0;
      if (!v || !parse_int(*v, n) || n < 20) return make_exit(2, "error: --width requires an integer >= 20\n");
      opt.width_override = n;
    } else if (a == "--height") {
      auto v = value("--height");
      int n = 0;
      if (!v || !parse_int(*v, n) || n < 8) return make_exit(2, "error: --height requires an integer >= 8\n");
      opt.height_override = n;
    } else if (a == "--max-seconds") {
      auto v = value("--max-seconds");
      int n = 0;
      if (!v || !parse_int(*v, n) || n <= 0) return make_exit(2, "error: --max-seconds requires a positive integer\n");
      opt.report_max_seconds = n;
    } else if (a == "--tour") {
      opt.start_tour = true;
    } else {
      return make_exit(2, "error: unrecognized option '" + args[i] + "'\n\n" + usage(prog));
    }
  }
  return opt;
}

}  // namespace cerebra

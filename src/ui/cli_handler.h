#ifndef BRAIN_MODELER_CLI_HPP
#define BRAIN_MODELER_CLI_HPP

#include <optional>
#include <string>
#include <vector>

#include "io/input_source.hpp"
#include "visualization/scene_renderer.h"

namespace cerebra {

enum class OutputFormat { Interactive, Report };

struct CliOptions {
  OutputFormat format = OutputFormat::Interactive;
  bool format_explicit = false;     // user passed --format

  InputSpec input;
  std::optional<std::string> neurotransmitters_file;  // --neurotransmitters
  std::optional<std::string> regions_file;            // --regions
  std::optional<std::string> pathways_file;           // --pathways
  std::optional<std::string> states_file;             // --states (brain-state presets)
  bool show_pathways = false;                          // --show-pathways

  ViewMode view = ViewMode::Slice2D;
  std::string theme_key;            // empty -> default
  bool force_no_color = false;
  bool force_ascii = false;
  double fps = 10.0;
  bool start_tour = false;
  std::optional<std::string> preselect_region;
  std::optional<int> width_override;
  std::optional<int> height_override;
  std::optional<int> report_max_seconds;  // wall-clock cap for interactive runs
  bool use_memory_serial = false;   // testing/demo: simulate a device

  // When set, main() should print `message` and exit with `exit_code` instead
  // of running the app (covers --help, --version, --list-*, and parse errors).
  bool should_exit = false;
  int exit_code = 0;
  std::string message;
};

class Cli {
public:
  // Parse argv (excluding the program name is fine too; the parser tolerates a
  // leading program name). On --help/--version/--list-*/errors, fills in
  // should_exit/exit_code/message.
  static CliOptions parse(const std::vector<std::string>& args, bool stdout_is_tty);

  static std::string usage(const std::string& program_name = "brain_modeler");
  static std::string version_string();
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_CLI_HPP

#include "ui/app.hpp"

#include <iostream>

#include "ui/cli_handler.h"
#include "interactive_ui.h"
#include "ui/report_ui.hpp"
#include "ui/terminal_renderer.h"

namespace cerebra {

int App::run(const std::vector<std::string>& args) {
  bool stdout_tty = cerebra::stdout_is_tty();
  CliOptions opts = Cli::parse(args, stdout_tty);
  if (opts.should_exit) {
    if (!opts.message.empty()) {
      if (opts.exit_code == 0) std::cout << opts.message;
      else std::cerr << opts.message;
    }
    return opts.exit_code;
  }

  if (opts.format == OutputFormat::Report) {
    // Simplified report mode call
    Simulation sim;
    return run_report(sim, opts.theme_key, std::cout);
  }

  Simulation sim;
  InteractiveOptions iopts;
  iopts.initial_theme = opts.theme_key;
  return run_interactive(sim, iopts);
}

}  // namespace cerebra

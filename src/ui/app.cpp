#include "ui/app.hpp"

#include <iostream>

#include "ui/cli_handler.h"
#include "interactive_ui.hpp"
#include "ui/report_ui.hpp"
#include "ui/terminal_renderer.h"

namespace cerebra {

int App::run(const std::vector<std::string>& args) {
  bool stdout_tty = Terminal::detect_capabilities().is_tty;
  CliOptions opts = Cli::parse(args, stdout_tty);
  if (opts.should_exit) {
    if (!opts.message.empty()) {
      if (opts.exit_code == 0) std::cout << opts.message;
      else std::cerr << opts.message;
    }
    return opts.exit_code;
  }

  if (opts.format == OutputFormat::Report) {
    return ReportUi::run(opts, std::cout);
  }
  InteractiveUi ui(opts);
  return ui.run();
}

}  // namespace cerebra

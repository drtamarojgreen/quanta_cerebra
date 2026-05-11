#ifndef BRAIN_MODELER_REPORT_UI_HPP
#define BRAIN_MODELER_REPORT_UI_HPP

#include <ostream>
#include <string>

#include "ui/cli_handler.h"
#include "core/bm_simulation_engine.h"

namespace cerebra {

// Non-interactive output: render the whole simulation as a text report, print
// it, and return. Used for --format report (and when stdout is not a TTY).
class ReportUi {
public:
  static int run(const CliOptions& options, std::ostream& out);

  // Build the report text for an already-loaded simulation (exposed for tests).
  static std::string build(const Simulation& sim, const CliOptions& options);
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_REPORT_UI_HPP

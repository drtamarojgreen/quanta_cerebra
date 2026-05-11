#ifndef BRAIN_MODELER_APP_HPP
#define BRAIN_MODELER_APP_HPP

#include <string>
#include <vector>

namespace cerebra {

// Top-level entry point: parses arguments and dispatches to the interactive UI
// or the report writer. Returns a process exit code.
class App {
public:
  static int run(const std::vector<std::string>& args);
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_APP_HPP

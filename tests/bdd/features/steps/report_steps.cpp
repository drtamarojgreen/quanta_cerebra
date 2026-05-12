// Step definitions for features/report_mode.feature.

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "gherkin.hpp"
#include "../../../test_config.h"

namespace {

void produce_report(bdd::World& w) {
  std::ostringstream os;
  w.exit_code = cerebra::ReportUi::run(w.opts, os);
  w.report_text = os.str();
}

std::string report_line_starting_with(const std::string& report, const std::string& prefix) {
  std::istringstream in(report);
  std::string line;
  while (std::getline(in, line))
    if (line.rfind(prefix, 0) == 0) return line;
  return ";
}

}  // namespace

GIVEN("a temporary JSON activity file with {int} frames") {
  int n = bdd::arg_int(args, 0);
  std::string path = cerebra::test::temp_path("bm_bdd_report_feature.json");
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out << "[";
  for (int i = 0; i < n; ++i) {
    if (i) out << ",";
    double pfc = 0.80 - 0.05 * i;
    double amy = 0.55 + 0.05 * i;
    if (pfc < 0.0) pfc = 0.0;
    if (amy > 1.0) amy = 1.0;
    out << "{\"brain_activity\":[{\"region\":\"prefrontal_cortex\",\"intensity\":" << pfc
        << "},{\"region\":\"amygdala\",\"intensity\":" << amy
        << "},{\"region\":\"thalamus\",\"intensity\":0.5}],\"timestamp_ms\":" << (i * 100) << "}";
  }
  out << "]";
  out.close();
  world.temp_path = path;

  world.opts = cerebra::CliOptions{};
  world.opts.format = cerebra::OutputFormat::Report;
  world.opts.input.kind = cerebra::InputKind::JsonFile;
  world.opts.input.path = path;
  world.opts.force_ascii = true;
  world.opts.width_override = 80;
}

GIVEN("report options for the {string} preset in {word} view with {int} frames") {
  world.opts = cerebra::CliOptions{};
  world.opts.format = cerebra::OutputFormat::Report;
  world.opts.input.kind = cerebra::InputKind::BrainState;
  world.opts.input.state_key = bdd::arg_str(args, 0);
  std::string view = bdd::arg_str(args, 1);
  world.opts.view = (view == "3d" || view == "3D") ? cerebra::ViewMode::Projection3D
                                                   : cerebra::ViewMode::Slice2D;
  world.opts.input.synth_frames = bdd::arg_int(args, 2);
  world.opts.force_ascii = true;
  world.opts.width_override = 72;
}

GIVEN("report options for the input file {string}") {
  world.opts = cerebra::CliOptions{};
  world.opts.format = cerebra::OutputFormat::Report;
  world.opts.input.kind = cerebra::InputKind::JsonFile;
  world.opts.input.path = bdd::arg_str(args, 0);
}

WHEN("the tool runs in report mode against that file") { produce_report(world); }

WHEN("the report is produced") { produce_report(world); }

THEN("the run exits successfully") { CHECK_EQ(world.exit_code, 0); }

THEN("the run exits with a non-zero code") { CHECK(world.exit_code != 0); }

THEN("the report contains {string}") {
  const std::string& needle = bdd::arg_str(args, 0);
  CHECK_MSG(world.report_text.find(needle) != std::string::npos,
            "report does not contain: " + needle);
}

THEN("the report contains only ASCII bytes") {
  for (unsigned char c : world.report_text)
    CHECK_MSG(c < 0x80, "report contains a non-ASCII byte");
  CHECK_MSG(world.report_text.find('\x1b') == std::string::npos, "report contains an ANSI escape");
}

THEN("every report line is at most {int} columns wide") {
  std::size_t limit = static_cast<std::size_t>(bdd::arg_int(args, 0));
  std::istringstream in(world.report_text);
  std::string line;
  while (std::getline(in, line))
    CHECK_MSG(line.size() <= limit, "report line exceeds " + std::to_string(limit) + ": " + line);
}

THEN("the report flags region {string} as a region of interest") {
  std::string line = report_line_starting_with(world.report_text, bdd::arg_str(args, 0));
  CHECK_MSG(!line.empty(), "no report row for region " + bdd::arg_str(args, 0));
  CHECK_MSG(line.find("yes") != std::string::npos,
            "region " + bdd::arg_str(args, 0) + " not flagged ROI: " + line);
}

THEN("the report flags region {string} as not a region of interest") {
  std::string line = report_line_starting_with(world.report_text, bdd::arg_str(args, 0));
  CHECK_MSG(!line.empty(), "no report row for region " + bdd::arg_str(args, 0));
  CHECK_MSG(line.find(" no ") != std::string::npos,
            "region " + bdd::arg_str(args, 0) + " unexpectedly flagged ROI: " + line);
}

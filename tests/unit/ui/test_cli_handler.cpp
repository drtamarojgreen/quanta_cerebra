#include <cstdio>
#include <fstream>

#include "../test_framework.hpp"
#include "../../test_config.h"
#include "core/state_manager.h"
#include "ui/cli_handler.h"
#include "core/neurochemistry.h"
#include "core/pathway_logic.h"
#include "core/region.hpp"

using namespace cerebra;

namespace {
CliOptions parse(std::vector<std::string> args, bool tty = true) {
  return Cli::parse(args, tty);
}
bool contains(const std::string& s, const std::string& n) { return s.find(n) != std::string::npos; }
}  // namespace

TEST_CASE("defaults depend on whether stdout is a tty") {
  CHECK(parse({}, true).format == OutputFormat::Interactive);
  CHECK(parse({}, false).format == OutputFormat::Report);
}

TEST_CASE("format flag overrides the default") {
  auto a = parse({"--format", "report"}, true);
  CHECK(a.format == OutputFormat::Report);
  CHECK(a.format_explicit);
  auto b = parse({"--format=interactive"}, false);
  CHECK(b.format == OutputFormat::Interactive);
  auto bad = parse({"--format", "wibble"});
  CHECK(bad.should_exit);
  CHECK(bad.exit_code == 2);
}

TEST_CASE("input sources are mutually exclusive") {
  auto a = parse({"--input", "foo.json"});
  CHECK(a.input.kind == InputKind::JsonFile);
  CHECK_EQ(a.input.path, std::string("foo.json"));

  auto b = parse({"--state", "focused"});
  CHECK(b.input.kind == InputKind::BrainState);
  CHECK_EQ(b.input.state_key, std::string("focused"));

  auto c = parse({"--serial", "/dev/ttyUSB0", "--baud", "57600"});
  CHECK(c.input.kind == InputKind::Serial);
  CHECK_EQ(c.input.serial.device, std::string("/dev/ttyUSB0"));
  CHECK_EQ(c.input.serial.baud_rate, 57600);

  auto clash = parse({"--input", "foo.json", "--state", "relaxed"});
  CHECK(clash.should_exit);
  CHECK(clash.exit_code == 2);

  auto bad_state = parse({"--state", "nope"});
  CHECK(bad_state.should_exit);
}

TEST_CASE("view, theme, color and fps options") {
  auto a = parse({"--view", "3d", "--theme", "matrix", "--no-color", "--ascii", "--fps", "20"});
  CHECK(a.view == ViewMode::Projection3D);
  CHECK_EQ(a.theme_key, std::string("matrix"));
  CHECK(a.force_no_color);
  CHECK(a.force_ascii);
  CHECK_NEAR(a.fps, 20.0, 1e-9);

  auto bad_theme = parse({"--theme", "ultraviolet"});
  CHECK(bad_theme.should_exit);
  auto bad_view = parse({"--view", "7d"});
  CHECK(bad_view.should_exit);
  auto bad_fps = parse({"--fps", "0"});
  CHECK(bad_fps.should_exit);
}

TEST_CASE("memory serial implies a serial source") {
  auto a = parse({"--memory-serial"});
  CHECK(a.use_memory_serial);
  CHECK(a.input.kind == InputKind::Serial);
  CHECK(a.input.use_memory_port);
}

TEST_CASE("tour, region preselect and dimension overrides") {
  auto a = parse({"--tour", "--region", "Amygdala", "--width", "120", "--height", "40",
                  "--max-seconds", "3"});
  CHECK(a.start_tour);
  CHECK(a.preselect_region.has_value());
  CHECK_EQ(*a.preselect_region, std::string("amygdala"));
  CHECK(a.width_override.has_value());
  CHECK_EQ(*a.width_override, 120);
  CHECK_EQ(*a.height_override, 40);
  CHECK_EQ(*a.report_max_seconds, 3);

  CHECK(parse({"--width", "5"}).should_exit);
}

TEST_CASE("help, version and list commands exit cleanly") {
  auto h = parse({"--help"});
  CHECK(h.should_exit);
  CHECK_EQ(h.exit_code, 0);
  CHECK(contains(h.message, "Usage"));

  auto v = parse({"--version"});
  CHECK(v.should_exit);
  CHECK(contains(v.message, "Brain Modeler"));

  auto ls = parse({"--list-states"});
  CHECK(ls.should_exit);
  CHECK(contains(ls.message, "focused"));
  CHECK(contains(ls.message, "rem_sleep"));

  auto lr = parse({"--list-regions"});
  CHECK(contains(lr.message, "amygdala"));
  CHECK(contains(lr.message, "ROI"));

  auto lt = parse({"--list-themes"});
  CHECK(contains(lt.message, "matrix"));
}

TEST_CASE("unknown options are rejected with usage") {
  auto a = parse({"--frobnicate"});
  CHECK(a.should_exit);
  CHECK_EQ(a.exit_code, 2);
  CHECK(contains(a.message, "unrecognized option"));
}

TEST_CASE("a leading program name is tolerated") {
  auto a = parse({"./brain_modeler", "--state", "relaxed"});
  CHECK(!a.should_exit);
  CHECK(a.input.kind == InputKind::BrainState);
  auto b = parse({"/usr/local/bin/brain_modeler", "--version"});
  CHECK(b.should_exit);
  CHECK(contains(b.message, "Brain Modeler"));
}

TEST_CASE("the --flag=value form works for every value-taking option") {
  auto a = parse({"--format=report", "--view=3d", "--theme=amber", "--fps=15",
                  "--frames=24", "--step-ms=20", "--region=Amygdala", "--width=120",
                  "--height=40", "--max-seconds=5", "--state=relaxed", "--baud=57600"});
  CHECK(!a.should_exit);
  CHECK(a.format == OutputFormat::Report);
  CHECK(a.view == ViewMode::Projection3D);
  CHECK_EQ(a.theme_key, std::string("amber"));
  CHECK_NEAR(a.fps, 15.0, 1e-9);
  CHECK_EQ(a.input.synth_frames, 24);
  CHECK_EQ(*a.preselect_region, std::string("amygdala"));
  CHECK_EQ(*a.width_override, 120);
  CHECK_EQ(*a.height_override, 40);
  CHECK_EQ(*a.report_max_seconds, 5);
  CHECK(a.input.kind == InputKind::BrainState);
  CHECK_EQ(a.input.state_key, std::string("relaxed"));
}

TEST_CASE("fps sets the synthesized frame step, and --step-ms can override it") {
  auto a = parse({"--state", "focused", "--fps", "20"});
  CHECK_EQ(a.input.synth_step_ms, static_cast<std::int64_t>(50));  // 1000/20
  auto b = parse({"--state", "focused", "--fps", "20", "--step-ms", "13"});
  CHECK_EQ(b.input.synth_step_ms, static_cast<std::int64_t>(13));  // explicit wins (comes later)
}

TEST_CASE("--state combines with a serial source in either order") {
  // memory-serial first, then --state -> --state picks the device preset.
  auto a = parse({"--memory-serial", "--state", "stressed"});
  CHECK(!a.should_exit);
  CHECK(a.input.kind == InputKind::Serial);
  CHECK(a.use_memory_serial);
  CHECK(a.input.use_memory_port);
  CHECK_EQ(a.input.state_key, std::string("stressed"));

  // --state first, then memory-serial -> still a serial source, preset kept.
  auto b = parse({"--state", "stressed", "--memory-serial"});
  CHECK(!b.should_exit);
  CHECK(b.input.kind == InputKind::Serial);
  CHECK_EQ(b.input.state_key, std::string("stressed"));

  // --serial-preset names the preset to request from a real device.
  auto c = parse({"--serial", "/dev/ttyUSB0", "--serial-preset", "relaxed"});
  CHECK(!c.should_exit);
  CHECK(c.input.kind == InputKind::Serial);
  CHECK_EQ(c.input.serial.device, std::string("/dev/ttyUSB0"));
  CHECK_EQ(c.input.state_key, std::string("relaxed"));
}

TEST_CASE("value-taking options reject missing or invalid arguments") {
  CHECK(parse({"--format"}).should_exit);
  CHECK(parse({"--input"}).should_exit);
  CHECK(parse({"--baud", "abc"}).should_exit);
  CHECK(parse({"--baud", "-1"}).should_exit);
  CHECK(parse({"--fps", "nope"}).should_exit);
  CHECK(parse({"--frames", "0"}).should_exit);
  CHECK(parse({"--step-ms", "-5"}).should_exit);
  CHECK(parse({"--height", "2"}).should_exit);
  CHECK(parse({"--max-seconds", "0"}).should_exit);
  for (const auto& bad : {parse({"--format"}), parse({"--baud", "abc"}), parse({"--fps", "x"})}) {
    CHECK_EQ(bad.exit_code, 2);
  }
}

TEST_CASE("--regions loads the catalog (or errors) and is recorded in the options") {
  // Missing / malformed files are rejected at parse time.
  auto missing = parse({"--regions", "/no/such/regions-xyz.json"});
  CHECK(missing.should_exit);
  CHECK_EQ(missing.exit_code, 2);
  CHECK(contains(missing.message, "--regions"));
  CHECK(!RegionCatalog::using_custom_catalog());

  // A valid file is applied and remembered.
  std::string path = cerebra::test::temp_path("bm_cli_regions.json");
  {
    std::ofstream out(path, std::ios::trunc);
    out << R"([{ "key": "amygdala", "primary_transmitter": "norepinephrine", "region_of_interest": true },
               { "key": "demo_area", "abbreviation": "DMO", "primary_transmitter": "glutamate" }])";
  }
  auto ok = parse({"--regions", path, "--format", "report"});
  CHECK(!ok.should_exit);
  CHECK(ok.regions_file.has_value());
  CHECK_EQ(*ok.regions_file, path);
  CHECK(RegionCatalog::using_custom_catalog());
  CHECK_EQ(RegionCatalog::all().size(), static_cast<std::size_t>(2));
  CHECK(RegionCatalog::find("demo_area").has_value());

  // Restore global state so the rest of the suite sees the built-in catalog.
  RegionCatalog::reset_to_defaults();
  std::remove(path.c_str());
  CHECK(!RegionCatalog::using_custom_catalog());
}

TEST_CASE("--pathways loads the connectivity catalog and implies --show-pathways") {
  auto missing = parse({"--pathways", "/no/such/pathways-xyz.json"});
  CHECK(missing.should_exit);
  CHECK_EQ(missing.exit_code, 2);
  CHECK(contains(missing.message, "--pathways"));
  CHECK(!PathwayCatalog::using_custom_catalog());

  std::string path = cerebra::test::temp_path("bm_cli_pathways.json");
  {
    std::ofstream out(path, std::ios::trunc);
    out << R"([{ "from": "amygdala", "to": "prefrontal_cortex", "kind": "modulatory" },
               { "from": "thalamus", "to": "occipital_lobe" }])";
  }
  auto ok = parse({"--pathways", path, "--list-pathways"});
  CHECK(ok.should_exit);
  CHECK_EQ(ok.exit_code, 0);
  CHECK(contains(ok.message, "amygdala -> prefrontal_cortex"));
  CHECK(PathwayCatalog::using_custom_catalog());
  CHECK_EQ(PathwayCatalog::all().size(), static_cast<std::size_t>(2));

  auto run = parse({"--pathways", path, "--format", "report"});
  CHECK(!run.should_exit);
  CHECK(run.pathways_file.has_value());
  CHECK_EQ(*run.pathways_file, path);
  CHECK(run.show_pathways);  // loading pathways implies wanting to see them

  CHECK(parse({"--show-pathways"}).show_pathways);

  PathwayCatalog::reset_to_defaults();
  std::remove(path.c_str());
  CHECK(!PathwayCatalog::using_custom_catalog());
}

TEST_CASE("--neurotransmitters and --states load their catalogs (or error)") {
  CHECK(parse({"--neurotransmitters", "/no/such/nt.json"}).should_exit);
  CHECK(parse({"--states", "/no/such/states.json"}).should_exit);
  CHECK(!Neurochemistry::using_custom_catalog());
  CHECK(!BrainStateLibrary::using_custom_catalog());

  std::string nt_path = cerebra::test::temp_path("bm_cli_nt.json");
  std::string st_path = cerebra::test::temp_path("bm_cli_st.json");
  {
    std::ofstream out(nt_path, std::ios::trunc);
    out << R"([{"key":"dopamine"},{"key":"acetylcholine"},{"key":"endorphin","symbol":"END"}])";
  }
  {
    std::ofstream out(st_path, std::ios::trunc);
    out << R"([{"key":"runner_high","display_name":"Runner High",
               "region_intensities":{"prefrontal_cortex":0.6,"insula":0.5}}])";
  }

  // Order matters: neurotransmitters first, then regions/states reference them.
  auto a = parse({"--neurotransmitters", nt_path, "--states", st_path, "--list-transmitters"});
  CHECK(a.should_exit);
  CHECK_EQ(a.exit_code, 0);
  CHECK(contains(a.message, "endorphin"));
  CHECK(Neurochemistry::using_custom_catalog());
  CHECK_EQ(Neurochemistry::catalog().size(), static_cast<std::size_t>(3));
  CHECK(BrainStateLibrary::using_custom_catalog());
  CHECK(BrainStateLibrary::find("runner_high") != nullptr);

  auto b = parse({"--neurotransmitters", nt_path, "--states", st_path,
                  "--state", "runner_high", "--format", "report"});
  CHECK(!b.should_exit);
  CHECK(b.neurotransmitters_file.has_value());
  CHECK_EQ(*b.neurotransmitters_file, nt_path);
  CHECK(b.states_file.has_value());
  CHECK(b.input.kind == InputKind::BrainState);
  CHECK_EQ(b.input.state_key, std::string("runner_high"));

  // --list-states reflects the loaded library when --states comes first.
  CHECK(contains(parse({"--states", st_path, "--list-states"}).message, "runner_high"));

  Neurochemistry::reset_to_defaults();
  BrainStateLibrary::reset_to_defaults();
  std::remove(nt_path.c_str());
  std::remove(st_path.c_str());
  CHECK(!Neurochemistry::using_custom_catalog());
  CHECK(!BrainStateLibrary::using_custom_catalog());
}

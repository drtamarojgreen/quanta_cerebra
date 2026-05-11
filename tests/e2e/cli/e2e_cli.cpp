// End-to-end tests: run the built `brain_modeler` executable as a child process
// and assert on its output and exit status.

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "../test_framework.hpp"

#if !defined(_WIN32)
#  include <sys/wait.h>
#endif

#ifndef BRAIN_MODELER_BINARY
#error "BRAIN_MODELER_BINARY must be defined by the build"
#endif
#ifndef BRAIN_MODELER_DATA_DIR
#error "BRAIN_MODELER_DATA_DIR must be defined by the build"
#endif

namespace {

struct RunResult {
  int exit_code = -1;
  std::string output;
};

std::string temp_dir() { return P_tmpdir ? std::string(P_tmpdir) : std::string("/tmp"); }

// Run `<binary> <args>`, capturing stdout+stderr via a temp file, and return
// the decoded exit code together with the captured text.
RunResult run(const std::string& args) {
  static int counter = 0;
  std::string out_path = temp_dir() + "/brain_modeler_e2e_" + std::to_string(++counter) + ".txt";
  std::string cmd = std::string("\") + BRAIN_MODELER_BINARY + "\" " + args +
                    " > \" + out_path + "\" 2>&1";
  int status = std::system(cmd.c_str());

  RunResult r;
#if defined(WIFEXITED)
  if (WIFEXITED(status)) r.exit_code = WEXITSTATUS(status);
  else r.exit_code = 128;
#else
  r.exit_code = status;
#endif

  std::ifstream in(out_path, std::ios::binary);
  std::ostringstream ss;
  ss << in.rdbuf();
  r.output = ss.str();
  in.close();
  std::remove(out_path.c_str());
  return r;
}

bool contains(const std::string& s, const std::string& n) { return s.find(n) != std::string::npos; }

std::string data_file(const std::string& name) {
  return std::string(BRAIN_MODELER_DATA_DIR) + "/" + name;
}

}  // namespace

TEST_CASE("e2e: --help prints usage and exits 0") {
  auto r = run("--help");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "Usage:"));
  CHECK(contains(r.output, "--format"));
  CHECK(contains(r.output, "--serial"));
  CHECK(contains(r.output, "--tour"));
}

TEST_CASE("e2e: --version prints the version") {
  auto r = run("--version");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "Brain Modeler"));
}

TEST_CASE("e2e: --list-states / --list-regions / --list-themes") {
  auto s = run("--list-states");
  CHECK_EQ(s.exit_code, 0);
  CHECK(contains(s.output, "focused"));
  CHECK(contains(s.output, "relaxed"));
  CHECK(contains(s.output, "stressed"));
  CHECK(contains(s.output, "rem_sleep"));

  auto rg = run("--list-regions");
  CHECK_EQ(rg.exit_code, 0);
  for (const char* roi : {"amygdala", "cuneus", "anterior_cingulate_cortex", "insula",
                          "gyrus", "hippocampus", "thalamus", "entorhinal_cortex"}) {
    CHECK(contains(rg.output, roi));
  }

  auto th = run("--list-themes");
  CHECK_EQ(th.exit_code, 0);
  CHECK(contains(th.output, "monochrome"));
}

TEST_CASE("e2e: report mode on the sample JSON file") {
  auto r = run("--format report --ascii --width 80 --input \" + data_file("sample_activity.json") + "\");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "simulation report"));
  CHECK(contains(r.output, "frames      : 5"));
  CHECK(contains(r.output, "Region activity"));
  CHECK(contains(r.output, "Amygdala"));
  CHECK(contains(r.output, "Occipital Lobe"));
  CHECK(contains(r.output, "Neurochemistry"));
  CHECK(contains(r.output, "Coronal slice"));
  CHECK(contains(r.output, "intensity legend"));
}

TEST_CASE("e2e: report mode for each brain-state preset, 3D view") {
  for (const char* preset : {"focused", "relaxed", "stressed", "rem_sleep"}) {
    auto r = run(std::string("--format report --ascii --width 76 --view 3d --frames 10 --state ") + preset);
    CHECK_MSG(r.exit_code == 0, std::string("preset ") + preset + " exited non-zero");
    CHECK(contains(r.output, "frames      : 10"));
    CHECK(contains(r.output, "3D projection"));
  }
}

TEST_CASE("e2e: interactive mode on a non-tty falls back gracefully") {
  // stdout is redirected to a file here, so --format interactive should print
  // a single frame plus a hint and exit 0.
  auto r = run("--format interactive --state relaxed --frames 5 --ascii --width 80 --height 24");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "Brain Modeler"));
  CHECK(contains(r.output, "interactive mode needs a TTY"));
}

TEST_CASE("e2e: a bad option exits with code 2 and shows usage") {
  auto r = run("--no-such-flag");
  CHECK_EQ(r.exit_code, 2);
  CHECK(contains(r.output, "unrecognized option"));
}

TEST_CASE("e2e: a missing input file exits non-zero") {
  auto r = run("--format report --input /definitely/not/here-123.json");
  CHECK(r.exit_code != 0);
  CHECK(contains(r.output, "brain_modeler:"));
}

TEST_CASE("e2e: the memory-serial device transmits a real timeline into the report") {
  auto r = run("--format report --memory-serial --state focused --ascii --width 76");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "simulation report"));
  // The simulated device transmits many JSON frames, so the report is no longer
  // just the single resting-baseline frame.
  CHECK(!contains(r.output, "frames      : 1\n"));
  CHECK(contains(r.output, "Region activity"));
}

TEST_CASE("e2e: --pathways loads a connectivity catalog and surfaces it in the report") {
  auto r = run("--pathways \" + data_file("pathways.json") + "\" --format report --ascii --width 90 "
               "--input \" + data_file("sample_activity.json") + "\");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "Pathways"));
  CHECK(contains(r.output, "Amygdala -> Prefrontal Cortex"));
  CHECK(contains(r.output, "modulatory"));
  CHECK(contains(r.output, "uncinate fasciculus"));
  CHECK(contains(r.output, "Hippocampus -- Entorhinal Cortex"));  // an undirected pathway

  auto lp = run("--pathways \" + data_file("pathways.json") + "\" --list-pathways");
  CHECK_EQ(lp.exit_code, 0);
  CHECK(contains(lp.output, "amygdala -> prefrontal_cortex"));
  auto lp2 = run("--list-pathways");
  CHECK(contains(lp2.output, "No pathways are loaded"));

  // --show-pathways alone (no catalog) is harmless.
  auto r2 = run("--format report --show-pathways --state relaxed --frames 6 --ascii --width 70");
  CHECK_EQ(r2.exit_code, 0);
}

TEST_CASE("e2e: the shipped neurotransmitter and brain-state catalogs round-trip") {
  auto r = run("--neurotransmitters \" + data_file("neurotransmitters.json") + "\" "
               "--states \" + data_file("brain_states.json") + "\" "
               "--state focused --format report --ascii --width 80 --frames 8");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "frames      : 8"));
  CHECK(contains(r.output, "Neurochemistry"));
  CHECK(contains(r.output, "Dopamine"));
  CHECK(contains(r.output, "Amygdala"));

  auto lt = run("--neurotransmitters \" + data_file("neurotransmitters.json") + "\" --list-transmitters");
  CHECK_EQ(lt.exit_code, 0);
  for (const char* nt : {"dopamine", "serotonin", "norepinephrine", "acetylcholine", "glutamate", "gaba"})
    CHECK(contains(lt.output, nt));

  auto ls = run("--states \" + data_file("brain_states.json") + "\" --list-states");
  CHECK_EQ(ls.exit_code, 0);
  for (const char* st : {"focused", "relaxed", "stressed", "rem_sleep"}) CHECK(contains(ls.output, st));
  CHECK(contains(ls.output, "tag=task-positive"));  // metadata from the file
}

TEST_CASE("e2e: a custom neurotransmitter can be referenced by a custom region") {
  std::string nt = temp_dir() + "/bm_e2e_nt.json";
  std::string rg = temp_dir() + "/bm_e2e_rg.json";
  { std::ofstream out(nt, std::ios::trunc);
    out << R"([{"key":"histamine","symbol":"HA"},{"key":"glutamate"}])"; }
  { std::ofstream out(rg, std::ios::trunc);
    out << R"([{"key":"tuberomammillary","display_name":"Tuberomammillary Nucleus","abbreviation":"TMN",
               "primary_transmitter":"histamine"},
              {"key":"thalamus","display_name":"Thalamus","primary_transmitter":"glutamate"}])"; }

  // Loaded in the right order, the custom region's transmitter resolves.
  auto ok = run("--neurotransmitters \" + nt + "\" --regions \" + rg + "\" "
                "--format report --ascii --width 80 --state focused --frames 6");
  CHECK_EQ(ok.exit_code, 0);
  CHECK(contains(ok.output, "Tuberomammillary Nucleus"));
  CHECK(contains(ok.output, "Histamine"));

  // Loaded in the wrong order, validation fails (built-in NT catalog has no histamine).
  auto bad = run("--regions \" + rg + "\" --neurotransmitters \" + nt + "\" --format report");
  CHECK(bad.exit_code != 0);
  CHECK(contains(bad.output, "histamine"));

  std::remove(nt.c_str());
  std::remove(rg.c_str());
}

TEST_CASE("e2e: invalid --neurotransmitters / --states arguments are hard errors") {
  CHECK(run("--neurotransmitters /no/such/nt.json --format report").exit_code != 0);
  CHECK(run("--states /no/such/states.json --format report").exit_code != 0);

  std::string bad = temp_dir() + "/bm_e2e_bad_cat.json";
  { std::ofstream out(bad, std::ios::trunc); out << "[]"; }  // empty catalogs are rejected
  CHECK(run("--neurotransmitters \" + bad + "\" --format report").exit_code != 0);
  CHECK(run("--states \" + bad + "\" --format report").exit_code != 0);
  std::remove(bad.c_str());
}

TEST_CASE("e2e: an invalid --pathways argument is a hard error") {
  auto missing = run("--pathways /no/such/pathways-xyz.json --format report");
  CHECK(missing.exit_code != 0);
  CHECK(contains(missing.output, "--pathways"));

  std::string bad_json = temp_dir() + "/bm_e2e_bad_pathways.json";
  { std::ofstream out(bad_json, std::ios::trunc); out << "not json {{{"; }
  CHECK(run("--pathways \" + bad_json + "\" --format report").exit_code != 0);
  std::remove(bad_json.c_str());

  std::string bad_ref = temp_dir() + "/bm_e2e_badref_pathways.json";
  { std::ofstream out(bad_ref, std::ios::trunc);
    out << R"([{"from":"amygdala","to":"the_shadow_realm"}])"; }
  CHECK(run("--pathways \" + bad_ref + "\" --format report").exit_code != 0);
  std::remove(bad_ref.c_str());
}

TEST_CASE("e2e: the shipped data/regions.json loads and round-trips") {
  auto r = run("--regions \" + data_file("regions.json") + "\" --format report --ascii --width 80 "
               "--input \" + data_file("sample_activity.json") + "\");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "simulation report"));
  CHECK(contains(r.output, "Amygdala"));
  CHECK(contains(r.output, "Region metadata"));
  CHECK(contains(r.output, "threat detection"));
}

TEST_CASE("e2e: --regions can fully replace the region catalog") {
  std::string path = temp_dir() + "/bm_e2e_regions.json";
  {
    std::ofstream out(path, std::ios::trunc);
    out << "[\n"
        << R"(  { "key": "amygdala", "display_name": "Amygdala", "abbreviation": "AMY",)" "\n"
        << R"(    "slice_x": 0.4, "slice_y": 0.6, "primary_transmitter": "norepinephrine",)" "\n"
        << R"(    "region_of_interest": true, "metadata": { "function": "threat detection" } },)" "\n"
        << R"(  { "key": "thalamus", "display_name": "Thalamus", "abbreviation": "THA",)" "\n"
        << R"(    "primary_transmitter": "glutamate", "region_of_interest": true },)" "\n"
        << R"(  { "key": "synthetic_nucleus", "display_name": "Synthetic Nucleus", "abbreviation": "SYN",)" "\n"
        << R"(    "primary_transmitter": "dopamine", "region_of_interest": false })" "\n"
        << "]\n";
  }

  auto r = run("--regions \" + path + "\" --format report --ascii --width 80 --input \" +
               data_file("sample_activity.json") + "\");
  CHECK_EQ(r.exit_code, 0);
  CHECK(contains(r.output, "Synthetic Nucleus"));
  CHECK(contains(r.output, "Region metadata"));
  CHECK(contains(r.output, "function=threat detection"));
  CHECK(!contains(r.output, "Hippocampus"));
  CHECK(!contains(r.output, "Occipital Lobe"));

  // --list-regions reflects the catalog when --regions comes first...
  auto lr = run("--regions \" + path + "\" --list-regions");
  CHECK_EQ(lr.exit_code, 0);
  CHECK(contains(lr.output, "synthetic_nucleus"));
  CHECK(!contains(lr.output, "hippocampus"));
  // ...and shows the built-in catalog when it comes after.
  auto lr2 = run("--list-regions --regions \" + path + "\");
  CHECK(contains(lr2.output, "hippocampus"));

  std::remove(path.c_str());
}

TEST_CASE("e2e: an invalid --regions argument is a hard error") {
  auto missing = run("--regions /no/such/regions-xyz.json --format report");
  CHECK(missing.exit_code != 0);
  CHECK(contains(missing.output, "--regions"));

  std::string path = temp_dir() + "/bm_e2e_bad_regions.json";
  {
    std::ofstream out(path, std::ios::trunc);
    out << "not valid json at all {{{";
  }
  auto bad = run("--regions \" + path + "\" --format report");
  CHECK(bad.exit_code != 0);
  std::remove(path.c_str());

  // An empty catalog array is rejected too.
  std::string path2 = temp_dir() + "/bm_e2e_empty_regions.json";
  {
    std::ofstream out(path2, std::ios::trunc);
    out << "[]";
  }
  auto empty = run("--regions \" + path2 + "\" --format report");
  CHECK(empty.exit_code != 0);
  std::remove(path2.c_str());
}

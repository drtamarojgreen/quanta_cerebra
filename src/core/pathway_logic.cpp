#include "core/pathway_logic.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "io/json_parser.h"
#include "core/neurochemistry.h"
#include "core/atlas_region.h"
#include "io/config_util.hpp"

namespace cerebra {
namespace {

using config_util::clamp01;

std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}

std::vector<Pathway>& active_catalog() {
  static std::vector<Pathway> active;  // empty by default
  return active;
}

bool& custom_flag() {
  static bool f = false;
  return f;
}

}  // namespace

const char* to_string(PathwayKind kind) {
  switch (kind) {
    case PathwayKind::Excitatory: return "excitatory";
    case PathwayKind::Inhibitory: return "inhibitory";
    case PathwayKind::Modulatory: return "modulatory";
  }
  return "excitatory";
}

bool parse_pathway_kind(const std::string& text, PathwayKind& out) {
  std::string t = lower(text);
  if (t == "excitatory" || t == "exc" || t == "+") { out = PathwayKind::Excitatory; return true; }
  if (t == "inhibitory" || t == "inh" || t == "-") { out = PathwayKind::Inhibitory; return true; }
  if (t == "modulatory" || t == "mod" || t == "~") { out = PathwayKind::Modulatory; return true; }
  return false;
}

const std::vector<Pathway>& PathwayCatalog::all() { return active_catalog(); }

std::vector<Pathway> PathwayCatalog::incident_to(const std::string& region_key) {
  std::string key = RegionCatalog::normalize_key(region_key);
  std::vector<Pathway> out;
  for (const auto& p : active_catalog()) {
    if (p.from == key || p.to == key) out.push_back(p);
  }
  return out;
}

std::vector<Pathway> PathwayCatalog::afferent(const std::string& region_key) {
  std::string key = RegionCatalog::normalize_key(region_key);
  std::vector<Pathway> out;
  for (const auto& p : active_catalog()) {
    if (p.to == key || (!p.directed && p.from == key)) out.push_back(p);
  }
  return out;
}

std::vector<Pathway> PathwayCatalog::efferent(const std::string& region_key) {
  std::string key = RegionCatalog::normalize_key(region_key);
  std::vector<Pathway> out;
  for (const auto& p : active_catalog()) {
    if (p.from == key || (!p.directed && p.to == key)) out.push_back(p);
  }
  return out;
}

void PathwayCatalog::load_from_json(const JsonValue& root) {
  if (!root.is_array()) {
    throw std::runtime_error("pathway catalog must be a JSON array");
  }
  std::vector<Pathway> parsed;
  for (const auto& elem : root.as_array()) {
    if (!elem.is_object()) {
      throw std::runtime_error("each pathway entry must be a JSON object");
    }
    Pathway p;
    const std::string raw_from = elem["from"].as_string();
    const std::string raw_to = elem["to"].as_string();
    if (raw_from.empty() || raw_to.empty()) {
      throw std::runtime_error("pathway entry needs non-empty 'from' and 'to'");
    }
    auto rf = RegionCatalog::find(raw_from);
    auto rt = RegionCatalog::find(raw_to);
    if (!rf) throw std::runtime_error("pathway references unknown region '" + raw_from + "'");
    if (!rt) throw std::runtime_error("pathway references unknown region '" + raw_to + "'");
    p.from = rf->id;
    p.to = rt->id;
    p.directed = elem.contains("directed") ? elem["directed"].as_bool(true) : true;
    p.weight = clamp01(elem["weight"].as_number(0.5));
    p.delay_ms = std::max<std::int64_t>(0, elem["delay_ms"].as_int(0));
    if (elem.contains("kind")) {
      if (!parse_pathway_kind(elem["kind"].as_string(), p.kind)) {
        throw std::runtime_error("pathway '" + p.label() + "' has unknown kind '" +
                                 elem["kind"].as_string() + "'");
      }
    }
    p.transmitter = elem["transmitter"].as_string();
    if (!p.transmitter.empty() && !Neurochemistry::find(p.transmitter)) {
      throw std::runtime_error("pathway '" + p.label() + "' references unknown neurotransmitter '" +
                               p.transmitter + "'");
    }
    p.extra = config_util::parse_metadata(elem["metadata"]);
    parsed.push_back(std::move(p));
  }
  active_catalog() = std::move(parsed);
  custom_flag() = true;
}

void PathwayCatalog::load_from_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("cannot open pathways file: " + path);
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  load_from_json(JsonValue::parse(buffer.str()));
}

void PathwayCatalog::reset_to_defaults() {
  active_catalog().clear();
  custom_flag() = false;
}

bool PathwayCatalog::using_custom_catalog() { return custom_flag(); }

}  // namespace cerebra

#ifndef BRAIN_MODELER_PATHWAY_HPP
#define BRAIN_MODELER_PATHWAY_HPP

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace cerebra {

class JsonValue;  // defined in json_utility.h; referenced only by reference here

enum class PathwayKind { Excitatory, Inhibitory, Modulatory };

const char* to_string(PathwayKind kind);
// Parse a kind name (case-insensitive: excitatory/exc/+, inhibitory/inh/-,
// modulatory/mod/~). Returns std::nullopt-equivalent via the bool out param.
bool parse_pathway_kind(const std::string& text, PathwayKind& out);

// A directed (or undirected) connection between two regions. Endpoints are
// canonical region keys (validated against the active RegionCatalog on load).
struct Pathway {
  std::string from;
  std::string to;
  bool directed = true;
  double weight = 0.5;            // connection strength in [0,1]
  std::int64_t delay_ms = 0;      // conduction delay
  PathwayKind kind = PathwayKind::Excitatory;
  std::string transmitter;        // optional; must be a known transmitter if set
  std::map<std::string, std::string> extra;  // open metadata

  std::string label() const { return from + (directed ? " -> " : " -- ") + to; }
};

class PathwayCatalog {
public:
  // The active list of pathways. Empty by default; load_from_json()/
  // load_from_file() can populate it (see data/pathways.json for the schema).
  static const std::vector<Pathway>& all();
  static bool empty() { return all().empty(); }

  // Pathways touching `region_key` (either endpoint).
  static std::vector<Pathway> incident_to(const std::string& region_key);
  // Directed pathways arriving at / leaving `region_key`; undirected pathways
  // touching it appear in both lists.
  static std::vector<Pathway> afferent(const std::string& region_key);
  static std::vector<Pathway> efferent(const std::string& region_key);

  // ---- configuration ------------------------------------------------------
  // Replace the active catalog with one parsed from a JSON array. Each entry's
  // `from`/`to` must resolve to a region in the active RegionCatalog. Throws
  // std::runtime_error / JsonParseError on a malformed or invalid document; the
  // previous catalog is left intact on failure.
  static void load_from_json(const JsonValue& root);
  static void load_from_file(const std::string& path);
  static void reset_to_defaults();          // -> empty
  static bool using_custom_catalog();
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_PATHWAY_HPP

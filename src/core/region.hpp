#ifndef BRAIN_MODELER_REGION_HPP
#define BRAIN_MODELER_REGION_HPP

#include "core/atlas_core.h"
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cerebra {

class JsonValue;  // defined in json_utility.h; only referenced by reference here

class RegionCatalog {
public:
  // The active, ordered list of regions the application understands. By default
  // this is the built-in catalog; load_from_json()/load_from_file() can replace
  // it (see data/regions.json for the schema).
  static const std::vector<RegionDefinition>& all();

  // The subset flagged as "regions of interest".
  static std::vector<RegionDefinition> regions_of_interest();

  // Look up by canonical key (case-insensitive, tolerates spaces/dashes) or by
  // abbreviation, falling back to a small alias table.
  static std::optional<RegionDefinition> find(const std::string& key);

  // Normalize an arbitrary user/input string to a catalog key when possible,
  // otherwise return a slugified version of the input.
  static std::string normalize_key(const std::string& raw);

  // True when the (normalized) key is one of the active catalog's regions of
  // interest.
  static bool is_region_of_interest(const std::string& key);

  // ---- configuration ------------------------------------------------------
  // Replace the active catalog with one parsed from a JSON array. Throws
  // std::runtime_error / JsonParseError on a malformed or invalid document; on
  // failure the previous catalog is left untouched.
  static void load_from_json(const JsonValue& root);
  static void load_from_file(const std::string& path);
  // Restore the built-in catalog.
  static void reset_to_defaults();
  // True if a custom catalog has been loaded (and not yet reset).
  static bool using_custom_catalog();
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_REGION_HPP

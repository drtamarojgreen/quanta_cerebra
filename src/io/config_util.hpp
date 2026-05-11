#ifndef BRAIN_MODELER_SRC_CONFIG_UTIL_HPP
#define BRAIN_MODELER_SRC_CONFIG_UTIL_HPP

// Internal helpers shared by the JSON catalog loaders (regions, pathways,
// neurotransmitters, brain states). Header-only; included only by .cpp files in
// src/ via a relative include.

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <map>
#include <string>

#include "io/json_parser.h"

namespace cerebra {
namespace config_util {

inline double clamp01(double v) { return std::max(0.0, std::min(1.0, v)); }

// Collapse runs of non-alphanumerics to single underscores; lowercase the rest.
inline std::string slugify(const std::string& raw) {
  std::string out;
  bool last_sep = false;
  for (unsigned char c : raw) {
    if (std::isalnum(c)) {
      out.push_back(static_cast<char>(std::tolower(c)));
      last_sep = false;
    } else if (!out.empty() && !last_sep) {
      out.push_back('_');
      last_sep = true;
    }
  }
  while (!out.empty() && out.back() == '_') out.pop_back();
  return out;
}

// "prefrontal_cortex" -> "Prefrontal Cortex"
inline std::string title_from_key(const std::string& key) {
  std::string out = key;
  std::replace(out.begin(), out.end(), '_', ' ');
  bool word_start = true;
  for (auto& c : out) {
    if (word_start && std::isalpha(static_cast<unsigned char>(c)))
      c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    word_start = (c == ' ');
  }
  return out;
}

// First `max_len` alphanumeric characters of `key`, uppercased; never empty.
inline std::string short_code(const std::string& key, std::size_t max_len, const char* fallback) {
  std::string out;
  for (unsigned char c : key) {
    if (std::isalnum(c)) {
      out.push_back(static_cast<char>(std::toupper(c)));
      if (out.size() >= max_len) break;
    }
  }
  return out.empty() ? std::string(fallback) : out;
}

// A JSON object's values flattened to a string map (string values verbatim,
// everything else via dump()). Empty if `v` is not an object.
inline std::map<std::string, std::string> parse_metadata(const JsonValue& v) {
  std::map<std::string, std::string> out;
  if (!v.is_object()) return out;
  for (const auto& kv : v.as_object())
    out[kv.first] = kv.second.is_string() ? kv.second.as_string() : kv.second.dump();
  return out;
}

}  // namespace config_util
}  // namespace cerebra

#endif  // BRAIN_MODELER_SRC_CONFIG_UTIL_HPP

#pragma once

#include "core/bm_atlas_region.h"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cerebra {

struct BrainFrame {
    std::int64_t timestamp_ms = 0;
    std::vector<RegionState> regions;
};

enum class BrainTemplate {
    Focused,
    Relaxed,
    Stressed,
    REMSleep,
};

const char* template_name(BrainTemplate t);

// Legacy enum-based parse. Recognises the four built-in templates plus the
// "rem" / "rem-sleep" aliases. New code should prefer template_exists.
bool parse_template(std::string_view name, BrainTemplate& out);

// String-based API backed by the current atlas's templates.
//
// resolve_template_id lower-cases the input and maps a few human aliases
// ("rem", "rem-sleep") to canonical ids, then checks whether the current
// atlas actually contains that template. Returns empty when no template
// matches.
std::string resolve_template_id(std::string_view input);
bool template_exists(std::string_view id);

// Build a BrainFrame from a template. The atlas-backed string form is the
// canonical one; the enum form is preserved as a thin shim so existing call
// sites are not forced to migrate.
BrainFrame template_frame(std::string_view template_id, std::int64_t timestamp_ms);
BrainFrame template_frame(BrainTemplate t, std::int64_t timestamp_ms);

}

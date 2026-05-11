#include "ui/visual_themes.h"

namespace cerebra {

const std::vector<Theme>& available_themes() {
    static const std::vector<Theme> themes = {
        {"classic",   "1;37", "1;36", "1;33", "1;31", "0"},
        {"matrix",    "1;32", "0;32", "1;92", "1;31", "0"},
        {"sunset",    "1;33", "0;33", "1;31", "1;35", "0"},
        {"ocean",     "1;36", "0;34", "1;94", "1;31", "0"},
        {"mono",      "1",    "0",    "1",    "1",    "0"},
    };
    return themes;
}

const Theme& theme_by_name(std::string_view name) {
    for (const auto& t : available_themes()) {
        if (t.name == name) return t;
    }
    return default_theme();
}

const Theme& default_theme() {
    return available_themes().front();
}

}

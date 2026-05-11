#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace cerebra {

struct Theme {
    std::string name;
    std::string title_color;       // ANSI SGR sequence (no escape prefix)
    std::string accent_color;
    std::string label_color;
    std::string warn_color;
    std::string reset = "0";
};

const std::vector<Theme>& available_themes();
const Theme& theme_by_name(std::string_view name);
const Theme& default_theme();

}

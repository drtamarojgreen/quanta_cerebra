#include "ui/color_theme.hpp"

#include <algorithm>
#include <cmath>

namespace cerebra {

ColorTheme::ColorTheme(std::string key, std::string display_name, bool color_enabled,
                       std::string accent, std::string secondary, std::string warning,
                       std::string dim, std::vector<std::string> intensity_ramp)
    : key_(std::move(key)),
      display_name_(std::move(display_name)),
      color_enabled_(color_enabled),
      accent_(std::move(accent)),
      secondary_(std::move(secondary)),
      warning_(std::move(warning)),
      dim_(std::move(dim)),
      intensity_ramp_(std::move(intensity_ramp)) {}

std::string ColorTheme::sgr(const std::string& params) const {
  if (!color_enabled_ || params.empty()) return "";
  return "\x1b[" + params + "m";
}

std::string ColorTheme::reset() const { return color_enabled_ ? "\x1b[0m" : ""; }

std::string ColorTheme::wrap(const std::string& params, const std::string& text) const {
  if (!color_enabled_ || params.empty()) return text;
  return "\x1b[" + params + "m" + text + "\x1b[0m";
}

std::string ColorTheme::intensity(const std::string& text, double value) const {
  if (!color_enabled_ || intensity_ramp_.empty()) return text;
  value = std::max(0.0, std::min(1.0, value));
  std::size_t idx = static_cast<std::size_t>(
      std::round(value * static_cast<double>(intensity_ramp_.size() - 1)));
  idx = std::min(idx, intensity_ramp_.size() - 1);
  return wrap(intensity_ramp_[idx], text);
}

namespace {

ColorTheme make_monochrome(bool color_enabled) {
  // Pure greyscale ramp (works on any 256-color terminal).
  std::vector<std::string> ramp = {"38;5;238", "38;5;242", "38;5;246",
                                   "38;5;250", "38;5;253", "38;5;231"};
  return ColorTheme("monochrome", "Monochrome", color_enabled, "1", "2", "1;31", "2",
                    std::move(ramp));
}

}  // namespace

std::vector<ColorTheme> ThemeLibrary::all(bool color_enabled) {
  std::vector<ColorTheme> themes;
  // Default: cool clinical blue/cyan.
  themes.emplace_back("clinical", "Clinical Blue", color_enabled, "38;5;39", "38;5;45",
                      "38;5;203", "38;5;240",
                      std::vector<std::string>{"38;5;17", "38;5;25", "38;5;31",
                                               "38;5;38", "38;5;45", "38;5;87"});
  // Amber CRT.
  themes.emplace_back("amber", "Amber Terminal", color_enabled, "38;5;214", "38;5;220",
                      "38;5;196", "38;5;94",
                      std::vector<std::string>{"38;5;52", "38;5;94", "38;5;130",
                                               "38;5;172", "38;5;214", "38;5;226"});
  // Matrix green.
  themes.emplace_back("matrix", "Matrix Green", color_enabled, "38;5;46", "38;5;82",
                      "38;5;226", "38;5;22",
                      std::vector<std::string>{"38;5;22", "38;5;28", "38;5;34",
                                               "38;5;40", "38;5;46", "38;5;120"});
  // Magma heat map.
  themes.emplace_back("magma", "Magma", color_enabled, "38;5;205", "38;5;213",
                      "38;5;226", "38;5;53",
                      std::vector<std::string>{"38;5;17", "38;5;54", "38;5;127",
                                               "38;5;167", "38;5;208", "38;5;226"});
  // High contrast.
  themes.emplace_back("high-contrast", "High Contrast", color_enabled, "1;97", "1;96",
                      "1;93", "37",
                      std::vector<std::string>{"38;5;236", "38;5;240", "38;5;245",
                                               "38;5;250", "38;5;254", "1;97"});
  themes.push_back(make_monochrome(color_enabled));
  return themes;
}

ColorTheme ThemeLibrary::by_key(const std::string& key, bool color_enabled) {
  for (auto& t : all(color_enabled)) {
    if (t.key() == key) return t;
  }
  return default_theme(color_enabled);
}

ColorTheme ThemeLibrary::default_theme(bool color_enabled) {
  if (!color_enabled) return make_monochrome(false);
  return all(true).front();
}

std::vector<std::string> ThemeLibrary::keys() {
  std::vector<std::string> out;
  for (const auto& t : all(true)) out.push_back(t.key());
  return out;
}

}  // namespace cerebra

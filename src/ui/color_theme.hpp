#ifndef BRAIN_MODELER_COLOR_THEME_HPP
#define BRAIN_MODELER_COLOR_THEME_HPP

#include <string>
#include <vector>

namespace cerebra {

// A color theme for the interactive display. Colors are stored as ANSI SGR
// parameter strings (e.g. "38;5;208"); an empty string means "use the
// terminal default". When color is disabled every accessor returns " so the
// renderer transparently degrades to plain ASCII.
class ColorTheme {
public:
  ColorTheme(std::string key, std::string display_name, bool color_enabled,
             std::string accent, std::string secondary, std::string warning,
             std::string dim, std::vector<std::string> intensity_ramp);

  const std::string& key() const { return key_; }
  const std::string& display_name() const { return display_name_; }
  bool color_enabled() const { return color_enabled_; }

  // Wrap text in this theme's accent / secondary / warning / dim color.
  std::string accent(const std::string& text) const { return wrap(accent_, text); }
  std::string secondary(const std::string& text) const { return wrap(secondary_, text); }
  std::string warning(const std::string& text) const { return wrap(warning_, text); }
  std::string dim(const std::string& text) const { return wrap(dim_, text); }

  // Color a block-character cell according to an intensity in [0,1].
  std::string intensity(const std::string& text, double value) const;

  // Raw SGR helpers for callers that build their own runs.
  std::string sgr(const std::string& params) const;
  std::string reset() const;

private:
  std::string wrap(const std::string& params, const std::string& text) const;

  std::string key_;
  std::string display_name_;
  bool color_enabled_;
  std::string accent_;
  std::string secondary_;
  std::string warning_;
  std::string dim_;
  std::vector<std::string> intensity_ramp_;
};

class ThemeLibrary {
public:
  // All built-in themes (color-enabled variants). The first entry is the
  // default. `monochrome` is always present for non-color terminals.
  static std::vector<ColorTheme> all(bool color_enabled);
  static ColorTheme by_key(const std::string& key, bool color_enabled);
  static ColorTheme default_theme(bool color_enabled);
  static std::vector<std::string> keys();
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_COLOR_THEME_HPP

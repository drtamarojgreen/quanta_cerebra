#ifndef BRAIN_MODELER_RENDERER_HPP
#define BRAIN_MODELER_RENDERER_HPP

#include <optional>
#include <string>
#include <vector>

#include "ui/color_theme.hpp"
#include "core/simulation_engine.h"

namespace cerebra {

enum class ViewMode { Slice2D, Projection3D };

struct RenderOptions {
  int width = 80;
  int height = 24;
  ViewMode view = ViewMode::Slice2D;
  bool ascii_only = false;       // never emit non-ASCII block glyphs
  bool show_help_bar = true;
  bool show_chemistry = true;
  bool show_pathways = false;    // overlay configured pathways on the brain view
  ColorTheme theme = ThemeLibrary::default_theme(false);
};

// Renders a Simulation snapshot into terminal-ready lines (already colorized
// for the active theme). All sizing adapts to RenderOptions::width/height.
class Renderer {
public:
  explicit Renderer(RenderOptions options);

  void set_options(RenderOptions options) { options_ = std::move(options); }
  const RenderOptions& options() const { return options_; }

  // Full interactive frame: title, brain view, chemistry panel, selected
  // region stats, help bar. Returns at most options_.height lines.
  std::vector<std::string> render_frame(const Simulation& sim) const;

  // Just the brain view (2D slice or 3D projection), `view_height` rows tall.
  std::vector<std::string> render_brain_view(const Simulation& sim, int view_width,
                                             int view_height) const;

  // Inverse mapping for mouse input: given a click at (x,y) inside a brain view
  // of the given size, return the nearest region key (within a small radius).
  std::optional<std::string> region_at(const Simulation& sim, int view_width,
                                       int view_height, int x, int y) const;

  // Layout the interactive frame uses, exposed so callers (mouse handling) can
  // figure out where the brain view sits.
  struct FrameLayout {
    int view_x = 0;
    int view_y = 0;
    int view_width = 0;
    int view_height = 0;
  };
  FrameLayout frame_layout() const;

  // A plain-text, color-free report of an entire timeline (for --format report).
  static std::string render_report(const Simulation& sim, int width = 80,
                                   ViewMode view = ViewMode::Slice2D,
                                   bool ascii_only = true);

  // Map an intensity in [0,1] to a shading glyph.
  static std::string intensity_glyph(double value, bool ascii_only);

private:
  RenderOptions options_;
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_RENDERER_HPP

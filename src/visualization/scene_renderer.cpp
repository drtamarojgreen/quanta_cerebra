#include "visualization/scene_renderer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "core/neurochemistry.h"
#include "core/pathway_logic.h"
#include "core/region.hpp"

namespace cerebra {
namespace {

constexpr double kPi = 3.14159265358979323846;

double clamp01(double v) { return std::max(0.0, std::min(1.0, v)); }

const char* kUnicodeShades[] = {" ", "░", "░", "▒", "▒",
                                "▓", "▓", "█", "█"};
const char* kAsciiShades[] = {" ", ".", ":", "-", "=", "+", "*", "#", "@"};

const char* kUnicodeSpark[] = {" ", "▁", "▂", "▃", "▄",
                               "▅", "▆", "▇", "█"};
const char* kAsciiSpark[] = {" ", ".", ",", "-", "=", "+", "*", "#", "@"};

std::string shade(double value, bool ascii_only) {
  value = clamp01(value);
  int idx = static_cast<int>(std::round(value * 8.0));
  idx = std::max(0, std::min(8, idx));
  return ascii_only ? kAsciiShades[idx] : kUnicodeShades[idx];
}

std::string spark_glyph(double value, bool ascii_only) {
  value = clamp01(value);
  int idx = static_cast<int>(std::round(value * 8.0));
  idx = std::max(0, std::min(8, idx));
  return ascii_only ? kAsciiSpark[idx] : kUnicodeSpark[idx];
}

// Color params for an intensity value, picking a slot from the theme ramp.
std::string ramp_params(const ColorTheme& theme, double value) {
  // The ColorTheme exposes intensity() which wraps text; we want raw params
  // for the canvas, so re-derive from the wrapped output (cheap, infrequent).
  std::string wrapped = theme.intensity("X", value);
  // wrapped is "\x1b[<params>mX\x1b[0m" when colored, else "X".
  auto m = wrapped.find('m');
  if (wrapped.size() > 2 && wrapped[0] == '\x1b' && m != std::string::npos) {
    return wrapped.substr(2, m - 2);
  }
  return "";
}

std::string bar(double value, int width, bool ascii_only) {
  value = clamp01(value);
  int filled = static_cast<int>(std::round(value * width));
  std::string out;
  const char* full = ascii_only ? "#" : "█";
  const char* empty = ascii_only ? "." : "─";
  for (int i = 0; i < width; ++i) out += (i < filled) ? full : empty;
  return out;
}

std::string sparkline(const std::vector<double>& series, int width, bool ascii_only) {
  if (series.empty() || width <= 0) return std::string(static_cast<std::size_t>(std::max(0, width)), ' ');
  std::string out;
  for (int i = 0; i < width; ++i) {
    double pos = (width == 1) ? 0.0 : static_cast<double>(i) / (width - 1);
    double fidx = pos * (series.size() - 1);
    std::size_t lo = static_cast<std::size_t>(std::floor(fidx));
    std::size_t hi = std::min(lo + 1, series.size() - 1);
    double frac = fidx - lo;
    double v = series[lo] * (1 - frac) + series[hi] * frac;
    out += spark_glyph(v, ascii_only);
  }
  return out;
}

std::string fmt2(double v) {
  std::ostringstream os;
  os << std::fixed << std::setprecision(2) << v;
  return os.str();
}

std::string pad_right(const std::string& s, std::size_t n) {
  if (s.size() >= n) return s.substr(0, n);
  return s + std::string(n - s.size(), ' ');
}

std::string repeat(const std::string& s, int n) {
  std::string out;
  for (int i = 0; i < n; ++i) out += s;
  return out;
}

// ---------------------------------------------------------------------------
// Canvas: a grid of single-column glyphs each with optional SGR color params.
// ---------------------------------------------------------------------------
class Canvas {
public:
  Canvas(int w, int h) : w_(std::max(1, w)), h_(std::max(1, h)),
                         glyphs_(static_cast<std::size_t>(w_) * h_, " "),
                         colors_(static_cast<std::size_t>(w_) * h_, ") {}

  int width() const { return w_; }
  int height() const { return h_; }

  void put(int x, int y, const std::string& g, const std::string& color = ") {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) return;
    std::size_t i = static_cast<std::size_t>(y) * w_ + x;
    glyphs_[i] = g;
    colors_[i] = color;
  }

  void put_dim_if_empty(int x, int y, const std::string& g, const std::string& color = ") {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) return;
    std::size_t i = static_cast<std::size_t>(y) * w_ + x;
    if (glyphs_[i] == " ") {
      glyphs_[i] = g;
      colors_[i] = color;
    }
  }

  void text(int x, int y, const std::string& s, const std::string& color = ") {
    for (std::size_t k = 0; k < s.size(); ++k) {
      put(x + static_cast<int>(k), y, std::string(1, s[k]), color);
    }
  }

  std::vector<std::string> to_lines(const ColorTheme& theme) const {
    std::vector<std::string> lines;
    lines.reserve(static_cast<std::size_t>(h_));
    for (int y = 0; y < h_; ++y) {
      std::string line;
      std::string cur_color;
      bool color_open = false;
      for (int x = 0; x < w_; ++x) {
        std::size_t i = static_cast<std::size_t>(y) * w_ + x;
        const std::string& want = colors_[i];
        if (want != cur_color) {
          if (color_open) { line += theme.reset(); color_open = false; }
          if (!want.empty()) { line += theme.sgr(want); color_open = !theme.sgr(want).empty(); }
          cur_color = want;
        }
        line += glyphs_[i];
      }
      if (color_open) line += theme.reset();
      lines.push_back(line);
    }
    return lines;
  }

private:
  int w_, h_;
  std::vector<std::string> glyphs_;
  std::vector<std::string> colors_;
};

struct PlacedRegion {
  RegionInfo info;
  double intensity = 0.0;
  int cx = 0;
  int cy = 0;
};

void draw_ellipse_outline(Canvas& c, double cx, double cy, double rx, double ry,
                          const std::string& glyph, const std::string& color) {
  int steps = static_cast<int>(std::max(24.0, (rx + ry) * 3.0));
  for (int i = 0; i < steps; ++i) {
    double a = 2.0 * kPi * i / steps;
    int x = static_cast<int>(std::lround(cx + rx * std::cos(a)));
    int y = static_cast<int>(std::lround(cy + ry * std::sin(a)));
    c.put_dim_if_empty(x, y, glyph, color);
  }
}

std::string pathway_color(const ColorTheme& theme, PathwayKind kind) {
  if (!theme.color_enabled()) return "";
  switch (kind) {
    case PathwayKind::Excitatory: return "38;5;65";   // muted green
    case PathwayKind::Inhibitory: return "38;5;131";  // muted red
    case PathwayKind::Modulatory: return "38;5;137";  // muted amber
  }
  return "38;5;245";
}

// Draw a Bresenham line between two cells, overwriting whatever is there but
// leaving the two endpoints alone (they carry the region blobs/labels).
void plot_pathway(Canvas& c, int x0, int y0, int x1, int y1, const std::string& glyph,
                  const std::string& color) {
  int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  int cx = x0, cy = y0;
  while (true) {
    bool endpoint = (cx == x0 && cy == y0) || (cx == x1 && cy == y1);
    if (!endpoint) c.put(cx, cy, glyph, color);
    if (cx == x1 && cy == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; cx += sx; }
    if (e2 <= dx) { err += dx; cy += sy; }
  }
}

// Overlay the configured pathways onto a canvas given each region's cell.
void draw_pathways(Canvas& c, const RenderOptions& opt,
                   const std::map<std::string, std::pair<int, int>>& region_pos) {
  if (!opt.show_pathways) return;
  for (const auto& p : PathwayCatalog::all()) {
    if (p.from == p.to) continue;  // self-connections aren't drawable
    auto a = region_pos.find(p.from);
    auto b = region_pos.find(p.to);
    if (a == region_pos.end() || b == region_pos.end()) continue;
    std::string glyph = opt.ascii_only ? (p.kind == PathwayKind::Inhibitory ? "x" : ".")
                                       : (p.kind == PathwayKind::Inhibitory ? "x" : "·");
    plot_pathway(c, a->second.first, a->second.second, b->second.first, b->second.second,
                 glyph, pathway_color(opt.theme, p.kind));
  }
}

// Render a coronal-ish heat-mapped slice of the brain.
std::vector<std::string> render_slice(const Simulation& sim, const RenderOptions& opt,
                                      int w, int h) {
  Canvas canvas(w, h);
  const auto& sample = sim.current_sample();
  double cx = (w - 1) / 2.0;
  double cy = (h - 1) / 2.0;
  double rx = (w - 2) / 2.0;
  double ry = (h - 2) / 2.0;
  if (rx < 2) rx = 2;
  if (ry < 2) ry = 2;

  std::vector<PlacedRegion> placed;
  for (const auto& info : RegionCatalog::all()) {
    PlacedRegion pr;
    pr.info = info;
    pr.intensity = sample.intensity_of(info.key);
    pr.cx = static_cast<int>(std::lround(cx + (info.slice_x - 0.5) * 2.0 * rx * 0.82));
    pr.cy = static_cast<int>(std::lround(cy + (info.slice_y - 0.5) * 2.0 * ry * 0.88));
    placed.push_back(pr);
  }

  // Heat map: each interior cell takes a distance-weighted blend of region
  // intensities. The brain silhouette is everything inside the ellipse.
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      double nx = (x - cx) / rx;
      double ny = (y - cy) / ry;
      double r2 = nx * nx + ny * ny;
      if (r2 > 1.0) continue;  // outside the brain
      double wsum = 0.0, vsum = 0.0;
      for (const auto& pr : placed) {
        double dx = static_cast<double>(x - pr.cx);
        double dy = static_cast<double>(y - pr.cy) * 2.0;  // cells are ~2:1
        double d2 = dx * dx + dy * dy;
        double sigma2 = std::max(4.0, (rx * ry) / 6.0);
        double weight = std::exp(-d2 / sigma2);
        wsum += weight;
        vsum += weight * pr.intensity;
      }
      double value = wsum > 0 ? vsum / wsum : 0.0;
      // Slight vignette toward the cortex edge to suggest the gyral surface.
      value *= 0.55 + 0.45 * (1.0 - std::sqrt(r2));
      canvas.put(x, y, shade(value, opt.ascii_only), ramp_params(opt.theme, value));
    }
  }

  // Connectivity overlay (drawn over the heat map, under the outline/labels).
  if (opt.show_pathways) {
    std::map<std::string, std::pair<int, int>> region_pos;
    for (const auto& pr : placed) region_pos[pr.info.key] = {pr.cx, pr.cy};
    draw_pathways(canvas, opt, region_pos);
  }

  // Midline + silhouette outline.
  draw_ellipse_outline(canvas, cx, cy, rx, ry,
                       opt.ascii_only ? "*" : "·", opt.theme.color_enabled() ? "38;5;240" : ");
  for (int y = 1; y < h - 1; ++y) {
    double ny = (y - cy) / ry;
    if (ny * ny <= 1.0) {
      canvas.put_dim_if_empty(static_cast<int>(std::lround(cx)), y,
                              opt.ascii_only ? "|" : "│",
                              opt.theme.color_enabled() ? "38;5;238" : ");
    }
  }

  // Region labels (abbreviations), highlighting the selected one.
  const auto& selected = sim.selected_region();
  for (const auto& pr : placed) {
    bool is_sel = selected && *selected == pr.info.key;
    std::string label = pr.info.abbreviation;
    int lx = pr.cx - static_cast<int>(label.size()) / 2;
    int ly = pr.cy;
    // Try to avoid drawing the label off the slice.
    lx = std::max(1, std::min(lx, w - 1 - static_cast<int>(label.size())));
    ly = std::max(1, std::min(ly, h - 2));
    std::string color;
    if (is_sel) color = opt.theme.color_enabled() ? "1;97;48;5;33" : ";
    else color = opt.theme.color_enabled() ? "1;38;5;231" : ";
    canvas.text(lx, ly, label, color);
    if (is_sel) {
      canvas.put_dim_if_empty(lx - 1, ly, ">", opt.theme.color_enabled() ? "1;93" : ");
    }
  }
  return canvas.to_lines(opt.theme);
}

// Render a rotating 3D ASCII projection of the modelled regions.
std::vector<std::string> render_projection(const Simulation& sim, const RenderOptions& opt,
                                           int w, int h) {
  Canvas canvas(w, h);
  double cx = (w - 1) / 2.0;
  double cy = (h - 1) / 2.0;
  double angle = static_cast<double>(sim.current_index()) * 0.12;
  double sx = (w - 2) / 2.0 * 0.92;
  double sy = (h - 2) / 2.0 * 0.92;

  auto project = [&](double rx, double ry, double rz, double& out_x, double& out_y, double& out_depth) {
    // rx,ry,rz in [-0.5, 0.5]. Rotate around the vertical (Y) axis.
    double X = rx * std::cos(angle) + rz * std::sin(angle);
    double Z = -rx * std::sin(angle) + rz * std::cos(angle);
    out_depth = Z;  // larger -> closer to viewer
    double persp = 1.0 + 0.35 * Z;
    out_x = cx + X * sx * 1.6 * persp;
    out_y = cy + (ry - 0.22 * Z) * sy * persp;
  };

  // Ellipsoid wireframe: a few latitude rings + meridians.
  std::string wire_color = opt.theme.color_enabled() ? "38;5;239" : ";
  for (double lat = -0.4; lat <= 0.401; lat += 0.2) {
    double rr = std::sqrt(std::max(0.0, 0.25 - lat * lat));
    int steps = 48;
    for (int i = 0; i < steps; ++i) {
      double a = 2.0 * kPi * i / steps;
      double px, py, pd;
      project(rr * std::cos(a), lat, rr * std::sin(a), px, py, pd);
      canvas.put_dim_if_empty(static_cast<int>(std::lround(px)),
                              static_cast<int>(std::lround(py)),
                              opt.ascii_only ? "." : "·", wire_color);
    }
  }
  for (double mer = 0.0; mer < kPi - 0.01; mer += kPi / 4.0) {
    int steps = 40;
    for (int i = 0; i < steps; ++i) {
      double t = kPi * i / steps - kPi / 2.0;
      double rr = std::cos(t) * 0.5;
      double yy = std::sin(t) * 0.5;
      double px, py, pd;
      project(rr * std::cos(mer), yy, rr * std::sin(mer), px, py, pd);
      canvas.put_dim_if_empty(static_cast<int>(std::lround(px)),
                              static_cast<int>(std::lround(py)),
                              opt.ascii_only ? "." : "·", wire_color);
    }
  }

  // Region blobs, painted far-to-near so nearer regions overdraw farther ones.
  struct Blob { RegionInfo info; double intensity; int x; int y; double depth; };
  std::vector<Blob> blobs;
  const auto& sample = sim.current_sample();
  for (const auto& info : RegionCatalog::all()) {
    double px, py, pd;
    project(info.slice_x - 0.5, info.slice_y - 0.5, info.depth - 0.5, px, py, pd);
    blobs.push_back({info, sample.intensity_of(info.key),
                     static_cast<int>(std::lround(px)), static_cast<int>(std::lround(py)), pd});
  }
  if (opt.show_pathways) {
    std::map<std::string, std::pair<int, int>> region_pos;
    for (const auto& b : blobs) region_pos[b.info.key] = {b.x, b.y};
    draw_pathways(canvas, opt, region_pos);
  }
  std::sort(blobs.begin(), blobs.end(), [](const Blob& a, const Blob& b) { return a.depth < b.depth; });

  const auto& selected = sim.selected_region();
  for (const auto& b : blobs) {
    double v = b.intensity * (0.7 + 0.3 * (b.depth + 0.5));
    std::string params = ramp_params(opt.theme, v);
    std::string g = shade(b.intensity, opt.ascii_only);
    // A 3x1 (plus optional center) blob.
    canvas.put(b.x, b.y, g, params);
    canvas.put_dim_if_empty(b.x - 1, b.y, g, params);
    canvas.put_dim_if_empty(b.x + 1, b.y, g, params);
    bool is_sel = selected && *selected == b.info.key;
    std::string label = b.info.abbreviation;
    int lx = b.x - static_cast<int>(label.size()) / 2;
    int ly = b.y - 1;
    lx = std::max(0, std::min(lx, w - static_cast<int>(label.size())));
    ly = std::max(0, std::min(ly, h - 1));
    std::string lcolor = is_sel ? (opt.theme.color_enabled() ? "1;97;48;5;33" : ")
                                : (opt.theme.color_enabled() ? "1;38;5;231" : ");
    canvas.text(lx, ly, label, lcolor);
  }
  return canvas.to_lines(opt.theme);
}

std::vector<std::string> chemistry_lines(const Simulation& sim, const RenderOptions& opt,
                                         int width) {
  std::vector<std::string> out;
  const auto& chem = sim.chemical_state();
  out.push_back(opt.theme.accent("Neurotransmitters"));
  int label_w = 16;
  int bar_w = std::max(8, width - label_w - 10);
  for (const auto& nt : Neurochemistry::catalog()) {
    double v = chem.count(nt.key) ? chem.at(nt.key) : nt.baseline;
    std::string row = pad_right(nt.display_name, label_w) + " " +
                      opt.theme.intensity(bar(v, bar_w, opt.ascii_only), v) + " " + fmt2(v);
    out.push_back(row);
  }
  // Chemical flow arrows.
  auto flows = sim.current_flows();
  if (!flows.empty()) {
    out.push_back(opt.theme.dim("Flow this step:"));
    int shown = 0;
    for (const auto& f : flows) {
      if (shown++ >= 4) break;
      const auto* nt = Neurochemistry::find(f.transmitter);
      std::string sym = nt ? nt->symbol : f.transmitter;
      std::string dir;
      if (f.magnitude > 0) {
        dir = (opt.ascii_only ? "^ +" : "▲ +") + fmt2(f.magnitude);
        if (!f.source_region.empty()) {
          auto ri = RegionCatalog::find(f.source_region);
          dir += (opt.ascii_only ? "  <- " : "  ← ") + (ri ? ri->abbreviation : f.source_region);
        }
      } else {
        dir = (opt.ascii_only ? "v " : "▼ ") + fmt2(f.magnitude) + "  (reuptake)";
      }
      std::string line = "  " + pad_right(sym, 5) + " " + dir;
      out.push_back(opt.theme.color_enabled() ? (f.magnitude > 0 ? opt.theme.secondary(line) : opt.theme.dim(line)) : line);
    }
  }
  return out;
}

std::vector<std::string> selected_region_lines(const Simulation& sim, const RenderOptions& opt,
                                               int width) {
  std::vector<std::string> out;
  if (!sim.selected_region()) {
    out.push_back(opt.theme.dim("(no region selected - press Tab to inspect one)"));
    return out;
  }
  std::string key = *sim.selected_region();
  auto info = RegionCatalog::find(key);
  std::string name = info ? info->display_name : key;
  out.push_back(opt.theme.accent("Region: ") + opt.theme.accent(name) +
                (RegionCatalog::is_region_of_interest(key) ? opt.theme.warning("  [ROI]") : "));

  // Stats over the whole timeline.
  double cur = sim.current_sample().intensity_of(key);
  double mn = 1e9, mx = -1e9, sum = 0.0;
  std::vector<double> series;
  for (const auto& s : sim.timeline().samples()) {
    double v = s.intensity_of(key);
    series.push_back(v);
    mn = std::min(mn, v);
    mx = std::max(mx, v);
    sum += v;
  }
  std::size_t n = series.size();
  if (n == 0) { mn = mx = 0; }
  double avg = n ? sum / n : 0.0;
  int bar_w = std::max(8, width - 24);
  out.push_back("  current  " + opt.theme.intensity(bar(cur, bar_w, opt.ascii_only), cur) + " " + fmt2(cur));
  out.push_back("  peak " + fmt2(mx) + "   mean " + fmt2(avg) + "   min " + fmt2(mn));
  if (info) {
    const auto* nt = Neurochemistry::find(info->primary_transmitter);
    out.push_back("  primary transmitter: " + (nt ? nt->display_name : info->primary_transmitter));
  }
  if (info && !info->extra.empty()) {
    std::string m = "  metadata:";
    for (const auto& kv : info->extra) m += "  " + kv.first + "=" + kv.second;
    out.push_back(m);
  }
  {
    auto conns = PathwayCatalog::incident_to(key);
    if (!conns.empty()) {
      out.push_back(opt.theme.dim("connections:"));
      int shown = 0;
      for (const auto& p : conns) {
        if (shown++ >= 4) break;
        std::string other = (p.from == key) ? p.to : p.from;
        auto ri = RegionCatalog::find(other);
        std::string oname = ri ? ri->abbreviation : other;
        std::string arrow = !p.directed ? "--" : (p.from == key ? "->" : "<-");
        out.push_back("  " + arrow + " " + oname + "  (" + fmt2(p.weight) + ", " +
                      std::string(to_string(p.kind)) + ")");
      }
    }
  }
  if (n > 1) {
    out.push_back("  trace " + opt.theme.secondary(sparkline(series, std::min<int>(static_cast<int>(width - 8), 48), opt.ascii_only)));
  }
  return out;
}

}  // namespace

Renderer::Renderer(RenderOptions options) : options_(std::move(options)) {}

std::string Renderer::intensity_glyph(double value, bool ascii_only) {
  return shade(value, ascii_only);
}

std::vector<std::string> Renderer::render_brain_view(const Simulation& sim, int view_width,
                                                     int view_height) const {
  if (options_.view == ViewMode::Projection3D) {
    return render_projection(sim, options_, view_width, view_height);
  }
  return render_slice(sim, options_, view_width, view_height);
}

std::vector<std::string> Renderer::render_frame(const Simulation& sim) const {
  std::vector<std::string> lines;
  const int W = std::max(40, options_.width);
  const int H = std::max(16, options_.height);

  // Title row.
  std::ostringstream title;
  title << "Brain Modeler  |  frame " << (sim.frame_count() ? sim.current_index() + 1 : 0)
        << "/" << sim.frame_count();
  if (sim.frame_count()) {
    title << "  t=" << sim.current_sample().timestamp_ms << "ms";
  }
  title << "  |  " << (sim.playing() ? "PLAY" : "PAUSE")
        << "  speed " << fmt2(sim.speed()) << "x"
        << "  |  view " << (options_.view == ViewMode::Projection3D ? "3D" : "2D")
        << "  |  theme " << options_.theme.display_name();
  lines.push_back(options_.theme.accent(pad_right(title.str(), static_cast<std::size_t>(W))));
  lines.push_back(options_.theme.dim(repeat(options_.ascii_only ? "-" : "─", W)));

  // Layout: left = brain view, right = chemistry + selected region.
  int reserved_top = static_cast<int>(lines.size());
  int reserved_bottom = options_.show_help_bar ? 2 : 0;
  int body_h = std::max(8, H - reserved_top - reserved_bottom);
  int right_w = options_.show_chemistry ? std::min(40, W / 2 - 2) : 0;
  int left_w = W - right_w - (right_w > 0 ? 2 : 0);
  if (left_w < 20) { left_w = W; right_w = 0; }

  auto brain = render_brain_view(sim, left_w, body_h);

  std::vector<std::string> right;
  if (right_w > 0) {
    auto chem = chemistry_lines(sim, options_, right_w);
    auto reg = selected_region_lines(sim, options_, right_w);
    right.insert(right.end(), chem.begin(), chem.end());
    right.push_back(");
    right.insert(right.end(), reg.begin(), reg.end());
  }

  for (int i = 0; i < body_h; ++i) {
    std::string left_cell = (i < static_cast<int>(brain.size())) ? brain[i] : std::string();
    // Pad the (possibly colorized) left cell to left_w *visible* columns. We
    // can't easily measure colored width, so the brain renderer already fills
    // exactly left_w columns; trust it. For plain rows pad with spaces.
    std::string line = left_cell;
    if (right_w > 0) {
      // The canvas always produces left_w glyph columns, so a literal space
      // gutter then the right column text is good enough for a TUI.
      std::string r = (i < static_cast<int>(right.size())) ? right[i] : std::string();
      line += "  " + r;
    }
    lines.push_back(line);
  }

  if (options_.show_help_bar) {
    lines.push_back(options_.theme.dim(repeat(options_.ascii_only ? "-" : "─", W)));
    lines.push_back(options_.theme.dim(
        "[space]play/pause  [<-/->]step  [r]ewind  [f]fwd  [Tab]region  "
        "[2/3]view  [t]heme  [p]aths  [T]our  [q]uit"));
  }
  if (static_cast<int>(lines.size()) > H) lines.resize(static_cast<std::size_t>(H));
  return lines;
}

Renderer::FrameLayout Renderer::frame_layout() const {
  const int W = std::max(40, options_.width);
  const int H = std::max(16, options_.height);
  int reserved_top = 2;  // title + separator
  int reserved_bottom = options_.show_help_bar ? 2 : 0;
  int body_h = std::max(8, H - reserved_top - reserved_bottom);
  int right_w = options_.show_chemistry ? std::min(40, W / 2 - 2) : 0;
  int left_w = W - right_w - (right_w > 0 ? 2 : 0);
  if (left_w < 20) left_w = W;
  FrameLayout l;
  l.view_x = 1;                 // 1-based terminal column
  l.view_y = reserved_top + 1;  // 1-based terminal row
  l.view_width = left_w;
  l.view_height = body_h;
  return l;
}

std::optional<std::string> Renderer::region_at(const Simulation& sim, int view_width,
                                               int view_height, int x, int y) const {
  if (view_width < 4 || view_height < 4) return std::nullopt;
  if (x < 0 || y < 0 || x >= view_width || y >= view_height) return std::nullopt;
  double best_d2 = 1e18;
  std::string best_key;
  if (options_.view == ViewMode::Projection3D) {
    double cx = (view_width - 1) / 2.0;
    double cy = (view_height - 1) / 2.0;
    double angle = static_cast<double>(sim.current_index()) * 0.12;
    double sx = (view_width - 2) / 2.0 * 0.92;
    double sy = (view_height - 2) / 2.0 * 0.92;
    for (const auto& info : RegionCatalog::all()) {
      double rx = info.slice_x - 0.5, ry = info.slice_y - 0.5, rz = info.depth - 0.5;
      double X = rx * std::cos(angle) + rz * std::sin(angle);
      double Z = -rx * std::sin(angle) + rz * std::cos(angle);
      double persp = 1.0 + 0.35 * Z;
      double px = cx + X * sx * 1.6 * persp;
      double py = cy + (ry - 0.22 * Z) * sy * persp;
      double dx = x - px, dy = (y - py) * 2.0;
      double d2 = dx * dx + dy * dy;
      if (d2 < best_d2) { best_d2 = d2; best_key = info.key; }
    }
  } else {
    double cx = (view_width - 1) / 2.0;
    double cy = (view_height - 1) / 2.0;
    double rx = std::max(2.0, (view_width - 2) / 2.0);
    double ry = std::max(2.0, (view_height - 2) / 2.0);
    double nx = (x - cx) / rx, ny = (y - cy) / ry;
    if (nx * nx + ny * ny > 1.3) return std::nullopt;  // clicked outside the brain
    for (const auto& info : RegionCatalog::all()) {
      double rcx = cx + (info.slice_x - 0.5) * 2.0 * rx * 0.82;
      double rcy = cy + (info.slice_y - 0.5) * 2.0 * ry * 0.88;
      double dx = x - rcx, dy = (y - rcy) * 2.0;
      double d2 = dx * dx + dy * dy;
      if (d2 < best_d2) { best_d2 = d2; best_key = info.key; }
    }
  }
  if (best_key.empty()) return std::nullopt;
  return best_key;
}

std::string Renderer::render_report(const Simulation& sim, int width, ViewMode view,
                                    bool ascii_only) {
  width = std::max(40, width);
  std::ostringstream os;
  os << "Brain Modeler - simulation report\n";
  os << repeat("=", width) << "\n";
  os << "frames      : " << sim.frame_count() << "\n";
  if (sim.frame_count()) {
    os << "duration    : " << sim.timeline().duration_ms() << " ms\n";
    os << "start ts    : " << sim.timeline().samples().front().timestamp_ms << " ms\n";
    os << "end ts      : " << sim.timeline().samples().back().timestamp_ms << " ms\n";
  }
  os << "view        : " << (view == ViewMode::Projection3D ? "3D projection" : "2D slice") << "\n\n";

  // Per-region table.
  os << "Region activity\n" << repeat("-", width) << "\n";
  int name_w = 28;
  int spark_w = std::max(12, width - name_w - 26);
  os << pad_right("region", name_w) << " ROI  peak  mean  last  trace\n";
  for (const auto& info : RegionCatalog::all()) {
    std::vector<double> series;
    double mx = 0.0, sum = 0.0, last = 0.0;
    for (const auto& s : sim.timeline().samples()) {
      double v = s.intensity_of(info.key);
      series.push_back(v);
      mx = std::max(mx, v);
      sum += v;
      last = v;
    }
    double mean = series.empty() ? 0.0 : sum / series.size();
    os << pad_right(info.display_name, name_w) << " "
       << (info.region_of_interest ? "yes " : " no ") << " "
       << fmt2(mx) << "  " << fmt2(mean) << "  " << fmt2(last) << "  "
       << sparkline(series, spark_w, ascii_only) << "\n";
  }
  os << "\n";

  // Region metadata (only shown when a configured catalog supplies any).
  bool any_meta = false;
  for (const auto& info : RegionCatalog::all()) {
    if (!info.extra.empty()) { any_meta = true; break; }
  }
  if (any_meta) {
    os << "Region metadata\n" << repeat("-", width) << "\n";
    for (const auto& info : RegionCatalog::all()) {
      if (info.extra.empty()) continue;
      std::string line = pad_right(info.display_name, name_w) + " ";
      bool first = true;
      for (const auto& kv : info.extra) {
        if (!first) line += ", ";
        first = false;
        line += kv.first + "=" + kv.second;
      }
      os << line << "\n";  // metadata lines may exceed `width`
    }
    os << "\n";
  }

  // Pathways (only shown when a connectivity catalog is loaded).
  if (!PathwayCatalog::all().empty()) {
    os << "Pathways\n" << repeat("-", width) << "\n";
    for (const auto& p : PathwayCatalog::all()) {
      auto rf = RegionCatalog::find(p.from);
      auto rt = RegionCatalog::find(p.to);
      std::string fn = rf ? rf->display_name : p.from;
      std::string tn = rt ? rt->display_name : p.to;
      std::string line = fn + (p.directed ? " -> " : " -- ") + tn + "  weight " + fmt2(p.weight) +
                         "  delay " + std::to_string(p.delay_ms) + "ms  " +
                         std::string(to_string(p.kind));
      if (!p.transmitter.empty()) line += "  (" + p.transmitter + ")";
      for (const auto& kv : p.extra) line += "  " + kv.first + "=" + kv.second;
      os << line << "\n";  // pathway lines may exceed `width`
    }
    os << "\n";
  }

  // Final neurochemistry.
  os << "Neurochemistry (final frame)\n" << repeat("-", width) << "\n";
  const auto& chem = sim.chemical_state();
  for (const auto& nt : Neurochemistry::catalog()) {
    double v = chem.count(nt.key) ? chem.at(nt.key) : nt.baseline;
    os << pad_right(nt.display_name, 18) << " " << bar(v, std::max(10, width - 30), ascii_only)
       << " " << fmt2(v) << "\n";
  }
  os << "\n";

  // A rendered brain view of the final frame.
  os << (view == ViewMode::Projection3D ? "3D projection (final frame)\n" : "Coronal slice (final frame)\n");
  os << repeat("-", width) << "\n";
  RenderOptions ropt;
  ropt.width = width;
  ropt.ascii_only = ascii_only;
  ropt.view = view;
  ropt.show_pathways = !PathwayCatalog::all().empty();
  ropt.theme = ThemeLibrary::default_theme(false);
  Renderer r(ropt);
  int view_h = std::max(14, std::min(28, width / 3));
  for (const auto& l : r.render_brain_view(sim, width, view_h)) os << l << "\n";
  os << "\n";

  // Legend.
  os << "intensity legend: ";
  for (int i = 0; i <= 8; ++i) os << shade(i / 8.0, ascii_only);
  os << "  (low -> high)\n";
  return os.str();
}

}  // namespace cerebra

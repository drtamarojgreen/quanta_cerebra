#include "visualization/view_2d.h"
#include "core/atlas_core.h"
#include "ui/terminal_renderer.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace cerebra {

namespace {

constexpr int kBaseW = 38; // intrinsic slice template width
constexpr int kBaseH = 22; // intrinsic slice template height

double intensity_for(const cerebra::BrainFrame& f, const std::string& id) {
    for (const auto& r : f.regions) if (r.region == id) return r.intensity;
    return 0.0;
}

}

std::string render_2d_slice(const cerebra::BrainFrame& frame, int width, const Theme& theme,
                            const std::string& highlight) {
    if (width < 50) width = 50;
    int inner_w = width - 4;
    if (inner_w < kBaseW) inner_w = kBaseW;
    int scale_w = inner_w / kBaseW;
    if (scale_w < 1) scale_w = 1;
    int slice_w = kBaseW * scale_w;

    std::vector<std::string> grid(kBaseH, std::string(slice_w, ' '));

    // Build an outline of the brain
    auto plot = [&](int r, int c, char ch) {
        if (r < 0 || r >= kBaseH) return;
        if (c < 0 || c >= slice_w) return;
        grid[r][c] = ch;
    };

    // Outer skull arc – approximate an oval.
    double cy = (kBaseH - 1) / 2.0;
    double cx = (slice_w - 1) / 2.0;
    double rx = slice_w / 2.0 - 1.0;
    double ry = (kBaseH - 1) / 2.0 - 0.5;
    for (int r = 0; r < kBaseH; ++r) {
        for (int c = 0; c < slice_w; ++c) {
            double dy = (r - cy) / ry;
            double dx = (c - cx) / rx;
            double d = dx * dx + dy * dy;
            if (d > 0.9 && d <= 1.05) {
                char ch = '.';
                if (r == 0 || r == kBaseH - 1) ch = '_';
                else if (c < cx) ch = '(';
                else ch = ')';
                plot(r, c, ch);
            }
        }
    }

    // Place each region as a filled rectangle of intensity blocks. We use
    // single-byte ASCII glyphs so the grid stays in fixed-byte alignment
    // (multi-byte UTF-8 would break overlapping writes / clipping math).
    for (const auto& region : known_regions()) {
        double inten = intensity_for(frame, region.id);
        char block = grayscale_block(inten);
        int col0 = static_cast<int>((region.slice_col / static_cast<double>(kBaseW)) * slice_w);
        int colw = std::max(1, static_cast<int>((region.slice_w / static_cast<double>(kBaseW)) * slice_w));
        for (int dr = 0; dr < region.slice_h; ++dr) {
            int r = region.slice_row + dr;
            if (r < 0 || r >= kBaseH) continue;
            for (int dc = 0; dc < colw; ++dc) {
                int c = col0 + dc;
                if (c < 0 || c >= slice_w) continue;
                grid[r][c] = block;
            }
        }
    }

    std::ostringstream out;
    out << ansi(theme.title_color) << "+--- 2D coronal slice ---" << std::string(slice_w - 22, '-')
        << "+" << ansi_reset() << '\n';
    for (int r = 0; r < kBaseH; ++r) {
        out << ansi(theme.title_color) << "| " << ansi_reset();
        out << grid[r];
        out << ansi(theme.title_color) << " |" << ansi_reset() << '\n';
    }
    out << ansi(theme.title_color) << "+" << std::string(slice_w + 2, '-') << "+" << ansi_reset() << '\n';

    // Legend with highlight
    out << "  Regions: ";
    bool first = true;
    for (const auto& region : known_regions()) {
        double inten = intensity_for(frame, region.id);
        if (!first) out << "  ";
        first = false;
        const Theme& t = theme;
        const std::string& color = (region.id == highlight) ? t.warn_color : t.label_color;
        out << ansi(color) << region.display_name << ansi_reset()
            << " [" << grayscale_block(inten) << " "
            << std::fixed << std::setprecision(2) << inten << "]";
    }
    out << '\n';

    return out.str();
}

std::string render_region_table(const cerebra::BrainFrame& frame, int width, const Theme& theme,
                                const std::string& highlight) {
    (void)width;
    std::ostringstream out;
    out << ansi(theme.title_color)
        << "Region                       Intensity  Bar          Neurotransmitters\n"
        << ansi_reset();
    auto lookup = [&](const std::string& id) -> const cerebra::RegionState* {
        for (const auto& r : frame.regions) if (r.region == id) return &r;
        return nullptr;
    };
    for (const auto& info : known_regions()) {
        const cerebra::RegionState* rs = lookup(info.id);
        double inten = rs ? rs->intensity : 0.0;
        int bar = static_cast<int>(inten * 10);
        std::string b;
        for (int i = 0; i < 10; ++i) b += (i < bar) ? "#" : "-";
        const std::string& color = (info.id == highlight) ? theme.warn_color : theme.label_color;
        out << ansi(color) << std::left << std::setw(28) << info.display_name << ansi_reset()
            << " " << std::fixed << std::setprecision(2) << std::setw(7) << inten
            << "  [" << b << "]  ";
        if (rs) {
            bool first = true;
            for (const auto& nf : rs->flows) {
                if (!first) out << ", ";
                first = false;
                out << nf.type << "=" << std::fixed
                    << std::setprecision(2) << nf.rate;
            }
            if (!rs->metrics.empty()) {
                out << "  |  ";
                bool first_metric = true;
                for (const auto& kv : rs->metrics) {
                    if (!first_metric) out << ", ";
                    first_metric = false;
                    out << kv.first << "=" << std::fixed << std::setprecision(2) << kv.second;
                }
            }
        }
        out << '\n';
    }
    return out.str();
}

double pathway_activation(const PathwayDefinition& p, const cerebra::BrainFrame& frame) {
    if (p.nodes.empty()) return 0.0;
    double sum = 0.0;
    int counted = 0;
    for (const auto& node : p.nodes) {
        for (const auto& r : frame.regions) {
            if (r.region == node) { sum += r.intensity; ++counted; break; }
        }
    }
    if (counted == 0) return 0.0;
    double mean = sum / counted;
    return mean * p.strength;
}

std::string render_pathways_table(const cerebra::BrainFrame& frame, int width, const Theme& theme,
                                  const std::string& highlight) {
    (void)width;
    std::ostringstream out;
    const auto& paths = current_atlas().pathways();
    if (paths.empty()) return out.str();

    out << ansi(theme.title_color)
        << "Pathway                       Strength  Activation  Route\n"
        << ansi_reset();
    for (const auto& p : paths) {
        double act = pathway_activation(p, frame);
        // Build a route string from the regions' display names when available.
        std::string route;
        for (std::size_t i = 0; i < p.nodes.size(); ++i) {
            if (i) route += p.bidirectional ? " <-> " : " -> ";
            const cerebra::RegionDefinition* def = find_region(p.nodes[i]);
            route += def ? def->display_name : p.nodes[i];
        }
        const std::string& color = (p.id == highlight) ? theme.warn_color : theme.label_color;
        out << ansi(color) << std::left << std::setw(28)
            << (p.name.empty() ? p.id : p.name) << ansi_reset()
            << "  " << std::fixed << std::setprecision(2) << std::setw(6) << p.strength
            << "    " << std::setw(6) << act
            << "      " << route;
        if (!p.transmitter.empty()) out << "  [" << p.transmitter << "]";
        out << '\n';
    }
    return out.str();
}

}

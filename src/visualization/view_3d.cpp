#include "visualization/view_3d.h"
#include "core/atlas_core.h"
#include "ui/terminal_renderer.h"

#include <cmath>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace cerebra {

namespace {

double intensity_for(const BrainFrame& f, const std::string& id) {
    for (const auto& r : f.regions) if (r.region == id) return r.intensity;
    return 0.0;
}

}

std::string render_3d_projection(const BrainFrame& frame, int width, int height,
                                 const Theme& theme, double yaw_deg,
                                 const std::string& highlight) {
    if (width < 40) width = 40;
    if (height < 16) height = 16;

    std::vector<std::string> canvas(height, std::string(width, ' '));
    std::vector<std::vector<double>> depth(height, std::vector<double>(width, 1e9));
    std::vector<std::vector<std::string>> color_grid(height, std::vector<std::string>(width));

    double yaw = yaw_deg * M_PI / 180.0;
    double cy = std::cos(yaw), sy = std::sin(yaw);

    auto project = [&](double x, double y, double z, int& px, int& py, double& pz) {
        double xr = cy * x + sy * z;
        double zr = -sy * x + cy * z;
        // map roughly [-1.2, 1.2] to canvas
        double fx = (xr + 1.2) / 2.4;
        double fy = (1.0 - y) / 2.0; // y up
        px = static_cast<int>(fx * (width - 1));
        py = static_cast<int>(fy * (height - 1));
        pz = zr;
    };

    // Draw skull silhouette (ellipsoid wireframe sampled in two angle loops).
    for (int i = 0; i < 36; ++i) {
        double t = i * (2 * M_PI / 36);
        for (int j = 0; j < 24; ++j) {
            double u = -M_PI / 2 + j * (M_PI / 24);
            double x = 1.0 * std::cos(u) * std::cos(t);
            double y = 0.95 * std::sin(u);
            double z = 1.05 * std::cos(u) * std::sin(t);
            int px, py; double pz;
            project(x, y, z, px, py, pz);
            if (px >= 0 && px < width && py >= 0 && py < height) {
                if (pz < depth[py][px]) {
                    depth[py][px] = pz;
                    canvas[py][px] = '.';
                    color_grid[py][px] = theme.accent_color;
                }
            }
        }
    }

    // Render regions as soft circular blobs.
    auto block_for = [](double intensity) {
        if (intensity < 0.2) return ".";
        if (intensity < 0.4) return ":";
        if (intensity < 0.6) return "o";
        if (intensity < 0.8) return "O";
        return "@";
    };

    for (const auto& g : current_atlas().regions()) {
        double inten = intensity_for(frame, g.id);
        if (inten <= 0.0) continue;
        int px, py; double pz;
        project(g.proj_x, g.proj_y, g.proj_z, px, py, pz);
        int rpx = std::max(2, static_cast<int>(g.proj_radius * (width / 4.0)));
        int rpy = std::max(1, static_cast<int>(g.proj_radius * (height / 3.0)));
        const char* glyph = block_for(inten);
        for (int dy = -rpy; dy <= rpy; ++dy) {
            for (int dx = -rpx; dx <= rpx; ++dx) {
                double fx = static_cast<double>(dx) / rpx;
                double fy = static_cast<double>(dy) / rpy;
                if (fx * fx + fy * fy > 1.0) continue;
                int x = px + dx;
                int y = py + dy;
                if (x < 0 || x >= width || y < 0 || y >= height) continue;
                if (pz < depth[y][x]) {
                    depth[y][x] = pz;
                    canvas[y][x] = glyph[0];
                    color_grid[y][x] = (g.id == highlight) ? theme.warn_color : theme.accent_color;
                }
            }
        }

        // Label
        {
            const std::string& label = g.display_name;
            int lx = px - static_cast<int>(label.size() / 2);
            int ly = py + rpy + 1;
            if (ly < height) {
                for (std::size_t i = 0; i < label.size(); ++i) {
                    int x = lx + static_cast<int>(i);
                    if (x < 0 || x >= width) continue;
                    canvas[ly][x] = label[i];
                    color_grid[ly][x] = (g.id == highlight) ? theme.warn_color : theme.label_color;
                }
            }
        }
    }

    std::ostringstream out;
    out << ansi(theme.title_color) << "+--- 3D ASCII projection (yaw "
        << static_cast<int>(yaw_deg) << "°) ---" << std::string(width > 36 ? width - 35 : 1, '-')
        << "+" << ansi_reset() << '\n';
    std::string current_color;
    for (int r = 0; r < height; ++r) {
        out << ansi(theme.title_color) << "|" << ansi_reset();
        current_color.clear();
        for (int c = 0; c < width; ++c) {
            const std::string& col = color_grid[r][c];
            if (col != current_color) {
                if (!current_color.empty()) out << ansi_reset();
                if (!col.empty()) out << ansi(col);
                current_color = col;
            }
            out << canvas[r][c];
        }
        if (!current_color.empty()) out << ansi_reset();
        out << ansi(theme.title_color) << "|" << ansi_reset() << '\n';
    }
    out << ansi(theme.title_color) << "+" << std::string(width, '-') << "+" << ansi_reset() << '\n';
    return out.str();
}

}

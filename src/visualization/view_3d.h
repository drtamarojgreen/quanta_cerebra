#pragma once

#include "core/state_manager.h"
#include "ui/visual_themes.h"

#include <string>

namespace cerebra {

// Render a 3D ASCII projection (parallel projection with simple shading) of the
// brain regions in the given frame.
std::string render_3d_projection(const cerebra::BrainFrame& frame, int width, int height,
                                 const Theme& theme, double yaw_deg = 35.0,
                                 const std::string& highlight = {});

}

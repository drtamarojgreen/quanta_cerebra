#pragma once

#include "core/state_manager.h"
#include "ui/visual_themes.h"

#include <string>

namespace cerebra {

// Render a 2D coronal-slice ASCII representation of a single cerebra::BrainFrame.
// width is the target terminal width; the slice scales horizontally to fit.
std::string render_2d_slice(const cerebra::BrainFrame& frame, int width, const Theme& theme,
                            const std::string& highlight = {});

// Plain-text region status table.
std::string render_region_table(const cerebra::BrainFrame& frame, int width, const Theme& theme,
                                const std::string& highlight = {});

// Plain-text neurological pathway table. Activation for each pathway is the
// mean intensity of its node regions in `frame` multiplied by the pathway's
// baseline strength from the current atlas.
double pathway_activation(const PathwayDefinition& p, const cerebra::BrainFrame& frame);

std::string render_pathways_table(const cerebra::BrainFrame& frame, int width, const Theme& theme,
                                  const std::string& highlight = {});

}

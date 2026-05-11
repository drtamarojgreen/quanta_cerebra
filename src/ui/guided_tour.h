#pragma once

#include "ui/visual_themes.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace cerebra {

struct TourStep {
    std::string title;
    std::string body;
};

const std::vector<TourStep>& tour_script();

// Print the tour as plain text to the given stream. Returns 0 on success.
int run_tour(std::ostream& out, const Theme& theme);

}

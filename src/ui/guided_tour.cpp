#include "ui/guided_tour.h"
#include "ui/terminal_renderer.h"

#include <ostream>

namespace cerebra {

const std::vector<TourStep>& tour_script() {
    static const std::vector<TourStep> steps = {
        {"Welcome",
         "Brain Modeler simulates brain activity as ASCII art. This guided\n"
         "tour walks you through every feature available in interactive mode."},
        {"Playback controls",
         "Space pauses and resumes. Left/Right arrows rewind and fast-forward\n"
         "one frame. '<' and '>' jump 10 frames. '+'/'-' change playback speed."},
        {"Region selection",
         "Press a number (1-9, 0) to highlight a specific region of interest –\n"
         "amygdala, cuneus, ACC, insula, gyrus, hippocampus, thalamus,\n"
         "entorhinal cortex, prefrontal cortex, or occipital lobe."},
        {"Themes",
         "Press 't' to cycle through color themes if your terminal supports\n"
         "256-color ANSI sequences."},
        {"Display modes",
         "The default view shows a 2D coronal slice with grayscale block\n"
         "intensities; press 'v' to toggle the 3D ASCII projection."},
        {"Mouse support",
         "If your terminal supports SGR mouse mode, click any region on the\n"
         "2D slice to highlight it and see detailed neurotransmitter flows."},
        {"Serial / IoT",
         "Pass --serial <device> to stream frames live from an experimental\n"
         "IoT device. Templates --template focused|relaxed|stressed|rem_sleep\n"
         "drive the simulation when no input data is supplied."},
        {"Exiting",
         "Press 'q' or Esc to quit. Have fun exploring."},
    };
    return steps;
}

int run_tour(std::ostream& out, const Theme& theme) {
    out << ansi(theme.title_color) << "=== Brain Modeler interactive tour ===" << ansi_reset() << "\n\n";
    int n = 1;
    for (const auto& step : tour_script()) {
        out << ansi(theme.accent_color) << n << ". " << step.title << ansi_reset() << '\n';
        out << step.body << "\n\n";
        ++n;
    }
    out << ansi(theme.label_color)
        << "(End of tour. Launch with --interactive to try it live.)" << ansi_reset() << '\n';
    return 0;
}

}

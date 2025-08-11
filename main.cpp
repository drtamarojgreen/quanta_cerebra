#include <iostream>
#include <fstream>
#include "json_logic.h"
#include "video_logic.h"
#include "config.h"

int main() {
    // Load application configuration
    AppConfig config = loadConfig("config.ini");

    // For verification, print the loaded configuration
    std::cout << "--- Configuration Settings ---" << std::endl;
    std::cout << "  Enable Color: " << (config.enable_color ? "true" : "false") << std::endl;
    std::cout << "  Smoothing Window: " << config.smoothing_window_size << std::endl;
    std::cout << "  Layout Mode: " << config.layout_mode << std::endl;
    std::cout << "----------------------------" << std::endl << std::endl;

    std::ifstream inputFile("sample_input.json");
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input file.\n";
        return 1;
    }

    std::string jsonData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    std::vector<BrainFrame> frames = parseBrainActivityJSON(jsonData);
    std::vector<BrainFrame> interpolatedFrames = interpolateFrames(frames, 4);
    std::vector<std::string> visualFrames = generateFrames(interpolatedFrames, config);

    for (const std::string& frame : visualFrames) {
        std::cout << frame << "\n";
        // Simple delay simulation without threading
        for (volatile int i = 0; i < 10000000; ++i) {
            // Busy wait for demonstration
        }
    }

    return 0;
}

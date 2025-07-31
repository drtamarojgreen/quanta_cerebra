#include <iostream>
#include <fstream>
#include "json_logic.h"
#include "video_logic.h"

int main() {
    std::ifstream inputFile("sample_input.json");
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input file.\n";
        return 1;
    }

    std::string jsonData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    std::vector<BrainFrame> frames = parseBrainActivityJSON(jsonData);
    std::vector<std::string> visualFrames = generateFrames(frames);

    for (const std::string& frame : visualFrames) {
        std::cout << frame << "\n";
        // Simple delay simulation without threading
        for (volatile int i = 0; i < 10000000; ++i) {
            // Busy wait for demonstration
        }
    }

    return 0;
}

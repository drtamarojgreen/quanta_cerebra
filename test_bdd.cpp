#include "json_logic.h"
#include "video_logic.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>

int main() {
    std::ifstream in("sample_input.json");
    std::ifstream expected("expected_output.txt");
    if (!in.is_open() || !expected.is_open()) {
        std::cerr << "Missing test files.\n";
        return 1;
    }

    std::string json((std::istreambuf_iterator<char>(in)), {});
    std::string expectedOut((std::istreambuf_iterator<char>(expected)), {});

    auto frames = parseBrainActivityJSON(json);

    // Use default config for testing to ensure colors are off, matching the expected output
    AppConfig test_config;
    auto visuals = generateFrames(frames, test_config);

    std::ostringstream combined;
    for (const auto& f : visuals) combined << f << "\n";

    if (combined.str().find(trim(expectedOut)) != std::string::npos) {
        std::cout << "BDD Test Passed\n";
    } else {
        std::cerr << "BDD Test Failed\n";
    }
    return 0;
}

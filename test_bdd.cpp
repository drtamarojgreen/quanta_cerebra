#include "json_logic.h"
#include "video_logic.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

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
    auto visuals = generateFrames(frames);

    std::ostringstream combined;
    for (const auto& f : visuals) combined << f << "\n";

    if (combined.str().find(trim(expectedOut)) != std::string::npos) {
        std::cout << "BDD Test Passed\n";
    } else {
        std::cerr << "BDD Test Failed\n";
    }
    return 0;
}

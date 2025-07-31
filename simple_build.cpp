// QuantaCerebra - Single File Version for Testing
// Compile with: g++ -std=c++11 simple_build.cpp -o QuantaCerebra

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

// Data structures
struct BrainRegion {
    std::string region_name;
    double intensity;
};

struct BrainFrame {
    int timestamp_ms;
    std::vector<BrainRegion> regions;
};

// Utility function
std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// JSON parsing function
std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json) {
    std::vector<BrainFrame> frames;
    std::size_t pos = 0;
    while ((pos = json.find("{", pos)) != std::string::npos) {
        BrainFrame frame;
        frame.timestamp_ms = 0;
        frame.regions.clear();

        std::size_t end = json.find("}", pos);
        std::string block = json.substr(pos, end - pos);
        pos = end + 1;

        std::size_t timePos = block.find("timestamp_ms");
        if (timePos != std::string::npos) {
            std::size_t colon = block.find(":", timePos);
            std::size_t comma = block.find(",", colon);
            std::string val = block.substr(colon + 1, comma - colon - 1);
            frame.timestamp_ms = std::stoi(trim(val));
        }

        std::size_t actPos = block.find("brain_activity");
        if (actPos != std::string::npos) {
            std::size_t startArray = block.find("[", actPos);
            std::size_t endArray = block.find("]", startArray);
            std::string arr = block.substr(startArray + 1, endArray - startArray - 1);

            std::istringstream iss(arr);
            std::string line;
            while (std::getline(iss, line, '}')) {
                std::size_t rPos = line.find("region");
                std::size_t iPos = line.find("intensity");
                if (rPos != std::string::npos && iPos != std::string::npos) {
                    std::size_t rStart = line.find("\"", rPos);
                    std::size_t rEnd = line.find("\"", rStart + 1);
                    std::string region = line.substr(rStart + 1, rEnd - rStart - 1);

                    std::size_t colon = line.find(":", iPos);
                    std::string intensityVal = line.substr(colon + 1);
                    double intensity = std::stod(trim(intensityVal));

                    frame.regions.push_back({region, intensity});
                }
            }
        }

        frames.push_back(frame);
    }
    return frames;
}

// Visualization functions
char intensityToChar(double intensity) {
    if (intensity < 0.2) return ' ';
    if (intensity < 0.4) return '.';
    if (intensity < 0.6) return ':';
    if (intensity < 0.75) return '+';
    if (intensity < 0.9) return 'X';
    return '@';
}

std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input) {
    std::vector<std::string> result;
    for (const BrainFrame& frame : input) {
        std::ostringstream oss;
        oss << "[Time: " << frame.timestamp_ms << " ms]\n";
        for (const BrainRegion& region : frame.regions) {
            oss << "[" << region.region_name << "] ";
            int barWidth = static_cast<int>(region.intensity * 10);
            for (int i = 0; i < barWidth; ++i) {
                oss << intensityToChar(region.intensity);
            }
            oss << "\n";
        }
        result.push_back(oss.str());
    }
    return result;
}

// Main function
int main() {
    std::cout << "QuantaCerebra - Brain Activity Visualizer\n";
    std::cout << "==========================================\n\n";
    
    std::ifstream inputFile("sample_input.json");
    if (!inputFile.is_open()) {
        std::cerr << "Error: Failed to open sample_input.json\n";
        std::cerr << "Please ensure the file exists in the current directory.\n";
        return 1;
    }

    std::string jsonData((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    std::vector<BrainFrame> frames = parseBrainActivityJSON(jsonData);
    std::vector<std::string> visualFrames = generateFrames(frames);

    std::cout << "Visualizing " << frames.size() << " brain activity frames:\n\n";
    
    for (const std::string& frame : visualFrames) {
        std::cout << frame << "\n";
        // Simple delay simulation without threading
        for (volatile int i = 0; i < 5000000; ++i) {
            // Busy wait for demonstration
        }
    }

    std::cout << "\nVisualization complete!\n";
    return 0;
}

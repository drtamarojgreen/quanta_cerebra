#include "json_logic.h"
#include <sstream>
#include <algorithm>
#include <cstddef>

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

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

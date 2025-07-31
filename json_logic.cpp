#include "json_logic.h"
#include <sstream>
#include <algorithm>
#include <cstddef>

size_t find_matching_brace(const std::string& s, size_t start, char open, char close) {
    int level = 1;
    for (size_t i = start + 1; i < s.length(); ++i) {
        if (s[i] == open) level++;
        else if (s[i] == close) {
            level--;
            if (level == 0) return i;
        }
    }
    return std::string::npos;
}

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json) {
    std::vector<BrainFrame> frames;
    std::size_t pos = 0;

    std::size_t array_start = json.find('[');
    if (array_start == std::string::npos) return frames;

    pos = array_start + 1;

    while ((pos = json.find("{", pos)) < json.length()) {
        BrainFrame frame;
        frame.timestamp_ms = 0;
        frame.regions.clear();

        std::size_t end = find_matching_brace(json, pos, '{', '}');
        if (end == std::string::npos) break;

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
            std::size_t endArray = find_matching_brace(block, startArray, '[', ']');
            if (endArray == std::string::npos) continue;

            std::string arr = block.substr(startArray + 1, endArray - startArray - 1);

            std::size_t region_pos = 0;
            while((region_pos = arr.find("{", region_pos)) != std::string::npos) {
                std::size_t region_end = find_matching_brace(arr, region_pos, '{', '}');
                if (region_end == std::string::npos) break;

                std::string line = arr.substr(region_pos, region_end - region_pos);
                region_pos = region_end + 1;

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

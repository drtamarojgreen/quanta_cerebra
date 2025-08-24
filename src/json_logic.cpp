#include "json_logic.h"
#include <sstream>
#include <algorithm>
#include <cstddef>

// Forward declarations
std::vector<BrainRegion> parseRegions(const std::string& arr_str);
size_t find_matching_brace(const std::string& s, size_t start, char open, char close);
BrainRegion parseRegion(const std::string& obj_str);

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

BrainRegion parseRegion(const std::string& obj_str) {
    BrainRegion region;
    region.intensity = 0;

    // Parse "region"
    size_t region_name_pos = obj_str.find("\"region\"");
    if (region_name_pos != std::string::npos) {
        size_t colon_pos = obj_str.find(':', region_name_pos);
        size_t quote_start = obj_str.find('"', colon_pos);
        size_t quote_end = obj_str.find('"', quote_start + 1);
        if (quote_start != std::string::npos && quote_end != std::string::npos) {
            region.region_name = obj_str.substr(quote_start + 1, quote_end - quote_start - 1);
        }
    }

    // Parse "intensity"
    size_t intensity_pos = obj_str.find("\"intensity\"");
    if (intensity_pos != std::string::npos) {
        size_t colon_pos = obj_str.find(':', intensity_pos);
        if (colon_pos != std::string::npos) {
            size_t comma_pos = obj_str.find(',', colon_pos);
            size_t brace_pos = obj_str.find('}', colon_pos);
            size_t end_pos = (comma_pos != std::string::npos && comma_pos < brace_pos) ? comma_pos : brace_pos;
            if (end_pos == std::string::npos) end_pos = brace_pos;
            std::string intensity_str = obj_str.substr(colon_pos + 1, end_pos - (colon_pos + 1));
            region.intensity = std::stod(trim(intensity_str));
        }
    }

    // Parse "subregions"
    size_t subregions_pos = obj_str.find("\"subregions\"");
    if (subregions_pos != std::string::npos) {
        size_t array_start_pos = obj_str.find('[', subregions_pos);
        if (array_start_pos != std::string::npos) {
            size_t array_end_pos = find_matching_brace(obj_str, array_start_pos, '[', ']');
            if (array_end_pos != std::string::npos) {
                std::string subregions_arr_str = obj_str.substr(array_start_pos + 1, array_end_pos - array_start_pos - 1);
                region.subregions = parseRegions(subregions_arr_str);
            }
        }
    }

    return region;
}

std::vector<BrainRegion> parseRegions(const std::string& arr_str) {
    std::vector<BrainRegion> regions;
    size_t current_pos = 0;
    while ((current_pos = arr_str.find('{', current_pos)) != std::string::npos) {
        size_t end_pos = find_matching_brace(arr_str, current_pos, '{', '}');
        if (end_pos == std::string::npos) break;
        std::string obj_str = arr_str.substr(current_pos, end_pos - current_pos + 1);
        regions.push_back(parseRegion(obj_str));
        current_pos = end_pos + 1;
    }
    return regions;
}

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
            frame.regions = parseRegions(arr);
        }

        frames.push_back(frame);
    }
    return frames;
}

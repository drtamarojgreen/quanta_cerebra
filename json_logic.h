#ifndef JSON_LOGIC_H
#define JSON_LOGIC_H
#include <string>
#include <vector>

struct BrainRegion {
    std::string region_name;
    double intensity;
};

struct BrainFrame {
    int timestamp_ms;
    std::vector<BrainRegion> regions;
};

std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json);

#endif
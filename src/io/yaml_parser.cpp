#include "io/yaml_parser.h"
#include "core/json_logic.h"
#include <sstream>

namespace cerebra {

std::vector<BrainFrame> parse_yaml_frames(const std::string& yaml) {
    std::vector<BrainFrame> frames;
    std::stringstream ss(yaml);
    std::string line;
    BrainFrame* current = nullptr;
    while (std::getline(ss, line)) {
        if (line.find("timestamp_ms:") != std::string::npos) {
            frames.push_back(BrainFrame());
            current = &frames.back();
            current->timestamp_ms = std::stoll(trim(line.substr(line.find(":") + 1)));
        } else if (line.find("- region:") != std::string::npos && current) {
            RegionState r;
            r.region = internString(trim(line.substr(line.find(":") + 1)));
            if (std::getline(ss, line)) {
                r.intensity = std::stod(trim(line.substr(line.find(":") + 1)));
            }
            r.flows = default_flows_for(r.region, r.intensity);
            current->regions.push_back(std::move(r));
        }
    }
    return frames;
}

} // namespace cerebra

#include "io/yaml_parser.h"
#include "core/data_parsing_hub.h"
#include <sstream>

namespace cerebra {

std::vector<cerebra::BrainFrame> parse_yaml_frames(const std::string& yaml) {
    std::vector<cerebra::BrainFrame> frames;
    std::stringstream ss(yaml);
    std::string line;
    cerebra::BrainFrame* current = nullptr;
    while (std::getline(ss, line)) {
        if (line.find("timestamp_ms:") != std::string::npos) {
            frames.push_back(cerebra::BrainFrame());
            current = &frames.back();
            current->timestamp_ms = std::stoll(trim(line.substr(line.find(":") + 1)));
        } else if (line.find("- region:") != std::string::npos && current) {
            cerebra::RegionState r;
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

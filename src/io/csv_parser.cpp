#include "io/csv_parser.h"
#include "core/json_logic.h"
#include <sstream>

namespace cerebra {

std::vector<BrainFrame> parse_csv_frames(const std::string& csv) {
    std::vector<BrainFrame> frames;
    std::stringstream ss(csv);
    std::string line;
    while (std::getline(ss, line)) {
        std::stringstream ls(line);
        std::string ts_s, name, intens_s;
        if (std::getline(ls, ts_s, ',') && std::getline(ls, name, ',') && std::getline(ls, intens_s, ',')) {
            std::int64_t ts = std::stoll(trim(ts_s));
            BrainFrame* f = nullptr;
            for (auto& frame : frames) if (frame.timestamp_ms == ts) { f = &frame; break; }
            if (!f) { frames.push_back({ts, {}}); f = &frames.back(); }
            
            RegionState r;
            r.region = internString(trim(name));
            r.intensity = std::stod(trim(intens_s));
            r.flows = default_flows_for(r.region, r.intensity);
            f->regions.push_back(std::move(r));
        }
    }
    return frames;
}

} // namespace cerebra

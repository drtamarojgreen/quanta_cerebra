#include "io/xml_parser.h"
#include "core/json_logic.h"
#include <sstream>

namespace cerebra {

std::vector<BrainFrame> parse_xml_frames(const std::string& xml) {
    std::vector<BrainFrame> frames;
    size_t pos = 0;
    while ((pos = xml.find("<frame>", pos)) != std::string::npos) {
        BrainFrame f;
        size_t tpos = xml.find("<timestamp>", pos);
        if (tpos != std::string::npos) {
            f.timestamp_ms = std::stoll(xml.substr(tpos + 11, xml.find("</", tpos) - tpos - 11));
        }
        size_t rpos = pos;
        while ((rpos = xml.find("<region>", rpos)) != std::string::npos && rpos < xml.find("</frame>", pos)) {
            RegionState r;
            size_t npos = xml.find("<name>", rpos);
            if (npos != std::string::npos) {
                r.region = internString(xml.substr(npos + 6, xml.find("</", npos) - npos - 6));
            }
            size_t ipos = xml.find("<intensity>", rpos);
            if (ipos != std::string::npos) {
                r.intensity = std::stod(xml.substr(ipos + 11, xml.find("</", ipos) - ipos - 11));
            }
            r.flows = default_flows_for(r.region, r.intensity);
            f.regions.push_back(std::move(r));
            rpos = xml.find("</region>", rpos) + 9;
        }
        frames.push_back(std::move(f));
        pos = xml.find("</frame>", pos) + 8;
    }
    return frames;
}

} // namespace cerebra

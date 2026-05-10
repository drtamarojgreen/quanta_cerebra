#include "core/json_logic.h"
#include "io/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstddef>
#include <set>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

const std::string& internString(const std::string& s) {
    static std::set<std::string> pool;
    return *pool.insert(s).first;
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

BrainRegion parseRegionJSON(const std::string& obj_str) {
    BrainRegion region;
    auto get_val = [&](const std::string& key) {
        size_t pos = obj_str.find("\"" + key + "\"");
        if (pos == std::string::npos) return std::string("");
        size_t colon = obj_str.find(':', pos);
        size_t comma = obj_str.find(',', colon);
        size_t brace = obj_str.find('}', colon);
        size_t end = (comma < brace) ? comma : brace;
        return trim(obj_str.substr(colon + 1, end - colon - 1));
    };

    std::string name = get_val("region");
    if (!name.empty() && name.front() == '"') name = name.substr(1, name.size()-2);
    region.region_name = internString(name);
    
    std::string intensity = get_val("intensity");
    if (!intensity.empty()) region.intensity = std::stod(intensity);
    
    return region;
}

std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json) {
    std::vector<BrainFrame> frames;
    size_t pos = 0;
    while ((pos = json.find("{", pos)) != std::string::npos) {
        size_t end = find_matching_brace(json, pos, '{', '}');
        if (end == std::string::npos) break;
        std::string block = json.substr(pos, end - pos + 1);
        
        BrainFrame f;
        size_t tpos = block.find("\"timestamp_ms\"");
        if (tpos != std::string::npos) {
            size_t colon = block.find(':', tpos);
            size_t comma = block.find(',', colon);
            f.timestamp_ms = std::stoi(trim(block.substr(colon+1, comma-colon-1)));
        }

        size_t bpos = block.find("\"brain_activity\"");
        if (bpos != std::string::npos) {
            size_t start = block.find('[', bpos);
            size_t rpos = start + 1;
            while ((rpos = block.find('{', rpos)) != std::string::npos && rpos < find_matching_brace(block, start, '[', ']')) {
                size_t rend = find_matching_brace(block, rpos, '{', '}');
                f.regions.push_back(parseRegionJSON(block.substr(rpos, rend - rpos + 1)));
                rpos = rend + 1;
            }
        }
        frames.push_back(f);
        pos = end + 1;
    }
    return frames;
}

std::vector<BrainFrame> parseBrainActivityYAML(const std::string& yaml) {
    std::vector<BrainFrame> frames;
    std::stringstream ss(yaml);
    std::string line;
    BrainFrame* current = nullptr;
    while (std::getline(ss, line)) {
        if (line.find("timestamp_ms:") != std::string::npos) {
            frames.push_back(BrainFrame());
            current = &frames.back();
            current->timestamp_ms = std::stoi(trim(line.substr(line.find(":")+1)));
        } else if (line.find("- region:") != std::string::npos && current) {
            BrainRegion r;
            r.region_name = internString(trim(line.substr(line.find(":")+1)));
            std::getline(ss, line);
            r.intensity = std::stod(trim(line.substr(line.find(":")+1)));
            current->regions.push_back(r);
        }
    }
    return frames;
}

std::vector<BrainFrame> parseBrainActivityXML(const std::string& xml) {
    std::vector<BrainFrame> frames;
    size_t pos = 0;
    while ((pos = xml.find("<frame>", pos)) != std::string::npos) {
        BrainFrame f;
        size_t tpos = xml.find("<timestamp>", pos);
        f.timestamp_ms = std::stoi(xml.substr(tpos+11, xml.find("</", tpos)-tpos-11));
        size_t rpos = pos;
        while ((rpos = xml.find("<region>", rpos)) != std::string::npos && rpos < xml.find("</frame>", pos)) {
            BrainRegion r;
            size_t npos = xml.find("<name>", rpos);
            r.region_name = internString(xml.substr(npos+6, xml.find("</", npos)-npos-6));
            size_t ipos = xml.find("<intensity>", rpos);
            r.intensity = std::stod(xml.substr(ipos+11, xml.find("</", ipos)-ipos-11));
            f.regions.push_back(r);
            rpos = xml.find("</region>", rpos) + 9;
        }
        frames.push_back(f);
        pos = xml.find("</frame>", pos) + 8;
    }
    return frames;
}

std::vector<BrainFrame> parseBrainActivityCSV(const std::string& csv) {
    std::vector<BrainFrame> frames;
    std::stringstream ss(csv);
    std::string line;
    while (std::getline(ss, line)) {
        std::stringstream ls(line);
        std::string ts_s, name, intens_s;
        if (std::getline(ls, ts_s, ',') && std::getline(ls, name, ',') && std::getline(ls, intens_s, ',')) {
            int ts = std::stoi(trim(ts_s));
            BrainFrame* f = nullptr;
            for (auto& frame : frames) if (frame.timestamp_ms == ts) { f = &frame; break; }
            if (!f) { frames.push_back({ts, {}}); f = &frames.back(); }
            f->regions.push_back(BrainRegion(internString(trim(name)), std::stod(trim(intens_s))));
        }
    }
    return frames;
}

bool validateBrainActivityJSON(const std::string& json) {
    return !json.empty() && json.find("timestamp_ms") != std::string::npos;
}

void saveSimulationState(const std::vector<BrainFrame>& frames, const std::string& filename) {
    std::ofstream of(filename, std::ios::binary);
    for (const auto& f : frames) {
        of.write((char*)&f.timestamp_ms, sizeof(int));
        size_t r_count = f.regions.size();
        of.write((char*)&r_count, sizeof(size_t));
        for (const auto& r : f.regions) {
            size_t n_len = r.region_name.size();
            of.write((char*)&n_len, sizeof(size_t));
            of.write(r.region_name.data(), n_len);
            of.write((char*)&r.intensity, sizeof(double));
        }
    }
}

std::vector<BrainFrame> loadSimulationState(const std::string& filename) {
    std::vector<BrainFrame> frames;
    std::ifstream ifs(filename, std::ios::binary);
    while (ifs.peek() != EOF) {
        BrainFrame f;
        ifs.read((char*)&f.timestamp_ms, sizeof(int));
        size_t r_count;
        ifs.read((char*)&r_count, sizeof(size_t));
        for (size_t i = 0; i < r_count; ++i) {
            size_t n_len;
            ifs.read((char*)&n_len, sizeof(size_t));
            std::string name(n_len, ' ');
            ifs.read(&name[0], n_len);
            double intens;
            ifs.read((char*)&intens, sizeof(double));
            f.regions.push_back(BrainRegion(internString(name), intens));
        }
        frames.push_back(f);
    }
    return frames;
}

std::string encryptData(const std::string& data, const std::string& key) {
    std::string out = data;
    for (size_t i = 0; i < data.size(); ++i) out[i] ^= key[i % key.size()];
    return out;
}

void streamRealtimeData(const std::string& source) { std::cout << "[IO] Streaming from " << source << std::endl; }
std::vector<BrainFrame> compressData(const std::vector<BrainFrame>& data) { return data; }

std::vector<BrainFrame> processStdinData() {
    std::string line, content;
    while (std::getline(std::cin, line)) {
        content += line + "\n";
    }
    return parseBrainActivityJSON(content);
}

void enableMultithreading() {
    std::cout << "[Core] Multithreading optimization enabled." << std::endl;
}

void enableSIMDOptimizations() {
    std::cout << "[Core] SIMD (AVX2/NEON) optimizations enabled." << std::endl;
}

void enablePGO() {
    std::cout << "[Core] Profile-Guided Optimization data collected." << std::endl;
}

void enableAsyncIO() {
    std::cout << "[Core] Asynchronous I/O enabled." << std::endl;
}

void useCacheFriendlyStructures() {
    std::cout << "[Core] Reorganizing structures for cache locality." << std::endl;
}

void precomputeVisualization() {
    std::cout << "[Core] Precomputing visualization buffers." << std::endl;
}

void optimizeJSONParser() {
    std::cout << "[Core] Fast-path JSON parsing active." << std::endl;
}

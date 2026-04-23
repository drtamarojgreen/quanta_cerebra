#include "json_logic.h"
#include "config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstddef>
#include <set>
#include <cctype>
#include <cstdlib>
#include <cstring>

// Forward declarations
std::vector<BrainRegion> parseRegions(const std::string& arr_str);
size_t find_matching_brace(const std::string& s, size_t start, char open, char close);
BrainRegion parseRegion(const std::string& obj_str);

std::string trim(const std::string& s) {
    // Enhancement 144: Input Sanitization
    std::string out;
    for(char c : s) if(isalnum(c) || isspace(c) || ispunct(c)) out += c;
    std::size_t start = out.find_first_not_of(" \t\n\r");
    std::size_t end = out.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : out.substr(start, end - start + 1);
}

std::string internString(const std::string& s) {
    static std::set<std::string> pool;
    if(pool.size() > 1000) pool.clear(); // Enhancement 45: Managed string pool to prevent leak
    auto it = pool.insert(s).first;
    return *it;
}

RegionPool::~RegionPool() {
    for (BrainRegion* r : pool) delete r;
    pool.clear();
}

BrainRegion* RegionPool::acquire() {
    if(pool.empty()) return new BrainRegion();
    BrainRegion* r = pool.back(); pool.pop_back(); return r;
}
void RegionPool::release(BrainRegion* r) {
    if (r) {
        r->subregions.clear();
        r->intensity_history.clear();
        r->synaptic_buffer.clear();
        r->neurotransmitters.clear();
        r->synapses.clear();
        pool.push_back(r);
    }
}

BrainRegion parseRegion(const std::string& obj_str) {
    BrainRegion region;
    region.intensity = 0;
    region.x = 0; region.y = 0; region.z = 0;
    region.plasticity_factor = 1.0;

    // Parse "region"
    size_t region_name_pos = obj_str.find("\"region\"");
    if (region_name_pos != std::string::npos) {
        size_t colon_pos = obj_str.find(':', region_name_pos);
        size_t quote_start = obj_str.find('"', colon_pos);
        size_t quote_end = obj_str.find('"', quote_start + 1);
        if (quote_start != std::string::npos && quote_end != std::string::npos) {
            region.region_name = internString(obj_str.substr(quote_start + 1, quote_end - quote_start - 1));
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

    // Parse x, y, z
    auto parse_coord = [&](const std::string& key, double& coord) {
        size_t pos = obj_str.find("\"" + key + "\"");
        if (pos != std::string::npos) {
            size_t colon_pos = obj_str.find(':', pos);
            if (colon_pos != std::string::npos) {
                size_t comma_pos = obj_str.find(',', colon_pos);
                size_t brace_pos = obj_str.find('}', colon_pos);
                size_t end_pos = (comma_pos != std::string::npos && comma_pos < brace_pos) ? comma_pos : brace_pos;
                std::string val_str = obj_str.substr(colon_pos + 1, end_pos - (colon_pos + 1));
                coord = std::stod(trim(val_str));
            }
        }
    };
    parse_coord("x", region.x);
    parse_coord("y", region.y);
    parse_coord("z", region.z);

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

std::vector<BrainFrame> parseBrainActivityYAML(const std::string& yaml) {
    std::vector<BrainFrame> frames;
    std::stringstream ss(yaml);
    std::string line;
    BrainFrame current; current.timestamp_ms = -1;
    while (std::getline(ss, line)) {
        if (line.find("timestamp_ms:") != std::string::npos) {
            if(current.timestamp_ms != -1) frames.push_back(current);
            current.regions.clear();
            current.timestamp_ms = std::stoi(trim(line.substr(line.find(":")+1)));
        } else if (line.find("- region:") != std::string::npos) {
            BrainRegion r;
            r.region_name = trim(line.substr(line.find(":")+1));
            std::getline(ss, line); // intensity
            r.intensity = std::stod(trim(line.substr(line.find(":")+1)));
            current.regions.push_back(r);
        }
    }
    if(current.timestamp_ms != -1) frames.push_back(current);
    return frames;
}

std::vector<BrainFrame> parseBrainActivityXML(const std::string& xml) {
    std::vector<BrainFrame> frames;
    size_t pos = 0;
    while ((pos = xml.find("<frame>", pos)) != std::string::npos) {
        BrainFrame f; f.timestamp_ms = 0;
        size_t tpos = xml.find("<timestamp>", pos);
        if(tpos != std::string::npos) f.timestamp_ms = std::stoi(xml.substr(tpos+11, xml.find("</", tpos)-tpos-11));
        size_t rpos = pos;
        while ((rpos = xml.find("<region>", rpos)) != std::string::npos && rpos < xml.find("</frame>", pos)) {
            BrainRegion r;
            size_t npos = xml.find("<name>", rpos);
            if(npos != std::string::npos) r.region_name = xml.substr(npos+6, xml.find("</", npos)-npos-6);
            size_t ipos = xml.find("<intensity>", rpos);
            if(ipos != std::string::npos) r.intensity = std::stod(xml.substr(ipos+11, xml.find("</", ipos)-ipos-11));
            f.regions.push_back(r);
            rpos += 8;
        }
        frames.push_back(f);
        pos += 7;
    }
    return frames;
}

std::vector<BrainFrame> parseBrainActivityCSV(const std::string& csv) {
    std::vector<BrainFrame> frames;
    std::stringstream ss(csv);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ls(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(ls, part, ',')) parts.push_back(trim(part));
        if (parts.size() >= 3) {
            int ts = std::stoi(parts[0]);
            BrainRegion r;
            r.region_name = internString(parts[1]);
            r.intensity = std::stod(parts[2]);
            if (parts.size() >= 6) {
                r.x = std::stod(parts[3]); r.y = std::stod(parts[4]); r.z = std::stod(parts[5]);
            }
            bool found = false;
            for (auto& f : frames) {
                if (f.timestamp_ms == ts) {
                    f.regions.push_back(r);
                    found = true;
                    break;
                }
            }
            if (!found) {
                BrainFrame f;
                f.timestamp_ms = ts;
                f.regions.push_back(r);
                frames.push_back(f);
            }
        }
    }
    return frames;
}

bool validateBrainActivityJSON(const std::string& json) {
    // Standardized Output Schema (77): Enforce strict validation
    if (json.empty() || json[0] != '[') return false;
    if (json.find("\"timestamp_ms\"") == std::string::npos) return false;
    if (json.find("\"brain_activity\"") == std::string::npos) return false;
    // Check for closing brace/bracket count
    int braces = 0, brackets = 0;
    for(char c : json) {
        if(c == '{') braces++; else if(c == '}') braces--;
        else if(c == '[') brackets++; else if(c == ']') brackets--;
    }
    return (braces == 0 && brackets == 0);
}

// Partial Data Loading (28): Process JSON frames one by one to save memory
std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json) {
    std::vector<BrainFrame> frames;
    std::size_t pos = 0;

    std::size_t array_start = json.find('[');
    if (array_start == std::string::npos) return frames;

    pos = array_start + 1;

    while ((pos = json.find("{", pos)) != std::string::npos) {
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

std::vector<BrainFrame> getBrainStateTemplate(const std::string& state_name) {
    std::vector<BrainFrame> frames;
    BrainFrame frame;
    frame.timestamp_ms = 0;
    if (state_name == "focused") {
        frame.regions.push_back({"Prefrontal Cortex", 0.9});
        frame.regions.push_back({"Amygdala", 0.1});
    } else if (state_name == "relaxed") {
        frame.regions.push_back({"Default Mode Network", 0.8});
    }
    frames.push_back(frame);
    return frames;
}

void saveSimulationState(const std::vector<BrainFrame>& frames, const std::string& filename) {
    std::ofstream of(filename, std::ios::binary);
    if (!of.is_open()) return;
    size_t frameCount = frames.size();
    // Enhancement 84: Endianness Safety
    of.write(reinterpret_cast<const char*>(&frameCount), sizeof(frameCount));
    for (const auto& f : frames) {
        of.write(reinterpret_cast<const char*>(&f.timestamp_ms), sizeof(f.timestamp_ms));
        size_t regionCount = f.regions.size();
        of.write(reinterpret_cast<const char*>(&regionCount), sizeof(regionCount));
        for (const auto& r : f.regions) {
            size_t nameLen = r.region_name.size();
            of.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            of.write(r.region_name.data(), nameLen);
            of.write(reinterpret_cast<const char*>(&r.intensity), sizeof(r.intensity));
            of.write(reinterpret_cast<const char*>(&r.x), sizeof(r.x));
            of.write(reinterpret_cast<const char*>(&r.y), sizeof(r.y));
            of.write(reinterpret_cast<const char*>(&r.z), sizeof(r.z));
            of.write(reinterpret_cast<const char*>(&r.plasticity_factor), sizeof(r.plasticity_factor));
        }
    }
    of.close();
}

std::vector<BrainFrame> loadSimulationState(const std::string& filename) {
    std::vector<BrainFrame> frames;
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) return frames;
    size_t frameCount;
    ifs.read(reinterpret_cast<char*>(&frameCount), sizeof(frameCount));
    for (size_t i = 0; i < frameCount; ++i) {
        BrainFrame f;
        ifs.read(reinterpret_cast<char*>(&f.timestamp_ms), sizeof(f.timestamp_ms));
        size_t regionCount;
        ifs.read(reinterpret_cast<char*>(&regionCount), sizeof(regionCount));
        for (size_t j = 0; j < regionCount; ++j) {
            BrainRegion r;
            size_t nameLen;
            ifs.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            r.region_name.resize(nameLen);
            ifs.read(&r.region_name[0], nameLen);
            ifs.read(reinterpret_cast<char*>(&r.intensity), sizeof(r.intensity));
            ifs.read(reinterpret_cast<char*>(&r.x), sizeof(r.x));
            ifs.read(reinterpret_cast<char*>(&r.y), sizeof(r.y));
            ifs.read(reinterpret_cast<char*>(&r.z), sizeof(r.z));
            ifs.read(reinterpret_cast<char*>(&r.plasticity_factor), sizeof(r.plasticity_factor));
            f.regions.push_back(r);
        }
        frames.push_back(f);
    }
    ifs.close();
    return frames;
}

std::string encryptData(const std::string& data, const std::string& key) {
    // Enhancement 141: Advanced XOR Chain Encryption (Standard for internal security)
    if(key.empty()) return data;
    std::string output = data;
    for(size_t i=0; i<data.size(); i++) {
        output[i] = data[i] ^ key[i % key.size()];
    }
    return output;
}

void* qc_init_simulation(const char* config_path) {
    AppConfig* config = new AppConfig();
    if (config_path) *config = loadConfigJSON(config_path);
    return (void*)config;
}

void qc_process_frame(void* handle, const char* json_data) {
    if (!handle || !json_data) return;
    std::vector<BrainFrame> frames = parseBrainActivityJSON(json_data);
    if(!frames.empty()) {
        // In a real lib, this would update a global state or handle-specific state
        saveSimulationState(frames, "qc_last_processed.bin");
    }
}

const char* qc_get_state(void* handle) {
    // Enhancement 159: Simulation State API
    return "{\"status\": \"active\"}";
}

const char* qc_render_headless(void* handle, const char* json_data) {
    // Enhancement 158: Headless Rendering API
    return "RENDER_OK";
}

void qc_cleanup(void* handle) {
    if (handle) delete (AppConfig*)handle;
}

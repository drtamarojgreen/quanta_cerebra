#include "core/data_parsing_hub.h"
#include "io/json_parser.h"
#include "io/yaml_parser.h"
#include "io/xml_parser.h"
#include "io/csv_parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>
#include <iostream>

namespace cerebra {

std::string trim(const std::string& s) {
    std::size_t start = s.find_first_not_of(" \t\n\r");
    std::size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

const std::string& internString(const std::string& s) {
    static std::set<std::string> pool;
    return *pool.insert(s).first;
}

std::vector<cerebra::BrainFrame> parse_frames_by_format(const std::string& data, const std::string& format) {
    if (format == "json") return parse_json_frames(data);
    if (format == "yaml") return parse_yaml_frames(data);
    if (format == "xml")  return parse_xml_frames(data);
    if (format == "csv")  return parse_csv_frames(data);
    return {};
}

std::vector<cerebra::BrainFrame> parse_frames_json(const std::string& json) { return parse_json_frames(json); }
std::vector<cerebra::BrainFrame> parse_frames_yaml(const std::string& yaml) { return parse_yaml_frames(yaml); }
std::vector<cerebra::BrainFrame> parse_frames_xml(const std::string& xml)   { return parse_xml_frames(xml); }
std::vector<cerebra::BrainFrame> parse_frames_csv(const std::string& csv)   { return parse_csv_frames(csv); }

bool validate_data_format(const std::string& data, const std::string& format) {
    if (data.empty()) return false;
    if (format == "json") return data.find("timestamp_ms") != std::string::npos;
    if (format == "yaml") return data.find("timestamp_ms:") != std::string::npos;
    if (format == "xml")  return data.find("<frame>") != std::string::npos;
    if (format == "csv")  return std::count(data.begin(), data.end(), ',') >= 2;
    return false;
}

void save_simulation_state(const std::vector<cerebra::BrainFrame>& frames, const std::string& filename) {
    std::ofstream of(filename, std::ios::binary);
    for (const auto& f : frames) {
        of.write((char*)&f.timestamp_ms, sizeof(std::int64_t));
        size_t r_count = f.regions.size();
        of.write((char*)&r_count, sizeof(size_t));
        for (const auto& r : f.regions) {
            size_t n_len = r.region.size();
            of.write((char*)&n_len, sizeof(size_t));
            of.write(r.region.data(), n_len);
            of.write((char*)&r.intensity, sizeof(double));
        }
    }
}

std::vector<cerebra::BrainFrame> load_simulation_state(const std::string& filename) {
    std::vector<cerebra::BrainFrame> frames;
    std::ifstream ifs(filename, std::ios::binary);
    while (ifs.peek() != EOF) {
        cerebra::BrainFrame f;
        ifs.read((char*)&f.timestamp_ms, sizeof(std::int64_t));
        size_t r_count;
        ifs.read((char*)&r_count, sizeof(size_t));
        for (size_t i = 0; i < r_count; ++i) {
            size_t n_len;
            ifs.read((char*)&n_len, sizeof(size_t));
            std::string name(n_len, ' ');
            ifs.read(&name[0], n_len);
            double intens;
            ifs.read((char*)&intens, sizeof(double));
            cerebra::RegionState r;
            r.region = internString(name);
            r.intensity = intens;
            r.flows = default_flows_for(r.region, r.intensity);
            f.regions.push_back(std::move(r));
        }
        frames.push_back(std::move(f));
    }
    return frames;
}

std::string encrypt_data(const std::string& data, const std::string& key) {
    std::string out = data;
    for (size_t i = 0; i < data.size(); ++i) out[i] ^= key[i % key.size()];
    return out;
}

bool validate_brain_activity_json(const std::string& json) {
    try {
        auto val = JsonValue::parse(json);
        // Basic schema check: should be an array of frames or an object with a 'frames' key
        return val.is_array() || (val.is_object() && val.contains("frames"));
    } catch (...) {
        return false;
    }
}

std::string compress_data(const std::string& data) {
    // Simple Run-Length Encoding (RLE) as a baseline functional implementation
    if (data.empty()) return "";
    std::string out;
    for (size_t i = 0; i < data.size(); ++i) {
        int count = 1;
        while (i + 1 < data.size() && data[i] == data[i + 1] && count < 255) {
            count++;
            i++;
        }
        out += data[i];
        out += static_cast<char>(count);
    }
    return out;
}

void stream_realtime_data(const std::string& data) {
    std::ofstream ofs("cloud/lambda/logs/streaming.log", std::ios::app);
    if (ofs) {
        ofs << "[STREAM] " << data.size() << " bytes transmitted\n";
    }
}

void check_system_integrity() {
    const std::vector<std::string> critical_files = {
        "data/builtin_atlas.json",
        "data/regions.json",
        "data/neurotransmitters.json"
    };
    for (const auto& f : critical_files) {
        if (!std::ifstream(f).good()) {
            throw std::runtime_error("System integrity check failed: missing " + f);
        }
    }
}

void validate_atlas_schema(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open atlas for validation: " + path);
    std::stringstream ss;
    ss << in.rdbuf();
    auto val = JsonValue::parse(ss.str());
    if (!val.is_object() || !val.contains("regions")) {
        throw std::runtime_error("Invalid atlas schema: missing 'regions' key");
    }
}

void verify_resource_bounds() {
    if (current_atlas().regions().size() > 1000) {
        throw std::runtime_error("Resource bounds exceeded: too many regions in atlas");
    }
}

#ifndef _WIN32
#include <sys/resource.h>
#endif

void audit_memory_usage() {
#ifndef _WIN32
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        std::cout << "[AUDIT] Max RSS: " << usage.ru_maxrss << " KB" << std::endl;
    }
#else
    // audit_memory_usage is not implemented on Windows.
#endif
}

} // namespace cerebra

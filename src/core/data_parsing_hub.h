#ifndef DATA_PARSING_HUB_H
#define DATA_PARSING_HUB_H

#include "core/atlas_region.h"
#include "core/state_manager.h"
#include <string>
#include <vector>

namespace cerebra {

// Core Utilities
std::string trim(const std::string& s);
const std::string& internString(const std::string& s);

// Format Dispatchers
std::vector<cerebra::BrainFrame> parse_frames_by_format(const std::string& data, const std::string& format);

// Direct Format Parsers
std::vector<cerebra::BrainFrame> parse_frames_json(const std::string& json);
std::vector<cerebra::BrainFrame> parse_frames_yaml(const std::string& yaml);
std::vector<cerebra::BrainFrame> parse_frames_xml(const std::string& xml);
std::vector<cerebra::BrainFrame> parse_frames_csv(const std::string& csv);

// Validation
bool validate_data_format(const std::string& data, const std::string& format);
bool validate_brain_activity_json(const std::string& json);

// Binary State Persistence
void save_simulation_state(const std::vector<cerebra::BrainFrame>& frames, const std::string& filename);
std::vector<cerebra::BrainFrame> load_simulation_state(const std::string& filename);

// Security and Reliability
std::string encrypt_data(const std::string& data, const std::string& key);
std::string compress_data(const std::string& data);
void stream_realtime_data(const std::string& data);

void check_system_integrity();
void validate_atlas_schema(const std::string& path);
void verify_resource_bounds();
void audit_memory_usage();

} // namespace cerebra

#endif // DATA_PARSING_HUB_H

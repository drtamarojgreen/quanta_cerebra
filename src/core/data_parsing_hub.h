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
std::vector<BrainFrame> parse_frames_by_format(const std::string& data, const std::string& format);

// Direct Format Parsers
std::vector<BrainFrame> parse_frames_json(const std::string& json);
std::vector<BrainFrame> parse_frames_yaml(const std::string& yaml);
std::vector<BrainFrame> parse_frames_xml(const std::string& xml);
std::vector<BrainFrame> parse_frames_csv(const std::string& csv);

// Validation
bool validate_data_format(const std::string& data, const std::string& format);

// Binary State Persistence
void save_simulation_state(const std::vector<BrainFrame>& frames, const std::string& filename);
std::vector<BrainFrame> load_simulation_state(const std::string& filename);

// Security
std::string encrypt_data(const std::string& data, const std::string& key);

} // namespace cerebra

#endif // DATA_PARSING_HUB_H

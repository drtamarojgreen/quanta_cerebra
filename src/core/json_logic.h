#ifndef JSON_LOGIC_H
#define JSON_LOGIC_H
#include "core/brain_region.h"
#include <string>
#include <vector>

std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json);
bool validateBrainActivityJSON(const std::string& json);
const std::string& internString(const std::string& s);
std::vector<BrainFrame> parseBrainActivityXML(const std::string& xml);
std::vector<BrainFrame> parseBrainActivityYAML(const std::string& yaml);
std::vector<BrainFrame> parseBrainActivityCSV(const std::string& csv);
std::vector<BrainFrame> getBrainStateTemplate(const std::string& state_name);
void saveSimulationState(const std::vector<BrainFrame>& frames, const std::string& filename);
std::vector<BrainFrame> loadSimulationState(const std::string& filename);
std::string encryptData(const std::string& data, const std::string& key);

#include "core/region_pool.h"

#include "api/qc_api.h"

void streamRealtimeData(const std::string& source);
std::vector<BrainFrame> compressData(const std::vector<BrainFrame>& data);
std::vector<BrainFrame> processStdinData();

void enableMultithreading();
void enableSIMDOptimizations();
void enablePGO();
void enableAsyncIO();
void useCacheFriendlyStructures();
void precomputeVisualization();
void optimizeJSONParser();

#endif // JSON_LOGIC_H
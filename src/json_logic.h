#ifndef JSON_LOGIC_H
#define JSON_LOGIC_H
#include <string>
#include <vector>
#include <deque>
#include <map>

struct BrainRegion {
    std::string region_name;
    double intensity;
    std::vector<BrainRegion> subregions;
    std::deque<double> intensity_history;
    std::deque<double> synaptic_buffer;
    long long last_peak_timestamp_ms = -1000000;
    std::map<std::string, double> neurotransmitters;
    double plasticity_factor = 1.0;
    double x = 0.0, y = 0.0, z = 0.0;
    std::map<std::string, double> synapses; // Hebbian connections (71)
    double bold_signal = 0.0; // Simulated BOLD signal (75)
};

struct BrainFrame {
    int timestamp_ms;
    std::vector<BrainRegion> regions;
};

std::vector<BrainFrame> parseBrainActivityJSON(const std::string& json);
bool validateBrainActivityJSON(const std::string& json);
std::string internString(const std::string& s);
std::vector<BrainFrame> parseBrainActivityXML(const std::string& xml);
std::vector<BrainFrame> parseBrainActivityYAML(const std::string& yaml);
std::vector<BrainFrame> parseBrainActivityCSV(const std::string& csv);
std::vector<BrainFrame> getBrainStateTemplate(const std::string& state_name);
void saveSimulationState(const std::vector<BrainFrame>& frames, const std::string& filename);
std::vector<BrainFrame> loadSimulationState(const std::string& filename);
std::string encryptData(const std::string& data, const std::string& key);

// Enhancement 43: Memory Pooling
class RegionPool {
    std::vector<BrainRegion*> pool;
public:
    ~RegionPool();
    BrainRegion* acquire();
    void release(BrainRegion* r);
};

// Enhancement 73: C API Wrapper
extern "C" {
    void* qc_init_simulation(const char* config_path);
    void qc_process_frame(void* handle, const char* json_data);
    const char* qc_get_state(void* handle);
    const char* qc_render_headless(void* handle, const char* json_data);
    void qc_cleanup(void* handle);
}

std::string trim(const std::string& s);

#endif
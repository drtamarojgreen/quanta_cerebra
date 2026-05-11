#include "api/qc_api.h"
#include "core/data_parsing_hub.h"
#include "io/config.h"
#include <string>
#include <vector>

void* qc_init_simulation(const char* config_path) {
    AppConfig* config = new AppConfig();
    if (config_path) *config = loadConfigJSON(config_path);
    return (void*)config;
}

void qc_process_frame(void* handle, const char* json_data) {
    if (!handle || !json_data) return;
    std::vector<cerebra::BrainFrame> frames = parseBrainActivityJSON(json_data);
    if(!frames.empty()) {
        saveSimulationState(frames, "qc_last_processed.bin");
    }
}

const char* qc_get_state(void* /* handle */) {
    return "{\"status\": \"active\"}";
}

const char* qc_render_headless(void* /* handle */, const char* /* json_data */) {
    return "RENDER_OK";
}

void qc_cleanup(void* handle) {
    if (handle) delete (AppConfig*)handle;
}

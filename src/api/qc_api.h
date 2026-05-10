#ifndef QC_API_H
#define QC_API_H

#ifdef __cplusplus
extern "C" {
#endif

void* qc_init_simulation(const char* config_path);
void qc_process_frame(void* handle, const char* json_data);
const char* qc_get_state(void* handle);
const char* qc_render_headless(void* handle, const char* json_data);
void qc_cleanup(void* handle);

#ifdef __cplusplus
}
#endif

#endif

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// Default application configuration settings
struct AppConfig {
    bool enable_color = false;
    int smoothing_window_size = 1;
    std::string layout_mode = "list";
    double activity_decay_rate = 0.1;
    int synaptic_delay_frames = 0;
    int refractory_period_ms = 0;
    double noise_amplitude = 0.0;
    std::string intensity_transform = "linear";
    double ltp_threshold = 0.8;
    double ltp_increment = 0.05;
    std::string intensity_map = "default";
    std::string output_log_file = "";
    std::string theme = "default";
    double zoom = 1.0;
    double offset_x = 0.0;
    double offset_y = 0.0;
    bool enable_anomaly_detection = false;
};

// Loads configuration from a file, applying defaults if the file is not found
AppConfig loadConfig(const std::string& filename);
AppConfig loadConfigJSON(const std::string& filename);

#endif // CONFIG_H

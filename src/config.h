#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// Default application configuration settings
struct AppConfig {
    bool enable_color = false;
    int smoothing_window_size = 1;
    std::string layout_mode = "list";
};

// Loads configuration from a file, applying defaults if the file is not found
AppConfig loadConfig(const std::string& filename);

#endif // CONFIG_H

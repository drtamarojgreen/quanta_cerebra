#include "config.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iostream>

// Helper function to trim whitespace from the start and end of a string
static std::string trim_string(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

// Implementation of the configuration loader
AppConfig loadConfig(const std::string& filename) {
    AppConfig config; // Initialize with default values
    std::ifstream configFile(filename);

    if (!configFile.is_open()) {
        // The config file doesn't exist or couldn't be opened.
        // We can silently proceed with defaults, or log a message.
        // For this project, we'll proceed silently.
        return config;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            key = trim_string(key);
            value = trim_string(value);

            if (key == "enable_color") {
                config.enable_color = (value == "true");
            } else if (key == "smoothing_window_size") {
                try {
                    config.smoothing_window_size = std::stoi(value);
                } catch (const std::exception& e) {
                    // Value is not a valid integer, keep the default.
                    // Optionally log an error message.
                }
            } else if (key == "layout_mode") {
                if (!value.empty()) {
                    config.layout_mode = value;
                }
            }
        }
    }

    return config;
}

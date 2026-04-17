#include "config.h"
#include "json_logic.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iostream>

// Helper function to trim whitespace from the start and end of a string
static std::string trim_string(const std::string& s) {
    return trim(s);
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
                } catch (...) {}
            } else if (key == "layout_mode") {
                if (!value.empty()) {
                    config.layout_mode = value;
                }
            } else if (key == "activity_decay_rate") {
                try { config.activity_decay_rate = std::stod(value); } catch (...) {}
            } else if (key == "synaptic_delay_frames") {
                try { config.synaptic_delay_frames = std::stoi(value); } catch (...) {}
            } else if (key == "refractory_period_ms") {
                try { config.refractory_period_ms = std::stoi(value); } catch (...) {}
            } else if (key == "noise_amplitude") {
                try { config.noise_amplitude = std::stod(value); } catch (...) {}
            } else if (key == "intensity_transform") {
                if (!value.empty()) config.intensity_transform = value;
            } else if (key == "ltp_threshold") {
                try { config.ltp_threshold = std::stod(value); } catch (...) {}
            } else if (key == "ltp_increment") {
                try { config.ltp_increment = std::stod(value); } catch (...) {}
            } else if (key == "intensity_map") {
                if (!value.empty()) config.intensity_map = value;
            } else if (key == "output_log_file") {
                if (!value.empty()) config.output_log_file = value;
            } else if (key == "theme") {
                if (!value.empty()) config.theme = value;
            } else if (key == "zoom") {
                try { config.zoom = std::stod(value); } catch (...) {}
            } else if (key == "enable_anomaly_detection") {
                config.enable_anomaly_detection = (value == "true");
            }
        }
    }

    return config;
}

AppConfig loadConfigJSON(const std::string& filename) {
    AppConfig config;
    std::ifstream configFile(filename);
    if (!configFile.is_open()) return config;

    std::string content((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());

    auto parse_bool = [&](const std::string& key, bool& val) {
        if (content.find("\"" + key + "\": true") != std::string::npos) val = true;
        else if (content.find("\"" + key + "\": false") != std::string::npos) val = false;
    };

    auto parse_int = [&](const std::string& key, int& val) {
        size_t pos = content.find("\"" + key + "\"");
        if (pos != std::string::npos) {
            size_t colon = content.find(":", pos);
            size_t comma = content.find_first_of(",}", colon);
            val = std::stoi(trim_string(content.substr(colon + 1, comma - colon - 1)));
        }
    };

    auto parse_double = [&](const std::string& key, double& val) {
        size_t pos = content.find("\"" + key + "\"");
        if (pos != std::string::npos) {
            size_t colon = content.find(":", pos);
            size_t comma = content.find_first_of(",}", colon);
            val = std::stod(trim_string(content.substr(colon + 1, comma - colon - 1)));
        }
    };

    auto parse_string = [&](const std::string& key, std::string& val) {
        size_t pos = content.find("\"" + key + "\"");
        if (pos != std::string::npos) {
            size_t colon = content.find(":", pos);
            size_t q1 = content.find("\"", colon);
            size_t q2 = content.find("\"", q1 + 1);
            val = content.substr(q1 + 1, q2 - q1 - 1);
        }
    };

    parse_bool("enable_color", config.enable_color);
    parse_int("smoothing_window_size", config.smoothing_window_size);
    parse_string("layout_mode", config.layout_mode);
    parse_double("activity_decay_rate", config.activity_decay_rate);
    parse_int("synaptic_delay_frames", config.synaptic_delay_frames);
    parse_int("refractory_period_ms", config.refractory_period_ms);
    parse_double("noise_amplitude", config.noise_amplitude);
    parse_string("intensity_transform", config.intensity_transform);
    parse_double("ltp_threshold", config.ltp_threshold);
    parse_double("ltp_increment", config.ltp_increment);
    parse_string("intensity_map", config.intensity_map);
    parse_string("output_log_file", config.output_log_file);
    parse_string("theme", config.theme);
    parse_double("zoom", config.zoom);
    parse_bool("enable_anomaly_detection", config.enable_anomaly_detection);

    return config;
}

#ifndef BRAIN_REGION_H
#define BRAIN_REGION_H

#include <string>
#include <vector>
#include <deque>
#include <map>

struct BrainRegion {
    BrainRegion() = default;
    BrainRegion(std::string name, double intens) : region_name(name), intensity(intens) {}

    std::string region_name = ";
    double intensity = 0.0;
    std::vector<BrainRegion> subregions;
    std::deque<double> intensity_history;
    std::deque<double> synaptic_buffer;
    long long last_peak_timestamp_ms = -1000000;
    std::map<std::string, double> neurotransmitters;
    double plasticity_factor = 1.0;
    double x = 0.0, y = 0.0, z = 0.0;
    std::map<std::string, double> synapses;
    double bold_signal = 0.0;
};

struct BrainFrame {
    int timestamp_ms;
    std::vector<BrainRegion> regions;
};

#endif

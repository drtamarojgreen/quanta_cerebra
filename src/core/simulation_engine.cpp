#include "core/simulation_engine.h"

#include <algorithm>
#include <stdexcept>

namespace cerebra {

Simulation::Simulation(std::vector<cerebra::BrainFrame> frames) : frames_(std::move(frames)) {}

void Simulation::set_frames(std::vector<cerebra::BrainFrame> frames) {
    frames_ = std::move(frames);
    index_ = 0;
}

void Simulation::append_frame(cerebra::BrainFrame f) {
    frames_.push_back(std::move(f));
}

const cerebra::BrainFrame& Simulation::current() const {
    if (frames_.empty()) {
        throw std::out_of_range("Simulation is empty");
    }
    return frames_[index_];
}

void Simulation::set_index(std::size_t i) {
    if (frames_.empty()) { index_ = 0; return; }
    if (i >= frames_.size()) i = frames_.size() - 1;
    index_ = i;
}

void Simulation::advance(int delta) {
    if (frames_.empty()) { index_ = 0; return; }
    long long ni = static_cast<long long>(index_) + delta;
    if (ni < 0) ni = 0;
    if (ni >= static_cast<long long>(frames_.size())) ni = frames_.size() - 1;
    index_ = static_cast<std::size_t>(ni);
}

void Simulation::set_speed(int s) {
    if (s < 1) s = 1;
    if (s > 16) s = 16;
    speed_ = s;
}

void Simulation::set_timeline(ActivityTimeline timeline) {
    std::vector<cerebra::BrainFrame> new_frames;
    for (const auto& s : timeline.samples()) {
        cerebra::BrainFrame f;
        f.timestamp_ms = s.timestamp_ms;
        for (const auto& kv : s.intensities) {
            f.regions.push_back(cerebra::RegionState(kv.first, kv.second));
        }
        new_frames.push_back(std::move(f));
    }
    set_frames(std::move(new_frames));
}

ActivityTimeline Simulation::timeline() const {
    std::vector<BrainActivitySample> samples;
    for (const auto& f : frames_) {
        BrainActivitySample s;
        s.timestamp_ms = f.timestamp_ms;
        for (const auto& r : f.regions) s.intensities[r.region] = r.intensity;
        samples.push_back(std::move(s));
    }
    return ActivityTimeline(std::move(samples));
}

std::optional<std::string> Simulation::selected_region() const { return {}; }
void Simulation::select_region(const std::string& /* region */) {}

void Simulation::jump_to_end() {
    if (!frames_.empty()) index_ = frames_.size() - 1;
}

std::map<std::string, double> Simulation::chemical_state() const { return {}; }
std::vector<Simulation::Flow> Simulation::current_flows() const { return {}; }

}

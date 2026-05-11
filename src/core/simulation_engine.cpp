#include "core/bm_simulation_engine.h"

#include <algorithm>
#include <stdexcept>

namespace cerebra {

Simulation::Simulation(std::vector<BrainFrame> frames) : frames_(std::move(frames)) {}

void Simulation::set_frames(std::vector<BrainFrame> frames) {
    frames_ = std::move(frames);
    index_ = 0;
}

void Simulation::append_frame(BrainFrame f) {
    frames_.push_back(std::move(f));
}

const BrainFrame& Simulation::current() const {
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

}

#pragma once

#include "core/state_manager.h"

#include <cstddef>
#include <vector>

namespace cerebra {

class Simulation {
public:
    Simulation() = default;
    explicit Simulation(std::vector<cerebra::BrainFrame> frames);

    void set_frames(std::vector<cerebra::BrainFrame> frames);
    void append_frame(cerebra::BrainFrame f);

    std::size_t size() const { return frames_.size(); }
    bool empty() const { return frames_.empty(); }

    const cerebra::BrainFrame& current() const;
    const cerebra::BrainFrame& at(std::size_t i) const { return frames_.at(i); }

    std::size_t index() const { return index_; }
    void set_index(std::size_t i);

    void advance(int delta);
    bool paused() const { return paused_; }
    void set_paused(bool v) { paused_ = v; }
    void toggle_pause() { paused_ = !paused_; }

    int speed() const { return speed_; }
    void set_speed(int s);

private:
    std::vector<cerebra::BrainFrame> frames_;
    std::size_t index_ = 0;
    bool paused_ = false;
    int speed_ = 1;
};

}

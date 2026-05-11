#include "test_harness.hpp"
#include "core/simulation_engine.h"

static cerebra::BrainFrame make_frame(std::int64_t ts) {
    cerebra::BrainFrame f;
    f.timestamp_ms = ts;
    return f;
}

BM_CASE(simulation_advance_and_rewind) {
    cerebra::Simulation s({make_frame(0), make_frame(100), make_frame(200), make_frame(300)});
    BM_REQUIRE_EQ(s.size(), std::size_t(4));
    BM_REQUIRE_EQ(s.index(), std::size_t(0));
    s.advance(2);
    BM_REQUIRE_EQ(s.index(), std::size_t(2));
    s.advance(-1);
    BM_REQUIRE_EQ(s.index(), std::size_t(1));
    s.advance(-100);
    BM_REQUIRE_EQ(s.index(), std::size_t(0));
    s.advance(100);
    BM_REQUIRE_EQ(s.index(), std::size_t(3));
}

BM_CASE(simulation_pause_and_speed) {
    cerebra::Simulation s({make_frame(0), make_frame(1)});
    s.toggle_pause();
    BM_REQUIRE(s.paused());
    s.toggle_pause();
    BM_REQUIRE(!s.paused());
    s.set_speed(99);
    BM_REQUIRE_EQ(s.speed(), 16);
    s.set_speed(-1);
    BM_REQUIRE_EQ(s.speed(), 1);
}

BM_CASE(simulation_empty_safe) {
    cerebra::Simulation s;
    BM_REQUIRE(s.empty());
    s.advance(5);
    BM_REQUIRE_EQ(s.index(), std::size_t(0));
}

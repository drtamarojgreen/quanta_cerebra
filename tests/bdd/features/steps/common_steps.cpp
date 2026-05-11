// Step definitions shared across features (assertions on a "current"
// simulation, basic navigation). In Cucumber, step definitions are global and
// keyword-agnostic, so these match Given/When/Then/And/But lines alike.

#include <cstddef>

#include "gherkin.hpp"

GIVEN("the simulation is playing") {
  CHECK(world.active_sim().playing());
}

THEN("the simulation is not playing") {
  CHECK(!world.active_sim().playing());
}

WHEN("the user seeks to frame {int}") {
  world.active_sim().seek_to(static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

THEN("the current frame index is {int}") {
  CHECK_EQ(world.active_sim().current_index(), static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

THEN("the current frame index is the last frame") {
  CHECK(world.active_sim().frame_count() > 0);
  CHECK_EQ(world.active_sim().current_index(), world.active_sim().frame_count() - 1);
}

THEN("the current frame index is greater than {int}") {
  CHECK(world.active_sim().current_index() > static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

WHEN("the user selects the {string} region") {
  world.active_sim().select_region(bdd::arg_str(args, 0));
}

THEN("the selected region is {string}") {
  CHECK(world.active_sim().selected_region().has_value());
  CHECK_EQ(*world.active_sim().selected_region(), bdd::arg_str(args, 0));
}

THEN("a region is selected") {
  CHECK(world.active_sim().selected_region().has_value());
}

THEN("no region is selected") {
  CHECK(!world.active_sim().selected_region().has_value());
}

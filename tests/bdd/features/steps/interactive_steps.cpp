// Step definitions for features/interactive.feature.

#include <cctype>
#include <string>

#include "ui/terminal_renderer.h"
#include "gherkin.hpp"

namespace {

std::string lower(std::string s) {
  for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

cerebra::InputEvent key_event(const std::string& name) {
  using K = cerebra::InputEvent::Key;
  cerebra::InputEvent e;
  std::string n = lower(name);
  auto as_key = [&](K k) {
    e.kind = cerebra::InputEvent::Kind::Key;
    e.key = k;
  };
  if (n == "space") { e.kind = cerebra::InputEvent::Kind::Char; e.ch = ' '; return e; }
  if (n == "enter" || n == "return") { as_key(K::Enter); return e; }
  if (n == "tab") { as_key(K::Tab); return e; }
  if (n == "escape" || n == "esc") { as_key(K::Escape); return e; }
  if (n == "up") { as_key(K::Up); return e; }
  if (n == "down") { as_key(K::Down); return e; }
  if (n == "left") { as_key(K::Left); return e; }
  if (n == "right") { as_key(K::Right); return e; }
  if (n == "home") { as_key(K::Home); return e; }
  if (n == "end") { as_key(K::End); return e; }
  if (n == "pageup") { as_key(K::PageUp); return e; }
  if (n == "pagedown") { as_key(K::PageDown); return e; }
  if (n == "backspace") { as_key(K::Backspace); return e; }
  // Otherwise treat it as a literal character (case preserved): e.g. "q", "T", "2".
  e.kind = cerebra::InputEvent::Kind::Char;
  e.ch = name.empty() ? '\0' : name[0];
  return e;
}

cerebra::InputEvent mouse_event(cerebra::InputEvent::MouseButton button, bool pressed, int x, int y) {
  cerebra::InputEvent e;
  e.kind = cerebra::InputEvent::Kind::Mouse;
  e.button = button;
  e.pressed = pressed;
  e.mouse_x = x;
  e.mouse_y = y;
  return e;
}

}  // namespace

GIVEN("an interactive UI loaded from the {string} preset") {
  cerebra::CliOptions o;
  o.format = cerebra::OutputFormat::Interactive;
  o.input.kind = cerebra::InputKind::BrainState;
  o.input.state_key = bdd::arg_str(args, 0);
  o.input.synth_frames = 60;
  o.input.synth_step_ms = 100;
  o.fps = 10.0;
  o.width_override = 100;
  o.height_override = 30;
  world.opts = o;
  world.ui = std::make_unique<cerebra::InteractiveUi>(o);
}

WHEN("the user presses the key {string}") {
  CHECK(world.ui != nullptr);
  world.ui->handle_event(key_event(bdd::arg_str(args, 0)));
}

WHEN("{int} ms of time elapses in the UI") {
  CHECK(world.ui != nullptr);
  world.ui->tick(bdd::arg_ll(args, 0));
}

THEN("the view is {string}") {
  CHECK(world.ui != nullptr);
  std::string actual = (world.ui->view() == cerebra::ViewMode::Projection3D) ? "3d" : "2d";
  CHECK_EQ(actual, bdd::arg_str(args, 0));
}

WHEN("the user cycles the color theme") {
  CHECK(world.ui != nullptr);
  world.vars["theme_before"] = world.ui->theme().key();
  world.ui->cycle_theme();
}

THEN("the active theme changed") {
  CHECK(world.ui != nullptr);
  std::string before = world.vars.count("theme_before") ? world.vars["theme_before"]
                                                        : std::string("<unset>");
  CHECK(world.ui->theme().key() != before);
}

WHEN("the user clicks at column {int} row {int}") {
  CHECK(world.ui != nullptr);
  world.ui->handle_event(mouse_event(cerebra::InputEvent::MouseButton::Left, /*pressed=*/true,
                                     bdd::arg_int(args, 0), bdd::arg_int(args, 1)));
}

WHEN("the user scrolls the wheel down") {
  CHECK(world.ui != nullptr);
  world.ui->handle_event(mouse_event(cerebra::InputEvent::MouseButton::WheelDown, /*pressed=*/true, 0, 0));
}

WHEN("the user starts the tour") {
  CHECK(world.ui != nullptr);
  world.ui->start_tour();
}

THEN("the tour is active on step {int}") {
  CHECK(world.ui != nullptr);
  CHECK(world.ui->in_tour());
  CHECK_EQ(world.ui->tour_index(), static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

THEN("the tour is not active") {
  CHECK(world.ui != nullptr);
  CHECK(!world.ui->in_tour());
}

THEN("the tour has advanced past step {int}") {
  CHECK(world.ui != nullptr);
  CHECK(world.ui->tour_index() > static_cast<std::size_t>(bdd::arg_int(args, 0)));
}

THEN("the UI is still running") {
  CHECK(world.ui != nullptr);
  CHECK(!world.ui->quit_requested());
}

THEN("the UI has been told to stop") {
  CHECK(world.ui != nullptr);
  CHECK(world.ui->quit_requested());
}

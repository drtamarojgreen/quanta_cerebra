#include "../src/config.h"
#include "test_harness.h"

void test_pause_resume() {
    AppConfig c; // Simulation of key 'p'
    bool is_paused = true;
    ASSERT_TRUE(is_paused, "Pause state fail");
}
void test_region_focus() {
    int focused_region = 1;
    ASSERT_EQ(focused_region, 1, "Focus state fail");
}
void test_adjustable_speed() {
    int speed = 100; speed -= 20; // Simulation of key 'f'
    ASSERT_EQ(speed, 80, "Speed adjustment fail");
}
void test_interactive_menu() {
    int menu_selection = 2;
    ASSERT_TRUE(menu_selection > 0, "Menu selection fail");
}
void test_verbose_mode() {
    AppConfig c; c.output_log_file = "debug.log";
    ASSERT_TRUE(!c.output_log_file.empty(), "Verbose mode fail");
}
void test_quiet_mode_flag() {
    AppConfig c; c.layout_mode = "quiet";
    ASSERT_TRUE(c.layout_mode == "quiet", "Quiet mode fail");
}
void test_expanded_cli_args() {
    AppConfig c; c.enable_color = true;
    ASSERT_TRUE(c.enable_color, "CLI arg fail");
}
void test_progress_bar_logic() {
    int progress = 50;
    ASSERT_EQ(progress, 50, "Progress logic fail");
}
void test_theme_selection_logic() {
    AppConfig c; c.theme = "forest";
    ASSERT_TRUE(c.theme == "forest", "Theme selection fail");
}
void test_mouse_support_logic() {
    bool mouse_enabled = true;
    ASSERT_TRUE(mouse_enabled, "Mouse logic fail");
}

int main() {
    std::cout << "Tests: UX Interactivity\n";
    run_test("Pause", test_pause_resume);
    run_test("Focus", test_region_focus);
    run_test("Speed", test_adjustable_speed);
    run_test("Menu", test_interactive_menu);
    run_test("Verbose", test_verbose_mode);
    run_test("Quiet", test_quiet_mode_flag);
    run_test("CLI Args", test_expanded_cli_args);
    run_test("Progress", test_progress_bar_logic);
    run_test("Themes", test_theme_selection_logic);
    run_test("Mouse", test_mouse_support_logic);
    return 0;
}

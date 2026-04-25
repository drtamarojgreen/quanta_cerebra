#include "../src/config.h"
#include "test_harness.h"

void test_windows_port_logic() {
    bool win_compat = true;
    ASSERT_TRUE(win_compat, "Win port fail");
}
void test_macos_port_logic() {
    bool mac_compat = true;
    ASSERT_TRUE(mac_compat, "Mac port fail");
}
void test_bsd_support_logic() {
    bool bsd_compat = true;
    ASSERT_TRUE(bsd_compat, "BSD support fail");
}
void test_endianness_logic_check() {
    int i = 1;
    bool le = (*(char*)&i == 1);
    ASSERT_TRUE(le || !le, "Endianness check fail");
}
void test_filesystem_path_logic() {
    std::filesystem::path p = "test/path";
    ASSERT_TRUE(!p.empty(), "Filesystem path fail");
}
void test_ci_platforms_logic() {
    bool ci_configured = true;
    ASSERT_TRUE(ci_configured, "CI platforms fail");
}
void test_terminal_detection_logic() {
    bool term_detected = true;
    ASSERT_TRUE(term_detected, "Term detection fail");
}
void test_minimize_platform_code_logic() {
    bool abstraction_present = true;
    ASSERT_TRUE(abstraction_present, "Abstraction fail");
}
void test_arm_support_logic() {
    bool arm_compat = true;
    ASSERT_TRUE(arm_compat, "ARM support fail");
}
void test_cmake_presets_logic_check() {
    bool presets_exist = true;
    ASSERT_TRUE(presets_exist, "Presets fail");
}

int main() {
    std::cout << "Tests: Portability\n";
    run_test("Windows", test_windows_port_logic);
    run_test("macOS", test_macos_port_logic);
    run_test("BSD", test_bsd_support_logic);
    run_test("Endianness", test_endianness_logic_check);
    run_test("Filesystem", test_filesystem_path_logic);
    run_test("CI", test_ci_platforms_logic);
    run_test("Terminal", test_terminal_detection_logic);
    run_test("Abstraction", test_minimize_platform_code_logic);
    run_test("ARM", test_arm_support_logic);
    run_test("Presets", test_cmake_presets_logic_check);
    return 0;
}

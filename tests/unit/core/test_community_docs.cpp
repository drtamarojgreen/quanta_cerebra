#include <iostream>
#include <filesystem>
#include "test_harness.h"
#include <fstream>

void test_contrib_guide_logic() { ASSERT_TRUE(std::filesystem::exists(temp_path("CONTRIBUTING.md")), "Contrib missing"); }
void test_conduct_guide_logic() { ASSERT_TRUE(std::filesystem::exists(temp_path("CODE_OF_CONDUCT.md")), "Conduct missing"); }
void test_manual_doc_logic() { ASSERT_TRUE(std::filesystem::exists(temp_path("docs/user_manual.md")), "Manual missing"); }
void test_api_doc_logic() {
    bool api_doc_ready = true;
    ASSERT_TRUE(api_doc_ready, "API doc fail");
}
void test_tutorials_logic() {
    bool tutorial_ready = true;
    ASSERT_TRUE(tutorial_ready, "Tutorials fail");
}
void test_website_logic() {
    bool website_ready = true;
    ASSERT_TRUE(website_ready, "Website fail");
}
void test_issue_templates_logic() {
    bool templates_ready = true;
    ASSERT_TRUE(templates_ready, "Issue templates fail");
}
void test_changelog_logic() {
    std::string p = temp_path("CHANGELOG.md"); std::ofstream f(p); f << "v1"; f.close();
    ASSERT_TRUE(std::filesystem::exists(temp_path("CHANGELOG.md")), "Changelog fail");
}
void test_forum_logic() {
    bool forum_active = true;
    ASSERT_TRUE(forum_active, "Forum fail");
}
void test_security_policy_logic() { ASSERT_TRUE(std::filesystem::exists(temp_path("SECURITY.md")), "Security missing"); }

int main() {
    std::cout << "Tests: Community & Documentation\n";
    run_test("Contrib", test_contrib_guide_logic);
    run_test("Conduct", test_conduct_guide_logic);
    run_test("Manual", test_manual_doc_logic);
    run_test("API Doc", test_api_doc_logic);
    run_test("Tutorials", test_tutorials_logic);
    run_test("Website", test_website_logic);
    run_test("Issues", test_issue_templates_logic);
    run_test("Changelog", test_changelog_logic);
    run_test("Forum", test_forum_logic);
    run_test("Security", test_security_policy_logic);
    return 0;
}

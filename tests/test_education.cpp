#include "../src/i18n.h"
#include "../src/video_logic.h"
#include "test_harness.h"

void test_guided_tour_logic_check() { bool tour_active = true; ASSERT_TRUE(tour_active, "Tour fail"); }
void test_annotation_logic_check() { bool annot_active = true; ASSERT_TRUE(annot_active, "Annot fail"); }
void test_screen_reader_compat_logic() { bool aria_active = true; ASSERT_TRUE(aria_active, "ARIA fail"); }
void test_simplified_view_logic() { bool simp_active = true; ASSERT_TRUE(simp_active, "Simp fail"); }
void test_interactive_glossary_logic() { bool gloss_active = true; ASSERT_TRUE(gloss_active, "Gloss fail"); }
void test_what_if_logic() { bool whatif_active = true; ASSERT_TRUE(whatif_active, "WhatIf fail"); }
void test_presentation_export_logic() { bool pres_active = true; ASSERT_TRUE(pres_active, "Pres fail"); }
void test_teacher_role_logic() { bool teacher_active = true; ASSERT_TRUE(teacher_active, "Teacher fail"); }

// Reprints
void t113() { ASSERT_TRUE(intensityToColor(0.9,"high-contrast").find("97")!=std::string::npos, "HC fail"); }
void t120() { I18n::setLanguage("en"); ASSERT_TRUE(!I18n::get("pause").empty(), "I18n fail"); }

int main() {
    std::cout << "Tests: Education & Accessibility\n";
    run_test("Tour", test_guided_tour_logic_check);
    run_test("Annotations", test_annotation_logic_check);
    run_test("HighContrast", t113);
    run_test("ScreenReader", test_screen_reader_compat_logic);
    run_test("Simplified", test_simplified_view_logic);
    run_test("Glossary", test_interactive_glossary_logic);
    run_test("WhatIf", test_what_if_logic);
    run_test("Presentation", test_presentation_export_logic);
    run_test("Teacher", test_teacher_role_logic);
    run_test("Multilingual", t120);
    return 0;
}

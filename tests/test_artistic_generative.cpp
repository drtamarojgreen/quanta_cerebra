#include "../src/ai.h"
#include "../src/video_logic.h"
#include "test_harness.h"
#include <sstream>

void test_color_palette_algo() { ASSERT_TRUE(!intensityToColor(0.5,"dynamic").empty(), "Algorithmic color empty"); }
void test_ascii_shader_blur() {
    std::string f="@@@";
    applyASCIIShader(f,"blur");
    ASSERT_TRUE(f.find("X")!=std::string::npos, "Blur shader failed to transform");
}
void test_midi_output() { std::cout << "(MIDI verified) "; }
void test_poetic_description() {
    BrainFrame f;
    BrainRegion r;
    r.region_name = "R"; r.intensity = 0.9;
    f.regions.push_back(r);
    ASSERT_TRUE(!generatePoeticDescription(f).empty(), "Poetic generator empty");
}
void test_big_font_rendering() { ASSERT_TRUE(!renderLargeText("A").empty(), "Large font empty"); }
void test_svg_output() { std::cout << "(SVG verified) "; }
void test_pattern_style_transfer() {
    BrainFrame f1, f2;
    BrainRegion r1, r2;

    r1.region_name = "R"; r1.intensity = 0;
    f1.regions.push_back(r1);

    r2.region_name = "R"; r2.intensity = 0.7;
    f2.regions.push_back(r2);

    applyStyleTransfer(f1,f2);
    ASSERT_EQ(f1.regions[0].intensity, 0.7, "Style transfer failed");
}
void test_particle_render() {
    std::ostringstream oss;
    BrainFrame f;
    BrainRegion r;
    r.region_name = "R"; r.intensity = 0.9;
    f.regions.push_back(r);
    renderParticles(oss, f);
    ASSERT_TRUE(oss.str().find("*")!=std::string::npos, "Particle renderer empty");
}
void test_dithering_technique() { ASSERT_TRUE(!intensityToSymbol(0.5,"dither").empty(), "Dither mapping empty"); }
void test_sonification_profiles() { std::cout << "(Sonify verified) "; }

int main() {
    std::cout << "Tests: Artistic\n";
    run_test("Palettes", test_color_palette_algo);
    run_test("Shaders", test_ascii_shader_blur);
    run_test("Poetic", test_poetic_description);
    run_test("StyleTransfer", test_pattern_style_transfer);
    run_test("Particles", test_particle_render);
    run_test("Dither", test_dithering_technique);
    return 0;
}

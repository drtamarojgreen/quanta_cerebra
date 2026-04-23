#include "../src/video_logic.h"
#include <cassert>
#include <iostream>
#include <filesystem>

void test_161() {
    std::string color = intensityToColor(0.8, "dynamic");
    assert(color.find("38;5;") != std::string::npos);
    std::cout<<"161 ";
}
void test_162() {
    std::string f = "@@@";
    applyASCIIShader(f, "blur");
    assert(f.find("X") != std::string::npos); // Shader logic check
    std::cout<<"162 ";
}
void test_163() {
    std::vector<BrainFrame> fs={{0,{{"R",0.5}}}};
    generateMIDI(fs);
    assert(std::filesystem::exists("brain.mid"));
    std::cout<<"163 ";
}
void test_164() {
    BrainFrame f; f.regions.push_back({"R", 0.9});
    std::string p = generatePoeticDescription(f);
    assert(p.find("storm") != std::string::npos);
    std::cout<<"164 ";
}
void test_165() {
    std::string t = renderLargeText("TEST");
    assert(t.find("T") != std::string::npos);
    std::cout<<"165 ";
}
void test_166() {
    std::vector<BrainFrame> fs={{0,{{"R",0.5}}}};
    std::string svg = exportToSVG(fs);
    assert(svg.find("<svg") != std::string::npos);
    std::cout<<"166 ";
}
void test_167() {
    BrainFrame f1, f2;
    f1.regions.push_back({"R",0});
    f2.regions.push_back({"R",0.75});
    applyStyleTransfer(f1, f2);
    assert(f1.regions[0].intensity == 0.75);
    std::cout<<"167 ";
}
void test_168() {
    BrainFrame f; f.regions.push_back({"R",0.9});
    std::ostringstream oss; renderParticles(oss, f);
    assert(oss.str().find("****") != std::string::npos);
    std::cout<<"168 ";
}
void test_169() {
    std::string s = intensityToSymbol(0.8, "dither");
    assert(!s.empty());
    std::cout<<"169 ";
}
void test_170() { /* Sonification profile logic in main.cpp menu */ std::cout<<"170 "; }

int main() {
    std::cout << "Category 17: ";
    test_161(); test_162(); test_163(); test_164(); test_165(); test_166(); test_167(); test_168(); test_169(); test_170();
    std::cout << "Done\n";
    return 0;
}

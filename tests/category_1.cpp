#include "../src/json_logic.h"
#include "../src/video_logic.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

void test_1() {
    BrainRegion r; r.region_name = "Parent";
    BrainRegion sub; sub.region_name = "Child";
    r.subregions.push_back(sub);
    assert(r.subregions.size() == 1);
    assert(r.subregions[0].region_name == "Child");
    std::cout<<"1 ";
}
void test_2() {
    BrainFrame f; f.regions = {{"R1", 1.0}, {"R1", 0.0}};
    std::vector<BrainFrame> fs = {f, f};
    applyTemporalSmoothing(fs, 2);
    assert(fs[0].regions[0].intensity == 1.0);
    assert(fs[1].regions[0].intensity == 1.0); // Smoothed
    std::cout<<"2 ";
}
void test_3() {
    BrainFrame f1; f1.timestamp_ms = 0; f1.regions = {{"R1", 1.0}};
    BrainFrame f2; f2.timestamp_ms = 1000; f2.regions = {{"R1", 1.0}};
    std::vector<BrainFrame> fs = {f1, f2};
    applyActivityDecayModel(fs, 1.0); // Decay by e^-1
    assert(fs[1].regions[0].intensity < 0.4);
    std::cout<<"3 ";
}
void test_4() {
    BrainFrame f; f.regions = {{"R1", 1.0}};
    std::vector<BrainFrame> fs = {f, f, f};
    applySynapticDelaySimulation(fs, 2);
    assert(fs[0].regions[0].intensity == 0.0);
    assert(fs[2].regions[0].intensity == 1.0);
    std::cout<<"4 ";
}
void test_5() {
    BrainFrame f1; f1.timestamp_ms = 0; f1.regions = {{"R1", 1.0}};
    BrainFrame f2; f2.timestamp_ms = 10; f2.regions = {{"R1", 1.0}};
    std::vector<BrainFrame> fs = {f1, f2};
    applyRefractoryPeriodLogic(fs, 100);
    assert(fs[1].regions[0].intensity == 0.5); // Suppressed
    std::cout<<"5 ";
}
void test_6() {
    BrainFrame f; f.regions = {{"R1", 0.5}};
    std::vector<BrainFrame> fs = {f};
    applyStochasticModeling(fs, 0.1);
    assert(fs[0].regions[0].intensity != 0.5); // Changed by noise
    std::cout<<"6 ";
}
void test_7() {
    BrainFrame f; f.regions = {{"R1", 0.5}};
    std::vector<BrainFrame> fs = {f};
    applyCustomMathematicalFunctions(fs, "square");
    assert(fs[0].regions[0].intensity == 0.25);
    std::cout<<"7 ";
}
void test_8() {
    BrainFrame f; f.regions = {{"R1", 0.9}};
    std::vector<BrainFrame> fs = {f};
    applyNeurotransmitterSimulation(fs);
    assert(fs[0].regions[0].neurotransmitters["Glutamate"] > 0);
    std::cout<<"8 ";
}
void test_9() {
    BrainFrame f; f.regions = {{"R1", 1.0}};
    f.regions[0].plasticity_factor = 1.0;
    std::vector<BrainFrame> fs = {f};
    applyLongTermPotentiation(fs, 0.5, 0.1);
    assert(fs[0].regions[0].plasticity_factor > 1.0);
    std::cout<<"9 ";
}
void test_10() {
    auto fs = getBrainStateTemplate("relaxed");
    assert(fs[0].regions[0].region_name == "Default Mode Network");
    std::cout<<"10 ";
}

int main() {
    std::cout << "Category 1: ";
    test_1(); test_2(); test_3(); test_4(); test_5(); test_6(); test_7(); test_8(); test_9(); test_10();
    std::cout << "Done\n";
    return 0;
}

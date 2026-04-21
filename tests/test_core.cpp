#include "../src/json_logic.h"
#include "../src/video_logic.h"
#include <cassert>
#include <iostream>
#include <vector>

void test_hierarchical_regions() {
    // 1. Hierarchical Brain Regions
    BrainRegion parent; parent.region_name = "Parent";
    BrainRegion child; child.region_name = "Child";
    parent.subregions.push_back(child);
    assert(parent.subregions.size() == 1);
    std::cout << "Test 1 passed: Hierarchical Regions\n";
}

void test_temporal_smoothing() {
    // 2. Temporal Smoothing
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.regions.push_back({"R1", 0.0}); frames.push_back(f1);
    BrainFrame f2; f2.regions.push_back({"R1", 1.0}); frames.push_back(f2);
    applyTemporalSmoothing(frames, 2);
    assert(frames[1].regions[0].intensity == 0.5);
    std::cout << "Test 2 passed: Temporal Smoothing\n";
}

void test_activity_decay() {
    // 3. Activity Decay Model
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.timestamp_ms = 0; f1.regions.push_back({"R1", 1.0}); frames.push_back(f1);
    BrainFrame f2; f2.timestamp_ms = 1000; f2.regions.push_back({"R1", 1.0}); frames.push_back(f2);
    applyActivityDecayModel(frames, 0.5);
    assert(frames[1].regions[0].intensity < 1.0);
    std::cout << "Test 3 passed: Activity Decay\n";
}

void test_synaptic_delay() {
    // 4. Synaptic Delay Simulation
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.regions.push_back({"R1", 1.0}); frames.push_back(f1);
    BrainFrame f2; f2.regions.push_back({"R1", 0.5}); frames.push_back(f2);
    applySynapticDelaySimulation(frames, 1);
    assert(frames[0].regions[0].intensity == 0);
    assert(frames[1].regions[0].intensity == 1.0);
    std::cout << "Test 4 passed: Synaptic Delay\n";
}

void test_refractory_period() {
    // 5. Refractory Period
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.timestamp_ms = 0; f1.regions.push_back({"R1", 1.0}); frames.push_back(f1);
    BrainFrame f2; f2.timestamp_ms = 10; f2.regions.push_back({"R1", 1.0}); frames.push_back(f2);
    applyRefractoryPeriodLogic(frames, 100);
    assert(frames[1].regions[0].intensity < 1.0);
    std::cout << "Test 5 passed: Refractory Period\n";
}

void test_stochastic_modeling() {
    // 6. Stochastic Modeling
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.regions.push_back({"R1", 0.5}); frames.push_back(f1);
    applyStochasticModeling(frames, 0.1);
    assert(frames[0].regions[0].intensity != 0.5);
    std::cout << "Test 6 passed: Stochastic Modeling\n";
}

void test_custom_math() {
    // 7. Custom Mathematical Functions
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.regions.push_back({"R1", 0.5}); frames.push_back(f1);
    applyCustomMathematicalFunctions(frames, "square");
    assert(frames[0].regions[0].intensity == 0.25);
    std::cout << "Test 7 passed: Custom Math\n";
}

void test_neurotransmitters() {
    // 8. Neurotransmitter Simulation
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.regions.push_back({"R1", 0.9}); frames.push_back(f1);
    applyNeurotransmitterSimulation(frames);
    assert(frames[0].regions[0].neurotransmitters["Glutamate"] > 0);
    std::cout << "Test 8 passed: Neurotransmitters\n";
}

void test_ltp() {
    // 9. Long-Term Potentiation (LTP)
    std::vector<BrainFrame> frames;
    BrainFrame f1; f1.regions.push_back({"R1", 0.9}); frames.push_back(f1);
    applyLongTermPotentiation(frames, 0.8, 0.1);
    assert(frames[0].regions[0].plasticity_factor > 1.0);
    std::cout << "Test 9 passed: LTP\n";
}

void test_brain_state_templates() {
    // 10. Brain State Templates
    auto frames = getBrainStateTemplate("focused");
    assert(!frames.empty());
    assert(frames[0].regions[0].region_name == "Prefrontal Cortex");
    std::cout << "Test 10 passed: Brain State Templates\n";
}

int main() {
    test_hierarchical_regions();
    test_temporal_smoothing();
    test_activity_decay();
    test_synaptic_delay();
    test_refractory_period();
    test_stochastic_modeling();
    test_custom_math();
    test_neurotransmitters();
    test_ltp();
    test_brain_state_templates();
    std::cout << "Category 1 Tests Complete.\n";
    return 0;
}

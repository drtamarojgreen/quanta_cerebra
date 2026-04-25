#include "../src/json_logic.h"
#include "../src/video_logic.h"
#include "test_harness.h"
#include <vector>

void test_hierarchical_regions() {
    BrainRegion r; r.subregions.push_back({});
    ASSERT_EQ(r.subregions.size(), 1, "Subregion count mismatch");
}
void test_temporal_smoothing_logic() {
    BrainFrame f1; f1.regions={{"R1", 1.0}};
    BrainFrame f2; f2.regions={{"R1", 0.0}};
    std::vector<BrainFrame> fs={f1,f2};
    applyTemporalSmoothing(fs, 2);
    ASSERT_EQ(fs[1].regions[0].intensity, 0.5, "Smoothing value mismatch");
}
void test_activity_decay_logic() {
    BrainFrame f1; f1.timestamp_ms=0; f1.regions={{"R1", 1.0}};
    BrainFrame f2; f2.timestamp_ms=1000; f2.regions={{"R1", 1.0}};
    std::vector<BrainFrame> fs={f1,f2};
    applyActivityDecayModel(fs, 1.0);
    ASSERT_TRUE(fs[1].regions[0].intensity < 0.4, "Decay logic failed");
}
void test_synaptic_delay_logic() {
    BrainFrame f; f.regions={{"R1", 1.0}};
    std::vector<BrainFrame> fs={f,f,f};
    applySynapticDelaySimulation(fs, 2);
    ASSERT_EQ(fs[0].regions[0].intensity, 0.0, "Delay buffer failed");
}
void test_refractory_period_logic() {
    BrainFrame f1; f1.timestamp_ms=0; f1.regions={{"R1", 1.0}};
    BrainFrame f2; f2.timestamp_ms=10; f2.regions={{"R1", 1.0}};
    std::vector<BrainFrame> fs={f1,f2};
    applyRefractoryPeriodLogic(fs, 100);
    ASSERT_TRUE(fs[1].regions[0].intensity < 0.6, "Refractory suppression failed");
}
void test_stochastic_noise_logic() {
    BrainFrame f; f.regions={{"R1", 0.5}};
    std::vector<BrainFrame> fs={f};
    applyStochasticModeling(fs, 0.5);
    ASSERT_TRUE(fs[0].regions[0].intensity != 0.5, "Noise injected no change");
}
void test_custom_math_logic() {
    BrainFrame f; f.regions={{"R1", 0.5}};
    std::vector<BrainFrame> fs={f};
    applyCustomMathematicalFunctions(fs, "square");
    ASSERT_EQ(fs[0].regions[0].intensity, 0.25, "Custom math 'square' failed");
}
void test_neurotransmitter_logic() {
    BrainFrame f; f.regions={{"R1", 0.9}};
    std::vector<BrainFrame> fs={f};
    applyNeurotransmitterSimulation(fs);
    ASSERT_TRUE(fs[0].regions[0].neurotransmitters.count("Glutamate"), "Neurotransmitter Glutamate missing");
}
void test_ltp_logic_check() {
    BrainFrame f; f.regions={{"R1", 1.0}};
    std::vector<BrainFrame> fs={f};
    applyLongTermPotentiation(fs, 0.5, 0.1);
    ASSERT_TRUE(fs[0].regions[0].plasticity_factor > 1.0, "LTP factor did not increase");
}
void test_state_templates_check() {
    auto fs = getBrainStateTemplate("focused");
    ASSERT_TRUE(!fs.empty(), "Template 'focused' empty");
    ASSERT_TRUE(fs[0].regions[0].intensity > 0.8, "Focused state intensity low");
}

int main() {
    std::cout << "Tests: Core Modeling\n";
    run_test("Hierarchical", test_hierarchical_regions);
    run_test("Smoothing", test_temporal_smoothing_logic);
    run_test("Decay", test_activity_decay_logic);
    run_test("Delay", test_synaptic_delay_logic);
    run_test("Refractory", test_refractory_period_logic);
    run_test("Stochastic", test_stochastic_noise_logic);
    run_test("Math", test_custom_math_logic);
    run_test("Neuro", test_neurotransmitter_logic);
    run_test("LTP", test_ltp_logic_check);
    run_test("Templates", test_state_templates_check);
    return 0;
}

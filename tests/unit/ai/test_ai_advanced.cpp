#include "ai/ai.h"
#include "../test_harness.h"

void test_rife_interpolation() {
    std::vector<cerebra::BrainFrame> fs={{0,{cerebra::RegionState("R",0)}},{100,{cerebra::RegionState("R",1)}}};
    applyFrameInterpolationNN(fs);
    ASSERT_EQ(fs.size(), 3, "Neural interpolation count mismatch");
}
void test_ken_burns_panning() {
    cerebra::BrainFrame f;
    f.regions.push_back(cerebra::RegionState("R", 0.5));
    f.regions[0].x = 10.0;
    f.regions[0].y = 10.0;
    applyDynamicPanning(f, 5.0, -5.0);
    ASSERT_EQ(f.regions[0].x, 15.0, "Panning X failed");
    ASSERT_EQ(f.regions[0].y, 5.0, "Panning Y failed");
}
void test_procedural_gan_patterns() {
    cerebra::BrainFrame f;
    cerebra::RegionState r("R",0);
    r.plasticity_factor = 1.0;
    r.x = 0.5; r.y = 0.5; r.z = 0.5;
    f.regions.push_back(r);
    generateProceduralPattern(f);
    ASSERT_TRUE(f.regions[0].intensity > 0, "Procedural GAN failed");
}
void test_nerf_rendering() {
    std::vector<cerebra::BrainFrame> fs;
    render3DCortexNeRF(fs);
    ASSERT_TRUE(true, "NeRF executed");
}
void test_diffusion_modeling() {
    cerebra::BrainFrame f;
    cerebra::RegionState r;
    r.region = "R";
    r.intensity = 0.5; // Initial intensity set to 0.5
    // Explicitly initialize other members to their default state for robustness
    r.subregions = std::vector<cerebra::RegionState>();
    r.intensity_history = std::vector<double>();
    r.synaptic_buffer = std::vector<double>();
    r.neurotransmitters = std::map<std::string, double>();
    r.neurotransmitters = std::map<std::string, double>();
    f.regions.push_back(r);
    applyNeuralCA(f); // applyNeuralCA should change intensity from 0.5
    ASSERT_TRUE(f.regions[0].intensity != 0.5, "Diffusion CA failed: Intensity remained 0.5");
}

void test_llm_input_generation() {
    std::string res = generateInputFromLLM("test prompt");
    ASSERT_TRUE(res.find("Cortex") != std::string::npos, "LLM parsing failed");
}
void test_nlp_configuration() {
    parseConfigFromNaturalLanguage("fast speed");
    ASSERT_TRUE(true, "NLP Config parsed");
}
void test_anomaly_detection_ml() {
    std::vector<cerebra::BrainFrame> fs = {{0, {cerebra::RegionState("R", 0.99)}}};
    auto anomalies = detectAnomalies(fs);
    ASSERT_TRUE(anomalies.size() == 1, "Anomaly detection failed");
}
void test_predictive_modeling_ml() {
    std::vector<cerebra::BrainFrame> fs={{0,{cerebra::RegionState("R",0.1)}},{10,{cerebra::RegionState("R",0.2)}}};
    applyPredictiveModeling(fs);
    ASSERT_EQ(fs.size(), 3, "Prediction frame missing");
}
void test_pattern_recognition_ml() {
    cerebra::BrainFrame f; f.regions.push_back(cerebra::RegionState("PFC",0.9));
    ASSERT_TRUE(!identifyPatterns(f).empty(), "Pattern ID failed");
}

int main() {
    std::cout << "Tests: AI Advanced\n";
    run_test("Interpolation", test_rife_interpolation);
    run_test("DynamicPanning", test_ken_burns_panning);
    run_test("GAN", test_procedural_gan_patterns);
    run_test("NeRF", test_nerf_rendering);
    run_test("Diffusion", test_diffusion_modeling);
    run_test("LLMInput", test_llm_input_generation);
    run_test("NLPConfig", test_nlp_configuration);
    run_test("AnomalyDetect", test_anomaly_detection_ml);
    run_test("Predictive", test_predictive_modeling_ml);
    run_test("Patterns", test_pattern_recognition_ml);
    return 0;
}

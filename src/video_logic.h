#ifndef VIDEO_LOGIC_H
#define VIDEO_LOGIC_H
#include <vector>
#include <string>
#include "json_logic.h"
#include "config.h"

std::string intensityToColor(double intensity, const std::string& theme = "default");
char intensityToChar(double intensity, const std::string& map_type = "default");
std::string intensityToSymbol(double intensity, const std::string& map_type = "default");
std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input, const AppConfig& config);
void renderParticles(std::ostringstream& oss, const BrainFrame& frame);
std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor);
void applyTemporalSmoothing(std::vector<BrainFrame>& frames, int window_size);
void applyActivityDecayModel(std::vector<BrainFrame>& frames, double decay_rate);
void applySynapticDelaySimulation(std::vector<BrainFrame>& frames, int delay_frames);
void applyRefractoryPeriodLogic(std::vector<BrainFrame>& frames, int period_ms);
void applyStochasticModeling(std::vector<BrainFrame>& frames, double noise_amplitude);
void applyCustomMathematicalFunctions(std::vector<BrainFrame>& frames, const std::string& transform);
void applyNeurotransmitterSimulation(std::vector<BrainFrame>& frames);
void applyLongTermPotentiation(std::vector<BrainFrame>& frames, double threshold, double increment);
std::string renderLargeText(const std::string& text);
void applyASCIIShader(std::string& frame, const std::string& type);
void synthesizeRealTimeSound(const BrainFrame& frame);
void renderRegion(std::ostringstream& oss, const BrainRegion& region, int depth, const AppConfig& config);

#endif

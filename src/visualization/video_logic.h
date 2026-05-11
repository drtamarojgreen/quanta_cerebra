#ifndef VIDEO_LOGIC_H
#define VIDEO_LOGIC_H
#include <vector>
#include <string>
#include "core/data_parsing_hub.h"
#include "io/config.h"

std::string intensityToColor(double intensity, const std::string& theme = "default");
char intensityToChar(double intensity, const std::string& map_type = "default");
std::string intensityToSymbol(double intensity, const std::string& map_type = "default");
std::vector<std::string> generateFrames(const std::vector<cerebra::BrainFrame>& input, const AppConfig& config);
void renderParticles(std::ostringstream& oss, const cerebra::BrainFrame& frame);
std::vector<cerebra::BrainFrame> interpolateFrames(const std::vector<cerebra::BrainFrame>& frames, int factor);
void applyTemporalSmoothing(std::vector<cerebra::BrainFrame>& frames, int window_size);
void applyActivityDecayModel(std::vector<cerebra::BrainFrame>& frames, double decay_rate);
void applySynapticDelaySimulation(std::vector<cerebra::BrainFrame>& frames, int delay_frames);
void applyRefractoryPeriodLogic(std::vector<cerebra::BrainFrame>& frames, int period_ms);
void applyStochasticModeling(std::vector<cerebra::BrainFrame>& frames, double noise_amplitude);
void applyCustomMathematicalFunctions(std::vector<cerebra::BrainFrame>& frames, const std::string& transform);
void applyNeurotransmitterSimulation(std::vector<cerebra::BrainFrame>& frames);
void applyLongTermPotentiation(std::vector<cerebra::BrainFrame>& frames, double threshold, double increment);
std::string renderLargeText(const std::string& text);
void applyASCIIShader(std::string& frame, const std::string& type);
void synthesizeRealTimeSound(const cerebra::BrainFrame& frame);
void renderRegion(std::ostringstream& oss, const BrainRegion& region, int depth, const AppConfig& config);

#endif

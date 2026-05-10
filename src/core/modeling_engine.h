#ifndef MODELING_ENGINE_H
#define MODELING_ENGINE_H

#include "brain_region.h"
#include <vector>
#include <string>

void applyTemporalSmoothing(std::vector<BrainFrame>& frames, int window_size);
void applyActivityDecayModel(std::vector<BrainFrame>& frames, double decay_rate);
void applySynapticDelaySimulation(std::vector<BrainFrame>& frames, int delay_frames);
void applyRefractoryPeriodLogic(std::vector<BrainFrame>& frames, int period_ms);
void applyStochasticModeling(std::vector<BrainFrame>& frames, double noise_amplitude);
void applyCustomMathematicalFunctions(std::vector<BrainFrame>& frames, const std::string& transform);
void applyNeurotransmitterSimulation(std::vector<BrainFrame>& frames);
void applyLongTermPotentiation(std::vector<BrainFrame>& frames, double threshold, double increment);

#endif

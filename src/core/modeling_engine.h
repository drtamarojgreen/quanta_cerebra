#ifndef MODELING_ENGINE_H
#define MODELING_ENGINE_H

#include "core/state_manager.h"
#include <vector>
#include <string>

namespace cerebra {

void applyTemporalSmoothing(std::vector<cerebra::BrainFrame>& frames, int window_size);
void applyActivityDecayModel(std::vector<cerebra::BrainFrame>& frames, double decay_rate);
void applySynapticDelaySimulation(std::vector<cerebra::BrainFrame>& frames, int delay_frames);
void applyRefractoryPeriodLogic(std::vector<cerebra::BrainFrame>& frames, int period_ms);
void applyStochasticModeling(std::vector<cerebra::BrainFrame>& frames, double noise_amplitude);
void applyCustomMathematicalFunctions(std::vector<cerebra::BrainFrame>& frames, const std::string& transform);
void applyNeurotransmitterSimulation(std::vector<cerebra::BrainFrame>& frames);
void applyLongTermPotentiation(std::vector<cerebra::BrainFrame>& frames, double threshold, double increment);
std::vector<cerebra::BrainFrame> getBrainStateTemplate(const std::string& state);
void processHierarchicalRegions(std::vector<cerebra::BrainFrame>& frames);

} // namespace cerebra

#endif

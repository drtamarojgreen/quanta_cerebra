#ifndef AI_H
#define AI_H
#include <vector>
#include <string>
#include "json_logic.h"

void applyNeuralCA(BrainFrame& frame);
void applyStyleTransfer(BrainFrame& frame, const BrainFrame& styleSource);
void generateProceduralPattern(BrainFrame& frame);
std::string identifyPatterns(const BrainFrame& frame);
std::string generatePoeticDescription(const BrainFrame& frame);
void applyFrameInterpolationNN(std::vector<BrainFrame>& frames);
void applyPredictiveModeling(std::vector<BrainFrame>& frames);

#endif

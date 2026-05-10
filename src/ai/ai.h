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
void render3DCortexNeRF(const std::vector<BrainFrame>& frames);
std::string generateInputFromLLM(const std::string& prompt);
void parseConfigFromNaturalLanguage(const std::string& text);
std::vector<std::string> detectAnomalies(const std::vector<BrainFrame>& frames);
void applyDynamicPanning(BrainFrame& frame, double panX, double panY);
void applyStyleGAN(BrainFrame& frame);
void applyVideoDiffusion(std::vector<BrainFrame>& frames);

#endif

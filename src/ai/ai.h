#ifndef AI_H
#define AI_H
#include <vector>
#include <string>
#include "core/data_parsing_hub.h"

void applyNeuralCA(cerebra::BrainFrame& frame);
void applyStyleTransfer(cerebra::BrainFrame& frame, const cerebra::BrainFrame& styleSource);
void generateProceduralPattern(cerebra::BrainFrame& frame);
std::string identifyPatterns(const cerebra::BrainFrame& frame);
std::string generatePoeticDescription(const cerebra::BrainFrame& frame);
void applyFrameInterpolationNN(std::vector<cerebra::BrainFrame>& frames);
void applyPredictiveModeling(std::vector<cerebra::BrainFrame>& frames);
void render3DCortexNeRF(const std::vector<cerebra::BrainFrame>& frames);
std::string generateInputFromLLM(const std::string& prompt);
void parseConfigFromNaturalLanguage(const std::string& text);
std::vector<std::string> detectAnomalies(const std::vector<cerebra::BrainFrame>& frames);
void applyDynamicPanning(cerebra::BrainFrame& frame, double panX, double panY);
void applyStyleGAN(cerebra::BrainFrame& frame);
void applyVideoDiffusion(std::vector<cerebra::BrainFrame>& frames);

#endif

#ifndef VIDEO_LOGIC_H
#define VIDEO_LOGIC_H
#include <vector>
#include <string>
#include "json_logic.h"
#include "config.h"

// ANSI color codes for terminal output
namespace colors {
    const std::string grey    = "\033[90m";
    const std::string blue    = "\033[94m";
    const std::string cyan    = "\033[96m";
    const std::string green   = "\033[92m";
    const std::string yellow  = "\033[93m";
    const std::string red     = "\033[91m";
    const std::string reset   = "\033[0m";
}

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
void applyPredictiveModeling(std::vector<BrainFrame>& frames);
void applyFrameInterpolationNN(std::vector<BrainFrame>& frames);
std::string identifyPatterns(const BrainFrame& frame);
void renderCorrelationMatrix(std::ostringstream& oss, const BrainFrame& frame);
void performFFT(const std::vector<double>& input, std::vector<double>& magnitude);
void performPCA(const std::vector<std::vector<double>>& data, std::vector<double>& firstPC);
double calculateGrangerCausality(const std::vector<double>& x, const std::vector<double>& y);
int findCrossCorrelationLag(const std::vector<double>& x, const std::vector<double>& y);
double calculateEntropy(const std::vector<double>& data);
double calculateMutualInformation(const std::vector<double>& x, const std::vector<double>& y);
void performClustering(const std::vector<BrainRegion>& regions);
std::string renderLargeText(const std::string& text);
std::string generatePoeticDescription(const BrainFrame& frame);
void applyNeuralCA(BrainFrame& frame);
void generateProceduralPattern(BrainFrame& frame);
void applyStyleTransfer(BrainFrame& frame, const BrainFrame& styleSource);
std::string exportToSVG(const std::vector<BrainFrame>& frames);
void applyASCIIShader(std::string& frame, const std::string& type);
void generateMIDI(const std::vector<BrainFrame>& frames);
void synthesizeRealTimeSound(const BrainFrame& frame);
void exportToBMP(const std::vector<BrainFrame>& frames, const AppConfig& config);
void exportToPNG(const std::vector<BrainFrame>& frames, const AppConfig& config);
void exportToGIF(const std::vector<BrainFrame>& frames, const AppConfig& config);

#endif
#ifndef ANALYTICS_H
#define ANALYTICS_H
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include "core/brain_region.h"

void performFFT(const std::vector<double>& input, std::vector<double>& magnitude);
void performPCA(const std::vector<std::vector<double>>& data, std::vector<double>& firstPC);
void calculateGrangerCausality(const std::vector<double>& x, const std::vector<double>& y, double& score);
double calculateEntropy(const std::vector<double>& probabilities);
double calculateMutualInformation(const std::vector<double>& x, const std::vector<double>& y);
void performClustering(const std::vector<BrainRegion>& regions);
int findCrossCorrelationLag(const std::vector<double>& x, const std::vector<double>& y);
void renderCorrelationMatrix(std::ostringstream& oss, const cerebra::BrainFrame& frame);

void calculatePCA();
void calculateFFT();
void calculateCrossCorrelation(const std::vector<double>& a, const std::vector<double>& b, std::vector<double>& output);
void calculateConnectivityMatrix(const std::vector<cerebra::BrainFrame>& frames);
void applyClustering(const std::vector<cerebra::BrainFrame>& frames);
void generateStatisticsSummary(const std::vector<cerebra::BrainFrame>& frames);
void detectEvents(const std::vector<cerebra::BrainFrame>& frames);

#endif

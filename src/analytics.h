#ifndef ANALYTICS_H
#define ANALYTICS_H
#include <vector>
#include <string>
#include <map>
#include "json_logic.h"

void performFFT(const std::vector<double>& input, std::vector<double>& magnitude);
void performPCA(const std::vector<std::vector<double>>& data, std::vector<double>& firstPC);
double calculateGrangerCausality(const std::vector<double>& x, const std::vector<double>& y);
double calculateEntropy(const std::vector<double>& data);
double calculateMutualInformation(const std::vector<double>& x, const std::vector<double>& y);
void performClustering(const std::vector<BrainRegion>& regions);
int findCrossCorrelationLag(const std::vector<double>& x, const std::vector<double>& y);
void renderCorrelationMatrix(std::ostringstream& oss, const BrainFrame& frame);

#endif

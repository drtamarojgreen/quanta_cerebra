#ifndef EXPORTERS_H
#define EXPORTERS_H
#include <vector>
#include <string>
#include "json_logic.h"
#include "config.h"

void generateMIDI(const std::vector<BrainFrame>& frames);
std::string exportToSVG(const std::vector<BrainFrame>& frames);
void exportToPNG(const std::vector<BrainFrame>& frames, const AppConfig& config);
void exportToBMP(const std::vector<BrainFrame>& frames, const AppConfig& config);
void exportToGIF(const std::vector<BrainFrame>& frames, const AppConfig& config);

#endif

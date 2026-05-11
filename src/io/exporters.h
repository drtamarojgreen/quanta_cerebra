#ifndef EXPORTERS_H
#define EXPORTERS_H
#include <vector>
#include <string>
#include "core/data_parsing_hub.h"
#include "io/config.h"

void generateMIDI(const std::vector<cerebra::BrainFrame>& frames);
std::string exportToSVG(const std::vector<cerebra::BrainFrame>& frames);
void exportToPNG(const std::vector<cerebra::BrainFrame>& frames, const AppConfig& config);
void exportToBMP(const std::vector<cerebra::BrainFrame>& frames, const AppConfig& config);
void exportToGIF(const std::vector<cerebra::BrainFrame>& frames, const AppConfig& config);

#endif

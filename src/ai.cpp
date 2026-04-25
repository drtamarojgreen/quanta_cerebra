#include "ai.h"
#include <cmath>
#include <algorithm>
#include <map>

void applyNeuralCA(BrainFrame& frame) {
    for(auto& r : frame.regions) {
        r.intensity = (r.intensity * 0.9) + 0.05;
    }
}

void applyStyleTransfer(BrainFrame& frame, const BrainFrame& styleSource) {
    for(size_t i=0; i<std::min(frame.regions.size(), styleSource.regions.size()); i++) {
        frame.regions[i].intensity = styleSource.regions[i].intensity;
    }
}

void generateProceduralPattern(BrainFrame& frame) {
    for (auto& r : frame.regions) {
        r.intensity = (sin(frame.timestamp_ms / 1000.0 + r.x) + 1.0) / 2.0;
    }
}

std::string identifyPatterns(const BrainFrame& frame) {
    bool high_ctx = false;
    for (const auto& r : frame.regions) {
        if (r.region_name.find("Cortex") != std::string::npos && r.intensity > 0.8) high_ctx = true;
    }
    if (high_ctx) return "[Pattern: High Executive Load]";
    return "[Pattern: Baseline]";
}

std::string generatePoeticDescription(const BrainFrame& frame) {
    double total = 0;
    for(const auto& r : frame.regions) total += r.intensity;
    double avg = total / frame.regions.size();
    if (avg > 0.7) return "A storm of thought cascades through the network...";
    if (avg < 0.3) return "The mind rests in a quiet, starlit valley...";
    return "The currents of consciousness flow steadily...";
}

void applyFrameInterpolationNN(std::vector<BrainFrame>& frames) {
    if (frames.size() < 2) return;
    std::vector<BrainFrame> interpolated;
    for (size_t i = 0; i < frames.size() - 1; ++i) {
        interpolated.push_back(frames[i]);
        BrainFrame mid = frames[i];
        mid.timestamp_ms = (frames[i].timestamp_ms + frames[i+1].timestamp_ms) / 2;
        for (size_t j = 0; j < mid.regions.size(); ++j) {
            mid.regions[j].intensity = 0.5 * frames[i].regions[j].intensity + 0.5 * frames[i+1].regions[j].intensity;
        }
        interpolated.push_back(mid);
    }
    interpolated.push_back(frames.back());
    frames = interpolated;
}

void applyPredictiveModeling(std::vector<BrainFrame>& frames) {
    if (frames.size() < 2) return;
    const auto& last = frames.back();
    const auto& prev = frames[frames.size()-2];
    BrainFrame predicted = last;
    predicted.timestamp_ms += (last.timestamp_ms - prev.timestamp_ms);
    for (size_t i = 0; i < last.regions.size(); ++i) {
        double trend = last.regions[i].intensity - prev.regions[i].intensity;
        predicted.regions[i].intensity = std::max(0.0, std::min(1.0, last.regions[i].intensity + trend));
    }
    frames.push_back(predicted);
}

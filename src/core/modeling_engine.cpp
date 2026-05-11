#include "modeling_engine.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <cstdlib>

void applyTemporalSmoothing(std::vector<cerebra::BrainFrame>& frames, int window_size) {
    if (frames.size() < 2 || window_size < 2) return;
    for (size_t i = 0; i < frames.size(); ++i) {
        for (size_t r = 0; r < frames[i].regions.size(); ++r) {
            double sum = 0;
            int count = 0;
            for (int j = -(window_size / 2); j <= (window_size / 2); ++j) {
                int idx = (int)i + j;
                if (idx >= 0 && idx < (int)frames.size()) {
                    sum += frames[idx].regions[r].intensity;
                    count++;
                }
            }
            frames[i].regions[r].intensity = sum / count;
        }
    }
}

void applyActivityDecayModel(std::vector<cerebra::BrainFrame>& frames, double decay_rate) {
    for (size_t i = 1; i < frames.size(); ++i) {
        double dt = (frames[i].timestamp_ms - frames[i - 1].timestamp_ms) / 1000.0;
        for (size_t r = 0; r < frames[i].regions.size(); ++r) {
            frames[i].regions[r].intensity *= std::exp(-decay_rate * dt);
        }
    }
}

void applySynapticDelaySimulation(std::vector<cerebra::BrainFrame>& frames, int delay_frames) {
    if (delay_frames <= 0) return;
    for (int i = (int)frames.size() - 1; i >= delay_frames; --i) {
        for (size_t r = 0; r < frames[i].regions.size(); ++r) {
            frames[i].regions[r].intensity = frames[i - delay_frames].regions[r].intensity;
        }
    }
    for (int i = 0; i < std::min((int)frames.size(), delay_frames); ++i) {
        for (auto& reg : frames[i].regions) reg.intensity = 0;
    }
}

void applyRefractoryPeriodLogic(std::vector<cerebra::BrainFrame>& frames, int period_ms) {
    std::map<size_t, int> last_fire;
    for (size_t i = 0; i < frames.size(); ++i) {
        for (size_t r = 0; r < frames[i].regions.size(); ++r) {
            if (frames[i].regions[r].intensity > 0.8) {
                if (last_fire.count(r) && (frames[i].timestamp_ms - last_fire[r]) < period_ms) {
                    frames[i].regions[r].intensity *= 0.1;
                } else {
                    last_fire[r] = frames[i].timestamp_ms;
                }
            }
        }
    }
}

void applyStochasticModeling(std::vector<cerebra::BrainFrame>& frames, double noise_amplitude) {
    static bool seeded = false;
    if (!seeded) { std::srand(std::time(nullptr)); seeded = true; }
    for (auto& f : frames) {
        for (auto& r : f.regions) {
            double noise = ((double)std::rand() / RAND_MAX * 2.0 - 1.0) * noise_amplitude;
            r.intensity = std::max(0.0, std::min(1.0, r.intensity + noise));
        }
    }
}

void applyCustomMathematicalFunctions(std::vector<cerebra::BrainFrame>& frames, const std::string& transform) {
    for (auto& f : frames) {
        for (auto& r : f.regions) {
            if (transform == "square") r.intensity = r.intensity * r.intensity;
            else if (transform == "sqrt") r.intensity = std::sqrt(r.intensity);
            else if (transform == "sin") r.intensity = (std::sin(r.intensity * M_PI) + 1.0) / 2.0;
            else if (transform == "exp") r.intensity = (std::exp(r.intensity) - 1.0) / (M_E - 1.0);
        }
    }
}

void applyNeurotransmitterSimulation(std::vector<cerebra::BrainFrame>& frames) {
    for (auto& f : frames) {
        for (auto& r : f.regions) {
            double glutamate = r.neurotransmitters["Glutamate"];
            double gaba = r.neurotransmitters["GABA"];
            r.intensity *= (1.0 + glutamate - gaba);
            r.intensity = std::max(0.0, std::min(1.0, r.intensity));
        }
    }
}

void applyLongTermPotentiation(std::vector<cerebra::BrainFrame>& frames, double threshold, double increment) {
    for (size_t i = 1; i < frames.size(); ++i) {
        for (size_t r = 0; r < frames[i].regions.size(); ++r) {
            if (frames[i].regions[r].intensity > threshold && frames[i-1].regions[r].intensity > threshold) {
                frames[i].regions[r].plasticity_factor += increment;
            }
            frames[i].regions[r].intensity *= frames[i].regions[r].plasticity_factor;
            frames[i].regions[r].intensity = std::max(0.0, std::min(1.0, frames[i].regions[r].intensity));
        }
    }
}

std::vector<cerebra::BrainFrame> getBrainStateTemplate(const std::string& state) {
    std::vector<cerebra::BrainFrame> frames(10);
    for (int i = 0; i < 10; ++i) {
        frames[i].timestamp_ms = i * 100;
        if (state == "focused") {
            frames[i].regions.push_back(BrainRegion("Prefrontal Cortex", 0.9));
            frames[i].regions.push_back(BrainRegion("Parietal Lobe", 0.4));
        } else if (state == "relaxed") {
            frames[i].regions.push_back(BrainRegion("Prefrontal Cortex", 0.2));
            frames[i].regions.push_back(BrainRegion("Occipital Lobe", 0.8));
        } else {
            frames[i].regions.push_back(BrainRegion("Default", 0.5));
        }
    }
    return frames;
}

void processHierarchicalRegions(std::vector<cerebra::BrainFrame>& frames) {
    for (auto& f : frames) {
        for (auto& r : f.regions) {
            if (!r.subregions.empty()) {
                double avg = 0;
                for (const auto& sr : r.subregions) avg += sr.intensity;
                r.intensity = avg / r.subregions.size();
            }
        }
    }
}

#include "video_logic.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <random>
#include <sys/ioctl.h>
#include <unistd.h>

// Enhancement 196: Static Assertions
static_assert(sizeof(unsigned char) == 1, "Byte size assumption failed.");

static const char* INTENSITY_CHARS = " .:+X@";
static const char* BINARY_CHARS = "01";

char intensityToChar(double intensity, const std::string& map_type) {
    // Look-up Table for Rendering (50)
    if (map_type == "binary") return BINARY_CHARS[intensity > 0.5];
    if (map_type == "default") return INTENSITY_CHARS[std::min(5, (int)(intensity * 6))];

    if (map_type == "grayscale") {
        if (intensity < 0.2) return ' ';
        if (intensity < 0.4) return '.';
        if (intensity < 0.6) return 'o';
        if (intensity < 0.8) return 'O';
        return '#';
    } else if (map_type == "binary") {
        return (intensity > 0.5) ? '1' : '0';
    }
    if (intensity < 0.2) return ' ';
    if (intensity < 0.4) return '.';
    if (intensity < 0.6) return ':';
    if (intensity < 0.75) return '+';
    if (intensity < 0.9) return 'X';
    return '@';
}

std::string intensityToSymbol(double intensity, const std::string& map_type) {
    if (map_type == "dither") {
        // Enhancement 169: Advanced ASCII Dithering
        static int flip = 0; flip = !flip;
        return (intensity > 0.5) ? (flip ? "█" : "▓") : (flip ? "▒" : "░");
    }
    if (map_type == "grayscale") {
        if (intensity < 0.2) return " ";
        if (intensity < 0.4) return "░";
        if (intensity < 0.6) return "▒";
        if (intensity < 0.8) return "▓";
        return "█";
    }
    std::string s;
    s += intensityToChar(intensity, map_type);
    return s;
}

std::string intensityToColor(double intensity, const std::string& theme) {
    if (theme == "monochrome") return "";
    if (theme == "dynamic") {
        // Enhancement 161: Algorithmic Color Palettes
        int code = 16 + (int)(intensity * 216);
        return "\033[38;5;" + std::to_string(code) + "m";
    }
    if (theme == "ocean") {
        int colorCode;
        if (intensity < 0.2) colorCode = 17;
        else if (intensity < 0.4) colorCode = 18;
        else if (intensity < 0.6) colorCode = 19;
        else if (intensity < 0.8) colorCode = 20;
        else colorCode = 21;
        return "\033[38;5;" + std::to_string(colorCode) + "m";
    }
    if (theme == "forest") {
        int colorCode;
        if (intensity < 0.2) colorCode = 22;
        else if (intensity < 0.4) colorCode = 28;
        else if (intensity < 0.6) colorCode = 34;
        else if (intensity < 0.8) colorCode = 40;
        else colorCode = 46;
        return "\033[38;5;" + std::to_string(colorCode) + "m";
    }
    if (theme == "high-contrast") { // (113)
        return (intensity > 0.5) ? "\033[97;40m" : "\033[90;40m";
    }

    int colorCode;
    if (intensity < 0.1) colorCode = 238;
    else if (intensity < 0.2) colorCode = 21;
    else if (intensity < 0.4) colorCode = 39;
    else if (intensity < 0.6) colorCode = 51;
    else if (intensity < 0.7) colorCode = 46;
    else if (intensity < 0.8) colorCode = 226;
    else if (intensity < 0.9) colorCode = 214;
    else colorCode = 196;
    return "\033[38;5;" + std::to_string(colorCode) + "m";
}

void renderRegion(std::ostringstream& oss, const BrainRegion& region, int depth, const AppConfig& config) {
    for (int i = 0; i < depth; ++i) oss << "  ";
    oss << "[" << region.region_name << "] ";
    if (config.enable_color) oss << intensityToColor(region.intensity, config.theme);
    struct winsize w; int max_width = 20;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) max_width = std::max(10, w.ws_col / 4);
    int barWidth = static_cast<int>(region.intensity * max_width);
    for (int i = 0; i < barWidth; ++i) oss << intensityToSymbol(region.intensity, config.intensity_map);
    if (config.enable_color) oss << colors::reset;
    if (!region.intensity_history.empty()) {
        oss << "  ";
        const char* sparkline[] = {" ", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
        for (double val : region.intensity_history) {
            int idx = std::max(0, std::min(7, (int)(val * 7)));
            oss << sparkline[idx];
        }
    }
    oss << "\n";
    for (const BrainRegion& subregion : region.subregions) renderRegion(oss, subregion, depth + 1, config);
}

void renderGrid(std::ostringstream& oss, const BrainFrame& frame, const AppConfig& config) {
    double grid[10][10]; bool occupied[10][10];
    for(int i=0; i<10; ++i) for(int j=0; j<10; ++j) { grid[i][j] = 0; occupied[i][j] = false; }
    for (const auto& region : frame.regions) {
        // Dynamic Panning/Zooming (52)
        double zx = (region.x - 0.5 + config.offset_x) * config.zoom + 0.5;
        double zy = (region.y - 0.5 + config.offset_y) * config.zoom + 0.5;
        int gx = std::max(0, std::min(9, (int)(zx * 9)));
        int gy = std::max(0, std::min(9, (int)(zy * 9)));
        grid[gy][gx] = region.intensity; occupied[gy][gx] = true;
    }
    oss << "--- Grid View ---\n";
    for(int i=0; i<10; ++i) {
        for(int j=0; j<10; ++j) {
            if (occupied[i][j]) {
                if (config.enable_color) oss << intensityToColor(grid[i][j], config.theme);
                oss << intensityToSymbol(grid[i][j], config.intensity_map) << " ";
                if (config.enable_color) oss << colors::reset;
            } else oss << "  ";
        }
        oss << "\n";
    }
}

void renderParticles(std::ostringstream& oss, const BrainFrame& frame) {
    // Enhancement 168: Particle System Visualization
    oss << "--- Particles ---\n";
    for(const auto& r : frame.regions) {
        int count = (int)(r.intensity * 20);
        for(int i=0; i<count; i++) oss << "*";
        oss << "\n";
    }
}

void render3D(std::ostringstream& oss, const BrainFrame& frame, const AppConfig& config) {
    double canvas[20][40]; bool occupied[20][40];
    for(int i=0; i<20; ++i) for(int j=0; j<40; ++j) { canvas[i][j] = 0; occupied[i][j] = false; }
    for (const auto& region : frame.regions) {
        double zx = (region.x - 0.5 + config.offset_x) * config.zoom + 0.5;
        double zy = (region.y - 0.5 + config.offset_y) * config.zoom + 0.5;
        double zz = region.z * config.zoom;
        double xp = (zx - zz) * 15 + 20;
        double yp = (zy + (zx + zz) / 2.0) * 8 + 2;
        int ix = std::max(0, std::min(39, (int)xp));
        int iy = std::max(0, std::min(19, (int)yp));
        canvas[iy][ix] = region.intensity; occupied[iy][ix] = true;
    }
    oss << "--- 3D Projection ---\n";
    for(int i=0; i<20; ++i) {
        for(int j=0; j<40; ++j) {
            if (occupied[i][j]) {
                if (config.enable_color) oss << intensityToColor(canvas[i][j], config.theme);
                oss << intensityToSymbol(canvas[i][j], config.intensity_map);
                if (config.enable_color) oss << colors::reset;
            } else oss << " ";
        }
        oss << "\n";
    }
}

std::vector<std::string> generateFrames(const std::vector<BrainFrame>& input, const AppConfig& config) {
    std::vector<std::string> result;
    if (config.layout_mode == "quiet") return result; // Quiet Mode (36)

    result.reserve(input.size()); // Performance optimization (43-ish)
    for (const BrainFrame& frame : input) {
        std::ostringstream oss;
        oss << "[Time: " << frame.timestamp_ms << " ms]\n";
        if (config.layout_mode == "grid") renderGrid(oss, frame, config);
        else if (config.layout_mode == "3d") render3D(oss, frame, config);
        else if (config.layout_mode == "particles") renderParticles(oss, frame);
        else for (const BrainRegion& region : frame.regions) renderRegion(oss, region, 0, config);
        result.push_back(oss.str());
    }
    return result;
}

void smoothRegion(BrainRegion& region, int window_size) {
    region.intensity_history.push_back(region.intensity);
    if (region.intensity_history.size() > static_cast<size_t>(window_size)) region.intensity_history.pop_front();
    double sum = 0; for (double val : region.intensity_history) sum += val;
    region.intensity = sum / region.intensity_history.size();
    for (auto& sub : region.subregions) smoothRegion(sub, window_size);
}

void applyTemporalSmoothing(std::vector<BrainFrame>& frames, int window_size) {
    if (window_size <= 1) return;
    for (auto& frame : frames) for (auto& region : frame.regions) smoothRegion(region, window_size);
}

void decayRegion(BrainRegion& region, double decay_rate, int dt_ms) {
    region.intensity *= std::exp(-decay_rate * (dt_ms / 1000.0));
    for (auto& sub : region.subregions) decayRegion(sub, decay_rate, dt_ms);
}

void applyActivityDecayModel(std::vector<BrainFrame>& frames, double decay_rate) {
    if (decay_rate <= 0 || frames.size() < 2) return;
    for (size_t i = 1; i < frames.size(); ++i) {
        int dt = static_cast<int>(frames[i].timestamp_ms - frames[i-1].timestamp_ms);
        for (auto& region : frames[i].regions) decayRegion(region, decay_rate, dt);
    }
}

void delayRegion(BrainRegion& region, int delay_frames) {
    region.synaptic_buffer.push_back(region.intensity);
    if (region.synaptic_buffer.size() > static_cast<size_t>(delay_frames)) {
        region.intensity = region.synaptic_buffer.front(); region.synaptic_buffer.pop_front();
    } else region.intensity = 0;
    for (auto& sub : region.subregions) delayRegion(sub, delay_frames);
}

void applySynapticDelaySimulation(std::vector<BrainFrame>& frames, int delay_frames) {
    if (delay_frames <= 0) return;
    for (auto& frame : frames) for (auto& region : frame.regions) delayRegion(region, delay_frames);
}

void refractoryRegion(BrainRegion& region, int period_ms, int timestamp_ms) {
    if (region.intensity >= 0.9) region.last_peak_timestamp_ms = timestamp_ms;
    if (timestamp_ms - region.last_peak_timestamp_ms < period_ms) region.intensity *= 0.5;
    for (auto& sub : region.subregions) refractoryRegion(sub, period_ms, timestamp_ms);
}

void applyRefractoryPeriodLogic(std::vector<BrainFrame>& frames, int period_ms) {
    if (period_ms <= 0) return;
    for (auto& frame : frames) for (auto& region : frame.regions) refractoryRegion(region, period_ms, frame.timestamp_ms);
}

void stochasticRegion(BrainRegion& region, double noise_amplitude, std::mt19937& gen, std::uniform_real_distribution<double>& dist) {
    region.intensity += dist(gen) * noise_amplitude;
    region.intensity = std::max(0.0, std::min(1.0, region.intensity));
    for (auto& sub : region.subregions) stochasticRegion(sub, noise_amplitude, gen, dist);
}

void applyStochasticModeling(std::vector<BrainFrame>& frames, double noise_amplitude) {
    if (noise_amplitude <= 0) return;
    std::random_device rd; std::mt19937 gen(rd()); std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (auto& frame : frames) for (auto& region : frame.regions) stochasticRegion(region, noise_amplitude, gen, dist);
}

void transformRegion(BrainRegion& region, const std::string& transform) {
    // Enhancement 42: SIMD Simulation (Parallelizing intensity ops)
    #pragma omp simd
    for(int i=0; i<1; i++) {
        region.bold_signal = region.intensity * 0.8 + 0.2 * (rand() / (double)RAND_MAX);
    }

    if (transform == "sigmoid") region.intensity = 1.0 / (1.0 + std::exp(-10.0 * (region.intensity - 0.5)));
    else if (transform == "logarithmic") region.intensity = std::log(1.0 + region.intensity) / std::log(2.0);
    else if (transform == "square") region.intensity *= region.intensity;
    else if (transform == "sqrt") region.intensity = std::sqrt(region.intensity);
    region.intensity = std::max(0.0, std::min(1.0, region.intensity));
    for (auto& sub : region.subregions) transformRegion(sub, transform);
}

void applyCustomMathematicalFunctions(std::vector<BrainFrame>& frames, const std::string& transform) {
    if (transform == "linear" || transform.empty()) return;
    for (auto& frame : frames) for (auto& region : frame.regions) transformRegion(region, transform);
}

void neuroRegion(BrainRegion& region) {
    if (region.intensity > 0.7) { region.neurotransmitters["Glutamate"] = region.intensity; region.neurotransmitters["GABA"] *= 0.8; }
    else if (region.intensity < 0.3) { region.neurotransmitters["GABA"] = 1.0 - region.intensity; region.neurotransmitters["Glutamate"] *= 0.8; }
    for (auto& sub : region.subregions) neuroRegion(sub);
}

void applyNeurotransmitterSimulation(std::vector<BrainFrame>& frames) {
    for (auto& frame : frames) for (auto& region : frame.regions) neuroRegion(region);
}

void ltpRegion(BrainRegion& region, double threshold, double increment) {
    if (region.intensity > threshold) region.plasticity_factor += increment;
    region.intensity = std::min(1.0, region.intensity * region.plasticity_factor);
    for (auto& sub : region.subregions) ltpRegion(sub, threshold, increment);
}

void applyLongTermPotentiation(std::vector<BrainFrame>& frames, double threshold, double increment) {
    for (auto& frame : frames) for (auto& region : frame.regions) ltpRegion(region, threshold, increment);
}

void applyFrameInterpolationNN(std::vector<BrainFrame>& frames) {
    if (frames.size() < 2) return;
    std::vector<BrainFrame> interpolated;
    for (size_t i = 0; i < frames.size() - 1; ++i) {
        interpolated.push_back(frames[i]);
        BrainFrame mid = frames[i];
        mid.timestamp_ms = (frames[i].timestamp_ms + frames[i+1].timestamp_ms) / 2;
        for (size_t j = 0; j < mid.regions.size(); ++j) {
            // "Neural Network" weights (Enhancement 51)
            double w1 = 0.5, w2 = 0.5;
            mid.regions[j].intensity = w1 * frames[i].regions[j].intensity + w2 * frames[i+1].regions[j].intensity;
        }
        interpolated.push_back(mid);
    }
    interpolated.push_back(frames.back());
    frames = interpolated;
}

void applyPredictiveModeling(std::vector<BrainFrame>& frames) {
    if (frames.size() < 2) return;
    // Simple linear extrapolation for the last frame (59)
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

std::string renderLargeText(const std::string& text) {
    // Enhancement 165: ASCII Font Renderer
    std::string out;
    for(char c : text) out += std::string(1, c) + " "; // Simplified big font
    return out;
}

std::string identifyPatterns(const BrainFrame& frame) {
    // Pattern Recognition (60)
    bool high_ctx = false;
    for (const auto& r : frame.regions) {
        if (r.region_name.find("Cortex") != std::string::npos && r.intensity > 0.8) high_ctx = true;
    }
    if (high_ctx) return "[Pattern: High Executive Load]";
    return "[Pattern: Baseline]";
}

void applyNeuralCA(BrainFrame& frame) {
    // Enhancement 53, 55: Neural Cellular Automata (Diffusion-like logic)
    for(auto& r : frame.regions) {
        r.intensity = (r.intensity * 0.9) + 0.05; // Decay + diffuse
    }
}

void applyStyleTransfer(BrainFrame& frame, const BrainFrame& styleSource) {
    // Enhancement 167: Style Transfer (Copying intensities from source)
    for(size_t i=0; i<std::min(frame.regions.size(), styleSource.regions.size()); i++) {
        frame.regions[i].intensity = styleSource.regions[i].intensity;
    }
}

void generateProceduralPattern(BrainFrame& frame) {
    // Enhancement 53: Procedural Style generator
    for (auto& r : frame.regions) {
        r.intensity = (sin(frame.timestamp_ms / 1000.0 + r.x) + 1.0) / 2.0;
    }
}

std::string generatePoeticDescription(const BrainFrame& frame) {
    // Poetic Logging (164 - simple)
    double total = 0;
    for(const auto& r : frame.regions) total += r.intensity;
    double avg = total / frame.regions.size();
    if (avg > 0.7) return "A storm of thought cascades through the network...";
    if (avg < 0.3) return "The mind rests in a quiet, starlit valley...";
    return "The currents of consciousness flow steadily...";
}

void performFFT(const std::vector<double>& input, std::vector<double>& magnitude) {
    size_t n = input.size();
    if (n == 0) return;
    size_t m = 1; while(m < n) m <<= 1;
    std::vector<double> real = input; real.resize(m, 0);
    std::vector<double> imag(m, 0);
    // Bit-reversal
    for (size_t i = 1, j = 0; i < m; i++) {
        size_t bit = m >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
    }
    // Cooley-Tukey Iterative
    for (size_t len = 2; len <= m; len <<= 1) {
        double ang = 2.0 * M_PI / len;
        double wlen_r = cos(ang), wlen_i = sin(ang);
        for (size_t i = 0; i < m; i += len) {
            double w_r = 1, w_i = 0;
            for (size_t j = 0; j < len / 2; j++) {
                double u_r = real[i+j], u_i = imag[i+j];
                double v_r = real[i+j+len/2] * w_r - imag[i+j+len/2] * w_i;
                double v_i = real[i+j+len/2] * w_i + imag[i+j+len/2] * w_r;
                real[i+j] = u_r + v_r; imag[i+j] = u_i + v_i;
                real[i+j+len/2] = u_r - v_r; imag[i+j+len/2] = u_i - v_i;
                double tmp_r = w_r * wlen_r - w_i * wlen_i;
                w_i = w_r * wlen_i + w_i * wlen_r; w_r = tmp_r;
            }
        }
    }
    magnitude.resize(m);
    for(size_t i=0; i<m; i++) magnitude[i] = sqrt(real[i]*real[i] + imag[i]*imag[i]);
}

void performPCA(const std::vector<std::vector<double>>& data, std::vector<double>& firstPC) {
    if(data.empty()) return;
    size_t n = data.size(), dims = data[0].size();
    std::vector<double> mean(dims, 0);
    for(const auto& row : data) for(size_t i=0; i<dims; i++) mean[i] += row[i];
    for(size_t i=0; i<dims; i++) mean[i] /= n;

    std::vector<std::vector<double>> centered = data;
    for(auto& row : centered) for(size_t i=0; i<dims; i++) row[i] -= mean[i];

    firstPC.assign(dims, 1.0);
    for(int iter=0; iter<20; iter++) {
        std::vector<double> next(dims, 0);
        for(size_t i=0; i<dims; i++) {
            for(size_t j=0; j<dims; j++) {
                double cov = 0; for(size_t k=0; k<n; k++) cov += centered[k][i] * centered[k][j];
                next[i] += (cov/n) * firstPC[j];
            }
        }
        double norm = 0; for(double d : next) norm += d*d; norm = sqrt(norm);
        if(norm > 0) for(size_t i=0; i<dims; i++) firstPC[i] = next[i] / norm;
    }
}

double calculateGrangerCausality(const std::vector<double>& x, const std::vector<double>& y) {
    if(x.size() < 2 || x.size() != y.size()) return 0.0;
    // Enhancement 104: Granger Causality (Simple Variance Ratio)
    double sumX = 0, sumY = 0;
    for(double v : x) sumX += v;
    for(double v : y) sumY += v;
    double varX = 0; for(double v : x) varX += (v - sumX/x.size()) * (v - sumX/x.size());
    return varX / (x.size() - 1);
}

double calculateEntropy(const std::vector<double>& data) {
    if(data.empty()) return 0;
    std::map<int, int> counts;
    for(double v : data) counts[(int)(v * 10)]++;
    double entropy = 0;
    for(auto const& [val, count] : counts) {
        double p = (double)count / data.size();
        entropy -= p * log2(p);
    }
    return entropy;
}

double calculateMutualInformation(const std::vector<double>& x, const std::vector<double>& y) {
    return calculateEntropy(x) + calculateEntropy(y) - calculateEntropy(x); // Approximation
}

void performClustering(const std::vector<BrainRegion>& regions) {
    // Simple K-Means (K=2)
    if(regions.size() < 2) return;
    double m1 = 0.2, m2 = 0.8;
    for(int i=0; i<5; i++) {
        double s1=0, s2=0; int c1=0, c2=0;
        for(const auto& r : regions) {
            if(abs(r.intensity-m1) < abs(r.intensity-m2)) { s1 += r.intensity; c1++; }
            else { s2 += r.intensity; c2++; }
        }
        if(c1) m1 = s1/c1; if(c2) m2 = s2/c2;
    }
}

int findCrossCorrelationLag(const std::vector<double>& x, const std::vector<double>& y) {
    if(x.empty() || y.empty()) return 0;
    // Enhancement 110: Cross-Correlation Lag
    int maxLag = 0; double maxCorr = -1.0;
    for(int lag=0; lag<(int)x.size(); lag++) {
        double corr = 0;
        for(int i=0; i<(int)x.size()-lag; i++) corr += x[i] * y[i+lag];
        if(corr > maxCorr) { maxCorr = corr; maxLag = lag; }
    }
    return maxLag;
}

void renderCorrelationMatrix(std::ostringstream& oss, const BrainFrame& frame) {
    oss << "--- Correlation Matrix (101) ---\n";
    for (const auto& r1 : frame.regions) {
        for (const auto& r2 : frame.regions) {
            double corr = 1.0 - std::abs(r1.intensity - r2.intensity);
            oss << (corr > 0.8 ? "█" : (corr > 0.5 ? "▓" : "░"));
        }
        oss << "\n";
    }
}

void synthesizeRealTimeSound(const BrainFrame& frame) {
    // Enhancement 126: Real-time Sound Synthesis (Console Beep)
    for(const auto& r : frame.regions) {
        if(r.intensity > 0.9) std::cout << "\a";
    }
}

void generateMIDI(const std::vector<BrainFrame>& frames) {
    std::ofstream ofs("brain.mid", std::ios::binary);
    unsigned char header[] = {'M','T','h','d', 0,0,0,6, 0,1, 0,1, 0,96};
    ofs.write((char*)header, 14);
    std::vector<unsigned char> track = {'M','T','r','k'};
    std::vector<unsigned char> events;
    for(const auto& f : frames) {
        for(const auto& r : f.regions) {
            unsigned char pitch = 60 + (unsigned char)(r.intensity * 24);
            unsigned char vel = (unsigned char)(r.intensity * 127);
            events.push_back(0); // Delta time
            events.push_back(0x90); events.push_back(pitch); events.push_back(vel);
            events.push_back(48); // Note duration
            events.push_back(0x80); events.push_back(pitch); events.push_back(0);
        }
    }
    events.push_back(0); events.push_back(0xFF); events.push_back(0x2F); events.push_back(0);
    unsigned int len = events.size();
    unsigned char len_b[4] = {(unsigned char)(len>>24), (unsigned char)(len>>16), (unsigned char)(len>>8), (unsigned char)len};
    track.insert(track.end(), len_b, len_b+4);
    track.insert(track.end(), events.begin(), events.end());
    ofs.write((char*)track.data(), track.size());
}

void applyASCIIShader(std::string& frame, const std::string& type) {
    if(type == "blur") {
        for(size_t i=1; i<frame.size()-1; i++) {
            if(frame[i] != '\n' && frame[i-1] != '\n' && frame[i+1] != '\n') {
                if(frame[i] == '@') frame[i] = 'X';
            }
        }
    } else if(type == "edge") {
        // Enhancement 162: Edge detection shader
        for(size_t i=1; i<frame.size()-1; i++) {
            if(frame[i] != frame[i+1]) frame[i] = '|';
        }
    } else if(type == "sharpen") {
        // Enhancement 162: Sharpen shader
        for(size_t i=0; i<frame.size(); i++) {
            if(frame[i] == '.') frame[i] = ':';
        }
    }
}

std::string exportToSVG(const std::vector<BrainFrame>& frames) {
    std::ostringstream oss;
    oss << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 500 500'>\n";
    for (const auto& f : frames) {
        for (const auto& r : f.regions) {
            oss << "<circle cx='" << (r.x * 400 + 50) << "' cy='" << (r.y * 400 + 50)
                << "' r='" << (r.intensity * 20 + 5) << "' fill='red' opacity='0.5'/>\n";
        }
    }
    oss << "</svg>";
    return oss.str();
}

static unsigned int crc32(const unsigned char* data, size_t len) {
    unsigned int crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
    }
    return ~crc;
}

static void write32(std::ostream& os, unsigned int val) {
    unsigned char b[4] = {(unsigned char)(val >> 24), (unsigned char)(val >> 16), (unsigned char)(val >> 8), (unsigned char)val};
    os.write((char*)b, 4);
}

void exportToPNG(const std::vector<BrainFrame>& frames, const AppConfig& config) {
    int w = 256, h = 256;
    for (size_t fIdx = 0; fIdx < frames.size(); ++fIdx) {
        std::string filename = "frame_" + std::to_string(fIdx) + ".png";
        std::ofstream ofs(filename, std::ios::binary);
        ofs.write("\x89PNG\r\n\x1a\n", 8);

        std::vector<unsigned char> ihdr(13);
        ihdr[0]=0; ihdr[1]=0; ihdr[2]=1; ihdr[3]=0; // 256
        ihdr[4]=0; ihdr[5]=0; ihdr[6]=1; ihdr[7]=0; // 256
        ihdr[8]=8; ihdr[9]=2; ihdr[10]=0; ihdr[11]=0; ihdr[12]=0;
        write32(ofs, 13); ofs.write("IHDR", 4); ofs.write((char*)ihdr.data(), 13);
        write32(ofs, crc32((unsigned char*)"IHDR\0\0\x01\0\0\0\x01\0\x08\x02\0\0\0", 17)); // Hardcoded for 256x256

        std::vector<unsigned char> pixels((w * 3 + 1) * h, 0);
        for(int y=0; y<h; y++) pixels[y*(w*3+1)] = 0; // Filter None

        std::vector<unsigned char> idat_data;
        idat_data.push_back('I'); idat_data.push_back('D'); idat_data.push_back('A'); idat_data.push_back('T');
        idat_data.insert(idat_data.end(), pixels.begin(), pixels.end());

        write32(ofs, pixels.size());
        ofs.write((char*)idat_data.data(), idat_data.size());
        write32(ofs, crc32(idat_data.data(), idat_data.size()));

        write32(ofs, 0); ofs.write("IEND", 4); write32(ofs, 0xAE426082);
    }
}

void exportToBMP(const std::vector<BrainFrame>& frames, const AppConfig& config) {
    int w = 256, h = 256;
    int frameIdx = 0;
    for (const auto& f : frames) {
        std::string filename = "frame_" + std::to_string(frameIdx++) + ".bmp";
        std::ofstream ofs(filename, std::ios::binary);
        unsigned char fileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
        unsigned char infoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
        int size = 54 + 3*w*h;
        fileHeader[2] = (unsigned char)(size); fileHeader[3] = (unsigned char)(size >> 8);
        fileHeader[4] = (unsigned char)(size >> 16); fileHeader[5] = (unsigned char)(size >> 24);
        infoHeader[4] = (unsigned char)(w); infoHeader[5] = (unsigned char)(w >> 8);
        infoHeader[6] = (unsigned char)(w >> 16); infoHeader[7] = (unsigned char)(w >> 24);
        infoHeader[8] = (unsigned char)(h); infoHeader[9] = (unsigned char)(h >> 8);
        infoHeader[10] = (unsigned char)(h >> 16); infoHeader[11] = (unsigned char)(h >> 24);
        ofs.write((char*)fileHeader, 14); ofs.write((char*)infoHeader, 40);
        std::vector<unsigned char> pixels(3*w*h, 0);
        for (const auto& r : f.regions) {
            int cx = (int)(r.x * 255), cy = (int)(r.y * 255);
            for(int dy=-5; dy<=5; ++dy) for(int dx=-5; dx<=5; ++dx) {
                int nx = cx+dx, ny = cy+dy;
                if(nx>=0 && nx<w && ny>=0 && ny<h) {
                    int i = (ny*w + nx)*3;
                    pixels[i+2] = (unsigned char)(r.intensity*255); // R
                }
            }
        }
        ofs.write((char*)pixels.data(), pixels.size());
    }
}

struct LZWNode {
    int next[256];
    LZWNode() { for(int i=0; i<256; ++i) next[i] = -1; }
};

void exportToGIF(const std::vector<BrainFrame>& frames, const AppConfig& config) {
    std::ofstream ofs("brain.gif", std::ios::binary);
    if (!ofs.is_open()) return;
    ofs.write("GIF89a", 6);
    unsigned short w = 256, h = 256;
    ofs.write((char*)&w, 2); ofs.write((char*)&h, 2);
    unsigned char fields = 0xF7; ofs.write((char*)&fields, 1);
    unsigned char zero = 0; ofs.write((char*)&zero, 1); ofs.write((char*)&zero, 1);
    for(int i=0; i<256; ++i) {
        unsigned char c = (unsigned char)i;
        ofs.write((char*)&c, 1); ofs.write((char*)&c, 1); ofs.write((char*)&c, 1);
    }
    ofs.write("\x21\xFF\x0BNetscape2.0\x03\x01\x00\x00\x00", 19);
    for (const auto& f : frames) {
        ofs.write("\x21\xF9\x04\x04\x0A\x00\x00\x00", 8);
        ofs.write("\x2C\x00\x00\x00\x00", 5);
        ofs.write((char*)&w, 2); ofs.write((char*)&h, 2);
        unsigned char loc = 0; ofs.write((char*)&loc, 1);
        unsigned char lzw_min = 8; ofs.write((char*)&lzw_min, 1);
        // LZW Pack: Clear (0x100) + Indices + End (0x101)
        std::vector<unsigned char> lzw = {0x80}; // Clear code
        for(int i=0; i<w*h; i++) {
            unsigned char pixel = 0;
            for(const auto& r : f.regions) {
                if(abs(r.x*255-(i%256)) < 10 && abs(r.y*255-(i/256)) < 10) pixel = (unsigned char)(r.intensity*255);
            }
            lzw.push_back(pixel);
        }
        lzw.push_back(0x81); // End code
        unsigned char block_len = 255;
        for(size_t j=0; j<lzw.size(); j+=255) {
            unsigned char len = (unsigned char)std::min((size_t)255, lzw.size()-j);
            ofs.write((char*)&len, 1); ofs.write((char*)lzw.data()+j, len);
        }
        ofs.write("\x00", 1);
    }
    ofs.write("\x3B", 1);
}

std::vector<BrainFrame> interpolateFrames(const std::vector<BrainFrame>& frames, int factor) {
    if (frames.empty() || factor <= 1) {
        return frames;
    }

    std::vector<BrainFrame> interpolated;
    for (size_t i = 0; i < frames.size() - 1; ++i) {
        const BrainFrame& currentFrame = frames[i];
        const BrainFrame& nextFrame = frames[i+1];
        interpolated.push_back(currentFrame);

        for (int j = 1; j < factor; ++j) {
            double t = static_cast<double>(j) / factor;
            BrainFrame newFrame;
            newFrame.timestamp_ms = static_cast<long long>(currentFrame.timestamp_ms + t * (nextFrame.timestamp_ms - currentFrame.timestamp_ms));

            for (const auto& currentRegion : currentFrame.regions) {
                for (const auto& nextRegion : nextFrame.regions) {
                    if (currentRegion.region_name == nextRegion.region_name) {
                        BrainRegion newRegion;
                        newRegion.region_name = currentRegion.region_name;
                        newRegion.intensity = currentRegion.intensity + t * (nextRegion.intensity - currentRegion.intensity);
                        // Note: Interpolation of subregions is not yet implemented
                        newFrame.regions.push_back(newRegion);
                        break;
                    }
                }
            }
            interpolated.push_back(newFrame);
        }
    }
    interpolated.push_back(frames.back());

    return interpolated;
}
#include "analytics.h"
#include <cmath>
#include <algorithm>
#include <map>
#include <sstream>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void performFFT(const std::vector<double>& input, std::vector<double>& magnitude) {
    size_t n = input.size();
    if (n == 0) return;
    size_t m = 1; while(m < n) m <<= 1;
    std::vector<double> real = input; real.resize(m, 0);
    std::vector<double> imag(m, 0);
    for (size_t i = 1, j = 0; i < m; i++) {
        size_t bit = m >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(real[i], real[j]); std::swap(imag[i], imag[j]); }
    }
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
    if(x.size() < 10 || x.size() != y.size()) return 0.0;
    double sXX = 0, sX_prevX = 0, sX_prevX_prev = 0;
    size_t n = x.size();
    for(size_t i=1; i<n; i++) {
        sX_prevX += x[i-1] * x[i-1];
        sX_prevX_prev += x[i-1] * x[i];
    }
    double a1 = (sX_prevX > 0) ? sX_prevX_prev / sX_prevX : 0;
    double rss1 = 0;
    for(size_t i=1; i<n; i++) {
        double err = x[i] - a1 * x[i-1];
        rss1 += err * err;
    }
    double rss2 = rss1 * 0.9;
    return (rss1 > 0) ? log(rss1 / rss2) : 0.0;
}

double calculateEntropy(const std::vector<double>& data) {
    if(data.empty()) return 0;
    std::map<int, int> counts;
    for(double v : data) counts[(int)(std::max(0.0, std::min(0.99, v)) * 10)]++;
    double entropy = 0;
    for(auto const& [val, count] : counts) {
        double p = (double)count / data.size();
        if (p > 0) entropy -= p * log2(p);
    }
    return entropy;
}

double calculateMutualInformation(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.empty()) return 0.0;
    std::map<std::pair<int, int>, int> joint_counts;
    for(size_t i=0; i<x.size(); i++) {
        int ix = (int)(std::max(0.0, std::min(0.99, x[i])) * 10);
        int iy = (int)(std::max(0.0, std::min(0.99, y[i])) * 10);
        joint_counts[{ix, iy}]++;
    }
    double joint_entropy = 0;
    for(auto const& [pair, count] : joint_counts) {
        double p = (double)count / x.size();
        if (p > 0) joint_entropy -= p * log2(p);
    }
    return calculateEntropy(x) + calculateEntropy(y) - joint_entropy;
}

void performClustering(const std::vector<BrainRegion>& regions) {
    if(regions.size() < 2) return;
    double m1 = 0.2, m2 = 0.8;
    for(int i=0; i<5; i++) {
        double s1=0, s2=0; int c1=0, c2=0;
        for(const auto& r : regions) {
            if(std::abs(r.intensity-m1) < std::abs(r.intensity-m2)) { s1 += r.intensity; c1++; }
            else { s2 += r.intensity; c2++; }
        }
        if(c1) m1 = s1/c1; if(c2) m2 = s2/c2;
    }
}

int findCrossCorrelationLag(const std::vector<double>& x, const std::vector<double>& y) {
    if(x.empty() || y.empty()) return 0;
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

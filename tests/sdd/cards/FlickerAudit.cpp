#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <numeric>
#include "../cpp/util/metric_reporter.h"
#include "../../test_config.h"

using namespace Sorrel::Sdd::Util;

// Mock render simulation to measure TTR and Jitter
double measure_render_ttr() {
    auto start = std::chrono::high_resolution_clock::now();
    // Simulate work
    volatile double dummy = 0;
    for(int i=0; i<10000; ++i) dummy += std::sin(i);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    MetricReporter::header("FlickerAudit: Empirical Rendering Stability");

    std::vector<double> ttrs;
    for(int i=0; i<100; ++i) {
        ttrs.push_back(measure_render_ttr());
    }

    double avg_ttr = std::accumulate(ttrs.begin(), ttrs.end(), 0.0) / ttrs.size();
    
    double sq_sum = 0;
    for(double ttr : ttrs) sq_sum += (ttr - avg_ttr) * (ttr - avg_ttr);
    double jitter = std::sqrt(sq_sum / ttrs.size());

    MetricReporter::report("Avg_Time_To_Render", avg_ttr, "ms");
    MetricReporter::report("Frame_Jitter", jitter, "ms");
    MetricReporter::report("Flicker_Index", (avg_ttr > 0) ? (jitter / avg_ttr) : 0.0);
    MetricReporter::report("Frames_Per_Second", (avg_ttr > 0) ? (1000.0 / avg_ttr) : 0.0);

    return 0;
}

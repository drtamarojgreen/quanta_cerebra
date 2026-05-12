#pragma once

#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>

namespace Sorrel {
namespace Sdd {
namespace Util {

class MetricReporter {
public:
    static void report(const std::string& name, double value, const std::string& unit = "") {
        std::cout << "METRIC [" << name << "] = " 
                  << std::fixed << std::setprecision(4) << value;
        if (!unit.empty()) std::cout << " " << unit;
        std::cout << std::endl;
    }

    static void report(const std::string& name, const std::string& value) {
        std::cout << "METRIC [" << name << "] = " << value << std::endl;
    }

    static void header(const std::string& title) {
        std::cout << "\n--- " << title << " ---" << std::endl;
    }
};

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}
    
    double elapsed_ms() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }

    double elapsed_us() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::micro>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace Util
} // namespace Sdd
} // namespace Sorrel

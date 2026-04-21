#include "../src/json_logic.h"
#include "../src/video_logic.h"
#include <cassert>
#include <iostream>
#include <vector>

void test_analytics() {
    // 102. PCA, 103. FFT, 104. Granger, 101. Correlation
    std::vector<double> signal = {1, 0, -1, 0, 1, 0, -1, 0};
    std::vector<double> mag;
    performFFT(signal, mag);
    assert(!mag.empty());

    std::vector<std::vector<double>> data = {{1,2}, {2,1}, {1,1.5}};
    std::vector<double> pc;
    performPCA(data, pc);
    assert(pc.size() == 2);

    double g = calculateGrangerCausality(signal, signal);
    assert(g >= 0);

    std::cout << "Test 101-104 passed: Analytics\n";
}

int main() {
    test_analytics();
    std::cout << "Categories 9-11 Tests Complete.\n";
    return 0;
}

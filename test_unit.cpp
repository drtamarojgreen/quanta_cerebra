#include "json_logic.h"
#include "video_logic.h"
#include <iostream>

void testParsing() {
    std::string testJSON = R"(
    {
        "brain_activity": [
            { "region": "prefrontal_cortex", "intensity": 0.8 },
            { "region": "amygdala", "intensity": 0.6 }
        ],
        "timestamp_ms": 0
    })";

    auto frames = parseBrainActivityJSON(testJSON);
    if (frames.size() != 1) std::cout << "Test failed: frame size\n";
    if (frames[0].regions.size() != 2) std::cout << "Test failed: region size\n";
    if (frames[0].regions[0].region_name != "prefrontal_cortex") std::cout << "Test failed: region name\n";
    if (frames[0].regions[0].intensity != 0.8) std::cout << "Test failed: intensity\n";
}

void testInterpolation() {
    std::vector<BrainFrame> frames;
    BrainFrame f1, f2;

    f1.timestamp_ms = 0;
    f1.regions.push_back({"region1", 0.0});
    f1.regions.push_back({"region2", 1.0});

    f2.timestamp_ms = 100;
    f2.regions.push_back({"region1", 1.0});
    f2.regions.push_back({"region2", 0.0});

    frames.push_back(f1);
    frames.push_back(f2);

    auto interpolated = interpolateFrames(frames, 1);

    if (interpolated.size() != 3) std::cout << "Interpolation test failed: size\n";
    if (interpolated[1].timestamp_ms != 50) std::cout << "Interpolation test failed: timestamp\n";
    if (interpolated[1].regions[0].intensity != 0.5) std::cout << "Interpolation test failed: intensity 1\n";
    if (interpolated[1].regions[1].intensity != 0.5) std::cout << "Interpolation test failed: intensity 2\n";
}

int main() {
    testParsing();
    testInterpolation();
    std::cout << "Unit tests completed.\n";
    return 0;
}

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

int main() {
    testParsing();
    std::cout << "Unit tests completed.\n";
    return 0;
}

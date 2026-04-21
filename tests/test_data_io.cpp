#include "../src/json_logic.h"
#include "../src/config.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

void test_config_files() {
    // 21. Configuration File Support
    std::ofstream ofs("test_config.json");
    ofs << "{\"simulation\": {\"enable_color\": true}}";
    ofs.close();
    auto cfg = loadConfigJSON("test_config.json");
    assert(cfg.enable_color == true);
    std::cout << "Test 21 passed: Config Support\n";
}

void test_alternative_formats() {
    // 23. Alternative Input Formats (CSV, XML, YAML)
    auto csv = parseBrainActivityCSV("0,R1,0.5,0,0,0");
    assert(!csv.empty());
    auto xml = parseBrainActivityXML("<frame><timestamp>0</timestamp><region><name>R1</name><intensity>0.5</intensity></region></frame>");
    assert(!xml.empty());
    auto yaml = parseBrainActivityYAML("timestamp_ms: 0\n- region: R1\n  intensity: 0.5");
    assert(!yaml.empty());
    std::cout << "Test 23 passed: Alternative Formats\n";
}

void test_validation_schema() {
    // 25. Input Validation Schema
    assert(validateBrainActivityJSON("[{\"timestamp_ms\": 0, \"brain_activity\": []}]") == true);
    assert(validateBrainActivityJSON("{bad: data}") == false);
    std::cout << "Test 25 passed: Validation Schema\n";
}

void test_state_save_load() {
    // 27. State Save/Load & 30. Binary Data Format
    std::vector<BrainFrame> frames;
    BrainFrame f; f.timestamp_ms = 123; f.regions.push_back({"R1", 0.7});
    frames.push_back(f);
    saveSimulationState(frames, "test_state.bin");
    auto loaded = loadSimulationState("test_state.bin");
    assert(!loaded.empty());
    assert(loaded[0].timestamp_ms == 123);
    assert(loaded[0].regions[0].intensity == 0.7);
    std::cout << "Test 27/30 passed: State Save/Load (Binary)\n";
}

int main() {
    test_config_files();
    test_alternative_formats();
    test_validation_schema();
    test_state_save_load();
    std::cout << "Category 3 Tests Complete.\n";
    return 0;
}

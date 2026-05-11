#include "core/data_parsing_hub.h"
#include <cassert>
#include <iostream>

void test_trim() {
    assert(cerebra::trim("  hello  ") == "hello");
    assert(cerebra::trim("\tworld\n") == "world");
    assert(cerebra::trim("already_trimmed") == "already_trimmed");
    std::cout << "test_trim passed" << std::endl;
}

void test_json_parsing() {
    std::string json = R"({
        "timestamp_ms": 1000,
        "brain_activity": [
            {"region": "prefrontal_cortex", "intensity": 0.8}
        ]
    })";
    auto frames = cerebra::parse_frames_by_format(json, "json");
    assert(frames.size() == 1);
    assert(frames[0].timestamp_ms == 1000);
    assert(frames[0].regions.size() == 1);
    assert(frames[0].regions[0].region == "prefrontal_cortex");
    assert(frames[0].regions[0].intensity == 0.8);
    std::cout << "test_json_parsing passed" << std::endl;
}

void test_yaml_parsing() {
    std::string yaml = "timestamp_ms: 2000\n- region: insula\n  intensity: 0.5\n";
    auto frames = cerebra::parse_frames_by_format(yaml, "yaml");
    assert(frames.size() == 1);
    assert(frames[0].timestamp_ms == 2000);
    assert(frames[0].regions.size() == 1);
    assert(frames[0].regions[0].region == "insula");
    std::cout << "test_yaml_parsing passed" << std::endl;
}

void test_xml_parsing() {
    std::string xml = "<frame><timestamp>3000</timestamp><region><name>amygdala</name><intensity>0.9</intensity></region></frame>";
    auto frames = cerebra::parse_frames_by_format(xml, "xml");
    assert(frames.size() == 1);
    assert(frames[0].timestamp_ms == 3000);
    assert(frames[0].regions.size() == 1);
    assert(frames[0].regions[0].region == "amygdala");
    std::cout << "test_xml_parsing passed" << std::endl;
}

void test_csv_parsing() {
    std::string csv = "4000,thalamus,0.7\n4000,hippocampus,0.6\n";
    auto frames = cerebra::parse_frames_by_format(csv, "csv");
    assert(frames.size() == 1);
    assert(frames[0].timestamp_ms == 4000);
    assert(frames[0].regions.size() == 2);
    std::cout << "test_csv_parsing passed" << std::endl;
}

int main() {
    test_trim();
    test_json_parsing();
    test_yaml_parsing();
    test_xml_parsing();
    test_csv_parsing();
    std::cout << "All DataParsingHub unit tests passed!" << std::endl;
    return 0;
}

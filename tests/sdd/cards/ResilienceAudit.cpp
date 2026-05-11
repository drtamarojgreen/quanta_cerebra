#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include "../cpp/util/fact_utils.h"

using namespace Sorrel::Sdd::Util;
namespace fs = std::filesystem;

// @Card: resilience_audit
// @Description: Verifies that integrated components follow namespace and error handling standards.

int main() {
    std::string fact_path = "tests/sdd/facts/integration_hub.facts";
    if (!fs::exists(fact_path)) {
        std::cerr << "error = integration_hub.facts not found" << std::endl;
        return 1;
    }

    auto hub_facts = FactReader::readFacts(fact_path);
    std::string target_ns = hub_facts["namespace"];
    
    std::string src_dir = "src";
    bool all_ok = true;
    int checked_count = 0;
    int namespace_errors = 0;

    std::cout << "Starting Resilience Audit (Standard: " << hub_facts["reliability_standard"] << ")" << std::endl;

    if (fs::exists(src_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(src_dir)) {
            if (entry.path().extension() == ".cpp" || entry.path().extension() == ".h") {
                std::ifstream file(entry.path());
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                
                // Simple check for namespace presence in integrated files
                if (entry.path().string().find("json_parser") != std::string::npos ||
                    entry.path().string().find("data_parsing_hub") != std::string::npos) {
                    
                    checked_count++;
                    if (content.find("namespace " + target_ns) == std::string::npos) {
                        std::cout << "RESILIENCE ERROR: Namespace " << target_ns << " missing in " << entry.path() << std::endl;
                        namespace_errors++;
                        all_ok = false;
                    }
                }
            }
        }
    }

    std::cout << "Audit Summary: Checked " << checked_count << " core files, " << namespace_errors << " namespace errors found." << std::endl;

    return (all_ok && checked_count > 0) ? 0 : 1;
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <regex>
#include "../cpp/util/fact_utils.h"
#include "../../test_config.h"

using namespace Sorrel::Sdd::Util;
namespace fs = std::filesystem;

// @Card: method_audit
// @Results found_methods, missing_methods, found_count, missing_count

std::set<std::string> parse_required_methods(const std::string& raw) {
    std::set<std::string> methods;
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) methods.insert(trim(token));
    return methods;
}

int main() {
    std::string checkin_path = "tests/sdd/sorrel_checkins.md";
    std::string checkout_path = cerebra::test::temp_path("sorrel_checkouts.md");
    
    if (!fs::exists(checkin_path)) {
        std::cerr << "error = sorrel_checkins.md not found" << std::endl;
        return 1;
    }

    auto checkin_data = FactReader::readFacts(checkin_path);
    std::set<std::string> required_methods;
    for (auto const& [category, methods_raw] : checkin_data) {
        if (category.find("#") == 0) continue;
        auto ms = parse_required_methods(methods_raw);
        required_methods.insert(ms.begin(), ms.end());
    }

    std::map<std::string, std::string> empirical_evidence;
    std::string src_dir = "src";
    
    if (fs::exists(src_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(src_dir)) {
            if (entry.path().extension() == ".cpp" || entry.path().extension() == ".h") {
                std::ifstream file(entry.path());
                std::string line;
                int line_num = 0;
                while (std::getline(file, line)) {
                    line_num++;
                    for (const auto& method : required_methods) {
                        // Better check for method definition/declaration
                        if (line.find(method + "(") != std::string::npos || 
                            line.find(method + " (") != std::string::npos ||
                            line.find("::" + method) != std::string::npos) {
                            empirical_evidence[method] = entry.path().string() + ":" + std::to_string(line_num);
                        }
                    }
                }
            }
        }
    }

    std::ofstream checkout(checkout_path);
    checkout << "# Sorrel Checkout: Empirical Audit Results\n";
    int found_count = 0;
    int missing_count = 0;
    
    for (const auto& method : required_methods) {
        if (empirical_evidence.count(method)) {
            checkout << "STATUS [FOUND] " << method << " -> " << empirical_evidence[method] << "\n";
            found_count++;
        } else {
            checkout << "STATUS [MISSING] " << method << "\n";
            missing_count++;
        }
    }

    checkout << "\nSUMMARY:\n";
    checkout << "found_count = " << found_count << "\n";
    checkout << "missing_count = " << missing_count << "\n";
    checkout.close();

    std::cout << "Empirical audit complete. Results written to " << checkout_path << std::endl;
    std::cout << "Summary: " << found_count << " found, " << missing_count << " missing." << std::endl;

    return (missing_count == 0) ? 0 : 1;
}

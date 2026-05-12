#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <set>
#include <algorithm>
#include "../cpp/util/fact_utils.h"
#include "../../test_config.h"

using namespace Sorrel::Sdd::Util;
namespace fs = std::filesystem;

// @Card: structural_audit
// @Description: Enforces CDD structural restrictions (forbidden patterns, namespacing, complexity)

std::vector<std::string> split_list(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        std::string trimmed = trim(item);
        if (!trimmed.empty()) result.push_back(trimmed);
    }
    return result;
}

// Simple check to see if a pattern is likely inside a comment
bool is_commented(const std::string& line, size_t pos) {
    size_t line_comment = line.find("//");
    if (line_comment != std::string::npos && line_comment < pos) return true;

    // Multi-line comment tracking would require stateful parsing,
    // for this minimal card we handle single-line comments.
    return false;
}

int main() {
    std::string fact_path = "tests/sdd/facts/restrictions.facts";
    std::string checkout_path = cerebra::test::temp_path("structural_audit_checkouts.md");

    if (!fs::exists(fact_path)) {
        std::cerr << "error = restrictions.facts not found" << std::endl;
        return 1;
    }

    auto facts = FactReader::readFacts(fact_path);
    auto forbidden = split_list(facts["forbidden_patterns"], ',');
    std::string required_ns = facts["required_namespacing"];
    int max_lines = 0;
    try { max_lines = std::stoi(facts["max_complexity_per_file"]); } catch (...) { max_lines = 1000; }

    std::cout << "Starting Enhanced Structural Audit..." << std::endl;

    std::ofstream checkout(checkout_path);
    checkout << "# Sorrel Checkout: Enhanced Structural Audit Results\n";

    int violations = 0;
    std::string src_dir = "src";

    if (fs::exists(src_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(src_dir)) {
            if (entry.path().extension() == ".cpp" || entry.path().extension() == ".h" || entry.path().extension() == ".hpp") {
                std::ifstream file(entry.path());
                std::string line;
                int line_num = 0;
                bool found_ns = false;

                while (std::getline(file, line)) {
                    line_num++;

                    // 1. Check forbidden patterns (with minimal comment filtering)
                    for (const auto& pattern : forbidden) {
                        size_t pos = line.find(pattern);
                        if (pos != std::string::npos && !is_commented(line, pos)) {
                            std::cout << "[VIOLATION] [PATTERN] " << entry.path().string() << ":" << line_num
                                      << " contains: '" << pattern << "'" << std::endl;
                            checkout << "VIOLATION [PATTERN] [" << pattern << "] " << entry.path().string() << ":" << line_num << "\n";
                            violations++;
                        }
                    }

                    // 2. Check namespacing
                    if (!required_ns.empty() && line.find("namespace " + required_ns) != std::string::npos) {
                        found_ns = true;
                    }
                }

                // 3. Complexity check
                if (line_num > max_lines) {
                    std::cout << "[VIOLATION] [COMPLEXITY] " << entry.path().string() << " has " << line_num << " lines (Max: " << max_lines << ")" << std::endl;
                    checkout << "VIOLATION [COMPLEXITY] " << entry.path().string() << " lines=" << line_num << "\n";
                    violations++;
                }

                // 4. Namespace enforcement (for core/src files)
                if (!required_ns.empty() && !found_ns && entry.path().extension() == ".cpp") {
                    // Only enforce on .cpp files to avoid header complexity in this simple card
                     std::cout << "[VIOLATION] [NAMESPACE] " << entry.path().string() << " missing 'namespace " << required_ns << "'" << std::endl;
                     checkout << "VIOLATION [NAMESPACE] " << entry.path().string() << "\n";
                     violations++;
                }
            }
        }
    }

    checkout << "\nSUMMARY:\n";
    checkout << "violations_found = " << violations << "\n";
    checkout.close();

    std::cout << "\nAudit Summary: " << violations << " violations found." << std::endl;
    std::cout << "Results written to " << checkout_path << std::endl;

    bool success = (violations == 0);
    std::cout << "Structural Compliance: " << (success ? "PASSED" : "FAILED") << std::endl;

    return success ? 0 : 1;
}

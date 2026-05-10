#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <regex>
#include "../cpp/util/fact_utils.h"

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
    std::set<std::string> required_methods;
    std::set<std::string> required_files;
    std::string facts_dir = "tests/sdd/facts";
    for (const auto& entry : fs::directory_iterator(facts_dir)) {
        if (entry.path().extension() == ".facts") {
            auto facts = FactReader::readFacts(entry.path().string());
            if (facts.count("required_methods")) {
                auto methods = parse_required_methods(facts.at("required_methods"));
                required_methods.insert(methods.begin(), methods.end());
            }
            if (facts.count("required_files")) {
                auto files = parse_required_methods(facts.at("required_files"));
                required_files.insert(files.begin(), files.end());
            }
        }
    }

    if (required_methods.empty() && required_files.empty()) {
        std::cerr << "error = no required_methods or required_files found in facts files" << std::endl;
        return 1;
    }

    std::set<std::string> found_methods;
    std::set<std::string> found_files;
    std::string src_dir = "src";
    
    if (fs::exists(src_dir)) {
        for (const auto& entry : fs::recursive_directory_iterator(src_dir)) {
            if (entry.path().extension() == ".cpp" || entry.path().extension() == ".h") {
                std::ifstream file(entry.path());
                std::string line;
                while (std::getline(file, line)) {
                    for (const auto& method : required_methods) {
                        if (line.find(method + "(") != std::string::npos || line.find(method + " ") != std::string::npos) {
                            found_methods.insert(method);
                        }
                    }
                }
            }
        }
    }

    for (const auto& f : required_files) {
        if (fs::exists(f)) found_files.insert(f);
    }

    std::vector<std::string> missing;
    for (const auto& method : required_methods) {
        if (found_methods.find(method) == found_methods.end()) missing.push_back(method);
    }
    for (const auto& f : required_files) {
        if (found_files.find(f) == found_files.end()) missing.push_back(f);
    }

    std::cout << "found_count = " << (found_methods.size() + found_files.size()) << std::endl;
    std::cout << "missing_count = " << missing.size() << std::endl;
    
    std::cout << "found_items = ";
    for (const auto& m : found_methods) std::cout << m << " ";
    for (const auto& f : found_files) std::cout << f << " ";
    std::cout << std::endl;

    if (!missing.empty()) {
        std::cout << "missing_items = ";
        for (const auto& m : missing) std::cout << m << " ";
        std::cout << std::endl;
        return 1;
    }

    return 0;
}

#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include <string>
#include <filesystem>

namespace cerebra {
namespace test {

inline std::string temp_dir() {
    std::filesystem::create_directories("tests/temp");
    return "tests/temp";
}

inline std::string temp_path(const std::string& filename) {
    return (std::filesystem::path(temp_dir()) / filename).string();
}

} // namespace test
} // namespace cerebra

#endif

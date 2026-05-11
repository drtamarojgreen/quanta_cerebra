#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include <string>
#include <filesystem>

namespace cerebra {
namespace test {

inline std::string temp_path(const std::string& filename) {
    std::filesystem::create_directories("tests/temp");
    return (std::filesystem::path("tests/temp") / filename).string();
}

} // namespace test
} // namespace cerebra

#endif

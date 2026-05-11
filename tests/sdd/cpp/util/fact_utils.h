#ifndef FACT_UTILS_H
#define FACT_UTILS_H

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

namespace Sorrel {
namespace Sdd {
namespace Util {

inline std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return ";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

class FactReader {
public:
    static std::map<std::string, std::string> readFacts(const std::string& path) {
        std::map<std::string, std::string> facts;
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string val = trim(line.substr(pos + 1));
                facts[key] = val;
            }
        }
        return facts;
    }
};

}
}
}

#endif

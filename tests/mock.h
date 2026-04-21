#ifndef MOCK_H
#define MOCK_H
#include <string>
#include <map>

// Enhancement 199: Lightweight Mocking Framework
class Mock {
    std::map<std::string, int> calls;
public:
    void expectCall(std::string name) { calls[name]++; }
    int getCalls(std::string name) { return calls[name]; }
};

#endif

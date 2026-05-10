#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <sstream>

struct TestFailure : public std::exception {
    std::string message;
    TestFailure(std::string m) : message(m) {}
    const char* what() const noexcept override { return message.c_str(); }
};

template<typename T>
std::string to_string_custom(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

// Specialization for strings to avoid extra quotes if not needed, 
// but here we just want it to work for any type that supports <<
inline std::string to_string_custom(const std::string& val) {
    return val;
}

inline std::string to_string_custom(const char* val) {
    return std::string(val);
}

#define ASSERT_TRUE(condition, msg) \
    if (!(condition)) throw TestFailure(std::string("Assertion failed: ") + msg + " at " + __FILE__ + ":" + std::to_string(__LINE__))

#define ASSERT_EQ(a, b, msg) \
    if (!((a) == (b))) throw TestFailure(std::string("Assertion failed: ") + msg + " (Expected " + to_string_custom(b) + ", got " + to_string_custom(a) + ") at " + __FILE__ + ":" + std::to_string(__LINE__))

inline void run_test(const std::string& name, void (*test_func)()) {
    try {
        test_func();
        std::cout << "[PASS] " << name << std::endl;
    } catch (const TestFailure& e) {
        std::cerr << "[FAIL] " << name << ": " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERR ] " << name << ": Unexpected exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERR ] " << name << ": Unknown error" << std::endl;
    }
}

#endif

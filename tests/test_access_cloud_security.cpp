#include "../src/json_logic.h"
#include <cassert>
#include <iostream>

void test_security() {
    // 141. Encryption
    std::string secret = "hello";
    std::string key = "key";
    std::string enc = encryptData(secret, key);
    assert(enc != secret);
    assert(encryptData(enc, key) == secret);
    std::cout << "Test 141 passed: Encryption\n";
}

void test_sanitization() {
    // 144. Input Sanitization
    std::string bad = "data\x01\x02";
    std::string clean = trim(bad);
    assert(clean.find('\x01') == std::string::npos);
    std::cout << "Test 144 passed: Sanitization\n";
}

int main() {
    test_security();
    test_sanitization();
    std::cout << "Categories 12-15 Tests Complete.\n";
    return 0;
}

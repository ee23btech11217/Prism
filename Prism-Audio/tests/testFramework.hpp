#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cmath>

// Simple test framework
class TestFramework {
private:
    struct TestCase {
        std::string name;
        std::function<void()> test_func;
    };
    
    std::vector<TestCase> tests;
    int passed = 0;
    int failed = 0;
    
public:
    void add_test(const std::string& name, std::function<void()> test_func) {
        tests.push_back({name, test_func});
    }
    
    void run_all() {
        std::cout << "Running " << tests.size() << " tests...\n";
        std::cout << "======================================\n";
        
        for (const auto& test : tests) {
            std::cout << "TEST: " << test.name << " ... ";
            try {
                test.test_func();
                std::cout << "PASSED\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "FAILED: " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cout << "FAILED: Unknown error\n";
                failed++;
            }
        }
        
        std::cout << "======================================\n";
        std::cout << "Results: " << passed << " passed, " << failed << " failed\n";
    }
    
    int get_exit_code() const {
        return failed > 0 ? 1 : 0;
    }
};

// Test assertion macros
#define TEST_ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #condition); \
    }

#define TEST_ASSERT_EQUAL(expected, actual) \
    if (!((expected) == (actual))) { \
        throw std::runtime_error(std::string("Expected ") + #expected + " == " + #actual); \
    }

#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
    if (!((expected) != (actual))) { \
        throw std::runtime_error(std::string("Expected ") + #expected + " != " + #actual); \
    }

#define TEST_ASSERT_FLOAT_EQUAL(expected, actual, epsilon) \
    if (!(std::abs((expected) - (actual)) <= (epsilon))) { \
        throw std::runtime_error(std::string("Expected ") + #expected + " approximately equal to " + #actual); \
    } 
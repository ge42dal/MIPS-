#pragma once

// Catch2 single header include - simplified version for this project
#define CATCH_CONFIG_MAIN

#ifndef CATCH2_HPP
#define CATCH2_HPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <sstream>

namespace Catch {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry registry;
        return registry;
    }
    
    void add_test(const std::string& name, std::function<void()> func) {
        tests.push_back({name, func});
    }
    
    void run_all() {
        int passed = 0;
        int failed = 0;
        
        for (const auto& test : tests) {
            std::cout << "Running: " << test.name << std::endl;
            try {
                test.func();
                std::cout << "  PASSED" << std::endl;
                passed++;
            } catch (const std::exception& e) {
                std::cout << "  FAILED: " << e.what() << std::endl;
                failed++;
            }
        }
        
        std::cout << "\nResults: " << passed << " passed, " << failed << " failed" << std::endl;
        if (failed > 0) {
            exit(1);
        }
    }
    
private:
    std::vector<TestCase> tests;
};

class AssertionFailure : public std::exception {
public:
    AssertionFailure(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
private:
    std::string message;
};

// Helper to convert values to string for output
template<typename T>
std::string to_string_helper(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename T, typename U>
void require_equal(const T& actual, const U& expected, const char* file, int line) {
    if (!(actual == expected)) {
        std::ostringstream oss;
        oss << "Assertion failed at " << file << ":" << line 
            << " - Expected: " << to_string_helper(expected) << ", Actual: " << to_string_helper(actual);
        throw AssertionFailure(oss.str());
    }
}

template<typename T>
void require_true(const T& condition, const char* file, int line) {
    if (!condition) {
        std::ostringstream oss;
        oss << "Assertion failed at " << file << ":" << line << " - Expected true, got false";
        throw AssertionFailure(oss.str());
    }
}

template<typename T>
void require_false(const T& condition, const char* file, int line) {
    if (condition) {
        std::ostringstream oss;
        oss << "Assertion failed at " << file << ":" << line << " - Expected false, got true";
        throw AssertionFailure(oss.str());
    }
}

void require_throws(std::function<void()> func, const char* file, int line) {
    try {
        func();
        std::ostringstream oss;
        oss << "Assertion failed at " << file << ":" << line << " - Expected exception, but none was thrown";
        throw AssertionFailure(oss.str());
    } catch (const AssertionFailure&) {
        throw; // Re-throw assertion failures
    } catch (...) {
        // Expected exception was thrown, test passes
    }
}

} // namespace Catch

// Macros with unique naming using __LINE__
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define UNIQUE_NAME(base) CONCAT(base, __LINE__)

#define TEST_CASE(name) \
    static void UNIQUE_NAME(test_func_)(); \
    static bool UNIQUE_NAME(registered_) = []() { \
        Catch::TestRegistry::instance().add_test(name, UNIQUE_NAME(test_func_)); \
        return true; \
    }(); \
    static void UNIQUE_NAME(test_func_)()

#define REQUIRE_EQ(actual, expected) Catch::require_equal(actual, expected, __FILE__, __LINE__)
#define REQUIRE(condition) Catch::require_true(condition, __FILE__, __LINE__)
#define REQUIRE_FALSE(condition) Catch::require_false(condition, __FILE__, __LINE__)
#define REQUIRE_THROWS(expression) Catch::require_throws([&]() { expression; }, __FILE__, __LINE__)

// Main function for tests
#define CATCH_CONFIG_MAIN \
int main() { \
    Catch::TestRegistry::instance().run_all(); \
    return 0; \
}

#endif // CATCH2_HPP

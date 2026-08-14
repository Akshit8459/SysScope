#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <exception>

namespace sysscope::test {

struct TestRegistration {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestRegistration>& get_test_registry() {
    static std::vector<TestRegistration> registry;
    return registry;
}

inline bool register_test(const std::string& name, std::function<void()> func) {
    get_test_registry().push_back({name, std::move(func)});
    return true;
}

#define TEST_CASE_IMPL(name, id) \
    static void test_func_##id(); \
    static bool registered_##id = ::sysscope::test::register_test(name, test_func_##id); \
    static void test_func_##id()

#define TEST_CASE_EXPAND(name, id) TEST_CASE_IMPL(name, id)
#define TEST_CASE(name) TEST_CASE_EXPAND(name, __COUNTER__)

#define REQUIRE(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "   [FAIL] Assertion failed: " #cond " at " __FILE__ ":" << __LINE__ << "\n"; \
            throw std::runtime_error("Assertion failed: " #cond); \
        } \
    } while (0)

#define REQUIRE_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "   [FAIL] Expected " #a " == " #b " at " __FILE__ ":" << __LINE__ << "\n"; \
            throw std::runtime_error("Assertion failed: " #a " == " #b); \
        } \
    } while (0)

} // namespace sysscope::test

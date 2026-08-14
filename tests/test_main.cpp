#include "tests/test_framework.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "===================================================\n";
    std::cout << " Running SysScope Unit Test Suite\n";
    std::cout << "===================================================\n\n";

    auto& registry = sysscope::test::get_test_registry();
    int passed = 0;
    int failed = 0;

    for (const auto& test : registry) {
        std::cout << "[RUN] " << test.name << " ... ";
        try {
            test.func();
            std::cout << "PASSED\n";
            passed++;
        } catch (const std::exception& ex) {
            std::cout << "FAILED (" << ex.what() << ")\n";
            failed++;
        } catch (...) {
            std::cout << "FAILED (Unknown Exception)\n";
            failed++;
        }
    }

    std::cout << "\n===================================================\n";
    std::cout << " Test Results: " << passed << " Passed, " << failed << " Failed.\n";
    std::cout << "===================================================\n";

    return (failed == 0) ? 0 : 1;
}

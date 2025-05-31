#include <iostream>
#include <stdexcept> // For std::exception

// Forward declaration for the main test execution function
// This function is defined in sdk_unit_tests.cpp
void run_all_unit_tests();

// The SpacetimeDB::LogLevel enum and other necessary definitions for host stubs
// are expected to be included via test_common.h, which is included by sdk_unit_tests.cpp.
// Minimal stubs for ModuleSchema registration functions are also expected to be handled
// by test_common.h or by direct includes in sdk_unit_tests.cpp if needed by macros.

int main() {
    try {
        run_all_unit_tests();
    } catch (const std::exception& e) {
        std::cerr << "Unit tests failed with an unhandled exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unit tests failed with an unknown unhandled exception." << std::endl;
        return 1;
    }
    // run_all_unit_tests() should print its own summary.
    // This main just provides the entry point and top-level catch.
    std::cout << "Test runner main finished." << std::endl;
    return 0;
}

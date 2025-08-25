#include "catch2.hpp"

// this file serves as the main test runner
// all test cases are automatically registered via the TEST_CASE macro

int main() {
    Catch::TestRegistry::instance().run_all();
    return 0;
}
// the main() function is provided by catch2.hpp

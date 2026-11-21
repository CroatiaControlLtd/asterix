/*
 * Unit tests of the asterix decoder (src/asterix), built with CMake (-DASTERIX_TESTS=ON).
 *
 * usage: asterix_unit_tests [definitions ini, default install/config/asterix.ini] [name filter]
 * Run it from the repository root (ctest does). Exit status: number of failed test cases.
 */
#include <cstdio>
#include <cstring>
#include <iostream>

#include "minitest.h"
#include "test_support.h"

// globals the decoder expects from the application (src/main/asterix.cpp)
bool gVerbose = false;
bool gSynchronous = false;
bool gForceRouting = false;
int gHeartbeat = 0;
const char *gAsterixDefinitionsFile = NULL;
bool gFiltering = false;

namespace minitest {

std::vector<TestCase> &registry() {
    static std::vector<TestCase> tests;
    return tests;
}

static int g_failures = 0;
static const char *g_current = "";

void report(const char *file, int line, const std::string &msg) {
    if (g_failures++ < 200)
        std::cout << "  FAILED " << file << ":" << line << " [" << g_current << "]\n          " << msg << "\n";
}

}  // namespace minitest

const char *g_definitionsFile = "install/config/asterix.ini";

int main(int argc, char **argv) {
    if (argc > 1)
        g_definitionsFile = argv[1];
    const char *filter = argc > 2 ? argv[2] : NULL;

    int failedCases = 0, run = 0;
    std::vector<minitest::TestCase> &tests = minitest::registry();
    for (size_t i = 0; i < tests.size(); i++) {
        if (filter && !strstr(tests[i].name, filter))
            continue;
        minitest::g_current = tests[i].name;
        int before = minitest::g_failures;
        try {
            tests[i].func();
        } catch (minitest::RequireFailed &) {
        } catch (std::exception &e) {
            minitest::report(tests[i].file, tests[i].line, std::string("exception: ") + e.what());
        }
        run++;
        if (minitest::g_failures != before) {
            failedCases++;
            std::cout << "FAILED  " << tests[i].name << "\n";
        } else {
            std::cout << "OK      " << tests[i].name << "\n";
        }
    }
    std::cout << "\n" << run << " test cases, " << failedCases << " failed, " << minitest::g_failures
              << " failed checks\n";
    return failedCases > 100 ? 100 : failedCases;
}

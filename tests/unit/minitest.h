/*
 * Minimal unit test support for the asterix decoder tests (C++11, no dependencies).
 *
 *   TEST_CASE("name") { CHECK(cond); CHECK_EQ(a, b); REQUIRE(cond); }
 *
 * CHECK* record a failure and continue, REQUIRE stops the current test case. main() is in
 * test_main.cpp: it runs every test case (or those whose name contains argv[2]) and returns the
 * number of failed test cases.
 */
#ifndef MINITEST_H_
#define MINITEST_H_

#include <sstream>
#include <string>
#include <vector>

namespace minitest {

typedef void (*TestFunc)();

struct TestCase {
    const char *name;
    TestFunc func;
    const char *file;
    int line;
};

std::vector<TestCase> &registry();
void report(const char *file, int line, const std::string &msg);  // records a failed check

struct Registrar {
    Registrar(const char *name, TestFunc func, const char *file, int line) {
        TestCase t = {name, func, file, line};
        registry().push_back(t);
    }
};

struct RequireFailed {};

template<typename A, typename B>
std::string describe(const char *ea, const char *eb, const A &a, const B &b) {
    std::ostringstream s;
    s << ea << " == " << eb << "\n          got: " << a << "\n     expected: " << b;
    return s.str();
}

}  // namespace minitest

#define MT_CAT2(a, b) a##b
#define MT_CAT(a, b) MT_CAT2(a, b)
#define TEST_CASE(name)                                                                       \
    static void MT_CAT(mt_test_, __LINE__)();                                                 \
    static minitest::Registrar MT_CAT(mt_reg_, __LINE__)(name, MT_CAT(mt_test_, __LINE__),    \
                                                         __FILE__, __LINE__);                 \
    static void MT_CAT(mt_test_, __LINE__)()

#define CHECK(cond)                                                                           \
    do {                                                                                      \
        if (!(cond)) minitest::report(__FILE__, __LINE__, "CHECK(" #cond ")");               \
    } while (0)

#define CHECK_EQ(a, b)                                                                        \
    do {                                                                                      \
        if (!((a) == (b))) minitest::report(__FILE__, __LINE__, minitest::describe(#a, #b, (a), (b))); \
    } while (0)

#define REQUIRE(cond)                                                                         \
    do {                                                                                      \
        if (!(cond)) {                                                                        \
            minitest::report(__FILE__, __LINE__, "REQUIRE(" #cond ")");                      \
            throw minitest::RequireFailed();                                                  \
        }                                                                                     \
    } while (0)

#endif /* MINITEST_H_ */

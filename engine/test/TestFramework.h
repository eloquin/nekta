#pragma once

// Minimal header-only test framework. Deliberately dependency-free: SPEC §10
// says ask before pulling in a third-party library, and a test runner is not
// worth the ask.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace nekta {
namespace test {

struct TestCase {
  const char* name;
  void (*fn)();
};

inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> cases;
  return cases;
}

inline int& failures() {
  static int f = 0;
  return f;
}

inline int& checks() {
  static int c = 0;
  return c;
}

inline bool& currentFailed() {
  static bool f = false;
  return f;
}

struct Registrar {
  Registrar(const char* name, void (*fn)()) { registry().push_back({name, fn}); }
};

inline void reportFailure(const char* file, int line, const char* expr, const char* detail) {
  ++failures();
  currentFailed() = true;
  std::printf("    FAIL %s:%d: %s%s%s\n", file, line, expr, detail ? "  " : "", detail ? detail : "");
}

inline int runAll(const char* suite) {
  std::printf("== %s ==\n", suite);
  int failed = 0;
  for (const TestCase& c : registry()) {
    currentFailed() = false;
    const int before = failures();
    c.fn();
    const bool ok = failures() == before;
    if (!ok) ++failed;
    std::printf("  [%s] %s\n", ok ? "pass" : "FAIL", c.name);
  }
  std::printf("-- %s: %d/%d tests passed, %d checks, %d failures\n", suite,
              static_cast<int>(registry().size()) - failed,
              static_cast<int>(registry().size()), checks(), failures());
  return failures() == 0 ? 0 : 1;
}

}  // namespace test
}  // namespace nekta

#define NEKTA_TEST(name)                                              \
  static void name();                                                 \
  static ::nekta::test::Registrar nekta_reg_##name(#name, name);      \
  static void name()

#define CHECK(cond)                                                            \
  do {                                                                         \
    ++::nekta::test::checks();                                                 \
    if (!(cond)) ::nekta::test::reportFailure(__FILE__, __LINE__, #cond, nullptr); \
  } while (0)

// Aborts the current test case (not the process) on failure.
#define REQUIRE(cond)                                                          \
  do {                                                                         \
    ++::nekta::test::checks();                                                 \
    if (!(cond)) {                                                             \
      ::nekta::test::reportFailure(__FILE__, __LINE__, #cond, "(required)");   \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define CHECK_NEAR(a, b, tol)                                                  \
  do {                                                                         \
    ++::nekta::test::checks();                                                 \
    const double nekta_a = static_cast<double>(a);                             \
    const double nekta_b = static_cast<double>(b);                             \
    const double nekta_t = static_cast<double>(tol);                           \
    if (!(std::fabs(nekta_a - nekta_b) <= nekta_t)) {                          \
      char buf[192];                                                           \
      std::snprintf(buf, sizeof(buf), "(%.9g vs %.9g, tol %.9g, delta %.9g)",  \
                    nekta_a, nekta_b, nekta_t, nekta_a - nekta_b);             \
      ::nekta::test::reportFailure(__FILE__, __LINE__, #a " ~= " #b, buf);     \
    }                                                                          \
  } while (0)

#define CHECK_EQ_INT(a, b)                                                     \
  do {                                                                         \
    ++::nekta::test::checks();                                                 \
    const long long nekta_a = static_cast<long long>(a);                       \
    const long long nekta_b = static_cast<long long>(b);                       \
    if (nekta_a != nekta_b) {                                                  \
      char buf[128];                                                           \
      std::snprintf(buf, sizeof(buf), "(%lld vs %lld)", nekta_a, nekta_b);     \
      ::nekta::test::reportFailure(__FILE__, __LINE__, #a " == " #b, buf);     \
    }                                                                          \
  } while (0)

#define NEKTA_TEST_MAIN(suite) \
  int main() { return ::nekta::test::runAll(suite); }

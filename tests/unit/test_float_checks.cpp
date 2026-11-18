// Compiled with -ffast-math (see CMakeLists.txt): the std:: forms fold to
// constants there, the xl:: forms must not.
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstring>

#include "utils/FloatChecks.h"

namespace {
template <typename T>
T Opaque(T v) {
    volatile T x = v;
    return x;
}

// Under -ffinite-math-only clang treats std::numeric_limits<T>::quiet_NaN() / infinity() as values that
// cannot occur and folds them to an arbitrary constant (Apple clang 21 turns quiet_NaN() into 32.0), so the
// test must build them from their bit patterns to feed a real NaN / Inf to the xl:: checks.
float MakeNan(float) { const uint32_t b = 0x7FC00000U; float f; std::memcpy(&f, &b, sizeof f); return f; }
double MakeNan(double) { const uint64_t b = 0x7FF8000000000000ULL; double d; std::memcpy(&d, &b, sizeof d); return d; }
float MakeInf(float) { const uint32_t b = 0x7F800000U; float f; std::memcpy(&f, &b, sizeof f); return f; }
double MakeInf(double) { const uint64_t b = 0x7FF0000000000000ULL; double d; std::memcpy(&d, &b, sizeof d); return d; }
}

TEMPLATE_TEST_CASE("xl float classification survives -ffast-math", "[floatchecks]", float, double) {
    const TestType nan = Opaque(MakeNan(TestType()));
    const TestType inf = Opaque(MakeInf(TestType()));
    const TestType one = Opaque(TestType(1));

    CHECK(xl::isnan(nan));
    CHECK_FALSE(xl::isnan(one));
    CHECK_FALSE(xl::isnan(inf));

    CHECK(xl::isinf(inf));
    CHECK(xl::isinf(-inf));
    CHECK_FALSE(xl::isinf(nan));
    CHECK_FALSE(xl::isinf(one));

    CHECK(xl::isfinite(one));
    CHECK(xl::isfinite(Opaque(TestType(0))));
    CHECK_FALSE(xl::isfinite(nan));
    CHECK_FALSE(xl::isfinite(inf));
}

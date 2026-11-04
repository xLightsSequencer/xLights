// Compiled with -ffast-math (see CMakeLists.txt): the std:: forms fold to
// constants there, the xl:: forms must not.
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <limits>

#include "utils/FloatChecks.h"

namespace {
template <typename T>
T Opaque(T v) {
    volatile T x = v;
    return x;
}
}

TEMPLATE_TEST_CASE("xl float classification survives -ffast-math", "[floatchecks]", float, double) {
    using L = std::numeric_limits<TestType>;
    const TestType nan = Opaque(L::quiet_NaN());
    const TestType inf = Opaque(L::infinity());
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

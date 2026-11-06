#include <catch2/catch_test_macros.hpp>

#include "render/ColorCurve.h"

TEST_CASE("ccSortableColorPoint tolerates empty or malformed input", "[curves][colorcurve]") {
    SECTION("empty") {
        ccSortableColorPoint p{std::string()};
        CHECK(p.x == 0.0f);
    }
    SECTION("no separator") {
        ccSortableColorPoint p{std::string("garbage")};
        CHECK(p.x == 0.0f);
    }
}

TEST_CASE("ColorCurve tolerates malformed Values", "[curves][colorcurve]") {
    ColorCurve cc("Active=TRUE|Id=x|Values=garbage|");
    CHECK(cc.GetId() == "x");
    CHECK(cc.GetValueAt(0.5f) == xlBLACK);
    ColorCurve cc2("Active=TRUE|Id=x|Values=;;|");
    CHECK(cc2.GetId() == "x");
}

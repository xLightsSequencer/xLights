#include <catch2/catch_test_macros.hpp>

#include "utils/string_utils.h"

TEST_CASE("hex helpers", "[string_utils]") {
    CHECK(isHexChar('0'));
    CHECK(isHexChar('f'));
    CHECK(isHexChar('F'));
    CHECK_FALSE(isHexChar('g'));
    CHECK(HexToChar('a') == 10);
    CHECK(HexToChar('F') == 15);
    CHECK((unsigned char)HexToChar('f', 'f') == 0xFF);
    CHECK(HexToChar('1', '0') == 16);
}

TEST_CASE("counting", "[string_utils]") {
    CHECK(CountChar("a,b,,c", ',') == 3);
    CHECK(CountChar("", ',') == 0);
    CHECK(CountStrings("ab", "ababab") == 3);
    CHECK(CountStrings("aa", "aaaa") == 2);
    CHECK(CountStrings("x", "abc") == 0);
}

TEST_CASE("searching", "[string_utils]") {
    CHECK(Contains("hello", "ell"));
    CHECK_FALSE(Contains("hello", "z"));
    CHECK(ContainsBetweenCommas("a,bb,c", "bb"));
    CHECK_FALSE(ContainsBetweenCommas("a,bb,c", "b"));
    CHECK(StartsWith("hello", "he"));
    CHECK_FALSE(StartsWith("he", "hello"));
    CHECK(hasAlpha("12a3"));
    CHECK_FALSE(hasAlpha("123"));
}

TEST_CASE("slicing and padding", "[string_utils]") {
    CHECK(PadLeft("7", '0', 3) == "007");
    CHECK(PadLeft("1234", '0', 3) == "1234");
    CHECK(StripAllBut("a1b2c3", "123") == "123");
    CHECK(AfterFirst("key=value=x", '=') == "value=x");
    CHECK(AfterFirst("novalue", '=') == "");
}

TEST_CASE("GenerateGuid yields RFC 4122 v4 UUIDs", "[string_utils]") {
    auto g = GenerateGuid();
    REQUIRE(g.size() == 36);
    CHECK(g[8] == '-');
    CHECK(g[13] == '-');
    CHECK(g[18] == '-');
    CHECK(g[23] == '-');
    CHECK(g[14] == '4');
    CHECK(std::string("89ab").find(g[19]) != std::string::npos);
    CHECK(GenerateGuid() != g);
}

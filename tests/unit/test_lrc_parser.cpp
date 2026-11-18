#include <catch2/catch_test_macros.hpp>

#include "lyrics/LRCParser.h"

using lrc::ParseLRC;
using lrc::SanitizePhraseText;
using Lines = std::vector<std::pair<int, std::string>>;

TEST_CASE("ParseLRC reads centisecond timestamps", "[lrc]") {
    auto r = ParseLRC("[00:12.34] Hello world\n[01:02.50] Second line\n");
    REQUIRE(r.size() == 2);
    CHECK(r[0] == std::make_pair(12340, std::string("Hello world")));
    CHECK(r[1] == std::make_pair(62500, std::string("Second line")));
}

TEST_CASE("ParseLRC accepts every fraction width", "[lrc]") {
    CHECK(ParseLRC("[00:01] a")[0].first == 1000);
    CHECK(ParseLRC("[00:01.5] a")[0].first == 1500);
    CHECK(ParseLRC("[00:01.50] a")[0].first == 1500);
    CHECK(ParseLRC("[00:01.500] a")[0].first == 1500);
    CHECK(ParseLRC("[00:01.123] a")[0].first == 1123);
    CHECK(ParseLRC("[10:00.00] a")[0].first == 600000);
}

TEST_CASE("ParseLRC skips metadata, junk and blank lines", "[lrc]") {
    auto r = ParseLRC("[ti:Song Title]\n[ar:Artist]\n\nno bracket\n[00:05.00] kept\n[bad\n");
    REQUIRE(r.size() == 1);
    CHECK(r[0].second == "kept");
}

TEST_CASE("ParseLRC keeps empty text and only trims one leading space", "[lrc]") {
    auto r = ParseLRC("[00:01.00]\n[00:02.00]  two spaces\n");
    REQUIRE(r.size() == 2);
    CHECK(r[0] == std::make_pair(1000, std::string()));
    CHECK(r[1].second == " two spaces");
}

TEST_CASE("ParseLRC handles empty input and CRLF", "[lrc]") {
    CHECK(ParseLRC("").empty());
    auto r = ParseLRC("[00:01.00] a\r\n[00:02.00] b\r\n");
    REQUIRE(r.size() == 2);
    CHECK(r[0].second == "a\r");
}

TEST_CASE("SanitizePhraseText cleans phrase labels", "[lrc]") {
    std::string t = "  \xE2\x80\x9CIt\xE2\x80\x99s <fine>\xE2\x80\x9D  ";
    CHECK(SanitizePhraseText(t));
    CHECK(t == "It's fine");

    std::string p = "Hello,world.Yes";
    CHECK(SanitizePhraseText(p));
    CHECK(p == "Hello, world. Yes");

    std::string e = " \t ";
    CHECK_FALSE(SanitizePhraseText(e));
    CHECK(e.empty());

    std::string only = "\"<>\"";
    CHECK_FALSE(SanitizePhraseText(only));
}

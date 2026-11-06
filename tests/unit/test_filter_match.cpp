#include <catch2/catch_test_macros.hpp>

#include "utils/FilterMatch.h"

namespace {
bool M(const std::string& query, const std::string& text) {
    return xl::FilterQuery(query).Matches(text);
}
}

TEST_CASE("FilterQuery empty query matches everything", "[filter]") {
    xl::FilterQuery q("");
    CHECK(q.IsEmpty());
    CHECK(q.Matches("anything"));
    CHECK(xl::FilterQuery("   ").IsEmpty());
}

TEST_CASE("FilterQuery substring, case-insensitive", "[filter]") {
    CHECK(M("house", "grp_All_House_Display"));
    CHECK(M("HOUSE", "grp_all_house_display"));
    CHECK_FALSE(M("garage", "grp_all_house_display"));
}

TEST_CASE("FilterQuery words in any order", "[filter]") {
    CHECK(M("all house", "grp_all_house_display"));
    CHECK(M("house all", "grp_all_house_display"));
    CHECK_FALSE(M("house roof", "grp_all_house_display"));
}

TEST_CASE("FilterQuery separators are optional", "[filter]") {
    CHECK(M("allhouse", "grp_all_house_display"));
    CHECK(M("all-house", "grp_all_house_display"));
    CHECK(M("all house", "grp-all-house"));
}

TEST_CASE("FilterQuery wildcards", "[filter]") {
    CHECK(M("grp_*_house", "grp_all_house_display"));
    CHECK(M("h?use", "grp_all_house_display"));
    CHECK_FALSE(M("h??use", "grp_all_house_display"));
    CHECK(M("*", "x"));
}

TEST_CASE("FilterQuery regex", "[filter]") {
    xl::FilterQuery q("/^grp_.*display$/");
    CHECK(q.IsRegex());
    CHECK(q.Matches("grp_all_house_display"));
    CHECK_FALSE(q.Matches("xgrp_all_house_display"));
    CHECK(M("/HOUSE/", "grp_all_house_display"));
}

TEST_CASE("FilterQuery tolerates half-typed regexes", "[filter]") {
    xl::FilterQuery q("/(/");
    CHECK_FALSE(q.IsRegex());
    CHECK_NOTHROW(q.Matches("abc"));
    CHECK(M("/", "anything"));
}

TEST_CASE("AsRegexPattern", "[filter]") {
    std::string p;
    CHECK(xl::AsRegexPattern("/abc/", p));
    CHECK(p == "abc");
    CHECK_FALSE(xl::AsRegexPattern("//", p));
    CHECK_FALSE(xl::AsRegexPattern("abc", p));
    CHECK_FALSE(xl::AsRegexPattern("/abc", p));
}

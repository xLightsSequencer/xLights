#include <catch2/catch_test_macros.hpp>

#include "utils/NodeUtils.h"

using namespace NodeUtils;

TEST_CASE("ExpandNodes", "[nodeutils]") {
    CHECK(ExpandNodes("") == "");
    CHECK(ExpandNodes("5") == "5");
    CHECK(ExpandNodes("1-4") == "1,2,3,4");
    CHECK(ExpandNodes("4-1") == "4,3,2,1");
    CHECK(ExpandNodes("3-3") == "3");
    CHECK(ExpandNodes("1-3,7,9-10") == "1,2,3,7,9,10");
    CHECK(ExpandNodes("0,1-2") == "0,1,2");
}

TEST_CASE("CompressNodes", "[nodeutils]") {
    CHECK(CompressNodes("") == "");
    CHECK(CompressNodes("5") == "5");
    CHECK(CompressNodes("1,2,3,4") == "1-4");
    CHECK(CompressNodes("4,3,2,1") == "4-1");
    CHECK(CompressNodes("1,3,5") == "1,3,5");
    CHECK(CompressNodes("1,2,3,7,9,10") == "1-3,7,9-10");
}

TEST_CASE("CompressNodes keeps blanks and zeros as separators", "[nodeutils]") {
    CHECK(CompressNodes("1,2,3,0,5,6") == "1-3,0,5-6");
    CHECK(CompressNodes("1,2,,4,5") == "1-2,,4-5");
}

TEST_CASE("Compress and Expand round-trip", "[nodeutils]") {
    for (std::string s : { "1,2,3", "10,9,8,20,21", "1,5,9", "1,2,0,0,3,2,1", "7" }) {
        CHECK(ExpandNodes(CompressNodes(s)) == s);
    }
}

TEST_CASE("ShiftNodes", "[nodeutils]") {
    std::map<std::string, std::string> nodes{
        { "Line0", "1-3" },
        { "Line1", "10,20" },
        { "Line2Color", "1-3" },
        { "Line3Name", "5" },
        { "Line4Type", "5" },
        { "Empty", "" },
    };
    ShiftNodes(nodes, 2, 1, 20);
    CHECK(nodes["Line0"] == "3-5");
    CHECK(nodes["Line1"] == "12,2");
    CHECK(nodes["Line2Color"] == "1-3");
    CHECK(nodes["Line3Name"] == "5");
    CHECK(nodes["Line4Type"] == "5");
    CHECK(nodes["Empty"] == "");
}

TEST_CASE("ShiftNodes wraps past the limits", "[nodeutils]") {
    std::map<std::string, std::string> up{ { "L", "19,20" } };
    ShiftNodes(up, 3, 1, 20);
    CHECK(up["L"] == "2-3");

    std::map<std::string, std::string> down{ { "L", "1,2" } };
    ShiftNodes(down, -3, 1, 20);
    CHECK(down["L"] == "18-19");
}

TEST_CASE("ReverseNodes", "[nodeutils]") {
    std::map<std::string, std::string> nodes{
        { "Line0", "1-3" },
        { "Line1Color", "1-3" },
        { "Empty", "" },
    };
    ReverseNodes(nodes, 10);
    CHECK(nodes["Line0"] == "9-7");
    CHECK(nodes["Line1Color"] == "1-3");
    CHECK(nodes["Empty"] == "");
}

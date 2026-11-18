#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializeFunctions.h"

using namespace XmlSerialize;
using Grid = std::vector<std::vector<std::vector<int>>>;

TEST_CASE("ParseCustomModel reads layers, rows and columns", "[xmlserialize]") {
    Grid g = ParseCustomModel("1,2,3;4,5,6|7,8,9;10,11,12");
    REQUIRE(g.size() == 2);
    REQUIRE(g[0].size() == 2);
    REQUIRE(g[0][0].size() == 3);
    CHECK(g[0][0] == std::vector<int>{ 1, 2, 3 });
    CHECK(g[0][1] == std::vector<int>{ 4, 5, 6 });
    CHECK(g[1][1] == std::vector<int>{ 10, 11, 12 });
}

TEST_CASE("ParseCustomModel pads short rows and blank cells with -1", "[xmlserialize]") {
    Grid g = ParseCustomModel("1,,3;4");
    REQUIRE(g.size() == 1);
    CHECK(g[0][0] == std::vector<int>{ 1, -1, 3 });
    CHECK(g[0][1] == std::vector<int>{ 4, -1, -1 });
}

TEST_CASE("ParseCustomModel tolerates leading spaces and junk", "[xmlserialize]") {
    Grid g = ParseCustomModel(" 1, 2,x");
    REQUIRE(g.size() == 1);
    CHECK(g[0][0] == std::vector<int>{ 1, 2, 0 });
}

TEST_CASE("ParseCompressed places nodes by row and column", "[xmlserialize]") {
    Grid g = ParseCompressed("1,0,0;2,0,2;3,1,1");
    REQUIRE(g.size() == 1);
    REQUIRE(g[0].size() == 2);
    CHECK(g[0][0] == std::vector<int>{ 1, -1, 2 });
    CHECK(g[0][1] == std::vector<int>{ -1, 3, -1 });
}

TEST_CASE("ParseCompressed supports a layer field and ignores malformed entries", "[xmlserialize]") {
    Grid g = ParseCompressed("1,0,0,0;2,0,0,1;bad;3,4");
    REQUIRE(g.size() == 2);
    CHECK(g[0][0][0] == 1);
    CHECK(g[1][0][0] == 2);
}

TEST_CASE("ParseCompressed with no nodes gives one empty cell", "[xmlserialize]") {
    Grid g = ParseCompressed("");
    REQUIRE(g.size() == 1);
    CHECK(g[0].size() == 1);
    CHECK(g[0][0] == std::vector<int>{ -1 });
}

TEST_CASE("ParseCustomModelDataFromXml prefers the compressed attribute", "[xmlserialize]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<m CustomModel="9,9" CustomModelCompressed="1,0,0"/>)"));
    Grid g = ParseCustomModelDataFromXml(doc.child("m"));
    CHECK(g[0][0] == std::vector<int>{ 1 });

    REQUIRE(doc.load_string(R"(<m CustomModel="5,6"/>)"));
    g = ParseCustomModelDataFromXml(doc.child("m"));
    CHECK(g[0][0] == std::vector<int>{ 5, 6 });
}

TEST_CASE("AbsolutizeFileReferences anchors relative paths", "[xmlserialize]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<r>
        <model name="a" ObjFile="meshes\house.obj" Image="pics/a.png"/>
        <model name="b" ObjFile="/already/abs.obj" Image="C:\abs\b.png"/>
        <group><model name="c" Image="deep.png"/></group>
        <model name="d" Image=""/>
    </r>)"));
    AbsolutizeFileReferences(doc.child("r"), "/show/base/");

    auto attr = [&](const char* name, const char* a) {
        return doc.select_node((std::string("//model[@name='") + name + "']").c_str()).node().attribute(a).as_string();
    };
    CHECK(std::string(attr("a", "ObjFile")) == "/show/base/meshes/house.obj");
    CHECK(std::string(attr("a", "Image")) == "/show/base/pics/a.png");
    CHECK(std::string(attr("b", "ObjFile")) == "/already/abs.obj");
    CHECK(std::string(attr("b", "Image")) == "C:\\abs\\b.png");
    CHECK(std::string(attr("c", "Image")) == "/show/base/deep.png");
    CHECK(std::string(attr("d", "Image")) == "");
}

TEST_CASE("AbsolutizeFileReferences ignores empty bases", "[xmlserialize]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<r><m Image="a.png"/></r>)"));
    AbsolutizeFileReferences(doc.child("r"), "");
    AbsolutizeFileReferences(doc.child("r"), "///");
    CHECK(std::string(doc.child("r").child("m").attribute("Image").as_string()) == "a.png");
}

TEST_CASE("RelativizeFileReferences undoes Absolutize", "[xmlserialize]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<r><m ObjFile="meshes/house.obj" Image="pics/a.png"/><m Image="/elsewhere/x.png"/></r>)"));
    AbsolutizeFileReferences(doc.child("r"), "/show/base");
    CHECK(std::string(doc.child("r").child("m").attribute("Image").as_string()) == "/show/base/pics/a.png");

    RelativizeFileReferences(doc.child("r"), "/show/base");
    auto m = doc.child("r").child("m");
    CHECK(std::string(m.attribute("ObjFile").as_string()) == "meshes/house.obj");
    CHECK(std::string(m.attribute("Image").as_string()) == "pics/a.png");
    CHECK(std::string(m.next_sibling("m").attribute("Image").as_string()) == "/elsewhere/x.png");
}

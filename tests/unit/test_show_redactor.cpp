#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "support/TempDir.h"
#include "utils/ShowRedactor.h"

namespace {
pugi::xml_document Parse(const char* xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml));
    return doc;
}
std::string Attr(const pugi::xml_document& doc, const char* xpath, const char* attr) {
    return doc.select_node(xpath).node().attribute(attr).as_string();
}
const std::string kTok = ShowRedactor::PATH_TOKEN;
}

TEST_CASE("RedactRgbEffects rewrites absolute paths, keeps the leaf", "[redactor]") {
    auto doc = Parse(R"(<xrgb>
        <model name="a" Image="/Users/someone/shows/pic.png"/>
        <model name="b" Image="C:\Users\someone\pic.gif"/>
        <model name="c" Image="C:/Users/someone/mesh.obj"/>
        <model name="d" Image="\\server\share\clip.mp4"/>
    </xrgb>)");
    auto stats = ShowRedactor::RedactRgbEffects(doc);
    CHECK(stats.paths == 4);
    CHECK(Attr(doc, "//model[@name='a']", "Image") == kTok + "/pic.png");
    CHECK(Attr(doc, "//model[@name='b']", "Image") == kTok + "/pic.gif");
    CHECK(Attr(doc, "//model[@name='c']", "Image") == kTok + "/mesh.obj");
    CHECK(Attr(doc, "//model[@name='d']", "Image") == kTok + "/clip.mp4");
}

TEST_CASE("RedactRgbEffects leaves non-paths alone", "[redactor]") {
    auto doc = Parse(R"(<xrgb>
        <model name="rel" Image="pics/a.png"/>
        <model name="sub" Sub="Group/Submodel"/>
        <model name="port" Port="/dev/ttyUSB0"/>
        <model name="com" Port="COM3"/>
        <model name="short" v="/"/>
        <model name="empty" v=""/>
    </xrgb>)");
    auto stats = ShowRedactor::RedactRgbEffects(doc);
    CHECK(stats.paths == 0);
    CHECK(Attr(doc, "//model[@name='rel']", "Image") == "pics/a.png");
    CHECK(Attr(doc, "//model[@name='port']", "Port") == "/dev/ttyUSB0");
    CHECK(Attr(doc, "//model[@name='com']", "Port") == "COM3");
}

TEST_CASE("RedactRgbEffects reaches paths inside settings blobs", "[redactor]") {
    auto doc = Parse(R"(<xrgb><effect settings="E_SLIDER=5,E_FILEPICKER_File=/home/me/a.png,B_X=1"/></xrgb>)");
    auto stats = ShowRedactor::RedactRgbEffects(doc);
    CHECK(stats.paths == 1);
    CHECK(Attr(doc, "//effect", "settings") == "E_SLIDER=5,E_FILEPICKER_File=" + kTok + "/a.png,B_X=1");
}

TEST_CASE("RedactRgbEffects is a no-op on the wrong kind of document", "[redactor]") {
    auto doc = Parse("<networks><Controller IP=\"192.168.1.5\"/></networks>");
    auto stats = ShowRedactor::RedactRgbEffects(doc);
    CHECK(stats.paths == 0);
    CHECK(stats.addresses == 0);
    CHECK(Attr(doc, "//Controller", "IP") == "192.168.1.5");
}

TEST_CASE("RedactNetworks maps distinct addresses to distinct synthetic ones", "[redactor]") {
    auto doc = Parse(R"(<networks>
        <Controller name="a" IP="192.168.1.5"/>
        <Controller name="b" IP="192.168.1.6" FPPProxy="192.168.1.5"/>
        <Controller name="c" IP="pi.local"/>
        <Controller name="d" IP="192.168.1.5"/>
        <Controller name="e" ComPort="/dev/ttyUSB0"/>
        <Controller name="f" IP=""/>
    </networks>)");
    auto stats = ShowRedactor::RedactNetworks(doc);
    CHECK(stats.addresses == 5);

    auto a = Attr(doc, "//Controller[@name='a']", "IP");
    auto b = Attr(doc, "//Controller[@name='b']", "IP");
    auto c = Attr(doc, "//Controller[@name='c']", "IP");
    CHECK(a == "10.0.0.2");
    CHECK(b == "10.0.0.3");
    CHECK(c == "10.0.0.4");
    CHECK(Attr(doc, "//Controller[@name='d']", "IP") == a);
    CHECK(Attr(doc, "//Controller[@name='b']", "FPPProxy") == a);
    CHECK(Attr(doc, "//Controller[@name='e']", "ComPort") == "/dev/ttyUSB0");
    CHECK(Attr(doc, "//Controller[@name='f']", "IP") == "");
}

TEST_CASE("RedactFileToFile", "[redactor]") {
    TempDir dir;
    auto src = dir.Write("rgb.xml", "<xrgb><model Image=\"/a/b/c.png\"/></xrgb>");
    auto dst = dir.File("out.xml");

    ShowRedactor::Stats stats;
    REQUIRE(ShowRedactor::RedactFileToFile(src, dst, false, &stats));
    CHECK(stats.paths == 1);

    pugi::xml_document out;
    REQUIRE(out.load_file(dst.c_str()));
    CHECK(Attr(out, "//model", "Image") == kTok + "/c.png");

    CHECK_FALSE(ShowRedactor::RedactFileToFile(dir.File("missing.xml"), dst, false));
    CHECK_FALSE(ShowRedactor::RedactFileToFile(src, dir.File("no_such_dir/x.xml"), false));
}

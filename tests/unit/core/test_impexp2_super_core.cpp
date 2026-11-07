// Synthetic SuperStar documents: preprocessing, timing adjustment, layout/options, prefix handling, placement.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "test_impexp2_super_common.h"

using namespace ss2;
using SuperStar::ImageResize;

namespace {
std::string Pre(const std::string& s) {
    std::vector<char> b(s.begin(), s.end());
    SuperStar::PreprocessXmlBuffer(b);
    return std::string(b.begin(), b.end());
}
std::string Shock(int cx, int cy, int layer = 0) {
    return "<flowys><flowy flowyType=\"Shockwave\" layer=\"" + std::to_string(layer) + "\" startTime=\"0\" endTime=\"10\" Colors=\"\" centerX=\"" + std::to_string(cx) +
           "\" centerY=\"" + std::to_string(cy) + "\" startRadius=\"1\" endRadius=\"2\"/></flowys>";
}
}

// ---------------------------------------------------------------- PreprocessXmlBuffer

TEST_CASE("Preprocess leaves empty and tiny buffers alone", "[impexp2][super]") {
    std::vector<char> empty;
    SuperStar::PreprocessXmlBuffer(empty);
    CHECK(empty.empty());
    for (std::string s : { "<", "<a", "<a/>", "1234567", "<pixels=" , "<timing " }) {
        CAPTURE(s);
        std::vector<char> b(s.begin(), s.end());
        SuperStar::PreprocessXmlBuffer(b);
        CHECK(b.size() == s.size());
    }
}

TEST_CASE("Preprocess repairs the <pixels= typo", "[impexp2][super]") {
    CHECK(Pre("<image><pixels=\"1,2,3\"/></image>") == "<image><pixe s=\"1,2,3\"/></image>");
    CHECK(Pre("<pixels=\"7\"/>") == "<pixe s=\"7\"/>");
    CHECK(Pre("<a><pixels=\"1\"/><pixels=\"2\"/></a>") == "<a><pixe s=\"1\"/><pixe s=\"2\"/></a>");
}

TEST_CASE("Preprocess leaves a correct pixels attribute element alone", "[impexp2][super]") {
    const std::string s = "<image><pixels s=\"1,2,3\"/></image>";
    CHECK(Pre(s) == s);
}

TEST_CASE("Preprocess closes elements SuperStar leaves open", "[impexp2][super]") {
    struct C { const char* in; const char* out; };
    auto c = GENERATE(values<C>({
        { "<a>\n<timing x=\"1\">\n</a>", "<a>\n<timing x=\"1\"/></a>" },
        { "<a>\n<group x=\"1\">\n</a>", "<a>\n<group x=\"1\"/></a>" },
        { "<a>\n<imageAction x=\"1\">\n</a>", "<a>\n<imageAction x=\"1\"/></a>" },
        { "<a>\n<textAction x=\"1\">\n</a>", "<a>\n<textAction x=\"1\"/></a>" },
        { "<a>\n<configuration x=\"1\">\n</a>", "<a>\n<configuration x=\"1\"/></a>" },
        { "<a>\r\n<timing x=\"1\">\r\n</a>", "<a>\r\n<timing x=\"1\"/>\n</a>" },
    }));
    CAPTURE(c.in);
    CHECK(Pre(c.in) == c.out);
}

TEST_CASE("Preprocess does not touch already closed or similarly named elements", "[impexp2][super]") {
    for (std::string s : { "<a>\n<timing x=\"1\"/>\n</a>", "<a>\n<timingChannel name=\"x\">\n</timingChannel>\n</a>", "<a>\n<timingGrid name=\"x\">\n</timingGrid>\n</a>",
                           "<a>\n<groups x=\"1\">\n</groups>\n</a>", "<a>\n<configurations>\n</configurations>\n</a>", "<a>\n<imageActions>\n</imageActions>\n</a>" }) {
        CAPTURE(s);
        CHECK(Pre(s) == s);
    }
}

TEST_CASE("Preprocess blanks the closing configuration tag", "[impexp2][super]") {
    auto out = Pre("<a>\n<configuration x=\"1\">\n<b/>\n</configuration>\n</a>");
    CHECK(out.find("</configuration>") == std::string::npos);
    pugi::xml_document d;
    CHECK(d.load_string(out.c_str()));
    CHECK(d.document_element().child("configuration"));
    CHECK(d.document_element().child("b"));
}

TEST_CASE("Preprocess handles a truncated buffer ending inside an open tag", "[impexp2][super]") {
    for (std::string s : { "<a>\n<timing x=\"1\"", "<a>\n<timing x=\"1\">", "<a>\n<timing ", "<a>\n<group x", "<a>\n<imageAction x=\"1\"> " }) {
        CAPTURE(s);
        std::vector<char> b(s.begin(), s.end());
        SuperStar::PreprocessXmlBuffer(b);
        CHECK(b.size() == s.size());
    }
}

TEST_CASE("Preprocess closing a tag must not eat the character after the >", "[impexp2][super]") {
    // PreprocessXmlBuffer: rewrites buf[x] (the char after '>') to '>' instead of inserting; <timing a="1"><b/> loses its '<'
    auto out = Pre("<r><timing a=\"1\"><b/></r>");
    pugi::xml_document d;
    d.load_string(out.c_str());
    CHECK(d.document_element().child("b"));
}

TEST_CASE("Preprocess leaves binary junk and non-ascii without crashing", "[impexp2][super]") {
    std::string junk;
    for (int i = 0; i < 4096; ++i) junk += (char)((i * 37) & 0xff);
    std::vector<char> b(junk.begin(), junk.end());
    SuperStar::PreprocessXmlBuffer(b);
    CHECK(b.size() == junk.size());
    std::string bom = "\xEF\xBB\xBF<a>\n<timing x=\"1\">\n</a>";
    CHECK(Pre(bom) == "\xEF\xBB\xBF<a>\n<timing x=\"1\"/></a>");
}

TEST_CASE("Preprocessed real-world style buffer parses and imports", "[impexp2][super]") {
    std::string doc = "<SuperStarMetaFile saveFileVersion=\"7\">\r\n\t<configurations>\r\n\t\t<configuration name=\"a\" v=\"1\">\r\n\t\t</configuration>\r\n\t</configurations>\r\n\t"
                      "<layouts>\r\n\t\t<layout visualizationMode=\"true\" ribbonOrientation=\"vertical\"/>\r\n\t</layouts>\r\n"
                      "\t<images>\r\n\t\t<image name=\"x\" height=\"2\" width=\"2\" xOffset=\"0\" yOffset=\"0\" savedIndex=\"1\">\r\n\t\t\t<pixels=\"25600,0,100,6579300\"/>\r\n\t\t</image>\r\n\t</images>\r\n"
                      "\t<imageActions>\r\n\t\t<imageAction name=\"n\" imageIndex=\"1\" layer=\"2\" startCentisecond=\"0\" endCentisecond=\"10\" xStart=\"0\" xEnd=\"0\" yStart=\"0\" yEnd=\"0\">\r\n"
                      "\t</imageActions>\r\n</SuperStarMetaFile>\r\n";
    Fixture f;
    f.opt.xSize = 2;
    f.opt.ySize = 2;
    REQUIRE(f.RunRaw(doc));
    REQUIRE(f.Count() == 1);
    CHECK(f.Find("Pictures").Get("E_TEXTCTRL_Pictures_Filename") == "P/1.png");
    CHECK(f.Media().HasMedia("P/1.png"));
}

// ---------------------------------------------------------------- AdjustAllTimings

TEST_CASE("AdjustAllTimings shifts centisecond attributes everywhere", "[impexp2][super]") {
    pugi::xml_document d;
    d.load_string("<r startCentisecond=\"1\"><a startCentisecond=\"10\" endCentisecond=\"20\"><b endCentisecond=\"30\"/></a><c centisecond=\"5\"/></r>");
    SuperStar::AdjustAllTimings(d.document_element(), 7);
    CHECK(d.document_element().attribute("startCentisecond").as_int() == 8);
    CHECK(d.document_element().child("a").attribute("startCentisecond").as_int() == 17);
    CHECK(d.document_element().child("a").attribute("endCentisecond").as_int() == 27);
    CHECK(d.document_element().child("a").child("b").attribute("endCentisecond").as_int() == 37);
    CHECK(d.document_element().child("c").attribute("centisecond").as_int() == 5);
}

TEST_CASE("AdjustAllTimings flowy start and end time", "[impexp2][super]") {
    pugi::xml_document d;
    d.load_string("<r><flowy startTime=\"10\" endTime=\"20\" tailTimeLength=\"5\"/><other startTime=\"10\" endTime=\"20\"/></r>");
    SuperStar::AdjustAllTimings(d.document_element(), -4);
    CHECK(d.document_element().child("flowy").attribute("startTime").as_int() == 6);
    CHECK(d.document_element().child("flowy").attribute("endTime").as_int() == 16);
    CHECK(d.document_element().child("flowy").attribute("tailTimeLength").as_int() == 5);
    CHECK(d.document_element().child("other").attribute("startTime").as_int() == 10);
    CHECK(d.document_element().child("other").attribute("endTime").as_int() == 20);
}

TEST_CASE("AdjustAllTimings morph states but not ramp extension", "[impexp2][super]") {
    pugi::xml_document d;
    d.load_string("<morph><state1 time=\"10\"/><state2 time=\"20\"/><ramp timeExt=\"5\" time=\"1\"/><state3 time=\"3\"/></morph>");
    SuperStar::AdjustAllTimings(d.document_element(), 100);
    auto m = d.document_element();
    CHECK(m.child("state1").attribute("time").as_int() == 110);
    CHECK(m.child("state2").attribute("time").as_int() == 120);
    CHECK(m.child("ramp").attribute("timeExt").as_int() == 5);
    CHECK(m.child("ramp").attribute("time").as_int() == 1);
    CHECK(m.child("state3").attribute("time").as_int() == 3);
}

TEST_CASE("AdjustAllTimings copes with null nodes, no attributes and zero offset", "[impexp2][super]") {
    SuperStar::AdjustAllTimings(pugi::xml_node(), 5);
    pugi::xml_document d;
    SuperStar::AdjustAllTimings(d.document_element(), 5);
    d.load_string("<r><a/><state1/><flowy/></r>");
    SuperStar::AdjustAllTimings(d.document_element(), 5);
    CHECK_FALSE(d.document_element().child("state1").attribute("time"));
    d.load_string("<r startCentisecond=\"3\"/>");
    SuperStar::AdjustAllTimings(d.document_element(), 0);
    CHECK(d.document_element().attribute("startCentisecond").as_int() == 3);
}

TEST_CASE("AdjustAllTimings treats non numeric attributes as zero", "[impexp2][super]") {
    pugi::xml_document d;
    d.load_string("<r startCentisecond=\"abc\" endCentisecond=\"\"/>");
    SuperStar::AdjustAllTimings(d.document_element(), 5);
    CHECK(d.document_element().attribute("startCentisecond").as_int() == 5);
    CHECK(d.document_element().attribute("endCentisecond").as_int() == 5);
}

TEST_CASE("AdjustAllTimings on a deep tree", "[impexp2][super]") {
    std::string s;
    for (int i = 0; i < 500; ++i) s += "<n startCentisecond=\"1\">";
    for (int i = 0; i < 500; ++i) s += "</n>";
    pugi::xml_document d;
    REQUIRE(d.load_string(s.c_str()));
    SuperStar::AdjustAllTimings(d.document_element(), 2);
    int depth = 0;
    for (auto n = d.document_element(); n; n = n.first_child()) {
        CHECK(n.attribute("startCentisecond").as_int() == 3);
        ++depth;
    }
    CHECK(depth == 500);
}

TEST_CASE("Run applies the timing offset in milliseconds", "[impexp2][super]") {
    struct C { int offsetMs; int expectStart; int expectEnd; };
    auto c = GENERATE(values<C>({ { 0, 100, 500 }, { 100, 200, 600 }, { 1000, 1100, 1500 }, { 15, 110, 510 }, { 9, 100, 500 }, { 10, 110, 510 }, { -50, 50, 450 }, { -15, 90, 490 } }));
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    f.opt.timingOffsetMs = c.offsetMs;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 10, 50, RED, Grid(1, 1)) + "</scenes>")));
    auto e = f.Find("On");
    CAPTURE(c.offsetMs);
    CHECK(e.start == c.expectStart);
    CHECK(e.end == c.expectEnd);
}

TEST_CASE("Run timing offset moves flowys morph states and image actions", "[impexp2][super]") {
    Fixture f;
    f.opt.timingOffsetMs = 200;
    f.opt.xSize = 12;
    f.opt.ySize = 50;
    std::string body = "<flowys><flowy flowyType=\"Shockwave\" layer=\"1\" startTime=\"10\" endTime=\"20\" centerX=\"1\" centerY=\"1\"/></flowys>"
                       "<morphs><morph layer=\"2\" acceleration=\"\"><state1 time=\"30\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\"/><state2 time=\"40\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\"/><ramp timeExt=\"5\"/></morph></morphs>"
                       "<imageActions><imageAction imageIndex=\"1\" layer=\"3\" startCentisecond=\"50\" endCentisecond=\"60\"/></imageActions>"
                       "<textActions><textAction layer=\"4\" startCentisecond=\"70\" endCentisecond=\"80\" text=\"a\"/></textActions>";
    REQUIRE(f.RunRaw(Doc(Layout(), body)));
    CHECK(f.Find("Shockwave").start == 300);
    CHECK(f.Find("Shockwave").end == 400);
    CHECK(f.Find("Morph").start == 500);
    CHECK(f.Find("Morph").end == 650);
    CHECK(f.Find("Pictures").start == 700);
    CHECK(f.Find("Pictures").end == 800);
    CHECK(f.Find("Text").start == 900);
    CHECK(f.Find("Text").end == 1000);
}

// ---------------------------------------------------------------- Run: structure / errors

TEST_CASE("Run on empty documents succeeds with no effects", "[impexp2][super]") {
    Fixture f;
    pugi::xml_document d;
    CHECK(f.RunDoc(d));
    CHECK(f.Count() == 0);
    CHECK(f.RunRaw(""));
    CHECK(f.RunRaw("<SuperStarMetaFile/>"));
    CHECK(f.RunRaw("not xml at all"));
    CHECK(f.RunRaw("<<<<>>>>"));
    CHECK(f.Count() == 0);
}

TEST_CASE("Run with a timing offset on an empty document is harmless", "[impexp2][super]") {
    Fixture f;
    f.opt.timingOffsetMs = 500;
    pugi::xml_document d;
    CHECK(f.RunDoc(d));
}

TEST_CASE("Run reports a missing layouts section only when morphs exist", "[impexp2][super]") {
    Fixture f;
    CHECK_FALSE(f.RunRaw(Doc("", "<morphs><morph layer=\"1\"/></morphs>")));
    CHECK(f.err == "The layouts section was not found in the SuperStar file!");
    CHECK(f.Count() == 0);
    Fixture g;
    CHECK(g.RunRaw(Doc("", "<morphs/>")) == false);
    CHECK(g.err.find("layouts") != std::string::npos);
}

TEST_CASE("Run without an error string pointer does not crash on fatal failure", "[impexp2][super]") {
    Fixture f;
    pugi::xml_document d;
    d.load_string("<r><morphs><morph/></morphs></r>");
    SuperStar::Importer imp(f.el, &f.Media(), f.opt);
    CHECK_FALSE(imp.Run(d));
}

TEST_CASE("Run without layouts still imports scenes on a 1x1 grid", "[impexp2][super]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc("", "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>")));
    REQUIRE(f.Count() == 1);
    CHECK(f.Effects()[0].name == "On");
}

TEST_CASE("Run clears queued state between runs of the same importer", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    pugi::xml_document d;
    d.load_string(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>").c_str());
    SuperStar::Importer imp(f.el, &f.Media(), f.opt);
    REQUIRE(imp.Run(d));
    REQUIRE(imp.Run(d));
    CHECK(f.Count() == 2);
}

TEST_CASE("Run ignores unknown sections and unknown children", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<customColors color1=\"1\"/><timingMap><variables/></timingMap><macros/><bogus><scene/></bogus><scenes><notascene/><text/>" +
                                       Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes><flowys><x/></flowys><textActions><y/></textActions><imageActions><z/></imageActions>")));
    CHECK(f.Count() == 1);
}

// ---------------------------------------------------------------- layout / coordinate mapping

TEST_CASE("Flowy centre maps through the option grid size", "[impexp2][super]") {
    struct C { int xs; int ys; int cx; int cy; const char* ex; const char* ey; };
    auto c = GENERATE(values<C>({
        { 12, 50, 0, 0, "4", "99" },
        { 12, 50, 11, 49, "95", "1" },
        { 12, 50, 6, 25, "54", "49" },
        { 12, 50, 5, 24, "45", "51" },
        { 12, 50, 12, 50, "4", "99" },   // wraps modulo the grid
        { 12, 50, 25, 100, "12", "99" }, // 25 % 12 == 1; 100 % 50 == 0
        { 1, 1, 0, 0, "50", "50" },
        { 2, 2, 1, 1, "75", "25" },
        { 100, 100, 0, 0, "0", "99" },
    }));
    Fixture f;
    f.opt.xSize = c.xs;
    f.opt.ySize = c.ys;
    REQUIRE(f.RunRaw(Doc(Layout(), Shock(c.cx, c.cy))));
    auto e = f.Find("Shockwave");
    CAPTURE(c.xs, c.ys, c.cx, c.cy);
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_Shockwave_CenterX") == c.ex);
    CHECK(e.Get("E_SLIDER_Shockwave_CenterY") == c.ey);
}

TEST_CASE("Flowy centre honours x and y offsets", "[impexp2][super]") {
    Fixture f;
    f.opt.xOffset = 2;
    f.opt.yOffset = 10;
    REQUIRE(f.RunRaw(Doc(Layout(), Shock(2, 10))));
    auto e = f.Find("Shockwave");
    CHECK(e.Get("E_SLIDER_Shockwave_CenterX") == "4");
    CHECK(e.Get("E_SLIDER_Shockwave_CenterY") == "99");
}

TEST_CASE("Flowy with a centre left of the offset is dropped", "[impexp2][super]") {
    Fixture f;
    f.opt.xOffset = 5;
    REQUIRE(f.RunRaw(Doc(Layout(), Shock(4, 1))));
    CHECK(f.Count() == 0);
    Fixture g;
    REQUIRE(g.RunRaw(Doc(Layout(), Shock(-1, 1))));
    CHECK(g.Count() == 0);
    Fixture h;
    REQUIRE(h.RunRaw(Doc(Layout(), Shock(1, -3))));
    CHECK(h.Count() == 0);
}

TEST_CASE("Horizontal ribbons swap the grid and the centre axes", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = 12;
    f.opt.ySize = 50;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), Shock(11, 49))));
    auto e = f.Find("Shockwave");
    REQUIRE(e.layer >= 0);
    // columns are the 50 option rows, rows are the 12 option columns, no row reversal
    CHECK(e.Get("E_SLIDER_Shockwave_CenterX") == "99");
    CHECK(e.Get("E_SLIDER_Shockwave_CenterY") == "95");
}

TEST_CASE("Non visualization layouts take the grid from the ribbon count", "[impexp2][super]") {
    struct C { const char* extra; const char* orient; int cx; int cy; const char* ex; const char* ey; };
    auto c = GENERATE(values<C>({
        { "nbrOfRibbons=\"4\" ribbonLength=\"full\"", "vertical", 3, 49, "87", "1" },
        { "nbrOfRibbons=\"4\" ribbonLength=\"half\"", "vertical", 7, 24, "93", "2" },
        { "nbrOfRibbons=\"4\"", "vertical", 0, 0, "12", "99" },
    }));
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(c.orient, "false", c.extra), Shock(c.cx, c.cy))));
    auto e = f.Find("Shockwave");
    CAPTURE(c.extra);
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_Shockwave_CenterX") == c.ex);
    CHECK(e.Get("E_SLIDER_Shockwave_CenterY") == c.ey);
}

TEST_CASE("Zero sized grid does not divide by zero", "[impexp2][super]") {
    // CalcPercentage: val %= static_cast<int>(base) with base == 0 (xSize 0 or nbrOfRibbons missing) is SIGFPE
    Fixture f;
    f.opt.xSize = 0;
    CHECK(f.RunRaw(Doc(Layout(), Shock(1, 1))));
}

TEST_CASE("Missing ribbon count in a non visualization layout does not divide by zero", "[impexp2][super]") {
    // layouts with visualizationMode=false and no nbrOfRibbons gives num_columns == 0
    Fixture f;
    CHECK(f.RunRaw(Doc(Layout("vertical", "false"), Shock(1, 1))));
}

TEST_CASE("A layouts section with no layout child uses option sizes", "[impexp2][super]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc("<layouts/>", Shock(0, 0))));
    auto e = f.Find("Shockwave");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_Shockwave_CenterX") == "4");
}

// ---------------------------------------------------------------- layer blend

TEST_CASE("Layer blend option becomes T_CHOICE_LayerMethod", "[impexp2][super]") {
    struct C { const char* blend; const char* expect; };
    auto c = GENERATE(values<C>({ { "2 reveals 1", "2 reveals 1" }, { "Average", "Average" }, { "Normal", "<none>" }, { "", "<none>" }, { "1 is Mask", "1 is Mask" } }));
    Fixture f;
    f.opt.layerBlend = c.blend;
    REQUIRE(f.RunRaw(Doc(Layout(), Shock(1, 1))));
    CAPTURE(c.blend);
    CHECK(f.Find("Shockwave").Get("T_CHOICE_LayerMethod") == c.expect);
}

// ---------------------------------------------------------------- image prefix

TEST_CASE("Image prefix comes from the option without prompting", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    f.opt.imageGroupPrefix = "Mine";
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(7, 1, 0, 10, RED, Elem(0, 0) ) + "</scenes>")));
    CHECK(f.prompts.empty());
}

TEST_CASE("Prefix callback result is trimmed and used", "[impexp2][super]") {
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.opt.xSize = 2;
    f.opt.ySize = 2;
    f.promptAnswer = " \t My Group \r\n";
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(3, 1, 0, 10, RED, Elem(0, 0) + Elem(1, 1)) + "</scenes>")));
    REQUIRE(f.prompts.size() == 1);
    CHECK(f.Media().HasMedia("My Group/s3.png"));
}

TEST_CASE("Prefix callback is only called once per import", "[impexp2][super]") {
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.opt.xSize = f.opt.ySize = 2;
    f.promptAnswer = "G";
    std::string body = "<images><image width=\"1\" height=\"1\" savedIndex=\"1\"><pixe s=\"100\"/></image></images><scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) +
                       Scene(2, 1, 20, 30, RED, Elem(1, 1)) + "</scenes>";
    REQUIRE(f.RunRaw(Doc(Layout(), body)));
    CHECK(f.prompts.size() == 1);
}

TEST_CASE("Blank prefix answers fall back to the default name", "[impexp2][super]") {
    struct C { const char* answer; const char* defName; const char* expect; };
    auto c = GENERATE(values<C>({ { "", "", "SuperStar" }, { "   ", "", "SuperStar" }, { "", "Fallback", "Fallback" }, { "\t\n", "Fallback", "Fallback" } }));
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.opt.defaultGroupName = c.defName;
    f.opt.xSize = f.opt.ySize = 2;
    f.promptAnswer = c.answer;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(5, 1, 0, 10, RED, Elem(0, 0) + Elem(1, 1)) + "</scenes>")));
    CHECK(f.Media().HasMedia(std::string(c.expect) + "/s5.png"));
}

TEST_CASE("Cancelling the prefix prompt aborts with no effects", "[impexp2][super]") {
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.promptCancel = true;
    CHECK_FALSE(f.RunRaw(Doc(Layout(), Shock(1, 1) + "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.Count() == 0);
    CHECK(f.prompts.size() == 1);
}

TEST_CASE("Cancelling the prefix prompt for images aborts", "[impexp2][super]") {
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.promptCancel = true;
    CHECK_FALSE(f.RunRaw(Doc(Layout(), "<images><image width=\"1\" height=\"1\" savedIndex=\"1\"><pixe s=\"100\"/></image></images>")));
}

TEST_CASE("No prompt without images or scenes", "[impexp2][super]") {
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    REQUIRE(f.RunRaw(Doc(Layout(), Shock(1, 1))));
    CHECK(f.prompts.empty());
    Fixture g;
    g.opt.imageGroupPrefix.clear();
    REQUIRE(g.RunRaw(Doc(Layout(), "<images/><scenes/>")));
    CHECK(g.prompts.empty());
}

TEST_CASE("Without a callback an empty prefix uses SuperStar", "[impexp2][super]") {
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.opt.xSize = f.opt.ySize = 2;
    pugi::xml_document d;
    d.load_string(Doc(Layout(), "<scenes>" + Scene(2, 1, 0, 10, RED, Elem(0, 0) + Elem(1, 1)) + "</scenes>").c_str());
    SuperStar::Importer imp(f.el, &f.Media(), f.opt);
    REQUIRE(imp.Run(d));
    CHECK(f.Media().HasMedia("SuperStar/s2.png"));
}

TEST_CASE("Prefix prompt receives the suggested default group name", "[impexp2][super]") {
    // PromptForPrefix: callback gets an empty string even though Options::defaultGroupName says it is the suggestion
    Fixture f;
    f.opt.imageGroupPrefix.clear();
    f.opt.defaultGroupName = "Suggested";
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>")));
    REQUIRE(f.prompts.size() == 1);
    CHECK(f.prompts[0] == "Suggested");
}

TEST_CASE("Stale unreferenced images under the prefix are dropped", "[impexp2][super]") {
    Fixture f;
    f.Media().AddEmbeddedImage("P/old.png", xlImage(2, 2));
    f.Media().AddEmbeddedImage("Other/keep.png", xlImage(2, 2));
    f.opt.xSize = f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>")));
    CHECK_FALSE(f.Media().HasMedia("P/old.png"));
    CHECK(f.Media().HasMedia("Other/keep.png"));
}

TEST_CASE("Prefix with images still in use is renamed with a numeric suffix", "[impexp2][super]") {
    Fixture f;
    f.Media().AddEmbeddedImage("P/old.png", xlImage(2, 2));
    auto* L = f.el->AddEffectLayer();
    L->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=P/old.png", "", 0, 1000, false, false);
    f.opt.xSize = f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0) + Elem(1, 1)) + "</scenes>")));
    CHECK(f.Media().HasMedia("P/old.png"));
    CHECK(f.Media().HasMedia("P-2/s1.png"));
}

TEST_CASE("Prefix skips every numbered group already in use", "[impexp2][super]") {
    Fixture f;
    auto* L = f.el->AddEffectLayer();
    int t = 0;
    for (std::string p : { "P/a.png", "P-2/a.png", "P-3/a.png" }) {
        f.Media().AddEmbeddedImage(p, xlImage(2, 2));
        L->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=" + p, "", t, t + 10, false, false);
        t += 20;
    }
    f.opt.xSize = f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0) + Elem(1, 1)) + "</scenes>")));
    CHECK(f.Media().HasMedia("P-4/s1.png"));
}

TEST_CASE("Prefix that is a prefix of another group name is not confused with it", "[impexp2][super]") {
    Fixture f;
    f.Media().AddEmbeddedImage("PP/x.png", xlImage(2, 2));
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.Media().HasMedia("PP/x.png"));
}

TEST_CASE("Null media pointer skips image storage but still imports", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 2;
    pugi::xml_document d;
    d.load_string(Doc(Layout(), "<images><image width=\"1\" height=\"1\" savedIndex=\"4\"><pixe s=\"100\"/></image></images><scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + "</scenes>").c_str());
    SuperStar::Importer imp(f.el, nullptr, f.opt);
    REQUIRE(imp.Run(d));
    CHECK(f.Count() == 1);
    CHECK(f.Media().GetImageCount() == 0);
}

// ---------------------------------------------------------------- placement of queued effects

TEST_CASE("Overlapping effects on different SuperStar layers get stacked rows", "[impexp2][super]") {
    struct C { const char* blend; int row1; int row2; };
    auto c = GENERATE(values<C>({ { "2 reveals 1", 0, 1 }, { "Normal", 1, 0 }, { "Average", 1, 0 } }));
    Fixture f;
    f.opt.layerBlend = c.blend;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0)) + Scene(2, 2, 50, 150, RED_TO_BLACK, Elem(0, 0)) + "</scenes>")));
    REQUIRE(f.Count() == 2);
    CAPTURE(c.blend);
    CHECK(f.Find("On", 0).layer == c.row1);
    CHECK(f.Find("On", 500).layer == c.row2);
    CHECK(f.el->GetEffectLayerCount() == 2);
}

TEST_CASE("Non overlapping effects on different layers share a row", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 10, RED, Elem(0, 0)) + Scene(2, 5, 10, 20, RED, Elem(0, 0)) + Scene(3, 9, 30, 40, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.Count() == 3);
    CHECK(f.el->GetEffectLayerCount() == 1);
}

TEST_CASE("Overlapping effects on the same layer are pushed to a free row", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 3, 0, 100, RED, Elem(0, 0)) + Scene(2, 3, 50, 150, RED, Elem(0, 0)) + "</scenes>")));
    REQUIRE(f.Count() == 2);
    CHECK(f.el->GetEffectLayerCount() == 2);
    CHECK(f.Find("On", 0).layer != f.Find("On", 500).layer);
}

TEST_CASE("A three deep stack of layers uses three rows", "[impexp2][super]") {
    Fixture f;
    f.opt.layerBlend = "2 reveals 1";
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 2, 0, 100, RED, Elem(0, 0)) + Scene(2, 4, 0, 100, RED, Elem(0, 0)) + Scene(3, 3, 0, 100, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.el->GetEffectLayerCount() == 3);
    std::vector<int> rows;
    for (auto& e : f.Effects()) rows.push_back(e.layer);
    std::sort(rows.begin(), rows.end());
    CHECK(rows == std::vector<int>{ 0, 1, 2 });
}

TEST_CASE("Effects already on the model are left alone and new rows go below", "[impexp2][super]") {
    Fixture f;
    auto* L = f.el->AddEffectLayer();
    L->AddEffect(0, "On", "", "", 0, 5000, false, false);
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.el->GetEffectLayerCount() == 2);
    CHECK(f.el->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(f.el->GetEffectLayer(1)->GetEffectCount() == 1);
}

TEST_CASE("Empty existing layers are reused", "[impexp2][super]") {
    Fixture f;
    f.el->AddEffectLayer();
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.el->GetEffectLayerCount() == 1);
    CHECK(f.Count() == 1);
}

TEST_CASE("Negative and wildly large layer numbers sort without trouble", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, -5, 0, 100, RED, Elem(0, 0)) + Scene(2, 2147483647, 0, 100, RED, Elem(0, 0)) + Scene(3, 0, 0, 100, RED, Elem(0, 0)) + "</scenes>")));
    CHECK(f.Count() == 3);
    CHECK(f.el->GetEffectLayerCount() == 3);
}

TEST_CASE("Zero length effects are placed without crashing", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 50, 50, RED, Elem(0, 0)) + Scene(2, 2, 50, 50, RED, Elem(0, 0)) + "</scenes>")));
    SUCCEED();
}

TEST_CASE("Reversed time ranges are placed without crashing", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 100, 10, RED, Elem(0, 0)) + Scene(2, 2, 100, 10, RED, Elem(0, 0)) + "</scenes>")));
    SUCCEED();
}

// ---------------------------------------------------------------- realistic documents

namespace {
std::string RealisticDoc(int nImages, int nImageActions, int nFlowys, int nMorphs, int nScenes, int elemsPerScene, int& expectedEffects) {
    std::string d = "<SuperStarMetaFile saveFileVersion=\"7\" audioCentisecondLength=\"18209\" sequenceCentisecondLength=\"18209\" musicFilename=\"C:\\x\\y.mp3\">\r\n";
    d += "\t<configurations>\r\n";
    for (int i = 0; i < 9; ++i) d += "\t\t<configuration name=\"c" + std::to_string(i) + "\" v=\"1\">\r\n\t\t</configuration>\r\n";
    d += "\t</configurations>\r\n\t<layouts>\r\n\t\t<layout name=\"L\" visualizationMode=\"true\" ribbonOrientation=\"vertical\" nbrOfRibbons=\"12\"/>\r\n\t</layouts>\r\n";
    d += "\t<customColors color1=\"255\" color2=\"65280\"/>\r\n\t<timingMap>\r\n\t\t<variables a=\"1\"/>\r\n";
    for (int i = 0; i < 24; ++i) d += "\t\t<ribbonTimingAssignment ribbon=\"" + std::to_string(i) + "\" timingFreqs=\"0,1,1,0,0,1\"/>\r\n";
    d += "\t</timingMap>\r\n\t<instantSequence>\r\n\t\t<variables/>\r\n\t\t<ribbonTcmAssignments/>\r\n\t\t<tcmSettings/><tcmSettings/><tcmSettings/><tcmSettings/>\r\n\t</instantSequence>\r\n\t<macros/>\r\n";
    d += "\t<images>\r\n";
    for (int i = 0; i < nImages; ++i) {
        d += "\t\t<image name=\"img" + std::to_string(i) + "\" height=\"5\" width=\"4\" xOffset=\"" + std::to_string(i % 7 - 3) + "\" yOffset=\"" + std::to_string(i % 9 - 4) + "\" savedIndex=\"" + std::to_string(i) + "\">\r\n\t\t\t<pixels=\"";
        for (int p = 0; p < 20; ++p) d += (p ? "," : "") + std::to_string((p * 3) % 5 ? 25600 : 6579300);
        d += "\"/>\r\n\t\t</image>\r\n";
    }
    d += "\t</images>\r\n\t<imageActions>\r\n";
    for (int i = 0; i < nImageActions; ++i) {
        d += "\t\t<imageAction name=\"a\" colorType=\"x\" maskType=\"y\" direction=\"8\" layer=\"" + std::to_string(2 + i % 32) + "\" savedIndex=\"" + std::to_string(i) +
             "\" imageIndex=\"" + std::to_string(nImages ? i % nImages : 0) + "\" startCentisecond=\"" + std::to_string(i * 5) + "\" endCentisecond=\"" + std::to_string(i * 5 + 40) +
             "\" xStart=\"" + std::to_string(i % 49 - 28) + "\" xEnd=\"" + std::to_string(i % 13 - 5) + "\" yStart=\"" + std::to_string(i % 20 - 10) + "\" yEnd=\"" + std::to_string(i % 20 - 10) +
             "\" preRampTime=\"0\" rampTime=\"" + std::to_string(i % 4) + "\" fadeFromBright=\"0\" fadeToBright=\"0\" rotation=\"0\" stopAtEdge=\"0\" transparency=\"0\">\r\n";
    }
    d += "\t</imageActions>\r\n\t<textActions/>\r\n\t<flowys>\r\n";
    expectedEffects = nImageActions;
    for (int i = 0; i < nFlowys; ++i) {
        bool fan = i % 2;
        d += std::string("\t\t<flowy flowyType=\"") + (fan ? "Fan" : "Shockwave") + "\" layer=\"" + std::to_string(2 + i % 30) + "\" savedIndex=\"" + std::to_string(i) +
             "\" startTime=\"" + std::to_string(i * 7) + "\" endTime=\"" + std::to_string(i * 7 + 30) + "\" centerX=\"" + std::to_string(i % 12) + "\" centerY=\"" + std::to_string(i % 50) +
             "\" startAngle=\"0\" endAngle=\"" + std::to_string(i % 360) + "\" startRadius=\"1\" endRadius=\"20\" acceleration=\"0\" transparency=\"0\" blendColors=\"0\" Colors=\"100,0,0 0,100,0\"" +
             (fan ? " blades=\"3\" width=\"10\" colorsSize=\"2\" elementAngle=\"20\" elementStepAngle=\"10\" endDotSize=\"3\" startDotSize=\"1\" revolutionsPerSecond=\"2\" showEntireBladeWidthAtStart=\"0\"" : " headWidth=\"4\" tailWidth=\"2\"") + "/>\r\n";
        ++expectedEffects;
    }
    d += "\t</flowys>\r\n\t<morphs>\r\n";
    for (int i = 0; i < nMorphs; ++i) {
        bool neg = i % 100 == 99;
        d += "\t\t<morph name=\"\" layer=\"" + std::to_string(2 + i % 31) + "\" savedIndex=\"" + std::to_string(i) + "\" acceleration=\"0\">\r\n\t\t\t<state1 time=\"" + std::to_string(i * 3) +
             "\" x1=\"" + (neg ? "-3" : std::to_string(i % 12)) + "\" y1=\"1\" x2=\"2\" y2=\"3\" red=\"100\" green=\"0\" blue=\"0\" white=\"0\" trailLen=\"2\"/>\r\n\t\t\t<state2 time=\"" + std::to_string(i * 3 + 20) +
             "\" x1=\"3\" y1=\"4\" x2=\"5\" y2=\"6\" red=\"0\" green=\"100\" blue=\"0\" white=\"0\" trailLen=\"2\"/>\r\n\t\t\t<ramp effectType=\"0\" red1=\"0\" green1=\"0\" blue1=\"100\" white1=\"0\" red2=\"100\" green2=\"100\" blue2=\"0\" white2=\"0\" timeExt=\"5\"/>\r\n\t\t</morph>\r\n";
        if (!neg) ++expectedEffects;
    }
    d += "\t</morphs>\r\n\t<scenes>\r\n";
    for (int i = 0; i < nScenes; ++i) {
        const char* type = i % 3 == 0 ? "intensity" : (i % 3 == 1 ? "shimmer" : "twinkle");
        d += std::string("\t\t<scene name=\"\" type=\"") + type + "\" layer=\"" + std::to_string(2 + i % 31) + "\" savedIndex=\"" + std::to_string(i) + "\" startCentisecond=\"" + std::to_string(i * 11) +
             "\" endCentisecond=\"" + std::to_string(i * 11 + 30) + "\" red1=\"100\" green1=\"0\" blue1=\"0\" white1=\"0\" red2=\"100\" green2=\"0\" blue2=\"0\" white2=\"0\" transparency=\"0\" shimmerSpeed=\"1\" twinkleSpeed=\"1\">\r\n";
        for (int e = 0; e < elemsPerScene; ++e) d += "\t\t\t<element ribbonIndex=\"" + std::to_string((i + e) % 12) + "\" pixelIndex=\"" + std::to_string((i * 7 + e * 3) % 50) + "\"/>\r\n";
        d += "\t\t</scene>\r\n";
        ++expectedEffects;
    }
    d += "\t</scenes>\r\n\t<timingChannels>\r\n\t\t<timingChannel name=\"t\" saveID=\"0\">\r\n";
    for (int i = 0; i < 489; ++i) d += "\t\t\t<timing startCentisecond=\"" + std::to_string(i) + "\" endCentisecond=\"" + std::to_string(i + 1) + "\"/>\r\n";
    d += "\t\t</timingChannel>\r\n\t</timingChannels>\r\n\t<timingGrids>\r\n\t\t<timingGrid name=\"g\" saveID=\"0\" type=\"freeform\">\r\n";
    for (int i = 0; i < 470; ++i) d += "\t\t\t<timing centisecond=\"" + std::to_string(i) + "\"/>\r\n";
    d += "\t\t</timingGrid>\r\n\t</timingGrids>\r\n</SuperStarMetaFile>\r\n";
    return d;
}
}

TEST_CASE("Small realistic document imports every effect type", "[impexp2][super]") {
    int expect = 0;
    Fixture f;
    f.opt.layerBlend = "2 reveals 1";
    auto doc = RealisticDoc(3, 6, 4, 5, 6, 3, expect);
    REQUIRE(f.RunRaw(doc));
    CHECK(f.Count() == (size_t)expect);
    CHECK(f.Find("Pictures").layer >= 0);
    CHECK(f.Find("Fan").layer >= 0);
    CHECK(f.Find("Shockwave").layer >= 0);
    CHECK(f.Find("Morph").layer >= 0);
    CHECK(f.Media().HasMedia("P/0.png"));
    CHECK(f.Media().HasMedia("P/2.png"));
}

TEST_CASE("Full sized realistic document (counts from a real file) imports", "[impexp2][super]") {
    int expect = 0;
    Fixture f;
    auto doc = RealisticDoc(135, 2068, 143, 1101, 719, 5, expect);
    REQUIRE(f.RunRaw(doc));
    CHECK(f.Count() == (size_t)expect);
    CHECK(f.Media().GetImageCount() >= 135);
    CHECK(f.el->GetEffectLayerCount() > 1);
}

TEST_CASE("Full sized realistic document with an offset and a different grid", "[impexp2][super]") {
    int expect = 0;
    Fixture f;
    f.opt.timingOffsetMs = 300;
    f.opt.imageResize = ImageResize::ExactWidthOrHeight;
    f.opt.modelWidth = 24;
    f.opt.modelHeight = 100;
    auto doc = RealisticDoc(20, 300, 30, 100, 100, 10, expect);
    REQUIRE(f.RunRaw(doc));
    CHECK(f.Count() == (size_t)expect);
}

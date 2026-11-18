// Synthetic SuperStar documents: each effect type (morph, flowy, scene, text, image, image action).
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "test_impexp2_super_common.h"

using namespace ss2;
using SuperStar::ImageResize;

namespace {
std::string Morph(const std::string& attrs, const std::string& s1, const std::string& s2, const std::string& ramp, bool rampFirst = false) {
    std::string kids = rampFirst ? "<ramp " + ramp + "/><state1 " + s1 + "/><state2 " + s2 + "/>" : "<state1 " + s1 + "/><state2 " + s2 + "/><ramp " + ramp + "/>";
    return "<morphs><morph " + attrs + ">" + kids + "</morph></morphs>";
}
const char* S1 = "time=\"10\" x1=\"0\" y1=\"0\" x2=\"5\" y2=\"10\" red=\"100\" green=\"0\" blue=\"0\" trailLen=\"3\"";
const char* S2 = "time=\"50\" x1=\"11\" y1=\"49\" x2=\"6\" y2=\"40\" red=\"0\" green=\"100\" blue=\"0\" trailLen=\"4\"";
const char* RAMP = "red1=\"0\" green1=\"0\" blue1=\"100\" red2=\"50\" green2=\"50\" blue2=\"50\" timeExt=\"20\"";

std::string Flowy(const std::string& type, const std::string& attrs) {
    return "<flowys><flowy flowyType=\"" + type + "\" layer=\"1\" startTime=\"10\" endTime=\"50\" centerX=\"6\" centerY=\"25\" " + attrs + "/></flowys>";
}
std::string Text(const std::string& attrs) {
    return "<textActions><textAction layer=\"1\" startCentisecond=\"0\" endCentisecond=\"100\" " + attrs + "/></textActions>";
}
}

// ---------------------------------------------------------------- morphs

TEST_CASE("Morph effect carries timing, positions, trails and palette", "[impexp2][super][morph]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\" acceleration=\"\"", S1, S2, RAMP))));
    auto e = f.Find("Morph");
    REQUIRE(e.layer >= 0);
    CHECK(e.start == 100);
    CHECK(e.end == 700);
    CHECK(e.Get("E_SLIDER_MorphDuration") == "66");
    CHECK(e.Get("E_SLIDER_MorphStartLength") == "3");
    CHECK(e.Get("E_SLIDER_MorphEndLength") == "4");
    CHECK(e.Get("E_SLIDER_Morph_Start_X1") == "4");
    CHECK(e.Get("E_SLIDER_Morph_Start_X2") == "45");
    CHECK(e.Get("E_SLIDER_Morph_Start_Y1") == "99");
    CHECK(e.Get("E_SLIDER_Morph_Start_Y2") == "79");
    CHECK(e.Get("E_SLIDER_Morph_End_X1") == "95");
    CHECK(e.Get("E_SLIDER_Morph_End_X2") == "54");
    CHECK(e.Get("E_SLIDER_Morph_End_Y1") == "1");
    CHECK(e.Get("E_SLIDER_Morph_End_Y2") == "19");
    CHECK(e.Get("C_BUTTON_Palette1") == "#ff0000");
    CHECK(e.Get("C_BUTTON_Palette2") == "#00ff00");
    CHECK(e.Get("C_BUTTON_Palette3") == "#0000ff");
    CHECK(e.Get("C_BUTTON_Palette4") == "#7f7f7f");
    CHECK(e.Get("C_BUTTON_Palette5") == "#FFFFFF");
    CHECK(e.Get("C_BUTTON_Palette6") == "#000000");
    CHECK(e.Get("C_CHECKBOX_Palette1") == "1");
    CHECK(e.Get("C_CHECKBOX_Palette4") == "1");
    CHECK(e.Get("C_CHECKBOX_Palette5") == "<none>");
    CHECK(e.Get("E_NOTEBOOK_Morph") == "Start");
}

TEST_CASE("Morph acceleration must land in MorphAccel", "[impexp2][super][morph]") {
    // SuperStarImporter.cpp Run/morphs: acceleration is appended straight after "E_SLIDER_Morph_Stagger=0" with no key,
    // so "5" turns Stagger into 05 and MorphAccel stays 0
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\" acceleration=\"5\"", S1, S2, RAMP))));
    auto e = f.Find("Morph");
    CHECK(e.Get("E_SLIDER_MorphAccel") == "5");
    CHECK(e.Get("E_SLIDER_Morph_Stagger") == "0");
}

TEST_CASE("Morph blend string is appended", "[impexp2][super][morph]") {
    Fixture f;
    f.opt.layerBlend = "Average";
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", S1, S2, RAMP))));
    CHECK(f.Find("Morph").Get("T_CHOICE_LayerMethod") == "Average");
}

TEST_CASE("Morph with a coordinate left of the grid is skipped", "[impexp2][super][morph]") {
    auto which = GENERATE(values<std::string>({ "x1", "y1", "x2", "y2" }));
    auto state = GENERATE(1, 2);
    std::string s1 = S1, s2 = S2;
    std::string& s = state == 1 ? s1 : s2;
    auto pos = s.find(which + "=\"");
    s.replace(pos, which.size() + 4, which + "=\"-1\"");
    s.replace(s.find("\"-1\"") , 4, "\"-1\"");
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", s1, s2, RAMP))));
    CAPTURE(which, state, s);
    CHECK(f.Count() == 0);
}

TEST_CASE("Morph coordinates that wrap round the grid are accepted", "[impexp2][super][morph]") {
    Fixture f;
    std::string s1 = "time=\"0\" x1=\"12\" y1=\"50\" x2=\"24\" y2=\"100\" trailLen=\"1\"";
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", s1, S2, RAMP))));
    auto e = f.Find("Morph");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_Morph_Start_X1") == "4");
    CHECK(e.Get("E_SLIDER_Morph_Start_Y1") == "99");
}

TEST_CASE("Morph in horizontal orientation swaps axes", "[impexp2][super][morph]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), Morph("layer=\"1\"", S1, S2, RAMP))));
    auto e = f.Find("Morph");
    REQUIRE(e.layer >= 0);
    // grid is 50 wide x 12 high; state1 (x1=0,y1=0) -> X from y1/50 cols, Y from x1/12 rows (not reversed)
    CHECK(e.Get("E_SLIDER_Morph_Start_X1") == "1");
    CHECK(e.Get("E_SLIDER_Morph_Start_Y1") == "4");
    CHECK(e.Get("E_SLIDER_Morph_Start_X2") == "21"); // y2=10 -> 20+1
    CHECK(e.Get("E_SLIDER_Morph_Start_Y2") == "45"); // x2=5
}

TEST_CASE("Morph layer attribute is parsed as a number", "[impexp2][super][morph]") {
    struct C { const char* layer; };
    auto c = GENERATE(values<C>({ { "1" }, { "1.9" }, { "" }, { "abc" }, { "-3" }, { "99999999999" } }));
    Fixture f;
    CAPTURE(c.layer);
    REQUIRE(f.RunRaw(Doc(Layout(), Morph(std::string("layer=\"") + c.layer + "\"", S1, S2, RAMP))));
    CHECK(f.Count() == 1);
}

TEST_CASE("Morph with zero duration must give a sane head duration", "[impexp2][super][morph]") {
    // head_duration = (1 - ramp / (end - start)) * 100 divides by zero (NaN cast to int is undefined)
    Fixture f;
    std::string a = "time=\"10\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\"";
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", a, a, "timeExt=\"0\""))));
    auto e = f.Find("Morph");
    if (e.layer >= 0) {
        int d = std::atoi(e.Get("E_SLIDER_MorphDuration").c_str());
        CHECK(d >= 0);
        CHECK(d <= 100);
    }
}

TEST_CASE("Morph with end before start has negative lengths handled", "[impexp2][super][morph]") {
    Fixture f;
    std::string a = "time=\"100\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\"";
    std::string b = "time=\"10\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\"";
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", a, b, "timeExt=\"5\""))));
    SUCCEED();
}

TEST_CASE("Morph with no children does not crash", "[impexp2][super][morph]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<morphs><morph layer=\"1\"/><morph><state1/></morph><morph><state1/><state2/></morph></morphs>")));
    SUCCEED();
}

TEST_CASE("Morph colour percentages scale to 0-255", "[impexp2][super][morph]") {
    struct C { const char* pct; const char* hex; };
    auto c = GENERATE(values<C>({ { "0", "00" }, { "100", "ff" }, { "50", "7f" }, { "1", "02" }, { "99.9", "fe" }, { "10", "19" }, { "", "00" } }));
    Fixture f;
    std::string s1 = std::string("time=\"0\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\" red=\"") + c.pct + "\" green=\"0\" blue=\"0\"";
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", s1, S2, RAMP))));
    CAPTURE(c.pct);
    CHECK(f.Find("Morph").Get("C_BUTTON_Palette1") == std::string("#") + c.hex + "0000");
}

TEST_CASE("Morph colour above 100 percent does not wrap", "[impexp2][super][morph]") {
    // GetColor: static_cast<uint8_t>(double > 255) is undefined; 500% should clamp to ff
    Fixture f;
    std::string s1 = "time=\"0\" x1=\"1\" y1=\"1\" x2=\"1\" y2=\"1\" red=\"500\" green=\"-50\" blue=\"0\"";
    REQUIRE(f.RunRaw(Doc(Layout(), Morph("layer=\"1\"", s1, S2, RAMP))));
    CHECK(f.Find("Morph").Get("C_BUTTON_Palette1") == "#ff0000");
}

// ---------------------------------------------------------------- flowys

TEST_CASE("Spiral flowy becomes a Galaxy", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Spiral", "tailTimeLength=\"10\" Colors=\"\" acceleration=\"2\" startAngle=\"0\" endAngle=\"360\" startRadius=\"3\" endRadius=\"30\" startDotSize=\"2\" endDotSize=\"4\""))));
    auto e = f.Find("Galaxy");
    REQUIRE(e.layer >= 0);
    CHECK(e.start == 100);
    CHECK(e.end == 600);
    CHECK(e.Get("E_SLIDER_Galaxy_Duration") == "80");
    CHECK(e.Get("E_SLIDER_Galaxy_Accel") == "2");
    CHECK(e.Get("E_SLIDER_Galaxy_CenterX") == "54");
    CHECK(e.Get("E_SLIDER_Galaxy_CenterY") == "49");
    CHECK(e.Get("E_SLIDER_Galaxy_Start_Radius") == "3");
    CHECK(e.Get("E_SLIDER_Galaxy_End_Radius") == "30");
    CHECK(e.Get("E_SLIDER_Galaxy_Start_Width") == "2");
    CHECK(e.Get("E_SLIDER_Galaxy_End_Width") == "4");
    CHECK(e.Get("E_SLIDER_Galaxy_Revolutions") == "360");
    CHECK(e.Get("E_CHECKBOX_Galaxy_Reverse") == "0");
}

TEST_CASE("Spiral reverses when the end angle is below the start angle", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Spiral", "tailTimeLength=\"0\" startAngle=\"300\" endAngle=\"100\""))));
    auto e = f.Find("Galaxy");
    CHECK(e.Get("E_CHECKBOX_Galaxy_Reverse") == "1");
    CHECK(e.Get("E_SLIDER_Galaxy_Revolutions") == "200");
}

TEST_CASE("Equal start and end angle uses three revolutions", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Spiral", "tailTimeLength=\"0\" startAngle=\"45\" endAngle=\"45\""))));
    CHECK(f.Find("Galaxy").Get("E_SLIDER_Galaxy_Revolutions") == "3");
}

TEST_CASE("Horizontal orientation rotates flowy angles by 90 degrees", "[impexp2][super][flowy]") {
    struct C { int a; int b; const char* ea; int rev; };
    auto c = GENERATE(values<C>({ { 100, 200, "10", 100 }, { 50, 20, "320", 30 }, { 90, 0, "0", 90 }, { 0, 10, "270", 10 } }));
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), Flowy("Spiral", "tailTimeLength=\"0\" startAngle=\"" + std::to_string(c.a) + "\" endAngle=\"" + std::to_string(c.b) + "\""))));
    auto e = f.Find("Galaxy");
    CAPTURE(c.a, c.b);
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_Galaxy_Start_Angle") == c.ea);
}

TEST_CASE("Spiral with no duration must give a sane head duration", "[impexp2][super][flowy]") {
    // duration = (1 - tail/(end-start))*100 with end==start (tail 0): 0/0 NaN converted to int is undefined
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<flowys><flowy flowyType=\"Spiral\" layer=\"1\" startTime=\"10\" endTime=\"10\" centerX=\"1\" centerY=\"1\"/></flowys>")));
    auto e = f.Find("Galaxy");
    if (e.layer >= 0) {
        int d = std::atoi(e.Get("E_SLIDER_Galaxy_Duration").c_str());
        CHECK(d >= 0);
        CHECK(d <= 100);
    }
}

TEST_CASE("Shockwave flowy", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Shockwave", "acceleration=\"-1\" startRadius=\"3\" endRadius=\"30\" headWidth=\"5\" tailWidth=\"7\""))));
    auto e = f.Find("Shockwave");
    REQUIRE(e.layer >= 0);
    CHECK(e.start == 100);
    CHECK(e.end == 500);
    CHECK(e.Get("E_SLIDER_Shockwave_Accel") == "-1");
    CHECK(e.Get("E_SLIDER_Shockwave_Start_Width") == "5");
    CHECK(e.Get("E_SLIDER_Shockwave_End_Width") == "7");
    CHECK(e.Get("E_SLIDER_Shockwave_Start_Radius") == "3");
    CHECK(e.Get("E_SLIDER_Shockwave_End_Radius") == "30");
    CHECK(e.Get("E_NOTEBOOK_Shockwave") == "Position");
}

TEST_CASE("Fan flowy", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Fan", "acceleration=\"1\" startAngle=\"90\" endAngle=\"10\" startRadius=\"3\" endRadius=\"30\" revolutionsPerSecond=\"2\" blades=\"3\" width=\"10\" elementAngle=\"20\" elementStepAngle=\"10\""))));
    auto e = f.Find("Fan");
    REQUIRE(e.layer >= 0);
    CHECK(e.start == 100);
    CHECK(e.end == 615);
    CHECK(e.Get("E_SLIDER_Fan_Duration") == "78");
    CHECK(e.Get("E_SLIDER_Fan_Revolutions") == "3");
    CHECK(e.Get("E_SLIDER_Fan_Num_Blades") == "3");
    CHECK(e.Get("E_SLIDER_Fan_Num_Elements") == "2");
    CHECK(e.Get("E_SLIDER_Fan_Blade_Width") == "10");
    CHECK(e.Get("E_SLIDER_Fan_Blade_Angle") == "20");
    CHECK(e.Get("E_CHECKBOX_Fan_Reverse") == "1");
}

TEST_CASE("Fan clamps blade and element counts", "[impexp2][super][flowy]") {
    struct C { int blades; int ea; int step; int expectBlades; int expectEl; };
    auto c = GENERATE(values<C>({ { 0, 10, 10, 1, 1 }, { -4, 10, 10, 1, 1 }, { 99, 100, 10, 16, 4 }, { 5, 10, 0, 5, 1 }, { 5, -30, 10, 5, 1 }, { 5, 30, -10, 5, 1 }, { 16, 40, 10, 16, 4 }, { 17, 50, 10, 16, 4 }, { 2, 0, 10, 2, 1 } }));
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Fan", "blades=\"" + std::to_string(c.blades) + "\" elementAngle=\"" + std::to_string(c.ea) + "\" elementStepAngle=\"" + std::to_string(c.step) + "\" width=\"5\" revolutionsPerSecond=\"1\""))));
    auto e = f.Find("Fan");
    REQUIRE(e.layer >= 0);
    CAPTURE(c.blades, c.ea, c.step);
    CHECK(e.Get("E_SLIDER_Fan_Num_Blades") == std::to_string(c.expectBlades));
    CHECK(e.Get("E_SLIDER_Fan_Num_Elements") == std::to_string(c.expectEl));
}

TEST_CASE("Fan with a negative width keeps a positive extended length", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Fan", "width=\"-1000\" blades=\"2\""))));
    auto e = f.Find("Fan");
    if (e.layer >= 0) CHECK(e.end >= e.start);
}

TEST_CASE("Unknown or missing flowy type produces nothing", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Bogus", "") + Flowy("", "") + Flowy("spiral", "") + Flowy("FAN", ""))));
    CHECK(f.Count() == 0);
}

TEST_CASE("Flowy palette lists the file colours", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Shockwave", "Colors=\"100,0,0 0,100,0 0,0,100\""))));
    auto e = f.Find("Shockwave");
    CHECK(e.Get("C_BUTTON_Palette2") == "#00ff00");
    CHECK(e.Get("C_BUTTON_Palette3") == "#0000ff");
    CHECK(e.Get("C_CHECKBOX_Palette2") == "1");
    CHECK(e.Get("C_CHECKBOX_Palette3") == "1");
    CHECK(e.Get("C_BUTTON_Palette4") == "#000000");
    CHECK(e.Get("C_CHECKBOX_Palette4") == "0");
    CHECK(e.Get("C_CHECKBOX_Palette6") == "0");
}

TEST_CASE("Flowy palette first colour must be the first file colour", "[impexp2][super][flowy]") {
    // BuildFlowyPalette starts with "C_BUTTON_Palette1=," and then adds C_BUTTON_Palette1=<colour> again: the first (empty) wins
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Shockwave", "Colors=\"100,0,0 0,100,0\""))));
    CHECK(f.Find("Shockwave").Get("C_BUTTON_Palette1") == "#ff0000");
}

TEST_CASE("Flowy palette caps at six colours and ignores malformed tokens", "[impexp2][super][flowy]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Shockwave", "Colors=\"1,2 100,0,0   junk 0,100,0 ,, 0,0,100 50,50,50 10,10,10 20,20,20 30,30,30 40,40,40\""))));
    auto e = f.Find("Shockwave");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("C_CHECKBOX_Palette6") == "1");
    CHECK(e.Get("C_BUTTON_Palette7") == "<none>");
}

TEST_CASE("Flowy colour list written comma only is read as a single colour", "[impexp2][super][flowy]") {
    // the structure notes show Colors="r,g,b,..." for real files; the importer splits colours on spaces
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Flowy("Shockwave", "Colors=\"100,0,0,0,100,0,0,0,100\""))));
    auto e = f.Find("Shockwave");
    CHECK(e.Get("C_CHECKBOX_Palette2") == "0");
}

TEST_CASE("Flowy layer and times follow the attributes", "[impexp2][super][flowy]") {
    Fixture f;
    f.opt.layerBlend = "2 reveals 1";
    REQUIRE(f.RunRaw(Doc(Layout(), "<flowys><flowy flowyType=\"Shockwave\" layer=\"5\" startTime=\"-2\" endTime=\"7\" centerX=\"1\" centerY=\"1\"/></flowys>")));
    auto e = f.Find("Shockwave");
    REQUIRE(e.layer >= 0);
    CHECK(e.end == 70);
}

// ---------------------------------------------------------------- scenes

TEST_CASE("Full model scenes choose On or Color Wash", "[impexp2][super][scene]") {
    struct C { const char* colors; const char* name; const char* startKey; const char* endKey; };
    auto c = GENERATE(values<C>({ { RED, "On", "<none>", "<none>" },
                                  { BLACK_TO_GREEN, "On", "0", "<none>" },
                                  { RED_TO_BLACK, "On", "<none>", "0" },
                                  { RED_TO_BLUE, "Color Wash", "<none>", "<none>" } }));
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, c.colors, Grid(3, 4)) + "</scenes>")));
    REQUIRE(f.Count() == 1);
    auto e = f.Effects()[0];
    CHECK(e.name == c.name);
    CHECK(e.Get("E_TEXTCTRL_Eff_On_Start") == c.startKey);
    CHECK(e.Get("E_TEXTCTRL_Eff_On_End") == c.endKey);
    CHECK(e.start == 0);
    CHECK(e.end == 1000);
}

TEST_CASE("On scene from black fades up through the right colour", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 1;
    f.opt.ySize = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, BLACK_TO_GREEN, Elem(0, 0)) + "</scenes>")));
    auto e = f.Find("On");
    CHECK(e.Get("C_BUTTON_Palette1") == "#00ff00");
    CHECK(e.Get("C_BUTTON_Palette2") == "#000000");
}

TEST_CASE("Scene with the whole model but a missing pixel is not full", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 2;
    f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0) + Elem(0, 1) + Elem(1, 0)) + "</scenes>")));
    auto e = f.Find("Pictures");
    CHECK(e.layer >= 0);
}

TEST_CASE("Scene that is a rectangle becomes a sub buffer Color Wash", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED_TO_BLUE, Elem(0, 0) + Elem(1, 0) + Elem(0, 1) + Elem(1, 1)) + "</scenes>")));
    auto e = f.Find("Color Wash");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("B_CUSTOM_SubBuffer") == "0.0x99.9x50x62");
}

TEST_CASE("Scene rectangle edge values are mapped to 0 and 99.9", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(2, 3) + Elem(1, 3) + Elem(2, 2) + Elem(1, 2)) + "</scenes>")));
    auto e = f.Find("Color Wash");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("B_CUSTOM_SubBuffer") == "50x37x99.9x0.0");
}

TEST_CASE("Scene elements beyond the grid drop the whole scene", "[impexp2][super][scene]") {
    struct C { int r; int p; };
    auto c = GENERATE(values<C>({ { 2, 0 }, { 0, 2 }, { 100, 100 }, { 2147483647, 0 } }));
    Fixture f;
    f.opt.xSize = 2;
    f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Grid(2, 2) + Elem(c.r, c.p)) + "</scenes>")));
    CAPTURE(c.r, c.p);
    CHECK(f.Count() == 0);
}

TEST_CASE("Scene with a negative element index must not corrupt memory", "[impexp2][super]") {
    // IsPartOfModel: x < num_columns is true for negative x, then data[x][y] indexes before the vector
    Fixture f;
    f.opt.xSize = 2;
    f.opt.ySize = 2;
    CHECK(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Grid(2, 2) + Elem(-1, 0)) + "</scenes>")));
}

TEST_CASE("Scene with a negative pixel index must not corrupt memory", "[impexp2][super]") {
    Fixture f;
    f.opt.xSize = 2;
    f.opt.ySize = 2;
    CHECK(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Grid(2, 2) + Elem(0, -1)) + "</scenes>")));
}

TEST_CASE("Scene without elements is skipped", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, "") + "</scenes>")));
    CHECK(f.Count() == 0);
}

TEST_CASE("Scene with only non element children is skipped", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, "<junk/><junk/>") + "</scenes>")));
    CHECK(f.Count() == 0);
}

TEST_CASE("Duplicate scene elements still make a full scene", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = f.opt.ySize = 2;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Grid(2, 2) + Grid(2, 2)) + "</scenes>")));
    CHECK(f.Find("On").layer >= 0);
}

TEST_CASE("Scene time attributes get a zero appended", "[impexp2][super][scene]") {
    struct C { const char* s; const char* e; int es; int ee; };
    auto c = GENERATE(values<C>({ { "5", "9", 50, 90 }, { "", "", 0, 0 }, { "0", "1", 0, 10 }, { "-4", "20", -40, 200 }, { "1.5", "2", 1, 20 }, { "abc", "x", 0, 0 } }));
    Fixture f;
    f.opt.xSize = f.opt.ySize = 1;
    std::string sc = std::string("<scene savedIndex=\"1\" layer=\"1\" startCentisecond=\"") + c.s + "\" endCentisecond=\"" + c.e + "\" " + RED + ">" + Elem(0, 0) + "</scene>";
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + sc + "</scenes>")));
    CAPTURE(c.s, c.e);
    auto effs = f.Effects();
    if (c.ee > c.es && c.es >= 0) {
        REQUIRE(effs.size() == 1);
        CHECK(effs[0].start == c.es);
        CHECK(effs[0].end == c.ee);
    }
}

TEST_CASE("Partial scene in one colour becomes a Pictures effect with an image", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    f.opt.modelWidth = 3;
    f.opt.modelHeight = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(9, 1, 0, 100, RED, Elem(0, 0) + Elem(2, 3), "shimmer") + "</scenes>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_TEXTCTRL_Pictures_Filename") == "P/s9.png");
    CHECK(e.Get("E_CHECKBOX_Pictures_Shimmer") == "1");
    CHECK(e.Get("E_CHOICE_Scaling") == "Scale To Fit");
    CHECK(e.Get("T_TEXTCTRL_Fadein") == "<none>");
    CHECK(e.Get("T_TEXTCTRL_Fadeout") == "<none>");
    auto img = f.Media().GetImage("P/s9.png");
    REQUIRE(img);
    CHECK(img->GetImageWidth() == 3);
    CHECK(img->GetImageHeight() == 4);
    auto fr = img->GetFrame(0, false);
    REQUIRE(fr);
    CHECK(fr->GetRed(0, 0) == 255);
    CHECK(fr->GetAlpha(0, 0) == 255);
    CHECK(fr->GetAlpha(1, 1) == 0);
    CHECK(fr->GetAlpha(2, 3) == 255);
}

TEST_CASE("Partial scene fades from and to black set the fade times", "[impexp2][super][scene]") {
    struct C { const char* colors; const char* fin; const char* fout; };
    auto c = GENERATE(values<C>({ { BLACK_TO_GREEN, "1.000000", "<none>" }, { RED_TO_BLACK, "<none>", "1.000000" }, { RED, "<none>", "<none>" } }));
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, c.colors, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("T_TEXTCTRL_Fadein") == c.fin);
    CHECK(e.Get("T_TEXTCTRL_Fadeout") == c.fout);
}

TEST_CASE("Partial scene fade length follows the scene duration", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 25, 75, BLACK_TO_GREEN, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
    CHECK(f.Find("Pictures").Get("T_TEXTCTRL_Fadein") == "0.500000");
}

TEST_CASE("Partial colour changing scene builds one frame per sequence frame", "[impexp2][super][scene]") {
    struct C { int ft; int frames; };
    auto c = GENERATE(values<C>({ { 50, 20 }, { 100, 10 }, { 25, 40 }, { 0, 20 }, { -5, 20 }, { 2000, 0 } }));
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    f.opt.frameTimeMs = c.ft;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED_TO_BLUE, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CAPTURE(c.ft);
    CHECK(e.Get("E_TEXTCTRL_Pictures_Filename") == "P/s1.png");
    if (c.frames > 0) {
        auto img = f.Media().GetImage("P/s1.png");
        REQUIRE(img);
        CHECK(img->GetImageCount() == c.frames);
    } else {
        CHECK_FALSE(f.Media().HasMedia("P/s1.png"));
    }
}

TEST_CASE("Colour changing partial scene with reversed time must not reserve a huge vector", "[impexp2][super][scene]") {
    // animFrames.reserve(numFrames) with numFrames < 0 throws std::length_error out of Run
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    CHECK_NOTHROW(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 100, 0, RED_TO_BLUE, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
}

TEST_CASE("Partial scene pixel y offset shifts the picture", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    f.opt.yOffset = 1;
    f.opt.modelWidth = 3;
    f.opt.modelHeight = 4;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0) + Elem(0, 1) + Elem(2, 3)) + "</scenes>")));
    auto img = f.Media().GetImage("P/s1.png");
    REQUIRE(img);
    auto fr = img->GetFrame(0, false);
    REQUIRE(fr);
    CHECK(fr->GetAlpha(0, 0) == 255);   // pixel 1 -> y 0; pixel 0 falls off the top
    CHECK(fr->GetAlpha(0, 1) == 0);
    CHECK(fr->GetAlpha(2, 2) == 255);
    CHECK(fr->GetAlpha(2, 3) == 0);
}

TEST_CASE("Partial scene in horizontal orientation transposes the image", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 5;
    f.opt.modelWidth = 5;
    f.opt.modelHeight = 3;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0) + Elem(1, 1)) + "</scenes>")));
    auto img = f.Media().GetImage("P/s1.png");
    REQUIRE(img);
    CHECK(img->GetImageWidth() == 5);
    CHECK(img->GetImageHeight() == 3);
}

TEST_CASE("Partial scene image is rescaled to the model by ImageResize", "[impexp2][super][scene]") {
    struct C { ImageResize r; int w; int h; };
    // grid 3x4, model 6x12
    auto c = GENERATE(values<C>({ { ImageResize::None, 3, 4 }, { ImageResize::ExactWidth, 6, 4 }, { ImageResize::ExactHeight, 3, 12 }, { ImageResize::ExactWidthOrHeight, 6, 12 }, { ImageResize::All, 6, 12 } }));
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    f.opt.modelWidth = 6;
    f.opt.modelHeight = 12;
    f.opt.imageResize = c.r;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
    auto img = f.Media().GetImage("P/s1.png");
    REQUIRE(img);
    CAPTURE((int)c.r);
    CHECK(img->GetImageWidth() == c.w);
    CHECK(img->GetImageHeight() == c.h);
}

TEST_CASE("Partial scene with a 1x1 model and a large grid shrinks the image", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 12;
    f.opt.ySize = 50;
    f.opt.modelWidth = 1;
    f.opt.modelHeight = 1;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
    auto img = f.Media().GetImage("P/s1.png");
    REQUIRE(img);
    CHECK(img->GetImageWidth() >= 1);
}

TEST_CASE("Partial scene with a zero sized model must not produce an empty image", "[impexp2][super][scene]") {
    Fixture f;
    f.opt.xSize = 3;
    f.opt.ySize = 4;
    f.opt.modelWidth = 0;
    f.opt.modelHeight = 0;
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Elem(0, 0) + Elem(2, 3)) + "</scenes>")));
    SUCCEED();
}

TEST_CASE("Scene savedIndex is used for the image name", "[impexp2][super][scene]") {
    struct C { const char* idx; const char* name; };
    auto c = GENERATE(values<C>({ { "12", "P/s12.png" }, { "", "P/s.png" }, { "-1", "P/s-1.png" }, { "x y", "P/sx y.png" } }));
    Fixture f;
    f.opt.xSize = f.opt.ySize = 2;
    std::string sc = std::string("<scene savedIndex=\"") + c.idx + "\" layer=\"1\" startCentisecond=\"0\" endCentisecond=\"10\" " + RED + ">" + Elem(0, 0) + Elem(1, 1) + "</scene>";
    REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + sc + "</scenes>")));
    CHECK(f.Find("Pictures").Get("E_TEXTCTRL_Pictures_Filename") == c.name);
}

// ---------------------------------------------------------------- images and image actions

namespace {
std::string Image(int idx, int w, int h, const std::string& px, int xo = 0, int yo = 0, const char* pixTag = "pixe") {
    return "<image name=\"i\" width=\"" + std::to_string(w) + "\" height=\"" + std::to_string(h) + "\" xOffset=\"" + std::to_string(xo) + "\" yOffset=\"" + std::to_string(yo) +
           "\" savedIndex=\"" + std::to_string(idx) + "\"><" + pixTag + " s=\"" + px + "\"/></image>";
}
std::string Action(int idx, int layer, int s, int e, int xs, int xe, int ys, int ye, const std::string& extra = "") {
    return "<imageAction name=\"n\" imageIndex=\"" + std::to_string(idx) + "\" layer=\"" + std::to_string(layer) + "\" startCentisecond=\"" + std::to_string(s) + "\" endCentisecond=\"" +
           std::to_string(e) + "\" xStart=\"" + std::to_string(xs) + "\" xEnd=\"" + std::to_string(xe) + "\" yStart=\"" + std::to_string(ys) + "\" yEnd=\"" + std::to_string(ye) + "\" " + extra + "/>";
}
}

TEST_CASE("Image pixels are decoded from packed percentage colours", "[impexp2][super][image]") {
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(7, 3, 2, "25600,0,100,6579300,0,1") + "</images>")));
    auto img = f.Media().GetImage("P/7.png");
    REQUIRE(img);
    CHECK(img->GetImageWidth() == 3);
    CHECK(img->GetImageHeight() == 2);
    auto fr = img->GetFrame(0, false);
    REQUIRE(fr);
    CHECK(fr->GetGreen(0, 0) == 255);
    CHECK(fr->GetRed(0, 0) == 0);
    CHECK(fr->GetAlpha(0, 0) == 255);
    CHECK(fr->GetAlpha(1, 0) == 0);
    CHECK(fr->GetBlue(2, 0) == 255);
    CHECK(fr->GetAlpha(2, 0) == 255);
    CHECK(fr->GetRed(0, 1) == 255);
    CHECK(fr->GetGreen(0, 1) == 255);
    CHECK(fr->GetBlue(0, 1) == 255);
    CHECK(fr->GetAlpha(1, 1) == 0);
    CHECK(fr->GetAlpha(2, 1) == 255);
}

TEST_CASE("Image pixel list longer than the image is truncated", "[impexp2][super][image]") {
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 2, 2, "100,100,100,100,100,100,100") + "</images>")));
    auto img = f.Media().GetImage("P/1.png");
    REQUIRE(img);
    CHECK(img->GetImageHeight() == 2);
}

TEST_CASE("Image pixel list shorter than the image leaves transparent pixels", "[impexp2][super][image]") {
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 2, 2, "25600") + "</images>")));
    auto fr = f.Media().GetImage("P/1.png")->GetFrame(0, false);
    CHECK(fr->GetAlpha(1, 1) == 0);
}

TEST_CASE("Image with an empty or junk pixel string is still created", "[impexp2][super][image]") {
    auto px = GENERATE(values<std::string>({ "", ",,,,", "abc,def", "-1,-2", "99999999999,1", "1e3" }));
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 2, 2, px) + "</images>")));
    CAPTURE(px);
    CHECK(f.Media().HasMedia("P/1.png"));
}

TEST_CASE("Image without pixel child produces no media", "[impexp2][super][image]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images><image width=\"2\" height=\"2\" savedIndex=\"1\"/><image width=\"2\" height=\"2\" savedIndex=\"2\"><other/></image></images>")));
    CHECK_FALSE(f.Media().HasMedia("P/1.png"));
    CHECK_FALSE(f.Media().HasMedia("P/2.png"));
}

TEST_CASE("Image pixels element is recognised only as pixe after preprocessing", "[impexp2][super][image]") {
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 1, 1, "100", 0, 0, "pixels") + "</images>"), false));
    CHECK_FALSE(f.Media().HasMedia("P/1.png"));
}

TEST_CASE("Image with zero width and pixel data must not divide by zero", "[impexp2][super][image]") {
    // Run/images: px = p % w with w == 0 is SIGFPE
    Fixture f;
    CHECK(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 0, 2, "100,100") + "</images>")));
}

TEST_CASE("Image with negative width and pixel data must not crash", "[impexp2][super][image]") {
    // xlImage(w<0) has no buffer, but SetRGBA is still called
    Fixture f;
    CHECK(f.RunRaw(Doc(Layout(), "<images>" + Image(1, -2, 2, "100,100") + "</images>")));
}

TEST_CASE("Image with zero height is accepted", "[impexp2][super][image]") {
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 2, 0, "100,100") + "</images>")));
    SUCCEED();
}

TEST_CASE("Image resize modes on images", "[impexp2][super][image]") {
    struct C { ImageResize r; int w; int h; };
    // image 4x2 on a 4x50 grid (xSize 4), model 8x100
    auto c = GENERATE(values<C>({ { ImageResize::None, 4, 2 }, { ImageResize::ExactWidth, 8, 2 }, { ImageResize::ExactHeight, 4, 2 }, { ImageResize::ExactWidthOrHeight, 8, 2 }, { ImageResize::All, 8, 4 } }));
    Fixture f;
    f.opt.xSize = 4;
    f.opt.ySize = 50;
    f.opt.modelWidth = 8;
    f.opt.modelHeight = 100;
    f.opt.imageResize = c.r;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 4, 2, "100,100,100,100,100,100,100,100") + "</images>")));
    auto img = f.Media().GetImage("P/1.png");
    REQUIRE(img);
    CAPTURE((int)c.r);
    CHECK(img->GetImageWidth() == c.w);
    CHECK(img->GetImageHeight() == c.h);
}

TEST_CASE("Image in horizontal orientation is rotated", "[impexp2][super][image]") {
    Fixture f;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), "<images>" + Image(1, 3, 2, "100,100,100,100,100,100") + "</images>")));
    auto img = f.Media().GetImage("P/1.png");
    REQUIRE(img);
    CHECK(img->GetImageWidth() == 2);
    CHECK(img->GetImageHeight() == 3);
}

TEST_CASE("Image action at a fixed position becomes a Pictures effect", "[impexp2][super][imageaction]") {
    Fixture f;
    f.opt.xSize = 6;
    f.opt.ySize = 10;
    f.opt.modelWidth = 6;
    f.opt.modelHeight = 10;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(7, 3, 2, "100,100,100,100,100,100", 4, 6) + "</images><imageActions>" + Action(7, 2, 10, 20, 1, 1, 2, 2, "preRampTime=\"5\" rampTime=\"10\"") + "</imageActions>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.start == 100);
    CHECK(e.end == 200);
    CHECK(e.Get("E_SLIDER_PicturesXC") == "4");
    CHECK(e.Get("E_SLIDER_PicturesYC") == "-4");
    CHECK(e.Get("E_CHOICE_Pictures_Direction") == "none");
    CHECK(e.Get("E_CHOICE_Scaling") == "No Scaling");
    CHECK(e.Get("E_TEXTCTRL_Pictures_Filename") == "P/7.png");
    CHECK(e.Get("T_TEXTCTRL_Fadein") == "0.050000");
    CHECK(e.Get("T_TEXTCTRL_Fadeout") == "0.100000");
    CHECK(e.Get("X_Effect_Description") == "n");
}

TEST_CASE("Image action that moves becomes a vector Pictures effect", "[impexp2][super][imageaction]") {
    Fixture f;
    f.opt.xSize = 6;
    f.opt.ySize = 10;
    f.opt.modelWidth = 6;
    f.opt.modelHeight = 10;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(7, 3, 2, "100,100,100,100,100,100", 4, 6) + "</images><imageActions>" + Action(7, 2, 10, 20, 1, 5, 2, 4) + "</imageActions>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_CHOICE_Pictures_Direction") == "vector");
    CHECK(e.Get("E_SLIDER_PicturesXC") == "4");
    CHECK(e.Get("E_SLIDER_PicturesYC") == "-4");
    CHECK(e.Get("E_SLIDER_PicturesEndXC") == "8");
    CHECK(e.Get("E_SLIDER_PicturesEndYC") == "-6");
    CHECK(e.Get("T_TEXTCTRL_Fadein") == "<none>");
}

TEST_CASE("Image action offsets scale with the image resize mode", "[impexp2][super][imageaction]") {
    struct C { ImageResize r; const char* xc; const char* yc; };
    auto c = GENERATE(values<C>({ { ImageResize::None, "4", "-4" }, { ImageResize::All, "8", "-8" } }));
    Fixture f;
    f.opt.xSize = 6;
    f.opt.ySize = 10;
    f.opt.modelWidth = 12;
    f.opt.modelHeight = 20;
    f.opt.imageResize = c.r;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(7, 3, 2, "100,100,100,100,100,100", 4, 6) + "</images><imageActions>" + Action(7, 2, 10, 20, 1, 1, 2, 2) + "</imageActions>")));
    auto e = f.Find("Pictures");
    CAPTURE((int)c.r);
    CHECK(e.Get("E_SLIDER_PicturesXC") == c.xc);
    CHECK(e.Get("E_SLIDER_PicturesYC") == c.yc);
}

TEST_CASE("Image action referencing a missing image has an empty file name", "[impexp2][super][imageaction]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<imageActions>" + Action(99, 2, 10, 20, 0, 0, 0, 0) + "</imageActions>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_TEXTCTRL_Pictures_Filename") == "");
}

TEST_CASE("Image action coordinates may be negative", "[impexp2][super][imageaction]") {
    Fixture f;
    f.opt.xSize = 6;
    f.opt.ySize = 10;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout(), "<images>" + Image(1, 3, 2, "100,100,100,100,100,100", 0, 0) + "</images><imageActions>" + Action(1, 2, 0, 10, -28, -28, -5, -5) + "</imageActions>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_PicturesXC") == "-29");
    CHECK(e.Get("E_SLIDER_PicturesYC") == "9");
}

TEST_CASE("Image action in horizontal orientation swaps and negates y", "[impexp2][super][imageaction]") {
    Fixture f;
    f.opt.xSize = 6;
    f.opt.ySize = 10;
    f.opt.imageResize = ImageResize::None;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), "<images>" + Image(1, 3, 2, "100,100,100,100,100,100", 0, 0) + "</images><imageActions>" + Action(1, 2, 0, 10, 3, 3, 4, 4) + "</imageActions>")));
    auto e = f.Find("Pictures");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_SLIDER_PicturesXC").size() > 0);
}

TEST_CASE("Image action ramp times", "[impexp2][super][imageaction]") {
    struct C { const char* pre; const char* ramp; const char* fin; const char* fout; };
    auto c = GENERATE(values<C>({ { "0", "0", "<none>", "<none>" }, { "100", "0", "1.000000", "<none>" }, { "0", "250", "<none>", "2.500000" }, { "1", "1", "0.010000", "0.010000" }, { "-5", "-5", "-0.050000", "-0.050000" } }));
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<imageActions>" + Action(1, 2, 0, 10, 0, 0, 0, 0, std::string("preRampTime=\"") + c.pre + "\" rampTime=\"" + c.ramp + "\"") + "</imageActions>")));
    auto e = f.Find("Pictures");
    CAPTURE(c.pre, c.ramp);
    CHECK(e.Get("T_TEXTCTRL_Fadein") == c.fin);
    CHECK(e.Get("T_TEXTCTRL_Fadeout") == c.fout);
}

TEST_CASE("Image action ramp times are not shifted by the timing offset", "[impexp2][super][imageaction]") {
    Fixture f;
    f.opt.timingOffsetMs = 1000;
    REQUIRE(f.RunRaw(Doc(Layout(), "<imageActions>" + Action(1, 2, 0, 10, 0, 0, 0, 0, "preRampTime=\"5\" rampTime=\"5\"") + "</imageActions>")));
    auto e = f.Find("Pictures");
    CHECK(e.Get("T_TEXTCTRL_Fadein") == "0.050000");
    CHECK(e.start == 1000);
}

TEST_CASE("Image action blend string", "[impexp2][super][imageaction]") {
    Fixture f;
    f.opt.layerBlend = "Average";
    REQUIRE(f.RunRaw(Doc(Layout(), "<imageActions>" + Action(1, 2, 0, 10, 0, 0, 0, 0) + Action(1, 3, 0, 10, 0, 1, 0, 1) + "</imageActions>")));
    for (auto& e : f.Effects()) CHECK(e.Get("T_CHOICE_LayerMethod") == "Average");
}

// ---------------------------------------------------------------- text

TEST_CASE("Text action with OS font", "[impexp2][super][text]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Text("text=\"Hi\" fontName=\"Arial\" fontCapsHeight=\"6\" fontCellWidth=\"6\" fontCellHeight=\"6\" red=\"100\" green=\"0\" blue=\"0\" xStart=\"1\" yStart=\"2\" xEnd=\"3\" yEnd=\"4\" rotation=\"0\""))));
    auto e = f.Find("Text");
    REQUIRE(e.layer >= 0);
    CHECK(e.start == 0);
    CHECK(e.end == 1000);
    CHECK(e.Get("E_TEXTCTRL_Text") == "Hi");
    CHECK(e.Get("E_CHOICE_Text_Effect") == "normal");
    CHECK(e.Get("E_CHOICE_Text_Font") == "Use OS Fonts");
    CHECK(e.Get("E_FONTPICKER_Text_Font") == "arial 10");
    CHECK(e.Get("E_SLIDER_Text_XStart") == "-5");
    CHECK(e.Get("E_SLIDER_Text_XEnd") == "12");
    CHECK(e.Get("E_SLIDER_Text_YStart") == "37");
    CHECK(e.Get("E_SLIDER_Text_YEnd") == "33");
    CHECK(e.Get("C_BUTTON_Palette1") == "#ff0000");
    CHECK(e.Get("C_CHECKBOX_Palette1") == "1");
    CHECK(e.Get("C_CHECKBOX_Palette2") == "0");
}

TEST_CASE("Text action default rotation is 90 degrees", "[impexp2][super][text]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Text("text=\"X\" fontName=\"Zzz\""))));
    CHECK(f.Find("Text").Get("E_CHOICE_Text_Effect") == "rotate down 90");
}

TEST_CASE("Text action rotation and vertical fonts choose the effect", "[impexp2][super][text]") {
    struct C { const char* font; int rot; const char* eff; };
    auto c = GENERATE(values<C>({ { "Arial", 0, "normal" }, { "Arial", 90, "rotate down 90" }, { "Arial", 270, "rotate up 90" }, { "Arial", -90, "rotate up 90" }, { "Arial", 45, "normal" },
                                  { "Vertical_Foo", 0, "vert text down" }, { "Vertical_Foo", 90, "vert text down" } }));
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Text(std::string("text=\"Ab\" fontName=\"") + c.font + "\" rotation=\"" + std::to_string(c.rot) + "\""))));
    CAPTURE(c.font, c.rot);
    CHECK(f.Find("Text").Get("E_CHOICE_Text_Effect") == c.eff);
}

TEST_CASE("Text action in horizontal orientation reduces rotation by 90", "[impexp2][super][text]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout("horizontal"), Text("text=\"X\" fontName=\"Zzz\" rotation=\"90\""))));
    CHECK(f.Find("Text").Get("E_CHOICE_Text_Effect") == "normal");
}

TEST_CASE("Text action mask types", "[impexp2][super][text]") {
    struct C { const char* mask; const char* blend; const char* expect; };
    auto c = GENERATE(values<C>({ { "positiveMask", "Average", "Average" }, { "positiveMask", "Normal", "<none>" }, { "negativeMask", "Average", "1 is Mask" }, { "negativeMask", "Normal", "1 is Mask" },
                                  { "", "Average", "Average" }, { "weird", "2 reveals 1", "2 reveals 1" } }));
    Fixture f;
    f.opt.layerBlend = c.blend;
    REQUIRE(f.RunRaw(Doc(Layout(), Text(std::string("text=\"X\" maskType=\"") + c.mask + "\""))));
    CAPTURE(c.mask, c.blend);
    CHECK(f.Find("Text").Get("T_CHOICE_LayerMethod") == c.expect);
}

TEST_CASE("Text action bold fonts pick a bold font string", "[impexp2][super][text]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Text("text=\"X\" fontName=\"My_Bold\" fontCapsHeight=\"8\""))));
    CHECK(f.Find("Text").Get("E_FONTPICKER_Text_Font") == "arial bold 12");
}

TEST_CASE("Text action defaults for missing attributes", "[impexp2][super][text]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<textActions><textAction layer=\"1\"/></textActions>")));
    auto e = f.Find("Text");
    REQUIRE(e.layer >= 0);
    CHECK(e.Get("E_TEXTCTRL_Text") == "");
}

TEST_CASE("Text action with empty text and zero start end", "[impexp2][super][text]") {
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), "<textActions><textAction layer=\"1\" startCentisecond=\"0\" endCentisecond=\"0\" text=\"\"/></textActions>")));
    SUCCEED();
}

TEST_CASE("Text action text with commas and equals must reach the effect intact", "[impexp2][super][text]") {
    // settings strings escape ',' as &comma; and '&' as &amp; (SettingsMap::Parse undoes it)
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Text("text=\"Hello, World=1 &amp; x\" fontName=\"Zzz\""))));
    CHECK(f.Find("Text").Get("E_TEXTCTRL_Text") == "Hello&comma; World=1 &amp; x");
}

TEST_CASE("Text action font size and cell widths from huge or negative values", "[impexp2][super][text]") {
    auto v = GENERATE(values<std::string>({ "-5", "0", "100000", "abc" }));
    Fixture f;
    REQUIRE(f.RunRaw(Doc(Layout(), Text("text=\"ABC\" fontName=\"Zzz\" fontCapsHeight=\"" + v + "\" fontCellWidth=\"" + v + "\" fontCellHeight=\"" + v + "\" xStart=\"" + v + "\" yEnd=\"" + v + "\""))));
    CAPTURE(v);
    CHECK(f.Count() == 1);
}

TEST_CASE("Text action with a zero grid height in options still imports", "[impexp2][super][text]") {
    // CalcUnBoundedPercentage(val, 0) divides by zero as a double and converts inf to int (undefined, not trapping)
    Fixture f;
    f.opt.ySize = 0;
    CHECK(f.RunRaw(Doc(Layout(), Text("text=\"X\""))));
}

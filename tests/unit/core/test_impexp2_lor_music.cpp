#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/LORMusic.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"

#include <pugixml.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct Fix {
    HeadlessRenderContext ctx;
    EffectLayer* layer = nullptr;
    Fix() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* e = se.AddElement("Model", "model", true, false, true, false, false);
        layer = e->AddEffectLayer();
    }
    int Count() const { return layer->GetEffectCount(); }
    Effect* At(int i) const { return layer->GetEffect(i); }
};

pugi::xml_document P(const std::string& xml) {
    pugi::xml_document d;
    d.load_buffer(xml.data(), xml.size(), pugi::parse_default, pugi::encoding_auto);
    return d;
}

std::string Eff(const std::string& type, int s, int e, const std::string& inten) {
    return "<effect type=\"" + type + "\" startCentisecond=\"" + std::to_string(s) + "\" endCentisecond=\"" + std::to_string(e) + "\" " + inten + "/>";
}
std::string I(int v) { return "intensity=\"" + std::to_string(v) + "\""; }
std::string R(int a, int b) { return "startIntensity=\"" + std::to_string(a) + "\" endIntensity=\"" + std::to_string(b) + "\""; }

std::string Ch(const std::string& name, int idx, const std::string& effects = "", const std::string& color = "255", int unit = 1, int circuit = 1) {
    return "<channel name=\"" + name + "\" deviceType=\"LOR\" color=\"" + color + "\" centiseconds=\"1000\" circuit=\"" + std::to_string(circuit) + "\" network=\"1\" unit=\"" +
           std::to_string(unit) + "\" savedIndex=\"" + std::to_string(idx) + "\">" + effects + "</channel>";
}
std::string Rgb(const std::string& name, int idx, std::vector<int> refs) {
    std::string r = "<rgbChannel name=\"" + name + "\" savedIndex=\"" + std::to_string(idx) + "\" totalCentiseconds=\"1000\"><channels>";
    for (int x : refs) r += "<channel savedIndex=\"" + std::to_string(x) + "\"/>";
    return r + "</channels></rgbChannel>";
}
std::string Doc(const std::string& channels, const std::string& timing = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\r\n<sequence saveFileVersion=\"14\"><animation columns=\"60\" rows=\"40\" hideControls=\"True\" image=\"\"/><channels>" + channels +
           "</channels><timingGrids>" + timing + "</timingGrids><tracks><track timingGrid=\"0\" totalCentiseconds=\"1000\"><channels><channel savedIndex=\"0\"/></channels><loopLevels/></track></tracks></sequence>";
}
std::string Grid(const std::string& name, const std::vector<int>& cs, const std::string& type = "freeform") {
    std::string s = "<timingGrid name=\"" + name + "\" saveID=\"0\" spacing=\"5\" type=\"" + type + "\">";
    for (int c : cs) s += "<timing centisecond=\"" + std::to_string(c) + "\"/>";
    return s + "</timingGrid>";
}
std::string Settings(Effect* e, const std::string& k) { return e->GetSettings().Get(k, "<none>"); }
bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }
}

// ---------- On effects ----------

TEST_CASE("LMS intensity forms map to On start and end", "[impexp2][lor][lms]") {
    struct Row { std::string eff; std::string start; std::string end; };
    auto r = GENERATE(Row{ I(100), "<none>", "<none>" }, Row{ I(50), "50", "50" }, Row{ I(0), "0", "0" }, Row{ R(0, 100), "0", "<none>" }, Row{ R(100, 0), "<none>", "0" },
                      Row{ R(20, 80), "20", "80" }, Row{ R(100, 100), "<none>", "<none>" });
    auto d = P(Doc(Ch("A", 0, Eff("intensity", 10, 20, r.eff))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetEffectName() == "On");
    CHECK(f.At(0)->GetStartTimeMS() == 100);
    CHECK(f.At(0)->GetEndTimeMS() == 200);
    CHECK(Settings(f.At(0), "E_TEXTCTRL_Eff_On_Start") == r.start);
    CHECK(Settings(f.At(0), "E_TEXTCTRL_Eff_On_End") == r.end);
    CHECK(Settings(f.At(0), "E_CHECKBOX_On_Shimmer") == "<none>");
}

TEST_CASE("LMS effect types", "[impexp2][lor][lms]") {
    struct Row { const char* type; const char* name; bool shimmer; };
    auto r = GENERATE(Row{ "intensity", "On", false }, Row{ "shimmer", "On", true }, Row{ "twinkle", "Twinkle", false }, Row{ "DMX intensity", "On", false },
                      Row{ "", "On", true }, Row{ "weird", "On", true });
    auto d = P(Doc(Ch("A", 0, Eff(r.type, 0, 100, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetEffectName() == r.name);
    if (std::string(r.name) == "On") CHECK((Settings(f.At(0), "E_CHECKBOX_On_Shimmer") == "1") == r.shimmer);
}

TEST_CASE("LMS twinkle settings", "[impexp2][lor][lms]") {
    SECTION("short effect steps are clamped to at least two") {
        auto d = P(Doc(Ch("A", 0, Eff("twinkle", 0, 100, I(100)))));
        LORMusic l(d, 20);
        Fix f;
        l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
        REQUIRE(f.Count() == 1);
        int steps = atoi(Settings(f.At(0), "E_SLIDER_Twinkle_Steps").c_str());
        CHECK(steps >= 2);
        CHECK(steps <= 7);
        CHECK(Settings(f.At(0), "E_CHECKBOX_Twinkle_Strobe") == "1");
        CHECK(Settings(f.At(0), "E_SLIDER_Twinkle_Count") == "2");
    }
    SECTION("very long effect steps cap at 200") {
        auto d = P(Doc(Ch("A", 0, Eff("twinkle", 0, 100000, I(100)))));
        LORMusic l(d, 20);
        Fix f;
        l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
        REQUIRE(f.Count() == 1);
        CHECK(Settings(f.At(0), "E_SLIDER_Twinkle_Steps") == "200");
    }
    SECTION("brightness from a fixed intensity") {
        auto d = P(Doc(Ch("A", 0, Eff("twinkle", 0, 100, I(40)))));
        LORMusic l(d, 20);
        Fix f;
        l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
        REQUIRE(f.Count() == 1);
        CHECK(f.At(0)->GetPaletteMap().Get("C_SLIDER_Brightness", "") == "40");
    }
    SECTION("full intensity has no brightness") {
        auto d = P(Doc(Ch("A", 0, Eff("twinkle", 0, 100, I(100)))));
        LORMusic l(d, 20);
        Fix f;
        l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
        REQUIRE(f.Count() == 1);
        CHECK(f.At(0)->GetPaletteMap().Get("C_SLIDER_Brightness", "none") == "none");
    }
    SECTION("ramp becomes a value curve") {
        auto d = P(Doc(Ch("A", 0, Eff("twinkle", 0, 100, R(10, 90)))));
        LORMusic l(d, 20);
        Fix f;
        l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
        REQUIRE(f.Count() == 1);
        auto vc = f.At(0)->GetPaletteMap().Get("C_VALUECURVE_Brightness", "");
        CHECK(Has(vc, "P1=10.00"));
        CHECK(Has(vc, "P2=90.00"));
    }
}

TEST_CASE("LMS DMX intensity scales 0-255 to 0-100", "[impexp2][lor][lms]") {
    struct Row { int v; const char* expect; };
    auto r = GENERATE(Row{ 255, "<none>" }, Row{ 128, "50" }, Row{ 0, "0" }, Row{ 1, "0" }, Row{ 254, "99" }, Row{ 510, "200" });
    auto d = P(Doc(Ch("A", 0, Eff("DMX intensity", 0, 10, I(r.v)))));
    LORMusic l(d, 20);
    Fix f;
    l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
    REQUIRE(f.Count() == 1);
    CHECK(Settings(f.At(0), "E_TEXTCTRL_Eff_On_Start") == r.expect);
}

TEST_CASE("LMS DMX ramp scales both ends", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0, Eff("DMX intensity", 0, 10, R(0, 255)))));
    LORMusic l(d, 20);
    Fix f;
    l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
    REQUIRE(f.Count() == 1);
    CHECK(Settings(f.At(0), "E_TEXTCTRL_Eff_On_Start") == "0");
    CHECK(Settings(f.At(0), "E_TEXTCTRL_Eff_On_End") == "<none>");
}

TEST_CASE("LMS effect without intensity attributes", "[impexp2][lor][lms][!mayfail]") {
    // BUG: with neither intensity nor startIntensity/endIntensity the strings stay empty and the settings read
    // "E_TEXTCTRL_Eff_On_Start=,E_TEXTCTRL_Eff_On_End=" (empty values), instead of defaulting to full intensity
    auto d = P(Doc(Ch("A", 0, "<effect type=\"intensity\" startCentisecond=\"0\" endCentisecond=\"10\"/>")));
    LORMusic l(d, 20);
    Fix f;
    l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetSettingsAsString().find("On_Start=") == std::string::npos);
}

TEST_CASE("LMS palette for single and multi channel nodes", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0, Eff("intensity", 0, 10, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    SECTION("single channel node is white") {
        l.MapChannelEffects(f.layer, "A", xlColor(1, 2, 3), 1, true);
        CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#FFFFFF");
    }
    SECTION("zero channel node is white") {
        l.MapChannelEffects(f.layer, "A", xlColor(1, 2, 3), 0, true);
        CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#FFFFFF");
    }
    SECTION("negative channel count is white") {
        l.MapChannelEffects(f.layer, "A", xlColor(1, 2, 3), -4, true);
        CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#FFFFFF");
    }
    SECTION("multi channel node uses the colour") {
        l.MapChannelEffects(f.layer, "A", xlColor(1, 2, 3), 3, true);
        CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#010203");
    }
}

TEST_CASE("LMS effects with odd times", "[impexp2][lor][lms]") {
    struct Row { int s, e; int expectCount; };
    auto r = GENERATE(Row{ 0, 10, 1 }, Row{ 20, 10, 0 }, Row{ 0, 100000, 1 });
    auto d = P(Doc(Ch("A", 0, Eff("intensity", r.s, r.e, I(50)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
    INFO(r.s << " " << r.e);
    CHECK(f.Count() == r.expectCount);
}

TEST_CASE("LMS zero length and negative start effects are skipped", "[impexp2][lor][lms][!mayfail]") {
    // SUSPECT: MapOnEffects (unlike the rgb path, which requires start < end) adds zero-length and negative-start effects
    for (auto se : { std::make_pair(10, 10), std::make_pair(0, 0), std::make_pair(-10, 10) }) {
        auto d = P(Doc(Ch("A", 0, Eff("intensity", se.first, se.second, I(50)))));
        LORMusic l(d, 20);
        Fix f;
        REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
        INFO(se.first << " " << se.second);
        CHECK(f.Count() == 0);
    }
}

TEST_CASE("LMS channel with several effects and stray children", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0, Eff("intensity", 0, 10, I(50)) + "<junk/>" + Eff("shimmer", 10, 20, I(60)) + Eff("intensity", 30, 40, R(0, 100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
    REQUIRE(f.Count() == 3);
    CHECK(f.At(1)->GetStartTimeMS() == 100);
    CHECK(f.At(2)->GetStartTimeMS() == 300);
}

TEST_CASE("LMS mapping names", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("Dup", 0, Eff("intensity", 0, 10, I(50)), "255", 1, 1) + Ch("Dup", 1, Eff("intensity", 20, 30, I(50)), "255", 2, 3) + Ch("", 2, Eff("intensity", 0, 5, I(50)))));
    LORMusic l(d, 20);
    Fix f;
    CHECK(l.GetChannelNames() == std::vector<std::string>{ "Dup", "Dup_Unit_2_Circuit_3" });
    REQUIRE(l.MapChannelEffects(f.layer, "Dup", xlWHITE, 1, true));
    CHECK(f.At(0)->GetStartTimeMS() == 0);
    REQUIRE(l.MapChannelEffects(f.layer, "Dup_Unit_2_Circuit_3", xlWHITE, 1, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetStartTimeMS() == 200);
    // the first channel also answers to its own long form
    REQUIRE(l.MapChannelEffects(f.layer, "Dup_Unit_1_Circuit_1", xlWHITE, 1, true));
    CHECK(f.At(0)->GetStartTimeMS() == 0);
    CHECK_FALSE(l.MapChannelEffects(f.layer, "", xlWHITE, 1, true));
}

TEST_CASE("LMS three channels of the same name give two identical long names", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("Dup", 0, "", "255", 1, 1) + Ch("Dup", 1, "", "255", 1, 1) + Ch("Dup", 2, "", "255", 1, 1)));
    LORMusic l(d, 20);
    auto n = l.GetChannelNames();
    REQUIRE(n.size() == 3);
    CHECK(n[0] == "Dup");
    CHECK(n[1] == "Dup_Unit_1_Circuit_1");
    CHECK(n[2] == "Dup_Unit_1_Circuit_1");
}

TEST_CASE("LMS same named channels on different networks get distinct names", "[impexp2][lor][lms][!mayfail]") {
    // SUSPECT: the disambiguated name is built from unit and circuit only; real files carry `network` too and often have no
    // `unit`, so the same channel name and circuit on two networks yields two identical "<name>_Unit__Circuit_<n>" entries
    auto d = P(Doc("<channel name=\"Dup\" circuit=\"5\" network=\"1\" savedIndex=\"0\"/><channel name=\"Dup\" circuit=\"5\" network=\"2\" savedIndex=\"1\"/>"
                   "<channel name=\"Dup\" circuit=\"5\" network=\"3\" savedIndex=\"2\"/>"));
    LORMusic l(d, 20);
    auto n = l.GetChannelNames();
    REQUIRE(n.size() == 3);
    CHECK(std::set<std::string>(n.begin(), n.end()).size() == 3);
}

TEST_CASE("LMS channel names are sorted number aware", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("Ch10", 0) + Ch("Ch2", 1) + Ch("Ch1", 2) + Ch("Ch 3", 3)));
    LORMusic l(d, 20);
    auto n = l.GetChannelNames();
    REQUIRE(n.size() == 4);
    CHECK(n[0] == "Ch1");
    CHECK(n[1] == "Ch2");
    CHECK(n[2] == "Ch10");
    CHECK(n[3] == "Ch 3");
}

TEST_CASE("LMS channel colours", "[impexp2][lor][lms]") {
    struct Row { const char* c; xlColor expect; };
    auto r = GENERATE(Row{ "255", xlColor(255, 0, 0) }, Row{ "65280", xlColor(0, 255, 0) }, Row{ "16711680", xlColor(0, 0, 255) }, Row{ "16777215", xlColor(255, 255, 255) },
                      Row{ "0", xlBLACK }, Row{ "", xlBLACK }, Row{ "abc", xlBLACK }, Row{ "-1", xlColor(255, 255, 255) }, Row{ "16711935", xlColor(255, 0, 255) },
                      Row{ "99999999999999", xlColor(0xff & 99999999999999LL, (99999999999999LL >> 8) & 0xff, (99999999999999LL >> 16) & 0xff) });
    auto d = P(Doc(Ch("A", 0, "", r.c)));
    LORMusic l(d, 20);
    CHECK(l.GetChannelColor("A") == r.expect);
}

TEST_CASE("LMS channel colour of rgb channels and unknown names is black", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0) + Rgb("RGB", 1, { 0, 0, 0 })));
    LORMusic l(d, 20);
    CHECK(l.GetChannelColor("RGB") == xlBLACK);
    CHECK(l.GetChannelColor("zzz") == xlBLACK);
    CHECK(l.GetChannelColor("") == xlBLACK);
}

// ---------- rgb ----------

namespace {
std::string RgbDoc(const std::string& r, const std::string& g, const std::string& b, const std::string& extra = "") {
    return Doc(Rgb("Tree", 9, { 0, 1, 2 }) + Ch("r", 0, r) + Ch("g", 1, g) + Ch("b", 2, b) + extra);
}
using Eff3 = std::tuple<std::string, int, int>;
std::vector<std::tuple<std::string, int, int>> Summ(Fix& f) {
    std::vector<std::tuple<std::string, int, int>> v;
    for (int i = 0; i < f.Count(); ++i) v.emplace_back(f.At(i)->GetEffectName(), f.At(i)->GetStartTimeMS(), f.At(i)->GetEndTimeMS());
    return v;
}
}

TEST_CASE("LMS rgb constant colours", "[impexp2][lor][lms][rgb]") {
    struct Row { int r, g, b; const char* col; };
    auto row = GENERATE(Row{ 100, 0, 0, "#ff0000" }, Row{ 0, 100, 0, "#00ff00" }, Row{ 0, 0, 100, "#0000ff" }, Row{ 100, 100, 100, "#ffffff" }, Row{ 100, 50, 0, "#ff7f00" },
                        Row{ 1, 1, 1, "#020202" }, Row{ 200, 0, 0, "#" }, Row{ 50, 50, 50, "#7f7f7f" });
    auto d = P(RgbDoc(Eff("intensity", 0, 100, I(row.r)), Eff("intensity", 0, 100, I(row.g)), Eff("intensity", 0, 100, I(row.b))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetEffectName() == "On");
    if (std::string(row.col).size() > 1) CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == row.col);
}

TEST_CASE("LMS rgb black produces nothing", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc(Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb ramps", "[impexp2][lor][lms][rgb]") {
    auto go = [](const std::string& r, const std::string& g, const std::string& b) {
        auto d = P(RgbDoc(r, g, b));
        LORMusic l(d, 20);
        auto f = std::make_unique<Fix>();
        l.MapChannelEffects(f->layer, "Tree", xlWHITE, 3, true);
        return f;
    };
    SECTION("fade in from black") {
        auto f = go(Eff("intensity", 0, 100, R(0, 100)), Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(f->At(0)->GetEffectName() == "On");
        CHECK(Settings(f->At(0), "E_TEXTCTRL_Eff_On_Start") == "0");
        CHECK(f->At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff0000");
    }
    SECTION("fade out to black") {
        auto f = go(Eff("intensity", 0, 100, R(100, 0)), Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(Settings(f->At(0), "E_TEXTCTRL_Eff_On_End") == "0");
    }
    SECTION("colour to colour is a colour wash") {
        auto f = go(Eff("intensity", 0, 100, R(100, 0)), Eff("intensity", 0, 100, R(0, 100)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(f->At(0)->GetEffectName() == "Color Wash");
        CHECK(f->At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff0000");
        CHECK(f->At(0)->GetPaletteMap().Get("C_BUTTON_Palette2", "") == "#00ff00");
    }
    SECTION("shimmer on one channel shimmers the effect") {
        auto f = go(Eff("shimmer", 0, 100, I(100)), Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(Settings(f->At(0), "E_CHECKBOX_On_Shimmer") == "1");
    }
    SECTION("shimmer on a colour wash") {
        auto f = go(Eff("shimmer", 0, 100, R(100, 0)), Eff("intensity", 0, 100, R(0, 100)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(f->At(0)->GetEffectName() == "Color Wash");
        CHECK(Settings(f->At(0), "E_CHECKBOX_ColorWash_Shimmer") == "1");
    }
    SECTION("shimmer fade in") {
        auto f = go(Eff("shimmer", 0, 100, R(0, 100)), Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(Settings(f->At(0), "E_CHECKBOX_On_Shimmer") == "1");
    }
    SECTION("shimmer fade out") {
        auto f = go(Eff("shimmer", 0, 100, R(100, 0)), Eff("intensity", 0, 100, I(0)), Eff("intensity", 0, 100, I(0)));
        REQUIRE(f->Count() == 1);
        CHECK(Settings(f->At(0), "E_CHECKBOX_On_Shimmer") == "1");
    }
}

TEST_CASE("LMS rgb channels with different timing are split", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc(Eff("intensity", 0, 100, I(100)), Eff("intensity", 50, 150, I(100)), ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(Summ(f) == std::vector<Eff3>{ { "On", 0, 500 }, { "On", 500, 1000 }, { "On", 1000, 1500 } });
    CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff0000");
    CHECK(f.At(1)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ffff00");
    CHECK(f.At(2)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#00ff00");
}

TEST_CASE("LMS rgb with a gap in one channel", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc(Eff("intensity", 0, 10, I(100)) + Eff("intensity", 30, 40, I(100)), "", ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(Summ(f) == std::vector<Eff3>{ { "On", 0, 100 }, { "On", 300, 400 } });
}

TEST_CASE("LMS rgb with only one channel having effects", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc("", "", Eff("intensity", 10, 20, I(100))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(Summ(f) == std::vector<Eff3>{ { "On", 100, 200 } });
    CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#0000ff");
}

TEST_CASE("LMS rgb with overlapping effects in one channel does not crash", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc(Eff("intensity", 0, 100, I(100)) + Eff("intensity", 50, 150, I(60)), "", ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() >= 1);
}

TEST_CASE("LMS rgb with out of order effects does not crash", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc(Eff("intensity", 50, 60, I(100)) + Eff("intensity", 0, 10, I(100)), Eff("intensity", 0, 70, I(100)), ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    SUCCEED();
}

TEST_CASE("LMS rgb with zero length and backwards effects ignores them", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc(Eff("intensity", 10, 10, I(100)) + Eff("intensity", 30, 20, I(100)), Eff("intensity", 0, 0, I(100)), ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb intensity above 100 clamps in the colour", "[impexp2][lor][lms][rgb][!mayfail]") {
    // BUG: GetRGBEffectData stores intensity*255/100 in a uint8_t without clamping, so 255 wraps to 0x8a
    auto d = P(RgbDoc(Eff("intensity", 0, 10, I(255)), "", ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    REQUIRE(f.Count() == 1);
    INFO(f.At(0)->GetPaletteAsString());
    CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff0000");
}

TEST_CASE("LMS rgb with missing intensity attributes", "[impexp2][lor][lms][rgb]") {
    auto d = P(RgbDoc("<effect type=\"intensity\" startCentisecond=\"0\" endCentisecond=\"10\"/>", "", ""));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb with a dangling savedIndex has no effects but resolves", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Rgb("Tree", 9, { 40, 41, 42 }) + Ch("r", 0, Eff("intensity", 0, 10, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb with partly dangling references uses what exists", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Rgb("Tree", 9, { 0, 41, 2 }) + Ch("r", 0, Eff("intensity", 0, 10, I(100))) + Ch("b", 2, Eff("intensity", 0, 10, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff00ff");
}

TEST_CASE("LMS rgb with no child refs", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Rgb("Tree", 9, {}) + Ch("r", 0, Eff("intensity", 0, 10, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb with fewer than three refs ignores channels that have no savedIndex", "[impexp2][lor][lms][rgb][!mayfail]") {
    // BUG: findRGB compares each channel's savedIndex ("" when the attribute is missing) with the unfilled idxs[1]/idxs[2]
    // (also ""), so a channel without a savedIndex is taken as both the green and the blue channel
    auto d = P(Doc(Rgb("Tree", 9, { 0 }) + Ch("r", 0, "") + "<channel name=\"stray\">" + Eff("intensity", 0, 10, I(100)) + "</channel>"));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb with duplicate savedIndex uses the last channel", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Rgb("Tree", 9, { 0, 1, 2 }) + Ch("r1", 0, Eff("intensity", 0, 10, I(100))) + Ch("r2", 0, Eff("intensity", 50, 60, I(100))) + Ch("g", 1) + Ch("b", 2)));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(Summ(f) == std::vector<Eff3>{ { "On", 500, 600 } });
}

TEST_CASE("LMS rgb referencing the same channel three times", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Rgb("Tree", 9, { 0, 0, 0 }) + Ch("x", 0, Eff("intensity", 0, 10, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ffffff");
}

TEST_CASE("LMS rgb with more than three refs ignores the extra ones", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Rgb("Tree", 9, { 0, 1, 2, 3 }) + Ch("r", 0) + Ch("g", 1) + Ch("b", 2) + Ch("w", 3, Eff("intensity", 0, 10, I(100)))));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 0);
}

TEST_CASE("LMS rgb channels placed before or after their colour channels", "[impexp2][lor][lms][rgb]") {
    auto d = P(Doc(Ch("r", 0, Eff("intensity", 0, 10, I(100))) + Ch("g", 1) + Ch("b", 2) + Rgb("Tree", 9, { 0, 1, 2 })));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, true));
    CHECK(f.Count() == 1);
}

TEST_CASE("LMS cosmic colour devices are not offered or mapped", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0) + "<cosmicColorDevice name=\"Cosmic\" savedIndex=\"5\" totalCentiseconds=\"10\"><channelGroups><channelGroup savedIndex=\"0\"/><channelGroup savedIndex=\"1\"/></channelGroups></cosmicColorDevice>"));
    LORMusic l(d, 20);
    CHECK(l.GetChannelNames() == std::vector<std::string>{ "A" });
    Fix f;
    CHECK_FALSE(l.MapChannelEffects(f.layer, "Cosmic", xlWHITE, 3, false));
}

TEST_CASE("LMS rgb channel and plain channel sharing a name resolve to the first", "[impexp2][lor][lms]") {
    auto d = P(Doc(Rgb("Same", 9, { 0, 1, 2 }) + Ch("Same", 0, Eff("intensity", 0, 10, I(100))) + Ch("g", 1) + Ch("b", 2)));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "Same", xlWHITE, 3, true));
    REQUIRE(f.Count() == 1);
    CHECK(f.At(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff0000");
}

// ---------- ccr ----------

TEST_CASE("LMS ccr channel names", "[impexp2][lor][lms][ccr]") {
    struct Row { const char* name; const char* prefix; };
    auto r = GENERATE(Row{ "Roof-P01", "Roof" }, Row{ "Roof p3", "Roof" }, Row{ "Roof P12", "Roof" }, Row{ "Roof-P1", "Roof" }, Row{ "Roof-P0", "" }, Row{ "Roof-P", "" },
                      Row{ "Roof", "" }, Row{ "Roof-Pabc", "" }, Row{ "-P01", "" }, Row{ " p5", "" }, Row{ "Rooftop-P99", "Rooftop" }, Row{ "A-P1 p2", "A-P1" });
    auto d = P(Doc(Rgb(r.name, 0, { 1, 2, 3 })));
    LORMusic l(d, 20);
    if (*r.prefix) {
        CHECK(l.GetCCRNames() == std::vector<std::string>{ r.prefix });
        CHECK(l.IsCCRName(r.name));
        CHECK(l.GetChannelNames().empty());
    } else {
        CHECK(l.GetCCRNames().empty());
    }
}

TEST_CASE("LMS ccr nodes resolve to the first naming variant that exists", "[impexp2][lor][lms][ccr]") {
    struct Row { const char* names; const char* expect; };
    auto r = GENERATE(Row{ "R-P01", "R-P01" }, Row{ "R p01", "R p01" }, Row{ "R-P1", "R-P1" }, Row{ "R p1", "R p1" }, Row{ "R P 01", "R P 01" }, Row{ "", "R-P01" },
                      Row{ "R p1,R-P1", "R-P1" });
    std::string chans;
    std::string all = r.names;
    int i = 0;
    size_t pos = 0;
    while (!all.empty() && pos != std::string::npos) {
        auto c = all.find(',', pos);
        chans += "<channel name=\"" + all.substr(pos, c == std::string::npos ? std::string::npos : c - pos) + "\" savedIndex=\"" + std::to_string(i++) + "\"/>";
        pos = c == std::string::npos ? c : c + 1;
    }
    auto d = P(Doc(chans));
    LORMusic l(d, 20);
    CHECK(l.ResolveCCRNodeName("R", 0) == r.expect);
}

TEST_CASE("LMS ccr node resolution with out of range node numbers", "[impexp2][lor][lms][ccr]") {
    auto d = P(Doc(""));
    LORMusic l(d, 20);
    CHECK(l.ResolveCCRNodeName("R", 99) == "R-P100");
    CHECK(l.ResolveCCRNodeName("R", -1) == "R-P00");
    CHECK(l.ResolveCCRNodeName("R", -10) == "R-P-9");
    CHECK(l.ResolveCCRNodeName("", 0) == "-P01");
    CHECK(l.ResolveCCRNodeName("R", 2147483647).size() > 0);
}

TEST_CASE("LMS plain channels sharing a ccr prefix remain available", "[impexp2][lor][lms][ccr][!mayfail]") {
    // BUG: IsCCRName is a plain starts-with test, so once any rgbChannel "Tree-P01" defines the prefix "Tree" every ordinary
    // channel whose name merely begins with "Tree" (e.g. "Tree Star") is erased from the available list
    auto d = P(Doc(Rgb("Tree-P01", 0, { 1, 2, 3 }) + Ch("Tree Star", 1)));
    LORMusic l(d, 20);
    auto n = l.GetChannelNames();
    CHECK(std::find(n.begin(), n.end(), "Tree Star") != n.end());
}

TEST_CASE("LMS ccr prefix matching needs a longer name", "[impexp2][lor][lms][ccr]") {
    auto d = P(Doc(Rgb("Tree-P01", 0, { 1, 2, 3 })));
    LORMusic l(d, 20);
    CHECK_FALSE(l.IsCCRName("Tree"));
    CHECK_FALSE(l.IsCCRName(""));
    CHECK(l.IsCCRName("Tree-P01"));
    CHECK(l.IsCCRName("Tree anything"));
    CHECK_FALSE(l.IsCCRName("tree-P01"));
}

// ---------- timing ----------

TEST_CASE("LMS timing track listing", "[impexp2][lor][lms][timing]") {
    auto d = P(Doc("", Grid("Fixed", { 0 }, "fixed") + Grid("Free", { 10 }) + Grid("NoType", { 10 }, "") + Grid("", { 10 }) + Grid("Other", { 10 }, "weird")));
    LORMusic l(d, 20);
    CHECK(l.GetTimingTracks() == std::vector<std::string>{ "Free", "NoType", "Other" });
}

TEST_CASE("LMS timing track without a type attribute is listed", "[impexp2][lor][lms][timing]") {
    auto d = P(Doc("", "<timingGrid name=\"T\"><timing centisecond=\"10\"/></timingGrid>"));
    LORMusic l(d, 20);
    CHECK(l.GetTimingTracks() == std::vector<std::string>{ "T" });
}

TEST_CASE("LMS empty timing grids", "[impexp2][lor][lms][timing]") {
    auto d = P(Doc("", ""));
    LORMusic l(d, 20);
    CHECK(l.GetTimingTracks().empty());
    CHECK(l.GetTimings("x").empty());
    auto d2 = P(Doc("", Grid("Empty", {})));
    LORMusic l2(d2, 20);
    CHECK(l2.GetTimingTracks() == std::vector<std::string>{ "Empty" });
    CHECK(l2.GetTimings("Empty").empty());
}

TEST_CASE("LMS timing rounding and offsets", "[impexp2][lor][lms][timing]") {
    struct Row { std::vector<int> cs; int freq; int off; std::vector<std::pair<uint32_t, uint32_t>> expect; };
    auto r = GENERATE(Row{ { 50, 125, 125, 300 }, 20, 0, { { 0, 500 }, { 500, 1250 }, { 1250, 3000 } } },
                      Row{ { 100, 200 }, 20, 500, { { 500, 1500 }, { 1500, 2500 } } },
                      Row{ { 100, 200 }, 40, 0, { { 0, 1000 }, { 1000, 2000 } } },
                      Row{ { 3, 9 }, 20, 0, { { 0, 50 }, { 50, 100 } } },
                      Row{ { 1, 2 }, 20, 0, {} },
                      Row{ { 100, 100, 100 }, 20, 0, { { 0, 1000 } } },
                      Row{ { 100, 50, 200 }, 20, 0, { { 0, 1000 }, { 1000, 2000 } } },
                      Row{ { 0 }, 20, 0, {} });
    auto d = P(Doc("", Grid("T", r.cs)));
    LORMusic l(d, r.freq);
    CHECK(l.GetTimings("T", r.off) == r.expect);
}

TEST_CASE("LMS timing negative offsets", "[impexp2][lor][lms][timing][!mayfail]") {
    // BUG: `last` starts at the raw negative offset and is pushed as (uint32_t)last, so the first mark's segment starts at ~4 billion
    auto d = P(Doc("", Grid("T", { 100, 200 })));
    LORMusic l(d, 20);
    auto t = l.GetTimings("T", -500);
    REQUIRE(!t.empty());
    CHECK(t[0].first < 100000u);
}

TEST_CASE("LMS timing track with duplicate names uses the last grid", "[impexp2][lor][lms][timing]") {
    auto d = P(Doc("", Grid("T", { 100 }) + Grid("T", { 200, 300 })));
    LORMusic l(d, 20);
    CHECK(l.GetTimingTracks().size() == 2);
    auto t = l.GetTimings("T");
    REQUIRE(t.size() == 2);
    CHECK(t[0] == std::make_pair(0u, 2000u));
}

TEST_CASE("LMS timing at a frequency over 1000 does not divide by zero", "[impexp2][lor][lms][timing][.][known-crash]") {
    // BUG: RoundToMultipleOfPeriod: int(1000/period) == 0 for period > 1000, then divides by it
    auto d = P(Doc("", Grid("T", { 100 })));
    LORMusic l(d, 2000);
    CHECK_NOTHROW(l.GetTimings("T"));
}

// ---------- document level ----------

TEST_CASE("LMS documents of unusual shape", "[impexp2][lor][lms]") {
    for (const char* x : { "", "<", "<sequence/>", "<sequence><channels/></sequence>", "<sequence><timingGrids/></sequence>", "garbage", "<a><channels><channel/></channels></a>",
                           "<sequence><channels><channel name=\"A\"><effect/></channel></channels></sequence>",
                           "<sequence><channels><rgbChannel name=\"A\"><channels/></rgbChannel></channels></sequence>",
                           "<sequence><channels><rgbChannel name=\"A\"/></channels></sequence>",
                           "<sequence><channels><rgbChannel name=\"A\"><channels><channel/><channel/><channel/></channels></rgbChannel></channels></sequence>" }) {
        INFO(x);
        pugi::xml_document d;
        d.load_string(x);
        LORMusic l(d, 20);
        Fix f;
        (void)l.GetChannelNames();
        (void)l.GetCCRNames();
        (void)l.GetTimingTracks();
        (void)l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true);
        (void)l.MapChannelEffects(f.layer, "", xlWHITE, 1, true);
    }
    SUCCEED();
}

TEST_CASE("LMS rgb channel with three empty refs matches stray channels", "[impexp2][lor][lms][rgb]") {
    auto d = P("<sequence><channels><rgbChannel name=\"A\"><channels><channel/><channel/><channel/></channels></rgbChannel><channel name=\"s\">" + Eff("intensity", 0, 10, I(100)) + "</channel></channels></sequence>");
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 3, true));
    // all three colours alias the unindexed channel, which makes white
    CHECK(f.Count() == 1);
}

TEST_CASE("LMS large channel list indexes quickly", "[impexp2][lor][lms][perf]") {
    std::string chans;
    for (int i = 0; i < 3000; ++i) chans += Ch("Ch" + std::to_string(i), i, i % 10 == 0 ? Eff("intensity", 0, 10, I(50)) : "");
    auto d = P(Doc(chans));
    LORMusic l(d, 20);
    CHECK(l.GetChannelNames().size() == 3000);
    CHECK(l.GetChannelNames().front() == "Ch0");
    CHECK(l.GetChannelNames().back() == "Ch2999");
}

TEST_CASE("LMS many effects on one channel map in order", "[impexp2][lor][lms][perf]") {
    std::string effs;
    for (int i = 0; i < 2000; ++i) effs += Eff(i % 3 ? "intensity" : "shimmer", i * 10, i * 10 + 5, I(i % 101));
    auto d = P(Doc(Ch("A", 0, effs)));
    LORMusic l(d, 20);
    Fix f;
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
    CHECK(f.Count() == 2000);
    CHECK(f.At(1999)->GetStartTimeMS() == 199900);
}

TEST_CASE("LMS mapping into a layer that already has effects", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0, Eff("intensity", 0, 10, I(50)))));
    LORMusic l(d, 20);
    Fix f;
    f.layer->AddEffect(0, "Bars", "", "", 5000, 6000, false, false);
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, false));
    CHECK(f.Count() == 2);
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, true));
    CHECK(f.Count() == 1);
}

TEST_CASE("LMS mapping on top of an overlapping existing effect", "[impexp2][lor][lms]") {
    auto d = P(Doc(Ch("A", 0, Eff("intensity", 0, 10, I(50)))));
    LORMusic l(d, 20);
    Fix f;
    f.layer->AddEffect(0, "Bars", "", "", 0, 1000, false, false);
    REQUIRE(l.MapChannelEffects(f.layer, "A", xlWHITE, 1, false));
    CHECK(f.Count() >= 1);
}

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/LORPixelEditor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"

#include <pugixml.hpp>

#include <string>
#include <tuple>
#include <vector>

namespace {

struct Fix {
    HeadlessRenderContext ctx;
    Element* el = nullptr;
    Fix() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        el = se.AddElement("Model", "model", true, false, true, false, false);
        el->AddEffectLayer();
    }
    EffectLayer* L(int i) const { return el->GetEffectLayer(i); }
    int Layers() const { return (int)el->GetEffectLayerCount(); }
    int Total() const {
        int t = 0;
        for (int i = 0; i < Layers(); ++i) t += L(i)->GetEffectCount();
        return t;
    }
};

pugi::xml_document P(const std::string& xml) {
    pugi::xml_document d;
    d.load_buffer(xml.data(), xml.size(), pugi::parse_default, pugi::encoding_auto);
    return d;
}

std::string Side(const std::string& type, const std::string& params, const std::string& pal = "FFFF0000,1;FF00FF00,1") { return type + ":" + pal + ":" + params; }
const std::string kNoneSide = "none";

// blend|pos|sparkle|x|y|left|right (the 7 field form)
std::string PE7(const std::string& left, const std::string& right = kNoneSide, const std::string& blend = "Mix_Average", const std::string& pos = "0", const std::string& sparkle = "0") {
    return blend + "|" + pos + "|" + sparkle + "|0|0|" + left + "|" + right;
}
// blend|pos|sparkle|left|right (the 5 field form)
std::string PE5(const std::string& left, const std::string& right = kNoneSide, const std::string& blend = "Mix_Average", const std::string& pos = "0", const std::string& sparkle = "0") {
    return blend + "|" + pos + "|" + sparkle + "|" + left + "|" + right;
}
std::string E(int s, int e, const std::string& pe, int si = 100, int ei = 100) {
    return "<effect type=\"pixelEffect\" startCentisecond=\"" + std::to_string(s) + "\" endCentisecond=\"" + std::to_string(e) + "\" startIntensity=\"" + std::to_string(si) +
           "\" endIntensity=\"" + std::to_string(ei) + "\" pixelEffect=\"" + pe + "\"/>";
}
std::string T(int id, const std::string& effects) { return "<track id=\"" + std::to_string(id) + "\">" + effects + "</track>"; }
std::string Prop(const std::string& name, const std::string& tracks) { return "<SeqProp name=\"" + name + "\">" + tracks + "</SeqProp>"; }
std::string Doc(const std::string& props) { return "<?xml version=\"1.0\"?><LPE><SequenceProps>" + props + "</SequenceProps></LPE>"; }
std::string One(const std::string& pe, int s = 0, int e = 100, int si = 100, int ei = 100) { return Doc(Prop("P", T(0, E(s, e, pe, si, ei)))); }

bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }

struct Mapped {
    std::string name, settings, palette;
    int start = -1, end = -1;
    int count = 0;
};
Mapped Run(const std::string& pe, int s = 0, int e = 100, int si = 100, int ei = 100, int freq = 20) {
    auto d = P(One(pe, s, e, si, ei));
    LORPixelEditor l(d, freq);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    Mapped m;
    m.count = f.Total();
    if (f.L(0)->GetEffectCount() > 0) {
        auto* ef = f.L(0)->GetEffect(0);
        m.name = ef->GetEffectName();
        m.settings = ef->GetSettingsAsString();
        m.palette = ef->GetPaletteAsString();
        m.start = ef->GetStartTimeMS();
        m.end = ef->GetEndTimeMS();
    }
    return m;
}
}

#define HAS(m, sub)                                          \
    do {                                                     \
        INFO(m.settings);                                    \
        CHECK(m.settings.find(sub) != std::string::npos);    \
    } while (0)
#define HASNOT(m, sub)                                       \
    do {                                                     \
        INFO(m.settings);                                    \
        CHECK(m.settings.find(sub) == std::string::npos);    \
    } while (0)

// ---------- discovery ----------

TEST_CASE("LPE prop discovery", "[impexp2][lor][lpe]") {
    auto d = P("<LPE><SequenceProps><SeqProp name=\"B2\"/><SeqProp name=\"B10\"/><SeqProp name=\"A\"/><SeqProp/><SeqProp><PropClass Name=\"Nested\"/></SeqProp>"
               "<Other name=\"X\"/></SequenceProps><ArchivedProps><ArchiveProp name=\"Old\"/></ArchivedProps><Junk><SeqProp name=\"Z\"/></Junk></LPE>");
    LORPixelEditor l(d, 20);
    CHECK(l.GetChannelNames() == std::vector<std::string>{ "A", "B2", "B10", "Nested", "Old" });
    CHECK(l.GetChannelColor("A") == xlBLACK);
    CHECK(l.GetChannelColor("") == xlBLACK);
}

TEST_CASE("LPE empty and odd documents", "[impexp2][lor][lpe]") {
    for (const char* x : { "", "<", "<LPE/>", "<LPE><SequenceProps/></LPE>", "garbage", "<LPE><SequenceProps><SeqProp name=\"P\"><track/></SeqProp></SequenceProps></LPE>",
                           "<LPE><SequenceProps><SeqProp name=\"P\"><track id=\"0\"><effect/></track></SeqProp></SequenceProps></LPE>",
                           "<LPE><SequenceProps><SeqProp name=\"P\"><track id=\"0\"><effect type=\"pixelEffect\"/></track></SeqProp></SequenceProps></LPE>" }) {
        INFO(x);
        pugi::xml_document d;
        d.load_string(x);
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        CHECK(f.Total() == 0);
        (void)l.GetChannelNames();
    }
}

TEST_CASE("LPE null targets are ignored", "[impexp2][lor][lpe]") {
    auto d = P(One(PE7(Side("colorwash", "full,full,none"))));
    LORPixelEditor l(d, 20);
    l.MapPropEffects(nullptr, "P", true);
    l.MapPropNodeEffects(nullptr, "P", 0, true);
    SUCCEED();
}

TEST_CASE("LPE unknown prop maps nothing", "[impexp2][lor][lpe]") {
    auto d = P(One(PE7(Side("colorwash", "full,full,none"))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "Nope", true);
    l.MapPropEffects(f.el, "", true);
    CHECK(f.Total() == 0);
    CHECK(f.Layers() == 1);
}

// ---------- layers / sides ----------

TEST_CASE("LPE sides and tracks fan out to layers", "[impexp2][lor][lpe]") {
    std::string c = Side("colorwash", "full,full,none");
    std::string f7 = Side("fire", "50,0");
    auto d = P(Doc(Prop("P", T(0, E(0, 10, PE7(c, f7))) + T(1, E(20, 30, PE7(f7, c))))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    REQUIRE(f.Layers() == 4);
    REQUIRE(f.L(0)->GetEffectCount() == 1);
    REQUIRE(f.L(1)->GetEffectCount() == 1);
    REQUIRE(f.L(2)->GetEffectCount() == 1);
    REQUIRE(f.L(3)->GetEffectCount() == 1);
    CHECK(f.L(0)->GetEffect(0)->GetEffectName() == "Color Wash");
    CHECK(f.L(1)->GetEffect(0)->GetEffectName() == "Fire");
    CHECK(f.L(2)->GetEffect(0)->GetEffectName() == "Fire");
    CHECK(f.L(3)->GetEffect(0)->GetEffectName() == "Color Wash");
    CHECK(f.L(2)->GetEffect(0)->GetStartTimeMS() == 200);
}

TEST_CASE("LPE left only uses the first layer", "[impexp2][lor][lpe]") {
    auto m = Run(PE7(Side("fire", "50,0")));
    CHECK(m.name == "Fire");
    CHECK(m.count == 1);
}

TEST_CASE("LPE right only effect lands on the second layer", "[impexp2][lor][lpe]") {
    // the first layer stays empty because the layer counter advances for a right side even when no left side was mapped
    auto d = P(One(PE7(kNoneSide, Side("fire", "50,0"))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    REQUIRE(f.Layers() == 2);
    CHECK(f.L(0)->GetEffectCount() == 0);
    CHECK(f.L(1)->GetEffectCount() == 1);
}

TEST_CASE("LPE start layer offsets the mapping", "[impexp2][lor][lpe]") {
    auto d = P(One(PE7(Side("fire", "50,0"))));
    LORPixelEditor l(d, 20);
    Fix f;
    f.el->AddEffectLayer();
    f.el->AddEffectLayer();
    l.MapPropEffects(f.el, "P", true, 2);
    REQUIRE(f.Layers() == 3);
    CHECK(f.L(2)->GetEffectCount() == 1);
    CHECK(f.L(0)->GetEffectCount() == 0);
}

TEST_CASE("LPE five field pixel effects use fields three and four", "[impexp2][lor][lpe]") {
    auto d = P(One(PE5(Side("fire", "50,0"), Side("bars", "up,1,False,False,8,0"))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    REQUIRE(f.Layers() == 2);
    CHECK(f.L(0)->GetEffect(0)->GetEffectName() == "Fire");
    CHECK(f.L(1)->GetEffect(0)->GetEffectName() == "Bars");
}

TEST_CASE("LPE erase existing clears the layer", "[impexp2][lor][lpe]") {
    auto d = P(One(PE7(Side("fire", "50,0")), 0, 100));
    LORPixelEditor l(d, 20);
    Fix f;
    f.L(0)->AddEffect(0, "Bars", "", "", 5000, 6000, false, false);
    l.MapPropEffects(f.el, "P", false);
    CHECK(f.L(0)->GetEffectCount() == 2);
    l.MapPropEffects(f.el, "P", true);
    CHECK(f.L(0)->GetEffectCount() == 1);
}

TEST_CASE("LPE existing effects block overlapping imports", "[impexp2][lor][lpe]") {
    auto d = P(One(PE7(Side("fire", "50,0")), 0, 100));
    LORPixelEditor l(d, 20);
    Fix f;
    f.L(0)->AddEffect(0, "Bars", "", "", 500, 1500, false, false);
    l.MapPropEffects(f.el, "P", false);
    CHECK(f.L(0)->GetEffectCount() == 1);
}

TEST_CASE("LPE types other than pixelEffect are ignored", "[impexp2][lor][lpe]") {
    std::string ef = "<effect type=\"intensity\" startCentisecond=\"0\" endCentisecond=\"10\" pixelEffect=\"" + PE7(Side("fire", "50,0")) + "\"/><junk/>";
    auto d = P(Doc(Prop("P", T(0, ef))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    CHECK(f.Total() == 0);
}

TEST_CASE("LPE tracks other than 0 and 1 are not mapped", "[impexp2][lor][lpe]") {
    auto d = P(Doc(Prop("P", T(2, E(0, 10, PE7(Side("fire", "50,0")))) + T(-1, E(0, 10, PE7(Side("fire", "50,0")))))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    CHECK(f.Total() == 0);
}

TEST_CASE("LPE node mapping uses the left side of the chosen track", "[impexp2][lor][lpe]") {
    auto d = P(Doc(Prop("P", T(0, E(0, 10, PE7(Side("fire", "50,0"), Side("bars", "up,1,False,False,8,0")))) + T(1, E(20, 30, PE7(Side("colorwash", "full,full,none")))))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropNodeEffects(f.L(0), "P", 0, true);
    REQUIRE(f.L(0)->GetEffectCount() == 1);
    CHECK(f.L(0)->GetEffect(0)->GetEffectName() == "Fire");
    l.MapPropNodeEffects(f.L(0), "P", 1, true);
    REQUIRE(f.L(0)->GetEffectCount() == 1);
    CHECK(f.L(0)->GetEffect(0)->GetEffectName() == "Color Wash");
    l.MapPropNodeEffects(f.L(0), "P", 9, true);
    CHECK(f.L(0)->GetEffectCount() == 0);
}

TEST_CASE("LPE unmapped or ignored effect names", "[impexp2][lor][lpe]") {
    for (const char* t : { "none", "lineshorizontal", "linesvertical" }) {
        INFO(t);
        auto m = Run(PE7(Side(t, "1,2,3")));
        CHECK(m.count == 0);
    }
}

// ---------- names ----------

TEST_CASE("LPE effect names", "[impexp2][lor][lpe]") {
    struct Row { const char* lpe; const char* params; const char* name; };
    auto r = GENERATE(Row{ "colorwash", "full,full,none", "Color Wash" }, Row{ "picture", "a.png,True,none,0,10,19,12", "Pictures" }, Row{ "countdown", "5,Arial,75,7", "Text" },
                      Row{ "fire", "50,0", "Fire" }, Row{ "BARS", "up,1,False,False,8,0", "Bars" }, Row{ "meteors", "rainbow,10,25,down,0,12", "Meteors" },
                      Row{ "snowflakes", "5,1,0,12,60", "Snowflakes" }, Row{ "twinkle", "50,25,twinkle,random", "Twinkle" }, Row{ "text", "x,50,left,0,10,0,4", "Text" },
                      Row{ "curtain", "center,open,0,once_at_speed,12", "Curtain" }, Row{ "spirals", "1,left_to_right,20,50,0,False,none,12", "Spirals" },
                      Row{ "butterfly", "linear,5,3,forward,0,10,rainbow", "Butterfly" }, Row{ "fireworks", "10,50,2,30,normal,continuous", "Fireworks" },
                      Row{ "pinwheel", "3,1,6,x,True,12,100,10,-23", "Pinwheel" });
    auto m = Run(PE7(Side(r.lpe, r.params)));
    CHECK(m.name == r.name);
}

TEST_CASE("LPE garland and movie are named as xLights effects", "[impexp2][lor][lpe]") {
    // BUG: MapLPEEffectType only special cases colorwash/picture/countdown; "garland" -> "Garland" (xLights: "Garlands")
    // and "movie" -> "Movie" (xLights: "Video"), neither of which exists
    // The effects are not even added: AddEffect refuses the unknown names so they vanish from the import
    CHECK(Run(PE7(Side("garland", "3,34,once_at_speed,12,bottom_to_top"))).count == 1);
    CHECK(Run(PE7(Side("movie", "x.avi,True,False"))).count == 1);
    CHECK(Run(PE7(Side("garland", "3,34,once_at_speed,12,bottom_to_top"))).name == "Garlands");
    CHECK(Run(PE7(Side("movie", "x.avi,True,False"))).name == "Video");
}

// ---------- timing / intensity ----------

TEST_CASE("LPE times round to the frame period", "[impexp2][lor][lpe][timing]") {
    struct Row { int s, e, freq, es, ee; };
    auto r = GENERATE(Row{ 0, 100, 20, 0, 1000 }, Row{ 3, 9, 20, 50, 100 }, Row{ 3, 8, 40, 25, 75 }, Row{ 10, 20, 50, 100, 200 }, Row{ 0, 1, 20, 0, 0 });
    auto m = Run(PE7(Side("fire", "50,0")), r.s, r.e, 100, 100, r.freq);
    INFO(r.s << " " << r.e);
    if (r.es == r.ee) return;
    CHECK(m.start == r.es);
    CHECK(m.end == r.ee);
}

TEST_CASE("LPE zero length effects", "[impexp2][lor][lpe][timing]") {
    auto m = Run(PE7(Side("fire", "50,0")), 10, 10);
    CHECK(m.count <= 1);
}

TEST_CASE("LPE backwards effects do not crash", "[impexp2][lor][lpe][timing]") {
    auto m = Run(PE7(Side("fire", "50,0")), 50, 10);
    CHECK(m.count <= 1);
}

TEST_CASE("LPE negative times do not crash", "[impexp2][lor][lpe][timing]") {
    auto m = Run(PE7(Side("fire", "50,0")), -50, -10);
    CHECK(m.count <= 1);
}

TEST_CASE("LPE brightness and fades", "[impexp2][lor][lpe][intensity]") {
    SECTION("full intensity adds nothing") {
        auto m = Run(PE7(Side("fire", "50,0")), 0, 100, 100, 100);
        CHECK_FALSE(Has(m.palette, "Brightness"));
        HASNOT(m, "Fadein");
    }
    SECTION("constant dimmer") {
        auto m = Run(PE7(Side("fire", "50,0")), 0, 100, 40, 40);
        CHECK(Has(m.palette, "C_SLIDER_Brightness=40"));
    }
    SECTION("ramp up on its own is a fade in over the effect") {
        auto m = Run(PE7(Side("fire", "50,0")), 0, 100, 0, 100);
        HAS(m, "T_TEXTCTRL_Fadein=1.00");
    }
    SECTION("ramp down on its own is a fade out over the effect") {
        auto m = Run(PE7(Side("fire", "50,0")), 50, 150, 100, 0);
        HAS(m, "T_TEXTCTRL_Fadeout=1.00");
    }
    SECTION("middling ramp becomes a value curve") {
        auto m = Run(PE7(Side("fire", "50,0")), 0, 100, 20, 60);
        INFO(m.palette);
        CHECK(Has(m.palette, "C_VALUECURVE_Brightness"));
        HASNOT(m, "Fade");
    }
    SECTION("missing intensities default to full") {
        auto d = P(Doc(Prop("P", T(0, "<effect type=\"pixelEffect\" startCentisecond=\"0\" endCentisecond=\"100\" pixelEffect=\"" + PE7(Side("fire", "50,0")) + "\"/>"))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        REQUIRE(f.Total() == 1);
        CHECK_FALSE(Has(f.L(0)->GetEffect(0)->GetPaletteAsString(), "Brightness"));
    }
}

TEST_CASE("LPE adjacent identical effects merge into one with fades", "[impexp2][lor][lpe][intensity]") {
    std::string pe = PE7(Side("fire", "50,0"));
    SECTION("fade in then hold") {
        auto d = P(Doc(Prop("P", T(0, E(0, 10, pe, 0, 100) + E(10, 30, pe, 100, 100)))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        REQUIRE(f.L(0)->GetEffectCount() == 1);
        auto* ef = f.L(0)->GetEffect(0);
        CHECK(ef->GetStartTimeMS() == 0);
        CHECK(ef->GetEndTimeMS() == 300);
        CHECK(Has(ef->GetSettingsAsString(), "T_TEXTCTRL_Fadein=0.10"));
    }
    SECTION("hold then fade out") {
        auto d = P(Doc(Prop("P", T(0, E(0, 20, pe, 100, 100) + E(20, 30, pe, 100, 0)))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        REQUIRE(f.L(0)->GetEffectCount() == 1);
        auto* ef = f.L(0)->GetEffect(0);
        CHECK(ef->GetEndTimeMS() == 300);
        CHECK(Has(ef->GetSettingsAsString(), "T_TEXTCTRL_Fadeout=0.10"));
    }
    SECTION("fade in, hold and fade out") {
        auto d = P(Doc(Prop("P", T(0, E(0, 10, pe, 0, 100) + E(10, 20, pe, 100, 100) + E(20, 30, pe, 100, 0)))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        REQUIRE(f.L(0)->GetEffectCount() == 1);
        auto s = f.L(0)->GetEffect(0)->GetSettingsAsString();
        CHECK(Has(s, "T_TEXTCTRL_Fadein=0.10"));
        CHECK(Has(s, "T_TEXTCTRL_Fadeout=0.10"));
    }
    SECTION("different settings do not merge") {
        auto d = P(Doc(Prop("P", T(0, E(0, 10, pe) + E(10, 20, PE7(Side("fire", "60,0")))))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        CHECK(f.L(0)->GetEffectCount() == 2);
    }
    SECTION("same settings and intensity merge") {
        auto d = P(Doc(Prop("P", T(0, E(0, 10, pe) + E(10, 20, pe) + E(20, 30, pe)))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        REQUIRE(f.L(0)->GetEffectCount() == 1);
        CHECK(f.L(0)->GetEffect(0)->GetEndTimeMS() == 300);
    }
    SECTION("a gap in time still merges identical neighbours") {
        auto d = P(Doc(Prop("P", T(0, E(0, 10, pe) + E(50, 60, pe)))));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        REQUIRE(f.L(0)->GetEffectCount() == 1);
        CHECK(f.L(0)->GetEffect(0)->GetEndTimeMS() == 600);
    }
}

// ---------- blend / sparkle / palette ----------

TEST_CASE("LPE blends apply to the left side only", "[impexp2][lor][lpe][blend]") {
    struct Row { const char* mix; const char* left; };
    auto r = GENERATE(Row{ "Mix_Average", "Average" }, Row{ "Mix_Overlay", "Normal" }, Row{ "Mix_Maximum", "Max" }, Row{ "Mix_Bottom_Top", "Bottom-Top" }, Row{ "Mix_Left-Right", "Left-Right" },
                      Row{ "Mix_Rt_Hides_Lt", "1 is Mask" }, Row{ "Mix_Rt_Reveals_Lt", "2 reveals 1" }, Row{ "Mix_Bogus", "Normal" }, Row{ "", "Normal" });
    auto d = P(One(PE7(Side("fire", "50,0"), Side("fire", "50,0"), r.mix)));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    REQUIRE(f.Layers() == 2);
    CHECK(Has(f.L(0)->GetEffect(0)->GetSettingsAsString(), std::string("T_CHOICE_LayerMethod=") + r.left));
    CHECK(Has(f.L(1)->GetEffect(0)->GetSettingsAsString(), "T_CHOICE_LayerMethod=Normal"));
}

TEST_CASE("LPE blend position becomes the layer mix on the left side", "[impexp2][lor][lpe][blend]") {
    auto d = P(One(PE7(Side("fire", "50,0"), Side("fire", "50,0"), "Mix_Average", "35")));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    REQUIRE(f.Layers() == 2);
    CHECK(Has(f.L(0)->GetEffect(0)->GetSettingsAsString(), "T_SLIDER_EffectLayerMix=35"));
    CHECK_FALSE(Has(f.L(1)->GetEffect(0)->GetSettingsAsString(), "EffectLayerMix"));
}

TEST_CASE("LPE blend position of zero or negative adds no mix", "[impexp2][lor][lpe][blend]") {
    for (const char* p : { "0", "-5", "abc", "" }) {
        INFO(p);
        auto m = Run(PE7(Side("fire", "50,0"), kNoneSide, "Mix_Average", p));
        HASNOT(m, "EffectLayerMix");
    }
}

TEST_CASE("LPE sparkle", "[impexp2][lor][lpe][blend]") {
    auto m = Run(PE7(Side("fire", "50,0"), kNoneSide, "Mix_Average", "0", "25"));
    CHECK(Has(m.palette, "C_SLIDER_SparkleFrequency=25"));
    auto z = Run(PE7(Side("fire", "50,0"), kNoneSide, "Mix_Average", "0", "0"));
    CHECK_FALSE(Has(z.palette, "Sparkle"));
}

TEST_CASE("LPE palette parsing", "[impexp2][lor][lpe][palette]") {
    struct Row { const char* pal; const char* has; const char* hasNot; };
    auto r = GENERATE(Row{ "FFFF0000,1", "C_BUTTON_Palette1=#FF0000", "Palette2" }, Row{ "FFFF0000,1;FF00FF00,1", "C_BUTTON_Palette2=#00FF00", "" },
                      Row{ "FFFF0000,0", "C_BUTTON_Palette1=#FF0000", "C_CHECKBOX_Palette1" }, Row{ "FFFF0000,1", "C_CHECKBOX_Palette1=1", "" },
                      Row{ "FFFF0000,FFFF0000,1", "C_BUTTON_Palette1=#FF0000", "Values" }, Row{ "", "", "C_BUTTON" }, Row{ "junk", "", "C_BUTTON" },
                      Row{ "FFFF0000,1;x;FF0000FF,1", "Palette1", "Palette2" });
    auto m = Run(PE7(Side("fire", "50,0", r.pal)));
    INFO(m.palette);
    if (*r.has) CHECK(Has(m.palette, r.has));
    if (*r.hasNot) CHECK_FALSE(Has(m.palette, r.hasNot));
}

TEST_CASE("LPE gradient palette entries are comma separated", "[impexp2][lor][lpe][palette]") {
    // BUG: ExtractLPEPallette emits "C_BUTTON_PaletteN=Active=TRUE|..." without the leading comma that every other entry has,
    // so after an earlier entry it is glued onto that entry's checkbox value
    auto d = P(One(PE7(Side("fire", "50,0", "FF000000,1;FFFF0000,FF00FF00,1"))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true);
    REQUIRE(f.Total() == 1);
    auto pm = f.L(0)->GetEffect(0)->GetPaletteMap();
    CHECK(pm.Get("C_CHECKBOX_Palette1", "") == "1");
    CHECK(pm.Get("C_BUTTON_Palette2", "").rfind("Active=TRUE", 0) == 0);
}

TEST_CASE("LPE gradient palette on its own", "[impexp2][lor][lpe][palette]") {
    auto m = Run(PE7(Side("fire", "50,0", "FFFF0000,FF00FF00,1")));
    CHECK(Has(m.palette, "C_BUTTON_Palette1=Active=TRUE"));
}

TEST_CASE("LPE palette entries too short to hold a colour do not throw", "[impexp2][lor][lpe][palette]") {
    // BUG: cc[0].substr(2) with no size check throws std::out_of_range for "F,1" or ",1"
    auto d = P(One(PE7(Side("fire", "50,0", "F,1"))));
    LORPixelEditor l(d, 20);
    Fix f;
    CHECK_NOTHROW(l.MapPropEffects(f.el, "P", true));
}

// ---------- effect settings ----------

#define LPE_CASE(NAME, TYPE, PARAMS, ...)                                              \
    TEST_CASE("LPE convert " NAME, "[impexp2][lor][lpe][effects]") {                   \
        auto m = Run(PE7(Side(TYPE, PARAMS)));                                         \
        REQUIRE(m.count == 1);                                                         \
        __VA_ARGS__                                                                    \
    }

LPE_CASE("butterfly", "butterfly", "linear,5,3,forward,0,10,rainbow", HAS(m, "E_SLIDER_Butterfly_Style=1"); HAS(m, "E_CHOICE_Butterfly_Colors=Rainbow");
         HAS(m, "E_CHOICE_Butterfly_Direction=Forward"); HAS(m, "E_SLIDER_Butterfly_Chunks=5"); HAS(m, "E_SLIDER_Butterfly_Skip=3"); HAS(m, "E_SLIDER_Butterfly_Speed=10");)
LPE_CASE("butterfly styles", "butterfly", "radial,5,3,forward,359,10,palette", HAS(m, "Butterfly_Style=2"); CHECK(Has(m.palette, "C_SLIDER_Color_HueAdjust=100"));)
LPE_CASE("butterfly blocks and corner", "butterfly", "blocks,5,3,forward,359,10,palette", HAS(m, "Butterfly_Style=3");)
LPE_CASE("butterfly corner", "butterfly", "corner,5,3,forward,359,10,palette", HAS(m, "Butterfly_Style=5");)
LPE_CASE("butterfly clamps", "butterfly", "linear,99,99,forward,0,999,rainbow", HAS(m, "Chunks=10"); HAS(m, "Skip=10"); HAS(m, "Speed=50");)

LPE_CASE("colorwash full", "colorwash", "full,full,none", HASNOT(m, "HFade"); HASNOT(m, "VFade");)
LPE_CASE("colorwash fades", "colorwash", "left_to_right,top_to_bottom,none", HAS(m, "E_CHECKBOX_ColorWash_HFade=1"); HAS(m, "E_CHECKBOX_ColorWash_VFade=1");)

LPE_CASE("spirals", "spirals", "1,left_to_right,20,50,0,False,trail_left,50",
         HAS(m, "E_SLIDER_Spirals_Count=1"); HAS(m, "E_SLIDER_Spirals_Thickness=50"); HAS(m, "E_CHECKBOX_Spirals_3D=1"); HASNOT(m, "Blend"); HAS(m, "E_TEXTCTRL_Spirals_Movement=");)
LPE_CASE("spirals right to left and 3d off", "spirals", "3,right_to_left,20,50,0,False,none,50", HAS(m, "E_TEXTCTRL_Spirals_Movement=-"); HASNOT(m, "Spirals_3D");)

TEST_CASE("LPE spirals blend setting survives the trailing comma", "[impexp2][lor][lpe][effects]") {
    // the blend branch appends ",E_CHECKBOX_Spirals_Blend=1," with a trailing comma (harmless: the settings parser drops the empty entry)
    auto m = Run(PE7(Side("spirals", "1,left_to_right,20,50,0,True,none,50")));
    HAS(m, "E_CHECKBOX_Spirals_Blend=1");
    HASNOT(m, ",,");
}

LPE_CASE("bars", "bars", "right,1,False,True,19,0", HAS(m, "E_CHOICE_Bars_Direction=Right"); HAS(m, "E_SLIDER_Bars_BarCount=1"); HAS(m, "E_CHECKBOX_Bars_3D=1"); HAS(m, "E_TEXTCTRL_Bars_Cycles=");
         HAS(m, "E_TEXTCTRL_Bars_Center=0");)
LPE_CASE("bars expand", "bars", "V_expand,2,False,False,8,0", HAS(m, "Bars_Direction=expand");)
LPE_CASE("bars h compress", "bars", "H_compress,2,False,False,8,0", HAS(m, "Bars_Direction=H-compress");)
LPE_CASE("bars block up", "bars", "block_up,2,False,False,8,0", HAS(m, "Bars_Direction=Alternate Up");)
LPE_CASE("bars block down", "bars", "block_down,2,False,False,8,0", HAS(m, "Bars_Direction=Alternate Down");)
LPE_CASE("bars block right", "bars", "block_right,2,False,False,8,0", HAS(m, "Bars_Direction=Alternate Right");)

TEST_CASE("LPE bars block left is Alternate Left", "[impexp2][lor][lpe][effects]") {
    // BUG: block_left is mapped to "Alternate Right" (copy and paste; LOREdit maps it to Alternate Left)
    auto m = Run(PE7(Side("bars", "block_left,2,False,False,8,0")));
    HAS(m, "Bars_Direction=Alternate Left");
}

TEST_CASE("LPE bars highlight keeps its own setting separator", "[impexp2][lor][lpe][effects]") {
    // BUG: "E_CHECKBOX_Bars_Highlight=1" is appended without a leading comma so it is glued onto the direction value
    auto m = Run(PE7(Side("bars", "down,2,True,False,8,0")));
    HAS(m, "E_CHECKBOX_Bars_Highlight=1");
    HAS(m, "E_CHOICE_Bars_Direction=down,");
}

LPE_CASE("countdown", "countdown", "5,Arial,75,7", HAS(m, "E_TEXTCTRL_Text=5"); HAS(m, "E_CHOICE_Text_Count=seconds"); HAS(m, "E_FONTPICKER_Text_Font='Arial' 75"); HAS(m, "E_SLIDER_Text_XStart=28");)
LPE_CASE("curtain", "curtain", "center,open_then_close,0,once_at_speed,12", HAS(m, "E_CHOICE_Curtain_Edge=center"); HAS(m, "E_CHOICE_Curtain_Effect=open then close");
         HAS(m, "E_CHECKBOX_Curtain_Repeat=0"); HAS(m, "E_TEXTCTRL_Curtain_Speed=2.4");)
LPE_CASE("curtain repeat", "curtain", "left,open,2,repeat_at_speed_rotate_colors,12", HAS(m, "Curtain_Repeat=1");)
LPE_CASE("curtain unknown repeat", "curtain", "left,open,2,sometimes,12", HASNOT(m, "Curtain_Repeat");)
LPE_CASE("fire", "fire", "50,0", HAS(m, "E_SLIDER_Fire_Height=44"); HAS(m, "E_SLIDER_Fire_HueShift=0");)
LPE_CASE("fireworks", "fireworks", "10,50,2,30,normal,continuous", HAS(m, "Fireworks_Explosions=5"); HAS(m, "Fireworks_Count=50"); HAS(m, "Fireworks_Velocity=2"); HAS(m, "Fireworks_Fade=30");)
LPE_CASE("meteors", "meteors", "RAINBOW,10,25,down,0,12", HAS(m, "E_CHOICE_Meteors_Type=rainbow"); HAS(m, "Meteors_Count=10"); HAS(m, "Meteors_Effect=Down"); HAS(m, "Meteors_Speed=12");)
LPE_CASE("picture", "picture", "file.jpg,True,none,0,10,19,12", HAS(m, "Pictures_Filename=file.jpg"); HAS(m, "Scaling=Scale To Fit"); HAS(m, "Pictures_Direction=none"); HAS(m, "Pictures_Speed=4.8");)
LPE_CASE("picture no scaling", "picture", "file.jpg,False,peekaboo_top,0,10,19,12", HAS(m, "Scaling=No Scaling"); HAS(m, "Pictures_Direction=peekaboo 180");)
LPE_CASE("picture peekaboo others", "picture", "file.jpg,False,peekaboo_left,0,10,19,12", HAS(m, "peekaboo 90");)
LPE_CASE("pinwheel", "pinwheel", "3,1,6,color_per_arm,True,12,100,10,-23", HAS(m, "Pinwheel_Arms=3"); HAS(m, "Pinwheel_Twist=216"); HAS(m, "Pinwheel_Rotation=1"); HAS(m, "Pinwheel_Speed=12"); HAS(m, "PinwheelXC=20");)
LPE_CASE("snowflakes", "snowflakes", "5,1,0,12,60", HAS(m, "Snowflakes_Count=5"); HAS(m, "Snowflakes_Type=1"); HAS(m, "Snowflakes_Speed=12"); HAS(m, "Falling &amp; Accumulating");)
LPE_CASE("snowflakes falling", "snowflakes", "5,1,0,12,0", HAS(m, "E_CHOICE_Falling=Falling"); HASNOT(m, "Accumulating");)
LPE_CASE("text", "text", "Hello,50,left,0,10,0,4", HAS(m, "E_TEXTCTRL_Text=Hello"); HAS(m, "E_FONTPICKER_Text_Font='segoe ui' 50"); HAS(m, "E_CHOICE_Text_Dir=left"); HAS(m, "Text_Speed=4");)
LPE_CASE("text encoded", "text", "a%20b%26amp%3Bc,50,left,0,10,0,4", HAS(m, "E_TEXTCTRL_Text=a b&amp;c");)
LPE_CASE("text peekaboo", "text", "x,50,peekaboo_bottom,0,10,0,4", HAS(m, "TextToCenter=1"); HAS(m, "Text_Dir=up");)
LPE_CASE("twinkle", "twinkle", "50,25,twinkle,random", HAS(m, "Twinkle_Count=25"); HAS(m, "Twinkle_Steps=50"); HAS(m, "Twinkle_ReRandom=1"); HAS(m, "Twinkle_Strobe=0");)
LPE_CASE("twinkle pulse", "twinkle", "50,25,pulse,interval", HAS(m, "Twinkle_ReRandom=0"); HAS(m, "Twinkle_Strobe=1");)

TEST_CASE("LPE text with an escape at the very end of the string", "[impexp2][lor][lpe][effects]") {
    auto m = Run(PE7(Side("text", "ab%41,50,left,0,10,0,4")));
    REQUIRE(m.count == 1);
    HAS(m, "E_TEXTCTRL_Text=abA");
}

TEST_CASE("LPE text with a truncated escape", "[impexp2][lor][lpe][effects]") {
    auto m = Run(PE7(Side("text", "ab%4,50,left,0,10,0,4")));
    REQUIRE(m.count == 1);
    HAS(m, "E_TEXTCTRL_Text=ab%4");
}

TEST_CASE("LPE start layer beyond the model's layers", "[impexp2][lor][lpe]") {
    // BUG: MapPropEffects adds a single layer when `count < layer + 1`, so a start layer two or more past the end leaves
    // GetEffectLayer(layer) out of range (null deref)
    auto d = P(One(PE7(Side("fire", "50,0"))));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropEffects(f.el, "P", true, 2);
    CHECK(f.Total() == 1);
}

// ---------- crashers (out of range reads on short parameter lists) ----------

TEST_CASE("LPE effect with fewer parameters than the effect needs", "[impexp2][lor][lpe][effects]") {
    // BUG: LPEParseEffectSettings splits the parameters with no padding (LOREdit pads to 24) and indexes parms[1].. directly
    auto m = Run(PE7(Side("fire", "50")));
    CHECK(m.count <= 1);
}

TEST_CASE("LPE effect with palette but no parameter field", "[impexp2][lor][lpe][effects]") {
    // BUG: `arrSettings.size() > 1` guards but arrSettings[2] is read
    auto m = Run(PE7("fire:FFFF0000,1"));
    CHECK(m.count <= 1);
}

TEST_CASE("LPE effect with only a type", "[impexp2][lor][lpe][effects]") {
    // BUG: ExtractLPEPallette reads ps[1] with no size check
    auto m = Run(PE7("fire"));
    CHECK(m.count <= 1);
}

TEST_CASE("LPE node mapping of a pixel effect string with too few fields", "[impexp2][lor][lpe]") {
    // BUG: MapLPE indexes settingsArray[3]/[4] (and [0..2]) after only checking for size 7; MapPropEffects guards with HasEffects but
    // MapPropNodeEffects does not
    auto d = P(One("a|b"));
    LORPixelEditor l(d, 20);
    Fix f;
    l.MapPropNodeEffects(f.L(0), "P", 0, true);
    CHECK(f.Total() == 0);
}

TEST_CASE("LPE pixel effect strings of the wrong shape are skipped by the prop mapping", "[impexp2][lor][lpe]") {
    for (const char* pe : { "", "a", "a|b|c", "a|b|c|d", "a|b|c|d|e|f", "a|b|c|d|e|f|g|h" }) {
        INFO(pe);
        auto d = P(One(pe));
        LORPixelEditor l(d, 20);
        Fix f;
        l.MapPropEffects(f.el, "P", true);
        CHECK(f.Total() == 0);
    }
}

// ---------- scale ----------

TEST_CASE("LPE many effects across many props", "[impexp2][lor][lpe][perf]") {
    std::string effs;
    for (int i = 0; i < 3000; ++i) effs += E(i * 10, i * 10 + 5, PE7(Side("fire", std::to_string(10 + i % 80) + ",0")), 100, 100);
    std::string props;
    for (int p = 0; p < 20; ++p) props += Prop("P" + std::to_string(p), T(0, p == 0 ? effs : E(0, 10, PE7(Side("fire", "50,0")))));
    auto d = P(Doc(props));
    LORPixelEditor l(d, 20);
    CHECK(l.GetChannelNames().size() == 20);
    Fix f;
    l.MapPropEffects(f.el, "P0", true);
    CHECK(f.L(0)->GetEffectCount() == 3000);
}

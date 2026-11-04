#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <string>
#include <vector>

#include "render/EffectLayer.h"
#include "render/Effect.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "utils/UtilClasses.h"

namespace {

struct EffectFixture {
    HeadlessRenderContext ctx;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;

    EffectFixture() {
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    Effect* Make(const std::string& name, const std::string& settings, const std::string& palette = "",
                 int startMs = 0, int endMs = 1000) {
        return layer->AddEffect(0, name, settings, palette, startMs, endMs, 0, false);
    }
};

} // namespace

TEST_CASE("SettingsMap Get returns the default for a missing key", "[effects][settingsmap]") {
    SettingsMap sm;
    CHECK(sm.Get("E_TEXTCTRL_Missing", "dflt") == "dflt");
    CHECK(sm.Get(std::string("E_TEXTCTRL_Missing"), std::string("d2")) == "d2");
    CHECK(sm.GetInt("E_SLIDER_Missing", 7) == 7);
    CHECK(sm.GetFloat("E_SLIDER_Missing", 1.5f) == 1.5f);
    CHECK(sm.GetDouble("E_SLIDER_Missing", 2.5) == 2.5);
    CHECK(sm.GetBool("E_CHECKBOX_Missing", true));
    CHECK_FALSE(sm.GetBool("E_CHECKBOX_Missing", false));
    CHECK_FALSE(sm.Contains("E_SLIDER_Missing"));
    CHECK(sm.empty());
}

TEST_CASE("SettingsMap Set and Get round trip", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["E_CHOICE_Direction"] = "Up";
    CHECK(sm.Get("E_CHOICE_Direction", "") == "Up");
    CHECK(sm["E_CHOICE_Direction"] == "Up");
    CHECK(sm.Contains("E_CHOICE_Direction"));
    CHECK(sm.size() == 1);
    sm["E_CHOICE_Direction"] = "Down";
    CHECK(sm.Get("E_CHOICE_Direction", "") == "Down");
    CHECK(sm.size() == 1);
}

TEST_CASE("SettingsMap keys are case sensitive", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["E_SLIDER_Speed"] = "5";
    CHECK(sm.Contains("E_SLIDER_Speed"));
    CHECK_FALSE(sm.Contains("e_slider_speed"));
    CHECK_FALSE(sm.Contains("E_SLIDER_speed"));
    CHECK(sm.GetInt("E_SLIDER_SPEED", -1) == -1);
}

TEST_CASE("SettingsMap const operator[] on a missing key does not insert", "[effects][settingsmap]") {
    SettingsMap sm;
    const SettingsMap& c = sm;
    CHECK(c["E_SLIDER_Nope"].empty());
    CHECK(c.size() == 0);
}

TEST_CASE("SettingsMap non-const operator[] inserts an empty value", "[effects][settingsmap]") {
    SettingsMap sm;
    std::string v = sm["E_SLIDER_Probe"];
    CHECK(v.empty());
    CHECK(sm.Contains("E_SLIDER_Probe"));
    CHECK(sm.size() == 1);
}

TEST_CASE("SettingsMap GetInt parses integers", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["a"] = "42";
    sm["b"] = "-17";
    sm["c"] = "0";
    sm["d"] = "  8";
    sm["e"] = "12abc";
    CHECK(sm.GetInt("a", -1) == 42);
    CHECK(sm.GetInt("b", -1) == -17);
    CHECK(sm.GetInt("c", -1) == 0);
    CHECK(sm.GetInt("d", -1) == 8);
    CHECK(sm.GetInt("e", -1) == 12);
}

TEST_CASE("SettingsMap GetInt falls back to default for bad values", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["empty"] = "";
    sm["text"] = "hello";
    sm["huge"] = "99999999999999999999";
    sm["big"] = "4294967296";
    sm["neg"] = "-4294967296";
    CHECK(sm.GetInt("empty", 5) == 5);
    CHECK(sm.GetInt("text", 6) == 6);
    CHECK(sm.GetInt("huge", 7) == 7);
    CHECK(sm.GetInt("big", 8) == 8);
    CHECK(sm.GetInt("neg", 9) == 9);
}

TEST_CASE("SettingsMap GetInt truncates a decimal value", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["x"] = "3.9";
    CHECK(sm.GetInt("x", -1) == 3);
}

TEST_CASE("SettingsMap GetFloat and GetDouble parse decimals", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["a"] = "1.5";
    sm["b"] = "-0.25";
    sm["c"] = "3";
    sm["d"] = "abc";
    sm["e"] = "";
    sm["f"] = "1e2";
    CHECK(sm.GetFloat("a", 0) == Catch::Approx(1.5f));
    CHECK(sm.GetFloat("b", 0) == Catch::Approx(-0.25f));
    CHECK(sm.GetFloat("c", 0) == Catch::Approx(3.0f));
    CHECK(sm.GetFloat("d", 9.5f) == Catch::Approx(9.5f));
    CHECK(sm.GetFloat("e", 8.5f) == Catch::Approx(8.5f));
    CHECK(sm.GetFloat("f", 0) == Catch::Approx(100.0f));
    CHECK(sm.GetDouble("a", 0) == Catch::Approx(1.5));
    CHECK(sm.GetDouble("b", 0) == Catch::Approx(-0.25));
    CHECK(sm.GetDouble("d", 9.5) == Catch::Approx(9.5));
    CHECK(sm.GetDouble("e", 8.5) == Catch::Approx(8.5));
}

TEST_CASE("SettingsMap GetBool accepts the 1/0 and True/False conventions", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["one"] = "1";
    sm["zero"] = "0";
    sm["T"] = "True";
    sm["F"] = "False";
    sm["junk"] = "yes";
    CHECK(sm.GetBool("one", false));
    CHECK_FALSE(sm.GetBool("zero", true));
    CHECK(sm.GetBool("T", false));
    CHECK_FALSE(sm.GetBool("F", true));
    CHECK_FALSE(sm.GetBool("junk", true));
}

TEST_CASE("SettingsMap GetBool on a present but empty value is false", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["k"] = "";
    CHECK_FALSE(sm.GetBool("k", true));
    CHECK_FALSE(sm.GetBool("k", false));
}

TEST_CASE("SettingsMap typed caches are invalidated by reassignment", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["k"] = "1";
    CHECK(sm.GetBool("k", false));
    CHECK(sm.GetInt("k", 0) == 1);
    sm["k"] = "0";
    CHECK_FALSE(sm.GetBool("k", true));
    CHECK(sm.GetInt("k", 9) == 0);
    sm["f"] = "2.5";
    CHECK(sm.GetFloat("f", 0) == Catch::Approx(2.5f));
    sm["f"] = "7.5";
    CHECK(sm.GetFloat("f", 0) == Catch::Approx(7.5f));
}

TEST_CASE("SettingsMap erase removes keys", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["a"] = "1";
    sm["b"] = "2";
    CHECK(sm.erase("a") == 1);
    CHECK(sm.erase(std::string("a")) == 0);
    CHECK_FALSE(sm.Contains("a"));
    CHECK(sm.Contains("b"));
    sm.clear();
    CHECK(sm.empty());
}

TEST_CASE("SettingsMap iterates keys in sorted order", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["c"] = "3";
    sm["a"] = "1";
    sm["b"] = "2";
    std::vector<std::string> keys;
    for (const auto& k : sm.keys()) {
        keys.push_back(k);
    }
    CHECK(keys == std::vector<std::string>{ "a", "b", "c" });
    CHECK(sm.AsString() == "a=1,b=2,c=3");
}

TEST_CASE("SettingsMap copy is independent of the original", "[effects][settingsmap]") {
    SettingsMap a;
    a["k"] = "1";
    SettingsMap b(a);
    b["k"] = "2";
    b["n"] = "x";
    CHECK(a.Get("k", "") == "1");
    CHECK_FALSE(a.Contains("n"));
    SettingsMap c;
    c = b;
    CHECK(c.Get("k", "") == "2");
    SettingsMap d(std::move(c));
    CHECK(d.Get("n", "") == "x");
}

TEST_CASE("SettingsMap AsString escapes commas and ampersands", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["E_TEXTCTRL_Text"] = "a,b&c";
    CHECK(sm.AsString() == "E_TEXTCTRL_Text=a&comma;b&amp;c");
}

TEST_CASE("SettingsMap Parse reads key value pairs", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "E_SLIDER_Speed=10,E_CHECKBOX_Fade=1,E_CHOICE_Dir=Left", "Bars");
    CHECK(sm.size() == 3);
    CHECK(sm.GetInt("E_SLIDER_Speed", 0) == 10);
    CHECK(sm.GetBool("E_CHECKBOX_Fade", false));
    CHECK(sm.Get("E_CHOICE_Dir", "") == "Left");
}

TEST_CASE("SettingsMap Parse replaces previous contents", "[effects][settingsmap]") {
    SettingsMap sm;
    sm["old"] = "1";
    sm.Parse(nullptr, "new=2", "");
    CHECK_FALSE(sm.Contains("old"));
    CHECK(sm.Get("new", "") == "2");
    sm.Parse(nullptr, "", "");
    CHECK(sm.empty());
}

TEST_CASE("SettingsMap Parse unescapes commas and ampersands", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "E_TEXTCTRL_Text=a&comma;b&amp;c", "Text");
    CHECK(sm.Get("E_TEXTCTRL_Text", "") == "a,b&c");
}

TEST_CASE("SettingsMap AsString and Parse round trip awkward values", "[effects][settingsmap]") {
    SettingsMap in;
    in["E_TEXTCTRL_Text"] = "hello, world & friends, again";
    in["E_SLIDER_N"] = "5";
    in["B_CHOICE_BufferStyle"] = "Default";
    in["E_TEXTCTRL_Empty"] = "";
    SettingsMap out;
    out.Parse(nullptr, in.AsString(), "Text");
    CHECK(out.size() == in.size());
    for (const auto& k : in.keys()) {
        INFO("key " << k);
        CHECK(out.Get(k, "<missing>") == in.Get(k, ""));
    }
}

TEST_CASE("SettingsMap Parse keeps the first of duplicate keys", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "a=1,a=2", "");
    CHECK(sm.size() == 1);
    CHECK(sm.Get("a", "") == "1");
}

TEST_CASE("SettingsMap Parse keeps equals signs inside the value", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "E_TEXTCTRL_Expr=a=b", "Text");
    CHECK(sm.Get("E_TEXTCTRL_Expr", "") == "a=b");
}

TEST_CASE("SettingsMap Parse drops retired controls and renames moved ones", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "E_TEXTCTRL_MorphAccel=3,E_CHOICE_CoroFaces_Eyes=Open,T_SLIDER_EffectBlur=4,E_SLIDER_Keep=1", "Morph");
    CHECK_FALSE(sm.Contains("E_TEXTCTRL_MorphAccel"));
    CHECK_FALSE(sm.Contains("E_CHOICE_CoroFaces_Eyes"));
    CHECK(sm.Get("E_CHOICE_Faces_Eyes", "") == "Open");
    CHECK_FALSE(sm.Contains("T_SLIDER_EffectBlur"));
    CHECK(sm.Get("B_SLIDER_Blur", "") == "4");
    CHECK(sm.Get("E_SLIDER_Keep", "") == "1");
}

TEST_CASE("SettingsMap Parse maps text and picture control renames", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "E_FILEPICKER_Pictures_Filename=a.png,E_TEXTCTRL_Text_Font1=Arial", "Pictures");
    CHECK(sm.Get("E_TEXTCTRL_Pictures_Filename", "") == "a.png");
    CHECK(sm.Get("E_FONTPICKER_Text_Font1", "") == "Arial");
}

TEST_CASE("SettingsMap Parse repairs a key fused into the previous value", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, "T_CHOICE_Out_Transition_Type=NoneB_CHOICE_BufferStyle=Default", "On");
    // the fused text has no separating comma so the repair only applies when '=' is present in the value
    CHECK(sm.Contains("T_CHOICE_Out_Transition_Type"));

    SettingsMap sm2;
    sm2.Parse(nullptr, "E_SLIDER_A=5B_CHOICE_BufferStyle=Default,E_SLIDER_B=2", "On");
    CHECK(sm2.Get("E_SLIDER_A", "") == "5");
    CHECK(sm2.Get("B_CHOICE_BufferStyle", "") == "Default");
    CHECK(sm2.Get("E_SLIDER_B", "") == "2");
}

TEST_CASE("SettingsMap Parse tolerates garbage", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.Parse(nullptr, ",,,", "");
    CHECK(sm.empty());
    sm.Parse(nullptr, "=value", "");
    CHECK(sm.empty());
    sm.Parse(nullptr, "justakey", "");
    CHECK(sm.Contains("justakey"));
    sm.Parse(nullptr, "k=", "");
    CHECK(sm.Contains("k"));
    CHECK(sm.Get("k", "x").empty());
}

TEST_CASE("SettingsMap ParseJson reads quoted pairs", "[effects][settingsmap]") {
    SettingsMap sm;
    sm.ParseJson(nullptr, "{\"E_SLIDER_Speed\":\"10\",\"E_CHOICE_Dir\":\"Left\"}", "Bars");
    CHECK(sm.size() == 2);
    CHECK(sm.GetInt("E_SLIDER_Speed", 0) == 10);
    CHECK(sm.Get("E_CHOICE_Dir", "") == "Left");
}

TEST_CASE("SettingsMap AsJSON and ParseJson round trip", "[effects][settingsmap]") {
    SettingsMap in;
    in["E_SLIDER_N"] = "5";
    in["E_TEXTCTRL_Text"] = "a,b&c";
    CHECK(in.AsJSON() == "{\"E_SLIDER_N\":\"5\",\"E_TEXTCTRL_Text\":\"a&comma;b&amp;c\"}");
    SettingsMap out;
    out.ParseJson(nullptr, in.AsJSON(), "Text");
    CHECK(out.Get("E_SLIDER_N", "") == "5");
    CHECK(out.Get("E_TEXTCTRL_Text", "") == "a,b&c");
}

TEST_CASE("SettingsMap AsJSON of an empty map is an empty object", "[effects][settingsmap]") {
    SettingsMap sm;
    CHECK(sm.AsJSON() == "{}");
    CHECK(sm.AsString().empty());
}

TEST_CASE("SettingValue accessors", "[effects][settingsmap]") {
    SettingValue v("3");
    CHECK(v.getInt(0) == 3);
    CHECK(v.getBool() == false);
    CHECK(v.getFloat(0) == Catch::Approx(3.0f));
    CHECK(v.getDouble(0) == Catch::Approx(3.0));
    v = "1";
    CHECK(v.getBool());
    v = std::string("Tx");
    CHECK(v.getBool());
    SettingValue copy(v);
    CHECK(copy == "Tx");
}

TEST_CASE("Effect settings string accessors", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "E_TEXTCTRL_Eff_On_Start=40,E_TEXTCTRL_Eff_On_End=60");
    REQUIRE(e != nullptr);
    CHECK(e->GetSetting("E_TEXTCTRL_Eff_On_Start") == "40");
    CHECK(e->GetSetting("E_TEXTCTRL_Missing").empty());
    CHECK(e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_End", 0) == 60);
    CHECK(e->GetSettingsAsString() == "E_TEXTCTRL_Eff_On_End=60,E_TEXTCTRL_Eff_On_Start=40");
}

TEST_CASE("Effect SetSetting reports whether the value changed", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "E_SLIDER_X=1");
    REQUIRE(e != nullptr);
    CHECK_FALSE(e->SetSetting("E_SLIDER_X", "1"));
    CHECK(e->SetSetting("E_SLIDER_X", "2"));
    CHECK(e->GetSetting("E_SLIDER_X") == "2");
    CHECK(e->SetSetting("E_SLIDER_New", "9"));
    CHECK(e->GetSetting("E_SLIDER_New") == "9");
}

TEST_CASE("Effect SetSettings replaces settings but can keep X_ settings", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "E_SLIDER_X=1,X_Effect_Description=hi");
    REQUIRE(e != nullptr);
    e->SetSettings("E_SLIDER_Y=2", true);
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_X"));
    CHECK(e->GetSetting("E_SLIDER_Y") == "2");
    CHECK(e->GetSetting("X_Effect_Description") == "hi");
    CHECK(e->GetDescription() == "hi");

    e->SetSettings("E_SLIDER_Z=3", false);
    CHECK_FALSE(e->GetSettings().Contains("X_Effect_Description"));
    CHECK(e->GetSetting("E_SLIDER_Z") == "3");
}

TEST_CASE("Effect SetSettings accepts JSON", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "");
    REQUIRE(e != nullptr);
    e->SetSettings("{\"E_SLIDER_A\":\"4\",\"B_CHOICE_BufferStyle\":\"Default\"}", false, true);
    CHECK(e->GetSetting("E_SLIDER_A") == "4");
    CHECK(e->GetSetting("B_CHOICE_BufferStyle") == "Default");
    CHECK(e->GetSettingsAsJSON() == "{\"B_CHOICE_BufferStyle\":\"Default\",\"E_SLIDER_A\":\"4\"}");
}

TEST_CASE("Effect SettingsChanged ignores ordering", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "E_SLIDER_A=1,E_SLIDER_B=2");
    REQUIRE(e != nullptr);
    CHECK_FALSE(e->SettingsChanged("E_SLIDER_B=2,E_SLIDER_A=1"));
    CHECK(e->SettingsChanged("E_SLIDER_A=1,E_SLIDER_B=3"));
    CHECK(e->SettingsChanged("E_SLIDER_A=1"));
    CHECK(e->SettingsChanged("E_SLIDER_A=1,E_SLIDER_B=2,E_SLIDER_C=3"));
}

TEST_CASE("Effect CopySettingsMap strips the E_ T_ and B_ prefixes", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "E_SLIDER_Speed=3,B_CHOICE_BufferStyle=Default,T_CHECKBOX_Canvas=1");
    REQUIRE(e != nullptr);
    SettingsMap stripped;
    e->CopySettingsMap(stripped, true);
    CHECK(stripped.GetInt("SLIDER_Speed", 0) == 3);
    CHECK(stripped.Get("CHOICE_BufferStyle", "") == "Default");
    CHECK(stripped.GetBool("CHECKBOX_Canvas", false));
    CHECK_FALSE(stripped.Contains("E_SLIDER_Speed"));

    SettingsMap raw;
    e->CopySettingsMap(raw, false);
    CHECK(raw.GetInt("E_SLIDER_Speed", 0) == 3);
    CHECK(raw.Contains("B_CHOICE_BufferStyle"));
}

TEST_CASE("Effect CopySettingsMap copies slider checkbox and curve palette entries only", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_SLIDER_Brightness=80,C_VALUECURVE_Brightness=Active=TRUE");
    REQUIRE(e != nullptr);
    SettingsMap stripped;
    e->CopySettingsMap(stripped, true);
    CHECK(stripped.GetInt("SLIDER_Brightness", 0) == 80);
    CHECK(stripped.Contains("CHECKBOX_Palette1"));
    CHECK(stripped.Contains("VALUECURVE_Brightness"));
    CHECK_FALSE(stripped.Contains("BUTTON_Palette1"));
}

TEST_CASE("Effect IsPersistent follows the overlay background checkbox", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* a = f.Make("On", "B_CHECKBOX_OverlayBkg=1", "", 0, 100);
    Effect* b = f.Make("On", "B_CHECKBOX_OverlayBkg=0", "", 200, 300);
    Effect* c = f.Make("On", "", "", 400, 500);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(c != nullptr);
    CHECK(a->IsPersistent());
    CHECK_FALSE(b->IsPersistent());
    CHECK_FALSE(c->IsPersistent());
}

TEST_CASE("Effect palette parsing keeps only checked colours", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "",
                       "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,"
                       "C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=0,"
                       "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1");
    REQUIRE(e != nullptr);
    REQUIRE(e->GetPaletteSize() == 2);
    CHECK(e->GetPalette()[0] == xlColor(255, 0, 0));
    CHECK(e->GetPalette()[1] == xlColor(0, 0, 255));
}

TEST_CASE("Effect with an empty palette has no colours", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "", "");
    REQUIRE(e != nullptr);
    CHECK(e->GetPaletteSize() == 0);
}

TEST_CASE("Effect SetPalette reparses the colour list", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1");
    REQUIRE(e != nullptr);
    e->SetPalette("C_BUTTON_Palette1=#102030,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#405060,C_CHECKBOX_Palette2=1");
    REQUIRE(e->GetPaletteSize() == 2);
    CHECK(e->GetPalette()[0] == xlColor(0x10, 0x20, 0x30));
    CHECK(e->GetPalette()[1] == xlColor(0x40, 0x50, 0x60));
    e->SetPalette("");
    CHECK(e->GetPaletteSize() == 0);
}

TEST_CASE("Effect UsesColour matches case insensitively and requires the checkbox", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "",
                       "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1,"
                       "C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=0");
    REQUIRE(e != nullptr);
    CHECK(e->UsesColour("#FF0000"));
    CHECK(e->UsesColour("#ff0000"));
    CHECK_FALSE(e->UsesColour("#00FF00"));
    CHECK_FALSE(e->UsesColour("#123456"));
}

TEST_CASE("Effect ReplaceColours rewrites matching checked colours", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "",
                       "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,"
                       "C_BUTTON_Palette2=#FF0000,C_CHECKBOX_Palette2=0,"
                       "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1");
    REQUIRE(e != nullptr);
    CHECK(e->ReplaceColours(nullptr, "#FF0000", "#00FF00") == 1);
    CHECK(e->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#00FF00");
    CHECK(e->GetPaletteMap().Get("C_BUTTON_Palette2", "") == "#FF0000");
    REQUIRE(e->GetPaletteSize() == 2);
    CHECK(e->GetPalette()[0] == xlColor(0, 255, 0));
    CHECK(e->ReplaceColours(nullptr, "#ABCDEF", "#000000") == 0);
}

TEST_CASE("Effect ErasePalette and EraseColourSettings partition the palette map", "[effects][settingsmap][effect]") {
    EffectFixture f;
    const std::string pal = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_SLIDER_Brightness=80";
    Effect* a = f.Make("On", "", pal, 0, 100);
    Effect* b = f.Make("On", "", pal, 200, 300);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    a->ErasePalette();
    CHECK(a->GetPaletteMap().size() == 1);
    CHECK(a->GetPaletteMap().Contains("C_SLIDER_Brightness"));
    b->EraseColourSettings();
    CHECK(b->GetPaletteMap().size() == 2);
    CHECK_FALSE(b->GetPaletteMap().Contains("C_SLIDER_Brightness"));
}

TEST_CASE("Effect SetColourOnlyPalette preserves non colour palette settings", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_SLIDER_Brightness=80");
    REQUIRE(e != nullptr);
    e->SetColourOnlyPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
    CHECK(e->GetPaletteMap().Get("C_SLIDER_Brightness", "") == "80");
    REQUIRE(e->GetPaletteSize() == 1);
    CHECK(e->GetPalette()[0] == xlColor(0, 0, 255));
}

TEST_CASE("Effect EraseSettingsStartingWith only erases settings with the given prefix", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "B_CHOICE_BufferStyle=Default,T_CHECKBOX_Canvas=1,E_SLIDER_X=2");
    REQUIRE(e != nullptr);
    e->EraseSettingsStartingWith("T_");
    CHECK_FALSE(e->GetSettings().Contains("T_CHECKBOX_Canvas"));
    CHECK(e->GetSettings().Contains("B_CHOICE_BufferStyle"));
    CHECK(e->GetSettings().Contains("E_SLIDER_X"));
}

TEST_CASE("EffectLayer rejects unknown effect names on model elements", "[effects][settingsmap][effect]") {
    EffectFixture f;
    CHECK(f.Make("Definitely Not An Effect", "") == nullptr);
    Effect* blank = f.Make("", "", "", 0, 100);
    REQUIRE(blank != nullptr);
    CHECK(blank->GetEffectName() == "Off");
}

TEST_CASE("EffectLayer rejects effects whose end precedes their start", "[effects][settingsmap][effect]") {
    EffectFixture f;
    CHECK(f.Make("On", "", "", 500, 100) == nullptr);
    CHECK(f.layer->GetEffectCount() == 0);
}

TEST_CASE("Effect index tracks the manager's index for the effect name", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("Color Wash", "");
    REQUIRE(e != nullptr);
    CHECK(e->GetEffectIndex() == f.ctx.effectManager.GetEffectIndex("Color Wash"));
    CHECK(e->GetEffectName() == "Color Wash");
}

TEST_CASE("Effect EraseSettingsStartingWith with B_ keeps transition settings", "[effects][settingsmap][effect]") {
    EffectFixture f;
    Effect* e = f.Make("On", "B_CHOICE_BufferStyle=Default,B_SLIDER_A=1,T_CHECKBOX_Canvas=1,E_SLIDER_X=2");
    REQUIRE(e != nullptr);
    e->EraseSettingsStartingWith("B_");
    CHECK_FALSE(e->GetSettings().Contains("B_CHOICE_BufferStyle"));
    CHECK_FALSE(e->GetSettings().Contains("B_SLIDER_A"));
    CHECK(e->GetSettings().Contains("T_CHECKBOX_Canvas"));
    CHECK(e->GetSettings().Contains("E_SLIDER_X"));
}

#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "../support/TempDir.h"
#include "render/DataLayer.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "XmlSerializer/StringSerializingVisitor.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "render/SequenceViewManager.h"

namespace {

struct Loaded {
    HeadlessRenderContext ctx;
    std::unique_ptr<SequenceFile> file;
    bool ok = false;

    Loaded(const std::string& path, const std::string& showDir) {
        ctx.showDirectory = showDir;
        file = std::make_unique<SequenceFile>(path);
        auto doc = file->Open(showDir, false, path);
        if (doc) {
            ok = ctx.LoadSequenceElements(*file, *doc);
        }
    }
    SequenceElements& se() { return ctx.GetSequenceElements(); }
};

struct Source {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    Source() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
};

std::string Read(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

// Builds a sequence with `build`, saves it, loads it, saves the loaded copy again and returns both files' text.
struct Result {
    std::string first, second;
    std::unique_ptr<Loaded> loaded;
    bool ok = false;
};

Result Cycle(const TempDir& td, const std::function<void(Source&, SequenceFile&)>& build, uint32_t frameMS = 0) {
    Result r;
    std::string p1 = td.File("one.xsq");
    std::string p2 = td.File("two.xsq");
    {
        Source s;
        SequenceFile sf(p1, frameMS);
        build(s, sf);
        if (!sf.Save(s.se)) return r;
    }
    r.first = Read(p1);
    r.loaded = std::make_unique<Loaded>(p1, td.path.string());
    if (!r.loaded->ok) return r;
    r.loaded->file->SetFullPath(p2);
    if (!r.loaded->file->Save(r.loaded->se())) return r;
    r.second = Read(p2);
    r.ok = true;
    return r;
}

Element* AddModel(Source& s, const std::string& name) {
    return s.se.AddElement(name, "model", true, false, true, false, false);
}

} // namespace

// ---- effect settings and palettes ------------------------------------------------------------------------

#define SETTING_CASE(ID, SETTINGS, KEY, VALUE)                                                           \
    TEST_CASE("Roundtrip xsq effect settings: " ID, "[roundtrip][sequence][settings]") {                 \
        TempDir td;                                                                                      \
        auto r = Cycle(td, [&](Source& s, SequenceFile&) {                                               \
            AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", SETTINGS, "", 0, 1000, 0, false);     \
        });                                                                                              \
        REQUIRE(r.ok);                                                                                   \
        CHECK(r.second == r.first);                                                                      \
        Effect* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);                     \
        REQUIRE(e != nullptr);                                                                           \
        CHECK(e->GetSetting(KEY) == VALUE);                                                              \
    }

SETTING_CASE("plain", "E_SLIDER_Eff_On_Start=80,E_SLIDER_Eff_On_End=20", "E_SLIDER_Eff_On_End", "20")
SETTING_CASE("negative number", "E_SLIDER_X=-123", "E_SLIDER_X", "-123")
SETTING_CASE("decimal", "E_TEXTCTRL_Speed=1.5", "E_TEXTCTRL_Speed", "1.5")
SETTING_CASE("empty value", "E_TEXTCTRL_Empty=,E_TEXTCTRL_B=2", "E_TEXTCTRL_B", "2")
SETTING_CASE("empty value kept", "E_TEXTCTRL_Empty=,E_TEXTCTRL_B=2", "E_TEXTCTRL_Empty", "")
SETTING_CASE("ampersand", "E_TEXTCTRL_Text=a&amp;b", "E_TEXTCTRL_Text", "a&b")
SETTING_CASE("raw ampersand", "E_TEXTCTRL_Text=a&b", "E_TEXTCTRL_Text", "a&b")
SETTING_CASE("unicode", "E_TEXTCTRL_Text=caf\xC3\xA9 \xE2\x9D\x84", "E_TEXTCTRL_Text", "caf\xC3\xA9 \xE2\x9D\x84")
SETTING_CASE("filename", "E_TEXTCTRL_Pictures_Filename=images/a b.png", "E_TEXTCTRL_Pictures_Filename", "images/a b.png")
SETTING_CASE("checkbox", "E_CHECKBOX_Reverse=1", "E_CHECKBOX_Reverse", "1")
SETTING_CASE("choice with space", "E_CHOICE_Direction=Up Left", "E_CHOICE_Direction", "Up Left")
SETTING_CASE("equals in value", "E_TEXTCTRL_Eq=a=b", "E_TEXTCTRL_Eq", "a=b")
SETTING_CASE("long value", "E_TEXTCTRL_Long=" + std::string(4000, 'x'), "E_TEXTCTRL_Long", std::string(4000, 'x'))
SETTING_CASE("transition keys", "T_CHOICE_In_Transition_Type=Fade,T_TEXTCTRL_Fadein=1.25,T_CHOICE_Out_Transition_Type=Wipe", "T_CHOICE_Out_Transition_Type", "Wipe")
SETTING_CASE("buffer keys", "B_CHOICE_BufferStyle=Per Model Default,B_CHOICE_PerPreviewCamera=2D,B_SLIDER_Rotation=45", "B_CHOICE_BufferStyle", "Per Model Default")
SETTING_CASE("layer method", "T_CHOICE_LayerMethod=Max,T_SLIDER_EffectLayerMix=33", "T_CHOICE_LayerMethod", "Max")
SETTING_CASE("quotes", "E_TEXTCTRL_Text=say \"hi\"", "E_TEXTCTRL_Text", "say \"hi\"")
SETTING_CASE("angle brackets", "E_TEXTCTRL_Text=<b>x</b>", "E_TEXTCTRL_Text", "<b>x</b>")
SETTING_CASE("percent and backslash", "E_TEXTCTRL_Text=100% C:\\x", "E_TEXTCTRL_Text", "100% C:\\x")

TEST_CASE("Roundtrip xsq many settings keep their values", "[roundtrip][sequence][settings]") {
    std::string settings;
    for (int i = 0; i < 200; ++i) {
        if (i) settings += ",";
        settings += "E_SLIDER_K" + std::to_string(i) + "=" + std::to_string(i * 7 - 300);
    }
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", settings, "", 0, 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    Effect* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);
    for (int i = 0; i < 200; ++i) {
        CHECK(e->GetSetting("E_SLIDER_K" + std::to_string(i)) == std::to_string(i * 7 - 300));
    }
}
TEST_CASE("Roundtrip xsq empty settings and palette", "[roundtrip][sequence][settings]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", "", 0, 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    Effect* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);
    CHECK(e->GetPaletteSize() == 0);
}
TEST_CASE("Roundtrip xsq palette with eight colours", "[roundtrip][sequence][palette]") {
    std::string pal;
    const char* cols[] = { "#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#FF00FF", "#00FFFF", "#FFFFFF", "#123456" };
    for (int i = 0; i < 8; ++i) {
        if (i) pal += ",";
        pal += "C_BUTTON_Palette" + std::to_string(i + 1) + "=" + cols[i] + ",C_CHECKBOX_Palette" + std::to_string(i + 1) + "=1";
    }
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", pal, 0, 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    Effect* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);
    REQUIRE(e->GetPaletteSize() == 8);
    CHECK(e->GetPalette()[7].Red() == 0x12);
    CHECK(e->GetPalette()[7].Green() == 0x34);
    CHECK(e->GetPalette()[7].Blue() == 0x56);
    CHECK(e->GetPalette()[3].Red() == 255);
    CHECK(e->GetPalette()[3].Green() == 255);
    CHECK(e->GetPalette()[3].Blue() == 0);
}
TEST_CASE("Roundtrip xsq palette with disabled colours", "[roundtrip][sequence][palette]") {
    std::string pal = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=0,C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1";
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", pal, 0, 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    Effect* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);
    CHECK(e->GetPaletteSize() == 2);
}
TEST_CASE("Roundtrip xsq palette with colour curves and slider settings", "[roundtrip][sequence][palette]") {
    std::string pal = "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF;|,C_CHECKBOX_Palette1=1,C_SLIDER_Brightness=80,C_SLIDER_Contrast=-20,C_SLIDER_SparkleFrequency=33,C_SLIDER_Color_HueAdjust=12,C_SLIDER_Color_SaturationAdjust=-5,C_SLIDER_Color_ValueAdjust=7,C_CHOICE_Palette_Mode=Custom";
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", pal, 0, 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    Effect* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);
    CHECK(e->GetPaletteMap()["C_SLIDER_Brightness"] == "80");
    CHECK(e->GetPaletteMap()["C_SLIDER_Contrast"] == "-20");
    CHECK(e->GetPaletteMap()["C_SLIDER_Color_SaturationAdjust"] == "-5");
}
TEST_CASE("Roundtrip xsq palettes are shared by identical effects and kept separate otherwise", "[roundtrip][sequence][palette]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* l = AddModel(s, "M")->AddEffectLayer();
        l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
        l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1", 100, 200, 0, false);
        l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", 200, 300, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* l = r.loaded->se().GetElement("M")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 3);
    CHECK(l->GetEffect(0)->GetPalette()[0].Red() == 255);
    CHECK(l->GetEffect(1)->GetPalette()[0].Green() == 255);
    CHECK(l->GetEffect(2)->GetPalette()[0].Red() == 255);
}

// ---- names ---------------------------------------------------------------------------------------------------

#define NAME_CASE(ID, NAME)                                                                                    \
    TEST_CASE("Roundtrip xsq names survive: " ID, "[roundtrip][sequence][names]") {                            \
        const std::string n = NAME;                                                                            \
        TempDir td;                                                                                            \
        auto r = Cycle(td, [&](Source& s, SequenceFile&) {                                                     \
            auto* m = AddModel(s, "Model " + n);                                                               \
            auto* l = m->AddEffectLayer();                                                                     \
            l->SetLayerName("Layer " + n);                                                                     \
            l->AddEffect(0, "On", "", "", 0, 1000, 0, false);                                                  \
            auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("Timing " + n, "timing", true, false, true, false, false)); \
            t->AddEffectLayer()->AddEffect(0, "Label " + n, "", "", 0, 500, 0, false);                         \
        });                                                                                                    \
        REQUIRE(r.ok);                                                                                         \
        CHECK(r.second == r.first);                                                                            \
        auto& se = r.loaded->se();                                                                             \
        auto* m = se.GetElement("Model " + n);                                                                 \
        REQUIRE(m != nullptr);                                                                                 \
        CHECK(m->GetEffectLayer(0)->GetLayerName() == "Layer " + n);                                           \
        auto* t = se.GetTimingElement("Timing " + n);                                                          \
        REQUIRE(t != nullptr);                                                                                 \
        REQUIRE(t->GetEffectLayer(0)->GetEffectCount() == 1);                                                  \
        CHECK(t->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "Label " + n);                            \
    }

NAME_CASE("ampersand", "a & b")
NAME_CASE("angle brackets", "<x>")
NAME_CASE("quotes", "he said \"hi\" 'x'")
NAME_CASE("unicode", "caf\xC3\xA9 \xE2\x9D\x84 \xE6\x97\xA5\xE6\x9C\xAC")
NAME_CASE("emoji", "\xF0\x9F\x8E\x84")
NAME_CASE("tab", "a\tb")
NAME_CASE("comma", "a,b")
NAME_CASE("semicolon", "a;b")
NAME_CASE("equals", "a=b")
NAME_CASE("pipe", "a|b")
NAME_CASE("percent", "100%")
NAME_CASE("slash", "a/b")
NAME_CASE("backslash", "a\\b")
NAME_CASE("long", std::string(500, 'n'))
NAME_CASE("digits", "12345")

TEST_CASE("Roundtrip xsq timing label containing an entity lookalike", "[roundtrip][sequence][names]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", true, false, true, false, false));
        t->AddEffectLayer()->AddEffect(0, "AT&amp;T &lt;3", "", "", 0, 500, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    CHECK(r.loaded->se().GetTimingElement("T")->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "AT&amp;T &lt;3");
}
TEST_CASE("Roundtrip xsq header info keeps non-ASCII text", "[roundtrip][sequence][header]") {
    const std::vector<std::string> values = { "caf\xC3\xA9", "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E", "\xE2\x9D\x84 snow", "\xF0\x9F\x8E\x84" };
    for (auto& v : values) {
        INFO(v);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::SONG, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::COMMENT, v);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR) == v);
        CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::ARTIST) == v);
        CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::SONG) == v);
        CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::COMMENT) == v);
    }
}
TEST_CASE("Roundtrip xsq effect label with a newline", "[roundtrip][sequence][names]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", true, false, true, false, false));
        t->AddEffectLayer()->AddEffect(0, "line1\nline2", "", "", 0, 500, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* t = r.loaded->se().GetTimingElement("T");
    REQUIRE(t != nullptr);
    CHECK(t->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "line1\nline2");
}
TEST_CASE("Roundtrip xsq empty timing label", "[roundtrip][sequence][names]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", true, false, true, false, false));
        auto* l = t->AddEffectLayer();
        l->AddEffect(0, "", "", "", 0, 500, 0, false);
        l->AddEffect(0, "x", "", "", 500, 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* t = r.loaded->se().GetTimingElement("T");
    REQUIRE(t != nullptr);
    CHECK(t->GetEffectLayer(0)->GetEffectCount() == 2);
}

// ---- times and frame periods ---------------------------------------------------------------------------------

TEST_CASE("Roundtrip xsq effect times on the frame grid", "[roundtrip][sequence][times]") {
    for (uint32_t frame : { 20u, 25u, 40u, 50u, 100u }) {
        INFO(frame);
        TempDir td;
        auto r = Cycle(
            td, [&](Source& s, SequenceFile& sf) {
                sf.SetSequenceDurationMS(60000);
                auto* l = AddModel(s, "M")->AddEffectLayer();
                for (int i = 0; i < 20; ++i) {
                    l->AddEffect(0, "On", "", "", (int)(i * frame * 3), (int)(i * frame * 3 + frame * 2), 0, false);
                }
            },
            frame);
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        auto* l = r.loaded->se().GetElement("M")->GetEffectLayer(0);
        REQUIRE(l->GetEffectCount() == 20);
        for (int i = 0; i < 20; ++i) {
            CHECK(l->GetEffect(i)->GetStartTimeMS() == (int)(i * frame * 3));
            CHECK(l->GetEffect(i)->GetEndTimeMS() == (int)(i * frame * 3 + frame * 2));
        }
    }
}
TEST_CASE("Roundtrip xsq effect times past the sequence end", "[roundtrip][sequence][times]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.SetSequenceDurationMS(1000);
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", "", 500, 5000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
}
TEST_CASE("Roundtrip xsq very long effect", "[roundtrip][sequence][times]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.SetSequenceDurationMS(3 * 3600 * 1000);
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", "", 0, 3 * 3600 * 1000, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* e = r.loaded->se().GetElement("M")->GetEffectLayer(0)->GetEffect(0);
    CHECK(e->GetEndTimeMS() == 3 * 3600 * 1000);
}
TEST_CASE("Roundtrip xsq sequence duration values", "[roundtrip][sequence][times]") {
    // 16777217 is past float precision, which a ms -> s conversion through float loses.
    for (int ms : { 50, 1000, 12345, 60000, 3599950, 16777217, 86400000 }) {
        INFO(ms);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetSequenceDurationMS(ms);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        CHECK(r.loaded->file->GetSequenceDurationMS() == ms);
    }
}
TEST_CASE("Roundtrip xsq sequence timing values", "[roundtrip][sequence][times]") {
    for (const char* t : { "20 ms", "25 ms", "33 ms", "40 ms", "50 ms", "100 ms" }) {
        INFO(t);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetSequenceTiming(t);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        CHECK(r.loaded->file->GetSequenceTiming() == t);
    }
}
TEST_CASE("Roundtrip xsq sequence types", "[roundtrip][sequence][times]") {
    for (const char* t : { "Animation", "Media" }) {
        INFO(t);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetSequenceType(t);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
    }
}

// ---- header info ---------------------------------------------------------------------------------------------

TEST_CASE("Roundtrip xsq header info with special characters", "[roundtrip][sequence][header]") {
    const std::vector<std::string> values = { "plain", "a & b", "<tag>", "\"quoted\" 'single'", "line1\nline2", "tab\there", "", "  padded  ", std::string(2000, 'h') };
    for (auto& v : values) {
        INFO(v.substr(0, 30));
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::WEBSITE, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::SONG, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::URL, v);
            sf.SetHeaderInfo(HEADER_INFO_TYPES::COMMENT, v);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        for (auto t : { HEADER_INFO_TYPES::AUTHOR, HEADER_INFO_TYPES::AUTHOR_EMAIL, HEADER_INFO_TYPES::WEBSITE, HEADER_INFO_TYPES::SONG, HEADER_INFO_TYPES::ARTIST, HEADER_INFO_TYPES::ALBUM, HEADER_INFO_TYPES::URL, HEADER_INFO_TYPES::COMMENT }) {
            CHECK(r.loaded->file->GetHeaderInfo(t) == v);
        }
    }
}
TEST_CASE("Roundtrip xsq header fields do not bleed into each other", "[roundtrip][sequence][header]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, "A");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL, "B");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::WEBSITE, "C");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::SONG, "D");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, "E");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, "F");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::URL, "G");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::COMMENT, "H");
        AddModel(s, "M")->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR) == "A");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL) == "B");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::WEBSITE) == "C");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::SONG) == "D");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::ARTIST) == "E");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::ALBUM) == "F");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::URL) == "G");
    CHECK(r.loaded->file->GetHeaderInfo(HEADER_INFO_TYPES::COMMENT) == "H");
}
TEST_CASE("Roundtrip xsq image directory", "[roundtrip][sequence][header]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.SetImageDir("some/images dir & more");
        AddModel(s, "M")->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
}

// ---- structure -------------------------------------------------------------------------------------------------

TEST_CASE("Roundtrip xsq empty layers are kept", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* m = AddModel(s, "M");
        m->AddEffectLayer();
        m->AddEffectLayer()->AddEffect(0, "On", "", "", 0, 100, 0, false);
        m->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* m = r.loaded->se().GetElement("M");
    CHECK(m->GetEffectLayerCount() == 3);
    CHECK(m->GetEffectLayer(0)->GetEffectCount() == 0);
    CHECK(m->GetEffectLayer(1)->GetEffectCount() == 1);
    CHECK(m->GetEffectLayer(2)->GetEffectCount() == 0);
}
TEST_CASE("Roundtrip xsq many layers keep their order", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* m = AddModel(s, "M");
        for (int i = 0; i < 30; ++i) {
            auto* l = m->AddEffectLayer();
            l->SetLayerName("L" + std::to_string(i));
            l->AddEffect(0, "On", "E_SLIDER_I=" + std::to_string(i), "", i * 100, i * 100 + 100, 0, false);
        }
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* m = r.loaded->se().GetElement("M");
    REQUIRE(m->GetEffectLayerCount() == 30);
    for (int i = 0; i < 30; ++i) {
        CHECK(m->GetEffectLayer(i)->GetLayerName() == "L" + std::to_string(i));
        CHECK(m->GetEffectLayer(i)->GetEffect(0)->GetSetting("E_SLIDER_I") == std::to_string(i));
    }
}
TEST_CASE("Roundtrip xsq many elements keep their order", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        for (int i = 0; i < 60; ++i) {
            AddModel(s, "Model" + std::to_string(i))->AddEffectLayer()->AddEffect(0, "On", "", "", 0, 100, 0, false);
        }
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    REQUIRE(r.loaded->se().GetElementCount() == 60);
    for (int i = 0; i < 60; ++i) {
        CHECK(r.loaded->se().GetElement((size_t)i)->GetName() == "Model" + std::to_string(i));
    }
}
TEST_CASE("Roundtrip xsq effect flags", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* l = AddModel(s, "M")->AddEffectLayer();
        l->AddEffect(0, "On", "", "", 0, 100, 0, true);
        l->AddEffect(0, "On", "", "", 100, 200, 0, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* l = r.loaded->se().GetElement("M")->GetEffectLayer(0);
    CHECK(l->GetEffect(0)->GetProtected());
    CHECK_FALSE(l->GetEffect(1)->GetProtected());
}
TEST_CASE("Roundtrip xsq selected effect flag is not saved", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "", "", 0, 100, 1, false);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    CHECK(r.first.find("selected=") == std::string::npos);
}
TEST_CASE("Roundtrip xsq element flag combinations", "[roundtrip][sequence][structure]") {
    for (int mask = 0; mask < 16; ++mask) {
        INFO(mask);
        bool visible = mask & 1, collapsed = mask & 2, active = mask & 4, renderDisabled = mask & 8;
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile&) {
            s.se.AddElement("M", "model", visible, collapsed, active, false, renderDisabled)->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        auto* m = r.loaded->se().GetElement("M");
        REQUIRE(m != nullptr);
        CHECK(m->GetVisible() == visible);
        CHECK(m->GetCollapsed() == collapsed);
        CHECK(m->IsRenderDisabled() == renderDisabled);
    }
}
TEST_CASE("Roundtrip xsq timing track flags", "[roundtrip][sequence][structure]") {
    for (int mask = 0; mask < 4; ++mask) {
        INFO(mask);
        bool visible = mask & 1, active = mask & 2;
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile&) {
            auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", visible, false, active, false, false));
            t->SetSubType("Words");
            t->AddEffectLayer()->AddEffect(0, "x", "", "", 0, 100, 0, false);
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        auto* t = r.loaded->se().GetTimingElement("T");
        REQUIRE(t != nullptr);
        CHECK(t->GetActive() == active);
        CHECK(t->GetSubType() == "Words");
    }
}
TEST_CASE("Roundtrip xsq timing track sub types", "[roundtrip][sequence][structure]") {
    for (const char* st : { "", "Beats", "Words", "Phrases", "Phonemes", "Notes", "Song Structure", "Custom Type" }) {
        INFO(st);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile&) {
            auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", true, false, true, false, false));
            t->SetSubType(st);
            t->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        CHECK(r.loaded->se().GetTimingElement("T")->GetSubType() == st);
    }
}
TEST_CASE("Roundtrip xsq timing track layers", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", true, false, true, false, false));
        for (int layer = 0; layer < 3; ++layer) {
            auto* l = t->AddEffectLayer();
            for (int i = 0; i < 10; ++i) {
                l->AddEffect(0, "w" + std::to_string(layer) + "_" + std::to_string(i), "", "", i * 100, i * 100 + 100, 0, false);
            }
        }
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto* t = r.loaded->se().GetTimingElement("T");
    REQUIRE(t->GetEffectLayerCount() == 3);
    for (int layer = 0; layer < 3; ++layer) {
        REQUIRE(t->GetEffectLayer(layer)->GetEffectCount() == 10);
        CHECK(t->GetEffectLayer(layer)->GetEffect(9)->GetEffectName() == "w" + std::to_string(layer) + "_9");
        CHECK(t->GetEffectLayer(layer)->GetEffect(9)->GetEndTimeMS() == 1000);
    }
}
TEST_CASE("Roundtrip xsq fixed timing tracks of several intervals", "[roundtrip][sequence][structure]") {
    for (int interval : { 50, 100, 250, 500, 1000 }) {
        INFO(interval);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetSequenceDurationMS(5000);
            auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("F", "timing", true, false, true, false, false));
            t->SetFixedTiming(interval);
            t->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        auto* t = r.loaded->se().GetTimingElement("F");
        REQUIRE(t != nullptr);
        CHECK(t->GetFixedTiming() == interval);
    }
}
TEST_CASE("Roundtrip xsq timing views membership", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("T", "timing", true, false, true, false, false));
        t->SetViews("0,1,2");
        t->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    CHECK(r.loaded->se().GetTimingElement("T")->GetViews() == "0,1,2");
}
TEST_CASE("Roundtrip xsq timeline tags", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer();
        for (int i = 0; i < 10; ++i) s.se.SetTagPosition(i, i * 1000 + 1);
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    for (int i = 0; i < 10; ++i) CHECK(r.loaded->se().GetTagPosition(i) == i * 1000 + 1);
}
TEST_CASE("Roundtrip xsq model blending flag", "[roundtrip][sequence][structure]") {
    for (bool b : { true, false }) {
        INFO(b);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            s.se.SetSupportsModelBlending(b);
            sf.setSupportsModelBlending(b);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        CHECK(r.loaded->se().SupportsModelBlending() == b);
    }
}
TEST_CASE("Roundtrip xsq render mode", "[roundtrip][sequence][structure]") {
    for (const std::string m : { SequenceFile::ERASE_MODE, SequenceFile::CANVAS_MODE }) {
        INFO(m);
        TempDir td;
        auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
            sf.SetRenderMode(m);
            AddModel(s, "M")->AddEffectLayer();
        });
        REQUIRE(r.ok);
        CHECK(r.second == r.first);
        CHECK(r.loaded->file->GetRenderMode() == m);
    }
}
namespace {

struct ViewFixture {
    OutputManager om;
    ModelManager mm{ &om };
    SequenceViewManager vm{ &mm };
};

std::string SaveViews(const SequenceViewManager& vm) {
    StringSerializingVisitor v(false, false);
    vm.Save(v);
    std::string s = v.GetResult();
    auto p = s.find("?>");
    return p == std::string::npos ? s : s.substr(p + 2);
}

void LoadViews(SequenceViewManager& vm, const std::string& xml, int selected = 0) {
    static pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    vm.Load(doc.child("views"), selected);
}

} // namespace

TEST_CASE("Roundtrip sequence views are stable", "[roundtrip][sequence][views]") {
    ViewFixture a;
    a.vm.AddView("First View")->SetModels("A,B,C");
    a.vm.AddView("Second_View-2")->SetModels("C,B");
    a.vm.AddView("Empty");
    std::string s1 = SaveViews(a.vm);
    ViewFixture b;
    LoadViews(b.vm, s1);
    CHECK(SaveViews(b.vm) == s1);
    CHECK(b.vm.GetViewCount() == a.vm.GetViewCount());
}
TEST_CASE("Roundtrip sequence views keep model order", "[roundtrip][sequence][views]") {
    ViewFixture a;
    a.vm.AddView("V")->SetModels("Z,Y,X,W,V,U");
    ViewFixture b;
    LoadViews(b.vm, SaveViews(a.vm));
    auto models = b.vm.GetView("V")->GetModels();
    CHECK(std::vector<std::string>(models.begin(), models.end()) == std::vector<std::string>{ "Z", "Y", "X", "W", "V", "U" });
}
TEST_CASE("Roundtrip sequence views with model names containing specials", "[roundtrip][sequence][views]") {
    ViewFixture a;
    a.vm.AddView("V")->SetModels("A&B,<x>,quote\"s,uni\xC3\xA9");
    std::string s1 = SaveViews(a.vm);
    ViewFixture b;
    LoadViews(b.vm, s1);
    CHECK(SaveViews(b.vm) == s1);
    CHECK(b.vm.GetView("V")->GetModels() == a.vm.GetView("V")->GetModels());
}
TEST_CASE("Roundtrip sequence views selected view index", "[roundtrip][sequence][views]") {
    ViewFixture a;
    a.vm.AddView("One");
    a.vm.AddView("Two");
    ViewFixture b;
    LoadViews(b.vm, SaveViews(a.vm), 2);
    CHECK(b.vm.GetSelectedViewIndex() == 2);
    ViewFixture c;
    LoadViews(c.vm, SaveViews(a.vm), 99);
    CHECK(c.vm.GetSelectedViewIndex() == 0);
}
TEST_CASE("Roundtrip sequence view names with a comma are sanitised once", "[roundtrip][sequence][views]") {
    ViewFixture a;
    LoadViews(a.vm, "<views><view name=\"A,B\" models=\"X\"/></views>");
    std::string s1 = SaveViews(a.vm);
    ViewFixture b;
    LoadViews(b.vm, s1);
    CHECK(SaveViews(b.vm) == s1);
    CHECK(s1.find("A,B") == std::string::npos);
}
TEST_CASE("Roundtrip sequence views many views", "[roundtrip][sequence][views]") {
    ViewFixture a;
    for (int i = 0; i < 40; ++i) {
        std::string models;
        for (int j = 0; j <= i % 7; ++j) models += (j ? ",M" : "M") + std::to_string(i * 10 + j);
        a.vm.AddView("View " + std::to_string(i))->SetModels(models);
    }
    std::string s1 = SaveViews(a.vm);
    ViewFixture b;
    LoadViews(b.vm, s1);
    CHECK(SaveViews(b.vm) == s1);
    CHECK(b.vm.GetViewCount() == 41);
}
TEST_CASE("Roundtrip sequence views empty view list", "[roundtrip][sequence][views]") {
    ViewFixture a;
    std::string s1 = SaveViews(a.vm);
    ViewFixture b;
    LoadViews(b.vm, s1);
    CHECK(SaveViews(b.vm) == s1);
}
// The alt track files are never created, so this also covers tracks whose media is missing when the sequence is opened.
TEST_CASE("Roundtrip xsq alternate audio tracks", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.AddAltTrack(td.path.string(), td.File("alt1.mp3"), "Drums & Bass");
        sf.AddAltTrack(td.path.string(), td.File("alt2.mp3"), "Vocals");
        AddModel(s, "M")->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    REQUIRE(r.loaded->file->GetAltTrackCount() == 2);
    CHECK(r.loaded->file->GetAltTrack(1).shortname == "Vocals");
    CHECK(r.loaded->file->GetAltTrackMedia(1) == nullptr);
}
TEST_CASE("Roundtrip xsq imported from list with specials", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.RecordImportedFrom("a & b.xsq");
        sf.RecordImportedFrom("c.xsq");
        AddModel(s, "M")->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    REQUIRE(r.loaded->file->GetImportedFrom().size() == 2);
    CHECK(r.loaded->file->GetImportedFrom()[1] == "a & b.xsq");
}
TEST_CASE("Roundtrip xsq jukebox buttons", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        auto d = std::make_unique<JukeboxButtonData>();
        d->number = 3;
        d->description = "desc & <1>";
        d->tooltip = "tip \"x\"";
        d->element = "Tree";
        d->layer = 2;
        d->time = 1500;
        d->loop = false;
        d->type = JukeboxButtonData::LookupType::DESCRIPTION;
        sf.GetJukeboxButtons()[3] = std::move(d);
        AddModel(s, "M")->AddEffectLayer();
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    auto& jb = r.loaded->file->GetJukeboxButtons();
    REQUIRE(jb.count(3) == 1);
    CHECK(jb.at(3)->description == "desc & <1>");
    CHECK(jb.at(3)->tooltip == "tip \"x\"");
    CHECK(jb.at(3)->element == "Tree");
    CHECK(jb.at(3)->layer == 2);
    CHECK(jb.at(3)->time == 1500);
    CHECK_FALSE(jb.at(3)->loop);
}
TEST_CASE("Roundtrip xsq large sequence is byte stable", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile& sf) {
        sf.SetSequenceDurationMS(120000);
        for (int m = 0; m < 25; ++m) {
            auto* me = AddModel(s, "Model " + std::to_string(m));
            for (int l = 0; l < 3; ++l) {
                auto* layer = me->AddEffectLayer();
                for (int e = 0; e < 40; ++e) {
                    layer->AddEffect(0, (e % 2) ? "Bars" : "Twinkle", "E_SLIDER_A=" + std::to_string(e) + ",E_CHECKBOX_B=" + std::to_string(e % 2),
                                     "C_BUTTON_Palette1=#" + std::string(e % 2 ? "FF0000" : "00FF00") + ",C_CHECKBOX_Palette1=1", e * 1000, e * 1000 + 800, 0, false);
                }
            }
        }
    });
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
}
TEST_CASE("Roundtrip xsq saved three times is identical", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M")->AddEffectLayer()->AddEffect(0, "On", "E_SLIDER_A=1", "C_BUTTON_Palette1=#FF00FF,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    });
    REQUIRE(r.ok);
    std::string p3 = td.File("three.xsq");
    Loaded again(td.File("two.xsq"), td.path.string());
    REQUIRE(again.ok);
    again.file->SetFullPath(p3);
    REQUIRE(again.file->Save(again.se()));
    CHECK(Read(p3) == r.first);
}
TEST_CASE("Roundtrip xsq empty sequence", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source&, SequenceFile&) {});
    REQUIRE(r.ok);
    CHECK(r.second == r.first);
    CHECK(r.loaded->se().GetElementCount() == 0);
}
TEST_CASE("Roundtrip xsq element with no layers gets one on load and is stable after", "[roundtrip][sequence][structure]") {
    TempDir td;
    auto r = Cycle(td, [&](Source& s, SequenceFile&) {
        AddModel(s, "M");
    });
    REQUIRE(r.ok);
    // second save may add the implicit layer, but a further save must not change it
    std::string p3 = td.File("three.xsq");
    Loaded again(td.File("two.xsq"), td.path.string());
    REQUIRE(again.ok);
    again.file->SetFullPath(p3);
    REQUIRE(again.file->Save(again.se()));
    CHECK(Read(p3) == r.second);
}


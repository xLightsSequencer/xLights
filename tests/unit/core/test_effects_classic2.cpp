#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

#include "test_render_common.h"

#include "XmlSerializer/XmlSerializingVisitor.h"
#include "effects/EffectManager.h"
#include "effects/EffectPresetManager.h"
#include "effects/KaleidoscopeEffect.h"
#include "effects/RenderableEffect.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "render/SequenceElements.h"
#include "render/ValueCurve.h"
#include "support/TempDir.h"
#include "utils/UtilClasses.h"

namespace {

const char* kRed = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";
const char* kRedBlue = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#0000FF,C_CHECKBOX_Palette2=1";
const char* kRGB = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1,C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1";
const char* kFive = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1,C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1,"
                    "C_BUTTON_Palette4=#FFFF00,C_CHECKBOX_Palette4=1,C_BUTTON_Palette5=#FF00FF,C_CHECKBOX_Palette5=1";

// A red -> blue colour curve. Timecurve 0 is a gradient over time, 1 is left to right, 5 is radial.
std::string CurvePalette(int timecurve) {
    return "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Timecurve=" + std::to_string(timecurve) +
           "|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF;|,C_CHECKBOX_Palette1=1";
}

std::string Ramp(const std::string& id, double lo, double hi, double p1, double p2) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Active=TRUE|Id=ID_VALUECURVE_%s|Type=Ramp|Min=%.2f|Max=%.2f|P1=%.2f|P2=%.2f|RV=TRUE|", id.c_str(), lo, hi, p1, p2);
    return buf;
}

using Pixels = std::vector<xlColor>;

bool Lit(const xlColor& c) { return c.alpha != 0 && (c.red || c.green || c.blue); }

bool SameRgba(const xlColor& a, const xlColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) n += Lit(c) ? 1 : 0;
        return n;
    }
    int totalLit() const {
        int n = 0;
        for (size_t i = 0; i < f.size(); ++i) n += lit(i);
        return n;
    }
    // Mean distance of lit pixels from (cx, cy); -1 when nothing is lit.
    double meanRadius(size_t frame, double cx, double cy) const {
        double sum = 0;
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (Lit(at(frame, x, y))) {
                    sum += std::hypot(x - cx, y - cy);
                    ++n;
                }
            }
        }
        return n ? sum / n : -1.0;
    }
    double meanY(size_t frame) const {
        double sum = 0;
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (Lit(at(frame, x, y))) {
                    sum += y;
                    ++n;
                }
            }
        }
        return n ? sum / n : -1.0;
    }
    // xlColor's own == ignores alpha; frames compare all four channels.
    bool operator==(const Frames& o) const {
        if (w != o.w || h != o.h || f.size() != o.f.size()) return false;
        for (size_t i = 0; i < f.size(); ++i) {
            if (f[i].size() != o.f[i].size()) return false;
            for (size_t j = 0; j < f[i].size(); ++j) {
                if (!SameRgba(f[i][j], o.f[i][j])) return false;
            }
        }
        return true;
    }
};

// Every lit pixel has no green: the output came from a red/blue palette.
bool NoGreen(const Frames& fr) {
    for (const auto& px : fr.f) {
        for (const auto& c : px) {
            if (Lit(c) && c.green != 0) return false;
        }
    }
    return true;
}

// Moving head fixture (below): channel 9 (index 8) is red.
bool DmxRedSeen(const Frames& fr) {
    for (size_t i = 0; i < fr.f.size(); ++i) {
        if (fr.at(i, 8, 0).red > 0) return true;
    }
    return false;
}

using Prefill = std::function<void(int frameIdx, int w, int h, xlColor* px)>;

xlColor PatternColor(int x, int y) { return xlColor((uint8_t)(10 + x * 20), (uint8_t)(10 + y * 20), 77, 255); }

void FillPattern(int, int w, int h, xlColor* px) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) px[y * w + x] = PatternColor(x, y);
    }
}

struct Opts {
    int w = 10;
    int h = 10;
    std::vector<int> periods = { 0 };
    int startMs = 0;
    int endMs = 1000;
    bool persist = true;
    bool allowAlpha = false;
    bool viaAdvanceState = false;
    bool noContext = false;
    Prefill prefill;
};

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

// Windowless show with one model; drives RenderBuffer + effect->Render directly, as the
// engine does, with no PixelBuffer layering.
struct Fx {
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> owned;
    Model* model = nullptr;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;
    Effect* last = nullptr;

    explicit Fx(const std::string& modelXml = "") {
        if (modelXml.empty()) {
            owned.reset(ctx.AllModels.CreateDefaultModel("Single Line"));
            model = owned.get();
        } else {
            pugi::xml_document doc;
            REQUIRE(doc.load_string(modelXml.c_str()));
            model = ctx.AllModels.createAndAddModel(doc.first_child(), 100, 100);
            REQUIRE(model != nullptr);
        }
        element = ctx._sequenceElements.AddElement(model->GetName(), "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    EffectLayer* AddTiming(const std::string& name, const std::vector<std::pair<int, int>>& marks, const std::string& label = "") {
        Element* t = ctx._sequenceElements.AddElement(name, "timing", true, false, true, false, false);
        EffectLayer* tl = t->GetEffectLayerCount() > 0 ? t->GetEffectLayer(0) : t->AddEffectLayer();
        for (const auto& m : marks) tl->AddEffect(0, label, "", "", m.first, m.second, 0, false);
        return tl;
    }

    RenderableEffect* Get(const std::string& name) {
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);
        return fx;
    }

    Frames Render(const std::string& name, const std::string& settings, const std::string& palette, const Opts& o = Opts()) {
        Effect* e = layer->AddEffect(0, name, settings, palette, o.startMs, o.endMs, 0, false);
        REQUIRE(e != nullptr);
        last = e;
        RenderableEffect* fx = Get(name);

        RenderBuffer rb(o.noContext ? nullptr : &ctx, nullptr, model);
        rb.InitBuffer(o.h, o.w, "None");
        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        if (colors.empty()) {
            colors.push_back(xlWHITE);
            curves.push_back(ColorCurve());
        }
        rb.SetPalette(colors, curves);
        rb.SetFrameTimeInMs(50);
        rb.SetEffectDuration(o.startMs, o.endMs);
        rb.SetAllowAlphaChannel(o.allowAlpha);

        SettingsMap rs;
        e->CopySettingsMap(rs, true);

        Frames out;
        out.w = o.w;
        out.h = o.h;
        bool first = true;
        int idx = 0;
        for (int p : o.periods) {
            rb.Clear();
            if (o.prefill && o.w * o.h > 0) o.prefill(idx, o.w, o.h, rb.GetPixels());
            rb.SetState(p, first || !o.persist);
            first = false;
            if (o.viaAdvanceState) {
                auto st = fx->AdvanceState(e, rs, rb);
                rb.pendingSnapshot = st.get();
                fx->Render(e, rs, rb);
                rb.pendingSnapshot = nullptr;
            } else {
                fx->Render(e, rs, rb);
            }
            Pixels px;
            if (o.w > 0 && o.h > 0) px.assign(rb.GetPixels(), rb.GetPixels() + (size_t)o.w * o.h);
            out.f.push_back(std::move(px));
            ++idx;
        }
        return out;
    }
};

// 12 channel moving head: RGB on 9-11, dimmer on 12. Single Color White string type.
const char* kMovingHead = "<model name=\"MH\" DisplayAs=\"DmxMovingHead\" StartChannel=\"1\" StringType=\"Single Color White\" "
                          "DmxChannelCount=\"12\" DmxRedChannel=\"9\" DmxGreenChannel=\"10\" DmxBlueChannel=\"11\" MhDimmerChannel=\"12\"/>";

struct RecordingCVA : xlVertexColorAccumulator {
    struct V {
        float x, y;
        xlColor c;
    };
    std::vector<V> v;
    void AddVertex(float x, float y, float z, const xlColor& c) override { v.push_back({ x, y, c }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t, float, float, float, const xlColor&) override {}
    void SetVertex(uint32_t, float, float, float) override {}
    void SetVertex(uint32_t, const xlColor&) override {}
};

// Runs a sweep of setting strings through one effect and checks every run lights something.
void SweepLights(Fx& f, const std::string& name, const std::vector<std::string>& settings, const std::string& palette, const Opts& o) {
    for (const auto& s : settings) {
        INFO(name << " settings: " << s);
        Frames fr = f.Render(name, s, palette, o);
        CHECK(fr.totalLit() > 0);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// On
// ---------------------------------------------------------------------------

TEST_CASE("On cycles repeat the brightness ramp", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 2;
    o.h = 2;
    o.periods = Range(0, 19);
    Frames one = f.Render("On", "E_TEXTCTRL_Eff_On_Start=0,E_TEXTCTRL_Eff_On_End=100", kRed, o);
    Frames two = f.Render("On", "E_TEXTCTRL_Eff_On_Start=0,E_TEXTCTRL_Eff_On_End=100,E_TEXTCTRL_On_Cycles=2.0", kRed, o);
    // One cycle climbs all the way; two cycles reach the top halfway and start again.
    CHECK(one.at(9, 0, 0).red < one.at(19, 0, 0).red);
    CHECK(two.at(9, 0, 0).red > 200);
    CHECK(two.at(10, 0, 0).red < 60);
    CHECK(two.at(19, 0, 0).red > 200);
}

TEST_CASE("On with a left to right colour curve grades across the buffer", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 10;
    o.h = 3;
    Frames fr = f.Render("On", "", CurvePalette(1), o);
    const xlColor& left = fr.at(0, 0, 1);
    const xlColor& right = fr.at(0, 9, 1);
    CHECK(left.red > left.blue);
    CHECK(right.blue > right.red);
    for (int y = 0; y < 3; ++y) CHECK(SameRgba(fr.at(0, 4, y), fr.at(0, 4, 0)));

    // The brightness ramp and transparency are applied to the spatial colours too.
    Frames dim = f.Render("On", "E_TEXTCTRL_Eff_On_Start=50,E_TEXTCTRL_Eff_On_End=50,E_SLIDER_On_Transparency=50", CurvePalette(1), o);
    CHECK(dim.at(0, 0, 1).red < left.red);
    CHECK(std::abs((int)dim.at(0, 0, 1).alpha - 128) <= 1);
}

TEST_CASE("On with a radial colour curve is symmetric about the centre", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 9;
    o.h = 9;
    Frames fr = f.Render("On", "", CurvePalette(5), o);
    CHECK_FALSE(SameRgba(fr.at(0, 4, 4), fr.at(0, 0, 4)));
    CHECK(SameRgba(fr.at(0, 0, 4), fr.at(0, 8, 4)));
    CHECK(SameRgba(fr.at(0, 4, 0), fr.at(0, 4, 8)));
    CHECK(fr.lit(0) == 81);
}

TEST_CASE("On with a time colour curve changes colour over the effect", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 3;
    o.h = 3;
    o.periods = { 0, 19 };
    Frames fr = f.Render("On", "", CurvePalette(0), o);
    CHECK(fr.at(0, 1, 1).red > fr.at(0, 1, 1).blue);
    CHECK(fr.at(1, 1, 1).blue > fr.at(1, 1, 1).red);
    CHECK(SameRgba(fr.at(1, 0, 0), fr.at(1, 2, 2)));
}

TEST_CASE("On drives the colour channels of a DMX fixture", "[effects][classic2]") {
    Fx f(kMovingHead);
    Opts o;
    o.w = 12;
    o.h = 1;
    Frames fr = f.Render("On", "", kRed, o);
    // Channels 9..11 are R, G, B.
    CHECK(fr.at(0, 8, 0).red == 255);
    CHECK(fr.at(0, 9, 0).red == 0);
    CHECK(fr.at(0, 10, 0).red == 0);
}

TEST_CASE("On background drawing reflects the ramp and shimmer settings", "[effects][classic2]") {
    Fx f;
    RenderableEffect* on = f.Get("On");
    Effect* e = f.layer->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=20,E_TEXTCTRL_Eff_On_End=100", kRed, 0, 1000, 0, false);
    Effect::EnableBackgroundDisplayLists(false);

    RecordingCVA plain;
    CHECK(on->DrawEffectBackground(e, 0, 0, 100, 10, plain, nullptr, false) == 2);
    REQUIRE(plain.v.size() == 6);
    // start colour (left edge) is dimmer than the end colour (right edge)
    CHECK(plain.v[0].c.red < plain.v[2].c.red);

    RecordingCVA ramp;
    CHECK(on->DrawEffectBackground(e, 0, 0, 100, 10, ramp, nullptr, true) == 2);
    CHECK(ramp.v.size() == 6);
    // ramp height follows the intensity: the start vertex sits higher (towards y1) at the end
    CHECK(ramp.v[0].y > ramp.v[3].y);

    Effect* shim = f.layer->AddEffect(0, "On", "E_CHECKBOX_On_Shimmer=1", kRed, 0, 1000, 0, false);
    RecordingCVA s;
    on->DrawEffectBackground(shim, 0, 0, 100, 10, s, nullptr, true);
    CHECK(s.v.size() > 6);
    bool anyBlack = false;
    for (const auto& v : s.v) anyBlack |= (v.c.red == 0 && v.c.green == 0 && v.c.blue == 0);
    CHECK(anyBlack);
    Effect::EnableBackgroundDisplayLists(true);
}

TEST_CASE("On fills the background display list when rendering", "[effects][classic2]") {
    Effect::EnableBackgroundDisplayLists(true);
    Fx f;
    Opts o;
    o.w = 2;
    o.h = 2;
    o.periods = { 0, 1 };
    f.Render("On", "E_TEXTCTRL_Eff_On_Start=10,E_TEXTCTRL_Eff_On_End=90", kRed, o);
    Effect* ramped = f.last;
    CHECK(ramped->HasBackgroundDisplayList());
    f.Render("On", "", kRed, o);
    Effect* flat = f.last;
    CHECK(flat->HasBackgroundDisplayList());
    f.Render("On", "E_CHECKBOX_On_Shimmer=1", kRedBlue, o);
    CHECK(f.last->HasBackgroundDisplayList());

    RecordingCVA acc;
    int sz = f.Get("On")->DrawEffectBackground(ramped, 0, 0, 100, 10, acc, nullptr, false);
    CHECK(sz == ramped->GetBackgroundDisplayList().iconSize);
    CHECK(!acc.v.empty());
}

// ---------------------------------------------------------------------------
// Off
// ---------------------------------------------------------------------------

TEST_CASE("Off Black to Transparent clears only black pixels", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 4;
    o.h = 2;
    o.prefill = [](int, int w, int h, xlColor* px) {
        for (int i = 0; i < w * h; ++i) px[i] = (i % 2) ? xlColor(200, 10, 10, 255) : xlBLACK;
    };
    Frames fr = f.Render("Off", "E_CHOICE_Off_Style=Black -> Transparent", kRed, o);
    for (int i = 0; i < 8; ++i) {
        const xlColor& c = fr.f[0][i];
        if (i % 2) {
            CHECK(SameRgba(c, xlColor(200, 10, 10, 255)));
        } else {
            CHECK(c.alpha == 0);
        }
    }
}

TEST_CASE("Off Transparent to Black paints only clear pixels", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 4;
    o.h = 2;
    o.prefill = [](int, int w, int h, xlColor* px) {
        for (int i = 0; i < w * h; ++i) px[i] = (i % 2) ? xlColor(0, 90, 0, 255) : xlCLEAR;
    };
    Frames fr = f.Render("Off", "E_CHOICE_Off_Style=Transparent -> Black", kRed, o);
    for (int i = 0; i < 8; ++i) {
        const xlColor& c = fr.f[0][i];
        if (i % 2) {
            CHECK(SameRgba(c, xlColor(0, 90, 0, 255)));
        } else {
            CHECK(SameRgba(c, xlBLACK));
        }
    }
}

TEST_CASE("Off on a DMX fixture with a dimmer only touches the dimmer channel", "[effects][classic2]") {
    Fx f(kMovingHead);
    Opts o;
    o.w = 12;
    o.h = 1;
    o.prefill = [](int, int w, int, xlColor* px) {
        for (int i = 0; i < w; ++i) px[i] = xlColor(100, 100, 100, 255);
    };
    Frames black = f.Render("Off", "E_CHOICE_Off_Style=Black", kRed, o);
    for (int x = 0; x < 11; ++x) CHECK(black.at(0, x, 0).red == 100);
    CHECK(black.at(0, 11, 0).red == 0);

    // Black -> Transparent: the dimmer is not black, so nothing changes.
    Frames b2t = f.Render("Off", "E_CHOICE_Off_Style=Black -> Transparent", kRed, o);
    CHECK(b2t.at(0, 11, 0).alpha == 255);

    o.prefill = [](int, int w, int, xlColor* px) {
        for (int i = 0; i < w; ++i) px[i] = (i == 11) ? xlBLACK : xlCLEAR;
    };
    Frames b2t2 = f.Render("Off", "E_CHOICE_Off_Style=Black -> Transparent", kRed, o);
    CHECK(b2t2.at(0, 11, 0).alpha == 0);

    o.prefill = [](int, int w, int, xlColor* px) {
        for (int i = 0; i < w; ++i) px[i] = xlCLEAR;
    };
    Frames t2b = f.Render("Off", "E_CHOICE_Off_Style=Transparent -> Black", kRed, o);
    CHECK(SameRgba(t2b.at(0, 11, 0), xlBLACK));
    CHECK(t2b.at(0, 0, 0).alpha == 0);
}

TEST_CASE("Off settings check warns about pointless canvas combinations", "[effects][classic2]") {
    Fx f;
    RenderableEffect* off = f.Get("Off");
    Effect* e = f.layer->AddEffect(0, "Off", "", "", 0, 1000, 0, false);
    SettingsMap s;
    s["T_CHECKBOX_Canvas"] = "1";
    s["E_CHOICE_Off_Style"] = "Black";
    CHECK(off->CheckEffectSettings(s, nullptr, f.model, e, false).size() == 1);
    s["T_CHECKBOX_Canvas"] = "0";
    s["E_CHOICE_Off_Style"] = "Transparent";
    CHECK(off->CheckEffectSettings(s, nullptr, f.model, e, false).size() == 1);
    s["T_CHECKBOX_Canvas"] = "1";
    CHECK(off->CheckEffectSettings(s, nullptr, f.model, e, false).empty());
    s["B_CHECKBOX_OverlayBkg"] = "1";
    s["E_CHOICE_Off_Style"] = "Black";
    CHECK(off->CheckEffectSettings(s, nullptr, f.model, e, false).empty());
    // the base check flags legacy buffer styles
    s["B_CHOICE_BufferStyle"] = "** Single Line Legacy";
    CHECK(off->CheckEffectSettings(s, nullptr, f.model, e, false).size() == 1);
}

// ---------------------------------------------------------------------------
// Adjust
// ---------------------------------------------------------------------------

namespace {
Frames RunAdjust(Fx& f, const std::string& settings, int w = 3, int h = 2) {
    Opts o;
    o.w = w;
    o.h = h;
    o.prefill = [](int, int w, int h, xlColor* px) {
        for (int i = 0; i < w * h; ++i) px[i] = xlColor(100, 50, 0, 255);
    };
    return f.Render("Adjust", settings, kRed, o);
}
void CheckAll(const Frames& fr, int r, int g, int b) {
    for (const auto& c : fr.f[0]) {
        CHECK((int)c.red == r);
        CHECK((int)c.green == g);
        CHECK((int)c.blue == b);
        CHECK(c.alpha == 255);
    }
}
} // namespace

TEST_CASE("Adjust actions change every channel of the lower layers", "[effects][classic2]") {
    Fx f;
    CheckAll(RunAdjust(f, "E_CHOICE_Action=None"), 100, 50, 0);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Adjust By Value,E_SPINCTRL_Value1=10"), 110, 60, 10);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Adjust By Value,E_SPINCTRL_Value1=-60"), 40, 0, 0);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Adjust By Percentage,E_SPINCTRL_Value1=50"), 150, 75, 0);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Set Minimum,E_SPINCTRL_Value1=80"), 100, 80, 80);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Set Maximum,E_SPINCTRL_Value1=60"), 60, 50, 0);
    // Value1 > Value2 is swapped into a 20..60 range
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Set Range,E_SPINCTRL_Value1=60,E_SPINCTRL_Value2=20"), 60, 50, 20);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Shift With Wrap By Value,E_SPINCTRL_Value1=200"), 44, 250, 200);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Shift With Wrap By Value,E_SPINCTRL_Value1=-60"), 40, 246, 196);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Shift With Wrap By Percentage,E_SPINCTRL_Value1=50"), 100, 50, 0);
    // 50 is closer to 40 than to 70; 100 and 0 are outside the range
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Prevent Range,E_SPINCTRL_Value1=70,E_SPINCTRL_Value2=40"), 100, 40, 0);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Prevent Range,E_SPINCTRL_Value1=30,E_SPINCTRL_Value2=60"), 100, 60, 0);
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Reverse"), 155, 205, 255);
}

TEST_CASE("Adjust honours the starting channel, nth channel and count", "[effects][classic2]") {
    Fx f;
    // Every third channel starting at channel 2: only the green channels move.
    CheckAll(RunAdjust(f, "E_CHOICE_Action=Reverse,E_SPINCTRL_StartingAt=2,E_SPINCTRL_NthChannel=3"), 100, 205, 0);

    // Only the first two channels (red and green of the first pixel).
    Frames fr = RunAdjust(f, "E_CHOICE_Action=Reverse,E_SPINCTRL_Count=2");
    CHECK((int)fr.f[0][0].red == 155);
    CHECK((int)fr.f[0][0].green == 205);
    CHECK((int)fr.f[0][0].blue == 0);
    CHECK((int)fr.f[0][1].red == 100);
}

TEST_CASE("Adjust on a single colour DMX model treats each pixel as one channel", "[effects][classic2]") {
    Fx f(kMovingHead);
    Frames fr = RunAdjust(f, "E_CHOICE_Action=Adjust By Value,E_SPINCTRL_Value1=5", 12, 1);
    // single colour: the red value is adjusted and copied to all three components
    for (const auto& c : fr.f[0]) {
        CHECK((int)c.red == 105);
        CHECK((int)c.green == 105);
        CHECK((int)c.blue == 105);
    }
}

TEST_CASE("Adjust settings check warns when canvas mode is off", "[effects][classic2]") {
    Fx f;
    RenderableEffect* adj = f.Get("Adjust");
    Effect* e = f.layer->AddEffect(0, "Adjust", "", "", 0, 1000, 0, false);
    SettingsMap s;
    CHECK(adj->CheckEffectSettings(s, nullptr, f.model, e, false).size() == 1);
    s["T_CHECKBOX_Canvas"] = "1";
    CHECK(adj->CheckEffectSettings(s, nullptr, f.model, e, false).empty());
}

// ---------------------------------------------------------------------------
// Curtain
// ---------------------------------------------------------------------------

TEST_CASE("Curtain open shrinks and close grows the lit area", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 10;
    o.h = 6;
    o.periods = { 0, 5, 10, 15, 19 };
    Frames open = f.Render("Curtain", "E_CHOICE_Curtain_Effect=open,E_SLIDER_Curtain_Swag=0", kRedBlue, o);
    Frames close = f.Render("Curtain", "E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=0", kRedBlue, o);
    for (size_t i = 1; i < open.f.size(); ++i) {
        CHECK(open.lit(i) <= open.lit(i - 1));
        CHECK(close.lit(i) >= close.lit(i - 1));
    }
    CHECK(open.lit(0) > open.lit(4));
    CHECK(close.lit(4) > close.lit(0));
    CHECK(NoGreen(open));
}

TEST_CASE("Curtain edges mirror each other", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 10;
    o.h = 8;
    o.periods = { 3, 8 };
    const std::string base = "E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=0,E_CHOICE_Curtain_Edge=";
    Frames left = f.Render("Curtain", base + "left", kRed, o);
    Frames right = f.Render("Curtain", base + "right", kRed, o);
    Frames bottom = f.Render("Curtain", base + "bottom", kRed, o);
    Frames top = f.Render("Curtain", base + "top", kRed, o);
    Frames center = f.Render("Curtain", base + "center", kRed, o);
    Frames middle = f.Render("Curtain", base + "middle", kRed, o);
    for (size_t i = 0; i < 2; ++i) {
        REQUIRE(left.lit(i) > 0);
        REQUIRE(bottom.lit(i) > 0);
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 10; ++x) {
                INFO("frame " << i << " pixel " << x << "," << y);
                CHECK(Lit(left.at(i, x, y)) == Lit(right.at(i, 9 - x, y)));
                CHECK(Lit(bottom.at(i, x, y)) == Lit(top.at(i, x, 7 - y)));
                CHECK(Lit(center.at(i, x, y)) == Lit(center.at(i, 9 - x, y)));
                CHECK(Lit(middle.at(i, x, y)) == Lit(middle.at(i, x, 7 - y)));
            }
        }
        // vertical edges light whole rows, horizontal edges whole columns
        for (int x = 0; x < 10; ++x) CHECK(Lit(bottom.at(i, x, 7)) == Lit(bottom.at(i, 0, 7)));
        for (int y = 0; y < 8; ++y) CHECK(Lit(left.at(i, 9, y)) == Lit(left.at(i, 9, 0)));
    }
}

TEST_CASE("Curtain open then close reverses direction halfway", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 10;
    o.h = 4;
    o.periods = Range(0, 19);
    Frames oc = f.Render("Curtain", "E_CHOICE_Curtain_Effect=open then close,E_SLIDER_Curtain_Swag=0", kRed, o);
    Frames co = f.Render("Curtain", "E_CHOICE_Curtain_Effect=close then open,E_SLIDER_Curtain_Swag=0", kRed, o);
    // open then close: lit area falls then rises again
    CHECK(oc.lit(8) < oc.lit(0));
    CHECK(oc.lit(19) > oc.lit(11));
    // close then open is the opposite
    CHECK(co.lit(8) > co.lit(0));
    CHECK(co.lit(19) < co.lit(11));
}

TEST_CASE("Curtain repeat and speed cycle the curtain more than once", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 10;
    o.h = 4;
    o.periods = Range(0, 19);
    Frames once = f.Render("Curtain", "E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=0", kRed, o);
    Frames fast = f.Render("Curtain", "E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=0,E_TEXTCTRL_Curtain_Speed=2.0", kRed, o);
    Frames rep = f.Render("Curtain", "E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=0,E_TEXTCTRL_Curtain_Speed=2.0,E_CHECKBOX_Curtain_Repeat=1", kRed, o);
    // without repeat, double speed closes fully by halfway and stays closed
    CHECK(fast.lit(12) == 40);
    CHECK(once.lit(12) < 40);
    // with repeat it starts over in the second half
    CHECK(rep.lit(11) < rep.lit(9));
}

TEST_CASE("Curtain swag drapes the leading edge", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 20;
    o.h = 8;
    o.periods = { 8 };
    for (const char* edge : { "left", "right", "bottom", "top" }) {
        INFO("edge " << edge);
        Frames fr = f.Render("Curtain", std::string("E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=10,E_CHOICE_Curtain_Edge=") + edge, kRed, o);
        Frames flat = f.Render("Curtain", std::string("E_CHOICE_Curtain_Effect=close,E_SLIDER_Curtain_Swag=0,E_CHOICE_Curtain_Edge=") + edge, kRed, o);
        // the swag adds a partially lit band beyond the straight edge
        CHECK(fr.lit(0) > flat.lit(0));
    }
}

// ---------------------------------------------------------------------------
// Spirals
// ---------------------------------------------------------------------------

TEST_CASE("Spirals blend and 3D take the CPU path and stay on palette", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 10;
    o.periods = { 0, 5, 10 };
    Frames plain = f.Render("Spirals", "", kRedBlue, o);
    Frames blend = f.Render("Spirals", "E_CHECKBOX_Spirals_Blend=1", kRedBlue, o);
    Frames d3 = f.Render("Spirals", "E_CHECKBOX_Spirals_3D=1", kRedBlue, o);
    Frames d3r = f.Render("Spirals", "E_CHECKBOX_Spirals_3D=1,E_CHECKBOX_Spirals_Reverse3D=1", kRedBlue, o);
    CHECK(blend.totalLit() > 0);
    CHECK(NoGreen(blend));
    CHECK_FALSE(blend == plain);
    CHECK(NoGreen(d3));
    CHECK_FALSE(d3 == d3r);
    // 3D fades the strand: some lit pixels are dimmer than the full palette colour
    bool dim = false;
    for (const auto& c : d3.f[0]) dim |= Lit(c) && c.red > 0 && c.red < 250;
    CHECK(dim);

    o.allowAlpha = true;
    Frames d3a = f.Render("Spirals", "E_CHECKBOX_Spirals_3D=1", kRedBlue, o);
    bool partial = false;
    for (const auto& c : d3a.f[0]) partial |= c.alpha > 0 && c.alpha < 255;
    CHECK(partial);
}

TEST_CASE("Spirals grow and shrink change the strand thickness over time", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 6;
    o.periods = { 0, 10, 19 };
    const std::string base = "E_SLIDER_Spirals_Thickness=0,E_TEXTCTRL_Spirals_Movement=1.0,";
    Frames grow = f.Render("Spirals", base + "E_CHECKBOX_Spirals_Grow=1", kRedBlue, o);
    Frames shrink = f.Render("Spirals", base + "E_CHECKBOX_Spirals_Shrink=1", kRedBlue, o);
    Frames both = f.Render("Spirals", base + "E_CHECKBOX_Spirals_Grow=1,E_CHECKBOX_Spirals_Shrink=1", kRedBlue, o);
    CHECK(grow.lit(2) > grow.lit(0));
    CHECK(shrink.lit(0) > shrink.lit(2));
    CHECK(both.lit(1) > both.lit(0));
    CHECK(both.lit(1) > both.lit(2));
}

TEST_CASE("Spirals movement direction follows the movement sign", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 1;
    o.periods = { 0, 1 };
    Frames fwd = f.Render("Spirals", "E_TEXTCTRL_Spirals_Movement=1.0,E_SLIDER_Spirals_Thickness=0", kRedBlue, o);
    Frames back = f.Render("Spirals", "E_TEXTCTRL_Spirals_Movement=-1.0,E_SLIDER_Spirals_Thickness=0", kRedBlue, o);
    Frames still = f.Render("Spirals", "E_TEXTCTRL_Spirals_Movement=0,E_SLIDER_Spirals_Thickness=0", kRedBlue, o);
    CHECK((fwd.f[0] == back.f[0]));
    CHECK_FALSE((fwd.f[1] == back.f[1]));
    CHECK((still.f[0] == still.f[1]));
}

TEST_CASE("Spirals option sweep covers value curves, spatial colours and DMX", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 8;
    o.periods = { 0, 7 };
    Frames spatial = f.Render("Spirals", "", CurvePalette(1), o);
    CHECK(spatial.totalLit() > 0);
    CHECK(NoGreen(spatial));
    Frames vc = f.Render("Spirals", "E_VALUECURVE_Spirals_Rotation=" + Ramp("Spirals_Rotation", -300, 300, -20, 20), kRedBlue, o);
    CHECK(vc.totalLit() > 0);
    Frames vcc = f.Render("Spirals", "E_VALUECURVE_Spirals_Count=" + Ramp("Spirals_Count", 1, 5, 1, 5), kRedBlue, o);
    CHECK(vcc.totalLit() > 0);
    // a zero count is treated as one
    CHECK(f.Render("Spirals", "E_SLIDER_Spirals_Count=0", kRedBlue, o) == f.Render("Spirals", "E_SLIDER_Spirals_Count=1", kRedBlue, o));
    // blend combined with 3D, both with and without an alpha channel
    for (bool alpha : { false, true }) {
        Opts oa = o;
        oa.allowAlpha = alpha;
        Frames b3 = f.Render("Spirals", "E_CHECKBOX_Spirals_Blend=1,E_CHECKBOX_Spirals_3D=1", kRedBlue, oa);
        Frames b3r = f.Render("Spirals", "E_CHECKBOX_Spirals_Blend=1,E_CHECKBOX_Spirals_3D=1,E_CHECKBOX_Spirals_Reverse3D=1", kRedBlue, oa);
        CHECK(b3.totalLit() > 0);
        CHECK(NoGreen(b3));
        CHECK_FALSE(b3 == b3r);
    }
    CHECK(f.Get("Spirals")->SupportsLinearColorCurves(SettingsMap()));
    SettingsMap blendOn;
    blendOn["E_CHECKBOX_Spirals_Blend"] = "1";
    CHECK_FALSE(f.Get("Spirals")->SupportsLinearColorCurves(blendOn));

    Fx d(kMovingHead);
    o.w = 12;
    o.h = 1;
    Frames dmx = d.Render("Spirals", "", kRed, o);
    // DMX output goes through the fixture's colour channels: green and blue stay dark for red
    CHECK(dmx.at(0, 9, 0).red == 0);
    CHECK(dmx.at(0, 10, 0).red == 0);
    CHECK(DmxRedSeen(dmx));
}

// ---------------------------------------------------------------------------
// Fan
// ---------------------------------------------------------------------------

TEST_CASE("Fan grows its blades during the ramp up", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 20;
    o.h = 20;
    o.periods = { 0, 4, 10, 16, 19 };
    Frames fr = f.Render("Fan", "E_SLIDER_Fan_Duration=50", kRedBlue, o);
    CHECK(fr.lit(1) > fr.lit(0));
    CHECK(fr.lit(2) >= fr.lit(1));
    CHECK(fr.lit(4) < fr.lit(2));
    CHECK(NoGreen(fr));
    Frames full = f.Render("Fan", "E_SLIDER_Fan_Duration=100", kRedBlue, o);
    CHECK(full.lit(0) > fr.lit(0));
}

TEST_CASE("Fan options change the output", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 16;
    o.h = 16;
    o.periods = { 3, 9 };
    Frames base = f.Render("Fan", "", kRedBlue, o);
    REQUIRE(base.totalLit() > 0);
    for (const char* s : { "E_CHECKBOX_Fan_Reverse=1", "E_CHECKBOX_Fan_Blend_Edges=0", "E_SLIDER_Fan_Num_Elements=3,E_SLIDER_Fan_Element_Width=50",
                           "E_SLIDER_Fan_Num_Blades=6", "E_SLIDER_Fan_Accel=5", "E_CHECKBOX_Fan_Scale=0,E_SLIDER_Fan_End_Radius=6",
                           "E_SLIDER_Fan_Start_Radius=60,E_SLIDER_Fan_End_Radius=10", "E_SLIDER_Fan_CenterX=20,E_SLIDER_Fan_CenterY=80" }) {
        INFO(s);
        Frames fr = f.Render("Fan", s, kRedBlue, o);
        CHECK(fr.totalLit() > 0);
        CHECK_FALSE(fr == base);
    }
    // blend edges off: only exact palette colours appear
    Frames hard = f.Render("Fan", "E_CHECKBOX_Fan_Blend_Edges=0", kRedBlue, o);
    for (const auto& c : hard.f[0]) {
        if (Lit(c)) CHECK((SameRgba(c, xlColor(255, 0, 0)) || SameRgba(c, xlColor(0, 0, 255))));
    }
    // a shrinking fan (start radius larger than the end) at the very end of the effect
    Opts late = o;
    late.periods = { 18 };
    CHECK(f.Render("Fan", "E_SLIDER_Fan_Start_Radius=60,E_SLIDER_Fan_End_Radius=10", kRedBlue, late).totalLit() > 0);
    // zero radius: nothing to twist, nothing much drawn, no crash
    Frames zero = f.Render("Fan", "E_SLIDER_Fan_Start_Radius=0,E_SLIDER_Fan_End_Radius=0", kRedBlue, o);
    CHECK(zero.totalLit() <= 2);
}

TEST_CASE("Fan spatial colours use the CPU path and match the GPU path's footprint", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 16;
    o.h = 16;
    o.periods = { 5 };
    // A constant spatial curve is the same colour as the plain palette entry.
    const std::string constCurve = "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Timecurve=1|Values=x=0.000^c=#FF0000;x=1.000^c=#FF0000;|,C_CHECKBOX_Palette1=1";
    Frames cpu = f.Render("Fan", "E_CHECKBOX_Fan_Blend_Edges=0", constCurve, o);
    Frames ispc = f.Render("Fan", "E_CHECKBOX_Fan_Blend_Edges=0", kRed, o);
    REQUIRE(ispc.lit(0) > 0);
    int agree = 0;
    for (int i = 0; i < 256; ++i) agree += Lit(cpu.f[0][i]) == Lit(ispc.f[0][i]) ? 1 : 0;
    CHECK(agree >= 240);
    Frames grad = f.Render("Fan", "", CurvePalette(1), o);
    CHECK(grad.totalLit() > 0);
    CHECK(NoGreen(grad));
    Frames gradRev = f.Render("Fan", "E_CHECKBOX_Fan_Reverse=1", CurvePalette(1), o);
    CHECK(gradRev.totalLit() > 0);
    CHECK_FALSE(gradRev == grad);
    o.allowAlpha = true;
    Frames alpha = f.Render("Fan", "", CurvePalette(1), o);
    bool partial = false;
    for (const auto& c : alpha.f[0]) partial |= c.alpha > 0 && c.alpha < 255;
    CHECK(partial);
}

TEST_CASE("Fan on a DMX fixture and background drawing", "[effects][classic2]") {
    Fx d(kMovingHead);
    Opts o;
    o.w = 12;
    o.h = 1;
    o.periods = { 0, 5 };
    Frames dmx = d.Render("Fan", "E_SLIDER_Fan_CenterX=0,E_SLIDER_Fan_CenterY=0,E_SLIDER_Fan_Start_Radius=0,E_CHECKBOX_Fan_Blend_Edges=0,E_SLIDER_Fan_Blade_Width=100", kRed, o);
    for (size_t i = 0; i < dmx.f.size(); ++i) {
        CHECK(dmx.at(i, 9, 0).red == 0);
        CHECK(dmx.at(i, 10, 0).red == 0);
    }
    CHECK(DmxRedSeen(dmx));

    Fx f;
    RenderableEffect* fan = f.Get("Fan");
    Effect* one = f.layer->AddEffect(0, "Fan", "", kRed, 0, 1000, 0, false);
    Effect* three = f.layer->AddEffect(0, "Fan", "E_SLIDER_Fan_Duration=50", kRGB, 0, 1000, 0, false);
    RecordingCVA a1, a3;
    CHECK(fan->DrawEffectBackground(one, 0, 0, 100, 10, a1, nullptr, false) == 2);
    CHECK(fan->DrawEffectBackground(three, 0, 0, 100, 10, a3, nullptr, false) == 2);
    CHECK(a3.v.size() == 3 * a1.v.size());
    for (const auto& v : a3.v) {
        CHECK(v.x >= 0);
        CHECK(v.x <= 100);
    }
}

// ---------------------------------------------------------------------------
// Galaxy
// ---------------------------------------------------------------------------

TEST_CASE("Galaxy options change the spiral", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 20;
    o.h = 20;
    o.periods = { 4, 10 };
    Frames base = f.Render("Galaxy", "", kRedBlue, o);
    REQUIRE(base.totalLit() > 0);
    CHECK(NoGreen(base));
    for (const char* s : { "E_CHECKBOX_Galaxy_Reverse=1", "E_CHECKBOX_Galaxy_Inward=1", "E_CHECKBOX_Galaxy_Blend_Edges=0", "E_CHECKBOX_Galaxy_Scale=0",
                           "E_SLIDER_Galaxy_Accel=4", "E_SLIDER_Galaxy_Duration=60", "E_SLIDER_Galaxy_Start_Angle=90" }) {
        INFO(s);
        Frames fr = f.Render("Galaxy", s, kRedBlue, o);
        CHECK(fr.totalLit() > 0);
        CHECK_FALSE(fr == base);
    }
    Frames none = f.Render("Galaxy", "E_SLIDER_Galaxy_Revolutions=0", kRedBlue, o);
    CHECK(none.totalLit() == 0);
    // a short tail on many colours forces the minimum colour length
    Frames shortTail = f.Render("Galaxy", "E_SLIDER_Galaxy_Revolutions=4,E_SLIDER_Galaxy_Duration=99", kFive, o);
    CHECK(shortTail.totalLit() > 0);
}

TEST_CASE("Galaxy large buffers render in parallel bands", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 150;
    o.h = 150;
    o.periods = { 10 };
    Frames fr = f.Render("Galaxy", "E_SLIDER_Galaxy_End_Radius=100,E_SLIDER_Galaxy_End_Width=20", kRedBlue, o);
    CHECK(fr.lit(0) > 100);
    CHECK(NoGreen(fr));
    Frames again = f.Render("Galaxy", "E_SLIDER_Galaxy_End_Radius=100,E_SLIDER_Galaxy_End_Width=20", kRedBlue, o);
    CHECK(fr == again);
}

TEST_CASE("Galaxy on a DMX fixture uses the scalar path for every option", "[effects][classic2]") {
    Fx d(kMovingHead);
    Opts o;
    o.w = 12;
    o.h = 1;
    o.periods = { 2, 8, 14 };
    for (const char* s : { "", "E_CHECKBOX_Galaxy_Reverse=1", "E_CHECKBOX_Galaxy_Inward=1", "E_CHECKBOX_Galaxy_Inward=1,E_CHECKBOX_Galaxy_Reverse=1",
                           "E_CHECKBOX_Galaxy_Blend_Edges=0", "E_CHECKBOX_Galaxy_Blend_Edges=0,E_CHECKBOX_Galaxy_Inward=1",
                           "E_SLIDER_Galaxy_Start_Width=60,E_SLIDER_Galaxy_End_Width=60,E_SLIDER_Galaxy_Start_Radius=80,E_SLIDER_Galaxy_End_Radius=80" }) {
        INFO(s);
        Frames fr = d.Render("Galaxy", s, kRedBlue, o);
        for (size_t i = 0; i < fr.f.size(); ++i) {
            // the fixture's green channel never lights for a red/blue palette
            CHECK(fr.at(i, 9, 0).red == 0);
        }
    }
}

TEST_CASE("Galaxy background shows head and tail colours", "[effects][classic2]") {
    Fx f;
    RenderableEffect* g = f.Get("Galaxy");
    Effect* one = f.layer->AddEffect(0, "Galaxy", "", kRed, 0, 1000, 0, false);
    Effect* three = f.layer->AddEffect(0, "Galaxy", "", kRGB, 0, 1000, 0, false);
    RecordingCVA a1, a3;
    CHECK(g->DrawEffectBackground(one, 0, 0, 100, 10, a1, nullptr, false) == 2);
    CHECK(g->DrawEffectBackground(three, 0, 0, 100, 10, a3, nullptr, false) == 2);
    CHECK(!a1.v.empty());
    CHECK(a3.v.size() > a1.v.size());
}

// ---------------------------------------------------------------------------
// Shockwave
// ---------------------------------------------------------------------------

TEST_CASE("Shockwave ring expands from the centre", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 21;
    o.h = 21;
    o.periods = { 2, 10, 18 };
    Frames fr = f.Render("Shockwave", "E_SLIDER_Shockwave_Start_Radius=1,E_SLIDER_Shockwave_End_Radius=90,E_SLIDER_Shockwave_Start_Width=10,E_SLIDER_Shockwave_End_Width=10", kRedBlue, o);
    double r0 = fr.meanRadius(0, 10, 10), r1 = fr.meanRadius(1, 10, 10), r2 = fr.meanRadius(2, 10, 10);
    REQUIRE(r0 >= 0);
    CHECK(r1 > r0);
    CHECK(r2 > r1);
    CHECK(NoGreen(fr));
    // colour moves from the first to the last palette entry
    auto dominant = [&](size_t i) {
        long r = 0, b = 0;
        for (const auto& c : fr.f[i]) {
            r += c.red;
            b += c.blue;
        }
        return r > b ? 'r' : 'b';
    };
    CHECK(dominant(0) == 'r');
    CHECK(dominant(2) == 'b');
}

TEST_CASE("Shockwave cycles repeat the ring", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 15;
    o.h = 15;
    o.periods = { 3, 13 };
    Frames fr = f.Render("Shockwave", "E_SLIDER_Shockwave_Cycles=2,E_CHECKBOX_Shockwave_Blend_Edges=0", kRed, o);
    // with two cycles over 20 frames, frame 3 and frame 13 are at the same phase
    CHECK((fr.f[0] == fr.f[1]));
    for (const auto& c : fr.f[0]) {
        if (Lit(c)) CHECK(SameRgba(c, xlColor(255, 0, 0)));
    }
    Frames one = f.Render("Shockwave", "E_SLIDER_Shockwave_Cycles=1,E_CHECKBOX_Shockwave_Blend_Edges=0", kRed, o);
    CHECK_FALSE((one.f[0] == one.f[1]));
}

TEST_CASE("Shockwave fires on timing marks", "[effects][classic2]") {
    Fx f;
    f.AddTiming("Beats", { { 250, 300 }, { 600, 650 } }, "hit");
    Opts o;
    o.w = 15;
    o.h = 15;
    o.periods = { 2, 6, 10, 13, 19 };
    const std::string s = "E_CHOICE_Shockwave_TimingTrack=Beats,E_SLIDER_Shockwave_Duration=200,E_SLIDER_Shockwave_End_Radius=60";
    Frames fr = f.Render("Shockwave", s, kRed, o);
    CHECK(fr.lit(0) == 0); // before the first mark
    CHECK(fr.lit(1) > 0);  // just after the 250ms mark
    CHECK(fr.lit(2) == 0); // the 200ms wave has finished
    CHECK(fr.lit(3) > 0);  // after the 600ms mark
    CHECK(fr.lit(4) == 0);

    // a label filter that matches nothing suppresses every wave
    Frames filtered = f.Render("Shockwave", s + ",E_TEXTCTRL_Shockwave_FilterLabel=nomatch", kRed, o);
    CHECK(filtered.totalLit() == 0);
    Frames matched = f.Render("Shockwave", s + ",E_TEXTCTRL_Shockwave_FilterLabel=hit", kRed, o);
    CHECK(matched == fr);

    // the background shows one band per mark inside the effect
    RecordingCVA acc;
    CHECK(f.Get("Shockwave")->DrawEffectBackground(f.last, 0, 0, 100, 10, acc, nullptr, false) == 2);
    CHECK(acc.v.size() == 12);

    RecordingCVA cyc;
    Effect* c3 = f.layer->AddEffect(0, "Shockwave", "E_SLIDER_Shockwave_Cycles=3", kRed, 0, 1000, 0, false);
    f.Get("Shockwave")->DrawEffectBackground(c3, 0, 0, 90, 10, cyc, nullptr, false);
    CHECK(cyc.v.size() == 18);
}

TEST_CASE("Shockwave renames its timing track", "[effects][classic2]") {
    Fx f;
    Effect* e = f.layer->AddEffect(0, "Shockwave", "E_CHOICE_Shockwave_TimingTrack=Old", kRed, 0, 1000, 0, false);
    Effect* other = f.layer->AddEffect(0, "Shockwave", "E_CHOICE_Shockwave_TimingTrack=Other", kRed, 1000, 2000, 0, false);
    f.Get("Shockwave")->RenameTimingTrack("Old", "New", e);
    f.Get("Shockwave")->RenameTimingTrack("Old", "New", other);
    CHECK(e->GetSettings().Get("E_CHOICE_Shockwave_TimingTrack", "") == "New");
    CHECK(other->GetSettings().Get("E_CHOICE_Shockwave_TimingTrack", "") == "Other");
}

TEST_CASE("Shockwave spatial colours, alpha and DMX", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 15;
    o.h = 15;
    o.periods = { 5, 12 };
    Frames sp = f.Render("Shockwave", "E_SLIDER_Shockwave_End_Radius=60", CurvePalette(1), o);
    CHECK(sp.totalLit() > 0);
    CHECK(NoGreen(sp));
    Frames hard = f.Render("Shockwave", "E_SLIDER_Shockwave_End_Radius=60,E_CHECKBOX_Shockwave_Blend_Edges=0", CurvePalette(1), o);
    CHECK(hard.totalLit() >= sp.totalLit());
    o.allowAlpha = true;
    Frames alpha = f.Render("Shockwave", "E_SLIDER_Shockwave_End_Radius=60", CurvePalette(1), o);
    bool partial = false;
    for (const auto& c : alpha.f[0]) partial |= c.alpha > 0 && c.alpha < 255;
    CHECK(partial);

    Fx d(kMovingHead);
    o.w = 12;
    o.h = 1;
    Frames dmx = d.Render("Shockwave", "E_SLIDER_Shockwave_CenterX=0,E_SLIDER_Shockwave_CenterY=0,E_SLIDER_Shockwave_Start_Radius=0,E_SLIDER_Shockwave_End_Radius=0,E_CHECKBOX_Shockwave_Blend_Edges=0", kRed, o);
    CHECK(dmx.at(0, 9, 0).red == 0);
    CHECK(DmxRedSeen(dmx));
}

// ---------------------------------------------------------------------------
// Circles
// ---------------------------------------------------------------------------

TEST_CASE("Circles radial rings are symmetric about the centre", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 21;
    o.h = 21;
    o.periods = { 10, 30 };
    o.endMs = 2000;
    Frames fr = f.Render("Circles", "E_CHECKBOX_Circles_Radial=1", kRedBlue, o);
    REQUIRE(fr.lit(1) > 0);
    CHECK(NoGreen(fr));
    for (int y = 0; y < 21; ++y) {
        for (int x = 0; x < 21; ++x) {
            CHECK(SameRgba(fr.at(1, x, y), fr.at(1, 20 - x, y)));
            CHECK(SameRgba(fr.at(1, x, y), fr.at(1, x, 20 - y)));
        }
    }
    // The rings grow outwards over time.
    CHECK(fr.lit(1) >= fr.lit(0));

    // 3D radial rotates through hues, so colours outside the palette appear.
    Frames d3 = f.Render("Circles", "E_CHECKBOX_Circles_Radial_3D=1", kRedBlue, o);
    CHECK_FALSE(NoGreen(d3));

    // Moving the centre breaks the symmetry about the buffer middle.
    Frames off = f.Render("Circles", "E_CHECKBOX_Circles_Radial=1,E_SLIDER_Circles_XC=40,E_SLIDER_Circles_YC=-40", kRedBlue, o);
    CHECK_FALSE(off == fr);
}

TEST_CASE("Circles radial with spatial colours uses the scalar ring renderer", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 15;
    o.h = 15;
    o.periods = { 15 };
    Frames fr = f.Render("Circles", "E_CHECKBOX_Circles_Radial=1", CurvePalette(1), o);
    CHECK(fr.totalLit() > 0);
    Frames d3 = f.Render("Circles", "E_CHECKBOX_Circles_Radial_3D=1", CurvePalette(1), o);
    CHECK(d3.totalLit() > 0);
    CHECK_FALSE(fr == d3);
}

TEST_CASE("Circles ball modes render balls of the palette colours", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 16;
    o.h = 16;
    o.periods = Range(0, 6);
    for (const char* s : { "", "E_CHECKBOX_Circles_Bounce=1", "E_CHECKBOX_Circles_Linear_Fade=1", "E_CHECKBOX_Circles_Bubbles=1",
                           "E_CHECKBOX_Circles_Bubbles=1,E_CHECKBOX_Circles_Linear_Fade=1", "E_CHECKBOX_Circles_Plasma=1" }) {
        INFO(s);
        Frames a = f.Render("Circles", s, kRedBlue, o);
        CHECK(a.totalLit() > 0);
        if (std::string(s) != "E_CHECKBOX_Circles_Plasma=1") CHECK(NoGreen(a));
        // the draw through a frame snapshot is identical to the fused render
        Opts o2 = o;
        o2.viaAdvanceState = true;
        Frames b = f.Render("Circles", s, kRedBlue, o2);
        CHECK(a == b);
    }
    // spatial colours fall back to the scalar ball renderer
    Frames sp = f.Render("Circles", "", CurvePalette(1), o);
    CHECK(sp.totalLit() > 0);
    Frames spf = f.Render("Circles", "E_CHECKBOX_Circles_Linear_Fade=1", CurvePalette(1), o);
    CHECK(spf.totalLit() > 0);
    Frames spp = f.Render("Circles", "E_CHECKBOX_Circles_Plasma=1", CurvePalette(1), o);
    CHECK(spp.totalLit() > 0);
}

TEST_CASE("Circles reinitialises when the count or size changes mid effect", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 16;
    o.h = 16;
    o.periods = Range(0, 19);
    const std::string s = "E_VALUECURVE_Circles_Count=" + Ramp("Circles_Count", 1, 10, 1, 10) + ",E_VALUECURVE_Circles_Size=" + Ramp("Circles_Size", 1, 20, 2, 6);
    Frames fr = f.Render("Circles", s, kRedBlue, o);
    CHECK(fr.lit(19) > fr.lit(0));
    Frames again = f.Render("Circles", s, kRedBlue, o);
    CHECK(fr == again);

    // a count beyond the ball limit is clamped rather than overrunning the ball arrays
    Frames many = f.Render("Circles", "E_SLIDER_Circles_Count=50", kRedBlue, o);
    CHECK(many.totalLit() > 0);
    Frames manyRadial = f.Render("Circles", "E_SLIDER_Circles_Count=50,E_CHECKBOX_Circles_Radial=1", kRedBlue, o);
    CHECK(manyRadial.totalLit() > 0);

    SettingsMap radial;
    radial["CHECKBOX_Circles_Radial"] = "1";
    {
        RenderBuffer rb(&f.ctx, nullptr, f.model);
        rb.InitBuffer(4, 4, "None");
        CHECK(f.Get("Circles")->AdvanceState(f.last, radial, rb) == nullptr);
    }
    CHECK(f.Get("Circles")->GetFrameParallelism(radial) == RenderableEffect::FrameParallelism::Pure);
    CHECK(f.Get("Circles")->GetFrameParallelism(SettingsMap()) == RenderableEffect::FrameParallelism::Snapshottable);
}

TEST_CASE("Circles on a DMX fixture", "[effects][classic2]") {
    Fx d(kMovingHead);
    Opts o;
    o.w = 12;
    o.h = 1;
    o.periods = { 0, 5 };
    for (const char* s : { "", "E_CHECKBOX_Circles_Radial=1,E_SLIDER_Circles_XC=-50" }) {
        INFO(s);
        Frames fr = d.Render("Circles", s, kRed, o);
        CHECK(fr.at(1, 9, 0).red == 0);
        CHECK(fr.at(1, 10, 0).red == 0);
        CHECK(DmxRedSeen(fr));
    }
}

// ---------------------------------------------------------------------------
// Morph
// ---------------------------------------------------------------------------

TEST_CASE("Morph moves the line from the start position to the end", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 10;
    o.h = 20;
    o.periods = { 0, 1, 3 };
    // default: a horizontal line at the bottom morphs to the top
    Frames fr = f.Render("Morph", "", kRedBlue, o);
    double y0 = fr.meanY(0), y1 = fr.meanY(1), y2 = fr.meanY(2);
    REQUIRE(y0 >= 0);
    CHECK(y1 > y0);
    CHECK(y2 > y1);
    CHECK(NoGreen(fr));
    // swapping start and end reverses the motion
    Frames rev = f.Render("Morph", "E_SLIDER_Morph_Start_Y1=100,E_SLIDER_Morph_Start_Y2=100,E_SLIDER_Morph_End_Y1=0,E_SLIDER_Morph_End_Y2=0", kRedBlue, o);
    CHECK(rev.meanY(2) < rev.meanY(0));
}

TEST_CASE("Morph options sweep", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 12;
    o.periods = { 0, 3, 8, 15 };
    Frames base = f.Render("Morph", "", kRedBlue, o);
    for (const char* s : { "E_CHECKBOX_Morph_Start_Link=1", "E_CHECKBOX_Morph_End_Link=1", "E_CHECKBOX_ShowHeadAtStart=1",
                           "E_SLIDER_MorphDuration=0", "E_SLIDER_MorphAccel=5", "E_SLIDER_MorphStartLength=20,E_SLIDER_MorphEndLength=40",
                           "E_SLIDER_Morph_Start_X2=20,E_SLIDER_Morph_End_X2=20,E_SLIDER_Morph_Repeat_Count=2,E_SLIDER_Morph_Repeat_Skip=4",
                           "E_SLIDER_Morph_Start_X2=20,E_SLIDER_Morph_End_X2=20,E_SLIDER_Morph_Repeat_Count=2,E_SLIDER_Morph_Repeat_Skip=4,E_SLIDER_Morph_Stagger=50",
                           "E_SLIDER_Morph_Start_X2=20,E_SLIDER_Morph_End_X2=20,E_SLIDER_Morph_Repeat_Count=2,E_SLIDER_Morph_Repeat_Skip=4,E_SLIDER_Morph_Stagger=-50,E_CHECKBOX_ShowHeadAtStart=1",
                           "E_CHECKBOX_Morph_AutoRepeat=1,E_SLIDER_Morph_Repeat_Skip=4,E_SLIDER_Morph_Start_X2=20,E_SLIDER_Morph_End_X2=20",
                           "E_SLIDER_Morph_Start_X1=0,E_SLIDER_Morph_Start_Y1=0,E_SLIDER_Morph_Start_X2=0,E_SLIDER_Morph_Start_Y2=100,E_SLIDER_Morph_End_X1=100,E_SLIDER_Morph_End_Y1=0,E_SLIDER_Morph_End_X2=100,E_SLIDER_Morph_End_Y2=100,E_SLIDER_Morph_Repeat_Count=1" }) {
        INFO(s);
        Frames fr = f.Render("Morph", s, kRedBlue, o);
        CHECK(fr.totalLit() > 0);
        CHECK(NoGreen(fr));
        CHECK_FALSE(fr == base);
    }
    // side A longer than side B: B collapses to a single point
    Frames fanIn = f.Render("Morph", "E_SLIDER_Morph_Start_Y2=100", kRedBlue, o);
    CHECK(fanIn.totalLit() > 0);
    // palettes of 1, 3 and 5 colours select different head/tail colour schemes
    for (const char* p : { kRed, kRGB, kFive }) {
        Frames fr = f.Render("Morph", "", p, o);
        CHECK(fr.totalLit() > 0);
    }
    o.allowAlpha = true;
    Frames alpha = f.Render("Morph", "", kRedBlue, o);
    bool partial = false;
    for (const auto& px : alpha.f) {
        for (const auto& c : px) partial |= c.alpha > 0 && c.alpha < 255;
    }
    CHECK(partial);
}

TEST_CASE("Morph repeats are offset by the skip", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 12;
    o.periods = { 3 };
    // a three pixel wide band moving up; the motion is vertical so repeats step sideways
    const std::string narrow = "E_SLIDER_Morph_Start_X2=20,E_SLIDER_Morph_End_X2=20";
    Frames one = f.Render("Morph", narrow, kRed, o);
    Frames rep = f.Render("Morph", narrow + ",E_SLIDER_Morph_Repeat_Count=2,E_SLIDER_Morph_Repeat_Skip=4", kRed, o);
    Frames autoRep = f.Render("Morph", narrow + ",E_CHECKBOX_Morph_AutoRepeat=1,E_SLIDER_Morph_Repeat_Skip=4", kRed, o);
    auto colLit = [](const Frames& fr, int x) {
        for (int y = 0; y < fr.h; ++y) {
            if (Lit(fr.at(0, x, y))) return true;
        }
        return false;
    };
    REQUIRE(one.lit(0) > 0);
    CHECK(rep.lit(0) == 3 * one.lit(0));
    for (int x = 0; x < 12; ++x) {
        INFO("column " << x);
        CHECK(colLit(one, x) == (x <= 2));
        CHECK(colLit(rep, x) == (x % 4 <= 2 && x <= 10));
    }
    // auto repeat fills the width with the same spacing: 12 / (1 + 4 - 1) - 1 = 2 repeats
    CHECK(autoRep == rep);
}

TEST_CASE("Morph swap button exchanges start and end points", "[effects][classic2]") {
    Fx f;
    SettingsMap pal;
    SettingsMap s;
    s["E_SLIDER_Morph_Start_X1"] = "10";
    s["E_SLIDER_Morph_End_X1"] = "90";
    s["E_SLIDER_Morph_Start_Y2"] = "5";
    RenderableEffect* m = f.Get("Morph");
    CHECK(m->PressButton("E_BUTTON_MORPH_SWAP", pal, s));
    CHECK(s.Get("E_SLIDER_Morph_Start_X1", "") == "90");
    CHECK(s.Get("E_SLIDER_Morph_End_X1", "") == "10");
    CHECK(s.Get("E_SLIDER_Morph_End_Y2", "") == "5");
    CHECK(s.Get("E_SLIDER_Morph_Start_Y2", "") == "0");
    CHECK_FALSE(m->PressButton("E_BUTTON_OTHER", pal, s));
}

TEST_CASE("Morph settings check warns about excessive repeats", "[effects][classic2]") {
    Fx f;
    RenderableEffect* m = f.Get("Morph");
    Effect* e = f.layer->AddEffect(0, "Morph", "", kRed, 0, 1000, 0, false);
    SettingsMap s;
    CHECK(m->CheckEffectSettings(s, nullptr, f.model, e, false).empty());
    s["E_SLIDER_Morph_Repeat_Count"] = "250";
    s["E_SLIDER_Morph_Repeat_Skip"] = "100";
    CHECK(m->CheckEffectSettings(s, nullptr, f.model, e, false).size() == 1);
    // a value curve makes the settings impossible to validate
    s["E_VALUECURVE_Morph_Start_X1"] = Ramp("Morph_Start_X1", 0, 100, 0, 100);
    CHECK(m->CheckEffectSettings(s, nullptr, f.model, e, false).empty());
}

TEST_CASE("Morph background draws head and tail bands", "[effects][classic2]") {
    Fx f;
    RenderableEffect* m = f.Get("Morph");
    const std::string four = std::string(kRGB) + ",C_BUTTON_Palette4=#FFFFFF,C_CHECKBOX_Palette4=1";
    for (const std::string& p : { std::string(kRed), std::string(kRedBlue), std::string(kRGB), four, std::string(kFive) }) {
        INFO(p);
        Effect* e = f.layer->AddEffect(0, "Morph", "E_CHECKBOX_MorphUseHeadStartColor=1,E_CHECKBOX_MorphUseHeadEndColor=1", p, 0, 1000, 0, false);
        RecordingCVA acc;
        m->DrawEffectBackground(e, 0, 0, 100, 10, acc, nullptr, false);
        CHECK(!acc.v.empty());
        for (const auto& v : acc.v) {
            CHECK(v.x >= 0);
            CHECK(v.x <= 100);
        }
    }
}

// ---------------------------------------------------------------------------
// Kaleidoscope
// ---------------------------------------------------------------------------

TEST_CASE("Kaleidoscope only rearranges existing pixels", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 12;
    o.periods = { 0, 5 };
    o.prefill = FillPattern;
    std::set<uint32_t> source;
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 12; ++x) {
            xlColor c = PatternColor(x, y);
            source.insert(((uint32_t)c.red << 16) | ((uint32_t)c.green << 8) | c.blue);
        }
    }
    for (const char* type : { "Triangle", "Square", "Square 2", "6-Fold", "8-Fold", "12-Fold", "Radial" }) {
        INFO("type " << type);
        Frames fr = f.Render("Kaleidoscope", std::string("E_CHOICE_Kaleidoscope_Type=") + type + ",E_SLIDER_Kaleidoscope_Size=4", kRed, o);
        int moved = 0;
        for (int y = 0; y < 12; ++y) {
            for (int x = 0; x < 12; ++x) {
                const xlColor& c = fr.at(1, x, y);
                CHECK(source.count(((uint32_t)c.red << 16) | ((uint32_t)c.green << 8) | c.blue) == 1);
                moved += SameRgba(c, PatternColor(x, y)) ? 0 : 1;
            }
        }
        CHECK(moved > 0);
        // static geometry: both frames are the same mapping of the same source
        CHECK((fr.f[0] == fr.f[1]));
    }
}

TEST_CASE("Kaleidoscope rotation and centre change the mapping", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 14;
    o.h = 14;
    o.periods = { 0, 10, 19 };
    o.prefill = FillPattern;
    for (const char* type : { "Triangle", "Square", "Square 2", "6-Fold", "8-Fold", "12-Fold", "Radial" }) {
        INFO("type " << type);
        const std::string t = std::string("E_CHOICE_Kaleidoscope_Type=") + type + ",E_SLIDER_Kaleidoscope_Size=5";
        Frames base = f.Render("Kaleidoscope", t, kRed, o);
        Frames rot = f.Render("Kaleidoscope", t + ",E_VALUECURVE_Kaleidoscope_Rotation=" + Ramp("Kaleidoscope_Rotation", 0, 359, 0, 300), kRed, o);
        CHECK_FALSE(base == rot);
        // a rotating mapping changes from frame to frame
        CHECK_FALSE((rot.f[0] == rot.f[2]));
        Frames moved = f.Render("Kaleidoscope", t + ",E_SLIDER_Kaleidoscope_X=30,E_SLIDER_Kaleidoscope_Y=70", kRed, o);
        CHECK_FALSE(base == moved);
        for (int size : { 2, 8, 14, 30 }) {
            Frames sz = f.Render("Kaleidoscope", std::string("E_CHOICE_Kaleidoscope_Type=") + type + ",E_SLIDER_Kaleidoscope_Size=" + std::to_string(size), kRed, o);
            CHECK(sz.totalLit() > 0);
        }
    }
}

namespace {
struct KaleidoProbe : KaleidoscopeEffect {
    KaleidoProbe() : KaleidoscopeEffect(0) {}
    using KaleidoscopeEffect::ComputeTriangle;
    using KaleidoscopeEffect::MapToSourceNewSquare;
    using KaleidoscopeEffect::MapToSourceTriangle;
    using KaleidoscopeEffect::ReflectCoord;
    using KaleidoscopeEffect::SignedDist;
};
} // namespace

TEST_CASE("Kaleidoscope reflection helpers fold points into the source shape", "[effects][classic2]") {
    // triangle wave of period 4 * halfSize
    CHECK(KaleidoProbe::ReflectCoord(1.0, 3.0) == 1.0);
    CHECK(KaleidoProbe::ReflectCoord(4.0, 3.0) == 2.0);
    CHECK(KaleidoProbe::ReflectCoord(-4.0, 3.0) == -2.0);
    CHECK(KaleidoProbe::ReflectCoord(13.0, 3.0) == 1.0);

    auto p = KaleidoProbe::MapToSourceNewSquare(5, 5, 5, 5, 3, 0);
    CHECK(p == std::make_pair(5, 5));
    p = KaleidoProbe::MapToSourceNewSquare(9, 5, 5, 5, 3, 0);
    CHECK(p == std::make_pair(7, 5));
    p = KaleidoProbe::MapToSourceNewSquare(5, 0, 5, 5, 3, 0);
    CHECK(p == std::make_pair(5, 4));

    for (const char* type : { "6-Fold", "8-Fold", "12-Fold" }) {
        INFO(type);
        for (double rot : { 0.0, 1.0, 2.5 }) {
            KaleidoscopeTriangle tri = KaleidoProbe::ComputeTriangle(type, 10, 10, 8, rot);
            // counter clockwise order
            CHECK(KaleidoProbe::SignedDist(tri.v[2].x, tri.v[2].y, tri.v[0].x, tri.v[0].y, tri.v[1].x, tri.v[1].y) > 0);
            // the centre is inside and maps to itself
            CHECK(KaleidoProbe::MapToSourceTriangle(10, 10, tri, 50) == std::make_pair(10, 10));
            // a far point is reflected back inside the triangle
            auto q = KaleidoProbe::MapToSourceTriangle(30, 2, tri, 200);
            CHECK(std::hypot(q.first - 10.0, q.second - 10.0) <= 8.0);
            // with no iterations allowed an outside point does not converge
            CHECK(KaleidoProbe::MapToSourceTriangle(30, 2, tri, 0) == std::make_pair(-1, -1));
        }
    }
}

TEST_CASE("Kaleidoscope on a DMX fixture fills from the seed region", "[effects][classic2]") {
    Fx d(kMovingHead);
    Opts o;
    o.w = 12;
    o.h = 1;
    o.periods = { 0, 1 };
    o.prefill = FillPattern;
    for (const char* type : { "Triangle", "Square", "Square 2", "Radial" }) {
        INFO("type " << type);
        Frames fr = d.Render("Kaleidoscope", std::string("E_CHOICE_Kaleidoscope_Type=") + type + ",E_SLIDER_Kaleidoscope_Size=40", kRed, o);
        CHECK(fr.f.size() == 2);
    }
}

TEST_CASE("Kaleidoscope settings check warns without canvas", "[effects][classic2]") {
    Fx f;
    RenderableEffect* k = f.Get("Kaleidoscope");
    Effect* e = f.layer->AddEffect(0, "Kaleidoscope", "", kRed, 0, 1000, 0, false);
    SettingsMap s;
    CHECK(k->CheckEffectSettings(s, nullptr, f.model, e, false).size() == 1);
    s["T_CHECKBOX_Canvas"] = "1";
    CHECK(k->CheckEffectSettings(s, nullptr, f.model, e, false).empty());
}

// ---------------------------------------------------------------------------
// Warp
// ---------------------------------------------------------------------------

TEST_CASE("Warp mirror, flip and copy remap pixels exactly", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 9;
    o.h = 7;
    o.prefill = FillPattern;
    // X=50 puts the mirror line at column 4; Y=100 disables the vertical mirror
    Frames mirror = f.Render("Warp", "E_CHOICE_Warp_Type=mirror,E_SLIDER_Warp_X=50,E_SLIDER_Warp_Y=100", kRed, o);
    for (int y = 0; y < 7; ++y) {
        for (int x = 0; x < 9; ++x) {
            CHECK(SameRgba(mirror.at(0, x, y), PatternColor(x <= 4 ? x : 8 - x, y)));
        }
    }
    // X=100 and Y=0 flip horizontally only
    Frames flip = f.Render("Warp", "E_CHOICE_Warp_Type=flip,E_SLIDER_Warp_X=100,E_SLIDER_Warp_Y=0", kRed, o);
    for (int y = 0; y < 7; ++y) {
        for (int x = 0; x < 9; ++x) CHECK(SameRgba(flip.at(0, x, y), PatternColor(8 - x, y)));
    }
    Frames copy = f.Render("Warp", "E_CHOICE_Warp_Type=copy,E_SLIDER_Warp_X=50,E_SLIDER_Warp_Y=100", kRed, o);
    for (int y = 0; y < 7; ++y) {
        for (int x = 0; x < 9; ++x) CHECK(SameRgba(copy.at(0, x, y), PatternColor(x > 4 ? x - 4 : x, y)));
    }
}

TEST_CASE("Warp sample on paints the sampled pixel everywhere", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 9;
    o.h = 7;
    o.prefill = FillPattern;
    Frames fr = f.Render("Warp", "E_CHOICE_Warp_Type=sample on,E_SLIDER_Warp_X=25,E_SLIDER_Warp_Y=50", kRed, o);
    // x = 0.25 * 8 = 2, y = 0.5 * 6 = 3
    for (const auto& c : fr.f[0]) CHECK(SameRgba(c, PatternColor(2, 3)));
}

TEST_CASE("Warp speed replays the canvas in slow motion", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 3;
    o.h = 2;
    o.periods = Range(0, 9);
    o.prefill = [](int frame, int w, int h, xlColor* px) {
        for (int i = 0; i < w * h; ++i) px[i] = xlColor((uint8_t)(frame * 10), 0, 0, 255);
    };
    Frames half = f.Render("Warp", "E_CHOICE_Warp_Type=speed,E_SLIDER_Warp_X=50", kRed, o);
    for (int i = 0; i < 10; ++i) {
        INFO("frame " << i);
        CHECK((int)half.at(i, 1, 1).red == (i / 2) * 10);
    }
    Frames normal = f.Render("Warp", "E_CHOICE_Warp_Type=speed,E_SLIDER_Warp_X=100", kRed, o);
    for (int i = 0; i < 10; ++i) CHECK((int)normal.at(i, 0, 0).red == i * 10);
}

TEST_CASE("Warp distortions keep the canvas colours and vary with treatment", "[effects][classic2]") {
    Fx f;
    Opts o;
    o.w = 12;
    o.h = 10;
    o.periods = { 3, 9, 15 };
    o.prefill = FillPattern;
    for (const char* type : { "water drops", "dissolve", "circle reveal", "banded swirl", "ripple", "circular swirl", "drop", "wavy" }) {
        INFO("type " << type);
        const std::string t = std::string("E_CHOICE_Warp_Type=") + type;
        Frames c = f.Render("Warp", t + ",E_CHOICE_Warp_Treatment_APPLYLAST=constant,E_TEXTCTRL_Warp_Cycle_Count=2", kRed, o);
        Frames in = f.Render("Warp", t + ",E_CHOICE_Warp_Treatment_APPLYLAST=in", kRed, o);
        Frames out = f.Render("Warp", t + ",E_CHOICE_Warp_Treatment_APPLYLAST=out", kRed, o);
        CHECK(c.totalLit() > 0);
        CHECK(in.totalLit() > 0);
        // a distortion is not the identity for every frame
        bool changed = false;
        for (size_t i = 0; i < c.f.size(); ++i) {
            for (int y = 0; y < o.h; ++y) {
                for (int x = 0; x < o.w; ++x) changed |= !SameRgba(c.at(i, x, y), PatternColor(x, y));
            }
        }
        CHECK(changed);
        CHECK(out.f.size() == 3);
    }
    // a single water drop only ripples a ring, so look over the whole effect
    o.w = 20;
    o.h = 20;
    o.periods = Range(0, 19);
    Frames drop = f.Render("Warp", "E_CHOICE_Warp_Type=single water drop,E_TEXTCTRL_Warp_Cycle_Count=2", kRed, o);
    bool changed = false;
    for (size_t i = 0; i < drop.f.size(); ++i) {
        for (int y = 0; y < o.h; ++y) {
            for (int x = 0; x < o.w; ++x) changed |= !SameRgba(drop.at(i, x, y), PatternColor(x, y));
        }
    }
    CHECK(changed);
}

TEST_CASE("Warp frame parallelism and settings check", "[effects][classic2]") {
    Fx f;
    RenderableEffect* w = f.Get("Warp");
    SettingsMap s;
    s["CHOICE_Warp_Type"] = "speed";
    CHECK(w->GetFrameParallelism(s) == RenderableEffect::FrameParallelism::Stateful);
    s["CHOICE_Warp_Type"] = "sample on";
    CHECK(w->GetFrameParallelism(s) == RenderableEffect::FrameParallelism::Stateful);
    s["CHOICE_Warp_Type"] = "ripple";
    CHECK(w->GetFrameParallelism(s) == RenderableEffect::FrameParallelism::Pure);

    Effect* e = f.layer->AddEffect(0, "Warp", "", kRed, 0, 1000, 0, false);
    SettingsMap c;
    CHECK(w->CheckEffectSettings(c, nullptr, f.model, e, false).size() == 1);
    c["T_CHECKBOX_Canvas"] = "1";
    CHECK(w->CheckEffectSettings(c, nullptr, f.model, e, false).empty());
}

// ---------------------------------------------------------------------------
// Twinkle (through the engine, which supplies the node layout the effect needs)
// ---------------------------------------------------------------------------

namespace {
renderhelp::Rendered RenderTwinkle(const std::string& modelXml, const std::string& settings, const std::string& palette = kRedBlue) {
    renderhelp::ShowSpec spec;
    spec.models = modelXml;
    spec.effects = { renderhelp::Eff("M", 0, "Twinkle", 0, 1000, palette, settings) };
    return renderhelp::Render(spec);
}
int LitNodes(const renderhelp::Rendered& r, unsigned frame) {
    int n = 0;
    for (unsigned c = 0; c + 2 < r.channels; c += 3) {
        n += (r.at(frame, c) || r.at(frame, c + 1) || r.at(frame, c + 2)) ? 1 : 0;
    }
    return n;
}
bool AnyGreen(const renderhelp::Rendered& r) {
    for (unsigned f = 0; f < r.frames; ++f) {
        for (unsigned c = 1; c < r.channels; c += 3) {
            if (r.at(f, c)) return true;
        }
    }
    return false;
}
const std::string kTwinkleMatrix = renderhelp::Matrix("M", 1, 10, 10);
// 10x10 custom model with three nodes: under 20% of the buffer has lights, so Twinkle works by node.
const std::string kTwinkleSparse = "<model name=\"M\" DisplayAs=\"Custom\" StartChannel=\"1\" StringType=\"RGB Nodes\" CustomWidth=\"10\" CustomHeight=\"10\" "
                                   "CustomModel=\"1,,,,,,,,,;,,,,,,,,,;,,,,,,,,,;,,,,,,,,,;,,,,2,,,,,;,,,,,,,,,;,,,,,,,,,;,,,,,,,,,;,,,,,,,,,;,,,,,,,,,3\"/>";
} // namespace

TEST_CASE("Twinkle per pixel lights a share of the nodes from the palette", "[effects][classic2]") {
    for (const char* style : { "New Render Method", "Old Render Method" }) {
        INFO(style);
        auto r = RenderTwinkle(kTwinkleMatrix, std::string("E_CHOICE_Twinkle_Style=") + style + ",E_SLIDER_Twinkle_Count=40,E_SLIDER_Twinkle_Steps=6");
        REQUIRE(r.rendered);
        REQUIRE(r.channels == 300);
        int total = 0;
        for (unsigned fr = 0; fr < r.frames; ++fr) {
            int n = LitNodes(r, fr);
            CHECK(n < 100);
            total += n;
        }
        CHECK(total > 0);
        CHECK_FALSE(AnyGreen(r));
        auto again = RenderTwinkle(kTwinkleMatrix, std::string("E_CHOICE_Twinkle_Style=") + style + ",E_SLIDER_Twinkle_Count=40,E_SLIDER_Twinkle_Steps=6");
        const bool same = r.data == again.data;
        CHECK(same);
    }
}

TEST_CASE("Twinkle strobe and re-randomise options", "[effects][classic2]") {
    auto plain = RenderTwinkle(kTwinkleMatrix, "E_SLIDER_Twinkle_Count=30,E_SLIDER_Twinkle_Steps=8");
    for (const char* s : { "E_SLIDER_Twinkle_Count=30,E_SLIDER_Twinkle_Steps=8,E_CHECKBOX_Twinkle_Strobe=1",
                           "E_SLIDER_Twinkle_Count=30,E_SLIDER_Twinkle_Steps=8,E_CHECKBOX_Twinkle_ReRandom=1",
                           "E_CHOICE_Twinkle_Style=Old Render Method,E_SLIDER_Twinkle_Count=30,E_SLIDER_Twinkle_Steps=8,E_CHECKBOX_Twinkle_Strobe=1,E_CHECKBOX_Twinkle_ReRandom=1" }) {
        INFO(s);
        auto r = RenderTwinkle(kTwinkleMatrix, s);
        REQUIRE(r.rendered);
        int total = 0;
        for (unsigned fr = 0; fr < r.frames; ++fr) total += LitNodes(r, fr);
        CHECK(total > 0);
        const bool same = r.data == plain.data;
        CHECK_FALSE(same);
        CHECK_FALSE(AnyGreen(r));
    }
    // a count value curve changes how many lights twinkle over time
    auto vc = RenderTwinkle(kTwinkleMatrix, "E_SLIDER_Twinkle_Steps=4,E_VALUECURVE_Twinkle_Count=" + Ramp("Twinkle_Count", 2, 100, 2, 100));
    int early = 0, late = 0;
    for (unsigned fr = 0; fr < 5; ++fr) early += LitNodes(vc, fr);
    for (unsigned fr = vc.frames - 5; fr < vc.frames; ++fr) late += LitNodes(vc, fr);
    CHECK(late > early);
}

TEST_CASE("Twinkle by node on a sparse model", "[effects][classic2]") {
    for (const char* s : { "E_CHOICE_Twinkle_Style=New Render Method,E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4",
                           "E_CHOICE_Twinkle_Style=New Render Method,E_SLIDER_Twinkle_Count=50,E_SLIDER_Twinkle_Steps=6,E_CHECKBOX_Twinkle_ReRandom=1,E_CHECKBOX_Twinkle_Strobe=1",
                           "E_CHOICE_Twinkle_Style=Old Render Method,E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4",
                           "E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4,E_CHECKBOX_Twinkle_Strobe=1",
                           "E_CHOICE_Twinkle_Style=Old Render Method,E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4,E_CHECKBOX_Twinkle_ReRandom=1",
                           "E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4,E_CHECKBOX_Twinkle_ReRandom=1,E_CHECKBOX_Twinkle_Strobe=1" }) {
        INFO(s);
        auto r = RenderTwinkle(kTwinkleSparse, s);
        REQUIRE(r.rendered);
        REQUIRE(r.channels > 0);
        REQUIRE(r.channels <= 12);
        int total = 0;
        for (unsigned fr = 0; fr < r.frames; ++fr) total += LitNodes(r, fr);
        CHECK(total > 0);
        CHECK_FALSE(AnyGreen(r));
    }
}

TEST_CASE("Twinkle on a large matrix and on a DMX fixture", "[effects][classic2]") {
    // enough lights to split the kernel into parallel blocks
    auto big = RenderTwinkle(renderhelp::Matrix("M", 1, 70, 70), "E_CHOICE_Twinkle_Style=New Render Method,E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4");
    REQUIRE(big.rendered);
    int total = 0;
    for (unsigned fr = 0; fr < big.frames; ++fr) total += LitNodes(big, fr);
    CHECK(total > 0);
    CHECK_FALSE(AnyGreen(big));

    std::string head = kMovingHead;
    head.replace(head.find("name=\"MH\""), 9, "name=\"M\"");
    auto dmx = RenderTwinkle(head, "E_CHOICE_Twinkle_Style=New Render Method,E_SLIDER_Twinkle_Count=100,E_SLIDER_Twinkle_Steps=4", kRed);
    REQUIRE(dmx.rendered);
    REQUIRE(dmx.channels >= 12);
    for (unsigned fr = 0; fr < dmx.frames; ++fr) {
        // green and blue channels of a red twinkle stay dark
        CHECK(dmx.at(fr, 9) == 0);
        CHECK(dmx.at(fr, 10) == 0);
    }
}

TEST_CASE("Twinkle background shows the brightness ramp", "[effects][classic2]") {
    Fx f;
    RenderableEffect* t = f.Get("Twinkle");
    Effect* e = f.layer->AddEffect(0, "Twinkle", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_SLIDER_Brightness=80", 0, 1000, 0, false);
    RecordingCVA flat;
    CHECK(t->DrawEffectBackground(e, 0, 0, 100, 10, flat, nullptr, true) == 2);
    CHECK(!flat.v.empty());
    RecordingCVA none;
    CHECK(t->DrawEffectBackground(e, 0, 0, 100, 10, none, nullptr, false) == 1);
    CHECK(none.v.empty());

    Effect* vc = f.layer->AddEffect(0, "Twinkle", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_VALUECURVE_Brightness=" + Ramp("Brightness", 0, 400, 20, 100), 0, 1000, 0, false);
    RecordingCVA ramp;
    CHECK(t->DrawEffectBackground(vc, 0, 0, 95, 10, ramp, nullptr, true) == 2);
    CHECK(ramp.v.size() > 0);
}

// ---------------------------------------------------------------------------
// RenderableEffect value-curve helpers and other shared infrastructure
// ---------------------------------------------------------------------------

namespace {
class VCProbe : public RenderableEffect {
public:
    VCProbe() : RenderableEffect(0, "VCProbe", nullptr, nullptr, nullptr, nullptr, nullptr) {}
    void Render(Effect*, const SettingsMap&, RenderBuffer&) override {}
    using RenderableEffect::GetValueCurveDouble;
    using RenderableEffect::GetValueCurveInt;
    using RenderableEffect::GetValueCurveIntMax;
    using RenderableEffect::GetSettingVCMin;
    using RenderableEffect::GetSettingVCMax;
    using RenderableEffect::GetSettingVCDivisor;
    using RenderableEffect::ResolveFileReference;
    using RenderableEffect::GetTimingTracks;
    using RenderableEffect::GetCurrentTiming;
};
} // namespace

TEST_CASE("GetValueCurveInt reads slider, text control and default values", "[effects][classic2][vc]") {
    VCProbe p;
    SettingsMap s;
    s["SLIDER_A"] = "12";
    s["TEXTCTRL_B"] = "34";
    s["SLIDER_E"] = "";
    CHECK(p.GetValueCurveInt("A", 1, s, 0.5f, 0, 100, 0, 1000) == 12);
    CHECK(p.GetValueCurveInt("B", 1, s, 0.5f, 0, 100, 0, 1000) == 34);
    CHECK(p.GetValueCurveInt("C", 7, s, 0.5f, 0, 100, 0, 1000) == 7);
    CHECK(p.GetValueCurveInt("E", 9, s, 0.5f, 0, 100, 0, 1000) == 9);
    // repeated calls hit the cached answer
    for (int i = 0; i < 3; ++i) CHECK(p.GetValueCurveInt("A", 1, s, 0.1f * i, 0, 100, 0, 1000) == 12);
    CHECK(p.GetValueCurveDouble("A", 1.0, s, 0.5f, 0, 100, 0, 1000) == 12.0);
    CHECK(p.GetValueCurveDouble("B", 1.0, s, 0.5f, 0, 100, 0, 1000) == 34.0);
    CHECK(p.GetValueCurveDouble("C", 2.5, s, 0.5f, 0, 100, 0, 1000) == 2.5);
}

TEST_CASE("GetValueCurveInt and Double follow an active ramp curve", "[effects][classic2][vc]") {
    VCProbe p;
    SettingsMap s;
    s["SLIDER_A"] = "5";
    s["VALUECURVE_A"] = Ramp("A", 0, 100, 0, 100);
    s["VALUECURVE_D"] = Ramp("D", 0, 100, 20, 80);
    s["SLIDER_I"] = "33";
    s["VALUECURVE_I"] = "Active=FALSE|Id=ID_VALUECURVE_I|Type=Ramp|Min=0.00|Max=100.00|P1=0.00|P2=100.00|RV=TRUE|";
    int a0 = p.GetValueCurveInt("A", 1, s, 0.0f, 0, 100, 0, 1000);
    int a5 = p.GetValueCurveInt("A", 1, s, 0.5f, 0, 100, 0, 1000);
    int a1 = p.GetValueCurveInt("A", 1, s, 1.0f, 0, 100, 0, 1000);
    CHECK(a0 == 0);
    CHECK(std::abs(a5 - 50) <= 1);
    CHECK(a1 == 100);
    double d0 = p.GetValueCurveDouble("D", 1.0, s, 0.0f, 0, 100, 0, 1000);
    double d1 = p.GetValueCurveDouble("D", 1.0, s, 1.0f, 0, 100, 0, 1000);
    CHECK(std::abs(d0 - 20.0) < 0.5);
    CHECK(std::abs(d1 - 80.0) < 0.5);
    // the divisor scales the curve output
    double dd = p.GetValueCurveDouble("D", 1.0, s, 1.0f, 0, 100, 0, 1000, 10);
    CHECK(std::abs(dd - 8.0) < 0.1);
    // an inactive curve falls back to the slider
    CHECK(p.GetValueCurveInt("I", 1, s, 0.7f, 0, 100, 0, 1000) == 33);
    CHECK(p.GetValueCurveDouble("I", 1.0, s, 0.7f, 0, 100, 0, 1000) == 33.0);
}

TEST_CASE("GetValueCurve caches are keyed on the limits", "[effects][classic2][vc]") {
    VCProbe p;
    SettingsMap s;
    s["SLIDER_A"] = "40";
    s["VALUECURVE_R"] = Ramp("R", 0, 100, 0, 100);
    CHECK(p.GetValueCurveInt("A", 1, s, 0.0f, 0, 100, 0, 1000) == 40);
    CHECK(p.GetValueCurveInt("A", 1, s, 0.0f, 0, 50, 0, 1000) == 40);
    int full = p.GetValueCurveInt("R", 1, s, 1.0f, 0, 100, 0, 1000);
    int narrow = p.GetValueCurveInt("R", 1, s, 1.0f, 0, 50, 0, 1000);
    CHECK(full == 100);
    CHECK(narrow != full);
    // more settings than the lock-free table holds still resolve correctly
    for (int i = 0; i < 24; ++i) s["SLIDER_S" + std::to_string(i)] = std::to_string(i * 2);
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < 24; ++i) {
            CHECK(p.GetValueCurveInt("S" + std::to_string(i), -1, s, 0.0f, 0, 100, 0, 1000) == i * 2);
            CHECK(p.GetValueCurveDouble("S" + std::to_string(i), -1, s, 0.0f, 0, 100, 0, 1000) == i * 2.0);
        }
    }
}

TEST_CASE("GetValueCurveIntMax returns the curve maximum or the slider", "[effects][classic2][vc]") {
    VCProbe p;
    SettingsMap s;
    s["E_SLIDER_A"] = "17";
    s["E_TEXTCTRL_B"] = "21";
    s["E_VALUECURVE_C"] = Ramp("C", 0, 100, 10, 60);
    s["E_SLIDER_C"] = "3";
    s["E_VALUECURVE_D"] = "Active=FALSE|Id=ID_VALUECURVE_D|Type=Ramp|Min=0.00|Max=100.00|P1=10.00|P2=60.00|RV=TRUE|";
    s["E_SLIDER_D"] = "4";
    for (int pass = 0; pass < 2; ++pass) {
        CHECK(p.GetValueCurveIntMax("A", 0, s, 0, 100) == 17);
        CHECK(p.GetValueCurveIntMax("B", 0, s, 0, 100) == 21);
        CHECK(p.GetValueCurveIntMax("C", 0, s, 0, 100) == 60);
        CHECK(p.GetValueCurveIntMax("D", 0, s, 0, 100) == 4);
        CHECK(p.GetValueCurveIntMax("Z", 8, s, 0, 100) == 8);
    }
}

TEST_CASE("UpgradeValueCurve rescales curves to the current limits", "[effects][classic2][vc]") {
    HeadlessRenderContext ctx;
    EffectManager& em = ctx.effectManager;
    // inactive curves and unknown effects come back unchanged
    const std::string inactive = "Active=FALSE|Id=ID_VALUECURVE_X|Type=Ramp|Min=0.00|Max=100.00|P1=10.00|P2=60.00|";
    CHECK(RenderableEffect::UpgradeValueCurve(&em, "E_VALUECURVE_On_Transparency", inactive, "On") == inactive);
    const std::string active = "Active=TRUE|Id=ID_VALUECURVE_X|Type=Ramp|Min=0.00|Max=100.00|P1=10.00|P2=60.00|RV=TRUE|";
    CHECK(RenderableEffect::UpgradeValueCurve(&em, "E_VALUECURVE_X", active, "NoSuchEffect") == active);
    // an unprefixed name is left alone
    CHECK(RenderableEffect::UpgradeValueCurve(&em, "VALUECURVE_X", active, "On") == active);

    if (em.GetEffect("On")->HasMetadata()) {
        std::string up = RenderableEffect::UpgradeValueCurve(&em, "E_VALUECURVE_On_Transparency", active, "On");
        ValueCurve vc(up);
        CHECK(vc.IsActive());
        CHECK(std::abs(vc.GetMax() - 100.0) < 0.01);
    }
    std::string c = RenderableEffect::UpgradeValueCurve(&em, "C_VALUECURVE_Brightness", "Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Ramp|Min=0.00|Max=400.00|P1=0.00|P2=50.00|", "On");
    CHECK(ValueCurve(c).IsActive());
    CHECK(c.find("RV=TRUE") != std::string::npos);
    std::string t = RenderableEffect::UpgradeValueCurve(&em, "T_VALUECURVE_In_Transition_Adjust", "Active=TRUE|Id=ID_VALUECURVE_In_Transition_Adjust|Type=Ramp|Min=0.00|Max=100.00|P1=0.00|P2=50.00|", "On");
    CHECK(ValueCurve(t).IsActive());
    std::string b = RenderableEffect::UpgradeValueCurve(&em, "B_VALUECURVE_Blur", "Active=TRUE|Id=ID_VALUECURVE_Blur|Type=Ramp|Min=1.00|Max=15.00|P1=0.00|P2=50.00|", "On");
    CHECK(ValueCurve(b).IsActive());
}

TEST_CASE("RenderableEffect setting VC limits come from metadata", "[effects][classic2][vc]") {
    VCProbe p;
    p.SetMetadata(nlohmann::json::parse(R"({"properties":[
        {"id":"A","min":1,"max":10,"divisor":100,"vcMin":2,"vcMax":8},
        {"id":"B","min":-5,"max":5}
    ]})"));
    CHECK(p.GetSettingVCMin("E_VALUECURVE_A") == 2.0);
    CHECK(p.GetSettingVCMax("E_VALUECURVE_A") == 8.0);
    CHECK(p.GetSettingVCDivisor("E_VALUECURVE_A") == 100);
    CHECK(p.GetSettingVCMin("E_VALUECURVE_B") == -5.0);
    CHECK(p.GetSettingVCMax("E_VALUECURVE_B") == 5.0);
    CHECK(p.GetSettingVCDivisor("E_VALUECURVE_B") == 1);
    CHECK(p.GetSettingVCDivisor("E_VALUECURVE_Missing") == 1);
    CHECK(p.GetSettingVCDivisor("E_VALUE") == 1);
}

TEST_CASE("RenderableEffect render cache and frame parallelism rules", "[effects][classic2]") {
    VCProbe p;
    SettingsMap s;
    CHECK_FALSE(p.SupportsRenderCache(s));
    s["B_SLIDER_Blur"] = "2";
    CHECK(p.SupportsRenderCache(s));
    SettingsMap r;
    r["B_VALUECURVE_Rotations"] = "x";
    CHECK(p.SupportsRenderCache(r));

    Fx f;
    RenderableEffect* on = f.Get("On");
    CHECK(on->GetEffectiveFrameParallelism(SettingsMap()) == on->GetFrameParallelism(SettingsMap()));
    for (const char* k : { "CHECKBOX_OverlayBkg=1", "CHECKBOX_Canvas=1", "SPINCTRL_FreezeEffectAtFrame=5", "SPINCTRL_SuppressEffectUntil=3" }) {
        INFO(k);
        SettingsMap m;
        std::string kv(k);
        m[kv.substr(0, kv.find('='))] = kv.substr(kv.find('=') + 1);
        CHECK(on->GetEffectiveFrameParallelism(m) == RenderableEffect::FrameParallelism::Stateful);
    }
    // Curtain open/close is pure, the two way variants carry state
    SettingsMap oc;
    oc["CHOICE_Curtain_Effect"] = "open then close";
    CHECK(f.Get("Curtain")->GetFrameParallelism(oc) == RenderableEffect::FrameParallelism::Stateful);
    oc["CHOICE_Curtain_Effect"] = "close";
    CHECK(f.Get("Curtain")->GetFrameParallelism(oc) == RenderableEffect::FrameParallelism::Pure);
}

TEST_CASE("RenderableEffect timing helpers find tracks and the current mark", "[effects][classic2]") {
    Fx f;
    f.AddTiming("Beats", { { 0, 100 }, { 100, 300 } });
    Element* twoLayer = f.ctx._sequenceElements.AddElement("Lyrics", "timing", true, false, true, false, false);
    if (twoLayer->GetEffectLayerCount() == 0) twoLayer->AddEffectLayer();
    twoLayer->AddEffectLayer();

    VCProbe p;
    p.SetSequenceElements(&f.ctx._sequenceElements);
    CHECK(p.GetTimingTracks() == "Beats|Lyrics");
    CHECK(p.GetTimingTracks(1) == "Beats");
    CHECK(p.GetTimingTracks(0, 2) == "Lyrics");

    RenderBuffer rb(&f.ctx, nullptr, f.model);
    rb.InitBuffer(1, 1, "None");
    rb.SetFrameTimeInMs(50);
    rb.SetState(3, true); // 150ms
    Effect* mark = p.GetCurrentTiming(rb, "Beats");
    REQUIRE(mark != nullptr);
    CHECK(mark->GetStartTimeMS() == 100);
    rb.SetState(10, false); // 500ms: past every mark
    CHECK(p.GetCurrentTiming(rb, "Beats") == nullptr);
    CHECK(p.GetCurrentTiming(rb, "Missing") == nullptr);
    CHECK(p.GetCurrentTiming(rb, "") == nullptr);

    CHECK(VCProbe::ResolveFileReference(nullptr, "").empty());
    TempDir dir;
    const std::string img = dir.Write("img.png", "x");
    CHECK(VCProbe::ResolveFileReference(&f.ctx, img) == img);

    VCProbe lonely;
    CHECK(lonely.GetTimingTracks().empty());
}

TEST_CASE("RenderableEffect base background uses the display list", "[effects][classic2]") {
    Effect::EnableBackgroundDisplayLists(true);
    Fx f;
    VCProbe p;
    Effect* e = f.layer->AddEffect(0, "On", "", kRed, 0, 1000, 0, false);
    RecordingCVA acc;
    CHECK(p.DrawEffectBackground(e, 0, 0, 100, 10, acc, nullptr, false) == 1);
    CHECK(acc.v.empty());
    // once a render has filled the display list it is drawn instead
    Opts o;
    o.w = 2;
    o.h = 2;
    o.periods = { 0, 1 };
    f.Render("On", "E_CHECKBOX_On_Shimmer=1", kRedBlue, o);
    REQUIRE(f.last->HasBackgroundDisplayList());
    RecordingCVA filled;
    CHECK(p.DrawEffectBackground(f.last, 0, 0, 100, 10, filled, nullptr, false) == f.last->GetBackgroundDisplayList().iconSize);
    CHECK(!filled.v.empty());
}

TEST_CASE("RenderableEffect metadata fallbacks for odd property shapes", "[effects][classic2]") {
    VCProbe p;
    CHECK(p.GetMetadata().is_null());
    p.SetMetadata(nlohmann::json::parse(R"({"properties":[
        {"id":"S","default":"text","min":"low","max":"high"},
        {"id":"N","vcMax":7},
        {"id":"X"}
    ]})"));
    CHECK(p.GetMetadata().contains("properties"));
    CHECK(p.GetDoubleDefault("S", 1.5) == 1.5);
    CHECK(p.GetFloatDefault("S", 2.5f) == 2.5f);
    CHECK(p.GetVCMinFromMetadata("S", -1) == -1.0);
    CHECK(p.GetVCMaxFromMetadata("S", -1) == -1.0);
    CHECK(p.GetVCMaxFromMetadata("N", -1) == 7.0);
    CHECK(p.GetVCMaxFromMetadata("X", 3) == 3.0);
    CHECK(p.GetVCMaxFromMetadata("Missing", 4) == 4.0);
}

TEST_CASE("Shockwave falls back to the effect's own sequence elements without a render context", "[effects][classic2]") {
    Fx f;
    f.AddTiming("Beats", { { 250, 300 } });
    f.Get("Shockwave")->SetSequenceElements(&f.ctx._sequenceElements);
    Opts o;
    o.w = 11;
    o.h = 11;
    o.periods = { 2, 6 };
    o.noContext = true;
    Frames fr = f.Render("Shockwave", "E_CHOICE_Shockwave_TimingTrack=Beats,E_SLIDER_Shockwave_Duration=200,E_SLIDER_Shockwave_End_Radius=60", kRed, o);
    CHECK(fr.lit(0) == 0);
    CHECK(fr.lit(1) > 0);
}

// ---------------------------------------------------------------------------
// EffectPresetManager
// ---------------------------------------------------------------------------

namespace {
const char* kPresetXml = R"(<effects version="0007">
  <effect name="Top" settings="CopyFormat1&#x9;x&#x9;0&#x9;100&#x9;300&#x9;2&#x9;r" version="0007" xLightsVersion="2024.01"/>
  <effect name="Skipped" settings="no tabs here"/>
  <effectGroup name="Group">
    <effect name="Inner" settings="On&#x9;a&#x9;b&#x9;0&#x9;500" version="0007"/>
    <effectGroup name="Deeper">
      <effect name="Leaf" settings="On&#x9;a&#x9;b&#x9;10&#x9;20"/>
      <effect name="NoTabs" settings="plain"/>
    </effectGroup>
  </effectGroup>
</effects>)";
} // namespace

TEST_CASE("EffectPresetManager loads presets and groups from XML", "[effects][classic2][presets]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(kPresetXml));
    EffectPresetManager m;
    m.Load(doc.child("effects"));
    CHECK(m.GetVersion() == "0007");
    auto paths = m.GetAllPresetPaths();
    REQUIRE(paths.size() == 3);
    CHECK(paths[0] == "Top");
    CHECK(paths[1] == "Group\\Inner");
    CHECK(paths[2] == "Group\\Deeper\\Leaf");
    CHECK(m.GetAllPresetPaths("/")[2] == "Group/Deeper/Leaf");

    EffectPreset* leaf = m.FindPresetByPath("Group\\Deeper\\Leaf");
    REQUIRE(leaf != nullptr);
    CHECK(leaf->GetXLightsVersion() == "4.0");
    CHECK(leaf->GetParent()->GetName() == "Deeper");
    CHECK(m.FindPresetByPath("Group/Deeper/Leaf", '/') == leaf);
    CHECK(m.FindPresetByPath("Group") == nullptr);
    CHECK(m.FindItemByPath("Group")->IsGroup());
    CHECK(m.FindItemByPath("Top\\Child") == nullptr);
    CHECK(m.FindItemByPath("Nope") == nullptr);

    EffectPreset* top = m.FindPresetByPath("Top");
    REQUIRE(top != nullptr);
    CHECK(top->GetVersion() == "0007");
    CHECK(top->GetXLightsVersion() == "2024.01");

    m.Load(pugi::xml_node());
    CHECK(m.GetAllPresetPaths().empty());
    CHECK(m.GetVersion().empty());

    pugi::xml_document noVersion;
    noVersion.load_string("<effects/>");
    m.Load(noVersion.child("effects"));
    CHECK(m.GetVersion() == "0000");
}

TEST_CASE("EffectPreset counts layers and duration for each copy format", "[effects][classic2][presets]") {
    // CopyFormat1 rows: column 5 is the row; start/end are columns 3/4
    EffectPreset cf1("a", "CopyFormat1\tx\nOn\tp\ts\t100\t400\t2\nOn\tp\ts\t50\t200\t4\nNone\tp\ts\t0\t9999\t9", "", "");
    CHECK(cf1.GetLayerCount() == 3);
    CHECK(cf1.GetDurationMS() == 350);

    // CopyFormatAC: start/end rows at 7/8 and times at 9/10
    EffectPreset ac("b", "CopyFormatAC\t0\t0\t0\t0\t0\t0\t2\t5\t100\t900", "", "");
    CHECK(ac.GetLayerCount() == 4);
    CHECK(ac.GetDurationMS() == 800);

    // legacy tab format without a CopyFormat header uses the second last column as the row
    EffectPreset legacy("c", "On\ta\tb\t0\t300\t7\tz\nOn\ta\tb\t100\t500\t9\tz", "", "");
    CHECK(legacy.GetLayerCount() == 3);
    CHECK(legacy.GetDurationMS() == 500);

    // comma format: effect1,effect2,blend,start,end
    EffectPreset comma("d", "On,Bars,Normal,100,600", "", "");
    CHECK(comma.GetLayerCount() == 2);
    CHECK(comma.GetDurationMS() == 500);
    EffectPreset half("e", "None,Bars,Normal", "", "");
    CHECK(half.GetLayerCount() == 1);
    CHECK(half.GetDurationMS() == 0);
    EffectPreset single("f", "On", "", "");
    CHECK(single.GetLayerCount() == 0);

    EffectPreset empty("g", "", "", "");
    CHECK(empty.GetLayerCount() == 0);
    CHECK(empty.GetDurationMS() == 0);
    EffectPreset nothing("h", "CopyFormat1\tx", "", "");
    CHECK(nothing.GetLayerCount() == 0);
    CHECK(nothing.GetDurationMS() == 0);
}

TEST_CASE("EffectPresetManager saves XML and JSON that load back the same tree", "[effects][classic2][presets]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(kPresetXml));
    EffectPresetManager m;
    m.Load(doc.child("effects"));

    pugi::xml_document out;
    {
        XmlSerializingVisitor v(&out);
        m.Save(v);
    }
    EffectPresetManager back;
    back.Load(out.child("effects"));
    CHECK(back.GetAllPresetPaths() == m.GetAllPresetPaths());
    CHECK(back.GetVersion() == "0007");
    CHECK(back.FindPresetByPath("Group\\Inner")->GetSettings() == m.FindPresetByPath("Group\\Inner")->GetSettings());

    std::ostringstream os;
    out.save(os);
    CHECK(os.str().find("effectGroup") != std::string::npos);
    CHECK(os.str().find("Leaf") != std::string::npos);

    nlohmann::json j = m.SaveToJson();
    CHECK(j["version"] == "0007");
    EffectPresetManager fromJson;
    fromJson.LoadFromJson(j);
    CHECK(fromJson.GetAllPresetPaths() == m.GetAllPresetPaths());
    CHECK(fromJson.FindPresetByPath("Top")->GetXLightsVersion() == "2024.01");

    // JSON entries without tabs, of unknown type, or a non object are dropped
    nlohmann::json odd = nlohmann::json::parse(R"({"children":[{"type":"effect","name":"x","settings":"none"},{"type":"weird"},
        {"type":"group","name":"g","children":[{"type":"effect","name":"y","settings":"a\tb"}]},{"type":"group","name":"empty"}]})");
    fromJson.LoadFromJson(odd);
    CHECK(fromJson.GetVersion() == "0000");
    CHECK(fromJson.GetAllPresetPaths() == std::vector<std::string>{ "g\\y" });
    CHECK(fromJson.FindPresetByPath("g\\y")->GetXLightsVersion() == "4.0");
    fromJson.LoadFromJson(nlohmann::json::array());
    CHECK(fromJson.GetAllPresetPaths().empty());
}

TEST_CASE("EffectPresetManager JSON files round trip through disk", "[effects][classic2][presets]") {
    TempDir dir;
    EffectPresetManager m;
    m.SetVersion("0009");
    EffectPresetGroup* g = m.AddGroup(nullptr, "Mine");
    m.AddPreset(g, "P1", "On\ta\tb\t0\t100", "0009", "2026.01");
    m.AddPreset(nullptr, "Root", "Bars\ta\tb\t0\t50", "0009", "2026.01");
    const std::string path = dir.File("presets.json");
    REQUIRE(m.SaveJsonFile(path));

    EffectPresetManager back;
    REQUIRE(back.LoadJsonFile(path));
    CHECK(back.GetVersion() == "0009");
    CHECK(back.GetAllPresetPaths() == std::vector<std::string>{ "Mine\\P1", "Root" });

    CHECK_FALSE(back.LoadJsonFile(dir.File("missing.json")));
    dir.Write("bad.json", "{ not json");
    CHECK_FALSE(back.LoadJsonFile(dir.File("bad.json")));
    CHECK_FALSE(m.SaveJsonFile(dir.File("no/such/dir/x.json")));
}

TEST_CASE("EffectPresetManager mutations move, rename and remove items", "[effects][classic2][presets]") {
    EffectPresetManager m;
    EffectPresetGroup* a = m.AddGroup(nullptr, "A");
    EffectPresetGroup* b = m.AddGroup(nullptr, "B");
    EffectPreset* p1 = m.AddPreset(a, "p1", "x\ty", "1", "v1");
    EffectPreset* p2 = m.AddPreset(a, "p2", "x\ty", "1", "v1");
    EffectPreset* p3 = m.AddPreset(b, "p3", "x\ty", "1", "v1");
    CHECK(a->HasChildNamed("p1"));
    CHECK_FALSE(a->HasChildNamed("p3"));

    // move p1 after p3 in B
    m.MoveItem(p1, b, p3);
    CHECK(p1->GetParent() == b);
    REQUIRE(b->GetChildren().size() == 2);
    CHECK(b->GetChildren()[1].get() == p1);
    // move p2 to the end of B
    m.MoveItem(p2, b);
    CHECK(b->GetChildren().back().get() == p2);
    CHECK(a->GetChildren().empty());

    // InsertChildAfter: prepend, after an existing sibling and after a stranger (appends)
    auto* first = b->InsertChildAfter(std::make_unique<EffectPreset>("first", "x\ty", "", ""), nullptr);
    CHECK(b->GetChildren().front().get() == first);
    EffectPreset stranger("s", "", "", "");
    auto* last = b->InsertChildAfter(std::make_unique<EffectPreset>("last", "x\ty", "", ""), &stranger);
    CHECK(b->GetChildren().back().get() == last);
    CHECK(b->RemoveChild(&stranger) == nullptr);

    m.RenameItem(p3, "renamed");
    CHECK(m.FindPresetByPath("B\\renamed") == p3);
    m.RenameItem(nullptr, "x");

    m.UpdatePresetSettings(p3, "new\tsettings", "v2");
    CHECK(p3->GetSettings() == "new\tsettings");
    CHECK(p3->GetXLightsVersion() == "v2");
    m.UpdatePresetSettings(nullptr, "a", "b");

    m.Remove(p2);
    CHECK_FALSE(b->HasChildNamed("p2"));
    m.Remove(nullptr);
    EffectPreset orphan("o", "", "", "");
    m.Remove(&orphan);
    m.MoveItem(nullptr, b);
    m.MoveItem(p3, nullptr);
    m.MoveItem(&orphan, a);
    CHECK(m.GetAllPresetPaths().size() == 4);

    m.Reset();
    CHECK(m.GetRoot().GetChildren().empty());
}

TEST_CASE("EffectPresetManager imports XML and fixes illegal or duplicate names", "[effects][classic2][presets]") {
    EffectPresetManager m;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<x>
        <effect name="a/b" settings="no tabs"/>
        <effect name="dup" settings="1&#x9;2"/>
        <effect name="dup" settings="1&#x9;2"/>
        <effect name="dup" settings="1&#x9;2"/>
        <effectGroup name="g:1"><effect name="in*ner" settings="1&#x9;2"/><effect name="inner" settings="1&#x9;2"/></effectGroup>
        <other/>
    </x>)"));
    m.ImportFromXml(doc.child("x"), nullptr);
    // import keeps presets even without tab separated settings
    CHECK(m.GetRoot().GetChildren().size() == 5);
    m.ImportFromXml(pugi::xml_node(), nullptr);

    CHECK(m.FixRgbEffects());
    auto paths = m.GetAllPresetPaths();
    CHECK(paths == std::vector<std::string>{ "ab", "dup", "dup 1", "dup 2", "g1\\inner", "g1\\inner 1" });
    CHECK_FALSE(m.FixRgbEffects());

    EffectPresetGroup* g = static_cast<EffectPresetGroup*>(m.FindItemByPath("g1"));
    REQUIRE(g != nullptr);
    pugi::xml_document more;
    more.load_string("<x><effect name=\"extra\" settings=\"a&#x9;b\"/></x>");
    m.ImportFromXml(more.child("x"), g);
    CHECK(m.FindPresetByPath("g1\\extra") != nullptr);
}

// ---------------------------------------------------------------------------
// Known bugs (hidden from the default run)
// ---------------------------------------------------------------------------

// Duration 100 (the metadata maximum) makes the tail length zero, so head and tail ends
// coincide and nothing is drawn for the whole effect.
TEST_CASE("Galaxy with a 100 percent head duration still draws the head", "[.knownbug]") {
    Fx f;
    Opts o;
    o.w = 20;
    o.h = 20;
    o.periods = Range(0, 19);
    Frames fr = f.Render("Galaxy", "E_SLIDER_Galaxy_Duration=100", kRedBlue, o);
    CHECK(fr.totalLit() > 0);
}

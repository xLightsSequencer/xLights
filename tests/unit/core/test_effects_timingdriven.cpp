// Faces, State and Guitar: the effects driven by timing tracks and by the face/state
// definitions stored on a model.
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "test_render_common.h"

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "effects/StateEffect.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "utils/UtilClasses.h"
#include "utils/xlImage.h"

namespace {

using renderhelp::Rendered;

struct Mark {
    int s;
    int e;
    std::string label;
};

// One timing track. A lyric track has three layers: phrases, words, phonemes.
struct Track {
    std::string name;
    std::vector<std::vector<Mark>> layers;
};

struct Fx {
    std::string name;
    int s = 0;
    int e = 1000;
    std::string settings;
    std::string palette = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";
};

// A model element row. With sub set, the effects go on that submodel's layer.
struct Row {
    std::string model;
    std::vector<Fx> fx;
    std::string sub;
};

std::string BuildSeq(int durationMs, const std::vector<Row>& rows, const std::vector<Track>& tracks, int frameMs = 50) {
    using renderhelp::XmlEscape;
    std::vector<std::string> pals;
    std::vector<std::string> sets;
    auto indexOf = [](std::vector<std::string>& v, const std::string& x) {
        for (size_t i = 0; i < v.size(); ++i) {
            if (v[i] == x) return (int)i;
        }
        v.push_back(x);
        return (int)v.size() - 1;
    };
    std::string out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
                      "<head><version>2024.01</version><mediaFile></mediaFile><sequenceTiming>" +
                      std::to_string(frameMs) + " ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>" + renderhelp::Seconds(durationMs) + "</sequenceDuration></head>";
    out += "<DisplayElements>";
    for (const auto& t : tracks) {
        out += "<Element collapsed=\"0\" type=\"timing\" name=\"" + t.name + "\" visible=\"1\" active=\"1\" views=\"\"/>";
    }
    std::set<std::string> seen;
    for (const auto& r : rows) {
        if (seen.insert(r.model).second) {
            out += "<Element collapsed=\"0\" type=\"model\" name=\"" + r.model + "\" visible=\"1\"/>";
        }
    }
    out += "</DisplayElements>";
    auto fxXml = [&](const std::vector<Fx>& fx) {
        std::string b;
        for (const auto& e : fx) {
            b += "<Effect ref=\"" + std::to_string(indexOf(sets, e.settings)) + "\" name=\"" + e.name + "\" startTime=\"" + std::to_string(e.s) + "\" endTime=\"" + std::to_string(e.e) + "\" palette=\"" + std::to_string(indexOf(pals, e.palette)) + "\"/>";
        }
        return b;
    };
    std::string body = "<ElementEffects>";
    for (const auto& t : tracks) {
        body += "<Element type=\"timing\" name=\"" + t.name + "\">";
        for (const auto& l : t.layers) {
            body += "<EffectLayer>";
            for (const auto& m : l) {
                body += "<Effect label=\"" + XmlEscape(m.label) + "\" startTime=\"" + std::to_string(m.s) + "\" endTime=\"" + std::to_string(m.e) + "\"/>";
            }
            body += "</EffectLayer>";
        }
        body += "</Element>";
    }
    for (const auto& r : rows) {
        body += "<Element type=\"model\" name=\"" + r.model + "\">";
        if (r.sub.empty()) {
            body += "<EffectLayer>" + fxXml(r.fx) + "</EffectLayer>";
        } else {
            body += "<EffectLayer/><SubModelEffectLayer name=\"" + r.sub + "\">" + fxXml(r.fx) + "</SubModelEffectLayer>";
        }
        body += "</Element>";
    }
    body += "</ElementEffects>";
    out += "<EffectDB>";
    for (const auto& x : sets) out += "<Effect>" + XmlEscape(x) + "</Effect>";
    out += "</EffectDB><ColorPalettes>";
    for (const auto& x : pals) out += "<ColorPalette>" + XmlEscape(x) + "</ColorPalette>";
    out += "</ColorPalettes>" + body + "</xsequence>";
    return out;
}

// Whole-show render through the engine (models with real nodes, so face/state node
// ranges map to channels).
struct EngineShow {
    TempDir dir;
    renderhelp::FixFileReset reset;
    std::unique_ptr<HeadlessRenderContext> ctx;
    Rendered r;

    EngineShow(const std::string& models, const std::string& groups, int durationMs, const std::vector<Row>& rows,
               const std::vector<Track>& tracks, const std::map<std::string, std::string>& extraFiles = {}) {
        renderhelp::EnsureLoggers();
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
        dir.Write("xlights_rgbeffects.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n" + models + "\n</models>\n<modelGroups>\n" + groups + "\n</modelGroups>\n</xrgb>\n");
        for (const auto& f : extraFiles) dir.Write(f.first, f.second);
        std::string xsq = dir.Write("seq.xsq", BuildSeq(durationMs, rows, tracks));
        ctx = std::make_unique<HeadlessRenderContext>();
        r.loaded = ctx->LoadShowFolder(dir.path.string());
        if (!r.loaded) return;
        r.opened = ctx->OpenSequence(xsq);
        if (!r.opened) return;
        r.rendered = ctx->RenderAndWait(60000);
        auto& sd = ctx->_seqData;
        r.channels = sd.NumChannels();
        r.frames = sd.NumFrames();
        r.data.assign(r.frames, std::vector<uint8_t>(r.channels));
        for (unsigned int f = 0; f < r.frames; ++f) {
            for (unsigned int c = 0; c < r.channels; ++c) r.data[f][c] = sd[f][c];
        }
    }
};

std::string Line(const std::string& name, int nodes, const std::string& children, const std::string& extraAttrs = "") {
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) + "\" StringType=\"RGB Nodes\"" + extraAttrs + ">" + children + "</model>";
}

using Rgb = Rendered::Rgb;
const Rgb kBlack{ 0, 0, 0 };
const Rgb kRed{ 255, 0, 0 };
const Rgb kGreen{ 0, 255, 0 };
const Rgb kBlue{ 0, 0, 255 };
const Rgb kWhite{ 255, 255, 255 };
const Rgb kYellow{ 255, 255, 0 };

// The set of lit nodes in a frame of a single-line render.
std::set<int> LitNodes(const Rendered& r, unsigned frame) {
    std::set<int> s;
    for (unsigned n = 0; n * 3 + 2 < r.channels; ++n) {
        if (!(r.rgb(frame, n) == kBlack)) s.insert((int)n);
    }
    return s;
}

std::set<int> Nodes(std::initializer_list<int> l) { return std::set<int>(l); }

std::set<int> NodeRange(int from, int to) {
    std::set<int> s;
    for (int i = from; i <= to; ++i) s.insert(i);
    return s;
}

int Brightness(const Rgb& c) { return c.r + c.g + c.b; }

std::string Pal(std::initializer_list<const char*> cols) {
    std::string p;
    int i = 1;
    for (const char* c : cols) {
        if (!p.empty()) p += ",";
        p += "C_BUTTON_Palette" + std::to_string(i) + "=" + c + ",C_CHECKBOX_Palette" + std::to_string(i) + "=1";
        ++i;
    }
    return p;
}

// ---------------------------------------------------------------------------------------
// Direct RenderBuffer driver (no engine) for effects that only draw with SetPixel
// (Guitar, and the "Rendered"/picture face types).

using Pixels = std::vector<xlColor>;

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;
    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    bool on(size_t frame, int x, int y) const {
        const xlColor& c = at(frame, x, y);
        return c.alpha != 0 && (c.red || c.green || c.blue);
    }
    int lit(size_t frame) const {
        int n = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                if (on(frame, x, y)) ++n;
        return n;
    }
    int litInRow(size_t frame, int y) const {
        int n = 0;
        for (int x = 0; x < w; ++x)
            if (on(frame, x, y)) ++n;
        return n;
    }
    std::set<int> litRows(size_t frame) const {
        std::set<int> s;
        for (int y = 0; y < h; ++y)
            if (litInRow(frame, y) > 0) s.insert(y);
        return s;
    }
};

struct Fixture {
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> model;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;

    Fixture() {
        renderhelp::EnsureLoggers();
        model.reset(ctx.AllModels.CreateDefaultModel("Single Line"));
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    void AddTrack(const Track& t) {
        Element* e = ctx._sequenceElements.AddElement(t.name, "timing", true, false, true, false, false);
        REQUIRE(e != nullptr);
        for (size_t i = 0; i < t.layers.size(); ++i) {
            EffectLayer* l = i < e->GetEffectLayerCount() ? e->GetEffectLayer(i) : e->AddEffectLayer();
            for (const auto& m : t.layers[i]) {
                l->AddEffect(0, m.label, "", "", m.s, m.e, 0, false);
            }
        }
    }

    RenderableEffect* Fx(const std::string& name) {
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);
        return fx;
    }

    Effect* Add(const std::string& name, const std::string& settings, const std::string& palette, int endMs) {
        Effect* e = layer->AddEffect(0, name, settings, palette, 0, endMs, 0, false);
        REQUIRE(e != nullptr);
        return e;
    }

    Frames Render(const std::string& name, const std::string& settings, const std::string& palette,
                  int w, int h, const std::vector<int>& periods, int endMs = 1000) {
        Effect* e = Add(name, settings, palette, endMs);
        RenderableEffect* fx = Fx(name);
        RenderBuffer rb(&ctx, nullptr, model.get());
        rb.InitBuffer(h, w, "None");
        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        if (colors.empty()) {
            colors.push_back(xlWHITE);
            curves.push_back(ColorCurve());
        }
        rb.SetPalette(colors, curves);
        rb.SetFrameTimeInMs(50);
        rb.SetEffectDuration(0, endMs);
        SettingsMap rs;
        e->CopySettingsMap(rs, true);
        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            rb.SetState(p, first);
            first = false;
            fx->Render(e, rs, rb);
            out.f.emplace_back(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
        }
        return out;
    }
};

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

bool Same(const xlColor& a, const xlColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

bool HasError(const std::list<std::string>& l, const std::string& needle) {
    for (const auto& s : l) {
        if (s.find(needle) != std::string::npos) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------
// State fixtures

const std::string kStateNR = "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-3\" s1-Name=\"one\" s2=\"4-6\" s2-Name=\"two\" s3=\"7,9\" s3-Name=\"three\"/>";

std::string StateSettings(const std::string& extra) {
    return "E_CHOICE_State_StateDefinition=S," + extra;
}

} // namespace

// =======================================================================================
// State
// =======================================================================================

TEST_CASE("State with a fixed state lights that state's node range in its allocated colour", "[effects][timingdriven]") {
    EngineShow s(Line("M", 10, kStateNR), "", 500,
                 { { "M", { { "State", 0, 500, StateSettings("E_CHOICE_State_State=two,E_CHOICE_State_Color=Allocate"), Pal({ "#FF0000", "#00FF00", "#0000FF" }) } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 3) == NodeRange(3, 5));
    // s2 -> second palette colour
    CHECK(s.r.rgb(3, 4) == kGreen);
}

TEST_CASE("State follows the timing track and changes when the mark changes", "[effects][timingdriven]") {
    Track t{ "T", { { { 0, 500, "one" }, { 500, 800, "Three" } } } };
    EngineShow s(Line("M", 10, kStateNR), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Default,E_CHOICE_State_Color=Allocate"), Pal({ "#FF0000", "#00FF00", "#0000FF" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == NodeRange(0, 2));
    CHECK(s.r.rgb(2, 0) == kRed);
    // labels are case-insensitive; s3 is "7,9" -> nodes 6 and 8, third colour
    CHECK(LitNodes(s.r, 12) == Nodes({ 6, 8 }));
    CHECK(s.r.rgb(12, 6) == kBlue);
    // no mark after 800ms -> nothing
    CHECK(LitNodes(s.r, 18).empty());
}

TEST_CASE("State labels can name several states, or all of them", "[effects][timingdriven]") {
    Track t{ "T", { { { 0, 500, "one,two" }, { 500, 1000, "*" } } } };
    EngineShow s(Line("M", 10, kStateNR), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Default,E_CHOICE_State_Color=Allocate"), Pal({ "#FF0000", "#00FF00" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 3) == NodeRange(0, 5));
    std::set<int> all = NodeRange(0, 5);
    all.insert(6);
    all.insert(8);
    CHECK(LitNodes(s.r, 15) == all);
    // three states, two colours: s3 wraps back to the first colour
    CHECK(s.r.rgb(15, 6) == kRed);
}

TEST_CASE("State custom colours come from the definition, white when unset", "[effects][timingdriven]") {
    std::string def = "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"1\" s1=\"1-2\" s1-Name=\"a\" s1-Color=\"#FFFF00\" s2=\"3\" s2-Name=\"b\"/>";
    EngineShow s(Line("M", 4, def), "", 300,
                 { { "M", { { "State", 0, 300, StateSettings("E_CHOICE_State_State=a b"), Pal({ "#FF0000" }) } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(s.r.rgb(2, 0) == kYellow);
    CHECK(s.r.rgb(2, 1) == kYellow);
    CHECK(s.r.rgb(2, 2) == kWhite);
    CHECK(s.r.rgb(2, 3) == kBlack);
}

TEST_CASE("State cycle colour mode advances one palette colour per timing mark", "[effects][timingdriven]") {
    Track t{ "T", { { { 0, 300, "one" }, { 300, 600, "one" }, { 600, 1000, "one" } } } };
    EngineShow s(Line("M", 10, kStateNR), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Default,E_CHOICE_State_Color=Cycle"), Pal({ "#FF0000", "#00FF00" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(s.r.rgb(2, 0) == kRed);
    CHECK(s.r.rgb(8, 0) == kGreen);
    CHECK(s.r.rgb(15, 0) == kRed);
}

TEST_CASE("State graduate colour mode blends across the effect", "[effects][timingdriven]") {
    EngineShow s(Line("M", 10, kStateNR), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_State=one,E_CHOICE_State_Color=Graduate"), Pal({ "#FF0000", "#0000FF" }) } } } }, {});
    REQUIRE(s.r.rendered);
    auto early = s.r.rgb(0, 0);
    auto late = s.r.rgb(19, 0);
    CHECK(early.r > early.b);
    CHECK(late.b > late.r);
    CHECK(early.g == 0);
}

TEST_CASE("State fade time ramps a mark in and out", "[effects][timingdriven]") {
    Track t{ "T", { { { 0, 1000, "one" } } } };
    EngineShow s(Line("M", 10, kStateNR), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Default,E_CHOICE_State_Color=Allocate,E_SLIDER_State_Fade_Time=300"), Pal({ "#FF0000" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    int first = s.r.rgb(0, 0).r;
    int mid = s.r.rgb(10, 0).r;
    int last = s.r.rgb(19, 0).r;
    CHECK(mid == 255);
    CHECK(first < 128);
    CHECK(last < 128);
    CHECK(s.r.rgb(2, 0).r > first);
}

TEST_CASE("State single-node definitions match node names, and Coro falls back to SingleNode", "[effects][timingdriven]") {
    std::string def = "<stateInfo Name=\"SingleNode\" Type=\"SingleNode\" CustomColors=\"0\" s1=\"Node 2,Node 4\" s1-Name=\"pair\"/>";
    EngineShow s(Line("M", 5, def), "", 300,
                 { { "M", { { "State", 0, 300, "E_CHOICE_State_StateDefinition=Coro,E_CHOICE_State_State=pair,E_CHOICE_State_Color=Allocate", Pal({ "#00FF00" }) } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == Nodes({ 1, 3 }));
    CHECK(s.r.rgb(2, 1) == kGreen);
}

TEST_CASE("State with an unknown definition or missing track draws nothing", "[effects][timingdriven]") {
    EngineShow s(Line("M", 10, kStateNR), "", 300,
                 { { "M", { { "State", 0, 300, "E_CHOICE_State_StateDefinition=Nope,E_CHOICE_State_State=one", Pal({ "#FF0000" }) },
                            } } },
                 {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2).empty());
    EngineShow s2(Line("M", 10, kStateNR), "", 300,
                  { { "M", { { "State", 0, 300, StateSettings("E_CHOICE_State_TimingTrack=Missing"), Pal({ "#FF0000" }) } } } }, {});
    REQUIRE(s2.r.rendered);
    CHECK(LitNodes(s2.r, 2).empty());
}

namespace {
// Digit-segment style states for the numeric modes.
std::string NumericStates(const std::vector<std::string>& names) {
    std::string d = "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"0\"";
    for (size_t i = 0; i < names.size(); ++i) {
        std::string k = "s" + std::to_string(i + 1);
        d += " " + k + "=\"" + std::to_string(i + 1) + "\" " + k + "-Name=\"" + names[i] + "\"";
    }
    return d + "/>";
}
} // namespace

TEST_CASE("State countdown mode splits the remaining seconds into digit states", "[effects][timingdriven]") {
    // node n+1 = state names[n]
    std::vector<std::string> names = { "10", "2", "1", "0", "100", "00" };
    Track t{ "T", { { { 0, 2000, "12" } } } };
    EngineShow s(Line("M", 6, NumericStates(names)), "", 2000,
                 { { "M", { { "State", 0, 2000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Countdown,E_CHOICE_State_Color=Allocate"), Pal({ "#FF0000" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    // 12 seconds left -> "10" + "2"
    CHECK(LitNodes(s.r, 0) == Nodes({ 0, 1 }));
    // the remaining time is truncated to whole seconds: 11.95s -> "10" + "1"
    CHECK(LitNodes(s.r, 1) == Nodes({ 0, 2 }));
    // 10.75s -> "10" + "0"
    CHECK(LitNodes(s.r, 25) == Nodes({ 0, 3 }));
}

TEST_CASE("State countdown above 100 pads the tens with zero states", "[effects][timingdriven]") {
    std::vector<std::string> names = { "100", "00", "5", "1000" };
    Track t{ "T", { { { 0, 500, "105" } } } };
    EngineShow s(Line("M", 4, NumericStates(names)), "", 500,
                 { { "M", { { "State", 0, 500, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Countdown"), Pal({ "#FF0000" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 0) == Nodes({ 0, 1, 2 }));
    // 104 -> "100" "00" "4"
    CHECK(LitNodes(s.r, 1) == Nodes({ 0, 1 }));
}

TEST_CASE("State time countdown mode shows minutes, seconds and the colon", "[effects][timingdriven]") {
    std::vector<std::string> names = { "0000", "100", "00", "5", "colon", "4", "10" };
    Track t{ "T", { { { 0, 2000, "01:05" } } } };
    EngineShow s(Line("M", 7, NumericStates(names)), "", 2000,
                 { { "M", { { "State", 0, 2000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Time Countdown"), Pal({ "#FF0000" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    // 1:05 -> 0000 100 00 5 colon
    CHECK(LitNodes(s.r, 1) == Nodes({ 0, 1, 2, 3, 4 }));
    // one second later -> 1:04
    CHECK(LitNodes(s.r, 25) == Nodes({ 0, 1, 2, 4, 5 }));
}

TEST_CASE("State time countdown accepts hours", "[effects][timingdriven]") {
    std::vector<std::string> names = { "1000", "200", "30", "4", "colon" };
    Track t{ "T", { { { 0, 500, "1:12:34" } } } };
    EngineShow s(Line("M", 5, NumericStates(names)), "", 500,
                 { { "M", { { "State", 0, 500, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Time Countdown"), Pal({ "#FF0000" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    // 72 minutes -> minutes shown modulo 60 = 12 -> 1000 200 ; 34 -> 30 4
    CHECK(LitNodes(s.r, 1) == Nodes({ 0, 1, 2, 3, 4 }));
}

TEST_CASE("State number mode lights the digits of a frequency", "[effects][timingdriven]") {
    std::vector<std::string> names = { "dot", "7", "800", "80", "000", "1000", "00" };
    Track t{ "T", { { { 0, 500, "88.7" }, { 500, 1000, "101.1" } } } };
    EngineShow s(Line("M", 7, NumericStates(names)), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Number"), Pal({ "#FF0000" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 3) == Nodes({ 0, 1, 2, 3 }));
    // 101.1 -> dot 1 1000 000 10 : only dot, 1000 and 000 exist here
    CHECK(LitNodes(s.r, 15) == Nodes({ 0, 4, 5 }));
}

TEST_CASE("State iterate mode steps through the listed states across a mark", "[effects][timingdriven]") {
    Track t{ "T", { { { 0, 1000, "one two" } } } };
    EngineShow s(Line("M", 10, kStateNR), "", 1000,
                 { { "M", { { "State", 0, 1000, StateSettings("E_CHOICE_State_TimingTrack=T,E_CHOICE_State_Mode=Iterate,E_CHOICE_State_Color=Allocate"), Pal({ "#FF0000", "#00FF00" }) } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == NodeRange(0, 2));
    CHECK(LitNodes(s.r, 15) == NodeRange(3, 5));
}

TEST_CASE("State on a submodel maps the parent's state nodes into the submodel", "[effects][timingdriven]") {
    std::string children = kStateNR + "<subModel name=\"Sub\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"4-8\"/>";
    EngineShow s(Line("M", 10, children), "", 300,
                 { { "M", { { "State", 0, 300, StateSettings("E_CHOICE_State_State=one two,E_CHOICE_State_Color=Allocate"), Pal({ "#FF0000", "#00FF00" }) } }, "Sub" } }, {});
    REQUIRE(s.r.rendered);
    // "one" (nodes 1-3) lies outside the submodel; "two" (4-6) is inside
    CHECK(LitNodes(s.r, 2) == NodeRange(3, 5));
}

TEST_CASE("State settings checks, state lists and timing-track renames", "[effects][timingdriven]") {
    Fixture f;
    f.model->SetStateInfo({ { "S", { { "Type", "NodeRange" }, { "s1", "1" }, { "s1-Name", "one" }, { "s2", "2" }, { "s2-Name", "two" }, { "s3", "3" }, { "s3-Name", "" } } } });
    f.AddTrack({ "T", { { { 0, 100, "one" } } } });
    RenderableEffect* fx = f.Fx("State");
    auto* state = dynamic_cast<StateEffect*>(fx);
    REQUIRE(state != nullptr);

    auto states = state->GetStates(f.model.get(), "S");
    CHECK(states == std::list<std::string>{ "one", "two" });
    CHECK(state->GetStates(f.model.get(), "Other").empty());
    CHECK(state->GetStates(nullptr, "S").empty());

    SettingsMap sm;
    sm["E_CHOICE_State_StateDefinition"] = "S";
    CHECK(state->GetStatesUsed(sm) == std::list<std::string>{ "S" });
    CHECK(state->GetStatesUsed(SettingsMap()).empty());

    Effect* e = f.Add("State", "E_CHOICE_State_TimingTrack=T", "", 1000);
    SettingsMap ok;
    ok["E_CHOICE_State_TimingTrack"] = "T";
    CHECK_FALSE(HasError(fx->CheckEffectSettings(ok, nullptr, f.model.get(), e, false), "ERR"));
    CHECK(HasError(fx->CheckEffectSettings(SettingsMap(), nullptr, f.model.get(), e, false), "no timing selected"));
    SettingsMap bad;
    bad["E_CHOICE_State_TimingTrack"] = "Gone";
    bad["B_CHOICE_BufferTransform"] = "Rotate CC 90";
    auto res = fx->CheckEffectSettings(bad, nullptr, f.model.get(), e, false);
    CHECK(HasError(res, "unknown timing"));
    CHECK(HasError(res, "transformed buffer"));

    fx->RenameTimingTrack("Other", "X", e);
    CHECK(e->GetSettings().Get("E_CHOICE_State_TimingTrack", "") == "T");
    fx->RenameTimingTrack("T", "T2", e);
    CHECK(e->GetSettings().Get("E_CHOICE_State_TimingTrack", "") == "T2");
}

// =======================================================================================
// Faces
// =======================================================================================

namespace {
// Node-range face on a 12 node line (1-based ranges in the definition).
const std::string kFaceNR = "<faceInfo Name=\"F\" Type=\"NodeRange\" CustomColors=\"0\" "
                            "Mouth-AI=\"1-2\" Mouth-E=\"3-4\" Mouth-O=\"5\" Mouth-rest=\"6\" Mouth-MBP=\"7\" "
                            "Eyes-Open=\"9-10\" Eyes-Closed=\"11\" FaceOutline=\"12\" FaceOutline2=\"8\"/>";

const std::string kFacePal = Pal({ "#FF0000", "#00FF00", "#0000FF", "#FFFF00" });

std::string FaceSettings(const std::string& extra) {
    return "E_CHOICE_Faces_FaceDefinition=F," + extra;
}

Track Lyrics(const std::vector<Mark>& phonemes) {
    int end = phonemes.empty() ? 0 : phonemes.back().e;
    return { "L", { { { 0, end, "phrase" } }, { { 0, end, "word" } }, phonemes } };
}
} // namespace

TEST_CASE("Faces node-range face lights the mouth and open eyes for a fixed phoneme", "[effects][timingdriven]") {
    EngineShow s(Line("M", 12, kFaceNR), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open"), kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == Nodes({ 0, 1, 8, 9 }));
    CHECK(s.r.rgb(2, 0) == kRed);
    CHECK(s.r.rgb(2, 8) == kGreen);
}

TEST_CASE("Faces follows the phoneme layer of a lyric track", "[effects][timingdriven]") {
    Track t = Lyrics({ { 0, 300, "AI" }, { 300, 600, "E" }, { 600, 800, "O" } });
    EngineShow s(Line("M", 12, kFaceNR), "", 1000,
                 { { "M", { { "Faces", 0, 1000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Open"), kFacePal } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == Nodes({ 0, 1, 8, 9 }));
    CHECK(LitNodes(s.r, 8) == Nodes({ 2, 3, 8, 9 }));
    CHECK(LitNodes(s.r, 14) == Nodes({ 4, 8, 9 }));
    // after the last phoneme the face rests
    CHECK(LitNodes(s.r, 18) == Nodes({ 5, 8, 9 }));
}

TEST_CASE("Faces closed eyes, no eyes and the outline use their own colours", "[effects][timingdriven]") {
    EngineShow s(Line("M", 12, kFaceNR), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=MBP,E_CHOICE_Faces_Eyes=Closed,E_CHECKBOX_Faces_Outline=1"), kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == Nodes({ 6, 7, 10, 11 }));
    CHECK(s.r.rgb(2, 6) == kRed);
    CHECK(s.r.rgb(2, 10) == kGreen);
    CHECK(s.r.rgb(2, 11) == kBlue);  // FaceOutline: third colour
    CHECK(s.r.rgb(2, 7) == kYellow); // FaceOutline2: fourth colour

    EngineShow off(Line("M", 12, kFaceNR), "", 300,
                   { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=(off),E_CHOICE_Faces_Eyes=(off)"), kFacePal } } } }, {});
    REQUIRE(off.r.rendered);
    CHECK(LitNodes(off.r, 2).empty());
}

TEST_CASE("Faces custom colours come from the face definition", "[effects][timingdriven]") {
    std::string def = "<faceInfo Name=\"F\" Type=\"NodeRange\" CustomColors=\"1\" Mouth-AI=\"1\" Mouth-AI-Color=\"#00FF00\" Mouth-AI2=\"2\" "
                      "Eyes-Open=\"3\" Eyes-Open-Color=\"#0000FF\" Eyes-Open2=\"4\" Eyes-Open3=\"5\" Eyes-Open3-Color=\"#FFFF00\" "
                      "Eyes-Closed=\"6\" Eyes-Closed-Color=\"#FF0000\" Eyes-Closed2=\"7\" Eyes-Closed3=\"8\" Eyes-Closed3-Color=\"#00FF00\" "
                      "FaceOutline=\"9\" FaceOutline2=\"10\" FaceOutline2-Color=\"#0000FF\"/>";
    EngineShow s(Line("M", 10, def), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_Outline=1"), Pal({ "#FF0000" }) } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(s.r.rgb(2, 0) == kGreen);
    CHECK(s.r.rgb(2, 1) == kWhite);
    CHECK(s.r.rgb(2, 2) == kBlue);
    CHECK(s.r.rgb(2, 3) == kWhite);
    CHECK(s.r.rgb(2, 4) == kYellow);
    CHECK(s.r.rgb(2, 8) == kWhite);
    CHECK(s.r.rgb(2, 9) == kBlue);
    CHECK(s.r.rgb(2, 5) == kBlack);

    EngineShow c(Line("M", 10, def), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=(off),E_CHOICE_Faces_Eyes=Closed"), Pal({ "#FF0000" }) } } } }, {});
    REQUIRE(c.r.rendered);
    CHECK(LitNodes(c.r, 2) == Nodes({ 5, 6, 7 }));
    CHECK(c.r.rgb(2, 5) == kRed);
    CHECK(c.r.rgb(2, 6) == kWhite);
    CHECK(c.r.rgb(2, 7) == kGreen);
}

TEST_CASE("Faces default colours use the palette's later entries for the extra eye groups", "[effects][timingdriven]") {
    std::string def = "<faceInfo Name=\"F\" Type=\"NodeRange\" CustomColors=\"0\" Mouth-AI=\"1\" Eyes-Open=\"2\" Eyes-Open2=\"3\" Eyes-Open3=\"4\" "
                      "Eyes-Closed=\"5\" Eyes-Closed2=\"6\" Eyes-Closed3=\"7\"/>";
    std::string pal = Pal({ "#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#00FFFF", "#FF00FF" });
    EngineShow s(Line("M", 7, def), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open"), pal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(s.r.rgb(2, 1) == kGreen);
    CHECK(s.r.rgb(2, 2) == (Rgb{ 0, 255, 255 }));
    CHECK(s.r.rgb(2, 3) == (Rgb{ 255, 0, 255 }));
    EngineShow c(Line("M", 7, def), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Closed"), pal } } } }, {});
    REQUIRE(c.r.rendered);
    CHECK(LitNodes(c.r, 2) == Nodes({ 0, 4, 5, 6 }));
    CHECK(c.r.rgb(2, 5) == (Rgb{ 0, 255, 255 }));
}

TEST_CASE("Faces shimmer phonemes blink the mouth unless shimmer is suppressed", "[effects][timingdriven]") {
    Track t = Lyrics({ { 0, 1000, "AI-shimmer" } });
    EngineShow s(Line("M", 12, kFaceNR), "", 1000,
                 { { "M", { { "Faces", 0, 1000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Open"), kFacePal } } } }, { t });
    REQUIRE(s.r.rendered);
    // 50ms frames: every fourth frame (150ms into each 200ms cycle) the mouth is off
    CHECK(LitNodes(s.r, 2) == Nodes({ 0, 1, 8, 9 }));
    CHECK(LitNodes(s.r, 3) == Nodes({ 8, 9 }));
    CHECK(LitNodes(s.r, 7) == Nodes({ 8, 9 }));
    CHECK(LitNodes(s.r, 8) == Nodes({ 0, 1, 8, 9 }));

    EngineShow sup(Line("M", 12, kFaceNR), "", 1000,
                   { { "M", { { "Faces", 0, 1000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_SuppressShimmer=1"), kFacePal } } } }, { t });
    REQUIRE(sup.r.rendered);
    CHECK(LitNodes(sup.r, 3) == Nodes({ 0, 1, 8, 9 }));
}

TEST_CASE("Faces suppressed when not singing goes dark between phonemes, with lead frames", "[effects][timingdriven]") {
    Track t = Lyrics({ { 0, 200, "AI" }, { 800, 1000, "E" } });
    EngineShow s(Line("M", 12, kFaceNR), "", 1000,
                 { { "M", { { "Faces", 0, 1000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_SuppressWhenNotSinging=1"), kFacePal } } } }, { t });
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 1) == Nodes({ 0, 1, 8, 9 }));
    CHECK(LitNodes(s.r, 10).empty());
    CHECK(LitNodes(s.r, 17) == Nodes({ 2, 3, 8, 9 }));

    // two lead frames keep the face up just before and after a phoneme
    EngineShow lead(Line("M", 12, kFaceNR), "", 1000,
                    { { "M", { { "Faces", 0, 1000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_SuppressWhenNotSinging=1,E_SPINCTRL_Faces_LeadFrames=2"), kFacePal } } } }, { t });
    REQUIRE(lead.r.rendered);
    CHECK_FALSE(LitNodes(lead.r, 4).empty()); // just after the first phoneme
    CHECK(lead.r.rgb(4, 8) == kGreen);
    CHECK_FALSE(LitNodes(lead.r, 15).empty()); // just before the second
    CHECK(LitNodes(lead.r, 10).empty());

    // with fade the lead-in ramps up
    EngineShow fade(Line("M", 12, kFaceNR), "", 1000,
                    { { "M", { { "Faces", 0, 1000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_SuppressWhenNotSinging=1,E_SPINCTRL_Faces_LeadFrames=4,E_CHECKBOX_Faces_Fade=1"), kFacePal } } } }, { t });
    REQUIRE(fade.r.rendered);
    int b13 = Brightness(fade.r.rgb(13, 8));
    int b15 = Brightness(fade.r.rgb(15, 8));
    CHECK(b13 > 0);
    CHECK(b15 > b13);
    CHECK(b15 < 255);
    int a5 = Brightness(fade.r.rgb(5, 8));
    int a6 = Brightness(fade.r.rgb(6, 8));
    CHECK(a5 > a6);
}

TEST_CASE("Faces auto eyes blink while the face rests", "[effects][timingdriven]") {
    // no track: blinks follow the stable per-effect schedule
    EngineShow s(Line("M", 12, kFaceNR), "", 6000,
                 { { "M", { { "Faces", 0, 6000, FaceSettings("E_CHOICE_Faces_Phoneme=rest,E_CHOICE_Faces_Eyes=Auto,E_CHOICE_Faces_EyeBlinkFrequency=Fastest,E_CHOICE_Faces_EyeBlinkDuration=Longer"), kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    int open = 0, closed = 0;
    for (unsigned f = 0; f < s.r.frames; ++f) {
        auto lit = LitNodes(s.r, f);
        CHECK(lit.count(5) == 1); // rest mouth always on
        if (lit.count(8)) ++open;
        if (lit.count(10)) ++closed;
        CHECK(lit.count(8) + lit.count(10) == 1);
    }
    CHECK(open > closed);
    CHECK(closed > 0);

    // with a lyric track: blinks only land in the rest gaps
    Track t = Lyrics({ { 0, 500, "AI" }, { 3500, 4000, "E" } });
    EngineShow tr(Line("M", 12, kFaceNR), "", 4000,
                  { { "M", { { "Faces", 0, 4000, FaceSettings("E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Auto,E_CHOICE_Faces_EyeBlinkDuration=Long"), kFacePal } } } }, { t });
    REQUIRE(tr.r.rendered);
    int closedInRest = 0;
    for (unsigned f = 0; f < tr.r.frames; ++f) {
        auto lit = LitNodes(tr.r, f);
        bool singing = f < 10 || f >= 70;
        if (singing) {
            CHECK(lit.count(8) == 1);
            CHECK(lit.count(10) == 0);
        } else if (lit.count(10)) {
            ++closedInRest;
        }
    }
    CHECK(closedInRest > 0);
}

TEST_CASE("Faces single-node definitions look up nodes by name", "[effects][timingdriven]") {
    std::string def = "<faceInfo Name=\"SingleNode\" Type=\"SingleNode\" CustomColors=\"0\" Mouth-AI=\"Node 2,lipB\" Eyes-Open=\"Node 4\"/>";
    EngineShow s(Line("M", 5, def, " NodeNames=\"lipA,,lipB\""), "", 300,
                 { { "M", { { "Faces", 0, 300, "E_CHOICE_Faces_FaceDefinition=Coro,E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open", kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == Nodes({ 1, 2, 3 }));
    CHECK(s.r.rgb(2, 2) == kRed);
    CHECK(s.r.rgb(2, 3) == kGreen);
}

TEST_CASE("Faces with an unknown definition falls back to the model's first face", "[effects][timingdriven]") {
    EngineShow s(Line("M", 12, kFaceNR), "", 300,
                 { { "M", { { "Faces", 0, 300, "E_CHOICE_Faces_FaceDefinition=Renamed,E_CHOICE_Faces_Phoneme=E,E_CHOICE_Faces_Eyes=(off)", kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2) == Nodes({ 2, 3 }));
}

TEST_CASE("Faces outline state paints a custom-colour state over the outline", "[effects][timingdriven]") {
    std::string children = kFaceNR + "<stateInfo Name=\"Glow\" Type=\"NodeRange\" CustomColors=\"1\" s001=\"12\" s001-Color=\"#00FFFF\" s002=\"8\" s002-Name=\"x\"/>";
    EngineShow s(Line("M", 12, children), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_Outline=1,E_CHOICE_Faces_UseState=Glow"), kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(s.r.rgb(2, 11) == (Rgb{ 0, 255, 255 }));
    CHECK(s.r.rgb(2, 7) == kWhite); // no colour given -> white
    CHECK(s.r.rgb(2, 0) == kRed);
}

TEST_CASE("Faces on a submodel maps the parent's face nodes into the submodel", "[effects][timingdriven]") {
    std::string children = kFaceNR + "<subModel name=\"Sub\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-4\"/>";
    EngineShow s(Line("M", 12, children), "", 300,
                 { { "M", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=E,E_CHOICE_Faces_Eyes=Open"), kFacePal }, }, "Sub" } }, {});
    REQUIRE(s.r.rendered);
    // eyes (9-10) are outside the submodel
    CHECK(LitNodes(s.r, 2) == Nodes({ 2, 3 }));
}

TEST_CASE("Faces node-range face on a model group draws nothing", "[effects][timingdriven]") {
    std::string group = "<modelGroup name=\"G\" models=\"M\" layout=\"minimalGrid\" GridSize=\"400\"/>";
    EngineShow s(Line("M", 12, kFaceNR), group, 300,
                 { { "G", { { "Faces", 0, 300, FaceSettings("E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open"), kFacePal } } } }, {});
    REQUIRE(s.r.rendered);
    CHECK(LitNodes(s.r, 2).empty());
}

TEST_CASE("Faces Papagayo definitions light the cells listed for the phoneme", "[effects][timingdriven]") {
    std::string pgo = "<?xml version=\"1.0\"?><papagayo><presets><group name=\"g\"><voice name=\"?M\" AI=\"x@C1\" Eyes_open=\"x@(2,1)\" Outline=\"x@E1\"/></group></presets></papagayo>";
    EngineShow s(Line("M", 6, ""), "", 300,
                 { { "M", { { "Faces", 0, 300, "E_CHOICE_Faces_FaceDefinition=xlights_papagayo.xml,E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open,E_CHECKBOX_Faces_Outline=1", kFacePal } } } },
                 {}, { { "xlights_papagayo.xml", pgo } });
    REQUIRE(s.r.rendered);
    // columns are 1-based (A = node 0); single-colour nodes are driven white
    CHECK(LitNodes(s.r, 2) == Nodes({ 1, 2, 4 }));
    CHECK(s.r.rgb(2, 2) == kWhite);

    EngineShow none(Line("M", 6, ""), "", 300,
                    { { "M", { { "Faces", 0, 300, "E_CHOICE_Faces_FaceDefinition=xlights_papagayo.xml,E_CHOICE_Faces_Phoneme=AI", kFacePal } } } }, {});
    REQUIRE(none.r.rendered);
    CHECK(LitNodes(none.r, 2).empty());
}

TEST_CASE("Faces Papagayo auto phonemes fall back to the drawn face", "[effects][timingdriven]") {
    std::string pgo = "<?xml version=\"1.0\"?><papagayo><corofaces><group name=\"g\"><voice name=\"M\" AI=\"x@C1\"/></group></corofaces></papagayo>";
    EngineShow s(Line("M", 6, ""), "", 300,
                 { { "M", { { "Faces", 0, 300, "E_CHOICE_Faces_FaceDefinition=xlights_papagayo.xml,E_CHOICE_Faces_Phoneme=a-AI,E_CHOICE_Faces_Eyes=Open", kFacePal } } } },
                 {}, { { "xlights_papagayo.xml", pgo } });
    REQUIRE(s.r.rendered);
    REQUIRE(s.r.frames > 2);
    EngineShow t(Line("M", 6, ""), "", 300,
                 { { "M", { { "Faces", 0, 300, "E_CHOICE_Faces_FaceDefinition=xlights_papagayo.xml,E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open", kFacePal } } } },
                 {}, { { "xlights_papagayo.xml", pgo } });
    REQUIRE(t.r.rendered);
    CHECK(LitNodes(t.r, 2) == Nodes({ 2 }));
}

namespace {
// The drawn ("Rendered"/default) face on a 20x20 buffer: Wt = Ht = 19.
// Mouth corners x1 = 4, x2 = 14; y1 = 9, y2 = 7; AI's second line at y4 = 3.
Frames DrawnFace(Fixture& f, const std::string& extra, const std::vector<int>& periods = { 2 }) {
    return f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Default," + extra, kFacePal, 20, 20, periods, 1000);
}
} // namespace

TEST_CASE("Faces drawn face mouth shapes follow the phoneme", "[effects][timingdriven]") {
    Fixture f;
    Frames ai = DrawnFace(f, "E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=(off)");
    CHECK(ai.on(0, 9, 7));
    CHECK(ai.on(0, 9, 3));
    CHECK(ai.on(0, 4, 5));
    CHECK(Same(ai.at(0, 9, 7), xlRED));

    Frames mbp = DrawnFace(f, "E_CHOICE_Faces_Phoneme=MBP,E_CHOICE_Faces_Eyes=(off)");
    CHECK(mbp.on(0, 9, 7));
    CHECK_FALSE(mbp.on(0, 9, 3));
    CHECK(mbp.lit(0) == 9 + 4);

    Frames e = DrawnFace(f, "E_CHOICE_Faces_Phoneme=E,E_CHOICE_Faces_Eyes=(off)");
    CHECK(e.on(0, 9, 4)); // y3 = 4
    Frames fv = DrawnFace(f, "E_CHOICE_Faces_Phoneme=FV,E_CHOICE_Faces_Eyes=(off)");
    CHECK(fv.on(0, 9, 6));
    Frames etc = DrawnFace(f, "E_CHOICE_Faces_Phoneme=etc,E_CHOICE_Faces_Eyes=(off)");
    CHECK(etc.on(0, 9, 5)); // y5 = 5, box between x 5 and 13
    CHECK(etc.on(0, 5, 6));

    Frames o = DrawnFace(f, "E_CHOICE_Faces_Phoneme=O,E_CHOICE_Faces_Eyes=(off)");
    Frames u = DrawnFace(f, "E_CHOICE_Faces_Phoneme=U,E_CHOICE_Faces_Eyes=(off)");
    Frames wq = DrawnFace(f, "E_CHOICE_Faces_Phoneme=WQ,E_CHOICE_Faces_Eyes=(off)");
    CHECK(o.lit(0) > u.lit(0));
    CHECK(u.lit(0) > wq.lit(0));
    CHECK(wq.lit(0) > 0);
    Frames off = DrawnFace(f, "E_CHOICE_Faces_Phoneme=(off),E_CHOICE_Faces_Eyes=(off)");
    CHECK(off.lit(0) == 0);
}

TEST_CASE("Faces drawn face eyes and outline", "[effects][timingdriven]") {
    Fixture f;
    Frames none = DrawnFace(f, "E_CHOICE_Faces_Phoneme=MBP,E_CHOICE_Faces_Eyes=(off)");
    Frames open = DrawnFace(f, "E_CHOICE_Faces_Phoneme=MBP,E_CHOICE_Faces_Eyes=Open");
    Frames closed = DrawnFace(f, "E_CHOICE_Faces_Phoneme=MBP,E_CHOICE_Faces_Eyes=Closed");
    int openEyes = open.lit(0) - none.lit(0);
    int closedEyes = closed.lit(0) - none.lit(0);
    CHECK(openEyes > closedEyes);
    CHECK(closedEyes > 0);
    // eyes are drawn in the third palette colour, above the mouth
    bool blueEye = false;
    for (int x = 0; x < 20; ++x) {
        for (int y = 10; y < 20; ++y) {
            if (open.on(0, x, y)) {
                CHECK(Same(open.at(0, x, y), xlBLUE));
                blueEye = true;
            }
        }
    }
    CHECK(blueEye);

    Frames outline = DrawnFace(f, "E_CHOICE_Faces_Phoneme=MBP,E_CHOICE_Faces_Eyes=(off),E_CHECKBOX_Faces_Outline=1");
    CHECK(outline.on(0, 0, 10));
    CHECK(outline.on(0, 19, 10));
    CHECK(outline.on(0, 10, 0));
    CHECK(outline.on(0, 10, 19));
    CHECK(Same(outline.at(0, 0, 10), xlGREEN));
    CHECK_FALSE(none.on(0, 0, 10));
}

TEST_CASE("Faces drawn face reads the lyric track and blinks at rest", "[effects][timingdriven]") {
    Fixture f;
    f.AddTrack(Lyrics({ { 0, 250, "AI" }, { 250, 500, "O" } }));
    Frames fr = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Rendered,E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=(off)", kFacePal, 20, 20, { 1, 7, 15 }, 1000);
    CHECK(fr.on(0, 6, 3));
    CHECK_FALSE(fr.on(1, 6, 3));
    CHECK(fr.lit(1) != fr.lit(0));
    // after the track, rest: a single line at y2
    CHECK(fr.lit(2) == 13);

    // auto eyes on a resting drawn face close at some point with fast blinking
    Fixture g;
    Frames blink = g.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Default,E_CHOICE_Faces_Phoneme=rest,E_CHOICE_Faces_Eyes=Auto,E_CHOICE_Faces_EyeBlinkFrequency=Fastest,E_CHOICE_Faces_EyeBlinkDuration=Longer", kFacePal, 20, 20, Range(0, 59), 3000);
    std::set<int> counts;
    for (size_t i = 0; i < blink.f.size(); ++i) counts.insert(blink.lit(i));
    CHECK(counts.size() == 2);

    // no track with that name -> rest face
    Fixture h;
    Frames miss = h.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Default,E_CHOICE_Faces_TimingTrack=Nope,E_CHOICE_Faces_Eyes=(off)", kFacePal, 20, 20, { 2 }, 1000);
    CHECK(miss.lit(0) == 13);
}

TEST_CASE("Faces drawn face shimmer and blink settings", "[effects][timingdriven]") {
    Fixture f;
    Frames sh = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Default,E_CHOICE_Faces_Phoneme=MBP-shimmer,E_CHOICE_Faces_Eyes=(off)", kFacePal, 20, 20, { 2, 3 }, 1000);
    CHECK(sh.lit(0) == 13);
    CHECK(sh.lit(1) == 0);

    // every blink frequency / duration renders and keeps a rest mouth
    for (const char* freq : { "Slowest", "Slow", "Normal", "Fast", "Fastest" }) {
        for (const char* dur : { "Short", "Normal", "Long", "Longer" }) {
            Fixture g;
            Frames fr = g.Render("Faces", std::string("E_CHOICE_Faces_FaceDefinition=Default,E_CHOICE_Faces_Phoneme=(off),E_CHOICE_Faces_Eyes=Auto,E_CHOICE_Faces_EyeBlinkFrequency=") + freq + ",E_CHOICE_Faces_EyeBlinkDuration=" + dur, kFacePal, 20, 20, { 0, 40 }, 3000);
            INFO(freq << " " << dur);
            CHECK(fr.lit(0) > 0);
        }
    }
}

namespace {
std::string WritePng(const TempDir& d, const std::string& name, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            img.SetRGBA(x, y, r, g, b, 255);
    std::vector<uint8_t> png;
    REQUIRE(img.SaveAsPNG(png));
    return d.Write(name, std::string(png.begin(), png.end()));
}
} // namespace

TEST_CASE("Faces matrix face shows the picture for each phoneme", "[effects][timingdriven]") {
    TempDir d;
    std::string red = WritePng(d, "ai.png", 4, 4, 255, 0, 0);
    std::string blue = WritePng(d, "e.png", 4, 4, 0, 0, 255);
    std::string dim = WritePng(d, "dim.png", 4, 4, 10, 10, 10);
    Fixture f;
    f.model->SetFaceInfo({ { "Mx", { { "Type", "Matrix" }, { "CustomColors", "0" }, { "ImagePlacement", "Centered" },
                                     { "Mouth-AI-EyesOpen", red }, { "Mouth-E-EyesOpen", blue }, { "Mouth-rest-EyesClosed", dim }, { "Mouth-rest-EyesOpen", dim }, { "Mouth-O-EyesOpen", blue } } } });
    f.AddTrack(Lyrics({ { 0, 250, "AI" }, { 250, 500, "E" } }));
    Frames fr = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_TimingTrack=L,E_CHOICE_Faces_Eyes=Auto", kFacePal, 4, 4, { 1, 2, 7 }, 1000);
    CHECK(fr.lit(0) == 16);
    CHECK(Same(fr.at(0, 1, 1), xlRED));
    CHECK(Same(fr.at(1, 1, 1), xlRED)); // cached image reused
    CHECK(Same(fr.at(2, 1, 1), xlBLUE));

    // closed eyes with no closed picture falls back to the open one
    Frames cl = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_Phoneme=E,E_CHOICE_Faces_Eyes=(off),E_CHOICE_Faces_Scaling=Scale To Fit", kFacePal, 4, 4, { 1 }, 1000);
    CHECK(Same(cl.at(0, 2, 2), xlBLUE));

    // transparent black drops pixels at or below the level
    Frames tb = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_Phoneme=rest,E_CHOICE_Faces_Eyes=Closed,E_CHECKBOX_Faces_TransparentBlack=1,E_TEXTCTRL_Faces_TransparentBlack=50", kFacePal, 4, 4, { 1 }, 1000);
    CHECK(tb.lit(0) == 0);
    Frames notb = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_Phoneme=rest,E_CHOICE_Faces_Eyes=Closed", kFacePal, 4, 4, { 1 }, 1000);
    CHECK(notb.lit(0) == 16);

    // shimmer swaps to the rest picture on the off frames
    Frames sh = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_Phoneme=AI-shimmer,E_CHOICE_Faces_Eyes=Open", kFacePal, 4, 4, { 2, 3 }, 1000);
    CHECK(Same(sh.at(0, 0, 0), xlRED));
    CHECK(Same(sh.at(1, 0, 0), xlColor(10, 10, 10)));
}

TEST_CASE("Faces matrix face with movement renders uncached frames", "[effects][timingdriven]") {
    TempDir d;
    std::string red = WritePng(d, "ai.png", 2, 2, 255, 0, 0);
    Fixture f;
    f.model->SetFaceInfo({ { "Mx", { { "Type", "Matrix" }, { "ImagePlacement", "Scale Keep Aspect Ratio" }, { "Mouth-AI-EyesOpen", red } } } });
    Frames fr = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open,E_CHOICE_Faces_Scaling=No Scaling,E_CHOICE_Faces_Direction=left", kFacePal, 8, 8, { 0, 10, 19 }, 1000);
    std::set<std::vector<int>> shapes;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        std::vector<int> v;
        for (int y = 0; y < 8; ++y)
            for (int x = 0; x < 8; ++x) {
                if (fr.on(i, x, y)) {
                    CHECK(Same(fr.at(i, x, y), xlRED));
                    v.push_back(y * 8 + x);
                }
            }
        shapes.insert(v);
    }
    CHECK(shapes.size() > 1);

    Frames tb = f.Render("Faces", "E_CHOICE_Faces_FaceDefinition=Mx,E_CHOICE_Faces_Phoneme=AI,E_CHOICE_Faces_Eyes=Open,E_CHOICE_Faces_Scaling=No Scaling,E_SLIDER_FacesXC=10,E_CHECKBOX_Faces_TransparentBlack=1,E_TEXTCTRL_Faces_TransparentBlack=0", kFacePal, 8, 8, { 0 }, 1000);
    CHECK(tb.lit(0) == 4);
}

TEST_CASE("Faces settings checks, used faces, file references and renames", "[effects][timingdriven]") {
    TempDir d;
    std::string red = WritePng(d, "ai.png", 2, 2, 255, 0, 0);
    Fixture f;
    f.model->SetFaceInfo({ { "Mx", { { "Type", "Matrix" }, { "Mouth-AI-EyesOpen", red }, { "Mouth-E-EyesOpen", (d.path / "missing.png").string() } } },
                           { "NR", { { "Type", "NodeRange" }, { "Mouth-AI", "1" } } } });
    f.AddTrack(Lyrics({ { 0, 100, "AI" } }));
    RenderableEffect* fx = f.Fx("Faces");
    Effect* e = f.Add("Faces", "E_CHOICE_Faces_TimingTrack=L", "", 1000);

    SettingsMap mx;
    mx["E_CHOICE_Faces_FaceDefinition"] = "Mx";
    mx["E_CHOICE_Faces_TimingTrack"] = "L";
    auto res = fx->CheckEffectSettings(mx, nullptr, f.model.get(), e, false);
    CHECK(HasError(res, "image file not found"));
    CHECK_FALSE(HasError(res, "no timing selected"));

    SettingsMap nr;
    nr["E_CHOICE_Faces_FaceDefinition"] = "NR";
    nr["B_CHOICE_BufferTransform"] = "Rotate CC 90";
    nr["B_SLIDER_Rotation"] = "10";
    nr["B_CUSTOM_SubBuffer"] = "1x2x3x4";
    res = fx->CheckEffectSettings(nr, nullptr, f.model.get(), e, false);
    CHECK(HasError(res, "transformed buffer"));
    CHECK(HasError(res, "rotozoom"));
    CHECK(HasError(res, "subbuffer"));
    CHECK(HasError(res, "no timing selected"));

    SettingsMap missing;
    missing["E_CHOICE_Faces_FaceDefinition"] = "Gone";
    missing["E_CHOICE_Faces_TimingTrack"] = "Nope";
    res = fx->CheckEffectSettings(missing, nullptr, f.model.get(), e, false);
    CHECK(HasError(res, "does not exist"));
    CHECK(HasError(res, "unknown timing"));

    SettingsMap used;
    used["E_CHOICE_Faces_FaceDefinition"] = "Mx";
    CHECK(fx->GetFacesUsed(used) == std::list<std::string>{ "Mx" });
    CHECK(fx->GetFacesUsed(SettingsMap()).empty());

    auto refs = fx->GetFileReferences(&f.ctx, f.model.get(), used);
    CHECK(refs.size() == 2);
    CHECK(fx->GetFileReferences(&f.ctx, f.model.get(), nr).empty());
    CHECK(fx->GetFileReferences(&f.ctx, nullptr, used).empty());

    SettingsMap pgo;
    pgo["CHOICE_Faces_FaceDefinition"] = "xlights_papagayo.xml";
    CHECK(fx->GetFrameParallelism(pgo) == RenderableEffect::FrameParallelism::Stateful);
    CHECK(fx->GetFrameParallelism(SettingsMap()) == RenderableEffect::FrameParallelism::Pure);

    fx->RenameTimingTrack("L", "Lyrics", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Faces_TimingTrack", "") == "Lyrics");
    fx->RenameTimingTrack("L", "X", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Faces_TimingTrack", "") == "Lyrics");
}

// =======================================================================================
// Guitar
// =======================================================================================

namespace {
const std::string kGuitarPal = Pal({ "#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#00FFFF", "#FF00FF" });

// Buffer y = row; string s is drawn in row (H - 1 - (s * H/strings + H/(2*strings))).
Frames Guitar(Fixture& f, const std::vector<Mark>& notes, const std::string& extra, int w, int h, const std::vector<int>& periods, int endMs = 1000) {
    f.AddTrack({ "N", { notes } });
    return f.Render("Guitar", "E_CHOICE_Guitar_MIDITrack_APPLYLAST=N," + extra, kGuitarPal, w, h, periods, endMs);
}
} // namespace

TEST_CASE("Guitar open low E lights the whole bottom string", "[effects][timingdriven]") {
    Fixture f;
    Frames fr = Guitar(f, { { 0, 500, "E2" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2, 15 });
    CHECK(fr.litRows(0) == std::set<int>{ 5 });
    CHECK(fr.litInRow(0, 5) == 19);
    CHECK(Same(fr.at(0, 3, 5), xlRED));
    // no note after 500ms
    CHECK(fr.lit(1) == 0);
}

TEST_CASE("Guitar higher frets light a shorter length of string", "[effects][timingdriven]") {
    Fixture f;
    // F2 = fret 1 on the low string, G2 = fret 3
    Frames fr = Guitar(f, { { 0, 300, "F2" }, { 300, 600, "G2" }, { 600, 1000, "41" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2, 8, 15 });
    CHECK(fr.litRows(0) == std::set<int>{ 5 });
    CHECK(fr.litInRow(0, 5) == 18);
    CHECK(fr.litInRow(1, 5) == 16);
    // a MIDI note number works like its name
    CHECK(fr.litInRow(2, 5) == 18);
    CHECK_FALSE(fr.on(0, 18, 5));
    CHECK(fr.on(0, 0, 5));
}

TEST_CASE("Guitar note changes move to another string when the mark changes", "[effects][timingdriven]") {
    Fixture f;
    // E4 is the open top string
    Frames fr = Guitar(f, { { 0, 500, "E2" }, { 500, 1000, "e4" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2, 15 });
    CHECK(fr.litRows(0) == std::set<int>{ 5 });
    CHECK(fr.litRows(1) == std::set<int>{ 0 });
    CHECK(Same(fr.at(1, 0, 0), xlColor(255, 0, 255)));
}

TEST_CASE("Guitar chord names use their fingering", "[effects][timingdriven]") {
    Fixture f;
    // C major: strings 0..5 at frets 0,3,2,0,1,0
    Frames fr = Guitar(f, { { 0, 1000, "CC" } }, "E_CHOICE_StringAppearance=On,E_SLIDER_MaxFrets=19", 19, 6, { 2 });
    CHECK(fr.litRows(0).size() == 6);
    CHECK(fr.litInRow(0, 5) == 19);
    CHECK(fr.litInRow(0, 4) == 16);
    CHECK(fr.litInRow(0, 3) == 17);
    CHECK(fr.litInRow(0, 1) == 18);

    // the same notes given individually match the chord table too
    Fixture g;
    Frames notes = Guitar(g, { { 0, 1000, "E2 C3 E3 G3 C4 E4" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2 });
    for (int y = 0; y < 6; ++y) {
        CHECK(notes.litInRow(0, y) == fr.litInRow(0, y));
    }
}

TEST_CASE("Guitar string and position labels place a finger directly", "[effects][timingdriven]") {
    Fixture f;
    // S1 is the top string, P0 open
    Frames fr = Guitar(f, { { 0, 500, "S1P0" }, { 500, 1000, "S6P5, S9P1" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2, 15 });
    CHECK(fr.litRows(0) == std::set<int>{ 0 });
    CHECK(fr.litInRow(0, 0) == 19);
    CHECK(fr.litRows(1) == std::set<int>{ 5 });
    CHECK(fr.litInRow(1, 5) == 14);
}

TEST_CASE("Guitar several notes are spread over strings", "[effects][timingdriven]") {
    Fixture f;
    Frames fr = Guitar(f, { { 0, 1000, "A2 D3" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2 });
    CHECK(fr.litRows(0).size() == 2);
    Fixture g;
    // an unplayable note is dropped, the rest still shows
    Frames lo = Guitar(g, { { 0, 1000, "C1 E2" } }, "E_CHOICE_StringAppearance=On", 19, 6, { 2 });
    CHECK(lo.litRows(0) == std::set<int>{ 5 });
}

TEST_CASE("Guitar fade and collapse shrink the note over its length", "[effects][timingdriven]") {
    Fixture f;
    Frames fade = Guitar(f, { { 0, 1000, "E2" } }, "E_CHOICE_StringAppearance=On,E_CHECKBOX_Fade=1", 19, 6, { 0, 10, 18 });
    CHECK(fade.at(0, 0, 5).alpha == 255);
    CHECK(fade.at(1, 0, 5).alpha < fade.at(0, 0, 5).alpha);
    CHECK(fade.at(2, 0, 5).alpha < fade.at(1, 0, 5).alpha);

    Fixture g;
    // 24 rows / 6 strings = 4 rows per string
    Frames col = Guitar(g, { { 0, 1000, "E2" } }, "E_CHOICE_StringAppearance=On,E_CHECKBOX_Collapse=1", 19, 24, { 0, 10, 19 });
    CHECK(col.litRows(0).size() == 4);
    CHECK(col.litRows(1).size() == 2);
    CHECK(col.litRows(2).size() == 1);
    Fixture h;
    Frames full = Guitar(h, { { 0, 1000, "E2" } }, "E_CHOICE_StringAppearance=On", 19, 24, { 19 });
    CHECK(full.litRows(0).size() == 4);
}

TEST_CASE("Guitar show strings draws the idle strings", "[effects][timingdriven]") {
    Fixture f;
    Frames fr = Guitar(f, { { 0, 500, "G2" } }, "E_CHOICE_StringAppearance=On,E_CHECKBOX_ShowStrings=1", 19, 6, { 2, 15 });
    CHECK(fr.litRows(0).size() == 6);
    // the fretted string continues past the finger in its own colour
    CHECK(fr.litInRow(0, 5) == 19);
    CHECK(Same(fr.at(0, 0, 0), xlColor(255, 0, 255)));
    // with no note every string is still shown
    CHECK(fr.litRows(1).size() == 6);
}

TEST_CASE("Guitar wave appearance draws a thin moving wave", "[effects][timingdriven]") {
    Fixture f;
    Frames wave = Guitar(f, { { 0, 1000, "E2" } }, "E_CHOICE_StringAppearance=Wave", 40, 60, { 2, 3 });
    // one pixel per column for the wave (10 rows per string)
    int bottomHalf = 0;
    for (int x = 0; x < 40; ++x) {
        int n = 0;
        for (int y = 50; y < 60; ++y)
            if (wave.on(0, x, y)) ++n;
        CHECK(n <= 1);
        bottomHalf += n;
    }
    CHECK(bottomHalf > 0);
    CHECK(wave.f[0] != wave.f[1]);

    Fixture g;
    Frames vary = Guitar(g, { { 0, 1000, "A2" } }, "E_CHOICE_StringAppearance=Wave,E_CHECKBOX_VaryWaveLengthOnFret=0,E_CHECKBOX_Collapse=1,E_CHECKBOX_Fade=1,E_TEXTCTRL_StringWaveFactor=2.0,E_TEXTCTRL_BaseWaveFactor=3.0", 40, 12, { 2 });
    CHECK(vary.lit(0) > 0);
}

TEST_CASE("Guitar other instruments have their own string counts", "[effects][timingdriven]") {
    Fixture f;
    // bass: 4 strings, open low E1
    Frames bass = Guitar(f, { { 0, 1000, "E1" } }, "E_CHOICE_Guitar_Type=Bass Guitar,E_CHOICE_StringAppearance=On,E_CHECKBOX_ShowStrings=1", 19, 8, { 2 });
    // 2 rows per string: the played string fills its band, the others are single lines
    CHECK(bass.litRows(0).size() == 5);
    CHECK(bass.litInRow(0, 7) == 19);

    Fixture g;
    Frames banjo = Guitar(g, { { 0, 1000, "CG" } }, "E_CHOICE_Guitar_Type=Banjo,E_CHOICE_StringAppearance=On", 19, 10, { 2 });
    CHECK(banjo.lit(0) > 0);
    Fixture h;
    Frames violin = Guitar(h, { { 0, 1000, "A4" } }, "E_CHOICE_Guitar_Type=Violin,E_CHOICE_StringAppearance=On", 19, 4, { 2 });
    CHECK(violin.lit(0) > 0);
    // violin's lowest string is G3; a guitar's low E is unplayable
    Fixture i;
    Frames low = Guitar(i, { { 0, 1000, "E2" } }, "E_CHOICE_Guitar_Type=Violin,E_CHOICE_StringAppearance=On", 19, 4, { 2 });
    CHECK(low.lit(0) == 0);
}

TEST_CASE("Guitar picks hand positions that follow the previous note", "[effects][timingdriven]") {
    Fixture f;
    // a run of single notes on several strings: each is drawn on exactly one string
    std::vector<Mark> run = { { 0, 100, "A2" }, { 100, 200, "B2" }, { 200, 300, "C3" }, { 300, 400, "D3" }, { 400, 500, "E3" }, { 600, 700, "A3" }, { 700, 800, "B3 D4" } };
    Frames fr = Guitar(f, run, "E_CHOICE_StringAppearance=On,E_SLIDER_MaxFrets=12", 24, 6, { 1, 3, 5, 7, 9, 13, 15 });
    for (size_t i = 0; i < 6; ++i) {
        INFO("frame " << i);
        CHECK(fr.litRows(i).size() == 1);
    }
    CHECK(fr.litRows(6).size() == 2);
    // determinism
    Fixture g;
    Frames again = Guitar(g, run, "E_CHOICE_StringAppearance=On,E_SLIDER_MaxFrets=12", 24, 6, { 1, 3, 5, 7, 9, 13, 15 });
    CHECK(fr.f == again.f);
}

TEST_CASE("Guitar settings checks, renames and wave-factor migration", "[effects][timingdriven]") {
    Fixture f;
    f.AddTrack({ "N", { { { 0, 100, "E2" } } } });
    f.AddTrack({ "Empty", { { { 0, 100, "zz" } } } });
    RenderableEffect* fx = f.Fx("Guitar");
    Effect* e = f.Add("Guitar", "E_CHOICE_Guitar_MIDITrack_APPLYLAST=N", "", 1000);

    CHECK(HasError(fx->CheckEffectSettings(SettingsMap(), nullptr, f.model.get(), e, false), "needs a timing track"));
    SettingsMap ok;
    ok["E_CHOICE_Guitar_MIDITrack_APPLYLAST"] = "N";
    CHECK_FALSE(HasError(fx->CheckEffectSettings(ok, nullptr, f.model.get(), e, false), "ERR"));
    SettingsMap empty;
    empty["E_CHOICE_Guitar_MIDITrack_APPLYLAST"] = "Empty";
    CHECK(HasError(fx->CheckEffectSettings(empty, nullptr, f.model.get(), e, false), "has no notes"));
    SettingsMap gone;
    gone["E_CHOICE_Guitar_MIDITrack_APPLYLAST"] = "Gone";
    CHECK(HasError(fx->CheckEffectSettings(gone, nullptr, f.model.get(), e, false), "has no notes"));

    fx->RenameTimingTrack("X", "Y", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Guitar_MIDITrack_APPLYLAST", "") == "N");
    fx->RenameTimingTrack("N", "Notes", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Guitar_MIDITrack_APPLYLAST", "") == "Notes");

    Effect* old = f.Add("Guitar", "E_SLIDER_BaseWaveFactor=25,E_SLIDER_StringWaveFactor=5", "", 1000);
    REQUIRE(fx->needToAdjustSettings("2025.01"));
    CHECK_FALSE(fx->needToAdjustSettings("2026.06"));
    fx->adjustSettings("2025.01", old, false);
    CHECK(old->GetSettings().Get("E_TEXTCTRL_BaseWaveFactor", "") == "2.5");
    CHECK(old->GetSettings().Get("E_TEXTCTRL_StringWaveFactor", "") == "0.5");
    CHECK_FALSE(old->GetSettings().Contains("E_SLIDER_BaseWaveFactor"));

    Effect* both = f.Add("Guitar", "E_SLIDER_BaseWaveFactor=25,E_TEXTCTRL_BaseWaveFactor=4.0", "", 1000);
    fx->adjustSettings("2025.01", both, false);
    CHECK(both->GetSettings().Get("E_TEXTCTRL_BaseWaveFactor", "") == "4.0");
    CHECK_FALSE(both->GetSettings().Contains("E_SLIDER_BaseWaveFactor"));
}

TEST_CASE("Guitar bass chords use the bass fingering", "[.knownbug]") {
    // Guitar.json offers "Bass Guitar", but the chord lookups compare against "Bass",
    // so a bass track gets the guitar chord shapes (A major: open low string on guitar,
    // fret 5 on bass).
    Fixture f;
    Frames fr = Guitar(f, { { 0, 1000, "CA" } }, "E_CHOICE_Guitar_Type=Bass Guitar,E_CHOICE_StringAppearance=On", 19, 4, { 2 });
    CAPTURE(fr.litInRow(0, 3));
    CHECK(fr.litInRow(0, 3) == 14);
}

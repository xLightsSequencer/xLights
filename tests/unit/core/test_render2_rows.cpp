#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <thread>

#include "test_render_common.h"

using namespace renderhelp;

// Submodel, strand, node and Duplicate rows. BuildXsq only writes plain model
// layers, so these sequences are written by hand.

namespace {

using Rgb = Rendered::Rgb;
const Rgb kBlack{ 0, 0, 0 };
const Rgb kRed{ 255, 0, 0 };
const Rgb kGreen{ 0, 255, 0 };
const Rgb kBlue{ 0, 0, 255 };

struct XsqBuilder {
    std::vector<std::string> sets, pals;
    std::vector<std::string> elements;
    std::string bodies;

    int Index(std::vector<std::string>& v, const std::string& x) {
        for (size_t i = 0; i < v.size(); ++i) {
            if (v[i] == x) return (int)i;
        }
        v.push_back(x);
        return (int)v.size() - 1;
    }
    std::string E(const std::string& name, int start, int end, const std::string& color, const std::string& settings = "") {
        return "<Effect ref=\"" + std::to_string(Index(sets, settings)) + "\" name=\"" + name + "\" startTime=\"" + std::to_string(start) +
               "\" endTime=\"" + std::to_string(end) + "\" palette=\"" + std::to_string(Index(pals, Pal(color))) + "\"/>";
    }
    void Element(const std::string& name, const std::string& inner) {
        elements.push_back(name);
        bodies += "<Element type=\"model\" name=\"" + name + "\">" + inner + "</Element>";
    }
    std::string Build() const {
        std::string out = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
                          "<head><version>2024.01</version><mediaFile></mediaFile><sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>1.000</sequenceDuration></head><DisplayElements>";
        for (auto& e : elements) out += "<Element collapsed=\"0\" type=\"model\" name=\"" + e + "\" visible=\"1\"/>";
        out += "</DisplayElements><EffectDB>";
        for (auto& s : sets) out += "<Effect>" + XmlEscape(s) + "</Effect>";
        out += "</EffectDB><ColorPalettes>";
        for (auto& p : pals) out += "<ColorPalette>" + XmlEscape(p) + "</ColorPalette>";
        out += "</ColorPalettes><ElementEffects>" + bodies + "</ElementEffects></xsequence>";
        return out;
    }
};

// 2 strands of 4 nodes: strand 0 is nodes 0-3, strand 1 is nodes 4-7.
const std::string kMatrix2x4 = "<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"8\" StrandsPerString=\"2\" StringType=\"RGB Nodes\">"
                               "<subModel name=\"s\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>";

Show RenderXsq(const std::string& models, const XsqBuilder& b) {
    ShowSpec spec;
    spec.models = models;
    return Show(spec, b.Build());
}

} // namespace

TEST_CASE("Submodel effect lights only the submodel's nodes", "[render2][rows]") {
    XsqBuilder b;
    b.Element("M", "<EffectLayer/><SubModelEffectLayer name=\"s\">" + b.E("On", 0, 1000, "#FF0000") + "</SubModelEffectLayer>");
    Show show = RenderXsq(kMatrix2x4, b);
    REQUIRE(show.result.rendered);
    for (unsigned n = 0; n < 8; ++n) {
        INFO("node " << n);
        CHECK(show.result.rgb(5, n) == (n < 2 ? kRed : kBlack));
    }
}

TEST_CASE("Strand effect lights only that strand", "[render2][rows]") {
    XsqBuilder b;
    b.Element("M", "<EffectLayer/><Strand index=\"1\">" + b.E("On", 0, 1000, "#00FF00") + "</Strand>");
    Show show = RenderXsq(kMatrix2x4, b);
    REQUIRE(show.result.rendered);
    for (unsigned n = 0; n < 8; ++n) {
        INFO("node " << n);
        CHECK(show.result.rgb(5, n) == (n >= 4 ? kGreen : kBlack));
    }
}

TEST_CASE("Node effect lights a single node", "[render2][rows]") {
    XsqBuilder b;
    b.Element("M", "<EffectLayer/><Strand index=\"0\"><Node index=\"2\">" + b.E("On", 0, 1000, "#0000FF") + "</Node></Strand>");
    Show show = RenderXsq(kMatrix2x4, b);
    REQUIRE(show.result.rendered);
    int lit = 0;
    for (unsigned n = 0; n < 8; ++n) {
        if (show.result.rgb(5, n) != kBlack) {
            ++lit;
            CHECK(show.result.rgb(5, n) == kBlue);
        }
    }
    CHECK(lit == 1);
}

TEST_CASE("Submodel, strand and model effects combine on one model", "[render2][rows]") {
    XsqBuilder b;
    b.Element("M", "<EffectLayer>" + b.E("On", 0, 1000, "#0000FF") + "</EffectLayer>" +
                       "<SubModelEffectLayer name=\"s\">" + b.E("On", 0, 500, "#FF0000") + "</SubModelEffectLayer>" +
                       "<Strand index=\"1\">" + b.E("On", 500, 1000, "#00FF00") + "</Strand>");
    Show show = RenderXsq(kMatrix2x4, b);
    REQUIRE(show.result.rendered);
    // first half: submodel red over the model's blue
    CHECK(show.result.rgb(3, 0) == kRed);
    CHECK(show.result.rgb(3, 1) == kRed);
    CHECK(show.result.rgb(3, 2) == kBlue);
    CHECK(show.result.rgb(3, 5) == kBlue);
    // second half: strand 1 green, the rest blue
    CHECK(show.result.rgb(15, 0) == kBlue);
    CHECK(show.result.rgb(15, 5) == kGreen);
}

TEST_CASE("Submodel effect can be re-rendered on its own", "[render2][rows]") {
    XsqBuilder b;
    b.Element("M", "<EffectLayer/><SubModelEffectLayer name=\"s\">" + b.E("On", 0, 1000, "#FF0000") + "</SubModelEffectLayer>");
    Show show = RenderXsq(kMatrix2x4, b);
    REQUIRE(show.result.rendered);
    auto& sd = show.ctx->_seqData;
    for (unsigned f = 0; f < sd.NumFrames(); ++f) sd[f].Zero();
    show.ctx->RenderEffectForModel("M", 0, 1000, false);
    for (int i = 0; i < 1000 && !show.ctx->IsRenderDone(); ++i) std::this_thread::sleep_for(std::chrono::milliseconds(5));
    REQUIRE(show.ctx->IsRenderDone());
    show.Capture();
    CHECK(show.result.rgb(7, 0) == kRed);
    CHECK(show.result.rgb(7, 3) == kBlack);
}

// ---- Duplicate ----

namespace {

const std::string kTwoLines = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);

Rendered Duplicate(const std::string& dupSettings, const std::string& dupColor = "#00FF00", const std::string& srcSettings = "") {
    XsqBuilder b;
    b.Element("A", "<EffectLayer>" + b.E("On", 0, 1000, "#FF0000", srcSettings) + "</EffectLayer>");
    b.Element("B", "<EffectLayer>" + b.E("Duplicate", 0, 1000, dupColor, "E_CHOICE_Duplicate_Model=A,E_SPINCTRL_Duplicate_Layer=1" + dupSettings) + "</EffectLayer>");
    Show show = RenderXsq(kTwoLines, b);
    REQUIRE(show.result.rendered);
    return show.result;
}

} // namespace

TEST_CASE("Duplicate effect copies another model's effect", "[render2][rows][duplicate]") {
    Rendered r = Duplicate("");
    for (unsigned n = 0; n < 4; ++n) CHECK(r.rgb(5, n) == kRed);
}

TEST_CASE("Duplicate can override the palette", "[render2][rows][duplicate]") {
    Rendered r = Duplicate(",E_CHECKBOX_Duplicate_Override_Palette=1", "#0000FF");
    CHECK(r.rgb(5, 0) == kRed);
    CHECK(r.rgb(5, 2) == kBlue);
    CHECK(r.rgb(5, 3) == kBlue);
}

TEST_CASE("Duplicate can override the layer timing settings", "[render2][rows][duplicate]") {
    Rendered r = Duplicate(",E_CHECKBOX_Duplicate_Override_Timing=1,T_SLIDER_Brightness=50");
    CHECK(r.rgb(5, 0) == kRed);
    CHECK(r.isNear(5, 2, 127, 0, 0, 1));
    // without the override the copy keeps the source's own (full) brightness
    Rendered plain = Duplicate(",T_SLIDER_Brightness=50");
    CHECK(plain.rgb(5, 2) == kRed);
}

TEST_CASE("Duplicate can override the buffer settings", "[render2][rows][duplicate]") {
    Rendered r = Duplicate(",E_CHECKBOX_Duplicate_Override_Buffer=1,B_CUSTOM_SubBuffer=0x0x50x100");
    CHECK(r.rgb(5, 0) == kRed);
    CHECK(r.rgb(5, 1) == kRed);
    int lit = 0;
    for (unsigned n = 2; n < 4; ++n) {
        if (r.rgb(5, n) != kBlack) ++lit;
    }
    CHECK(lit == 1);
    // the source's own sub buffer is replaced
    Rendered src = Duplicate(",E_CHECKBOX_Duplicate_Override_Buffer=1", "#00FF00", "B_CUSTOM_SubBuffer=0x0x50x100");
    CHECK(src.rgb(5, 2) == kRed);
    CHECK(src.rgb(5, 3) == kRed);
}

TEST_CASE("Duplicate can override colour settings", "[render2][rows][duplicate]") {
    XsqBuilder b;
    b.Element("A", "<EffectLayer>" + b.E("On", 0, 1000, "#FF0000") + "</EffectLayer>");
    std::string pal = Pal("#00FF00", "", "C_SLIDER_Brightness=50");
    int pidx = b.Index(b.pals, pal);
    int sidx = b.Index(b.sets, "E_CHOICE_Duplicate_Model=A,E_SPINCTRL_Duplicate_Layer=1,E_CHECKBOX_Duplicate_Override_Color=1");
    b.Element("B", "<EffectLayer><Effect ref=\"" + std::to_string(sidx) + "\" name=\"Duplicate\" startTime=\"0\" endTime=\"1000\" palette=\"" + std::to_string(pidx) + "\"/></EffectLayer>");
    Show show = RenderXsq(kTwoLines, b);
    REQUIRE(show.result.rendered);
    CHECK(show.result.rgb(5, 0) == kRed);
    // the palette colour stays the source's, the brightness is the duplicate's
    CHECK(show.result.isNear(5, 2, 127, 0, 0, 1));
}

TEST_CASE("Duplicate of a missing model or layer renders nothing", "[render2][rows][duplicate]") {
    for (const char* set : { "E_CHOICE_Duplicate_Model=Nope,E_SPINCTRL_Duplicate_Layer=1", "E_CHOICE_Duplicate_Model=A,E_SPINCTRL_Duplicate_Layer=5" }) {
        INFO(set);
        XsqBuilder b;
        b.Element("A", "<EffectLayer>" + b.E("On", 0, 1000, "#FF0000") + "</EffectLayer>");
        b.Element("B", "<EffectLayer>" + b.E("Duplicate", 0, 1000, "#00FF00", set) + "</EffectLayer>");
        Show show = RenderXsq(kTwoLines, b);
        REQUIRE(show.result.rendered);
        CHECK(show.result.rgb(5, 0) == kRed);
        CHECK(show.result.rgb(5, 2) == kBlack);
    }
}

TEST_CASE("Duplicate of a duplicate is not followed", "[render2][rows][duplicate]") {
    XsqBuilder b;
    b.Element("A", "<EffectLayer>" + b.E("On", 0, 1000, "#FF0000") + "</EffectLayer>");
    b.Element("B", "<EffectLayer>" + b.E("Duplicate", 0, 1000, "#00FF00", "E_CHOICE_Duplicate_Model=A,E_SPINCTRL_Duplicate_Layer=1") + "</EffectLayer>");
    b.Element("C", "<EffectLayer>" + b.E("Duplicate", 0, 1000, "#00FF00", "E_CHOICE_Duplicate_Model=B,E_SPINCTRL_Duplicate_Layer=1") + "</EffectLayer>");
    Show show = RenderXsq(kTwoLines + SingleLine("C", 13, 2), b);
    REQUIRE(show.result.rendered);
    CHECK(show.result.rgb(5, 2) == kRed);
    CHECK(show.result.rgb(5, 4) == kBlack);
}

TEST_CASE("Canvas layer can pick which layers it starts from", "[render2][rows][canvas]") {
    // layer 0 is a canvas over two layers below; LayersSelected names them by index below the canvas
    auto render = [](const std::string& selected) {
        ShowSpec s;
        s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#000000"), "T_CHECKBOX_Canvas=1,T_CHOICE_LayerMethod=Additive,E_TEXTCTRL_On_Transparency=100" + selected));
        s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive"));
        s.effects.push_back(Eff("M", 2, "On", 0, 1000, Pal("#0000FF")));
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        return r.rgb(5, 0);
    };
    auto all = render("");
    auto one = render(",T_LayersSelected=0");
    CHECK(all != one);
}

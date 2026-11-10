#include <catch2/catch_test_macros.hpp>

#include <cstdio>

#include "test_render_common.h"

using namespace renderhelp;

// Canvas layers, dimming curves and per model group buffers.

namespace {

using Rgb = Rendered::Rgb;
const Rgb kRed{ 255, 0, 0 };
const Rgb kGreen{ 0, 255, 0 };
const Rgb kBlue{ 0, 0, 255 };

int Lit(const Rendered& r, unsigned frame, unsigned first, unsigned count) {
    int n = 0;
    for (unsigned i = first; i < first + count; ++i) {
        auto c = r.rgb(frame, i);
        if (c.r || c.g || c.b) ++n;
    }
    return n;
}

Rendered OneLayer(const std::string& models, const std::string& palette, const std::string& settings = "") {
    ShowSpec s;
    s.models = models;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, palette, settings));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    return r;
}

std::string Ramp(const std::string& id, double min, double max, double p1, double p2) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Active=TRUE|Id=ID_VALUECURVE_%s|Type=Ramp|Min=%.2f|Max=%.2f|P1=%.2f|P2=%.2f|RV=TRUE|", id.c_str(), min, max, p1, p2);
    return buf;
}

const std::string kGammaLine = "<model name=\"M\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"4\" StringType=\"RGB Nodes\">"
                               "<dimmingCurve><all gamma=\"2.2\" brightness=\"0\"/></dimmingCurve></model>";

ShowSpec TwoMemberGroup() {
    ShowSpec s;
    s.models = SingleLine("A", 1, 4) + SingleLine("B", 13, 4);
    s.groups = R"(<modelGroup name="G" models="A,B"/>)";
    s.modelElements = { "A", "B", "G" };
    return s;
}

} // namespace

TEST_CASE("Dimming curve gamma is applied to the output", "[render2][canvas][dimming]") {
    Rendered half = OneLayer(kGammaLine, Pal("#808080"));
    // 255 * (128 / 255)^2.2 = 56
    CHECK(half.isNear(5, 0, 56, 56, 56, 2));
    Rendered full = OneLayer(kGammaLine, Pal("#FF0000"));
    CHECK(full.rgb(5, 3) == kRed);
}

TEST_CASE("Dimming curve model blends with a group effect", "[render2][canvas][dimming]") {
    ShowSpec s;
    s.models = "<model name=\"A\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"2\" StringType=\"RGB Nodes\">"
               "<dimmingCurve><all gamma=\"2.2\" brightness=\"0\"/></dimmingCurve></model>" +
               SingleLine("B", 7, 2);
    s.groups = R"(<modelGroup name="G" models="A,B"/>)";
    s.modelElements = { "A", "B", "G" };
    s.effects.push_back(Eff("A", 0, "On", 0, 1000, Pal("#FF0000")));
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#0000FF"), "T_CHOICE_LayerMethod=Additive"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    // full scale channels survive the curve and its reverse unchanged
    CHECK(r.rgb(5, 2) == kBlue);
    CHECK(r.rgb(5, 0).b == 255);
}

TEST_CASE("Canvas on a sparse model fills the gaps from the layers below", "[render2][canvas]") {
    // four corner nodes in a 3x3 buffer: the other five pixels have no node
    ShowSpec s;
    s.models = "<model name=\"M\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"3\" CustomHeight=\"3\" CustomModel=\"1,,2;,,;3,,4\"/>";
    // the canvas layer draws nothing and blurs what it inherited; if the empty
    // pixels were left black the corners would come out darker
    s.effects.push_back(Eff("M", 0, "Off", 0, 1000, Pal("#000000"), "E_CHOICE_Off_Style=Transparent,T_CHECKBOX_Canvas=1,B_SLIDER_Blur=3"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned n = 0; n < 4; ++n) CHECK(r.rgb(5, n) == kBlue);
}

TEST_CASE("Canvas on a per model group layer keeps the layer below", "[render2][canvas][group]") {
    ShowSpec s = TwoMemberGroup();
    s.effects.push_back(Eff("G", 0, "Off", 0, 1000, Pal("#000000"), "E_CHOICE_Off_Style=Transparent,T_CHECKBOX_Canvas=1,B_CHOICE_BufferStyle=Per Model Default"));
    s.effects.push_back(Eff("G", 1, "On", 0, 1000, Pal("#00FF00")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned n = 0; n < 8; ++n) CHECK(r.rgb(5, n) == kGreen);
}

namespace {
std::string Flat(double v) {
    char buf[200];
    snprintf(buf, sizeof(buf), "Active=TRUE|Id=ID_VALUECURVE_SubBuffer|Type=Flat|Min=1.00|Max=200.00|P1=%.2f|RV=TRUE|", v);
    return buf;
}
} // namespace

TEST_CASE("A flat variable sub buffer matches the same static sub buffer", "[render2][canvas][subbuffer]") {
    Rendered fixed = OneLayer(Matrix("M", 1, 8, 8), Pal("#FF0000"), "B_CUSTOM_SubBuffer=0x0x50x100");
    Rendered curve = OneLayer(Matrix("M", 1, 8, 8), Pal("#FF0000"), "B_CUSTOM_SubBuffer=0x0x" + Flat(50) + "x100");
    CHECK(Lit(fixed, 5, 0, 64) == 32);
    CHECK((curve.data == fixed.data));
}

// A sub buffer on a Per Model group layer behaves differently depending on
// whether it is static or a value curve. Expected: a flat 50% curve gives the
// same output as a static 50% sub buffer. Actual: the static rect is applied to
// the group's combined line (member A fully lit, B dark) while with the curve
// every node of both members lights. SetLayerSettings applies the static rect
// to the "Single Line" group nodes; PrepareVariableSubBuffer rebuilds them with
// the Per Model style (src-core/render/PixelBuffer.cpp, SetLayerSettings vs
// PrepareVariableSubBuffer).
TEST_CASE("Per model group static and flat variable sub buffers agree", "[.knownbug][knownbug_permodel_variable_subbuffer]") {
    auto render = [](const std::string& sb) {
        ShowSpec s = TwoMemberGroup();
        s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#FF0000"), "B_CHOICE_BufferStyle=Per Model Default,B_CUSTOM_SubBuffer=" + sb));
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        return r;
    };
    Rendered fixed = render("0x0x50x100");
    Rendered curve = render("0x0x" + Flat(50) + "x100");
    CHECK(Lit(curve, 5, 0, 8) == Lit(fixed, 5, 0, 8));
    CHECK((curve.data == fixed.data));
}

TEST_CASE("Per model group with a variable sub buffer renders every member", "[render2][canvas][group]") {
    ShowSpec s = TwoMemberGroup();
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#FF0000"),
                            "B_CHOICE_BufferStyle=Per Model Default,B_CUSTOM_SubBuffer=0x0x" + Ramp("SubBuffer", 1, 200, 25, 100) + "x100"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(Lit(r, 19, 0, 8) == 8);
}

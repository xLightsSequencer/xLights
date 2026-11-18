#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <map>

#include "models/Model.h"
#include "models/Node.h"

#include "test_render_common.h"


using namespace renderhelp;

namespace {

using Rgb = Rendered::Rgb;
const Rgb kBlack{ 0, 0, 0 };
const Rgb kRed{ 255, 0, 0 };
const Rgb kBlue{ 0, 0, 255 };

bool IsLit(const Rgb& c) { return c.r || c.g || c.b; }

int Lit(const Rendered& r, unsigned frame, unsigned first, unsigned count) {
    int n = 0;
    for (unsigned i = first; i < first + count; ++i) {
        if (IsLit(r.rgb(frame, i))) ++n;
    }
    return n;
}

Rendered OneLayer(const std::string& models, const std::string& effect, const std::string& palette, const std::string& settings, int durationMs = 1000) {
    ShowSpec s;
    s.models = models;
    s.durationMs = durationMs;
    s.effects.push_back(Eff("M", 0, effect, 0, durationMs, palette, settings));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    return r;
}

// (x, y) in the model's default buffer -> node index
std::map<std::pair<int, int>, int> BufferMap(const std::string& models, int& w, int& h) {
    Show show(ShowSpec{ models, "", {}, 100, 50, { "M" } });
    REQUIRE(show.result.loaded);
    Model* m = show.ctx->AllModels["M"];
    REQUIRE(m != nullptr);
    std::vector<NodeBaseClassPtr> nodes;
    m->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    std::map<std::pair<int, int>, int> out;
    for (size_t i = 0; i < nodes.size(); ++i) {
        for (auto& c : nodes[i]->Coords) {
            out[{ c.bufX, c.bufY }] = (int)i;
            break;
        }
    }
    return out;
}

const std::string kLine8 = SingleLine("M", 1, 8);
const std::string kMatrix8 = Matrix("M", 1, 8, 8);
const std::string kRedBlue = Pal("#FF0000", "#0000FF");

std::string Ramp(const std::string& id, double min, double max, double p1, double p2) {
    char buf[256];
    snprintf(buf, sizeof(buf), "Active=TRUE|Id=ID_VALUECURVE_%s|Type=Ramp|Min=%.2f|Max=%.2f|P1=%.2f|P2=%.2f|RV=TRUE|", id.c_str(), min, max, p1, p2);
    return buf;
}

} // namespace

TEST_CASE("X rotation of 180 degrees mirrors the buffer about its centre", "[render2][layers][rotozoom]") {
    int w = 0, h = 0;
    auto map = BufferMap(kLine8, w, h);
    REQUIRE(w == 8);
    REQUIRE(h == 1);
    Rendered base = OneLayer(kLine8, "Bars", kRedBlue, "E_CHOICE_Bars_Direction=Left,E_SLIDER_Bars_BarCount=4");
    Rendered rot = OneLayer(kLine8, "Bars", kRedBlue, "E_CHOICE_Bars_Direction=Left,E_SLIDER_Bars_BarCount=4,B_SLIDER_XRotation=180");
    for (unsigned f : { 0u, 7u, 13u }) {
        INFO("frame " << f);
        // the pivot sits at x = 4, so x maps to 8 - x and column 0 receives nothing
        CHECK(rot.rgb(f, map[{ 0, 0 }]) == kBlack);
        for (int x = 1; x < 8; ++x) {
            INFO("x " << x);
            CHECK(rot.rgb(f, map[{ x, 0 }]) == base.rgb(f, map[{ 8 - x, 0 }]));
        }
    }
}

TEST_CASE("Y rotation of 180 degrees mirrors the buffer top to bottom", "[render2][layers][rotozoom]") {
    int w = 0, h = 0;
    auto map = BufferMap(kMatrix8, w, h);
    REQUIRE(w == 8);
    REQUIRE(h == 8);
    Rendered base = OneLayer(kMatrix8, "Bars", kRedBlue, "E_CHOICE_Bars_Direction=up,E_SLIDER_Bars_BarCount=4");
    Rendered rot = OneLayer(kMatrix8, "Bars", kRedBlue, "E_CHOICE_Bars_Direction=up,E_SLIDER_Bars_BarCount=4,B_SLIDER_YRotation=180");
    const unsigned f = 5;
    for (int x = 0; x < 8; ++x) {
        CHECK(rot.rgb(f, map[{ x, 0 }]) == kBlack);
        for (int y = 1; y < 8; ++y) {
            INFO("x " << x << " y " << y);
            CHECK(rot.rgb(f, map[{ x, y }]) == base.rgb(f, map[{ x, 8 - y }]));
        }
    }
}

TEST_CASE("X rotation of 360 degrees is the identity", "[render2][layers][rotozoom]") {
    Rendered base = OneLayer(kMatrix8, "Bars", kRedBlue, "");
    Rendered rot = OneLayer(kMatrix8, "Bars", kRedBlue, "B_SLIDER_XRotation=360,B_SLIDER_YRotation=360");
    CHECK(rot.data == base.data);
}

TEST_CASE("X rotation of 90 degrees collapses the buffer onto the pivot column", "[render2][layers][rotozoom]") {
    int w = 0, h = 0;
    auto map = BufferMap(kMatrix8, w, h);
    Rendered rot = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_SLIDER_XRotation=90");
    // sin(180 degrees) is ~0 so every column lands on x = 4
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            INFO("x " << x << " y " << y);
            CHECK(IsLit(rot.rgb(3, map[{ x, y }])) == (x == 4));
        }
    }
}

TEST_CASE("Zoom out shrinks a solid layer towards the pivot", "[render2][layers][rotozoom]") {
    Rendered half = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_SLIDER_Zoom=5");
    Rendered full = OneLayer(kMatrix8, "On", Pal("#FF0000"), "");
    CHECK(Lit(full, 3, 0, 64) == 64);
    int n = Lit(half, 3, 0, 64);
    CHECK(n > 0);
    CHECK(n < 64);
    // zooming in maps each source pixel forward, so at quality 1 a 2x zoom only
    // hits every other pixel of the centre half: 4x4 of 8x8
    Rendered in = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_SLIDER_Zoom=20");
    CHECK(Lit(in, 3, 0, 64) == 16);
    // quality 2 steps half pixels and fills the holes
    Rendered in2 = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_SLIDER_Zoom=20,B_SLIDER_ZoomQuality=2");
    CHECK(Lit(in2, 3, 0, 64) == 64);
}

TEST_CASE("Zoom quality and rotation order are honoured", "[render2][layers][rotozoom]") {
    Rendered a = OneLayer(kMatrix8, "Bars", kRedBlue, "B_SLIDER_Rotation=10,B_SLIDER_XRotation=40,B_SLIDER_YRotation=30,B_CHOICE_RZ_RotationOrder=X-Y-Z");
    Rendered b = OneLayer(kMatrix8, "Bars", kRedBlue, "B_SLIDER_Rotation=10,B_SLIDER_XRotation=40,B_SLIDER_YRotation=30,B_CHOICE_RZ_RotationOrder=Z-Y-X");
    Rendered q = OneLayer(kMatrix8, "Bars", kRedBlue, "B_SLIDER_Rotation=10,B_SLIDER_Zoom=15,B_SLIDER_ZoomQuality=3");
    Rendered q1 = OneLayer(kMatrix8, "Bars", kRedBlue, "B_SLIDER_Rotation=10,B_SLIDER_Zoom=15,B_SLIDER_ZoomQuality=1");
    CHECK(a.data[5] != b.data[5]);
    CHECK(Lit(q, 5, 0, 64) >= Lit(q1, 5, 0, 64));
}

TEST_CASE("Pivot point moves where a zoom collapses to", "[render2][layers][rotozoom]") {
    int w = 0, h = 0;
    auto map = BufferMap(kMatrix8, w, h);
    Rendered lowLeft = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_SLIDER_Zoom=5,B_SLIDER_PivotPointX=0,B_SLIDER_PivotPointY=0");
    Rendered upRight = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_SLIDER_Zoom=5,B_SLIDER_PivotPointX=100,B_SLIDER_PivotPointY=100");
    CHECK(IsLit(lowLeft.rgb(3, map[{ 0, 0 }])));
    CHECK_FALSE(IsLit(lowLeft.rgb(3, map[{ 7, 7 }])));
    CHECK(IsLit(upRight.rgb(3, map[{ 7, 7 }])));
    CHECK_FALSE(IsLit(upRight.rgb(3, map[{ 0, 0 }])));
}

TEST_CASE("Rotation value curves animate the rotozoom", "[render2][layers][rotozoom]") {
    const std::string vcs[] = {
        "B_VALUECURVE_Zoom=" + Ramp("Zoom", 0, 30, 1, 30),
        "B_SLIDER_Rotations=10,B_VALUECURVE_Rotation=" + Ramp("Rotation", 0, 100, 0, 100),
        "B_VALUECURVE_Rotations=" + Ramp("Rotations", 0, 200, 0, 20) + ",B_VALUECURVE_Rotation=" + Ramp("Rotation", 0, 100, 0, 100),
        "B_VALUECURVE_XRotation=" + Ramp("XRotation", 0, 360, 0, 180) + ",B_VALUECURVE_XPivot=" + Ramp("XPivot", 0, 100, 0, 100),
        "B_VALUECURVE_YRotation=" + Ramp("YRotation", 0, 360, 0, 180) + ",B_VALUECURVE_YPivot=" + Ramp("YPivot", 0, 100, 100, 0),
        "B_SLIDER_Zoom=5,B_VALUECURVE_PivotPointX=" + Ramp("PivotPointX", 0, 100, 0, 100) + ",B_VALUECURVE_PivotPointY=" + Ramp("PivotPointY", 0, 100, 0, 100),
    };
    for (const auto& set : vcs) {
        INFO(set);
        Rendered r = OneLayer(kMatrix8, "On", Pal("#FF0000"), set);
        bool changes = false;
        for (unsigned f = 1; f < r.frames; ++f) {
            if (r.data[f] != r.data[1]) changes = true;
        }
        CHECK(changes);
    }
}

TEST_CASE("Brightness value curve ramps the layer brightness", "[render2][layers][adjust]") {
    Rendered r = OneLayer(kLine8, "On", Pal("#FF0000", "", "C_VALUECURVE_Brightness=" + Ramp("Brightness", 0, 400, 0, 100)), "");
    CHECK(r.rgb(0, 0) == kBlack);
    CHECK(r.isNear(19, 0, 255, 0, 0, 1));
    for (unsigned f = 1; f < 20; ++f) CHECK(r.rgb(f, 0).r >= r.rgb(f - 1, 0).r);
    CHECK(r.isNear(10, 0, 134, 0, 0, 3));
}

TEST_CASE("Colour adjust value curves change the colour over time", "[render2][layers][adjust]") {
    Rendered hue = OneLayer(kLine8, "On", Pal("#FF0000", "", "C_VALUECURVE_Color_HueAdjust=" + Ramp("Color_HueAdjust", -100, 100, 0, 33)), "");
    CHECK(hue.rgb(0, 0) == kRed);
    // a third of the way round the wheel is green
    CHECK(hue.isNear(19, 0, 0, 255, 0, 8));

    Rendered sat = OneLayer(kLine8, "On", Pal("#FF0000", "", "C_VALUECURVE_Color_SaturationAdjust=" + Ramp("Color_SaturationAdjust", -100, 100, 0, -100)), "");
    CHECK(sat.rgb(0, 0) == kRed);
    auto end = sat.rgb(19, 0);
    CHECK(end.r == 255);
    CHECK(end.g == end.b);
    CHECK(end.g > 200);

    Rendered val = OneLayer(kLine8, "On", Pal("#FF0000", "", "C_VALUECURVE_Color_ValueAdjust=" + Ramp("Color_ValueAdjust", -100, 100, 0, -100)), "");
    CHECK(val.rgb(0, 0) == kRed);
    CHECK(val.rgb(19, 0) == kBlack);
}

TEST_CASE("Sparkle value curve controls how many sparkles appear", "[render2][layers][sparkle]") {
    Rendered none = OneLayer(kMatrix8, "On", Pal("#0000FF", "", "C_VALUECURVE_SparkleFrequency=" + Ramp("SparkleFrequency", 0, 200, 0, 0)), "");
    for (unsigned f = 0; f < 20; ++f) CHECK(none.rgb(f, 5) == kBlue);
    Rendered some = OneLayer(kMatrix8, "On", Pal("#0000FF", "", "C_VALUECURVE_SparkleFrequency=" + Ramp("SparkleFrequency", 0, 200, 0, 200)), "");
    int white = 0;
    for (unsigned f = 0; f < 20; ++f) {
        for (unsigned n = 0; n < 64; ++n) {
            if (some.rgb(f, n) == Rgb{ 255, 255, 255 }) ++white;
        }
    }
    CHECK(white > 0);
    // music sparkles need audio; without it the layer is plain
    Rendered music = OneLayer(kMatrix8, "On", Pal("#0000FF"), "B_CHECKBOX_MusicSparkles=1");
    for (unsigned n = 0; n < 64; ++n) CHECK(music.rgb(4, n) == kBlue);
}

TEST_CASE("Blur value curve blurs only once it rises", "[render2][layers][blur]") {
    Rendered r = OneLayer(kMatrix8, "Bars", kRedBlue, "B_VALUECURVE_Blur=" + Ramp("Blur", 1, 15, 1, 15));
    Rendered base = OneLayer(kMatrix8, "Bars", kRedBlue, "");
    CHECK(r.data[0] == base.data[0]);
    CHECK(r.data[19] != base.data[19]);
}

TEST_CASE("Heavy blur of a single lit column spreads it sideways", "[render2][layers][blur]") {
    int w = 0, h = 0;
    auto map = BufferMap(kMatrix8, w, h);
    // light only column 4 (12.5% wide sub buffer), then blur the layer
    Rendered sharp = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_CUSTOM_SubBuffer=50x0x62.5x100");
    Rendered blur = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_CUSTOM_SubBuffer=50x0x62.5x100,B_SLIDER_Blur=3");
    CHECK(sharp.rgb(3, map[{ 4, 4 }]) == kRed);
    CHECK(sharp.rgb(3, map[{ 3, 4 }]) == kBlack);
    // blurring happens inside the one column sub buffer, so nothing leaks out
    CHECK(blur.rgb(3, map[{ 3, 4 }]) == kBlack);
    CHECK(blur.rgb(3, map[{ 4, 4 }]).r > 0);
}

TEST_CASE("Variable sub buffer grows with its value curve", "[render2][layers][subbuffer]") {
    // right edge ramps from 10% to 100% of an 8 node line
    const std::string sb = "B_CUSTOM_SubBuffer=0x0x" + Ramp("SubBuffer", 1, 200, 10, 100) + "x100";
    Rendered r = OneLayer(kLine8, "On", Pal("#FF0000"), sb);
    std::vector<int> lit;
    for (unsigned f = 0; f < 20; ++f) lit.push_back(Lit(r, f, 0, 8));
    CHECK(lit.front() <= 2);
    CHECK(lit.back() == 8);
    for (size_t f = 1; f < lit.size(); ++f) CHECK(lit[f] >= lit[f - 1]);
}

TEST_CASE("Sub buffer offsets shift the lit region", "[render2][layers][subbuffer]") {
    int w = 0, h = 0;
    auto map = BufferMap(kLine8, w, h);
    // x1 x y1 x x2 x y2 x xoffset x yoffset
    Rendered r = OneLayer(kLine8, "On", Pal("#FF0000"), "B_CUSTOM_SubBuffer=0x0x25x100x50x0");
    for (int x = 0; x < 8; ++x) {
        INFO("x " << x);
        CHECK(IsLit(r.rgb(3, map[{ x, 0 }])) == (x >= 4 && x < 6));
    }
}

TEST_CASE("Buffer transforms rotate a single lit corner", "[render2][layers][transform]") {
    const std::string m = Matrix("M", 1, 8, 4);
    int w = 0, h = 0;
    auto map = BufferMap(m, w, h);
    REQUIRE(w * h == 32);
    REQUIRE(w != h);
    // light only the bottom left quarter by quarter cell of whatever buffer the
    // transform produces, then see where it lands on the model
    auto litCells = [&](const std::string& transform) {
        Rendered r = OneLayer(m, "On", Pal("#FF0000"), "B_CUSTOM_SubBuffer=0x0x25x25,B_CHOICE_BufferTransform=" + transform);
        std::vector<std::pair<int, int>> cells;
        for (auto& [xy, n] : map) {
            if (IsLit(r.rgb(3, n))) cells.push_back(xy);
        }
        std::sort(cells.begin(), cells.end());
        return cells;
    };
    const int qw = (int)std::lround(w * 0.25), qh = (int)std::lround(h * 0.25);
    auto none = litCells("None");
    CHECK((int)none.size() == qw * qh);
    for (auto& c : none) {
        CHECK(c.first < qw);
        CHECK(c.second < qh);
    }
    // rotating counter clockwise then flipping is a transpose, which keeps the origin corner
    auto transposed = litCells("Rotate CC 90 Flip Horizontal");
    CHECK_FALSE(transposed.empty());
    for (auto& c : transposed) {
        CHECK(c.first < qw);
        CHECK(c.second < qh);
    }
    for (const char* t : { "Rotate CC 90", "Rotate CW 90", "Rotate CW 90 Flip Horizontal" }) {
        INFO(t);
        auto cells = litCells(t);
        CHECK_FALSE(cells.empty());
        CHECK(cells != none);
    }
    auto r180 = litCells("Rotate 180");
    CHECK(r180.size() == none.size());
    for (auto& c : r180) {
        CHECK(c.first >= w - qw);
        CHECK(c.second >= h - qh);
    }
    auto fh = litCells("Flip Horizontal");
    CHECK(fh.size() == none.size());
    for (auto& c : fh) {
        CHECK(c.first >= w - qw);
        CHECK(c.second < qh);
    }
    auto fv = litCells("Flip Vertical");
    CHECK(fv.size() == none.size());
    for (auto& c : fv) {
        CHECK(c.first < qw);
        CHECK(c.second >= h - qh);
    }
}

TEST_CASE("Render disabled effect setting blanks its layer", "[render2][layers]") {
    ShowSpec s;
    s.models = kLine8;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "X_Effect_RenderDisabled=True"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 0) == kBlue);
}

TEST_CASE("Brightness level checkbox keeps full brightness colours", "[render2][layers][adjust]") {
    Rendered r = OneLayer(kLine8, "On", Pal("#FF0000"), "T_CHECKBOXBRIGHTNESSLEVEL=1,T_SLIDER_Brightness=100");
    CHECK(r.rgb(5, 0) == kRed);
}

TEST_CASE("Persistent layer keeps earlier frames under a later sparse effect", "[render2][layers][persistent]") {
    // A single strand chase leaves a trail only when the buffer is not cleared between frames.
    Rendered plain = OneLayer(kLine8, "SingleStrand", Pal("#FF0000"), "");
    Rendered keep = OneLayer(kLine8, "SingleStrand", Pal("#FF0000"), "B_CHECKBOX_OverlayBkg=1");
    int plainLit = 0, keepLit = 0;
    for (unsigned f = 0; f < 20; ++f) {
        plainLit += Lit(plain, f, 0, 8);
        keepLit += Lit(keep, f, 0, 8);
    }
    CHECK(keepLit > plainLit);
    for (unsigned f = 1; f < 20; ++f) CHECK(Lit(keep, f, 0, 8) >= Lit(keep, f - 1, 0, 8));
}

TEST_CASE("Canvas layer blends over the layers below", "[render2][layers][canvas]") {
    ShowSpec s;
    s.models = kLine8;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHECKBOX_Canvas=1,T_CHOICE_LayerMethod=Additive"));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    // the canvas layer starts from the blue below and paints red over it; the
    // layers it consumed are not blended in a second time
    CHECK(r.rgb(5, 0) == kRed);
    // without canvas the additive blend adds the blue
    s.effects[0].settings = "T_CHOICE_LayerMethod=Additive";
    Rendered plain = Render(s);
    CHECK(plain.rgb(5, 0) == Rgb{ 255, 0, 255 });
}

TEST_CASE("Shader style in transitions end on the full effect", "[render2][layers][transitions]") {
    for (const char* type : { "Fold", "Circular Swirl", "Bow Tie", "Zoom", "Doorway", "Blobs", "Pinwheel", "Star", "Swap", "Shatter", "Circles" }) {
        INFO(type);
        ShowSpec s;
        s.models = kMatrix8;
        s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"),
                                std::string("T_CHOICE_In_Transition_Type=") + type + ",T_TEXTCTRL_Fadein=0.5,T_CHOICE_LayerMethod=Normal"));
        s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        bool partial = false;
        for (unsigned f = 0; f < 10; ++f) {
            for (unsigned n = 0; n < 64; ++n) {
                if (r.rgb(f, n) != kRed) partial = true;
            }
        }
        // zooming a uniform colour leaves it uniform
        if (std::string(type) != "Zoom") CHECK(partial);
        for (unsigned f = 12; f < 20; ++f) {
            for (unsigned n = 0; n < 64; ++n) {
                INFO("frame " << f << " node " << n);
                CHECK(r.rgb(f, n) == kRed);
            }
        }
    }
}

TEST_CASE("Shader style out transitions start on the full effect", "[render2][layers][transitions]") {
    for (const char* type : { "Fold", "Circular Swirl", "Bow Tie", "Zoom", "Doorway", "Blobs", "Pinwheel", "Star", "Swap", "Shatter", "Circles", "From Middle", "Square Explode", "Circle Explode", "Blinds", "Blend", "Slide Checks", "Slide Bars" }) {
        INFO(type);
        ShowSpec s;
        s.models = kMatrix8;
        s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"),
                                std::string("T_CHOICE_Out_Transition_Type=") + type + ",T_TEXTCTRL_Fadeout=0.5,T_CHOICE_LayerMethod=Normal,T_CHECKBOX_Out_Transition_Reverse=1,T_SLIDER_Out_Transition_Blur=3"));
        s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        for (unsigned f = 0; f < 8; ++f) {
            for (unsigned n = 0; n < 64; ++n) {
                INFO("frame " << f << " node " << n);
                CHECK(r.rgb(f, n) == kRed);
            }
        }
        bool partial = false;
        for (unsigned f = 10; f < 20; ++f) {
            for (unsigned n = 0; n < 64; ++n) {
                if (r.rgb(f, n) != kRed) partial = true;
            }
        }
        if (std::string(type) != "Zoom") CHECK(partial);
    }
}

TEST_CASE("Transition adjust value curve and blur are accepted", "[render2][layers][transitions]") {
    ShowSpec s;
    s.models = kMatrix8;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000"),
                            "T_CHOICE_In_Transition_Type=Wipe,T_TEXTCTRL_Fadein=0.5,T_SLIDER_In_Transition_Blur=5,T_VALUECURVE_In_Transition_Adjust=" + Ramp("In_Transition_Adjust", 0, 100, 0, 100)));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(Lit(r, 0, 0, 64) < 64);
    CHECK(Lit(r, 15, 0, 64) == 64);
    for (unsigned n = 0; n < 64; ++n) CHECK(r.rgb(15, n) == kRed);
}

// ---- model group buffer styles ----

namespace {

ShowSpec GroupSpec(const std::string& style, const std::string& settings, const std::string& layout = "") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 4) + SingleLine("B", 13, 4);
    s.groups = "<modelGroup name=\"G\" models=\"A,B\"" + (layout.empty() ? std::string() : " layout=\"" + layout + "\"") + "/>";
    s.modelElements = { "A", "B", "G" };
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#FF0000"), (style.empty() ? std::string() : "B_CHOICE_BufferStyle=" + style + ",") + settings));
    return s;
}

} // namespace

TEST_CASE("Per model group styles run the effect separately on each member", "[render2][layers][group]") {
    // a single strand chase: per model styles chase along each member at once,
    // a combined buffer chases along the whole group
    for (const char* style : { "Per Model Default", "Per Model Single Line", "Per Model Default Deep", "Per Model Horizontal Per Strand" }) {
        INFO(style);
        ShowSpec s = GroupSpec(style, "");
        s.effects.back().name = "SingleStrand";
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        int lit = 0;
        for (unsigned f = 0; f < 20; ++f) {
            for (unsigned i = 0; i < 4; ++i) {
                INFO("frame " << f << " node " << i);
                CHECK(r.rgb(f, i) == r.rgb(f, 4 + i));
                if (IsLit(r.rgb(f, i))) ++lit;
            }
        }
        CHECK(lit > 0);
    }
    for (const char* style : { "Horizontal Per Model", "Single Line" }) {
        INFO(style);
        ShowSpec s = GroupSpec(style, "");
        s.effects.back().name = "SingleStrand";
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        bool aOnly = false, bOnly = false;
        for (unsigned f = 0; f < 20; ++f) {
            int a = Lit(r, f, 0, 4), b = Lit(r, f, 4, 4);
            if (a && !b) aOnly = true;
            if (b && !a) bOnly = true;
        }
        CHECK(aOnly);
        CHECK(bOnly);
    }
}

TEST_CASE("Per model group styles light every member fully", "[render2][layers][group]") {
    for (const char* style : { "Per Model Default", "Per Model Per Preview", "Per Model Single Line", "Per Model Default Deep",
                               "Per Model Vertical Per Strand", "Per Model Horizontal Per Strand", "Horizontal Per Model/Strand",
                               "Vertical Per Model/Strand", "Overlay - Centered", "Overlay - Scaled", "Per Preview", "Single Line", "Default" }) {
        INFO(style);
        Rendered r = Render(GroupSpec(style, ""));
        REQUIRE(r.rendered);
        for (unsigned n = 0; n < 8; ++n) CHECK(r.rgb(5, n) == kRed);
    }
}

TEST_CASE("Group default buffer style can be a per model style", "[render2][layers][group]") {
    // layout picks the group's default; an effect left on Default follows it
    ShowSpec s = GroupSpec("", "", "Per Model Default");
    s.effects.back().name = "SingleStrand";
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f < 20; ++f) {
        for (unsigned i = 0; i < 4; ++i) CHECK(r.rgb(f, i) == r.rgb(f, 4 + i));
    }
}

TEST_CASE("Per model buffers apply layer adjustments to every member", "[render2][layers][group]") {
    Rendered r = Render(GroupSpec("Per Model Default", "T_SLIDER_Brightness=50,B_SLIDER_Rotation=0"));
    REQUIRE(r.rendered);
    for (unsigned n = 0; n < 8; ++n) CHECK(r.isNear(5, n, 127, 0, 0, 1));
}

TEST_CASE("Per Preview group buffer with a 3D camera still renders", "[render2][layers][group]") {
    Rendered r = Render(GroupSpec("Per Preview", "B_CHOICE_PerPreviewCamera=3D"));
    REQUIRE(r.rendered);
    for (unsigned n = 0; n < 8; ++n) CHECK(r.rgb(5, n) == kRed);
}

TEST_CASE("Buffer stagger is accepted on a matrix", "[render2][layers]") {
    Rendered r = OneLayer(kMatrix8, "On", Pal("#FF0000"), "B_CHOICE_BufferStyle=Horizontal Per Strand,B_SPINCTRL_BufferStagger=2");
    CHECK(Lit(r, 4, 0, 64) == 64);
}

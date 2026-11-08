#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "media/AudioManager.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "utils/UtilClasses.h"

// Coverage of the classic settings-only CPU effects: Ripple, Meteors, Liquid,
// Snowflakes, Tendril, Bars and Fill.  The buffer is driven directly (Clear,
// SetState, Render) the way RenderEngine drives it, with no pixel-buffer
// layering or GPU work, so every case runs on the CPU path on every platform.

namespace {

const char* kRed = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";
const char* kRedBlue = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#0000FF,C_CHECKBOX_Palette2=1";
const char* kRGB = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1,"
                   "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1";
// A colour curve that varies left to right makes the palette "spatial".
const char* kSpatial = "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Timecurve=1|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF;|,"
                       "C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1";

using Pixels = std::vector<xlColor>;

bool Lit(const xlColor& c) {
    return c.alpha != 0 && (c.red || c.green || c.blue);
}

bool SameRgb(const xlColor& a, const xlColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) {
            if (Lit(c)) ++n;
        }
        return n;
    }
    int totalLit() const {
        int n = 0;
        for (size_t i = 0; i < f.size(); ++i) n += lit(i);
        return n;
    }
    bool operator==(const Frames& o) const { return w == o.w && h == o.h && f == o.f; }
    bool sameFrame(size_t a, size_t b) const { return f[a] == f[b]; }
    // mean x / y of the lit pixels; false when nothing is lit
    bool centroid(size_t frame, double& cx, double& cy) const {
        double sx = 0, sy = 0;
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (Lit(at(frame, x, y))) {
                    sx += x;
                    sy += y;
                    ++n;
                }
            }
        }
        if (n == 0) return false;
        cx = sx / n;
        cy = sy / n;
        return true;
    }
    double meanDistFrom(size_t frame, double cx, double cy) const {
        double s = 0;
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (Lit(at(frame, x, y))) {
                    s += std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
                    ++n;
                }
            }
        }
        return n == 0 ? -1.0 : s / n;
    }
    // every lit pixel matches one of the given colours (rgb only)
    bool onlyColours(const std::vector<xlColor>& allowed) const {
        for (const auto& px : f) {
            for (const auto& c : px) {
                if (!Lit(c)) continue;
                bool ok = false;
                for (const auto& a : allowed) {
                    if (SameRgb(c, a)) ok = true;
                }
                if (!ok) return false;
            }
        }
        return true;
    }
    // every lit pixel is a pure shade of red (no green or blue)
    bool onlyRedShades() const {
        for (const auto& px : f) {
            for (const auto& c : px) {
                if (Lit(c) && (c.green != 0 || c.blue != 0)) return false;
            }
        }
        return true;
    }
    // every lit pixel is red, possibly whitened (green == blue <= red)
    bool onlyRedTints() const {
        for (const auto& px : f) {
            for (const auto& c : px) {
                if (Lit(c) && (c.green != c.blue || c.green > c.red)) return false;
            }
        }
        return true;
    }
    // number of lit pixels in frame b that were lit in frame a at (x+dx, y+dy):
    // a high score for (0,+1) means things moved down by one row.
    int shiftScore(size_t a, size_t b, int dx, int dy) const {
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int sx = x + dx;
                int sy = y + dy;
                if (sx < 0 || sy < 0 || sx >= w || sy >= h) continue;
                if (Lit(at(b, x, y)) && Lit(at(a, sx, sy))) ++n;
            }
        }
        return n;
    }
    // summed over consecutive frame pairs and shifts 1..maxStep along (dx,dy)
    int motionScore(int dx, int dy, int maxStep = 3, size_t first = 1) const {
        int n = 0;
        for (size_t i = first; i + 1 < f.size(); ++i) {
            for (int s = 1; s <= maxStep; ++s) {
                n += shiftScore(i, i + 1, dx * s, dy * s);
            }
        }
        return n;
    }
    bool rowUniform(size_t frame) const {
        for (int y = 0; y < h; ++y)
            for (int x = 1; x < w; ++x)
                if (!SameRgb(at(frame, x, y), at(frame, 0, y))) return false;
        return true;
    }
    bool colUniform(size_t frame) const {
        for (int x = 0; x < w; ++x)
            for (int y = 1; y < h; ++y)
                if (!SameRgb(at(frame, x, y), at(frame, x, 0))) return false;
        return true;
    }
    int litInRows(size_t frame, int y0, int y1) const {
        int n = 0;
        for (int y = y0; y <= y1; ++y)
            for (int x = 0; x < w; ++x)
                if (Lit(at(frame, x, y))) ++n;
        return n;
    }
    int litInCols(size_t frame, int x0, int x1) const {
        int n = 0;
        for (int y = 0; y < h; ++y)
            for (int x = x0; x <= x1; ++x)
                if (Lit(at(frame, x, y))) ++n;
        return n;
    }
};

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

struct Opts {
    int endMs = 1000;
    bool persistState = true;
    AudioManager* media = nullptr;
    bool allowAlpha = false;
};

// A windowless show with one model; the buffer is driven directly.
struct Fixture {
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> model;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;
    Effect* last = nullptr;

    explicit Fixture(const std::string& modelType = "Single Line") {
        model.reset(ctx.AllModels.CreateDefaultModel(modelType));
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    EffectLayer* AddTiming(const std::string& name, const std::vector<std::tuple<int, int, std::string>>& marks) {
        Element* t = ctx._sequenceElements.AddElement(name, "timing", true, false, true, false, false);
        EffectLayer* tl = t->AddEffectLayer();
        for (const auto& [s, e, label] : marks) {
            tl->AddEffect(0, label, "", "", s, e, 0, false);
        }
        return tl;
    }

    RenderableEffect* Fx(const std::string& name) {
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);
        return fx;
    }

    Frames Render(const std::string& name, const std::string& settings, const std::string& palette,
                  int w, int h, const std::vector<int>& periods, const Opts& o = Opts()) {
        Effect* e = layer->AddEffect(0, name, settings, palette, 0, o.endMs, 0, false);
        REQUIRE(e != nullptr);
        last = e;
        RenderableEffect* fx = Fx(name);

        RenderBuffer rb(&ctx, nullptr, model.get());
        rb.InitBuffer(h, w, "None");
        rb.allowAlpha = o.allowAlpha;
        rb._mediaOverride = o.media;
        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        if (colors.empty()) {
            colors.push_back(xlWHITE);
            curves.push_back(ColorCurve());
        }
        rb.SetPalette(colors, curves);
        rb.SetFrameTimeInMs(50);
        rb.SetEffectDuration(0, o.endMs);

        SettingsMap renderSettings;
        e->CopySettingsMap(renderSettings, true);

        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            rb.SetState(p, first || !o.persistState);
            first = false;
            fx->Render(e, renderSettings, rb);
            Pixels px;
            if (w > 0 && h > 0) {
                px.assign(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
            }
            out.f.push_back(std::move(px));
        }
        return out;
    }

    // The frame-parallel draw pass: advance a Snapshottable effect serially on
    // one buffer, then draw every captured snapshot on its own fresh buffer
    // (whose temp layer has never been touched), as the clone buffers do.
    Frames RenderSnapshotsOnFreshBuffers(const std::string& name, const std::string& settings, const std::string& palette,
                                         int w, int h, const std::vector<int>& periods, const Opts& o = Opts()) {
        Effect* e = layer->AddEffect(0, name, settings, palette, 0, o.endMs, 0, false);
        REQUIRE(e != nullptr);
        last = e;
        RenderableEffect* fx = Fx(name);

        xlColorVector colors;
        xlColorCurveVector curves;
        e->CopyPalette(colors, curves);
        auto makeBuffer = [&](RenderBuffer& rb) {
            rb.InitBuffer(h, w, "None");
            rb.SetPalette(colors, curves);
            rb.SetFrameTimeInMs(50);
            rb.SetEffectDuration(0, o.endMs);
        };
        SettingsMap renderSettings;
        e->CopySettingsMap(renderSettings, true);

        RenderBuffer advance(&ctx, nullptr, model.get());
        makeBuffer(advance);
        std::vector<std::unique_ptr<EffectFrameState>> snaps;
        bool first = true;
        for (int p : periods) {
            advance.Clear();
            advance.SetState(p, first);
            first = false;
            snaps.push_back(fx->AdvanceState(e, renderSettings, advance));
            REQUIRE(snaps.back() != nullptr);
        }

        Frames out;
        out.w = w;
        out.h = h;
        for (size_t i = 0; i < periods.size(); ++i) {
            RenderBuffer draw(&ctx, nullptr, model.get());
            makeBuffer(draw);
            draw.Clear();
            draw.SetState(periods[i], false);
            draw.pendingSnapshot = snaps[i].get();
            fx->Render(e, renderSettings, draw);
            draw.pendingSnapshot = nullptr;
            out.f.emplace_back(draw.GetPixels(), draw.GetPixels() + (size_t)w * h);
        }
        return out;
    }
};

// ---- synthetic audio --------------------------------------------------------

void Put16(std::string& s, uint16_t v) {
    s.push_back(char(v & 0xFF));
    s.push_back(char((v >> 8) & 0xFF));
}
void Put32(std::string& s, uint32_t v) {
    for (int i = 0; i < 4; i++) s.push_back(char((v >> (8 * i)) & 0xFF));
}

std::string MakeWav(int sampleRate, const std::vector<int16_t>& samples) {
    std::string s;
    uint32_t dataBytes = uint32_t(samples.size() * 2);
    s += "RIFF";
    Put32(s, 36 + dataBytes);
    s += "WAVEfmt ";
    Put32(s, 16);
    Put16(s, 1);
    Put16(s, 1);
    Put32(s, uint32_t(sampleRate));
    Put32(s, uint32_t(sampleRate * 2));
    Put16(s, 2);
    Put16(s, 16);
    s += "data";
    Put32(s, dataBytes);
    for (int16_t v : samples) Put16(s, uint16_t(v));
    return s;
}

// a loud 440 Hz tone, 2 seconds long
struct LoudAudio {
    TempDir td;
    std::unique_ptr<AudioManager> am;
    LoudAudio() {
        const int sr = 22050;
        std::vector<int16_t> v(size_t(sr * 2));
        for (size_t i = 0; i < v.size(); i++) {
            v[i] = int16_t(std::lround(0.9 * 32767.0 * std::sin(2.0 * 3.14159265358979 * 440.0 * double(i) / sr)));
        }
        am = std::make_unique<AudioManager>(td.Write("tone.wav", MakeWav(sr, v)), 50);
    }
};

} // namespace

// =============================================================================
// Bars
// =============================================================================

namespace {
// shift d (0..n-1) such that b[i] == a[(i + d) % n] along a line, or -1
int CyclicShift(const std::vector<xlColor>& a, const std::vector<xlColor>& b) {
    int n = (int)a.size();
    for (int d = 0; d < n; ++d) {
        bool ok = true;
        for (int i = 0; i < n && ok; ++i) {
            ok = SameRgb(b[i], a[(i + d) % n]);
        }
        if (ok) return d;
    }
    return -1;
}
std::vector<xlColor> Row(const Frames& fr, size_t f, int y) {
    std::vector<xlColor> r;
    for (int x = 0; x < fr.w; ++x) r.push_back(fr.at(f, x, y));
    return r;
}
std::vector<xlColor> Col(const Frames& fr, size_t f, int x) {
    std::vector<xlColor> r;
    for (int y = 0; y < fr.h; ++y) r.push_back(fr.at(f, x, y));
    return r;
}
} // namespace

TEST_CASE("Bars Right and Left scroll in opposite directions", "[effects][classic1]") {
    Fixture f;
    Frames left = f.Render("Bars", "E_CHOICE_Bars_Direction=Left", kRedBlue, 20, 3, Range(0, 6));
    Frames right = f.Render("Bars", "E_CHOICE_Bars_Direction=Right", kRedBlue, 20, 3, Range(0, 6));
    for (size_t i = 0; i < left.f.size(); ++i) {
        CHECK(left.colUniform(i));
        CHECK(right.colUniform(i));
    }
    std::set<int> ls, rs;
    for (size_t i = 0; i + 1 < left.f.size(); ++i) {
        ls.insert(CyclicShift(Row(left, i, 0), Row(left, i + 1, 0)));
        rs.insert(CyclicShift(Row(right, i, 0), Row(right, i + 1, 0)));
    }
    // one block (2 colours x 10 px) moves one pixel per frame over a 20 frame effect
    CHECK(ls == std::set<int>{ 1 });
    CHECK(rs == std::set<int>{ 19 });
}

TEST_CASE("Bars up and down scroll in opposite directions", "[effects][classic1]") {
    Fixture f;
    Frames up = f.Render("Bars", "E_CHOICE_Bars_Direction=up", kRedBlue, 3, 20, Range(0, 6));
    Frames down = f.Render("Bars", "E_CHOICE_Bars_Direction=down", kRedBlue, 3, 20, Range(0, 6));
    std::set<int> us, ds;
    for (size_t i = 0; i + 1 < up.f.size(); ++i) {
        CHECK(up.rowUniform(i));
        CHECK(down.rowUniform(i));
        us.insert(CyclicShift(Col(up, i, 0), Col(up, i + 1, 0)));
        ds.insert(CyclicShift(Col(down, i, 0), Col(down, i + 1, 0)));
    }
    CHECK(us.size() == 1);
    CHECK(ds.size() == 1);
    CHECK(*us.begin() > 0);
    CHECK(*ds.begin() > 0);
    CHECK(*us.begin() + *ds.begin() == 20);
}

TEST_CASE("Bars expand and compress are mirror symmetric about the centre", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "expand", "compress" }) {
        INFO(dir);
        Frames v = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=") + dir, kRedBlue, 2, 20, { 0, 3, 7 });
        Frames hz = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=H-") + dir, kRedBlue, 20, 2, { 0, 3, 7 });
        for (size_t i = 0; i < v.f.size(); ++i) {
            CHECK(v.rowUniform(i));
            CHECK(hz.colUniform(i));
            CHECK(v.onlyColours({ xlRED, xlBLUE }));
            int mirroredV = 0, mirroredH = 0;
            for (int y = 1; y < 20; ++y) {
                // the centre is line 10: lines 10+k and 10-k pair up
                if (SameRgb(v.at(i, 0, y), v.at(i, 0, 20 - y))) ++mirroredV;
                if (SameRgb(hz.at(i, y, 0), hz.at(i, 20 - y, 0))) ++mirroredH;
            }
            CHECK(mirroredV == 19);
            CHECK(mirroredH == 19);
        }
    }
    // a centre offset breaks the symmetry
    Frames off = f.Render("Bars", "E_CHOICE_Bars_Direction=expand,E_SLIDER_Bars_Center=50", kRedBlue, 2, 20, { 3 });
    Frames on = f.Render("Bars", "E_CHOICE_Bars_Direction=expand", kRedBlue, 2, 20, { 3 });
    CHECK_FALSE(off == on);
}

TEST_CASE("Bars alternate directions step a whole bar at a time", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "Alternate Up", "Alternate Down", "Alternate Left", "Alternate Right" }) {
        INFO(dir);
        bool vertical = std::string(dir) == "Alternate Up" || std::string(dir) == "Alternate Down";
        int w = vertical ? 2 : 20;
        int h = vertical ? 20 : 2;
        Frames fr = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=") + dir, kRedBlue, w, h, Range(0, 19));
        CHECK(fr.onlyColours({ xlRED, xlBLUE }));
        // two bars of ten pixels each: the picture only changes at the half way point
        int changes = 0;
        for (size_t i = 0; i + 1 < fr.f.size(); ++i) {
            CHECK((vertical ? fr.rowUniform(i) : fr.colUniform(i)));
            if (!fr.sameFrame(i, i + 1)) {
                ++changes;
            }
        }
        CHECK(changes == 2);
    }
}

TEST_CASE("Bars highlight adds white edges and can use the first colour instead", "[effects][classic1]") {
    Fixture f;
    Frames plain = f.Render("Bars", "E_CHOICE_Bars_Direction=up", kRedBlue, 2, 20, { 0 });
    Frames hl = f.Render("Bars", "E_CHOICE_Bars_Direction=up,E_CHECKBOX_Bars_Highlight=1", kRedBlue, 2, 20, { 0 });
    auto count = [](const Frames& fr, const xlColor& c) {
        int n = 0;
        for (const auto& p : fr.f[0]) n += SameRgb(p, c) ? 1 : 0;
        return n;
    };
    CHECK(count(plain, xlWHITE) == 0);
    CHECK(count(hl, xlWHITE) > 0);

    // first colour (green) becomes the highlight; bars are red/blue from colours 2 and 3
    const char* pal = "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#FF0000,C_CHECKBOX_Palette2=1,"
                      "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1";
    for (const char* dir : { "up", "Custom" }) {
        INFO(dir);
        Frames fc = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=") + dir +
                                         ",E_CHECKBOX_Bars_Highlight=1,E_CHECKBOX_Bars_UseFirstColorForHighlight=1",
                             pal, 2, 20, { 0 });
        CHECK(count(fc, xlWHITE) == 0);
        CHECK(count(fc, xlGREEN) > 0);
        CHECK(count(fc, xlRED) > 0);
        CHECK(count(fc, xlBLUE) > 0);
    }
    // with a single colour there is nothing to spare for the highlight, so it stays white
    Frames one = f.Render("Bars", "E_CHOICE_Bars_Direction=up,E_CHECKBOX_Bars_Highlight=1,E_CHECKBOX_Bars_UseFirstColorForHighlight=1", kRed, 2, 20, { 0 });
    CHECK(count(one, xlWHITE) > 0);
}

// the CPU paths (custom directions, spatial palettes) draw the highlight in the first colour, as the SIMD path does
TEST_CASE("Bars custom horizontal and vertical honour use first colour for highlight", "[effects][classic1]") {
    Fixture f;
    const char* pal = "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#FF0000,C_CHECKBOX_Palette2=1,"
                      "C_BUTTON_Palette3=#0000FF,C_CHECKBOX_Palette3=1";
    for (const char* dir : { "Custom Horz", "Custom Vert" }) {
        INFO(dir);
        Frames fc = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=") + dir +
                                         ",E_CHECKBOX_Bars_Highlight=1,E_CHECKBOX_Bars_UseFirstColorForHighlight=1",
                             pal, 20, 20, { 0 });
        CHECK(fc.onlyColours({ xlGREEN, xlRED, xlBLUE }));
    }
}

TEST_CASE("Bars 3D darkens along each bar and gradient blends between colours", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "up", "Left", "Custom Horz", "Custom" }) {
        INFO(dir);
        std::string base = std::string("E_CHOICE_Bars_Direction=") + dir;
        Frames plain = f.Render("Bars", base, kRedBlue, 20, 20, { 2 });
        Frames d3 = f.Render("Bars", base + ",E_CHECKBOX_Bars_3D=1", kRedBlue, 20, 20, { 2 });
        Frames grad = f.Render("Bars", base + ",E_CHECKBOX_Bars_Gradient=1", kRedBlue, 20, 20, { 2 });
        CHECK(plain.onlyColours({ xlRED, xlBLUE }));
        CHECK_FALSE(d3.onlyColours({ xlRED, xlBLUE }));
        CHECK_FALSE(grad.onlyColours({ xlRED, xlBLUE }));
        // 3D only scales brightness: every pixel stays pure red or pure blue in hue
        for (const auto& c : d3.f[0]) {
            CHECK((c.green == 0 && (c.red == 0 || c.blue == 0)));
        }
        // the gradient mixes red and blue into one pixel somewhere
        bool mixed = false;
        for (const auto& c : grad.f[0]) {
            if (c.red > 0 && c.blue > 0) mixed = true;
        }
        CHECK(mixed);
    }
}

TEST_CASE("Bars custom angle orients the stripes", "[effects][classic1]") {
    Fixture f;
    Frames a0 = f.Render("Bars", "E_CHOICE_Bars_Direction=Custom,E_SLIDER_Bars_Angle=0", kRedBlue, 16, 16, { 0, 5 });
    Frames a90 = f.Render("Bars", "E_CHOICE_Bars_Direction=Custom,E_SLIDER_Bars_Angle=90", kRedBlue, 16, 16, { 0, 5 });
    Frames a45 = f.Render("Bars", "E_CHOICE_Bars_Direction=Custom,E_SLIDER_Bars_Angle=45", kRedBlue, 16, 16, { 0, 5 });
    for (size_t i = 0; i < 2; ++i) {
        // the two axis-aligned angles give stripes of opposite orientation
        CHECK(a0.colUniform(i) != a0.rowUniform(i));
        CHECK(a90.colUniform(i) != a90.rowUniform(i));
        CHECK(a0.colUniform(i) != a90.colUniform(i));
        CHECK_FALSE(a45.colUniform(i));
        CHECK_FALSE(a45.rowUniform(i));
        CHECK(a45.lit(i) == 256);
    }
    CHECK_FALSE(a0.sameFrame(0, 1));
}

TEST_CASE("Bars spatial palettes take the CPU path for every direction", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "up", "down", "expand", "compress", "Left", "Right", "H-expand", "H-compress",
                             "Alternate Up", "Alternate Down", "Alternate Left", "Alternate Right", "Custom Horz", "Custom Vert", "Custom" }) {
        INFO(dir);
        for (const char* opt : { "", ",E_CHECKBOX_Bars_Highlight=1", ",E_CHECKBOX_Bars_3D=1", ",E_CHECKBOX_Bars_Gradient=1",
                                 ",E_CHECKBOX_Bars_Highlight=1,E_CHECKBOX_Bars_UseFirstColorForHighlight=1" }) {
            INFO(opt);
            Frames fr = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=") + dir + opt, kSpatial, 12, 10, { 0, 7 });
            CHECK(fr.lit(0) > 0);
            CHECK(fr.lit(1) > 0);
        }
    }
    // the spatial colour varies across the buffer, so stripes of the spatial colour are not uniform
    Frames sp = f.Render("Bars", "E_CHOICE_Bars_Direction=up", kSpatial, 12, 10, { 0 });
    bool varies = false;
    for (int y = 0; y < 10; ++y) {
        if (!SameRgb(sp.at(0, 0, y), sp.at(0, 11, y))) varies = true;
    }
    CHECK(varies);
}

TEST_CASE("Bars cycles repeat the motion", "[effects][classic1]") {
    Fixture f;
    Frames c2 = f.Render("Bars", "E_CHOICE_Bars_Direction=Left,E_TEXTCTRL_Bars_Cycles=2.0", kRedBlue, 20, 1, Range(0, 19));
    Frames c1 = f.Render("Bars", "E_CHOICE_Bars_Direction=Left,E_TEXTCTRL_Bars_Cycles=1.0", kRedBlue, 20, 1, Range(0, 19));
    // two cycles move twice as far per frame
    CHECK(CyclicShift(Row(c1, 2, 0), Row(c1, 3, 0)) == 1);
    CHECK(CyclicShift(Row(c2, 2, 0), Row(c2, 3, 0)) == 2);
}

TEST_CASE("Bars on a DMX model sets the first pixel from the palette", "[effects][classic1]") {
    Fixture f("DmxGeneral");
    for (const char* dir : { "up", "Left", "Custom" }) {
        INFO(dir);
        Frames fr = f.Render("Bars", std::string("E_CHOICE_Bars_Direction=") + dir, kRedBlue, 1, 1, { 0, 10 });
        CHECK(fr.onlyColours({ xlRED, xlBLUE }));
        CHECK(fr.lit(0) == 1);
    }
}

// =============================================================================
// Fill
// =============================================================================

TEST_CASE("Fill directions mirror each other", "[effects][classic1]") {
    Fixture f;
    Frames up = f.Render("Fill", "E_CHOICE_Fill_Direction=Up", kRedBlue, 3, 8, { 5 });
    Frames down = f.Render("Fill", "E_CHOICE_Fill_Direction=Down", kRedBlue, 3, 8, { 5 });
    Frames left = f.Render("Fill", "E_CHOICE_Fill_Direction=Left", kRedBlue, 8, 3, { 5 });
    Frames right = f.Render("Fill", "E_CHOICE_Fill_Direction=Right", kRedBlue, 8, 3, { 5 });
    CHECK(up.rowUniform(0));
    CHECK(left.colUniform(0));
    for (int i = 0; i < 8; ++i) {
        CHECK(SameRgb(up.at(0, 0, i), down.at(0, 0, 7 - i)));
        CHECK(SameRgb(right.at(0, i, 0), up.at(0, 0, i)));
        CHECK(SameRgb(left.at(0, i, 0), right.at(0, 7 - i, 0)));
    }
    CHECK(SameRgb(right.at(0, 0, 0), xlRED));
    CHECK(SameRgb(right.at(0, 7, 0), xlBLUE));
}

TEST_CASE("Fill position controls how far the fill reaches", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "Up", "Down", "Left", "Right" }) {
        INFO(dir);
        bool vertical = dir[0] == 'U' || dir[0] == 'D';
        int w = vertical ? 2 : 10;
        int h = vertical ? 10 : 2;
        Frames half = f.Render("Fill", std::string("E_CHOICE_Fill_Direction=") + dir + ",E_SLIDER_Fill_Position=50", kRed, w, h, { 0 });
        Frames none = f.Render("Fill", std::string("E_CHOICE_Fill_Direction=") + dir + ",E_SLIDER_Fill_Position=0", kRed, w, h, { 0 });
        CHECK(half.lit(0) == 10);
        CHECK(none.lit(0) == 0);
        int nearStart = vertical ? (dir[0] == 'U' ? half.litInRows(0, 0, 4) : half.litInRows(0, 5, 9))
                                 : (dir[0] == 'R' ? half.litInCols(0, 0, 4) : half.litInCols(0, 5, 9));
        CHECK(nearStart == 10);
    }
}

TEST_CASE("Fill bands and skips stripe the fill when colouring by time", "[effects][classic1]") {
    Fixture f;
    Frames fr = f.Render("Fill", "E_CHECKBOX_Fill_Color_Time=1,E_SLIDER_Fill_Band_Size=2,E_SLIDER_Fill_Skip_Size=1", kRedBlue, 2, 12, { 0 });
    // band 2 lit, skip 1 dark, colour advances per band: R R . B B . R R . B B .
    const xlColor expect[12] = { xlRED, xlRED, xlBLACK, xlBLUE, xlBLUE, xlBLACK, xlRED, xlRED, xlBLACK, xlBLUE, xlBLUE, xlBLACK };
    for (int y = 0; y < 12; ++y) {
        INFO("row " << y);
        CHECK(SameRgb(fr.at(0, 0, y), expect[y]));
    }
    // the offset shifts the stripes
    Frames off = f.Render("Fill", "E_CHECKBOX_Fill_Color_Time=1,E_SLIDER_Fill_Band_Size=2,E_SLIDER_Fill_Skip_Size=1,E_SLIDER_Fill_Offset=1", kRedBlue, 2, 12, { 0 });
    CHECK_FALSE(off == fr);
    Frames offPct = f.Render("Fill", "E_CHECKBOX_Fill_Color_Time=1,E_SLIDER_Fill_Band_Size=2,E_SLIDER_Fill_Skip_Size=1,E_SLIDER_Fill_Offset=50,E_CHECKBOX_Fill_Offset_In_Pixels=0", kRedBlue, 2, 12, { 0 });
    CHECK_FALSE(offPct == fr);
}

TEST_CASE("Fill colour by time without bands is one colour across the buffer", "[effects][classic1]") {
    Fixture f;
    Frames fr = f.Render("Fill", "E_CHECKBOX_Fill_Color_Time=1", kRedBlue, 3, 6, { 0, 19 });
    for (size_t i = 0; i < 2; ++i) {
        for (int y = 0; y < 6; ++y) {
            CHECK(SameRgb(fr.at(i, 0, y), fr.at(i, 0, 0)));
        }
    }
    CHECK(SameRgb(fr.at(0, 0, 0), xlRED));
    CHECK(fr.at(1, 0, 0).blue > fr.at(1, 0, 0).red);
}

TEST_CASE("Fill offsets and wrap for every direction", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "Up", "Down", "Left", "Right" }) {
        INFO(dir);
        for (const char* wrap : { "1", "0" }) {
            INFO("wrap " << wrap);
            std::string s = std::string("E_CHOICE_Fill_Direction=") + dir + ",E_CHECKBOX_Fill_Wrap=" + wrap + ",E_SLIDER_Fill_Position=60";
            Frames base = f.Render("Fill", s, kRedBlue, 10, 10, { 0 });
            Frames off = f.Render("Fill", s + ",E_SLIDER_Fill_Offset=3", kRedBlue, 10, 10, { 0 });
            Frames pct = f.Render("Fill", s + ",E_SLIDER_Fill_Offset=30,E_CHECKBOX_Fill_Offset_In_Pixels=0", kRedBlue, 10, 10, { 0 });
            CHECK(base.lit(0) == 60);
            CHECK_FALSE(off == base);
            CHECK(off.lit(0) > 0);
            CHECK(pct.lit(0) > 0);
        }
    }
}

TEST_CASE("Fill on a DMX model matches the regular path", "[effects][classic1]") {
    Fixture dmx("DmxGeneral");
    Fixture reg;
    const char* settings[] = {
        "",
        "E_CHOICE_Fill_Direction=Down",
        "E_CHOICE_Fill_Direction=Left,E_SLIDER_Fill_Position=50",
        "E_CHOICE_Fill_Direction=Right,E_CHECKBOX_Fill_Wrap=0,E_SLIDER_Fill_Offset=1",
        "E_CHECKBOX_Fill_Color_Time=1,E_SLIDER_Fill_Band_Size=1,E_SLIDER_Fill_Skip_Size=1",
        "E_CHECKBOX_Fill_Color_Time=1,E_CHECKBOX_Fill_Offset_In_Pixels=0,E_SLIDER_Fill_Offset=40",
    };
    for (const char* s : settings) {
        INFO(s);
        Frames a = dmx.Render("Fill", s, kRedBlue, 1, 1, { 0, 9, 19 });
        Frames b = reg.Render("Fill", s, kRedBlue, 1, 1, { 0, 9, 19 });
        CHECK(a == b);
    }
    // a bigger DMX buffer exercises the scalar loop over every line
    for (const char* dir : { "Up", "Down", "Left", "Right" }) {
        for (const char* wrap : { "1", "0" }) {
            Frames big = dmx.Render("Fill", std::string("E_CHOICE_Fill_Direction=") + dir + ",E_CHECKBOX_Fill_Wrap=" + wrap +
                                                ",E_SLIDER_Fill_Position=70,E_SLIDER_Fill_Offset=2,E_CHECKBOX_Fill_Color_Time=1,E_SLIDER_Fill_Band_Size=2",
                                    kRedBlue, 4, 4, { 3 });
            CHECK(big.f.size() == 1);
        }
    }
}

TEST_CASE("Fill settings checks and migration", "[effects][classic1]") {
    Fixture f;
    RenderableEffect* fx = f.Fx("Fill");
    Effect* e = f.layer->AddEffect(0, "Fill", "E_VALUECURVE_Fill_Position=Active=FALSE|", kRed, 0, 1000, 0, false);
    auto warn = fx->CheckEffectSettings(e->GetSettings(), nullptr, f.model.get(), e, false);
    bool found = false;
    for (const auto& s : warn) {
        if (s.find("position value curve") != std::string::npos) found = true;
    }
    CHECK(found);

    Effect* old = f.layer->AddEffect(0, "Fill", "", kRed, 0, 1000, 0, false);
    CHECK(fx->needToAdjustSettings("2016.10"));
    fx->adjustSettings("2016.10", old, false);
    CHECK(old->GetSettings().Get("E_CHECKBOX_Fill_Color_Time", "") == "1");
    CHECK(old->GetSettings().Get("E_CHECKBOX_Fill_Wrap", "") == "1");
    Effect* newer = f.layer->AddEffect(0, "Fill", "", kRed, 0, 1000, 0, false);
    fx->adjustSettings("2024.01", newer, false);
    CHECK(newer->GetSettings().Get("E_CHECKBOX_Fill_Color_Time", "") == "");
}

// =============================================================================
// Ripple
// =============================================================================

namespace {
const char* kRippleObjects[] = { "Circle", "Square", "Triangle", "Star", "Polygon", "Heart", "Tree",
                                 "Candy Cane", "Snow Flake", "Crucifix", "Present" };
}

TEST_CASE("Ripple old style draws every object with every movement", "[effects][classic1]") {
    Fixture f;
    for (const char* obj : kRippleObjects) {
        for (const char* mv : { "Explode", "Implode", "None" }) {
            INFO(obj << " " << mv);
            std::string s = std::string("E_CHOICE_Ripple_Object_To_Draw=") + obj + ",E_CHOICE_Ripple_Movement=" + mv;
            bool squareNone = std::string(obj) == "Square" && std::string(mv) == "None";
            Frames fr = f.Render("Ripple", s, kRedBlue, 24, 24, { 5, 10 });
            if (!squareNone) {
                CHECK(fr.lit(0) + fr.lit(1) > 0);
            }
            // colour index follows time: first half red, second half blue
            CHECK(fr.onlyColours({ xlRED, xlBLUE }));
            Frames d3 = f.Render("Ripple", s + ",E_CHECKBOX_Ripple3D=1,E_SLIDER_Ripple_Thickness=6", kRedBlue, 24, 24, { 5, 12 });
            Frames rot = f.Render("Ripple", s + ",E_SLIDER_Ripple_Rotation=30,E_SLIDER_RIPPLE_POINTS=6", kRedBlue, 24, 24, { 8 });
            if (!squareNone) {
                CHECK(d3.lit(0) + d3.lit(1) > 0);
                CHECK(rot.lit(0) > 0);
            }
        }
    }
}

// like every other object, the square draws a shape when Movement is None
TEST_CASE("Ripple old style square with no movement draws something", "[effects][classic1]") {
    Fixture f;
    Frames sq = f.Render("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Square,E_CHOICE_Ripple_Movement=None", kRed, 24, 24, { 5, 10 });
    CHECK(sq.lit(0) + sq.lit(1) > 0);
}

TEST_CASE("Ripple explode grows and implode shrinks", "[effects][classic1]") {
    Fixture f;
    Frames ex = f.Render("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Circle,E_CHOICE_Ripple_Movement=Explode,E_SLIDER_Ripple_Thickness=1", kRed, 31, 31, { 3, 8, 13 });
    Frames im = f.Render("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Circle,E_CHOICE_Ripple_Movement=Implode,E_SLIDER_Ripple_Thickness=1", kRed, 31, 31, { 3, 8, 13 });
    double c = 15;
    CHECK(ex.meanDistFrom(0, c, c) < ex.meanDistFrom(1, c, c));
    CHECK(ex.meanDistFrom(1, c, c) < ex.meanDistFrom(2, c, c));
    CHECK(im.meanDistFrom(0, c, c) > im.meanDistFrom(1, c, c));
    CHECK(im.meanDistFrom(1, c, c) > im.meanDistFrom(2, c, c));
    // the ring is centred on the buffer
    double cx, cy;
    REQUIRE(ex.centroid(1, cx, cy));
    CHECK(std::abs(cx - c) < 1.5);
    CHECK(std::abs(cy - c) < 1.5);
}

TEST_CASE("Ripple centre offsets move the shape", "[effects][classic1]") {
    Fixture f;
    for (const char* style : { "Old", "Lines Outward", "Solid Both" }) {
        INFO(style);
        std::string base = std::string("E_CHOICE_Ripple_Draw_Style=") + style + ",E_CHOICE_Ripple_Object_To_Draw=Circle,E_SLIDER_Ripple_Scale=30";
        Frames mid = f.Render("Ripple", base, kRed, 40, 40, { 10 });
        Frames right = f.Render("Ripple", base + ",E_SLIDER_Ripple_XC=50,E_SLIDER_Ripple_YC=-50", kRed, 40, 40, { 10 });
        double mx, my, rx, ry;
        REQUIRE(mid.centroid(0, mx, my));
        REQUIRE(right.centroid(0, rx, ry));
        CHECK(rx > mx + 5);
        CHECK(ry < my - 5);
    }
}

TEST_CASE("Ripple cycles repeat the animation", "[effects][classic1]") {
    Fixture f;
    Frames fr = f.Render("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Square,E_CHOICE_Ripple_Movement=Explode,E_TEXTCTRL_Ripple_Cycles=2.0", kRed, 20, 20, { 2, 12, 4, 14 });
    CHECK(fr.sameFrame(0, 1));
    CHECK(fr.sameFrame(2, 3));
    CHECK_FALSE(fr.sameFrame(0, 2));
}

TEST_CASE("Ripple new styles draw every object", "[effects][classic1]") {
    Fixture f;
    const char* styles[] = { "Lines Inward", "Lines Outward", "Lines Both", "Lines Inward Ripple", "Lines Outward Ripple", "Lines Both Ripple",
                             "Solid Inward", "Solid Outward", "Solid Both", "Solid Inward Ripple", "Solid Outward Ripple", "Solid Both Ripple",
                             "Highlight Inward", "Highlight Outward", "Highlight Both", "Highlight Inward Ripple", "Highlight Outward Ripple", "Highlight Both Ripple" };
    for (const char* style : styles) {
        for (const char* obj : kRippleObjects) {
            INFO(style << " " << obj);
            std::string s = std::string("E_CHOICE_Ripple_Draw_Style=") + style + ",E_CHOICE_Ripple_Object_To_Draw=" + obj +
                            ",E_SLIDER_Ripple_Scale=60,E_TEXTCTRL_Ripple_Spacing=10.0";
            Frames fr = f.Render("Ripple", s, kRGB, 20, 16, { 10 });
            CHECK(fr.lit(0) > 0);
        }
    }
}

TEST_CASE("Ripple new styles: solid fills more than lines, outward reaches further than inward", "[effects][classic1]") {
    Fixture f;
    auto r = [&](const std::string& style, bool fade = false) {
        return f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=" + style + ",E_CHOICE_Ripple_Object_To_Draw=Circle,E_CHOICE_Ripple_Movement=None,"
                                  "E_SLIDER_Ripple_Scale=40,E_TEXTCTRL_Ripple_Spacing=20.0,E_SLIDER_Ripple_Thickness=4" +
                                  std::string(fade ? ",E_CHECKBOX_Ripple3D=1" : ""),
                        kRGB, 41, 41, { 10 });
    };
    Frames lines = r("Lines Both");
    Frames solid = r("Solid Both");
    Frames in = r("Solid Inward");
    Frames out = r("Solid Outward");
    CHECK(solid.lit(0) > lines.lit(0));
    CHECK(out.meanDistFrom(0, 20.5, 20.5) > in.meanDistFrom(0, 20.5, 20.5));
    // the main shape uses colour 1, the ripples colour 2 (lines) / colour 3 (fill)
    auto has = [](const Frames& fr, const xlColor& c) {
        for (const auto& p : fr.f[0])
            if (SameRgb(p, c)) return true;
        return false;
    };
    CHECK(has(lines, xlRED));
    CHECK(has(lines, xlGREEN));
    CHECK(has(solid, xlBLUE));
    // 3D fades the ripples: colours that are not in the palette appear
    Frames fadeLines = r("Lines Both", true);
    Frames fadeSolid = r("Highlight Both Ripple", true);
    CHECK_FALSE(fadeLines.onlyColours({ xlRED, xlGREEN, xlBLUE }));
    CHECK_FALSE(fadeSolid.onlyColours({ xlRED, xlGREEN, xlBLUE }));
    CHECK(lines.onlyColours({ xlRED, xlGREEN, xlBLUE }));
}

TEST_CASE("Ripple velocity, direction, twist and outline change the drawing", "[effects][classic1]") {
    Fixture f;
    std::string base = "E_CHOICE_Ripple_Draw_Style=Lines Outward,E_CHOICE_Ripple_Object_To_Draw=Square,E_SLIDER_Ripple_Scale=40,E_TEXTCTRL_Ripple_Spacing=10.0";
    Frames plain = f.Render("Ripple", base, kRedBlue, 30, 30, { 10 });
    Frames vel = f.Render("Ripple", base + ",E_TEXTCTRL_Ripple_Velocity=20.0,E_SLIDER_Ripple_Direction=90", kRedBlue, 30, 30, { 10 });
    Frames twist = f.Render("Ripple", base + ",E_TEXTCTRL_Ripple_Twist=15.0", kRedBlue, 30, 30, { 10 });
    Frames outline = f.Render("Ripple", base + ",E_TEXTCTRL_Ripple_Outline=8.0", kRedBlue, 30, 30, { 10 });
    CHECK_FALSE(vel == plain);
    CHECK_FALSE(twist == plain);
    CHECK(outline.lit(0) > plain.lit(0));
    // velocity towards 90 degrees drifts the ripples to the right
    double px, py, vx, vy;
    REQUIRE(plain.centroid(0, px, py));
    REQUIRE(vel.centroid(0, vx, vy));
    CHECK(vx > px);
    // the same settings on a solid style
    Frames svel = f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=Solid Both Ripple,E_CHOICE_Ripple_Object_To_Draw=Star,E_TEXTCTRL_Ripple_Velocity=20.0,"
                                     "E_SLIDER_Ripple_Direction=45,E_TEXTCTRL_Ripple_Twist=10.0,E_SLIDER_Ripple_Scale=40",
                           kRGB, 30, 30, { 10 });
    CHECK(svel.lit(0) > 0);
}

TEST_CASE("Ripple 3D with alpha fades the ripples through transparency", "[effects][classic1]") {
    Fixture f;
    Opts o;
    o.allowAlpha = true;
    for (const char* style : { "Old", "Lines Both", "Solid Both" }) {
        INFO(style);
        Frames fr = f.Render("Ripple", std::string("E_CHOICE_Ripple_Draw_Style=") + style +
                                           ",E_CHOICE_Ripple_Object_To_Draw=Square,E_CHOICE_Ripple_Movement=Explode,E_CHECKBOX_Ripple3D=1,E_SLIDER_Ripple_Thickness=5,E_SLIDER_Ripple_Scale=50,E_TEXTCTRL_Ripple_Spacing=10.0",
                             kRGB, 30, 30, { 10 }, o);
        bool partial = false;
        for (const auto& c : fr.f[0]) {
            if (c.alpha > 0 && c.alpha < 255 && (c.red || c.green || c.blue)) partial = true;
        }
        CHECK(partial);
    }
}

TEST_CASE("Ripple SVG object outlines the shapes in the file", "[effects][classic1]") {
    TempDir td;
    std::string svg = td.Write("shape.svg",
                               "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"100\" height=\"100\" viewBox=\"0 0 100 100\">"
                               "<path d=\"M10 10 L90 10 L90 90 L10 90 Z\" fill=\"#00ff00\" stroke=\"#00ff00\"/>"
                               "<path d=\"M20 50 C20 20 80 20 80 50\" fill=\"none\" stroke=\"#ff00ff\"/>"
                               "<path d=\"M30 70 C40 60 60 60 70 70\" fill=\"none\" stroke=\"#0000ff\"/>"
                               "</svg>");
    Fixture f;
    for (const char* style : { "Lines Outward", "Solid Both", "Highlight Inward Ripple" }) {
        INFO(style);
        std::string s = std::string("E_CHOICE_Ripple_Draw_Style=") + style + ",E_CHOICE_Ripple_Object_To_Draw=SVG,E_FILEPICKERCTRL_Ripple_SVG=" + svg +
                        ",E_CHOICE_Ripple_Movement=None,E_SLIDER_Ripple_Scale=80";
        Frames fr = f.Render("Ripple", s, kRed, 30, 30, { 0, 5 });
        CHECK(fr.lit(0) > 0);
        // with one palette colour, the ripples fall back to the colours in the file
        Frames circle = f.Render("Ripple", std::string("E_CHOICE_Ripple_Draw_Style=") + style +
                                               ",E_CHOICE_Ripple_Object_To_Draw=Circle,E_CHOICE_Ripple_Movement=None,E_SLIDER_Ripple_Scale=80",
                                 kRed, 30, 30, { 0, 5 });
        CHECK_FALSE(fr == circle);
    }
    // every outline is drawn in palette colours
    Frames lines = f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=Lines Outward,E_CHOICE_Ripple_Object_To_Draw=SVG,E_FILEPICKERCTRL_Ripple_SVG=" + svg +
                                          ",E_CHOICE_Ripple_Movement=None,E_SLIDER_Ripple_Scale=80,E_TEXTCTRL_Ripple_Spacing=10.0",
                            kRedBlue, 30, 30, { 0 });
    CHECK(lines.lit(0) > 0);
    CHECK(lines.onlyColours({ xlRED, xlBLUE }));

    // a missing file falls back to a circle; the "Old" style ignores SVG entirely
    Frames missing = f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=Lines Outward,E_CHOICE_Ripple_Object_To_Draw=SVG,E_FILEPICKERCTRL_Ripple_SVG=" +
                                            td.File("missing.svg"),
                              kRed, 20, 20, { 10 });
    Frames circle = f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=Lines Outward,E_CHOICE_Ripple_Object_To_Draw=Circle", kRed, 20, 20, { 10 });
    CHECK(missing == circle);
    Frames oldSvg = f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=Old,E_CHOICE_Ripple_Object_To_Draw=SVG", kRed, 20, 20, { 10 });
    Frames oldCircle = f.Render("Ripple", "E_CHOICE_Ripple_Draw_Style=Old,E_CHOICE_Ripple_Object_To_Draw=Circle", kRed, 20, 20, { 10 });
    CHECK(oldSvg == oldCircle);
}

TEST_CASE("Ripple driven by a timing track starts at each mark", "[effects][classic1]") {
    Fixture f;
    f.AddTiming("Beats", { { 500, 600, "a" }, { 1500, 1600, "b" } });
    Opts o;
    o.endMs = 2000;
    std::string s = "E_CHOICE_Ripple_Object_To_Draw=Circle,E_CHOICE_Ripple_TimingTrack=Beats,E_SLIDER_Ripple_Duration=250";
    // frames are 50ms: marks at period 10 and 30, each ripple lasts 5 periods
    Frames fr = f.Render("Ripple", s, kRed, 20, 20, Range(0, 39), o);
    for (int p = 0; p < 10; ++p) CHECK(fr.lit(p) == 0);
    int litAfterA = 0;
    for (int p = 10; p < 15; ++p) litAfterA += fr.lit(p);
    CHECK(litAfterA > 0);
    for (int p = 16; p < 30; ++p) CHECK(fr.lit(p) == 0);
    int litAfterB = 0;
    for (int p = 30; p < 35; ++p) litAfterB += fr.lit(p);
    CHECK(litAfterB > 0);
    // the animation restarts at each mark
    CHECK(fr.sameFrame(11, 31));

    // a label filter keeps only matching marks
    Frames filtered = f.Render("Ripple", s + ",E_TEXTCTRL_Ripple_FilterLabel=b", kRed, 20, 20, Range(0, 39), o);
    for (int p = 0; p < 30; ++p) CHECK(filtered.lit(p) == 0);
    CHECK(filtered.sameFrame(31, 31));
    CHECK(filtered.lit(31) > 0);
    Frames regex = f.Render("Ripple", s + ",E_TEXTCTRL_Ripple_FilterLabel=^a$,E_CHECKBOX_Ripple_FilterRegex=1", kRed, 20, 20, Range(0, 39), o);
    CHECK(regex.lit(11) > 0);
    CHECK(regex.lit(31) == 0);

    // a track name that doesn't exist behaves as if there were no track
    Frames noTrack = f.Render("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Circle,E_CHOICE_Ripple_TimingTrack=Nope", kRed, 20, 20, { 5 });
    Frames plain = f.Render("Ripple", "E_CHOICE_Ripple_Object_To_Draw=Circle", kRed, 20, 20, { 5 });
    CHECK(noTrack == plain);
}

TEST_CASE("Ripple rename timing track updates the effect", "[effects][classic1]") {
    Fixture f;
    RenderableEffect* fx = f.Fx("Ripple");
    Effect* e = f.layer->AddEffect(0, "Ripple", "E_CHOICE_Ripple_TimingTrack=Old", kRed, 0, 1000, 0, false);
    fx->RenameTimingTrack("Other", "New", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Ripple_TimingTrack", "") == "Old");
    fx->RenameTimingTrack("Old", "New", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Ripple_TimingTrack", "") == "New");
}

// =============================================================================
// Meteors
// =============================================================================

TEST_CASE("Meteors move in the chosen direction", "[effects][classic1]") {
    Fixture f;
    struct D {
        const char* dir;
        int dx, dy; // where a pixel was in the previous frame
    };
    // y = 0 is the bottom row
    for (const D& d : { D{ "Down", 0, 1 }, D{ "Up", 0, -1 }, D{ "Left", 1, 0 }, D{ "Right", -1, 0 } }) {
        INFO(d.dir);
        Frames fr = f.Render("Meteors", std::string("E_CHOICE_Meteors_Effect=") + d.dir + ",E_SLIDER_Meteors_Count=30,E_SLIDER_Meteors_Length=10",
                             kRed, 24, 24, Range(0, 12));
        int fwd = fr.motionScore(d.dx, d.dy);
        int back = fr.motionScore(-d.dx, -d.dy);
        CHECK(fr.lit(10) > 0);
        CHECK(fwd > back * 2);
    }
}

TEST_CASE("Meteors colour schemes", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "Down", "Left", "Explode", "Implode", "Icicles" }) {
        INFO(dir);
        std::string base = std::string("E_CHOICE_Meteors_Effect=") + dir + ",E_SLIDER_Meteors_Count=40";
        Frames pal = f.Render("Meteors", base + ",E_CHOICE_Meteors_Type=Palette", kRed, 20, 20, Range(0, 8));
        CHECK(pal.totalLit() > 0);
        // tails fade and icicle tips whiten, but nothing ever turns green or blue
        CHECK(pal.onlyRedTints());
        Frames rainbow = f.Render("Meteors", base + ",E_CHOICE_Meteors_Type=Rainbow", kRed, 20, 20, Range(0, 8));
        CHECK_FALSE(rainbow.onlyRedShades());
        // a range between two palette colours
        Frames range = f.Render("Meteors", base + ",E_CHOICE_Meteors_Type=Range", kRedBlue, 20, 20, Range(0, 8));
        CHECK(range.totalLit() > 0);
    }
}

TEST_CASE("Meteors explode starts at the centre and implode at the edge", "[effects][classic1]") {
    Fixture f;
    Frames ex = f.Render("Meteors", "E_CHOICE_Meteors_Effect=Explode,E_SLIDER_Meteors_Count=50,E_CHOICE_Meteors_Type=Palette", kRed, 40, 40, Range(0, 30));
    Frames im = f.Render("Meteors", "E_CHOICE_Meteors_Effect=Implode,E_SLIDER_Meteors_Count=50,E_CHOICE_Meteors_Type=Palette", kRed, 40, 40, Range(0, 30));
    CHECK(ex.totalLit() > 0);
    CHECK(im.totalLit() > 0);
    // find the first lit frame of each and compare how far from the centre it is
    auto firstLit = [](const Frames& fr) {
        for (size_t i = 0; i < fr.f.size(); ++i)
            if (fr.lit(i) > 0) return i;
        return fr.f.size();
    };
    size_t fe = firstLit(ex), fi = firstLit(im);
    REQUIRE(fe < ex.f.size());
    REQUIRE(fi < im.f.size());
    CHECK(ex.meanDistFrom(fe, 20, 20) < im.meanDistFrom(fi, 20, 20));

    // an x offset moves the explosion centre right
    Frames off = f.Render("Meteors", "E_CHOICE_Meteors_Effect=Explode,E_SLIDER_Meteors_Count=50,E_SLIDER_Meteors_XOffset=50,E_CHOICE_Meteors_Type=Palette",
                          kRed, 40, 40, Range(0, 30));
    double cx0, cy0, cx1, cy1;
    REQUIRE(ex.centroid(fe, cx0, cy0));
    size_t fo = firstLit(off);
    REQUIRE(fo < off.f.size());
    REQUIRE(off.centroid(fo, cx1, cy1));
    CHECK(cx1 > cx0 + 3);
}

TEST_CASE("Meteors radial styles honour fade with distance", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "Explode", "Implode" }) {
        INFO(dir);
        std::string base = std::string("E_CHOICE_Meteors_Effect=") + dir + ",E_SLIDER_Meteors_Count=50,E_CHOICE_Meteors_Type=Palette,E_SLIDER_Meteors_YOffset=-20";
        Frames plain = f.Render("Meteors", base, kRed, 30, 30, Range(0, 20));
        Frames fade = f.Render("Meteors", base + ",E_CHECKBOX_FadeWithDistance=1", kRed, 30, 30, Range(0, 20));
        CHECK(plain.totalLit() > 0);
        CHECK(fade.totalLit() > 0);
        CHECK_FALSE(plain == fade);
        CHECK(fade.onlyRedShades());
    }
}

TEST_CASE("Meteors icicles drip down and the background variant lights the background", "[effects][classic1]") {
    Fixture f;
    Frames ic = f.Render("Meteors", "E_CHOICE_Meteors_Effect=Icicles,E_SLIDER_Meteors_Count=40,E_CHOICE_Meteors_Type=Palette", kRed, 20, 20, Range(0, 25));
    Frames bkg = f.Render("Meteors", "E_CHOICE_Meteors_Effect=Icicles + bkg,E_SLIDER_Meteors_Count=40,E_CHOICE_Meteors_Type=Palette", kRed, 20, 20, Range(0, 25));
    CHECK(ic.totalLit() > 0);
    CHECK(bkg.totalLit() > ic.totalLit());
    CHECK(ic.onlyRedTints());
    // drips only ever run downwards from the top
    CHECK(ic.litInRows(20, 10, 19) >= ic.litInRows(20, 0, 9));
}

TEST_CASE("Meteors swirl, speed and warm up change the picture", "[effects][classic1]") {
    Fixture f;
    for (const char* dir : { "Down", "Up", "Left", "Right", "Icicles" }) {
        INFO(dir);
        std::string base = std::string("E_CHOICE_Meteors_Effect=") + dir + ",E_SLIDER_Meteors_Count=30,E_CHOICE_Meteors_Type=Palette";
        Frames plain = f.Render("Meteors", base, kRed, 20, 20, Range(0, 8));
        Frames swirl = f.Render("Meteors", base + ",E_SLIDER_Meteors_Swirl_Intensity=10", kRed, 20, 20, Range(0, 8));
        Frames warm = f.Render("Meteors", base + ",E_SLIDER_Meteors_WamupFrames=20", kRed, 20, 20, { 0 });
        Frames cold = f.Render("Meteors", base, kRed, 20, 20, { 0 });
        CHECK_FALSE(plain == swirl);
        CHECK(warm.lit(0) > cold.lit(0));
        Frames still = f.Render("Meteors", base + ",E_SLIDER_Meteors_Speed=0", kRed, 20, 20, Range(0, 4));
        CHECK(still.f.size() == 5);
    }
}

TEST_CASE("Meteors use music scales the count by the audio level", "[effects][classic1]") {
    Fixture f;
    std::string s = "E_CHOICE_Meteors_Effect=Down,E_SLIDER_Meteors_Count=50,E_CHECKBOX_Meteors_UseMusic=1";
    // no audio: the count goes to zero and nothing is drawn
    Frames silent = f.Render("Meteors", s, kRed, 20, 20, Range(0, 10));
    CHECK(silent.totalLit() == 0);
    LoudAudio audio;
    Opts o;
    o.media = audio.am.get();
    Frames loud = f.Render("Meteors", s, kRed, 20, 20, Range(0, 10), o);
    CHECK(loud.totalLit() > 0);
}

TEST_CASE("Meteors render deterministically", "[effects][classic1]") {
    for (const char* dir : { "Down", "Explode", "Implode", "Icicles + bkg" }) {
        INFO(dir);
        Fixture a, b;
        std::string s = std::string("E_CHOICE_Meteors_Effect=") + dir + ",E_SLIDER_Meteors_Count=30";
        CHECK(a.Render("Meteors", s, kRedBlue, 16, 16, Range(0, 10)) == b.Render("Meteors", s, kRedBlue, 16, 16, Range(0, 10)));
    }
}

// =============================================================================
// Snowflakes
// =============================================================================

TEST_CASE("Snowflakes draw every flake type in palette colours", "[effects][classic1]") {
    Fixture f;
    for (int type = 0; type <= 9; ++type) {
        for (const char* mode : { "Driving", "Falling", "Falling & Accumulating" }) {
            INFO("type " << type << " " << mode);
            Frames fr = f.Render("Snowflakes", "E_SLIDER_Snowflakes_Type=" + std::to_string(type) + ",E_CHOICE_Falling=" + mode +
                                                   ",E_SLIDER_Snowflakes_Count=10,E_SLIDER_Snowflakes_Speed=20",
                                 kRedBlue, 20, 20, Range(0, 6));
            CHECK(fr.lit(0) > 0);
            CHECK(fr.onlyColours({ xlRED, xlBLUE }));
        }
    }
}

TEST_CASE("Snowflakes fall downwards", "[effects][classic1]") {
    Fixture f;
    for (const char* mode : { "Driving", "Falling" }) {
        INFO(mode);
        Frames fr = f.Render("Snowflakes", std::string("E_CHOICE_Falling=") + mode + ",E_SLIDER_Snowflakes_Type=1,E_SLIDER_Snowflakes_Count=30,E_SLIDER_Snowflakes_Speed=30",
                             kRed, 20, 20, Range(0, 12));
        int down = fr.motionScore(0, 1, 2, 0);
        int up = fr.motionScore(0, -1, 2, 0);
        CHECK(down > up);
    }
}

TEST_CASE("Snowflakes accumulate at the bottom", "[effects][classic1]") {
    Fixture f;
    Opts o;
    o.endMs = 5000;
    Frames acc = f.Render("Snowflakes", "E_CHOICE_Falling=Falling & Accumulating,E_SLIDER_Snowflakes_Type=1,E_SLIDER_Snowflakes_Count=20,E_SLIDER_Snowflakes_Speed=50",
                          kRed, 10, 10, Range(0, 99), o);
    Frames fall = f.Render("Snowflakes", "E_CHOICE_Falling=Falling,E_SLIDER_Snowflakes_Type=1,E_SLIDER_Snowflakes_Count=20,E_SLIDER_Snowflakes_Speed=50",
                           kRed, 10, 10, Range(0, 99), o);
    CHECK(acc.litInRows(99, 0, 1) > acc.litInRows(0, 0, 1));
    CHECK(acc.lit(99) > fall.lit(99));
}

TEST_CASE("Snowflakes driving speed zero holds still and warm up advances falling flakes", "[effects][classic1]") {
    Fixture f;
    Frames still = f.Render("Snowflakes", "E_CHOICE_Falling=Driving,E_SLIDER_Snowflakes_Speed=0,E_SLIDER_Snowflakes_Count=10", kRed, 16, 16, Range(0, 5));
    for (size_t i = 1; i < still.f.size(); ++i) CHECK(still.sameFrame(0, i));
    Frames moving = f.Render("Snowflakes", "E_CHOICE_Falling=Driving,E_SLIDER_Snowflakes_Speed=20,E_SLIDER_Snowflakes_Count=10", kRed, 16, 16, Range(0, 5));
    CHECK_FALSE(moving.sameFrame(0, 5));

    Frames warm = f.Render("Snowflakes", "E_CHOICE_Falling=Falling & Accumulating,E_SLIDER_Snowflakes_Speed=50,E_SLIDER_Snowflakes_WarmupFrames=100,E_SLIDER_Snowflakes_Count=10",
                           kRed, 16, 16, { 0 });
    Frames cold = f.Render("Snowflakes", "E_CHOICE_Falling=Falling & Accumulating,E_SLIDER_Snowflakes_Speed=50,E_SLIDER_Snowflakes_Count=10",
                           kRed, 16, 16, { 0 });
    CHECK_FALSE(warm == cold);
}

TEST_CASE("Snowflakes snapshots draw identically on a fresh buffer", "[effects][classic1]") {
    // A wide, short buffer with many fast flakes: flakes drift across the
    // column 0 / last column wrap, settle in the accumulating mode, and the
    // random type mixes in "3 nodes" flakes whose orientation is pre-rolled.
    for (int type = 0; type <= 9; ++type) {
        for (const char* mode : { "Driving", "Falling", "Falling & Accumulating" }) {
            for (const char* warmup : { "", ",E_SLIDER_Snowflakes_WarmupFrames=40" }) {
                INFO("type " << type << " " << mode << warmup);
                const std::string settings = "E_SLIDER_Snowflakes_Type=" + std::to_string(type) + ",E_CHOICE_Falling=" + mode +
                                             ",E_SLIDER_Snowflakes_Count=40,E_SLIDER_Snowflakes_Speed=50" + warmup;
                Opts o;
                o.endMs = 5000;
                Fixture serial;
                Frames a = serial.Render("Snowflakes", settings, kRedBlue, 48, 9, Range(0, 60), o);
                Fixture parallel;
                Frames b = parallel.RenderSnapshotsOnFreshBuffers("Snowflakes", settings, kRedBlue, 48, 9, Range(0, 60), o);
                CHECK(a.totalLit() > 0);
                CHECK(a == b);
            }
        }
    }
}

TEST_CASE("Snowflakes count changes re-seed driving flakes", "[effects][classic1]") {
    Fixture f;
    // a count value curve ramps from 1 to 40 over the effect: later frames hold more flakes
    Frames fr = f.Render("Snowflakes", "E_CHOICE_Falling=Driving,E_SLIDER_Snowflakes_Type=1,E_SLIDER_Snowflakes_Speed=0,"
                                       "E_VALUECURVE_Snowflakes_Count=Active=TRUE|Id=ID_VALUECURVE_Snowflakes_Count|Type=Ramp|Min=1.00|Max=100.00|P1=1.00|P2=40.00|RV=TRUE|",
                         kRed, 20, 20, { 0, 19 });
    CHECK(fr.lit(1) > fr.lit(0));
}

// =============================================================================
// Tendril
// =============================================================================

TEST_CASE("Tendril draws for every movement in palette colours", "[effects][classic1]") {
    Fixture f;
    for (const char* mv : { "Random", "Square", "Circle", "Horizontal Zig Zag", "Horiz. Zig Zag Return", "Vertical Zig Zag",
                            "Vert. Zig Zag Return", "Music Line", "Music Circle", "Manual", "Unknown" }) {
        for (int tune : { 0, 10 }) {
            INFO(mv << " tune " << tune);
            Frames fr = f.Render("Tendril", std::string("E_CHOICE_Tendril_Movement=") + mv + ",E_SLIDER_Tendril_TuneMovement=" + std::to_string(tune) +
                                                ",E_SLIDER_Tendril_ManualX=70,E_SLIDER_Tendril_ManualY=30",
                                 kRed, 24, 24, Range(0, 15));
            CHECK(fr.lit(15) > 0);
            CHECK(fr.onlyRedShades());
        }
    }
}

TEST_CASE("Tendril manual movement follows the manual point", "[effects][classic1]") {
    Fixture f;
    Frames a = f.Render("Tendril", "E_CHOICE_Tendril_Movement=Manual,E_SLIDER_Tendril_ManualX=90,E_SLIDER_Tendril_ManualY=90,E_SLIDER_Tendril_Thickness=1",
                        kRed, 30, 30, Range(0, 40));
    Frames b = f.Render("Tendril", "E_CHOICE_Tendril_Movement=Manual,E_SLIDER_Tendril_ManualX=10,E_SLIDER_Tendril_ManualY=10,E_SLIDER_Tendril_Thickness=1",
                        kRed, 30, 30, Range(0, 40));
    double ax, ay, bx, by;
    REQUIRE(a.centroid(40, ax, ay));
    REQUIRE(b.centroid(40, bx, by));
    CHECK(ax > bx + 5);
    CHECK(ay > by + 5);
    // offsets shift the target
    Frames off = f.Render("Tendril", "E_CHOICE_Tendril_Movement=Manual,E_SLIDER_Tendril_ManualX=10,E_SLIDER_Tendril_ManualY=10,E_SLIDER_Tendril_Thickness=1,"
                                     "E_SLIDER_Tendril_XOffset=50,E_SLIDER_Tendril_YOffset=50",
                          kRed, 30, 30, Range(0, 40));
    double ox, oy;
    REQUIRE(off.centroid(40, ox, oy));
    CHECK(ox > bx + 5);
    CHECK(oy > by + 5);
}

TEST_CASE("Tendril thickness, trails and length", "[effects][classic1]") {
    Fixture f;
    std::string base = "E_CHOICE_Tendril_Movement=Circle";
    Frames thin = f.Render("Tendril", base + ",E_SLIDER_Tendril_Thickness=1", kRed, 30, 30, Range(0, 15));
    Frames thick = f.Render("Tendril", base + ",E_SLIDER_Tendril_Thickness=4", kRed, 30, 30, Range(0, 15));
    CHECK(thick.lit(15) > thin.lit(15));
    Frames trails = f.Render("Tendril", base + ",E_SLIDER_Tendril_Thickness=1,E_TEXTCTRL_Tendril_Trails=4", kRed, 30, 30, Range(0, 15));
    CHECK_FALSE(trails == thin);
    Frames shortT = f.Render("Tendril", base + ",E_SLIDER_Tendril_Thickness=1,E_TEXTCTRL_Tendril_Length=5", kRed, 30, 30, Range(0, 15));
    CHECK(shortT.lit(15) < thin.lit(15));
    // a colour blend across the effect: two colours give pixels of both
    Frames two = f.Render("Tendril", base, kRedBlue, 30, 30, { 0, 19 });
    CHECK(two.at(0, 0, 0).blue == 0);
    double r0 = 0, b1 = 0;
    for (const auto& c : two.f[0]) r0 += c.red;
    for (const auto& c : two.f[1]) b1 += c.blue;
    CHECK(r0 > 0);
    CHECK(b1 > 0);
}

TEST_CASE("Tendril physics settings are clamped to their range", "[effects][classic1]") {
    Fixture f;
    std::string base = "E_CHOICE_Tendril_Movement=Square";
    // friction/dampening/tension below their range render the same as their minimum, and above it the same as the maximum
    CHECK(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Friction=-20", kRed, 20, 20, Range(0, 10)) ==
          f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Friction=0", kRed, 20, 20, Range(0, 10)));
    CHECK(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Friction=60", kRed, 20, 20, Range(0, 10)) ==
          f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Friction=20", kRed, 20, 20, Range(0, 10)));
    CHECK(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Dampening=-20", kRed, 20, 20, Range(0, 10)) ==
          f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Dampening=0", kRed, 20, 20, Range(0, 10)));
    CHECK(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Dampening=60", kRed, 20, 20, Range(0, 10)) ==
          f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Dampening=20", kRed, 20, 20, Range(0, 10)));
    CHECK(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Tension=-39", kRed, 20, 20, Range(0, 10)) ==
          f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Tension=0", kRed, 20, 20, Range(0, 10)));
    CHECK(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Tension=100", kRed, 20, 20, Range(0, 10)) ==
          f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Tension=39", kRed, 20, 20, Range(0, 10)));
    // and inside the range they matter
    CHECK_FALSE(f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Friction=0", kRed, 20, 20, Range(0, 10)) ==
                f.Render("Tendril", base + ",E_TEXTCTRL_Tendril_Friction=20", kRed, 20, 20, Range(0, 10)));
}

TEST_CASE("Tendril music movements react to the audio", "[effects][classic1]") {
    LoudAudio audio;
    Opts o;
    o.media = audio.am.get();
    Fixture f;
    for (const char* mv : { "Music Line", "Music Circle" }) {
        INFO(mv);
        Frames quiet = f.Render("Tendril", std::string("E_CHOICE_Tendril_Movement=") + mv, kRed, 24, 24, Range(0, 15));
        Frames loud = f.Render("Tendril", std::string("E_CHOICE_Tendril_Movement=") + mv, kRed, 24, 24, Range(0, 15), o);
        CHECK(loud.lit(15) > 0);
        CHECK_FALSE(quiet == loud);
    }
}

// =============================================================================
// Liquid
// =============================================================================

TEST_CASE("Liquid particles leave the emitter and fall with gravity", "[effects][classic1]") {
    Fixture f;
    // default emitter: top middle pointing down, gravity down, bottom barrier
    Frames fr = f.Render("Liquid", "", kRed, 20, 20, Range(0, 19));
    CHECK(fr.lit(2) > 0);
    CHECK(fr.onlyRedShades());
    double cx, cy;
    REQUIRE(fr.centroid(2, cx, cy));
    CHECK(std::abs(cx - 10) < 4);
    CHECK(cy > 10); // still near the top early on
    double lx, ly;
    REQUIRE(fr.centroid(19, lx, ly));
    CHECK(ly < cy);     // has fallen
    CHECK(fr.lit(19) > fr.lit(2));

    // gravity pointing up with an emitter at the bottom keeps the liquid in the top half
    Frames up = f.Render("Liquid", "E_SLIDER_Liquid_GravityAngle=180,E_CHECKBOX_TopBarrier=1,E_CHECKBOX_BottomBarrier=0,"
                                   "E_SLIDER_Y1=0,E_SLIDER_Direction1=90",
                         kRed, 20, 20, Range(0, 19));
    double ux, uy;
    REQUIRE(up.centroid(19, ux, uy));
    CHECK(uy > 10);
}

TEST_CASE("Liquid emitters can be placed on every side", "[effects][classic1]") {
    Fixture f;
    struct E {
        int n;
        bool left, right, bottom, top;
    };
    for (const E& e : { E{ 2, true, false, false, false }, E{ 3, false, false, true, false }, E{ 4, false, true, false, false } }) {
        INFO("emitter " << e.n);
        std::string n = std::to_string(e.n);
        Frames fr = f.Render("Liquid", "E_CHECKBOX_Enabled1=0,E_CHECKBOX_Enabled" + n + "=1,E_TEXTCTRL_Liquid_Gravity=0.0,E_TEXTCTRL_Liquid_SourceSize" + n + "=20",
                             kRedBlue, 20, 20, Range(0, 3));
        double cx, cy;
        REQUIRE(fr.centroid(3, cx, cy));
        if (e.left) CHECK(cx < 7);
        if (e.right) CHECK(cx > 13);
        if (e.bottom) CHECK(cy < 7);
    }
    // all four at once with several colours
    Frames all = f.Render("Liquid", "E_CHECKBOX_Enabled2=1,E_CHECKBOX_Enabled3=1,E_CHECKBOX_Enabled4=1,E_CHECKBOX_LeftBarrier=1,E_CHECKBOX_RightBarrier=1,E_CHECKBOX_TopBarrier=1",
                          kRGB, 20, 20, Range(0, 8));
    CHECK(all.lit(8) > 0);
}

TEST_CASE("Liquid particle types, colour mixing and despeckle", "[effects][classic1]") {
    Fixture f;
    for (const char* type : { "Powder", "Elastic", "Spring", "Tensile", "Viscous", "Static Pressure", "Water", "Reactive", "Repulsive" }) {
        INFO(type);
        Frames fr = f.Render("Liquid", std::string("E_CHOICE_ParticleType=") + type, kRed, 16, 16, Range(0, 6));
        CHECK(fr.lit(6) > 0);
        CHECK(fr.onlyRedShades());
    }
    Frames hold = f.Render("Liquid", "E_CHECKBOX_HoldColor=0", kRed, 16, 16, Range(0, 6));
    CHECK(hold.onlyColours({ xlRED }));
    Frames mix = f.Render("Liquid", "E_CHECKBOX_MixColors=1,E_CHECKBOX_Enabled3=1,E_SLIDER_Direction3=90", kRedBlue, 16, 16, Range(0, 10));
    CHECK(mix.lit(10) > 0);
    Frames plain = f.Render("Liquid", "", kRed, 16, 16, Range(0, 15));
    Frames desp = f.Render("Liquid", "E_TEXTCTRL_Despeckle=4", kRed, 16, 16, Range(0, 15));
    CHECK(desp.lit(15) >= plain.lit(15));
}

TEST_CASE("Liquid lifetime, warm up, size and flow", "[effects][classic1]") {
    Fixture f;
    Frames cold = f.Render("Liquid", "", kRed, 16, 16, { 0 });
    Frames warm = f.Render("Liquid", "E_TEXTCTRL_WarmUpTime=100", kRed, 16, 16, { 0 });
    CHECK(warm.lit(0) > cold.lit(0));
    Opts o;
    o.endMs = 2000;
    Frames longLife = f.Render("Liquid", "", kRed, 16, 16, Range(0, 30), o);
    Frames shortLife = f.Render("Liquid", "E_SLIDER_LifeTime=10", kRed, 16, 16, Range(0, 30), o);
    // the simulation is thrown away after the last frame of the effect
    Frames past = f.Render("Liquid", "", kRed, 16, 16, Range(0, 25));
    CHECK(past.lit(19) > 0);
    CHECK(past.lit(25) == 0);
    CHECK(shortLife.lit(30) < longLife.lit(30));
    Frames noFlow = f.Render("Liquid", "E_SLIDER_Flow1=0", kRed, 16, 16, Range(0, 6));
    CHECK(noFlow.totalLit() == 0);
    Frames big = f.Render("Liquid", "E_TEXTCTRL_Size=1500,E_SLIDER_Flow1=800", kRed, 16, 16, Range(0, 6));
    CHECK(big.lit(6) > 0);
}

TEST_CASE("Liquid flow to music follows the audio level", "[effects][classic1]") {
    Fixture f;
    LoudAudio audio;
    Opts o;
    o.media = audio.am.get();
    Frames quiet = f.Render("Liquid", "E_CHECKBOX_FlowMusic1=1", kRed, 16, 16, Range(0, 6));
    Frames loud = f.Render("Liquid", "E_CHECKBOX_FlowMusic1=1", kRed, 16, 16, Range(0, 6), o);
    CHECK(loud.lit(6) > quiet.lit(6));
}

TEST_CASE("Liquid settings checks warn about music, size and particle count", "[effects][classic1]") {
    Fixture f;
    RenderableEffect* fx = f.Fx("Liquid");
    auto check = [&](const std::string& settings, const std::string& needle) {
        Effect* e = f.layer->AddEffect(0, "Liquid", settings, kRed, 0, 10000, 0, false);
        auto res = fx->CheckEffectSettings(e->GetSettings(), nullptr, f.model.get(), e, false);
        for (const auto& s : res) {
            if (s.find(needle) != std::string::npos) return true;
        }
        return false;
    };
    CHECK(check("E_CHECKBOX_FlowMusic2=1", "flow to music"));
    CHECK_FALSE(check("", "flow to music"));
    CHECK(check("E_TEXTCTRL_Size=2000", "particle size"));
    CHECK_FALSE(check("", "particle size"));
    CHECK(check("E_TEXTCTRL_Flow1=1000,E_CHECKBOX_Enabled2=1,E_TEXTCTRL_Flow2=1000,E_TEXTCTRL_LifeTime=1000", "exceeds"));
    CHECK_FALSE(check("E_TEXTCTRL_Flow1=10,E_TEXTCTRL_LifeTime=100", "exceeds"));
    CHECK(check("E_TEXTCTRL_Flow1=1000,E_CHECKBOX_Enabled3=1,E_CHECKBOX_Enabled4=1,E_TEXTCTRL_LifeTime=0", "exceeds"));
}

TEST_CASE("Liquid migrates per-frame settings from older sequences", "[effects][classic1]") {
    Fixture f;
    RenderableEffect* fx = f.Fx("Liquid");
    CHECK(fx->needToAdjustSettings("2026.01"));
    Effect* e = f.layer->AddEffect(0, "Liquid",
                                   "E_TEXTCTRL_Velocity1=5,E_TEXTCTRL_Velocity2=20,E_TEXTCTRL_Flow1=10,E_TEXTCTRL_Flow2=500,E_TEXTCTRL_Flow3=0,E_TEXTCTRL_WarmUpFrames=10,"
                                   "E_VALUECURVE_Velocity3=Active=TRUE|Id=ID_VALUECURVE_Velocity3|Type=Flat|Min=0.00|Max=1000.00|P1=2.00|RV=TRUE|,"
                                   "E_VALUECURVE_Flow4=Active=TRUE|Id=ID_VALUECURVE_Flow4|Type=Flat|Min=0.00|Max=1000.00|P1=10.00|RV=TRUE|",
                                   kRed, 0, 1000, 0, false);
    fx->adjustSettings("2026.01", e, false);
    const SettingsMap& s = e->GetSettings();
    CHECK(s.Get("E_TEXTCTRL_Velocity1", "") == "500");
    CHECK(s.Get("E_TEXTCTRL_Velocity2", "") == "1000"); // clamped to the slider maximum
    // flow per frame at 20fps becomes per second, mapped through the slider curve
    CHECK(s.Get("E_TEXTCTRL_Flow1", "") == "50");
    CHECK(s.Get("E_TEXTCTRL_Flow3", "") == "0");
    long flow2 = std::strtol(s.Get("E_TEXTCTRL_Flow2", "").c_str(), nullptr, 10);
    CHECK(flow2 > 500); // 10000/s lands on the exponential half of the curve
    CHECK(flow2 <= 1000);
    CHECK(s.Get("E_TEXTCTRL_WarmUpTime", "") == "50");
    CHECK(s.Get("E_TEXTCTRL_WarmUpFrames", "missing") == "missing");
    CHECK(s.Get("E_VALUECURVE_Velocity3", "").find("P1=200") != std::string::npos);
    CHECK(s.Get("E_VALUECURVE_Flow4", "").find("P1=50") != std::string::npos);

    // a current sequence is left alone
    Effect* cur = f.layer->AddEffect(0, "Liquid", "E_TEXTCTRL_Velocity1=5", kRed, 0, 1000, 0, false);
    fx->adjustSettings("2026.10", cur, false);
    CHECK(cur->GetSettings().Get("E_TEXTCTRL_Velocity1", "") == "5");
}

TEST_CASE("Liquid renders deterministically", "[effects][classic1]") {
    Fixture a, b;
    std::string s = "E_CHECKBOX_Enabled2=1,E_CHOICE_ParticleType=Water";
    CHECK(a.Render("Liquid", s, kRedBlue, 16, 16, Range(0, 8)) == b.Render("Liquid", s, kRedBlue, 16, 16, Range(0, 8)));
}

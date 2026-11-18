// Text, Shape and Sketch effects, plus the sketch path model and SVG import.
//
// Text has two render paths: the built-in bitmap "xLights fonts" (FontManager,
// pure integer pixel copies) and the OS-font path that goes through
// TextDrawingContext. Unit tests have no platform text backend, so the OS-font
// path and Shape's emoji are driven through a small block-glyph context
// registered here: every glyph is a solid block, and every string drawn is
// logged, which makes the formatted text (countdowns, word flips, lyrics)
// directly observable without any system font.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <list>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "effects/SketchEffectDrawing.h"
#include "effects/SketchSVGImport.h"
#include "media/AudioManager.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/FontManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "render/TextDrawingContext.h"
#include "utils/UtilClasses.h"

#include "../support/TempDir.h"

#ifdef DrawText
#undef DrawText
#endif

using Catch::Matchers::WithinAbs;

namespace {

const char* kRed = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";
const char* kRedBlue = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#0000FF,C_CHECKBOX_Palette2=1";

using Settings = std::vector<std::pair<std::string, std::string>>;
using Pixels = std::vector<xlColor>;

bool IsLit(const xlColor& c) {
    return c.alpha != 0 && (c.red || c.green || c.blue);
}

struct Box {
    int minX = 1 << 30, minY = 1 << 30, maxX = -1, maxY = -1;
    bool empty() const { return maxX < 0; }
    int width() const { return empty() ? 0 : maxX - minX + 1; }
    int height() const { return empty() ? 0 : maxY - minY + 1; }
};

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) {
            if (IsLit(c)) ++n;
        }
        return n;
    }
    Box box(size_t frame) const {
        Box b;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (IsLit(at(frame, x, y))) {
                    b.minX = std::min(b.minX, x);
                    b.maxX = std::max(b.maxX, x);
                    b.minY = std::min(b.minY, y);
                    b.maxY = std::max(b.maxY, y);
                }
            }
        }
        return b;
    }
    double cx(size_t frame) const {
        double s = 0;
        int n = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                if (IsLit(at(frame, x, y))) { s += x; ++n; }
        return n ? s / n : -1;
    }
    double cy(size_t frame) const {
        double s = 0;
        int n = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                if (IsLit(at(frame, x, y))) { s += y; ++n; }
        return n ? s / n : -1;
    }
    int maxRed(size_t frame) const {
        int m = 0;
        for (const auto& c : f[frame]) {
            if (IsLit(c)) m = std::max(m, (int)c.red * c.alpha / 255);
        }
        return m;
    }
    // every lit pixel passes pred
    template <class P>
    bool allLit(size_t frame, P pred) const {
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                if (IsLit(at(frame, x, y)) && !pred(at(frame, x, y), x, y)) return false;
        return true;
    }
    bool same(size_t a, const Frames& o, size_t b) const {
        if (w != o.w || h != o.h) return false;
        for (size_t i = 0; i < f[a].size(); ++i) {
            const auto& p = f[a][i];
            const auto& q = o.f[b][i];
            if (p.red != q.red || p.green != q.green || p.blue != q.blue || p.alpha != q.alpha) return false;
        }
        return true;
    }
    bool operator==(const Frames& o) const { return w == o.w && h == o.h && f == o.f; }
};

bool RedOnly(const xlColor& c, int, int) { return c.green == 0 && c.blue == 0 && c.red > 0; }
bool PureRed(const xlColor& c, int, int) { return c.red == 255 && c.green == 0 && c.blue == 0; }
bool PureBlue(const xlColor& c, int, int) { return c.red == 0 && c.green == 0 && c.blue == 255; }

// Every lit pixel of frame a reappears at (x+dx, y+dy) in frame b, and nothing else is lit.
bool Shifted(const Frames& a, size_t fa, const Frames& b, size_t fb, int dx, int dy) {
    if (a.lit(fa) == 0 || a.lit(fa) != b.lit(fb)) return false;
    for (int y = 0; y < a.h; ++y) {
        for (int x = 0; x < a.w; ++x) {
            if (!IsLit(a.at(fa, x, y))) continue;
            int nx = x + dx, ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= b.w || ny >= b.h) return false;
            const auto& p = a.at(fa, x, y);
            const auto& q = b.at(fb, nx, ny);
            if (p.red != q.red || p.green != q.green || p.blue != q.blue) return false;
        }
    }
    return true;
}

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

// ---------------------------------------------------------------------------
// Block-glyph text backend
// ---------------------------------------------------------------------------
struct TextLog {
    std::vector<std::string> drawn;
    std::vector<double> rotations;
};
TextLog& Log() {
    static TextLog log;
    return log;
}
bool Drew(const std::string& s) {
    const auto& d = Log().drawn;
    return std::find(d.begin(), d.end(), s) != d.end();
}

TextFontInfo ParseFakeFont(const std::string& s) {
    TextFontInfo f;
    f.faceName = s;
    f.pixelSize = 6;
    // last integer token is the pixel size
    size_t pos = s.find_last_of("0123456789");
    if (pos != std::string::npos) {
        size_t start = pos;
        while (start > 0 && std::isdigit((unsigned char)s[start - 1])) --start;
        f.pixelSize = (int)std::strtol(s.substr(start, pos - start + 1).c_str(), nullptr, 10);
    }
    return f;
}

class BlockTextContext : public TextDrawingContext {
public:
    void ResetSize(int w, int h) override {
        _w = std::max(0, w);
        _h = std::max(0, h);
        _px.assign((size_t)_w * _h * 4, 0);
    }
    size_t GetWidth() const override { return _w; }
    size_t GetHeight() const override { return _h; }
    void Clear() override { std::fill(_px.begin(), _px.end(), 0); }
    const uint8_t* FlushAndGetImage(int* width, int* height) override {
        *width = _w;
        *height = _h;
        return _px.data();
    }
    void SetFont(const TextFontInfo& font, const xlColor& color) override {
        _font = font;
        _color = color;
    }
    void DrawText(const std::string& msg, int x, int y, double rotation) override {
        Log().rotations.push_back(rotation);
        Draw(msg, x, y);
    }
    void DrawText(const std::string& msg, int x, int y) override { Draw(msg, x, y); }
    void GetTextExtent(const std::string& msg, double* width, double* height) override {
        int lines = 0, longest = 0, cur = 0;
        bool any = false;
        for (size_t i = 0; i < msg.size(); ++i) {
            unsigned char c = (unsigned char)msg[i];
            if (c == '\n') {
                longest = std::max(longest, cur);
                cur = 0;
                ++lines;
                continue;
            }
            any = true;
            if ((c & 0xC0) != 0x80) ++cur;
        }
        longest = std::max(longest, cur);
        if (any || lines) ++lines;
        *width = longest * GW();
        *height = msg.empty() ? 0 : lines * GH();
    }
    void GetTextExtents(const std::string& msg, std::vector<double>& extents) override {
        extents.clear();
        int n = 0;
        for (unsigned char c : msg) {
            if ((c & 0xC0) != 0x80) ++n;
            extents.push_back(n * GW());
        }
    }
    void SetOverlayMode(bool) override {}

private:
    int GW() const { return std::max(1, _font.pixelSize / 2); }
    int GH() const { return std::max(1, _font.pixelSize); }
    void Draw(const std::string& msg, int x, int y) {
        Log().drawn.push_back(msg);
        int col = 0, row = 0;
        for (size_t i = 0; i < msg.size(); ++i) {
            unsigned char c = (unsigned char)msg[i];
            if (c == '\n') {
                col = 0;
                ++row;
                continue;
            }
            if ((c & 0xC0) == 0x80) continue;
            if (c != ' ') {
                int fillW = GW() > 1 ? GW() - 1 : 1;
                for (int yy = 0; yy < GH(); ++yy) {
                    for (int xx = 0; xx < fillW; ++xx) {
                        int px = x + col * GW() + xx;
                        int py = y + row * GH() + yy;
                        if (px < 0 || py < 0 || px >= _w || py >= _h) continue;
                        uint8_t* p = &_px[((size_t)py * _w + px) * 4];
                        p[0] = _color.red;
                        p[1] = _color.green;
                        p[2] = _color.blue;
                        p[3] = 255;
                    }
                }
            }
            ++col;
        }
    }

    int _w = 0;
    int _h = 0;
    std::vector<uint8_t> _px;
    TextFontInfo _font;
    xlColor _color;
};

// Installs the block backend for the lifetime of a fixture and restores the
// "no backend" state that the rest of the suite runs with.
struct BlockTextBackend {
    BlockTextBackend() {
        TextDrawingContext::CleanUp();
        TextDrawingContext::RegisterFactory(
            [](int w, int h, bool) -> TextDrawingContext* {
                auto* c = new BlockTextContext();
                c->ResetSize(w, h);
                return c;
            },
            ParseFakeFont, ParseFakeFont);
        TextDrawingContext::Initialize();
        Log() = TextLog();
    }
    ~BlockTextBackend() {
        TextDrawingContext::CleanUp();
        TextDrawingContext::RegisterFactory(nullptr, nullptr, nullptr);
    }
};

// ---------------------------------------------------------------------------
// Direct render fixture (same shape as test_effects_render.cpp's RenderFixture,
// with settings applied to the effect's map so values may contain commas)
// ---------------------------------------------------------------------------
struct RenderOpts {
    int endMs = 1000;
    AudioManager* media = nullptr;
    bool allowAlpha = false;
};

struct Fixture {
    std::unique_ptr<BlockTextBackend> backend; // first: outlives the buffers below
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> model;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;
    Effect* last = nullptr;

    explicit Fixture(bool withTextBackend = true) {
        if (withTextBackend) backend = std::make_unique<BlockTextBackend>();
        model.reset(ctx.AllModels.CreateDefaultModel("Single Line"));
        element = ctx._sequenceElements.AddElement("Fixture", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    Effect* Add(const std::string& name, const Settings& settings, const std::string& palette, int endMs = 1000) {
        Effect* e = layer->AddEffect(0, name, "", palette, 0, endMs, 0, false);
        REQUIRE(e != nullptr);
        for (const auto& [k, v] : settings) e->GetSettings()[k] = v;
        last = e;
        return e;
    }

    Frames Render(const std::string& name, const Settings& settings, const std::string& palette,
                  int w, int h, const std::vector<int>& periods, const RenderOpts& opts = RenderOpts()) {
        Effect* e = Add(name, settings, palette, opts.endMs);
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);

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
        rb.SetEffectDuration(0, opts.endMs);
        rb.SetAllowAlphaChannel(opts.allowAlpha);
        rb._mediaOverride = opts.media;

        SettingsMap renderSettings;
        e->CopySettingsMap(renderSettings, true);

        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            rb.SetState(p, first);
            first = false;
            fx->Render(e, renderSettings, rb);
            out.f.emplace_back(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
        }
        return out;
    }

    // A timing track; layers holds the marks of each layer as (label, startMs, endMs).
    Element* AddTiming(const std::string& name, const std::vector<std::vector<std::tuple<std::string, int, int>>>& layers) {
        Element* t = ctx._sequenceElements.AddElement(name, "timing", true, false, true, false, false);
        REQUIRE(t != nullptr);
        while (t->GetEffectLayerCount() < layers.size()) t->AddEffectLayer();
        for (size_t l = 0; l < layers.size(); ++l) {
            for (const auto& [label, s, e] : layers[l]) {
                t->GetEffectLayer(l)->AddEffect(0, label, "", "", s, e, 0, false);
            }
        }
        return t;
    }
};

Settings XL(const std::string& text, const std::string& font = "5-5x5 Thin", Settings extra = {}) {
    Settings s = { { "E_CHOICE_Text_Font", font }, { "E_TEXTCTRL_Text", text } };
    s.insert(s.end(), extra.begin(), extra.end());
    return s;
}

Settings OS(const std::string& text, const std::string& font, Settings extra = {}) {
    Settings s = { { "E_FONTPICKER_Text_Font", font }, { "E_TEXTCTRL_Text", text } };
    s.insert(s.end(), extra.begin(), extra.end());
    return s;
}

// In the first run of lit frames, the frames where the whole text is on screen
// (the most lit pixels) must advance strictly in one direction along the axis.
bool FirstRunMoves(const Frames& fr, bool xAxis, int sign, int minFull = 3) {
    size_t i = 0;
    while (i < fr.f.size() && fr.lit(i) == 0) ++i;
    size_t end = i;
    int full = 0;
    while (end < fr.f.size() && fr.lit(end) > 0) {
        full = std::max(full, fr.lit(end));
        ++end;
    }
    if (full == 0) return false;
    int n = 0;
    double prev = 0;
    for (size_t j = i; j < end; ++j) {
        if (fr.lit(j) != full) continue;
        double c = xAxis ? fr.cx(j) : fr.cy(j);
        if (n > 0 && (c - prev) * sign <= 0) return false;
        prev = c;
        ++n;
    }
    return n >= minFull;
}

std::string Wav(int sampleRate, const std::vector<int16_t>& samples) {
    auto put16 = [](std::string& s, uint16_t v) { s.push_back(char(v & 0xFF)); s.push_back(char(v >> 8)); };
    auto put32 = [](std::string& s, uint32_t v) { for (int i = 0; i < 4; i++) s.push_back(char((v >> (8 * i)) & 0xFF)); };
    std::string s;
    uint32_t dataBytes = uint32_t(samples.size() * 2);
    s += "RIFF";
    put32(s, 36 + dataBytes);
    s += "WAVEfmt ";
    put32(s, 16);
    put16(s, 1);
    put16(s, 1);
    put32(s, uint32_t(sampleRate));
    put32(s, uint32_t(sampleRate * 2));
    put16(s, 2);
    put16(s, 16);
    s += "data";
    put32(s, dataBytes);
    for (int16_t v : samples) put16(s, uint16_t(v));
    return s;
}

const char* kSquareSvg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="10" height="10" viewBox="0 0 10 10"><rect x="0" y="0" width="10" height="10" fill="#ffffff"/></svg>)";

} // namespace

// ===========================================================================
// Text: built-in xLights bitmap fonts
// ===========================================================================

TEST_CASE("Text xLights font draws glyphs centred, one palette colour per letter", "[effects][drawing]") {
    Fixture f;
    Frames fr = f.Render("Text", XL("HI"), kRedBlue, 30, 10, { 0, 7 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        REQUIRE(fr.lit(i) > 0);
        // H takes the first colour, I the second: every red pixel is left of every blue one
        int maxRedX = -1, minBlueX = 1000;
        CHECK(fr.allLit(i, [&](const xlColor& c, int x, int) {
            if (PureRed(c, x, 0)) maxRedX = std::max(maxRedX, x);
            if (PureBlue(c, x, 0)) minBlueX = std::min(minBlueX, x);
            return PureRed(c, x, 0) || PureBlue(c, x, 0);
        }));
        CHECK(maxRedX >= 0);
        CHECK(minBlueX < 1000);
        CHECK(maxRedX < minBlueX);
        Box b = fr.box(i);
        CHECK(b.height() <= 5);
        CHECK(std::abs((b.minX + b.maxX) / 2.0 - 15.0) <= 2.0);
        CHECK(std::abs((b.minY + b.maxY) / 2.0 - 5.0) <= 2.0);
    }
    // static text: every frame identical
    CHECK(fr.same(0, fr, 1));
}

TEST_CASE("Text xLights fonts all render and larger fonts draw taller glyphs", "[effects][drawing]") {
    Fixture f;
    FontManager::instance().init();
    const auto& names = FontManager::get_font_names();
    REQUIRE(names.size() >= 17);
    std::set<std::vector<uint8_t>> distinct;
    for (const auto& n : names) {
        INFO(n);
        Frames fr = f.Render("Text", XL("AB8", n), kRed, 40, 20, { 0 });
        CHECK(fr.lit(0) > 0);
        CHECK(fr.allLit(0, PureRed));
        std::vector<uint8_t> key;
        for (const auto& c : fr.f[0]) key.push_back(IsLit(c));
        distinct.insert(key);
    }
    CHECK(distinct.size() >= names.size() - 2);
    Frames small = f.Render("Text", XL("H", "5-5x5 Thin"), kRed, 40, 20, { 0 });
    Frames big = f.Render("Text", XL("H", "12-15x15 Bold"), kRed, 40, 20, { 0 });
    CHECK(big.box(0).height() > small.box(0).height());
    CHECK(big.box(0).width() > small.box(0).width());
}

TEST_CASE("Text xLights font scrolls in the configured direction", "[effects][drawing]") {
    Fixture f;
    const auto periods = Range(0, 39);
    RenderOpts o;
    o.endMs = 2000;
    Frames left = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "left" } }), kRed, 12, 8, periods, o);
    Frames right = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "right" } }), kRed, 12, 8, periods, o);
    Frames up = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "up" } }), kRed, 8, 12, periods, o);
    Frames down = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "down" } }), kRed, 8, 12, periods, o);
    CHECK(FirstRunMoves(left, true, -1));
    CHECK(FirstRunMoves(right, true, +1));
    CHECK(FirstRunMoves(up, false, +1));
    CHECK(FirstRunMoves(down, false, -1));
}

TEST_CASE("Text xLights font scrolling to centre stops where static text sits", "[effects][drawing]") {
    Fixture f;
    RenderOpts o;
    o.endMs = 2000;
    Frames still = f.Render("Text", XL("I"), kRed, 12, 8, { 0 }, o);
    Frames left = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "left" }, { "E_CHECKBOX_TextToCenter", "1" } }), kRed, 12, 8, { 0, 5, 38, 39 }, o);
    CHECK_FALSE(left.same(1, still, 0));
    CHECK(left.same(2, still, 0));
    CHECK(left.same(3, still, 0));
}

TEST_CASE("Text xLights font no-repeat scrolls off once and stays blank", "[effects][drawing]") {
    Fixture f;
    RenderOpts o;
    o.endMs = 6000;
    const auto periods = Range(0, 119);
    Frames once = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "left" }, { "E_CHECKBOX_TextNoRepeat", "1" } }), kRed, 12, 8, periods, o);
    Frames loop = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "left" } }), kRed, 12, 8, periods, o);
    int litOnce = 0, litOnceTail = 0, litLoopTail = 0;
    for (size_t i = 0; i < periods.size(); ++i) {
        if (once.lit(i)) ++litOnce;
        if (i >= 60 && once.lit(i)) ++litOnceTail;
        if (i >= 60 && loop.lit(i)) ++litLoopTail;
    }
    CHECK(litOnce > 0);
    CHECK(litOnceTail == 0);
    CHECK(litLoopTail > 0);
}

TEST_CASE("Text xLights font start offsets shift the text by pixels or percent", "[effects][drawing]") {
    Fixture f;
    Frames base = f.Render("Text", XL("I"), kRed, 40, 12, { 0 });
    Frames px = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_SLIDER_Text_XStart", "3" }, { "E_SLIDER_Text_YStart", "2" }, { "E_CHECKBOX_Text_PixelOffsets", "1" } }), kRed, 40, 12, { 0 });
    Frames pct = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_SLIDER_Text_XStart", "25" } }), kRed, 40, 12, { 0 });
    CHECK(Shifted(base, 0, px, 0, 3, 2));
    CHECK(Shifted(base, 0, pct, 0, 10, 0));
}

TEST_CASE("Text xLights font vector motion travels from start to end", "[effects][drawing]") {
    Fixture f;
    Frames fr = f.Render("Text", XL("I", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "vector" }, { "E_SLIDER_Text_XStart", "-30" }, { "E_SLIDER_Text_XEnd", "30" }, { "E_SLIDER_Text_YStart", "-20" }, { "E_SLIDER_Text_YEnd", "20" } }), kRed, 30, 20, Range(0, 19));
    REQUIRE(fr.lit(0) > 0);
    REQUIRE(fr.lit(19) > 0);
    CHECK(fr.cx(19) > fr.cx(0) + 10);
    CHECK(fr.cy(19) > fr.cy(0) + 5);
    for (int i = 1; i < 20; ++i) CHECK(fr.cx(i) >= fr.cx(i - 1));
}

TEST_CASE("Text xLights font vertical and rotated text stack the glyphs", "[effects][drawing]") {
    Fixture f;
    Frames flat = f.Render("Text", XL("AB"), kRed, 30, 30, { 0 });
    Frames vup = f.Render("Text", XL("AB", "5-5x5 Thin", { { "E_CHOICE_Text_Effect", "vert text up" } }), kRed, 30, 30, { 0 });
    Frames vdown = f.Render("Text", XL("AB", "5-5x5 Thin", { { "E_CHOICE_Text_Effect", "vert text down" } }), kRed, 30, 30, { 0 });
    Frames rup = f.Render("Text", XL("AB", "5-5x5 Thin", { { "E_CHOICE_Text_Effect", "rotate up 90" } }), kRed, 30, 30, { 0 });
    Frames rdown = f.Render("Text", XL("AB", "5-5x5 Thin", { { "E_CHOICE_Text_Effect", "rotate down 90" } }), kRed, 30, 30, { 0 });
    CHECK(flat.box(0).width() > flat.box(0).height());
    for (const Frames* v : { &vup, &vdown, &rup, &rdown }) {
        REQUIRE(v->lit(0) > 0);
        CHECK(v->box(0).height() > v->box(0).width());
        CHECK(v->allLit(0, PureRed));
        // rotation and stacking move pixels, they don't add or drop them
        CHECK(v->lit(0) == flat.lit(0));
    }
    CHECK_FALSE(vup.same(0, vdown, 0));
    CHECK_FALSE(rup.same(0, rdown, 0));
    CHECK_FALSE(vup.same(0, rup, 0));
}

TEST_CASE("Text xLights font colours per letter or per word", "[effects][drawing]") {
    Fixture f;
    Frames letters = f.Render("Text", XL("AB CD"), kRedBlue, 40, 10, { 0 });
    Frames words = f.Render("Text", XL("AB CD", "5-5x5 Thin", { { "E_CHECKBOX_Text_Color_PerWord", "1" } }), kRedBlue, 40, 10, { 0 });
    Box b = words.box(0);
    int mid = (b.minX + b.maxX) / 2;
    auto leftRed = [mid](const xlColor& c, int x, int) { return x >= mid || PureRed(c, x, 0); };
    auto rightBlue = [mid](const xlColor& c, int x, int) { return x <= mid || PureBlue(c, x, 0); };
    CHECK(words.allLit(0, leftRed));
    CHECK(words.allLit(0, rightBlue));
    // per letter: the first word already alternates colours
    bool redLeft = false, blueLeft = false;
    for (int y = 0; y < letters.h; ++y) {
        for (int x = 0; x < mid; ++x) {
            const auto& c = letters.at(0, x, y);
            if (!IsLit(c)) continue;
            redLeft |= PureRed(c, x, y);
            blueLeft |= PureBlue(c, x, y);
        }
    }
    CHECK(redLeft);
    CHECK(blueLeft);
}

TEST_CASE("Text xLights font seconds countdown shows the remaining seconds", "[effects][drawing]") {
    Fixture f;
    RenderOpts o;
    o.endMs = 4000;
    Frames cd = f.Render("Text", XL("5", "7-7x9 Thin", { { "E_CHOICE_Text_Count", "seconds" } }), kRed, 20, 12, { 0, 40, 79 }, o);
    Frames five = f.Render("Text", XL("5", "7-7x9 Thin"), kRed, 20, 12, { 0 });
    Frames three = f.Render("Text", XL("3", "7-7x9 Thin"), kRed, 20, 12, { 0 });
    Frames two = f.Render("Text", XL("2", "7-7x9 Thin"), kRed, 20, 12, { 0 });
    CHECK(cd.same(0, five, 0));
    CHECK(cd.same(1, three, 0));
    CHECK(cd.same(2, two, 0));
}

TEST_CASE("Text xLights font minutes-seconds countdown drops the spaces around the colon", "[effects][drawing]") {
    Fixture f;
    Frames cd = f.Render("Text", XL("1:05", "5-5x5 Thin", { { "E_CHOICE_Text_Count", "minutes seconds" } }), kRed, 40, 10, { 0 });
    Frames lit = f.Render("Text", XL(" 1:05"), kRed, 40, 10, { 0 });
    CHECK(cd.same(0, lit, 0));
}

TEST_CASE("Text xLights font reads text from a file the same as typed text", "[effects][drawing]") {
    TempDir td;
    std::string path = td.Write("lines.txt", "AB\r\nCD\n\n");
    Fixture f;
    Frames fromFile = f.Render("Text", { { "E_CHOICE_Text_Font", "5-5x5 Thin" }, { "E_FILEPICKERCTRL_Text_File", path } }, kRed, 20, 16, { 0 });
    Frames typed = f.Render("Text", XL("AB\\nCD"), kRed, 20, 16, { 0 });
    REQUIRE(typed.lit(0) > 0);
    CHECK(typed.box(0).height() > 6); // two lines
    CHECK(fromFile == typed);
}

TEST_CASE("Text xLights font shows the current lyric phrase or word", "[effects][drawing]") {
    Fixture f;
    f.AddTiming("Lyrics", { { { "AB", 0, 500 }, { "CD", 500, 1000 } }, { { "X", 0, 1000 } } });
    Frames phrases = f.Render("Text", { { "E_CHOICE_Text_Font", "5-5x5 Thin" }, { "E_CHOICE_Text_LyricTrack", "Lyrics - Phrases" } }, kRed, 20, 8, { 2, 15 });
    Frames words = f.Render("Text", { { "E_CHOICE_Text_Font", "5-5x5 Thin" }, { "E_CHOICE_Text_LyricTrack", "Lyrics - Words" } }, kRed, 20, 8, { 2 });
    Frames ab = f.Render("Text", XL("AB"), kRed, 20, 8, { 0 });
    Frames cd = f.Render("Text", XL("CD"), kRed, 20, 8, { 0 });
    Frames x = f.Render("Text", XL("X"), kRed, 20, 8, { 0 });
    CHECK(phrases.same(0, ab, 0));
    CHECK(phrases.same(1, cd, 0));
    CHECK(words.same(0, x, 0));
}

TEST_CASE("Text xLights font word flip shows one word at a time", "[effects][drawing]") {
    Fixture f;
    Settings flip = XL("AB CD EF", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "word-flip" }, { "E_TEXTCTRL_Text_Speed", "1" } });
    Frames fr = f.Render("Text", flip, kRed, 60, 8, { 0, 8, 15 });
    const char* words[] = { "AB", "CD", "EF" };
    for (int i = 0; i < 3; ++i) {
        Frames w = f.Render("Text", XL(words[i]), kRed, 60, 8, { 0 });
        INFO(words[i]);
        // the frame holds exactly that word's glyphs (its placement is checked in the known-bug case below)
        int dx = fr.box(i).minX - w.box(0).minX;
        CHECK(Shifted(w, 0, fr, i, dx, 0));
    }
    CHECK_FALSE(fr.same(0, fr, 1));
    CHECK_FALSE(fr.same(1, fr, 2));
}

// each word is placed by its own width, not the width of the whole line
TEST_CASE("Text xLights font word flip centres the shown word like static text", "[effects][drawing]") {
    Fixture f;
    Settings flip = XL("AB CD EF", "5-5x5 Thin", { { "E_CHOICE_Text_Dir", "word-flip" }, { "E_TEXTCTRL_Text_Speed", "1" } });
    Frames fr = f.Render("Text", flip, kRed, 20, 8, { 0, 8, 15 });
    const char* words[] = { "AB", "CD", "EF" };
    for (int i = 0; i < 3; ++i) {
        Frames w = f.Render("Text", XL(words[i]), kRed, 20, 8, { 0 });
        INFO(words[i]);
        CHECK(fr.same(i, w, 0));
    }
}

TEST_CASE("Text xLights font sweep of every direction and option", "[effects][drawing]") {
    Fixture f;
    const char* dirs[] = { "none", "left", "right", "up", "down", "vector", "up-left", "down-left", "up-right", "down-right", "wavey", "word-flip", "left-right", "up-down" };
    const char* effects[] = { "normal", "vert text up", "rotate down 90" };
    RenderOpts o;
    o.endMs = 2000;
    const std::vector<int> periods = Range(0, 39);
    for (const char* d : dirs) {
        for (const char* e : effects) {
            for (int opt = 0; opt < 3; ++opt) {
                Settings s = XL("AB CD", "6-5x6 Thin", { { "E_CHOICE_Text_Dir", d }, { "E_CHOICE_Text_Effect", e }, { "E_TEXTCTRL_Text_Speed", "20" } });
                if (opt == 1) s.push_back({ "E_CHECKBOX_TextToCenter", "1" });
                if (opt == 2) s.push_back({ "E_CHECKBOX_TextNoRepeat", "1" });
                INFO(d << " / " << e << " / opt " << opt);
                Frames fr = f.Render("Text", s, kRed, 16, 16, periods, o);
                int lit = 0;
                for (size_t i = 0; i < fr.f.size(); ++i) {
                    lit += fr.lit(i);
                    CHECK(fr.allLit(i, PureRed));
                }
                // Tall vertical text moving down-right with no-repeat ends its single pass when it
                // has crossed horizontally, before it has come down into view, so it never shows.
                bool neverVisible = opt == 2 && std::string(d) == "down-right" && std::string(e) == "vert text up";
                if (!neverVisible) CHECK(lit > 0);
            }
        }
    }
}

TEST_CASE("Text effect with an unknown xLights font name falls back to OS fonts", "[effects][drawing]") {
    Fixture f;
    Frames fr = f.Render("Text", { { "E_CHOICE_Text_Font", "No Such Font" }, { "E_TEXTCTRL_Text", "QZ" }, { "E_FONTPICKER_Text_Font", "fallback 4" } }, kRed, 20, 8, { 0 });
    CHECK(Drew("QZ"));
    CHECK(fr.lit(0) == 8);
}

// ===========================================================================
// Text: OS-font path through TextDrawingContext
// ===========================================================================

TEST_CASE("Text OS font draws the string once, centred, in the palette colour", "[effects][drawing]") {
    Fixture f;
    Frames fr = f.Render("Text", OS("AB", "os-static 4"), kRed, 20, 8, { 0, 3 });
    CHECK(Drew("AB"));
    for (size_t i = 0; i < fr.f.size(); ++i) {
        // two glyphs, each 1 column x 4 rows
        CHECK(fr.lit(i) == 8);
        CHECK(fr.allLit(i, PureRed));
        Box b = fr.box(i);
        CHECK(b.height() == 4);
        CHECK(std::abs((b.minX + b.maxX) / 2.0 - 10.0) <= 2.0);
    }
    CHECK(fr.same(0, fr, 1));
}

TEST_CASE("Text OS font draws letters or words in successive palette colours", "[effects][drawing]") {
    Fixture f;
    Frames letters = f.Render("Text", OS("AB", "os-letters 4"), kRedBlue, 20, 8, { 0 });
    CHECK(Drew("A"));
    CHECK(Drew("B"));
    Box b = letters.box(0);
    CHECK(PureRed(letters.at(0, b.minX, b.minY), 0, 0));
    CHECK(PureBlue(letters.at(0, b.maxX, b.minY), 0, 0));

    Frames words = f.Render("Text", OS("AB CD", "os-words 4", { { "E_CHECKBOX_Text_Color_PerWord", "1" } }), kRedBlue, 24, 8, { 0 });
    Box wb = words.box(0);
    int mid = (wb.minX + wb.maxX) / 2;
    CHECK(words.allLit(0, [mid](const xlColor& c, int x, int) { return x < mid ? PureRed(c, x, 0) : PureBlue(c, x, 0); }));
}

TEST_CASE("Text OS font countdown formats", "[effects][drawing]") {
    Fixture f;
    RenderOpts o;
    o.endMs = 4000;

    SECTION("seconds") {
        f.Render("Text", OS("5", "os-cd-s 4", { { "E_CHOICE_Text_Count", "seconds" } }), kRed, 20, 8, { 0, 40, 79 }, o);
        CHECK(Drew("5"));
        CHECK(Drew("3"));
        CHECK(Drew("2"));
    }
    SECTION("minutes seconds with prefix and suffix") {
        f.Render("Text", OS("T-/1:05/!", "os-cd-ms 4", { { "E_CHOICE_Text_Count", "minutes seconds" } }), kRed, 40, 8, { 0, 20 }, o);
        CHECK(Drew("T- 1 : 05!"));
        CHECK(Drew("T- 1 : 04!"));
    }
    SECTION("minutes seconds from plain seconds and a bad format") {
        f.Render("Text", OS("90", "os-cd-ms2 4", { { "E_CHOICE_Text_Count", "minutes seconds" } }), kRed, 40, 8, { 0 }, o);
        CHECK(Drew(" 1 : 30"));
        f.Render("Text", OS("1:2:3", "os-cd-ms3 4", { { "E_CHOICE_Text_Count", "minutes seconds" } }), kRed, 40, 8, { 0 }, o);
        CHECK(Drew("Invalid Format"));
    }
    SECTION("to-date formats against a far-future date") {
        const std::string future = "Fri, 01 Jan 2100 00:00:00 +0000";
        f.Render("Text", OS(future, "os-cd-d1 4", { { "E_CHOICE_Text_Count", "to date 'd h m s'" } }), kRed, 40, 8, { 0, 1 }, o);
        f.Render("Text", OS(future, "os-cd-d2 4", { { "E_CHOICE_Text_Count", "to date 'h:m:s'" } }), kRed, 40, 8, { 0 }, o);
        f.Render("Text", OS(future, "os-cd-d3 4", { { "E_CHOICE_Text_Count", "to date 'm' or 's'" } }), kRed, 40, 8, { 0 }, o);
        f.Render("Text", OS(future, "os-cd-d4 4", { { "E_CHOICE_Text_Count", "to date 's'" } }), kRed, 40, 8, { 0 }, o);
        f.Render("Text", OS("!" + future + "!%D days %H:%M:%S %E %l %% %Q", "os-cd-d5 4", { { "E_CHOICE_Text_Count", "!to date!%fmt" } }), kRed, 80, 8, { 0 }, o);
        const std::regex dhms("^([0-9]+)d [0-9]+h [0-9]+m [0-9]+s$");
        const std::regex hms("^[0-9]+ : [0-9]+ : [0-9]+$");
        const std::regex minutes("^[0-9]+ m$");
        const std::regex secs("^[0-9]+$");
        const std::regex freefmt("^[0-9]+ days [0-9]{2,}:[0-9]{2}:[0-9]{2} [0-9]+ 000 % %Q$");
        bool sawDhms = false, sawHms = false, sawMin = false, sawSecs = false, sawFree = false;
        for (const auto& s : Log().drawn) {
            std::smatch m;
            if (std::regex_match(s, m, dhms)) {
                sawDhms = true;
                CHECK(std::strtol(m[1].str().c_str(), nullptr, 10) > 20000);
            }
            sawHms |= std::regex_match(s, hms);
            sawMin |= std::regex_match(s, minutes);
            sawSecs |= std::regex_match(s, secs);
            sawFree |= std::regex_match(s, freefmt);
        }
        CHECK(sawDhms);
        CHECK(sawHms);
        CHECK(sawMin);
        CHECK(sawSecs);
        CHECK(sawFree);
    }
    SECTION("past, unparsable dates and bad free formats") {
        f.Render("Text", OS("Mon, 01 Jan 2001 00:00:00 +0000", "os-cd-p1 4", { { "E_CHOICE_Text_Count", "to date 'd h m s'" } }), kRed, 40, 8, { 0 }, o);
        CHECK(Drew("invalid date"));
        Log() = TextLog();
        f.Render("Text", OS("not a date", "os-cd-p2 4", { { "E_CHOICE_Text_Count", "to date 's'" } }), kRed, 40, 8, { 0 }, o);
        CHECK(Drew("invalid date"));
        f.Render("Text", OS("!Fri, 01 Jan 2100 00:00:00 -0500!%H %", "os-cd-p3 4", { { "E_CHOICE_Text_Count", "!to date!%fmt" } }), kRed, 40, 8, { 0 }, o);
        CHECK(Drew("invalid format"));
        f.Render("Text", OS("!x", "os-cd-p4 4", { { "E_CHOICE_Text_Count", "!to date!%fmt" } }), kRed, 40, 8, { 0 }, o);
        CHECK(Drew("invalid date"));
    }
}

TEST_CASE("Text OS font substitutes variables and case markers", "[effects][drawing]") {
    Fixture f;
    f.Render("Text", OS("ab${UPPER}c", "os-var1 4"), kRed, 40, 8, { 0 });
    f.Render("Text", OS("X${LOWER}yZ", "os-var2 4"), kRed, 40, 8, { 0 });
    f.Render("Text", OS("[${ARTIST}${ALBUM}${AUTHOR}${AUTHOREMAIL}${COMMENT}${URL}${WEBSITE}${SONG}]", "os-var3 4"), kRed, 40, 8, { 0 });
    CHECK(Drew("ABC"));
    CHECK(Drew("xyz"));
    CHECK(Drew("[]"));
}

TEST_CASE("Text OS font reads files, lyric tracks and flips words", "[effects][drawing]") {
    TempDir td;
    std::string path = td.Write("os.txt", "LINE1\r\nLINE2\n");
    Fixture f;
    f.Render("Text", { { "E_FONTPICKER_Text_Font", "os-file 4" }, { "E_FILEPICKERCTRL_Text_File", path } }, kRed, 40, 12, { 0 });
    CHECK(Drew("LINE1"));
    CHECK(Drew("LINE2"));

    f.AddTiming("Song", { { { "HELLO", 0, 500 }, { "BYE", 500, 1000 } }, { { "W1", 0, 1000 } } });
    f.Render("Text", { { "E_FONTPICKER_Text_Font", "os-lyr 4" }, { "E_CHOICE_Text_LyricTrack", "Song - Phrases" } }, kRed, 40, 8, { 2, 15 });
    f.Render("Text", { { "E_FONTPICKER_Text_Font", "os-lyr 4" }, { "E_CHOICE_Text_LyricTrack", "Song - Words" } }, kRed, 40, 8, { 2 });
    CHECK(Drew("HELLO"));
    CHECK(Drew("BYE"));
    CHECK(Drew("W1"));

    Log() = TextLog();
    f.Render("Text", OS("ONE TWO THREE", "os-flip 4", { { "E_CHOICE_Text_Dir", "word-flip" }, { "E_TEXTCTRL_Text_Speed", "1" } }), kRed, 40, 8, { 0, 8, 15 });
    CHECK(Log().drawn == std::vector<std::string>({ "ONE", "TWO", "THREE" }));

    Log() = TextLog();
    f.Render("Text", OS("ONE TWO THREE", "os-flip0 4", { { "E_CHOICE_Text_Dir", "word-flip" }, { "E_TEXTCTRL_Text_Speed", "0" } }), kRed, 40, 8, { 0, 15 });
    CHECK(Drew("ONE"));
    CHECK_FALSE(Drew("THREE"));
}

TEST_CASE("Text OS font scrolls in the configured direction", "[effects][drawing]") {
    Fixture f;
    RenderOpts o;
    o.endMs = 2000;
    const auto periods = Range(0, 39);
    Frames left = f.Render("Text", OS("I", "os-left 4", { { "E_CHOICE_Text_Dir", "left" } }), kRed, 12, 8, periods, o);
    Frames right = f.Render("Text", OS("I", "os-right 4", { { "E_CHOICE_Text_Dir", "right" } }), kRed, 12, 8, periods, o);
    Frames up = f.Render("Text", OS("I", "os-up 4", { { "E_CHOICE_Text_Dir", "up" } }), kRed, 8, 12, periods, o);
    Frames down = f.Render("Text", OS("I", "os-down 4", { { "E_CHOICE_Text_Dir", "down" } }), kRed, 8, 12, periods, o);
    CHECK(FirstRunMoves(left, true, -1));
    CHECK(FirstRunMoves(right, true, +1));
    CHECK(FirstRunMoves(up, false, +1));
    CHECK(FirstRunMoves(down, false, -1));
}

TEST_CASE("Text OS font vertical text draws one letter per line", "[effects][drawing]") {
    Fixture f;
    f.Render("Text", OS("AB", "os-vup 4", { { "E_CHOICE_Text_Effect", "vert text up" } }), kRed, 12, 20, { 0 });
    REQUIRE(Log().drawn.size() >= 2);
    CHECK(Log().drawn[0] == "B");
    CHECK(Log().drawn[1] == "A");
    Log() = TextLog();
    Frames fr = f.Render("Text", OS("AB", "os-vdown 4", { { "E_CHOICE_Text_Effect", "vert text down" } }), kRed, 12, 20, { 0 });
    REQUIRE(Log().drawn.size() >= 2);
    CHECK(Log().drawn[0] == "A");
    CHECK(Log().drawn[1] == "B");
    CHECK(fr.box(0).height() == 8);
    CHECK(fr.box(0).width() == 1);
}

TEST_CASE("Text OS font rotated text passes the rotation to the context for every direction", "[effects][drawing]") {
    Fixture f;
    const std::vector<std::pair<std::string, double>> rots = { { "rotate up 45", 45.0 }, { "rotate up 90", 90.0 }, { "rotate down 45", -45.0 }, { "rotate down 90", -90.0 } };
    const char* dirs[] = { "none", "left", "right", "up", "down", "vector", "up-left", "down-left", "up-right", "down-right" };
    for (const auto& [eff, deg] : rots) {
        for (const char* d : dirs) {
            INFO(eff << " / " << d);
            Log() = TextLog();
            f.Render("Text", OS("AB", "os-rot 4", { { "E_CHOICE_Text_Effect", eff }, { "E_CHOICE_Text_Dir", d } }), kRed, 16, 16, { 0, 5 });
            REQUIRE(Log().rotations.size() == 2);
            CHECK(Log().rotations[0] == deg);
            CHECK(Log().rotations[1] == deg);
        }
    }
    Frames still = f.Render("Text", OS("AB", "os-rot 4", { { "E_CHOICE_Text_Effect", "rotate up 90" } }), kRed, 16, 16, { 0 });
    CHECK(still.lit(0) == 8);
}

TEST_CASE("Text OS font sweep of every direction and option", "[effects][drawing]") {
    Fixture f;
    const char* dirs[] = { "none", "left", "right", "up", "down", "vector", "up-left", "down-left", "up-right", "down-right", "wavey", "word-flip", "left-right", "up-down" };
    RenderOpts o;
    o.endMs = 2000;
    const std::vector<int> periods = Range(0, 39);
    for (const char* d : dirs) {
        for (int opt = 0; opt < 4; ++opt) {
            Settings s = OS("AB CD", "os-sweep 4", { { "E_CHOICE_Text_Dir", d }, { "E_TEXTCTRL_Text_Speed", "20" }, { "E_SLIDER_Text_XEnd", "40" }, { "E_SLIDER_Text_YEnd", "20" } });
            if (opt == 1) s.push_back({ "E_CHECKBOX_TextToCenter", "1" });
            if (opt == 2) s.push_back({ "E_CHECKBOX_TextNoRepeat", "1" });
            if (opt == 3) s.push_back({ "E_CHECKBOX_Text_PixelOffsets", "1" });
            INFO(d << " / opt " << opt);
            Frames fr = f.Render("Text", s, kRed, 16, 16, periods, o);
            int lit = 0;
            for (size_t i = 0; i < fr.f.size(); ++i) {
                lit += fr.lit(i);
                CHECK(fr.allLit(i, PureRed));
            }
            CHECK(lit > 0);
        }
    }
}

TEST_CASE("Text effect settings checks, cache rules and file references", "[effects][drawing]") {
    Fixture f;
    RenderableEffect* fx = f.ctx.effectManager.GetEffect("Text");
    REQUIRE(fx != nullptr);

    auto check = [&](const Settings& s) {
        Effect* e = f.Add("Text", s, kRed);
        return fx->CheckEffectSettings(e->GetSettings(), nullptr, f.model.get(), e, false);
    };
    auto has = [](const std::list<std::string>& l, const std::string& needle) {
        for (const auto& s : l)
            if (s.find(needle) != std::string::npos) return true;
        return false;
    };
    CHECK(has(check({}), "no actual text"));
    CHECK_FALSE(has(check({ { "E_TEXTCTRL_Text", "hi" } }), "ERR"));
    TempDir td;
    CHECK(has(check({ { "E_FILEPICKERCTRL_Text_File", td.File("missing.txt") } }), "cant find file"));
    std::string present = td.Write("present.txt", "hello");
    auto res = check({ { "E_FILEPICKERCTRL_Text_File", present } });
    CHECK_FALSE(has(res, "cant find file"));

    SettingsMap sm;
    sm["TEXTCTRL_Text"] = "x";
    CHECK(fx->SupportsRenderCache(sm));
    sm["TEXTCTRL_Text"] = "";
    sm["CHOICE_Text_LyricTrack"] = "Lyrics - Words";
    CHECK_FALSE(fx->SupportsRenderCache(sm));
    SettingsMap fileOnly;
    fileOnly["FILEPICKERCTRL_Text_File"] = present;
    CHECK_FALSE(fx->SupportsRenderCache(fileOnly));
    // Text files are stored show-folder relative; editing one must not leave a stale cached render
    fileOnly["FILEPICKERCTRL_Text_File"] = "Text Files/present.txt";
    CHECK_FALSE(fx->SupportsRenderCache(fileOnly));

    SettingsMap pure;
    CHECK(fx->GetFrameParallelism(pure) == RenderableEffect::FrameParallelism::Pure);
    SettingsMap cd;
    cd["CHOICE_Text_Count"] = "seconds";
    CHECK(fx->GetFrameParallelism(cd) == RenderableEffect::FrameParallelism::Stateful);

    SettingsMap refs;
    CHECK(fx->GetFileReferences(&f.ctx, f.model.get(), refs).empty());
    refs["E_FILEPICKERCTRL_Text_File"] = present;
    CHECK(fx->GetFileReferences(&f.ctx, f.model.get(), refs).size() == 1);

    Effect* e = f.Add("Text", { { "E_TEXTCTRL_Text", "keep" } }, kRed);
    fx->adjustSettings("2016.1", e, false);
    CHECK(e->GetSettings()["E_TEXTCTRL_Text"] == "keep");
    fx->loadFiles(e);

    Effect* lf = f.Add("Text", { { "E_FILEPICKERCTRL_Text_File", present } }, kRed);
    fx->loadFiles(lf);
    auto entry = f.ctx._sequenceElements.GetSequenceMedia().GetTextFile(lf->GetSettings()["E_FILEPICKERCTRL_Text_File"]);
    REQUIRE(entry != nullptr);
    CHECK(entry->GetContent() == "hello");
}

// ===========================================================================
// Shape
// ===========================================================================

namespace {
Settings FixedShape(const std::string& shape, int size, Settings extra = {}) {
    Settings s = {
        { "E_CHOICE_Shape_ObjectToDraw", shape },
        { "E_CHECKBOX_Shape_RandomLocation", "0" },
        { "E_CHECKBOX_Shape_RandomInitial", "0" },
        { "E_CHECKBOX_Shape_FadeAway", "0" },
        { "E_SLIDER_Shape_Count", "1" },
        { "E_SLIDER_Shape_Growth", "0" },
        { "E_SLIDER_Shape_Lifetime", "100" },
        { "E_SLIDER_Shape_StartSize", std::to_string(size) },
        { "E_SLIDER_Shape_CentreX", "50" },
        { "E_SLIDER_Shape_CentreY", "50" },
    };
    for (const auto& kv : extra) {
        bool replaced = false;
        for (auto& existing : s) {
            if (existing.first == kv.first) {
                existing.second = kv.second;
                replaced = true;
            }
        }
        if (!replaced) s.push_back(kv);
    }
    return s;
}
} // namespace

TEST_CASE("Shape circle lights a ring at the start radius around the centre", "[effects][drawing]") {
    Fixture f;
    Frames fr = f.Render("Shape", FixedShape("Circle", 6), kRed, 21, 21, { 0, 3 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        CHECK(fr.lit(i) > 20);
        CHECK(fr.allLit(i, PureRed));
        CHECK(fr.allLit(i, [](const xlColor&, int x, int y) {
            double d = std::sqrt((x - 10.0) * (x - 10.0) + (y - 10.0) * (y - 10.0));
            return std::abs(d - 6.0) <= 1.0;
        }));
    }
    // no growth, no motion: the frame repeats
    CHECK(fr.same(0, fr, 1));
}

TEST_CASE("Shape every shape draws something distinct in the palette colour", "[effects][drawing]") {
    Fixture f;
    const char* shapes[] = { "Circle", "Ellipse", "Triangle", "Square", "Pentagon", "Hexagon", "Octagon", "Star", "Heart", "Tree", "Snowflake", "Candy Cane", "Crucifix", "Present" };
    std::set<std::vector<uint8_t>> distinct;
    for (const char* s : shapes) {
        INFO(s);
        Frames fr = f.Render("Shape", FixedShape(s, 9, { { "E_SLIDER_Shape_Thickness", "2" } }), kRed, 24, 24, { 0 });
        CHECK(fr.lit(0) > 8);
        CHECK(fr.allLit(0, PureRed));
        Box b = fr.box(0);
        CHECK(b.minX >= 12 - 10);
        CHECK(b.maxX <= 12 + 10);
        std::vector<uint8_t> key;
        for (const auto& c : fr.f[0]) key.push_back(IsLit(c));
        distinct.insert(key);
    }
    CHECK(distinct.size() == std::size(shapes));
}

TEST_CASE("Shape options change the drawing", "[effects][drawing]") {
    Fixture f;
    SECTION("thickness") {
        Frames thin = f.Render("Shape", FixedShape("Square", 8), kRed, 24, 24, { 0 });
        Frames thick = f.Render("Shape", FixedShape("Square", 8, { { "E_SLIDER_Shape_Thickness", "4" } }), kRed, 24, 24, { 0 });
        CHECK(thick.lit(0) > thin.lit(0) + 10);
    }
    SECTION("star points and rotation") {
        Frames p5 = f.Render("Shape", FixedShape("Star", 9, { { "E_SLIDER_Shape_Points", "5" } }), kRed, 24, 24, { 0 });
        Frames p6 = f.Render("Shape", FixedShape("Star", 9, { { "E_SLIDER_Shape_Points", "6" } }), kRed, 24, 24, { 0 });
        Frames p7 = f.Render("Shape", FixedShape("Star", 9, { { "E_SLIDER_Shape_Points", "7" } }), kRed, 24, 24, { 0 });
        Frames rot = f.Render("Shape", FixedShape("Star", 9, { { "E_SLIDER_Shape_Points", "5" }, { "E_SLIDER_Shape_Rotation", "36" } }), kRed, 24, 24, { 0 });
        CHECK_FALSE(p5 == p6);
        CHECK_FALSE(p6 == p7);
        CHECK_FALSE(p5 == rot);
    }
    SECTION("ellipse aspect follows the points slider") {
        Frames e = f.Render("Shape", FixedShape("Ellipse", 9, { { "E_SLIDER_Shape_Points", "5" } }), kRed, 24, 24, { 0 });
        CHECK(e.box(0).width() > e.box(0).height() + 4);
        Frames r = f.Render("Shape", FixedShape("Ellipse", 9, { { "E_SLIDER_Shape_Points", "5" }, { "E_SLIDER_Shape_Rotation", "90" } }), kRed, 24, 24, { 0 });
        CHECK(r.box(0).height() > r.box(0).width() + 4);
    }
    SECTION("rotated heart, tree, crucifix and present differ from unrotated") {
        for (const char* s : { "Heart", "Tree", "Crucifix", "Present", "Square" }) {
            INFO(s);
            Frames a = f.Render("Shape", FixedShape(s, 9), kRed, 24, 24, { 0 });
            Frames b = f.Render("Shape", FixedShape(s, 9, { { "E_SLIDER_Shape_Rotation", "30" } }), kRed, 24, 24, { 0 });
            CHECK(b.lit(0) > 0);
            CHECK_FALSE(a == b);
        }
    }
    SECTION("centre position") {
        Frames a = f.Render("Shape", FixedShape("Circle", 3, { { "E_SLIDER_Shape_CentreX", "25" }, { "E_SLIDER_Shape_CentreY", "75" } }), kRed, 20, 20, { 0 });
        CHECK_THAT(a.cx(0), WithinAbs(5.0, 0.6));
        CHECK_THAT(a.cy(0), WithinAbs(15.0, 0.6));
    }
}

TEST_CASE("Shape growth enlarges or shrinks the shape over its life", "[effects][drawing]") {
    Fixture f;
    Frames grow = f.Render("Shape", FixedShape("Circle", 1, { { "E_SLIDER_Shape_Growth", "40" } }), kRed, 30, 30, Range(0, 6));
    Frames shrink = f.Render("Shape", FixedShape("Circle", 10, { { "E_SLIDER_Shape_Growth", "-40" } }), kRed, 30, 30, Range(0, 6));
    for (int i = 1; i < 7; ++i) {
        CHECK(grow.box(i).width() >= grow.box(i - 1).width());
        CHECK(shrink.box(i).width() <= shrink.box(i - 1).width());
    }
    CHECK(grow.box(6).width() > grow.box(0).width() + 6);
    CHECK(shrink.box(6).width() + 6 < shrink.box(0).width());
}

TEST_CASE("Shape lifetime restarts the shape on a fixed period", "[effects][drawing]") {
    Fixture f;
    // 20 frames * 25% = 4.75 frames of life: a new shape replaces the old one every 5 frames
    Frames fr = f.Render("Shape", FixedShape("Circle", 2, { { "E_SLIDER_Shape_Lifetime", "25" }, { "E_SLIDER_Shape_Growth", "10" } }), kRed, 30, 30, Range(0, 10));
    CHECK(fr.same(0, fr, 5));
    CHECK(fr.same(0, fr, 10));
    CHECK(fr.same(2, fr, 7));
    for (int i = 1; i < 5; ++i) CHECK_FALSE(fr.same(0, fr, i));
}

TEST_CASE("Shape fade away dims the shape as it ages", "[effects][drawing]") {
    Fixture f;
    Settings s = FixedShape("Circle", 5, { { "E_CHECKBOX_Shape_FadeAway", "1" }, { "E_SLIDER_Shape_Lifetime", "50" } });
    Frames rgb = f.Render("Shape", s, kRed, 21, 21, Range(0, 8));
    RenderOpts alpha;
    alpha.allowAlpha = true;
    Frames a = f.Render("Shape", s, kRed, 21, 21, Range(0, 8), alpha);
    CHECK(rgb.maxRed(0) == 255);
    for (int i = 1; i < 9; ++i) {
        CHECK(rgb.maxRed(i) < rgb.maxRed(i - 1));
        CHECK(a.maxRed(i) < a.maxRed(i - 1));
    }
    // with an alpha channel the colour stays full and alpha carries the fade
    CHECK(a.allLit(5, [](const xlColor& c, int, int) { return c.red == 255 && c.alpha < 255; }));
}

TEST_CASE("Shape velocity moves the shape in the set direction", "[effects][drawing]") {
    Fixture f;
    auto run = [&](int dir) {
        return f.Render("Shape", FixedShape("Circle", 2, { { "E_SLIDER_Shapes_Velocity", "2" }, { "E_SLIDER_Shapes_Direction", std::to_string(dir) } }), kRed, 40, 40, Range(0, 5));
    };
    Frames east = run(0);
    Frames north = run(90);
    Frames west = run(180);
    for (int i = 1; i < 6; ++i) {
        CHECK(east.cx(i) > east.cx(i - 1));
        CHECK_THAT(east.cy(i), WithinAbs(east.cy(0), 0.01));
        CHECK(north.cy(i) > north.cy(i - 1));
        CHECK(west.cx(i) < west.cx(i - 1));
    }
}

TEST_CASE("Shape random options are deterministic and stay in the palette", "[effects][drawing]") {
    Fixture f;
    Settings s = {
        { "E_CHOICE_Shape_ObjectToDraw", "Random" },
        { "E_CHECKBOX_Shape_RandomLocation", "1" },
        { "E_CHECKBOX_Shape_RandomInitial", "1" },
        { "E_CHECKBOX_Shapes_RandomMovement", "1" },
        { "E_CHECKBOX_Shape_HoldColour", "0" },
        { "E_SLIDER_Shape_Count", "6" },
        { "E_SLIDER_Shape_StartSize", "3" },
        { "E_SLIDER_Shape_Lifetime", "30" },
    };
    Frames a = f.Render("Shape", s, kRedBlue, 30, 30, Range(0, 19));
    Frames b = f.Render("Shape", s, kRedBlue, 30, 30, Range(0, 19));
    CHECK(a == b);
    for (size_t i = 0; i < a.f.size(); ++i) {
        CHECK(a.lit(i) > 0);
        CHECK(a.allLit(i, [](const xlColor& c, int, int) { return c.green == 0 && (c.red == 0) != (c.blue == 0); }));
    }
    bool red = false, blue = false;
    for (size_t i = 0; i < a.f.size(); ++i) {
        red |= !a.allLit(i, PureBlue);
        blue |= !a.allLit(i, PureRed);
    }
    CHECK(red);
    CHECK(blue);

    Settings held = s;
    held.push_back({ "E_CHECKBOX_Shape_HoldColour", "1" });
    held.push_back({ "E_CHECKBOX_Shape_FadeAway", "1" });
    Frames h = f.Render("Shape", held, kRedBlue, 30, 30, Range(0, 9));
    CHECK(h.lit(3) > 0);
}

TEST_CASE("Shape emoji draws the glyph tinted with the palette colour", "[effects][drawing]") {
    Fixture f;
    Frames e = f.Render("Shape", FixedShape("Emoji", 10, { { "E_FONTPICKER_Shape_Font", "emoji 12" } }), kRed, 30, 30, { 0, 1 });
    REQUIRE(e.lit(0) > 10);
    CHECK(e.allLit(0, RedOnly));
    CHECK_THAT(e.cx(0), WithinAbs(15.0, 1.5));
    CHECK_THAT(e.cy(0), WithinAbs(15.0, 1.5));
    CHECK(e.same(0, e, 1));
    // a skin tone appends a modifier code point, which this backend draws as a second glyph
    Frames toned = f.Render("Shape", FixedShape("Emoji", 10, { { "E_CHOICE_Shape_SkinTone", "Dark" } }), kRed, 30, 30, { 0 });
    CHECK(toned.box(0).width() > e.box(0).width());
    for (const char* tone : { "Light", "Medium Light", "Medium", "Medium Dark" }) {
        Frames t = f.Render("Shape", FixedShape("Emoji", 10, { { "E_CHOICE_Shape_SkinTone", tone } }), kRed, 30, 30, { 0 });
        CHECK(t.box(0).width() == toned.box(0).width());
    }
    // different code points are separate cache entries
    Frames bmp = f.Render("Shape", FixedShape("Emoji", 10, { { "E_SPINCTRL_Shape_Char", "9731" } }), kRed, 30, 30, { 0 });
    CHECK(bmp.lit(0) > 0);
    Frames latin = f.Render("Shape", FixedShape("Emoji", 10, { { "E_SPINCTRL_Shape_Char", "200" } }), kRed, 30, 30, { 0 });
    CHECK(latin.lit(0) > 0);
    Frames ascii = f.Render("Shape", FixedShape("Emoji", 10, { { "E_SPINCTRL_Shape_Char", "65" }, { "E_CHECKBOX_Shape_FadeAway", "1" } }), kRed, 30, 30, { 0, 2 });
    CHECK(ascii.maxRed(1) < ascii.maxRed(0));
    // a space measures as nothing: nothing to draw
    Frames space = f.Render("Shape", FixedShape("Emoji", 10, { { "E_SPINCTRL_Shape_Char", "32" } }), kRed, 30, 30, { 0 });
    CHECK(space.lit(0) == 0);
    // larger start size, larger emoji
    Frames big = f.Render("Shape", FixedShape("Emoji", 20), kRed, 30, 30, { 0 });
    CHECK(big.lit(0) > e.lit(0));
}

TEST_CASE("Shape emoji without a text backend draws nothing", "[effects][drawing]") {
    Fixture f(false);
    Frames e = f.Render("Shape", FixedShape("Emoji", 10), kRed, 30, 30, { 0 });
    CHECK(e.lit(0) == 0);
}

TEST_CASE("Shape SVG rasterises the file in its own colours and flags a missing file", "[effects][drawing]") {
    TempDir td;
    std::string svg = td.Write("sq.svg", kSquareSvg);
    Fixture f;
    const char* blue = "C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1";
    // the drawn size is ten times the start size, in pixels
    Frames fr = f.Render("Shape", FixedShape("SVG", 1, { { "E_FILEPICKERCTRL_SVG", svg } }), blue, 24, 24, { 0, 1 });
    REQUIRE(fr.lit(0) > 20);
    CHECK(fr.lit(0) < 24 * 24);
    // the SVG's white fill is kept; the palette only scales alpha
    CHECK(fr.at(0, 12, 12).red == 255);
    CHECK(fr.at(0, 12, 12).green == 255);
    CHECK(fr.at(0, 12, 12).blue == 255);
    CHECK_THAT(fr.cx(0), WithinAbs(12.0, 1.5));
    CHECK_THAT(fr.cy(0), WithinAbs(12.0, 1.5));
    Frames bigger = f.Render("Shape", FixedShape("SVG", 2, { { "E_FILEPICKERCTRL_SVG", svg } }), blue, 24, 24, { 0 });
    CHECK(bigger.lit(0) > fr.lit(0));
    // a missing file paints the whole buffer red as an error marker
    Frames none = f.Render("Shape", FixedShape("SVG", 8, { { "E_FILEPICKERCTRL_SVG", td.File("missing.svg") } }), blue, 24, 24, { 0 });
    CHECK(none.lit(0) == 24 * 24);
    CHECK(none.allLit(0, PureRed));
}

TEST_CASE("Shape fires on timing marks and honours the label filter", "[effects][drawing]") {
    Fixture f;
    f.AddTiming("Beats", { { { "A", 0, 50 }, { "B x", 250, 300 }, { "A", 500, 550 } } });
    auto fire = [&](Settings extra) {
        Settings s = FixedShape("Circle", 4, { { "E_CHECKBOX_Shape_FireTiming", "1" }, { "E_CHOICE_Shape_FireTimingTrack", "Beats" }, { "E_SLIDER_Shape_Lifetime", "10" } });
        s.insert(s.end(), extra.begin(), extra.end());
        return f.Render("Shape", s, kRed, 20, 20, Range(0, 12));
    };
    Frames all = fire({});
    CHECK(all.lit(0) > 0);
    CHECK(all.lit(4) == 0);
    CHECK(all.lit(5) > 0);
    CHECK(all.lit(10) > 0);

    Frames onlyB = fire({ { "E_TEXTCTRL_Shape_FilterLabel", "B" } });
    CHECK(onlyB.lit(0) == 0);
    CHECK(onlyB.lit(5) > 0);
    CHECK(onlyB.lit(10) == 0);

    Frames regex = fire({ { "E_TEXTCTRL_Shape_FilterLabel", "^A$" }, { "E_CHECKBOX_Shape_FilterReg", "1" } });
    CHECK(regex.lit(0) > 0);
    CHECK(regex.lit(5) == 0);
    CHECK(regex.lit(10) > 0);

    // an invalid regex falls back to token matching
    Frames bad = fire({ { "E_TEXTCTRL_Shape_FilterLabel", "(" }, { "E_CHECKBOX_Shape_FilterReg", "1" } });
    for (size_t i = 0; i < bad.f.size(); ++i) CHECK(bad.lit(i) == 0);

    Settings missing = FixedShape("Circle", 4, { { "E_CHECKBOX_Shape_FireTiming", "1" }, { "E_CHOICE_Shape_FireTimingTrack", "Nope" } });
    Frames m = f.Render("Shape", missing, kRed, 20, 20, { 0, 5 });
    CHECK(m.lit(0) == 0);
    CHECK(m.lit(1) == 0);
}

TEST_CASE("Shape fires on loud music and stays dark in silence", "[effects][drawing]") {
    TempDir td;
    const int sr = 44100;
    std::vector<int16_t> samples(sr, 0);
    // silent for 0.3s, then loud for 0.3s, then silent
    for (int i = sr * 3 / 10; i < sr * 6 / 10; ++i) samples[i] = (int16_t)std::lround(30000.0 * std::sin(2.0 * 3.14159265358979 * 440.0 * i / sr));
    AudioManager am(td.Write("loud.wav", Wav(sr, samples)), 50);
    REQUIRE(am.IsOk());
    am.DoPrepareFrameData();
    Fixture f;
    RenderOpts o;
    o.media = &am;
    Settings s = FixedShape("Circle", 4, { { "E_CHECKBOX_Shape_UseMusic", "1" }, { "E_SLIDER_Shape_Sensitivity", "10" } });
    Frames fr = f.Render("Shape", s, kRed, 20, 20, Range(0, 19), o);
    for (int i = 0; i < 4; ++i) CHECK(fr.lit(i) == 0);
    CHECK(fr.lit(10) > 0);
    CHECK(fr.lit(19) > 0);
    // without any audio nothing ever fires
    Frames none = f.Render("Shape", s, kRed, 20, 20, Range(0, 19));
    for (size_t i = 0; i < none.f.size(); ++i) CHECK(none.lit(i) == 0);
}

TEST_CASE("Shape settings checks, migration and timing rename", "[effects][drawing]") {
    Fixture f;
    RenderableEffect* fx = f.ctx.effectManager.GetEffect("Shape");
    REQUIRE(fx != nullptr);
    auto has = [](const std::list<std::string>& l, const std::string& needle) {
        for (const auto& s : l)
            if (s.find(needle) != std::string::npos) return true;
        return false;
    };
    auto check = [&](const Settings& s) {
        Effect* e = f.Add("Shape", s, kRed);
        return fx->CheckEffectSettings(e->GetSettings(), nullptr, f.model.get(), e, false);
    };
    CHECK(has(check({ { "E_CHECKBOX_Shape_UseMusic", "1" } }), "no music"));
    CHECK_FALSE(has(check({}), "WARN"));
    CHECK(has(check({ { "E_CHOICE_Shape_ObjectToDraw", "SVG" } }), "cant find SVG"));
    TempDir td;
    CHECK(has(check({ { "E_CHOICE_Shape_ObjectToDraw", "SVG" }, { "E_FILEPICKERCTRL_SVG", td.File("none.svg") } }), "cant find SVG"));
    std::string svg = td.Write("ok.svg", kSquareSvg);
    CHECK_FALSE(has(check({ { "E_CHOICE_Shape_ObjectToDraw", "SVG" }, { "E_FILEPICKERCTRL_SVG", svg } }), "cant find SVG"));

    SettingsMap refs;
    CHECK(fx->GetFileReferences(&f.ctx, f.model.get(), refs).empty());
    refs["E_FILEPICKERCTRL_SVG"] = svg;
    CHECK(fx->GetFileReferences(&f.ctx, f.model.get(), refs).size() == 1);

    CHECK(fx->needToAdjustSettings("2023.10"));
    Effect* emoji = f.Add("Shape", { { "E_CHOICE_Shape_ObjectToDraw", "Emoji" }, { "E_SLIDER_Shape_CentreY", "30" } }, kRed);
    fx->adjustSettings("2023.10", emoji, false);
    CHECK(emoji->GetSettings()["E_SLIDER_Shape_CentreY"] == "70");
    Effect* circle = f.Add("Shape", { { "E_CHOICE_Shape_ObjectToDraw", "Circle" }, { "E_SLIDER_Shape_CentreY", "30" } }, kRed);
    fx->adjustSettings("2023.10", circle, false);
    CHECK(circle->GetSettings()["E_SLIDER_Shape_CentreY"] == "30");

    Effect* timed = f.Add("Shape", { { "E_CHOICE_Shape_FireTimingTrack", "Old" } }, kRed);
    fx->RenameTimingTrack("Other", "New", timed);
    CHECK(timed->GetSettings()["E_CHOICE_Shape_FireTimingTrack"] == "Old");
    fx->RenameTimingTrack("Old", "New", timed);
    CHECK(timed->GetSettings()["E_CHOICE_Shape_FireTimingTrack"] == "New");

    Effect* withSvg = f.Add("Shape", { { "E_FILEPICKERCTRL_SVG", svg } }, kRed);
    fx->loadFiles(withSvg);
    auto entry = f.ctx._sequenceElements.GetSequenceMedia().GetSVG(withSvg->GetSettings()["E_FILEPICKERCTRL_SVG"]);
    REQUIRE(entry != nullptr);
    CHECK(entry->GetSVGContent() == kSquareSvg);
    fx->loadFiles(timed);
}

// ===========================================================================
// Sketch path model
// ===========================================================================

TEST_CASE("Sketch default string parses into its two paths", "[effects][drawing][sketch]") {
    auto sketch = SketchEffectSketch::DefaultSketch();
    REQUIRE(sketch.pathCount() == 2);
    const auto& p0 = sketch.paths()[0];
    const auto& p1 = sketch.paths()[1];
    CHECK(p0->isClosed());
    CHECK(p1->isClosed());
    CHECK(p0->GetClosedState() == SketchCanvasPathState::LineToNewPoint);
    CHECK(p0->segments().size() == 18);
    CHECK(p1->segments().size() == 6);
    int quads = 0;
    for (const auto& s : p1->segments())
        if (std::dynamic_pointer_cast<SketchQuadraticBezier>(s)) ++quads;
    CHECK(quads == 4);
    CHECK_THAT(p0->segments()[0]->StartPoint().x, WithinAbs(0.490716, 1e-9));
    CHECK_THAT(p0->segments()[0]->StartPoint().y, WithinAbs(0.0175163, 1e-9));
    // consecutive segments are joined
    for (size_t i = 1; i < p0->segments().size(); ++i) {
        CHECK(p0->segments()[i]->StartPoint() == p0->segments()[i - 1]->EndPoint());
    }
    CHECK(sketch.getLength() > 2.0);
}

TEST_CASE("Sketch string round-trips through toString", "[effects][drawing][sketch]") {
    const std::string def = "0.1,0.2;Dhello path;L0.3,0.4;Q0.5,0.6,0.7,0.8;C0.1,0.9,0.2,0.8,0.3,0.7;c3|0.5,0.5;L0.6,0.6";
    auto sketch = SketchEffectSketch::SketchFromString(def);
    REQUIRE(sketch.pathCount() == 2);
    // 'D' carries hex; this one is not valid hex so it decodes to nothing
    CHECK(sketch.paths()[0]->GetDescription().empty());
    CHECK(sketch.paths()[0]->GetClosedState() == SketchCanvasPathState::QuadraticCurveToNewPoint);
    CHECK_FALSE(sketch.paths()[1]->isClosed());

    sketch.paths()[0]->SetDescription("Star outline");
    std::string s = sketch.toString();
    CHECK(s == "0.1,0.2;D53746172206F75746C696E65;L0.3,0.4;Q0.5,0.6,0.7,0.8;C0.1,0.9,0.2,0.8,0.3,0.7;c3|0.5,0.5;L0.6,0.6;");
    auto again = SketchEffectSketch::SketchFromString(s);
    CHECK(again.toString() == s);
    CHECK(again.paths()[0]->GetDescription() == "Star outline");

    auto def2 = SketchEffectSketch::SketchFromString(SketchEffectSketch::DefaultSketchString());
    CHECK(SketchEffectSketch::SketchFromString(def2.toString()).toString() == def2.toString());

    CHECK(SketchEffectSketch().toString().empty());
    CHECK(SketchEffectSketch::SketchFromString("").pathCount() == 0);
    // a path with no parsable segments is dropped
    CHECK(SketchEffectSketch::SketchFromString("0.1,0.1;Lx,y;Qa|0.2,0.2;C1").pathCount() == 0);
    // odd-length hex is rejected
    CHECK(SketchEffectSketch::SketchFromString("0,0;DABC;L1,1").paths()[0]->GetDescription().empty());
    CHECK(SketchEffectSketch::SketchFromString("0,0;D4142;L1,1").paths()[0]->GetDescription() == "AB");
    CHECK(SketchEffectSketch::SketchFromString("0,0;D4g;L1,1").paths()[0]->GetDescription().empty());
}

TEST_CASE("Sketch segment lengths, hit tests and progress positions", "[effects][drawing][sketch]") {
    SketchLine line({ 0, 0 }, { 0.3, 0.4 });
    CHECK_THAT(line.Length(), WithinAbs(0.5, 1e-12));
    CHECK(line.HitTest({ 0.15, 0.2 }));
    CHECK_FALSE(line.HitTest({ 0.3, 0.0 }));
    double x = -1, y = -1;
    line.getProgressPosition(0.25, x, y);
    CHECK_THAT(x, WithinAbs(0.15, 1e-12));
    CHECK_THAT(y, WithinAbs(0.2, 1e-12));

    // straight curves measure the same as the chord
    SketchQuadraticBezier quad({ 0, 0 }, { 0.15, 0.2 }, { 0.3, 0.4 });
    SketchCubicBezier cubic({ 0, 0 }, { 0.1, 0.4 / 3 }, { 0.2, 0.8 / 3 }, { 0.3, 0.4 });
    CHECK_THAT(quad.Length(), WithinAbs(0.5, 1e-6));
    CHECK_THAT(cubic.Length(), WithinAbs(0.5, 1e-6));
    quad.getProgressPosition(0.25, x, y);
    CHECK_THAT(x, WithinAbs(0.15, 1e-3));
    CHECK_THAT(y, WithinAbs(0.2, 1e-3));
    cubic.getProgressPosition(0.25, x, y);
    CHECK_THAT(x, WithinAbs(0.15, 1e-3));
    CHECK_THAT(y, WithinAbs(0.2, 1e-3));
    CHECK(quad.HitTest({ 0.15, 0.2 }));
    CHECK_FALSE(quad.HitTest({ 0.9, 0.9 }));
    CHECK(cubic.HitTest({ 0.15, 0.2 }));
    CHECK_FALSE(cubic.HitTest({ 0.9, 0.9 }));

    // a real curve is longer than its chord and bulges toward the control point
    SketchQuadraticBezier arc({ 0, 0 }, { 0.5, 1.0 }, { 1.0, 0 });
    CHECK(arc.Length() > 1.2);
    CHECK(arc.HitTest({ 0.5, 0.5 }));
    CHECK_FALSE(arc.HitTest({ 0.5, 0.1 }));

    SketchLine a({ 0, 0 }, { 1, 0 });
    a.SetStartPoint({ 0, 1 });
    a.SetEndPoint({ 1, 1 });
    CHECK(a.StartPoint() == xlPointD(0, 1));
    CHECK(a.EndPoint() == xlPointD(1, 1));
    quad.SetStartPoint({ 1, 1 });
    quad.SetEndPoint({ 2, 2 });
    quad.SetControlPoint({ 3, 3 });
    CHECK(quad.ControlPoint() == xlPointD(3, 3));
    cubic.SetStartPoint({ 1, 1 });
    cubic.SetEndPoint({ 2, 2 });
    cubic.SetControlPoint1({ 3, 3 });
    cubic.SetControlPoint2({ 4, 4 });
    CHECK(cubic.ControlPoint1() == xlPointD(3, 3));
    CHECK(cubic.ControlPoint2() == xlPointD(4, 4));
}

TEST_CASE("Sketch paths reverse, close and edit", "[effects][drawing][sketch]") {
    auto path = std::make_shared<SketchEffectPath>();
    CHECK(path->Length() == 0.0);
    path->appendSegment(std::make_shared<SketchLine>(xlPointD(0, 0), xlPointD(1, 0)));
    path->appendSegment(std::make_shared<SketchCubicBezier>(xlPointD(1, 0), xlPointD(1, 0.3), xlPointD(0.8, 0.6), xlPointD(0.5, 1)));
    path->reversePath();
    REQUIRE(path->segments().size() == 2);
    auto c = std::dynamic_pointer_cast<SketchCubicBezier>(path->segments()[0]);
    REQUIRE(c != nullptr);
    CHECK(c->StartPoint() == xlPointD(0.5, 1));
    CHECK(c->EndPoint() == xlPointD(1, 0));
    CHECK(c->ControlPoint1() == xlPointD(0.8, 0.6));
    CHECK(c->ControlPoint2() == xlPointD(1, 0.3));
    CHECK(path->segments()[1]->EndPoint() == xlPointD(0, 0));

    SketchQuadraticBezier q({ 0, 0 }, { 0.5, 1 }, { 1, 0 });
    q.ReverseSegment();
    CHECK(q.StartPoint() == xlPointD(1, 0));
    CHECK(q.EndPoint() == xlPointD(0, 0));
    CHECK(q.ControlPoint() == xlPointD(0.5, 1));

    for (auto [state, kind] : std::vector<std::pair<SketchCanvasPathState, int>>{
             { SketchCanvasPathState::LineToNewPoint, 0 }, { SketchCanvasPathState::QuadraticCurveToNewPoint, 1 }, { SketchCanvasPathState::CubicCurveToNewPoint, 2 } }) {
        auto p = std::make_shared<SketchEffectPath>();
        p->appendSegment(std::make_shared<SketchLine>(xlPointD(0, 0), xlPointD(1, 0)));
        p->appendSegment(std::make_shared<SketchLine>(xlPointD(1, 0), xlPointD(1, 1)));
        p->closePath(true, state);
        CHECK(p->isClosed());
        CHECK(p->GetClosedState() == state);
        REQUIRE(p->segments().size() == 3);
        const auto& closing = p->segments().back();
        CHECK(closing->StartPoint() == xlPointD(1, 1));
        CHECK(closing->EndPoint() == xlPointD(0, 0));
        if (kind == 0) CHECK(std::dynamic_pointer_cast<SketchLine>(closing) != nullptr);
        if (kind == 1) CHECK(std::dynamic_pointer_cast<SketchQuadraticBezier>(closing) != nullptr);
        if (kind == 2) CHECK(std::dynamic_pointer_cast<SketchCubicBezier>(closing) != nullptr);
        // closing twice is a no-op
        p->closePath(true, state);
        CHECK(p->segments().size() == 3);
        CHECK_THAT(p->Length(), WithinAbs(2.0 + std::sqrt(2.0), 1e-3));
    }
    auto open = std::make_shared<SketchEffectPath>();
    open->closePath(true, SketchCanvasPathState::LineToNewPoint);
    CHECK_FALSE(open->isClosed());

    SketchEffectSketch sk;
    auto p1 = std::make_shared<SketchEffectPath>();
    p1->appendSegment(std::make_shared<SketchLine>(xlPointD(0, 0), xlPointD(0.5, 0)));
    auto p2 = std::make_shared<SketchEffectPath>();
    p2->appendSegment(std::make_shared<SketchLine>(xlPointD(0, 1), xlPointD(1, 1)));
    sk.appendPath(p1);
    sk.appendPath(p2);
    CHECK_THAT(sk.getLength(), WithinAbs(1.5, 1e-12));
    sk.swapPaths(0, 1);
    CHECK(sk.paths()[0] == p2);
    sk.swapPaths(0, 5);
    sk.swapPaths(-1, 0);
    CHECK(sk.paths()[0] == p2);
    sk.reversePath(0);
    CHECK(p2->segments()[0]->StartPoint() == xlPointD(1, 1));
    sk.reversePath(9);
    auto p3 = std::make_shared<SketchEffectPath>();
    sk.updatePath(1, p3);
    CHECK(sk.paths()[1] == p3);
    sk.updatePath(7, p1);
    sk.deletePath(1);
    CHECK(sk.pathCount() == 1);
    sk.deletePath(3);
    sk.deletePath(-1);
    CHECK(sk.pathCount() == 1);
}

TEST_CASE("Sketch progress position walks across paths", "[effects][drawing][sketch]") {
    auto sk = SketchEffectSketch::SketchFromString("0,0;L1,0|0,1;L1,1");
    double x = -1, y = -1;
    sk.getProgressPosition(0.25, x, y);
    CHECK_THAT(x, WithinAbs(0.5, 1e-9));
    CHECK_THAT(y, WithinAbs(0.0, 1e-9));
    sk.getProgressPosition(0.75, x, y);
    CHECK_THAT(x, WithinAbs(0.5, 1e-9));
    CHECK_THAT(y, WithinAbs(1.0, 1e-9));
    sk.getProgressPosition(1.0, x, y);
    CHECK_THAT(x, WithinAbs(1.0, 1e-9));
    CHECK_THAT(y, WithinAbs(1.0, 1e-9));

    auto curved = SketchEffectSketch::SketchFromString("0,0;Q0.5,1,1,0;C1,0.5,0.5,0.5,0,0");
    curved.getProgressPosition(0.0, x, y);
    CHECK_THAT(x, WithinAbs(0.0, 1e-3));
    curved.getProgressPosition(0.999, x, y);
    CHECK_THAT(x, WithinAbs(0.0, 0.02));
    CHECK_THAT(y, WithinAbs(0.0, 0.02));

    SketchEffectSketch empty;
    x = y = 5;
    empty.getProgressPosition(0.5, x, y);
    CHECK(x == 0.0);
    CHECK(y == 0.0);
}

// a zero-length segment (a repeated point, as in the default sketch) is that point, not NaN
TEST_CASE("Sketch progress position on a repeated point is that point", "[effects][drawing]") {
    auto sk = SketchEffectSketch::SketchFromString("0.5,0.5;L0.5,0.5;L1,1");
    double x = -1, y = -1;
    sk.getProgressPosition(0.0, x, y);
    CHECK_THAT(x, WithinAbs(0.5, 1e-9));
    CHECK_THAT(y, WithinAbs(0.5, 1e-9));
}

TEST_CASE("Sketch paths emit cubic path data scaled to the buffer", "[effects][drawing][sketch]") {
    auto sk = SketchEffectSketch::SketchFromString("0,0;L1,0;Q1,1,0,1;C0,0.5,0,0.5,0,0");
    const auto& path = sk.paths()[0];
    SketchPathData whole;
    path->collectEntirePath(whole, 10, 20);
    REQUIRE(whole.subPaths.size() == 1);
    CHECK(whole.subPaths[0].startX == 0.0f);
    CHECK(whole.subPaths[0].cubics.size() == 18);
    // line end, quadratic end, cubic end
    CHECK(whole.subPaths[0].cubics[4] == 10.0f);
    CHECK(whole.subPaths[0].cubics[5] == 0.0f);
    CHECK(whole.subPaths[0].cubics[10] == 0.0f);
    CHECK(whole.subPaths[0].cubics[11] == 20.0f);
    CHECK(whole.subPaths[0].cubics[16] == 0.0f);
    CHECK(whole.subPaths[0].cubics[17] == 0.0f);
    // the quadratic is degree-elevated: cp1 = (P0 + 2 CP) / 3
    CHECK_THAT(whole.subPaths[0].cubics[6], WithinAbs(10.0, 1e-5));
    CHECK_THAT(whole.subPaths[0].cubics[7], WithinAbs(40.0 / 3.0, 1e-4));

    auto lastPoint = [](const SketchPathData& d, float& lx, float& ly) {
        const auto& c = d.subPaths.back().cubics;
        lx = c[c.size() - 2];
        ly = c[c.size() - 1];
    };
    // the line is 1 of ~3.6 units: 20% stops part-way along it
    SketchPathData part;
    path->collectPartialPath(part, 10, 20, std::nullopt, 0.2);
    float lx = 0, ly = 0;
    lastPoint(part, lx, ly);
    CHECK(lx > 5.0f);
    CHECK(lx < 10.0f);
    CHECK(ly == 0.0f);
    // into the curves
    for (double pct : { 0.5, 0.95 }) {
        SketchPathData d;
        path->collectPartialPath(d, 10, 20, std::nullopt, pct);
        CHECK(d.subPaths.size() == 1);
        CHECK(d.subPaths[0].cubics.size() > 18);
    }
    // with a start percentage each segment emits its own sub-path
    SketchPathData window;
    path->collectPartialPath(window, 10, 20, 0.1, 0.9);
    CHECK(window.subPaths.size() >= 2);
    CHECK(window.subPaths[0].startX > 0.0f);
    SketchPathData nothing;
    path->collectPartialPath(nothing, 10, 20, 0.4, 0.4);
    for (const auto& sp : nothing.subPaths) CHECK(sp.cubics.empty());

    // segment-level partial draws
    SketchLine line({ 0, 0 }, { 1, 0 });
    SketchPathData l1;
    line.DrawPartialSegment(l1, 10, 10, std::nullopt, 0.5);
    REQUIRE(l1.subPaths.size() == 1);
    CHECK(l1.subPaths[0].cubics[4] == 5.0f);
    SketchPathData l2;
    line.DrawPartialSegment(l2, 10, 10, std::nullopt, 0.0);
    CHECK(l2.empty());
    SketchPathData l3;
    line.DrawPartialSegment(l3, 10, 10, 0.2, 0.6);
    REQUIRE(l3.subPaths.size() == 1);
    CHECK(l3.subPaths[0].startX == 2.0f);
    CHECK(l3.subPaths[0].cubics[4] == 6.0f);
    SketchPathData l4;
    line.DrawPartialSegment(l4, 10, 10, 0.5, 0.5);
    CHECK(l4.empty());

    SketchPathData empty;
    SketchEffectPath none;
    none.collectEntirePath(empty, 10, 10);
    none.collectPartialPath(empty, 10, 10, std::nullopt, 0.5);
    CHECK(empty.empty());
    SketchEffectPath zero;
    zero.appendSegment(std::make_shared<SketchLine>(xlPointD(0.5, 0.5), xlPointD(0.5, 0.5)));
    zero.collectPartialPath(empty, 10, 10, std::nullopt, 0.5);
    CHECK(empty.empty());

    SketchPathData merged;
    merged.lineTo(0, 0, 1, 1); // a line without a moveTo starts at the origin
    REQUIRE(merged.subPaths.size() == 1);
    merged.append(whole);
    CHECK(merged.subPaths.size() == 2);
}

TEST_CASE("Sketch SVG import turns lines and curves into a sketch definition", "[effects][drawing][sketch]") {
    TempDir td;
    std::string svg = td.Write("paths.svg",
        R"(<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">)"
        R"(<path d="M10 10 L90 10" stroke="black" fill="none"/>)"
        R"(<path d="M10 50 C30 20 70 20 90 50" stroke="black" fill="none"/>)"
        R"(<path d="M10 90 Q50 60 90 90" stroke="black" fill="none"/>)"
        R"(<path d="M10 70 C30 40 90 70 90 70" stroke="black" fill="none"/>)"
        R"(</svg>)");
    std::string def = SketchDefFromSVGFile(svg);
    REQUIRE_FALSE(def.empty());
    auto sk = SketchEffectSketch::SketchFromString(def);
    REQUIRE(sk.pathCount() == 4);
    // a cubic whose second control point sits on its end point comes back as a quadratic
    CHECK(std::dynamic_pointer_cast<SketchQuadraticBezier>(sk.paths()[3]->segments()[0]) != nullptr);
    REQUIRE(sk.paths()[0]->segments().size() == 1);
    auto line = std::dynamic_pointer_cast<SketchLine>(sk.paths()[0]->segments()[0]);
    REQUIRE(line != nullptr);
    // y is flipped: SVG y=10 of 100 is sketch y=0.9
    CHECK_THAT(line->StartPoint().x, WithinAbs(0.1, 1e-4));
    CHECK_THAT(line->StartPoint().y, WithinAbs(0.9, 1e-4));
    CHECK_THAT(line->EndPoint().x, WithinAbs(0.9, 1e-4));
    CHECK(std::dynamic_pointer_cast<SketchCubicBezier>(sk.paths()[1]->segments()[0]) != nullptr);
    CHECK(sk.paths()[2]->segments().size() == 1);
    CHECK(std::dynamic_pointer_cast<SketchLine>(sk.paths()[2]->segments()[0]) == nullptr);

    CHECK(SketchDefFromSVGFile(td.File("missing.svg")).empty());
    std::string emptySvg = td.Write("empty.svg", R"(<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100"></svg>)");
    CHECK(SketchDefFromSVGFile(emptySvg).empty());
    std::string noSize = td.Write("nosize.svg", R"(<svg xmlns="http://www.w3.org/2000/svg" width="0" height="0"></svg>)");
    CHECK(SketchDefFromSVGFile(noSize).empty());
    std::string junk = td.Write("junk.svg", "not an svg at all");
    CHECK(SketchDefFromSVGFile(junk).empty());
}

// ===========================================================================
// Sketch effect rendering
// ===========================================================================

namespace {
Settings SketchDef(const std::string& def, Settings extra = {}) {
    Settings s = { { "E_TEXTCTRL_SketchDef", def } };
    s.insert(s.end(), extra.begin(), extra.end());
    return s;
}
} // namespace

TEST_CASE("Sketch effect draws a line progressively along its path", "[effects][drawing][sketch]") {
    Fixture f;
    Frames fr = f.Render("Sketch", SketchDef("0,0.5;L1,0.5", { { "E_SLIDER_DrawPercentage", "100" } }), kRed, 20, 10, Range(0, 19));
    for (int i = 1; i < 20; ++i) CHECK(fr.lit(i) >= fr.lit(i - 1));
    CHECK(fr.lit(19) > fr.lit(4));
    CHECK(fr.box(4).maxX <= 7);
    CHECK(fr.box(19).maxX >= 16);
    for (int i = 1; i < 20; ++i) {
        CHECK(fr.allLit(i, RedOnly));
        CHECK(fr.allLit(i, [](const xlColor&, int, int y) { return y >= 4 && y <= 5; }));
    }
}

TEST_CASE("Sketch effect draw percentage sets when the drawing completes", "[effects][drawing][sketch]") {
    Fixture f;
    Frames fr = f.Render("Sketch", SketchDef("0,0.5;L1,0.5", { { "E_SLIDER_DrawPercentage", "40" } }), kRed, 20, 10, Range(0, 19));
    CHECK(fr.lit(3) < fr.lit(8));
    for (int i = 9; i < 20; ++i) CHECK(fr.same(8, fr, i));
}

TEST_CASE("Sketch effect motion draws in then out", "[effects][drawing][sketch]") {
    Fixture f;
    Settings s = SketchDef("0,0.5;L1,0.5", { { "E_CHECKBOX_MotionEnabled", "1" }, { "E_SLIDER_MotionPercentage", "30" } });
    Frames fr = f.Render("Sketch", s, kRed, 20, 10, Range(0, 19));
    int peak = 0;
    for (int i = 0; i < 20; ++i) peak = std::max(peak, fr.lit(i));
    CHECK(fr.lit(19) < peak);
    CHECK(fr.box(19).minX > 8);
    CHECK(fr.box(5).minX < fr.box(15).minX);

    // a single closed path with motion chases around the loop, wrapping past the start
    Settings loop = SketchDef("0.1,0.1;L0.9,0.1;L0.9,0.9;L0.1,0.9;L0.1,0.1;c", { { "E_CHECKBOX_MotionEnabled", "1" }, { "E_SLIDER_MotionPercentage", "50" } });
    Frames lp = f.Render("Sketch", loop, kRed, 20, 20, Range(0, 19));
    for (int i = 0; i < 20; ++i) CHECK(lp.lit(i) > 0);
    CHECK_FALSE(lp.same(0, lp, 10));
    CHECK_FALSE(lp.same(10, lp, 19));
}

TEST_CASE("Sketch effect colours each path from the palette and thickness widens it", "[effects][drawing][sketch]") {
    Fixture f;
    Frames fr = f.Render("Sketch", SketchDef("0,0.25;L1,0.25|0,0.75;L1,0.75", { { "E_SLIDER_DrawPercentage", "10" } }), kRedBlue, 20, 10, { 19 });
    CHECK(fr.allLit(0, [](const xlColor& c, int, int y) { return y < 5 ? (c.green == 0 && c.blue == 0) : (c.green == 0 && c.red == 0); }));
    bool lower = false, upper = false;
    for (int x = 0; x < 20; ++x) {
        for (int y = 0; y < 10; ++y) {
            if (!IsLit(fr.at(0, x, y))) continue;
            (y < 5 ? lower : upper) = true;
        }
    }
    CHECK(lower);
    CHECK(upper);

    Frames thin = f.Render("Sketch", SketchDef("0,0.5;L1,0.5", { { "E_SLIDER_DrawPercentage", "10" } }), kRed, 20, 10, { 19 });
    Frames thick = f.Render("Sketch", SketchDef("0,0.5;L1,0.5", { { "E_SLIDER_DrawPercentage", "10" }, { "E_SLIDER_Thickness", "5" } }), kRed, 20, 10, { 19 });
    CHECK(thick.box(0).height() >= thin.box(0).height() + 3);
}

TEST_CASE("Sketch effect renders curves deterministically and nothing for empty sketches", "[effects][drawing][sketch]") {
    Fixture f;
    Settings def = SketchDef(SketchEffectSketch::DefaultSketchString(), { { "E_SLIDER_DrawPercentage", "50" } });
    Frames a = f.Render("Sketch", def, kRedBlue, 24, 24, { 0, 6, 12, 19 });
    Frames b = f.Render("Sketch", def, kRedBlue, 24, 24, { 0, 6, 12, 19 });
    CHECK(a == b);
    CHECK(a.lit(3) > a.lit(1));
    Settings motion = def;
    motion.push_back({ "E_CHECKBOX_MotionEnabled", "1" });
    Frames m = f.Render("Sketch", motion, kRedBlue, 24, 24, { 6, 12 });
    CHECK(m.lit(0) > 0);

    Frames none = f.Render("Sketch", {}, kRed, 10, 10, { 5 });
    CHECK(none.lit(0) == 0);
    Frames zero = f.Render("Sketch", SketchDef("0.5,0.5;L0.5,0.5"), kRed, 10, 10, { 5 });
    CHECK(zero.lit(0) == 0);
}

TEST_CASE("Sketch effect settings checks", "[effects][drawing][sketch]") {
    Fixture f;
    RenderableEffect* fx = f.ctx.effectManager.GetEffect("Sketch");
    REQUIRE(fx != nullptr);
    auto has = [](const std::list<std::string>& l, const std::string& needle) {
        for (const auto& s : l)
            if (s.find(needle) != std::string::npos) return true;
        return false;
    };
    Effect* e = f.Add("Sketch", {}, kRed);
    CHECK(has(fx->CheckEffectSettings(e->GetSettings(), nullptr, f.model.get(), e, false), "cant find image file"));
    TempDir td;
    Effect* e2 = f.Add("Sketch", { { "E_FILEPICKER_SketchBackground", td.File("nope.png") } }, kRed);
    CHECK(has(fx->CheckEffectSettings(e2->GetSettings(), nullptr, f.model.get(), e2, false), "cant find image file"));
    SettingsMap sm;
    CHECK(fx->GetFileReferences(&f.ctx, f.model.get(), sm).empty());
    CHECK_FALSE(fx->needToAdjustSettings("2099.1"));
    fx->adjustSettings("2016.1", e, false);
    CHECK(e->GetSettings()["E_TEXTCTRL_SketchDef"].empty());
    fx->loadFiles(e);
    fx->loadFiles(e2);
    CHECK_FALSE(e2->GetSettings()["E_FILEPICKER_SketchBackground"].empty());
}

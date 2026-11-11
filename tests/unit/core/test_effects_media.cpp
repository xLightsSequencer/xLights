#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <list>
#include <map>
#include <set>
#include <tuple>
#include <memory>
#include <string>
#include <vector>

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "effects/DMXEffect.h"
#include "effects/EffectManager.h"
#include "effects/GlediatorEffect.h"
#include "effects/MovingHeadEffect.h"
#include "effects/MovingHeadSongGenerator.h"
#include "effects/PicturesEffect.h"
#include "effects/RenderableEffect.h"
#include "effects/ServoEffect.h"
#include "effects/VideoEffect.h"
#include "media/VideoDecodeSizeRegistry.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxMovingHead.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxServo.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/DmxSkull.h"
#include "models/DMX/Servo.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "render/ValueCurve.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/UtilClasses.h"
#include "utils/xlImage.h"

namespace {

using Pixels = std::vector<xlColor>;

struct Frames {
    int w = 0;
    int h = 0;
    std::vector<Pixels> f;

    const xlColor& at(size_t frame, int x, int y) const { return f[frame][(size_t)y * w + x]; }
    int lit(size_t frame) const {
        int n = 0;
        for (const auto& c : f[frame]) {
            if (c.red || c.green || c.blue) ++n;
        }
        return n;
    }
    double centroidX(size_t frame) const {
        double sx = 0;
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const auto& c = at(frame, x, y);
                if (c.red || c.green || c.blue) {
                    sx += x;
                    ++n;
                }
            }
        }
        return n ? sx / n : -1;
    }
    double centroidY(size_t frame) const {
        double sy = 0;
        int n = 0;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const auto& c = at(frame, x, y);
                if (c.red || c.green || c.blue) {
                    sy += y;
                    ++n;
                }
            }
        }
        return n ? sy / n : -1;
    }
    // DMX buffers are one row of channels; channel is 1-based.
    int ch(size_t frame, int channel) const { return f[frame][channel - 1].red; }
};

bool Rgb(const xlColor& c, int r, int g, int b) { return c.red == r && c.green == g && c.blue == b; }

std::vector<int> Range(int from, int to) {
    std::vector<int> r;
    for (int i = from; i <= to; ++i) r.push_back(i);
    return r;
}

void EnsureLoggers() {
    for (const char* n : { "render", "job", "work", "curl", "opengl" }) {
        if (!spdlog::get(n)) {
            spdlog::register_logger(std::make_shared<spdlog::logger>(n, std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

std::string Dmx(const std::string& name, const std::string& type, const std::string& attrs, const std::string& children = "") {
    return "<model name=\"" + name + "\" DisplayAs=\"" + type + "\" StartChannel=\"1\" StringType=\"Single Color White\" " + attrs + ">" + children + "</model>";
}

std::string Line(const std::string& name, int nodes, const std::string& type = "RGB Nodes") {
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) + "\" StringType=\"" + type + "\"/>";
}

// One model in a windowless show. The buffer is driven directly (Clear, SetState, Render)
// the way RenderEngine drives it, with no layering or GPU work.
struct Fx {
    FixFileReset reset;
    HeadlessRenderContext ctx;
    Model* model = nullptr;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;

    // `others` are added first (e.g. the members of a group passed as modelXml).
    explicit Fx(const std::string& modelXml, const std::vector<std::string>& others = {}) {
        EnsureLoggers();
        for (const auto& x : others) {
            pugi::xml_document od;
            REQUIRE(od.load_string(x.c_str()));
            REQUIRE(ctx.AllModels.createAndAddModel(od.first_child(), 100, 100) != nullptr);
        }
        pugi::xml_document doc;
        REQUIRE(doc.load_string(modelXml.c_str()));
        model = ctx.AllModels.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(model != nullptr);
        ctx.AllModels.RecalcStartChannels();
        element = ctx._sequenceElements.AddElement(model->GetName(), "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    Effect* Add(const std::string& name, const std::string& settings, const std::map<std::string, std::string>& extra = {},
                int startMs = 0, int endMs = 1000, const std::string& palette = "") {
        Effect* e = layer->AddEffect(0, name, settings, palette, startMs, endMs, 0, false);
        REQUIRE(e != nullptr);
        for (const auto& [k, v] : extra) {
            e->GetSettings()[k] = v;
        }
        return e;
    }

    RenderableEffect* Get(const std::string& name) {
        RenderableEffect* fx = ctx.effectManager.GetEffect(name);
        REQUIRE(fx != nullptr);
        return fx;
    }

    Frames Run(Effect* e, int w, int h, const std::vector<int>& periods, const xlColor* prefill = nullptr, bool allowAlpha = false) {
        RenderableEffect* fx = Get(e->GetEffectName());
        RenderBuffer rb(&ctx, nullptr, model);
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
        rb.SetEffectDuration(e->GetStartTimeMS(), e->GetEndTimeMS());
        rb.SetAllowAlphaChannel(allowAlpha);

        SettingsMap settings;
        e->CopySettingsMap(settings, true);

        Frames out;
        out.w = w;
        out.h = h;
        bool first = true;
        for (int p : periods) {
            rb.Clear();
            if (prefill != nullptr) {
                std::fill(rb.GetPixels(), rb.GetPixels() + (size_t)w * h, *prefill);
            }
            rb.SetState(p, first);
            first = false;
            fx->Render(e, settings, rb);
            out.f.emplace_back(rb.GetPixels(), rb.GetPixels() + (size_t)w * h);
        }
        return out;
    }

    // DMX buffer: one row, one pixel per channel.
    Frames RunDmx(Effect* e, const std::vector<int>& periods) {
        return Run(e, (int)model->GetNodeCount(), 1, periods);
    }
};

// ---------------------------------------------------------------- image files

void WriteBytes(const std::string& path, const std::string& bytes) {
    std::ofstream(path, std::ios::binary).write(bytes.data(), (std::streamsize)bytes.size());
}

std::string WritePng(const TempDir& d, const std::string& name, const xlImage& img) {
    std::vector<uint8_t> png;
    REQUIRE(img.SaveAsPNG(png));
    WriteBytes(d.File(name), std::string(png.begin(), png.end()));
    return d.File(name);
}

// Four solid quadrants: top-left red, top-right green, bottom-left blue, bottom-right white.
xlImage Quadrants(int w, int h) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            bool left = x < w / 2;
            bool top = y < h / 2;
            if (top && left) img.SetRGBA(x, y, 255, 0, 0, 255);
            else if (top) img.SetRGBA(x, y, 0, 255, 0, 255);
            else if (left) img.SetRGBA(x, y, 0, 0, 255, 255);
            else img.SetRGBA(x, y, 255, 255, 255, 255);
        }
    }
    return img;
}

xlImage Solid(int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.SetRGBA(x, y, r, g, b, a);
        }
    }
    return img;
}

void Put16(std::string& s, int v) {
    s += (char)(v & 0xFF);
    s += (char)((v >> 8) & 0xFF);
}

// Uncompressed-style GIF: a clear code before every literal keeps the code width at 3 bits.
std::string Gif(int w, int h, const std::vector<xlColor>& palette, const std::vector<std::vector<int>>& frames, int delayCs) {
    std::string g = "GIF89a";
    Put16(g, w);
    Put16(g, h);
    g += (char)0x81; // global colour table, 4 entries
    g += '\0';
    g += '\0';
    for (int i = 0; i < 4; ++i) {
        xlColor c = i < (int)palette.size() ? palette[i] : xlBLACK;
        g += (char)c.red;
        g += (char)c.green;
        g += (char)c.blue;
    }
    for (const auto& idx : frames) {
        g += std::string("\x21\xF9\x04\x00", 4);
        Put16(g, delayCs);
        g += std::string("\x00\x00", 2);
        g += '\x2C';
        Put16(g, 0);
        Put16(g, 0);
        Put16(g, w);
        Put16(g, h);
        g += '\0';
        g += '\x02';
        std::vector<int> codes;
        for (int v : idx) {
            codes.push_back(4);
            codes.push_back(v);
        }
        codes.push_back(5);
        std::string data;
        uint32_t acc = 0;
        int bits = 0;
        for (int c : codes) {
            acc |= (uint32_t)c << bits;
            bits += 3;
            while (bits >= 8) {
                data += (char)(acc & 0xFF);
                acc >>= 8;
                bits -= 8;
            }
        }
        if (bits > 0) data += (char)(acc & 0xFF);
        for (size_t p = 0; p < data.size(); p += 255) {
            size_t n = std::min<size_t>(255, data.size() - p);
            g += (char)n;
            g += data.substr(p, n);
        }
        g += '\0';
    }
    g += '\x3B';
    return g;
}

int CountColour(const Frames& fr, size_t frame, int r, int g, int b) {
    int n = 0;
    for (const auto& c : fr.f[frame]) {
        if (Rgb(c, r, g, b)) ++n;
    }
    return n;
}

bool AllColour(const Frames& fr, size_t frame, int r, int g, int b) {
    return CountColour(fr, frame, r, g, b) == fr.w * fr.h;
}

} // namespace

// ================================================================== Pictures

TEST_CASE("Pictures with no movement centres the image without scaling", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "q.png", Quadrants(4, 4));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "E_CHOICE_Pictures_Direction=none,E_CHOICE_Scaling=No Scaling", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 8, 8, { 0, 10 });
    for (size_t i = 0; i < fr.f.size(); ++i) {
        CHECK(fr.lit(i) == 16);
        // Image pixel (x, y) lands at buffer (x + 2, 5 - y); buffer y = 0 is the bottom row.
        CHECK(Rgb(fr.at(i, 2, 5), 255, 0, 0));
        CHECK(Rgb(fr.at(i, 5, 5), 0, 255, 0));
        CHECK(Rgb(fr.at(i, 2, 2), 0, 0, 255));
        CHECK(Rgb(fr.at(i, 5, 2), 255, 255, 255));
        CHECK(Rgb(fr.at(i, 1, 5), 0, 0, 0));
        CHECK(Rgb(fr.at(i, 2, 6), 0, 0, 0));
    }
}

TEST_CASE("Pictures paints the buffer red when the image is missing", "[effects][media]") {
    TempDir d;
    Fx f(Line("L", 64));
    Effect* missing = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", d.File("nope.png") } });
    Frames fr = f.Run(missing, 5, 3, { 0, 1 });
    CHECK(AllColour(fr, 0, 255, 0, 0));
    CHECK(AllColour(fr, 1, 255, 0, 0));

    Effect* none = f.Add("Pictures", "");
    Frames fr2 = f.Run(none, 5, 3, { 0 });
    CHECK(AllColour(fr2, 0, 255, 0, 0));

    // A file that exists but does not decode is replaced by a blank 5x5 placeholder, not red.
    std::string junk = d.Write("junk.png", "this is not a png");
    Effect* bad = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", junk } });
    Frames fr3 = f.Run(bad, 5, 3, { 0 });
    CHECK(CountColour(fr3, 0, 255, 0, 0) == 0);
}

TEST_CASE("Pictures scale to fit fills the buffer keeping the quadrants", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "q.png", Quadrants(4, 4));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "E_CHOICE_Scaling=Scale To Fit", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 8, 8, { 0 });
    CHECK(fr.lit(0) == 64);
    // Corners come from the matching quadrant (top of the image is the top of the buffer).
    CHECK(fr.at(0, 0, 7).red > 200);
    CHECK(fr.at(0, 0, 7).green < 50);
    CHECK(fr.at(0, 7, 7).green > 200);
    CHECK(fr.at(0, 7, 7).red < 50);
    CHECK(fr.at(0, 0, 0).blue > 200);
    CHECK(fr.at(0, 0, 0).red < 50);
    CHECK(fr.at(0, 7, 0).red > 200);
    CHECK(fr.at(0, 7, 0).green > 200);
}

TEST_CASE("Pictures keep aspect ratio letterboxes and crop fills", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "wide.png", Solid(4, 2, 0, 200, 0));
    Fx f(Line("L", 64));
    Effect* keep = f.Add("Pictures", "E_CHOICE_Scaling=Scale Keep Aspect Ratio", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fk = f.Run(keep, 8, 8, { 0 });
    CHECK(fk.lit(0) == 32);
    for (int x = 0; x < 8; ++x) {
        CHECK(fk.at(0, x, 0).green == 0);
        CHECK(fk.at(0, x, 7).green == 0);
        CHECK(fk.at(0, x, 4).green > 150);
    }
    Effect* crop = f.Add("Pictures", "E_CHOICE_Scaling=Scale Keep Aspect Ratio Crop", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fc = f.Run(crop, 8, 8, { 0 });
    CHECK(fc.lit(0) == 64);
}

TEST_CASE("Pictures movement directions move the image the right way", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(2, 2, 255, 128, 0));
    Fx f(Line("L", 64));
    auto run = [&](const std::string& dir) {
        Effect* e = f.Add("Pictures", "E_CHOICE_Pictures_Direction=" + dir, { { "E_TEXTCTRL_Pictures_Filename", png } });
        return f.Run(e, 10, 10, { 6, 8, 10, 12 });
    };
    Frames left = run("left");
    Frames right = run("right");
    Frames up = run("up");
    Frames down = run("down");
    for (size_t i = 1; i < 4; ++i) {
        INFO("frame " << i);
        CHECK(left.centroidX(i) < left.centroidX(i - 1));
        CHECK(right.centroidX(i) > right.centroidX(i - 1));
        CHECK(up.centroidY(i) > up.centroidY(i - 1));
        CHECK(down.centroidY(i) < down.centroidY(i - 1));
        // horizontal moves keep the row, vertical moves keep the column
        CHECK(left.centroidY(i) == left.centroidY(0));
        CHECK(up.centroidX(i) == up.centroidX(0));
    }
    Frames ul = run("up-left");
    Frames dr = run("down-right");
    Frames ur = run("up-right");
    Frames dl = run("down-left");
    CHECK(ul.centroidX(3) < ul.centroidX(0));
    CHECK(ul.centroidY(3) > ul.centroidY(0));
    CHECK(dr.centroidX(3) > dr.centroidX(0));
    CHECK(dr.centroidY(3) < dr.centroidY(0));
    CHECK(ur.centroidX(3) > ur.centroidX(0));
    CHECK(ur.centroidY(3) > ur.centroidY(0));
    CHECK(dl.centroidX(3) < dl.centroidX(0));
    CHECK(dl.centroidY(3) < dl.centroidY(0));
}

TEST_CASE("Pictures offsets shift the image by percent or by pixel", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(2, 2, 0, 0, 255));
    Fx f(Line("L", 64));
    Effect* base = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames b = f.Run(base, 8, 8, { 0 });
    REQUIRE(b.lit(0) == 4);
    Effect* pct = f.Add("Pictures", "E_SLIDER_PicturesXC=25,E_SLIDER_PicturesYC=-25", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames p = f.Run(pct, 8, 8, { 0 });
    CHECK(p.centroidX(0) == b.centroidX(0) + 2);
    CHECK(p.centroidY(0) == b.centroidY(0) - 2);
    Effect* px = f.Add("Pictures", "E_SLIDER_PicturesXC=1,E_CHECKBOX_Pictures_PixelOffsets=1", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames x = f.Run(px, 8, 8, { 0 });
    CHECK(x.centroidX(0) == b.centroidX(0) + 1);
}

TEST_CASE("Pictures wrap X brings pixels pushed off the right edge back on the left", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(4, 2, 255, 0, 0));
    Fx f(Line("L", 64));
    // 4 wide image centred on 8 wide buffer covers x = 2..5; +4 pixels puts it at 6..9.
    Effect* nowrap = f.Add("Pictures", "E_SLIDER_PicturesXC=4,E_CHECKBOX_Pictures_PixelOffsets=1", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Effect* wrap = f.Add("Pictures", "E_SLIDER_PicturesXC=4,E_CHECKBOX_Pictures_PixelOffsets=1,E_CHECKBOX_Pictures_WrapX=1", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames a = f.Run(nowrap, 8, 8, { 0 });
    Frames b = f.Run(wrap, 8, 8, { 0 });
    CHECK(a.lit(0) == 4);
    CHECK(b.lit(0) == 8);
    CHECK(a.at(0, 0, 4).red == 0);
    CHECK(b.at(0, 0, 4).red == 255);
    CHECK(b.at(0, 1, 4).red == 255);
    CHECK(b.at(0, 2, 4).red == 0);
}

TEST_CASE("Pictures transparent black skips dark pixels", "[effects][media]") {
    TempDir d;
    xlImage img = Solid(2, 2, 0, 0, 0);
    img.SetRGBA(0, 0, 255, 255, 255, 255);
    img.SetRGBA(1, 0, 10, 10, 10, 255);
    std::string png = WritePng(d, "dark.png", img);
    Fx f(Line("L", 64));
    const xlColor fill(0, 40, 0);
    Effect* off = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Effect* on = f.Add("Pictures", "E_CHECKBOX_Pictures_TransparentBlack=1,E_TEXTCTRL_Pictures_TransparentBlack=50", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames a = f.Run(off, 4, 4, { 0 }, &fill);
    Frames b = f.Run(on, 4, 4, { 0 }, &fill);
    // Image covers x 1..2, y 1..2; image row 0 is buffer row 2.
    CHECK(Rgb(a.at(0, 1, 2), 255, 255, 255));
    CHECK(Rgb(a.at(0, 2, 2), 10, 10, 10));
    CHECK(Rgb(a.at(0, 1, 1), 0, 0, 0));
    CHECK(Rgb(b.at(0, 1, 2), 255, 255, 255));
    CHECK(Rgb(b.at(0, 2, 2), 0, 40, 0)); // level 30 <= 50
    CHECK(Rgb(b.at(0, 1, 1), 0, 40, 0));
}

TEST_CASE("Pictures transparent pixels keep what was underneath", "[effects][media]") {
    TempDir d;
    xlImage img = Solid(2, 2, 255, 0, 0);
    img.SetRGBA(0, 0, 0, 0, 255, 0);  // fully transparent
    img.SetRGBA(1, 0, 0, 255, 0, 30); // mostly transparent
    std::string png = WritePng(d, "alpha.png", img);
    Fx f(Line("L", 64));
    const xlColor fill(0, 0, 77);
    Effect* e = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 4, 4, { 0 }, &fill);
    CHECK(Rgb(fr.at(0, 1, 2), 0, 0, 77)); // skipped
    CHECK(Rgb(fr.at(0, 2, 2), 0, 0, 0));  // alpha < 64 without alpha support draws black
    CHECK(Rgb(fr.at(0, 1, 1), 255, 0, 0));
}

TEST_CASE("Pictures shimmer turns off some lit pixels", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(8, 8, 255, 255, 0));
    Fx f(Line("L", 64));
    Effect* plain = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Effect* sh = f.Add("Pictures", "E_CHECKBOX_Pictures_Shimmer=1", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames a = f.Run(plain, 8, 8, { 0, 1, 2 });
    Frames b = f.Run(sh, 8, 8, { 0, 1, 2 });
    for (size_t i = 0; i < 3; ++i) {
        CHECK(a.lit(i) == 64);
        CHECK(b.lit(i) > 5);
        CHECK(b.lit(i) < 59);
        for (const auto& c : b.f[i]) {
            CHECK((Rgb(c, 255, 255, 0) || Rgb(c, 0, 0, 0)));
        }
    }
    Frames b2 = f.Run(sh, 8, 8, { 0, 1, 2 });
    CHECK(b.f == b2.f);
}

TEST_CASE("Pictures vector movement interpolates between start and end offsets", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(2, 2, 0, 255, 255));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "E_CHOICE_Pictures_Direction=vector,E_SLIDER_PicturesXC=-50,E_SLIDER_PicturesEndXC=50,E_SLIDER_PicturesYC=0,E_SLIDER_PicturesEndYC=0",
                      { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 20, 4, { 0, 10, 19 });
    CHECK(fr.centroidX(0) < fr.centroidX(1));
    CHECK(fr.centroidX(1) < fr.centroidX(2));
    // -50% of 20 pixels pushes the centred image (x 9..10) to x -1..0: one column shows.
    CHECK(fr.lit(0) == 2);
    CHECK(fr.centroidX(0) == 0.0);
    CHECK(fr.lit(1) == 4);
    CHECK(fr.centroidX(1) == Catch::Approx(9.5).margin(1.0));
    CHECK(fr.centroidY(0) == fr.centroidY(2));
}

TEST_CASE("Pictures start and end scale grow the image over the effect", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(4, 4, 255, 0, 255));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "E_SLIDER_Pictures_StartScale=50,E_SLIDER_Pictures_EndScale=200", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 12, 12, { 0, 10, 19 });
    CHECK(fr.lit(0) == 4);
    CHECK(fr.lit(1) > fr.lit(0));
    CHECK(fr.lit(2) > fr.lit(1));
    CHECK(fr.lit(2) <= 64);
}

TEST_CASE("Pictures exotic movements draw only image colours", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "q.png", Quadrants(4, 4));
    Fx f(Line("L", 64));
    for (const char* dir : { "peekaboo", "peekaboo 90", "peekaboo 180", "peekaboo 270", "wiggle", "zoom in", "flag wave", "up once", "down once",
                             "tile-left", "tile-right", "tile-up", "tile-down", "bogus" }) {
        INFO(dir);
        Effect* e = f.Add("Pictures", std::string("E_CHOICE_Pictures_Direction=") + dir + ",E_TEXTCTRL_Pictures_Speed=2", { { "E_TEXTCTRL_Pictures_Filename", png } });
        Frames fr = f.Run(e, 8, 8, { 0, 5, 10, 15, 19 });
        int total = 0;
        for (size_t i = 0; i < fr.f.size(); ++i) {
            total += fr.lit(i);
            for (const auto& c : fr.f[i]) {
                bool ok = Rgb(c, 0, 0, 0) || Rgb(c, 255, 0, 0) || Rgb(c, 0, 255, 0) || Rgb(c, 0, 0, 255) || Rgb(c, 255, 255, 255);
                CHECK(ok);
            }
        }
        CHECK(total > 0);
    }
}

TEST_CASE("Pictures tiles fill the buffer and scroll with time", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "q.png", Quadrants(2, 2));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "E_CHOICE_Pictures_Direction=tile-left,E_TEXTCTRL_Pictures_Speed=1", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 6, 6, { 0, 1, 2 });
    CHECK(fr.lit(0) == 36);
    CHECK(fr.f[0] != fr.f[1]);
    CHECK(fr.f[0] == fr.f[2]); // tile period is the image width
}

TEST_CASE("Pictures once movements stop at the end", "[effects][media]") {
    TempDir d;
    std::string png = WritePng(d, "s.png", Solid(2, 2, 255, 255, 255));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "E_CHOICE_Pictures_Direction=up once,E_TEXTCTRL_Pictures_Speed=2", { { "E_TEXTCTRL_Pictures_Filename", png } });
    Frames fr = f.Run(e, 6, 6, { 2, 4, 10, 15, 19 });
    CHECK(fr.centroidY(1) > fr.centroidY(0));
    CHECK(fr.f[2] == fr.f[3]);
    CHECK(fr.f[3] == fr.f[4]);
}

TEST_CASE("Pictures animated GIF steps frames by time or across the effect", "[effects][media]") {
    TempDir d;
    std::string gif = d.File("anim.gif");
    WriteBytes(gif, Gif(2, 2, { xlRED, xlBLUE }, { { 0, 0, 0, 0 }, { 1, 1, 1, 1 } }, 10));
    Fx f(Line("L", 64));
    Effect* loop = f.Add("Pictures", "E_CHECKBOX_LoopGIF=1,E_CHOICE_Scaling=Scale To Fit", { { "E_TEXTCTRL_Pictures_Filename", gif } });
    Frames l = f.Run(loop, 2, 2, { 0, 1, 2, 3, 4, 6 });
    CHECK(AllColour(l, 0, 255, 0, 0));
    CHECK(AllColour(l, 1, 255, 0, 0));
    CHECK(AllColour(l, 2, 0, 0, 255));
    CHECK(AllColour(l, 3, 0, 0, 255));
    CHECK(AllColour(l, 4, 255, 0, 0));
    CHECK(AllColour(l, 5, 0, 0, 255));

    Effect* fit = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", gif } });
    Frames s = f.Run(fit, 2, 2, { 0, 5, 9, 10, 15, 19 });
    CHECK(AllColour(s, 0, 255, 0, 0));
    CHECK(AllColour(s, 1, 255, 0, 0));
    CHECK(AllColour(s, 2, 255, 0, 0));
    CHECK(AllColour(s, 3, 0, 0, 255));
    CHECK(AllColour(s, 5, 0, 0, 255));

    // Frame rate adjust doubles how fast the frames step across the effect.
    Effect* fast = f.Add("Pictures", "E_TEXTCTRL_Pictures_FrameRateAdj=2", { { "E_TEXTCTRL_Pictures_Filename", gif } });
    Frames q = f.Run(fast, 2, 2, { 0, 6 });
    CHECK(AllColour(q, 0, 255, 0, 0));
    CHECK(AllColour(q, 1, 0, 0, 255));
}

TEST_CASE("Pictures settings helpers", "[effects][media]") {
    CHECK(PicturesEffect::IsPictureFile("a.PNG"));
    CHECK(PicturesEffect::IsPictureFile("/x/y.jpeg"));
    CHECK(PicturesEffect::IsPictureFile("b.gif"));
    CHECK(PicturesEffect::IsPictureFile("b.svg"));
    CHECK_FALSE(PicturesEffect::IsPictureFile("b.mp4"));
    CHECK_FALSE(PicturesEffect::IsPictureFile("noext"));

    Fx f(Line("L", 64));
    auto* pe = f.Get("Pictures");
    Effect* e = f.Add("Pictures", "E_CHECKBOX_Pictures_ScaleToFit=1,E_CHECKBOX_Pictures_ForceGIFOverlay=1");
    CHECK(pe->needToAdjustSettings("2016.1"));
    pe->adjustSettings("2016.1", e, false);
    CHECK(e->GetSettings().Get("E_CHOICE_Scaling", "") == "Scale To Fit");
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Pictures_ScaleToFit"));
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Pictures_ForceGIFOverlay"));
    Effect* e2 = f.Add("Pictures", "E_CHECKBOX_Pictures_ScaleToFit=0");
    pe->adjustSettings("2016.1", e2, false);
    CHECK(e2->GetSettings().Get("E_CHOICE_Scaling", "") == "No Scaling");

    SettingsMap sm;
    CHECK(pe->GetFileReferences(&f.ctx, f.model, sm).empty());
    sm["E_TEXTCTRL_Pictures_Filename"] = "pic.png";
    CHECK(pe->GetFileReferences(&f.ctx, f.model, sm).size() == 1);
}

TEST_CASE("Pictures check settings reports missing, invalid and oversized images", "[effects][media]") {
    TempDir d;
    std::string big = WritePng(d, "big.png", Solid(64, 64, 1, 2, 3));
    std::string junk = d.Write("junk.png", "nope");
    Fx f(Line("L", 4));
    auto* pe = f.Get("Pictures");
    auto has = [](const std::list<std::string>& l, const std::string& s) {
        return std::any_of(l.begin(), l.end(), [&](const std::string& x) { return x.find(s) != std::string::npos; });
    };
    Effect* e = f.Add("Pictures", "");
    auto r = pe->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false);
    CHECK(has(r, "cant find image file"));

    // An undecodable file loads as a placeholder, so it is only flagged as outside the show.
    e->GetSettings()["E_TEXTCTRL_Pictures_Filename"] = junk;
    r = pe->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false);
    CHECK_FALSE(has(r, "cant find image file"));
    CHECK(has(r, "not under show directory"));

    e->GetSettings()["E_TEXTCTRL_Pictures_Filename"] = big;
    r = pe->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false);
    CHECK(has(r, "not under show directory"));
    CHECK(has(r, "times the height or width"));
    r = pe->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, true);
    CHECK_FALSE(has(r, "times the height or width"));
}

TEST_CASE("Pictures numbered image sequences play as frames", "[effects][media]") {
    TempDir d;
    std::string first = WritePng(d, "seq-1.png", Solid(2, 2, 255, 0, 0));
    WritePng(d, "seq-2.png", Solid(2, 2, 0, 255, 0));
    WritePng(d, "seq-3.png", Solid(2, 2, 0, 0, 255));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Pictures", "", { { "E_TEXTCTRL_Pictures_Filename", first } });
    f.Get("Pictures")->loadFiles(e);
    Frames fr = f.Run(e, 2, 2, { 0, 1, 2, 3, 19 });
    CHECK(AllColour(fr, 0, 255, 0, 0));
    CHECK(AllColour(fr, 1, 0, 255, 0));
    CHECK(AllColour(fr, 2, 0, 0, 255));
    CHECK(AllColour(fr, 4, 0, 0, 255));
}

// ================================================================== Check Sequence: Morph, Liquid

namespace {
bool HasWarning(const std::list<std::string>& l, const std::string& s) {
    return std::any_of(l.begin(), l.end(), [&](const std::string& x) { return x.find(s) != std::string::npos; });
}

// A horizontal matrix: each string is a row, so NumStrings is the height.
std::string MatrixXml(int wi, int ht) {
    return "<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"" + std::to_string(ht) +
           "\" NodesPerString=\"" + std::to_string(wi) + "\" StringType=\"RGB Nodes\"/>";
}

// A horizontal line from X 0..width at Y 0 that morphs straight down to Y 100,
// so Render() repeats it along X.
std::string FallingLine(int width, int count, int skip) {
    auto v = [](int n) { return std::to_string(n); };
    return "E_SLIDER_Morph_Start_X1=0,E_SLIDER_Morph_Start_X2=" + v(width) +
           ",E_SLIDER_Morph_Start_Y1=0,E_SLIDER_Morph_Start_Y2=0" +
           ",E_SLIDER_Morph_End_X1=0,E_SLIDER_Morph_End_X2=" + v(width) +
           ",E_SLIDER_Morph_End_Y1=100,E_SLIDER_Morph_End_Y2=100" +
           ",E_SLIDER_Morph_Repeat_Count=" + v(count) +
           ",E_SLIDER_Morph_Repeat_Skip=" + v(skip);
}

// The same line turned on its side: a vertical line from Y 0..height at X 0
// that morphs right to X 100, so Render() repeats it along Y.
std::string SlidingLine(int height, int count, int skip) {
    auto v = [](int n) { return std::to_string(n); };
    return "E_SLIDER_Morph_Start_X1=0,E_SLIDER_Morph_Start_X2=0" +
           std::string(",E_SLIDER_Morph_Start_Y1=0,E_SLIDER_Morph_Start_Y2=") + v(height) +
           ",E_SLIDER_Morph_End_X1=100,E_SLIDER_Morph_End_X2=100" +
           ",E_SLIDER_Morph_End_Y1=0,E_SLIDER_Morph_End_Y2=" + v(height) +
           ",E_SLIDER_Morph_Repeat_Count=" + v(count) +
           ",E_SLIDER_Morph_Repeat_Skip=" + v(skip);
}

bool MorphWarns(int wi, int ht, const std::string& settings) {
    Fx f(MatrixXml(wi, ht));
    REQUIRE(f.model->GetDefaultBufferWi() == wi);
    REQUIRE(f.model->GetDefaultBufferHt() == ht);
    Effect* e = f.Add("Morph", settings);
    return HasWarning(f.Get("Morph")->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false),
                      "repeat count and skip");
}
} // namespace

TEST_CASE("Morph check measures repeated copies by their real span", "[effects][media]") {
    // 32 wide: a 25% line is 8 px + 1. Copies span 9 + count * skip; the limit is 2 * 32.
    CHECK_FALSE(MorphWarns(32, 32, FallingLine(25, 20, 2)));  // 49
    CHECK_FALSE(MorphWarns(32, 32, FallingLine(25, 27, 2)));  // 63
    CHECK(MorphWarns(32, 32, FallingLine(25, 28, 2)));        // 65
    CHECK_FALSE(MorphWarns(32, 32, FallingLine(25, 0, 50)));  // no repeats
}

TEST_CASE("Morph check uses the axis the copies are repeated along", "[effects][media]") {
    // Narrow and tall: a falling line repeats across the 16 px width, so 20 copies
    // 2 px apart (5 + 40 = 45 > 32) overshoot even though the model is 64 tall.
    CHECK(MorphWarns(16, 64, FallingLine(25, 20, 2)));
    // The same settings sliding sideways repeat down the 64 px height: fine.
    CHECK_FALSE(MorphWarns(16, 64, SlidingLine(25, 20, 2)));
    // And on the wide-and-short model the two swap.
    CHECK_FALSE(MorphWarns(64, 16, FallingLine(25, 20, 2)));
    CHECK(MorphWarns(64, 16, SlidingLine(25, 20, 2)));
}

TEST_CASE("Morph check skips Auto Repeat and value-curve repeats", "[effects][media]") {
    // Auto Repeat replaces the Repeat Count slider with its own count.
    CHECK_FALSE(MorphWarns(16, 16, FallingLine(25, 200, 5) + ",E_CHECKBOX_Morph_AutoRepeat=1"));
    CHECK_FALSE(MorphWarns(16, 16, FallingLine(25, 200, 5) +
                           ",E_VALUECURVE_Morph_Repeat_Count=Active=TRUE|Id=ID_VALUECURVE_Morph_Repeat_Count|Type=Flat|Min=0.00|Max=250.00|P1=10.00|"));
    CHECK(MorphWarns(16, 16, FallingLine(25, 200, 5)));
}

namespace {
bool LiquidWarns(const std::string& settings, int endMs) {
    Fx f(MatrixXml(16, 16));
    Effect* e = f.Add("Liquid", settings, {}, 0, endMs);
    return HasWarning(f.Get("Liquid")->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false),
                      "Particle count will be limited");
}
} // namespace

TEST_CASE("Liquid check estimates particles from Life Time, not effect length", "[effects][media]") {
    // Flow 500 = 2000/s from one emitter; Life Time 300 = 3 s, about 3.3 s with
    // the renderer's random spread: about 6600 live particles however long the effect.
    CHECK_FALSE(LiquidWarns("E_SLIDER_LifeTime=300,E_SLIDER_Flow1=500", 600000));

    // Flow 1000 = 20000/s. 10 s Life Time lives about 11 s: 220000, over the cap.
    CHECK(LiquidWarns("E_SLIDER_LifeTime=1000,E_SLIDER_Flow1=1000", 30000));

    // 4.8 s of Life Time is 96000 nominal but about 105600 on average.
    CHECK(LiquidWarns("E_SLIDER_LifeTime=480,E_SLIDER_Flow1=1000", 30000));
    CHECK_FALSE(LiquidWarns("E_SLIDER_LifeTime=430,E_SLIDER_Flow1=1000", 30000)); // ~94600
}

TEST_CASE("Liquid check counts Warm Up Time toward reaching the steady state", "[effects][media]") {
    // A 1 s effect only builds up 20000 particles...
    CHECK_FALSE(LiquidWarns("E_SLIDER_LifeTime=1000,E_SLIDER_Flow1=1000", 1000));
    // ...unless 10 s of warm-up has already filled the model.
    CHECK(LiquidWarns("E_SLIDER_LifeTime=1000,E_SLIDER_Flow1=1000,E_TEXTCTRL_WarmUpTime=1000", 1000));
    // Life Time 0 never ages particles out, so the run length is the limit.
    CHECK_FALSE(LiquidWarns("E_SLIDER_LifeTime=0,E_SLIDER_Flow1=1000", 4000));
    CHECK(LiquidWarns("E_SLIDER_LifeTime=0,E_SLIDER_Flow1=1000", 6000));
}

// ================================================================== Glediator

namespace {
// Frame k: pixel (x, row) = (10*k + x, 100 + row, 200 + k) with row 0 the top of the model.
std::string GledFrames(int w, int h, int frames) {
    std::string s;
    for (int k = 0; k < frames; ++k) {
        for (int row = 0; row < h; ++row) {
            for (int x = 0; x < w; ++x) {
                s += (char)(10 * k + x);
                s += (char)(100 + row);
                s += (char)(200 + k);
            }
        }
    }
    return s;
}

bool IsGledFrame(const Frames& fr, size_t i, int k) {
    for (int row = 0; row < fr.h; ++row) {
        for (int x = 0; x < fr.w; ++x) {
            if (!Rgb(fr.at(i, x, fr.h - 1 - row), 10 * k + x, 100 + row, 200 + k)) return false;
        }
    }
    return true;
}
} // namespace

TEST_CASE("Glediator plays frames top row first and blanks after the end", "[effects][media]") {
    TempDir d;
    std::string file = d.Write("a.gled", GledFrames(3, 2, 3));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Normal", { { "E_FILEPICKERCTRL_Glediator_Filename", file } });
    const xlColor fill(9, 9, 9);
    Frames fr = f.Run(e, 3, 2, { 0, 1, 2, 3, 5 }, &fill);
    CHECK(IsGledFrame(fr, 0, 0));
    CHECK(IsGledFrame(fr, 1, 1));
    CHECK(IsGledFrame(fr, 2, 2));
    CHECK(AllColour(fr, 3, 0, 0, 0));
    CHECK(AllColour(fr, 4, 0, 0, 0));
}

TEST_CASE("Glediator loop restarts the file", "[effects][media]") {
    TempDir d;
    std::string file = d.Write("a.gled", GledFrames(3, 2, 3));
    Fx f(Line("L", 64));
    Effect* e = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Loop", { { "E_FILEPICKERCTRL_Glediator_Filename", file } });
    Frames fr = f.Run(e, 3, 2, Range(0, 7));
    for (int p = 0; p <= 7; ++p) {
        INFO("period " << p);
        CHECK(IsGledFrame(fr, p, p % 3));
    }
}

TEST_CASE("Glediator slow/accelerate stretches the file over the effect", "[effects][media]") {
    TempDir d;
    std::string file = d.Write("a.gled", GledFrames(3, 2, 5));
    Fx f(Line("L", 64));
    // 10 frame effect, 5 frame file: each file frame shows twice.
    Effect* slow = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Slow/Accelerate", { { "E_FILEPICKERCTRL_Glediator_Filename", file } }, 0, 500);
    Frames fr = f.Run(slow, 3, 2, Range(0, 9));
    for (int p = 0; p <= 9; ++p) {
        INFO("period " << p);
        CHECK(IsGledFrame(fr, p, p / 2));
    }
    // 2 frame effect, 5 frame file: frames are skipped to fit.
    Effect* fast = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Slow/Accelerate", { { "E_FILEPICKERCTRL_Glediator_Filename", file } }, 0, 100);
    Frames fq = f.Run(fast, 3, 2, { 0, 1 });
    CHECK(IsGledFrame(fq, 0, 0));
    CHECK(IsGledFrame(fq, 1, 2));
}

TEST_CASE("Glediator CSV files are one grey channel per pixel", "[effects][media]") {
    TempDir d;
    std::string file = d.Write("a.csv", "10,20,30,40,50,60\n70,80,90,100,110,120\n");
    Fx f(Line("L", 64));
    Effect* e = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Loop", { { "E_FILEPICKERCTRL_Glediator_Filename", file } });
    Frames fr = f.Run(e, 3, 2, { 0, 1, 2 });
    CHECK(Rgb(fr.at(0, 0, 1), 10, 10, 10));
    CHECK(Rgb(fr.at(0, 2, 1), 30, 30, 30));
    CHECK(Rgb(fr.at(0, 0, 0), 40, 40, 40));
    CHECK(Rgb(fr.at(0, 2, 0), 60, 60, 60));
    CHECK(Rgb(fr.at(1, 1, 0), 110, 110, 110));
    CHECK(fr.f[2] == fr.f[0]);

    Effect* normal = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Normal", { { "E_FILEPICKERCTRL_Glediator_Filename", file } });
    const xlColor fill(1, 2, 3);
    Frames fn = f.Run(normal, 3, 2, { 0, 2 }, &fill);
    CHECK(Rgb(fn.at(0, 0, 1), 10, 10, 10));
    CHECK(AllColour(fn, 1, 1, 2, 3)); // past the end the CSV reader leaves the buffer alone

    Effect* slow = f.Add("Glediator", "E_CHOICE_Glediator_DurationTreatment=Slow/Accelerate", { { "E_FILEPICKERCTRL_Glediator_Filename", file } }, 0, 200);
    Frames fs = f.Run(slow, 3, 2, { 0, 1, 2, 3 });
    CHECK(fs.f[0] == fs.f[1]);
    CHECK(fs.f[2] == fs.f[3]);
    CHECK(fs.f[0] != fs.f[2]);
}

TEST_CASE("Glediator draws nothing for a missing file", "[effects][media]") {
    TempDir d;
    Fx f(Line("L", 64));
    Effect* e = f.Add("Glediator", "", { { "E_FILEPICKERCTRL_Glediator_Filename", d.File("missing.gled") } });
    const xlColor fill(5, 6, 7);
    Frames fr = f.Run(e, 3, 2, { 0, 1 }, &fill);
    CHECK(AllColour(fr, 0, 5, 6, 7));
    Effect* none = f.Add("Glediator", "");
    Frames fn = f.Run(none, 3, 2, { 0 }, &fill);
    CHECK(AllColour(fn, 0, 5, 6, 7));
}

TEST_CASE("Glediator file reader handles odd sizes and bad frames", "[effects][media]") {
    TempDir d;
    std::string file = d.Write("odd.gled", GledFrames(2, 1, 2) + "xy");
    GlediatorReader r(file, xlSize(2, 1));
    CHECK(r.GetFrameCount() == 2);
    char buf[6];
    r.GetFrame(1, buf, 6);
    CHECK((uint8_t)buf[0] == 10);
    CHECK((uint8_t)buf[5] == 201);
    // Out of range frames and wrong sized buffers are filled with red.
    char bad[6] = {};
    r.GetFrame(5, bad, 6);
    CHECK((uint8_t)bad[0] == 0xFF);
    CHECK(bad[1] == 0);
    CHECK((uint8_t)bad[3] == 0xFF);
    char small[3] = {};
    r.GetFrame(0, small, 3);
    CHECK((uint8_t)small[0] == 0xFF);

    GlediatorReader missing(d.File("none.gled"), xlSize(2, 1));
    CHECK(missing.GetFrameCount() == 0);
    CSVReader csvMissing(d.File("none.csv"));
    CHECK(csvMissing.GetFrameCount() == 0);
    char cb[2] = { 7, 7 };
    csvMissing.GetFrame(0, cb, 2);
    CHECK(cb[0] == 7);
}

TEST_CASE("Glediator settings helpers", "[effects][media]") {
    CHECK(GlediatorEffect::IsGlediatorFile("a.gled"));
    CHECK(GlediatorEffect::IsGlediatorFile("a.CSV"));
    CHECK(GlediatorEffect::IsGlediatorFile("a.out"));
    CHECK_FALSE(GlediatorEffect::IsGlediatorFile("a.png"));

    TempDir d;
    std::string file = d.Write("x.gled", GledFrames(2, 2, 1));
    Fx f(Line("L", 4));
    auto* ge = f.Get("Glediator");
    Effect* e = f.Add("Glediator", "E_TEXTCTRL_Glediator_Filename=old.gled");
    CHECK(ge->needToAdjustSettings("2017.1"));
    ge->adjustSettings("2017.1", e, false);
    CHECK(e->GetSettings().Get("E_FILEPICKERCTRL_Glediator_Filename", "") == "old.gled");
    CHECK_FALSE(e->GetSettings().Contains("E_TEXTCTRL_Glediator_Filename"));
    CHECK(ge->GetFileReferences(&f.ctx, f.model, e->GetSettings()).size() == 1);

    auto has = [](const std::list<std::string>& l, const std::string& s) {
        return std::any_of(l.begin(), l.end(), [&](const std::string& x) { return x.find(s) != std::string::npos; });
    };
    Effect* empty = f.Add("Glediator", "");
    CHECK(has(ge->CheckEffectSettings(empty->GetSettings(), nullptr, f.model, empty, false), "cant find file"));
    CHECK(ge->GetFileReferences(&f.ctx, f.model, empty->GetSettings()).empty());
    Effect* missing = f.Add("Glediator", "", { { "E_FILEPICKERCTRL_Glediator_Filename", d.File("gone.gled") } });
    CHECK(has(ge->CheckEffectSettings(missing->GetSettings(), nullptr, f.model, missing, false), "cant find file"));
    Effect* present = f.Add("Glediator", "", { { "E_FILEPICKERCTRL_Glediator_Filename", file } });
    auto r = ge->CheckEffectSettings(present->GetSettings(), nullptr, f.model, present, false);
    CHECK_FALSE(has(r, "cant find file"));
    CHECK(has(r, "not under show directory"));
    ge->loadFiles(present);
    CHECK_FALSE(present->GetSettings().Get("E_FILEPICKERCTRL_Glediator_Filename", "").empty());
}

// ================================================================== Video

TEST_CASE("Video with a missing or unusable file paints the buffer red", "[effects][media]") {
    TempDir d;
    Fx f(Line("L", 64));
    const xlColor fill(3, 4, 5);
    for (const std::string& treatment : { "Normal", "Loop", "Slow/Accelerate", "Manual", "Manual and Loop" }) {
        INFO(treatment);
        Effect* e = f.Add("Video", "E_CHOICE_Video_DurationTreatment=" + treatment + ",E_SLIDER_Video_CropLeft=40,E_SLIDER_Video_CropRight=40,E_SLIDER_Video_CropTop=0,E_SLIDER_Video_CropBottom=0",
                          { { "E_FILEPICKERCTRL_Video_Filename", d.File("missing.mp4") } });
        Frames fr = f.Run(e, 4, 4, { 0, 1 }, &fill);
        CHECK(AllColour(fr, 0, 255, 0, 0));
        CHECK(AllColour(fr, 1, 255, 0, 0));
    }
    Effect* none = f.Add("Video", "");
    Frames fn = f.Run(none, 4, 4, { 0 }, &fill);
    CHECK(AllColour(fn, 0, 255, 0, 0));
    // A one pixel high buffer is refused before any file is touched.
    Effect* tall = f.Add("Video", "", { { "E_FILEPICKERCTRL_Video_Filename", d.File("missing.mp4") } });
    Frames ft = f.Run(tall, 4, 1, { 0 }, &fill);
    CHECK(AllColour(ft, 0, 255, 0, 0));
    // Sync to audio with no sequence media falls back to the (missing) file.
    Effect* sync = f.Add("Video", "E_CHECKBOX_SynchroniseWithAudio=1", { { "E_FILEPICKERCTRL_Video_Filename", d.File("missing.mp4") } });
    Frames fs = f.Run(sync, 4, 4, { 0 }, &fill);
    CHECK(AllColour(fs, 0, 255, 0, 0));
}

TEST_CASE("Video adjust settings migrates the old loop checkbox", "[effects][media]") {
    Fx f(Line("L", 4));
    auto* ve = f.Get("Video");
    CHECK(ve->needToAdjustSettings("2025.1"));
    CHECK_FALSE(ve->needToAdjustSettings("2099.1"));
    Effect* e = f.Add("Video", "E_CHECKBOX_Video_Loop=1,E_SLIDER_Video_Starttime=5");
    ve->adjustSettings("2025.1", e, false);
    CHECK(e->GetSettings().Get("E_CHOICE_Video_DurationTreatment", "") == "Loop");
    CHECK_FALSE(e->GetSettings().Contains("E_CHECKBOX_Video_Loop"));
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Video_Starttime"));
    Effect* e2 = f.Add("Video", "E_CHECKBOX_Video_Loop=0");
    ve->adjustSettings("2025.1", e2, false);
    CHECK_FALSE(e2->GetSettings().Contains("E_CHOICE_Video_DurationTreatment"));

    CHECK(VideoEffect::IsVideoFile("clip.mp4"));
    CHECK(VideoEffect::IsVideoFile("clip.MOV"));
    CHECK_FALSE(VideoEffect::IsVideoFile("clip.png"));

    SettingsMap sm;
    CHECK(ve->GetFileReferences(&f.ctx, f.model, sm).empty());
    sm["E_FILEPICKERCTRL_Video_Filename"] = "clip.mp4";
    CHECK(ve->GetFileReferences(&f.ctx, f.model, sm).size() == 1);
    // Without the opt-in, video is always rendered serially.
    CHECK(ve->GetFrameParallelism(sm) == RenderableEffect::FrameParallelism::Stateful);
}

TEST_CASE("Video check settings flags missing files and tiny models", "[effects][media]") {
    TempDir d;
    auto has = [](const std::list<std::string>& l, const std::string& s) {
        return std::any_of(l.begin(), l.end(), [&](const std::string& x) { return x.find(s) != std::string::npos; });
    };
    Fx f(Line("L", 4));
    auto* ve = f.Get("Video");
    Effect* e = f.Add("Video", "");
    auto r = ve->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false);
    CHECK(has(r, "does not exist"));
    CHECK(has(r, "not high or wide enough"));

    Fx m("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"64\" StrandsPerString=\"8\" StringType=\"RGB Nodes\"/>");
    Effect* e2 = m.Add("Video", "");
    CHECK_FALSE(has(m.Get("Video")->CheckEffectSettings(e2->GetSettings(), nullptr, m.model, e2, false), "not high or wide enough"));
}

TEST_CASE("Video decode sizes follow the buffer and the crop", "[effects][media]") {
    Fx f("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"60\" StrandsPerString=\"6\" StringType=\"RGB Nodes\"/>");
    int bw = 0, bh = 0;
    f.model->GetBufferSize("Default", "2D", "None", bw, bh, 0);
    REQUIRE(bw >= 2);
    REQUIRE(bh >= 2);
    const std::string file = "/no/such/dir/clip.mp4";
    Effect* e = f.Add("Video", "E_SLIDER_Video_CropLeft=0,E_SLIDER_Video_CropRight=50,E_SLIDER_Video_CropBottom=0,E_SLIDER_Video_CropTop=25",
                      { { "E_FILEPICKERCTRL_Video_Filename", file } });
    std::list<Model*> models{ f.model };
    VideoEffect::PrepareDecodeSizes(f.ctx._sequenceElements, models);
    int w = 0, h = 0;
    REQUIRE(VideoDecodeSizeRegistry::GetMaxDecodeSize(file, w, h));
    CHECK(w == bw * 2);
    CHECK(h == bh * 4);

    // An animated crop uses the tightest span reached during the effect.
    e->GetSettings().erase("E_SLIDER_Video_CropRight");
    e->GetSettings()["E_VALUECURVE_Video_CropRight"] = "Active=TRUE|Id=ID_VALUECURVE_Video_CropRight|Type=Ramp|Min=0.00|Max=100.00|P1=100.00|P2=10.00|RV=TRUE|";
    VideoEffect::PrepareDecodeSizes(f.ctx._sequenceElements, models);
    REQUIRE(VideoDecodeSizeRegistry::GetMaxDecodeSize(file, w, h));
    CHECK(w == bw * 10);

    // Sample spacing forces native decode.
    e->GetSettings()["E_TEXTCTRL_SampleSpacing"] = "2";
    VideoEffect::PrepareDecodeSizes(f.ctx._sequenceElements, models);
    REQUIRE(VideoDecodeSizeRegistry::GetMaxDecodeSize(file, w, h));
    CHECK(w > 100000);

    // Sync to audio and empty filenames are not recorded.
    e->GetSettings()["E_CHECKBOX_SynchroniseWithAudio"] = "1";
    VideoEffect::PrepareDecodeSizes(f.ctx._sequenceElements, models);
    CHECK_FALSE(VideoDecodeSizeRegistry::GetMaxDecodeSize(file, w, h));
    VideoDecodeSizeRegistry::Clear();
}

// ================================================================== DMX

TEST_CASE("DMX effect writes slider values to single colour channels", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"6\""));
    Effect* e = f.Add("DMX", "E_SLIDER_DMX1=10,E_SLIDER_DMX2=200,E_SLIDER_DMX3=0,E_CHECKBOX_INVDMX3=1,E_SLIDER_DMX4=77,E_CHECKBOX_INVDMX4=1");
    Frames fr = f.RunDmx(e, { 0, 10 });
    for (size_t i = 0; i < 2; ++i) {
        CHECK(fr.ch(i, 1) == 10);
        CHECK(fr.ch(i, 2) == 200);
        CHECK(fr.ch(i, 3) == 255);
        CHECK(fr.ch(i, 4) == 178);
        CHECK(fr.ch(i, 5) == 0);
        CHECK(Rgb(fr.f[i][1], 200, 200, 200));
    }
}

TEST_CASE("DMX effect value curves ramp a channel", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"3\""));
    Effect* e = f.Add("DMX", "E_VALUECURVE_DMX2=Active=TRUE|Id=ID_VALUECURVE_DMX2|Type=Ramp|Min=0.00|Max=255.00|P1=0.00|P2=255.00|RV=TRUE|,E_SLIDER_DMX3=40");
    Frames fr = f.RunDmx(e, Range(0, 19));
    int prev = -1;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        CHECK(fr.ch(i, 2) >= prev);
        prev = fr.ch(i, 2);
        CHECK(fr.ch(i, 3) == 40);
    }
    CHECK(fr.ch(0, 2) <= 5);
    CHECK(fr.ch(19, 2) >= 240);
}

TEST_CASE("DMX effect on RGB nodes packs three channels per node in string order", "[effects][media]") {
    Fx rgb(Line("L", 2, "RGB Nodes"));
    Effect* e = rgb.Add("DMX", "E_SLIDER_DMX1=1,E_SLIDER_DMX2=2,E_SLIDER_DMX3=3,E_SLIDER_DMX4=4,E_SLIDER_DMX5=5,E_SLIDER_DMX6=6,E_CHECKBOX_INVDMX6=1");
    Frames fr = rgb.Run(e, 2, 1, { 0 });
    CHECK(Rgb(fr.f[0][0], 1, 2, 3));
    CHECK(Rgb(fr.f[0][1], 4, 5, 249));

    Fx grb(Line("L", 1, "GRB Nodes"));
    Effect* e2 = grb.Add("DMX", "E_SLIDER_DMX1=1,E_SLIDER_DMX2=2,E_SLIDER_DMX3=3,E_CHECKBOX_INVDMX1=1,E_CHECKBOX_INVDMX2=1");
    Frames fg = grb.Run(e2, 1, 1, { 0 });
    CHECK(Rgb(fg.f[0][0], 253, 254, 3));
}

TEST_CASE("DMX effect stops at the model's channel count", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"2\""));
    Effect* e = f.Add("DMX", "E_SLIDER_DMX1=9,E_SLIDER_DMX2=8,E_SLIDER_DMX3=7");
    Frames fr = f.Run(e, 3, 1, { 0 });
    CHECK(fr.ch(0, 1) == 9);
    CHECK(fr.ch(0, 2) == 8);
    CHECK(fr.ch(0, 3) == 0);
}

TEST_CASE("DMX remap moves, scales and inverts channel settings", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"6\""));
    auto* de = dynamic_cast<DMXEffect*>(f.Get("DMX"));
    REQUIRE(de != nullptr);
    Effect* e = f.Add("DMX", "E_SLIDER_DMX1=100,E_CHECKBOX_INVDMX1=0,E_VALUECURVE_DMX3=Active=TRUE|Id=ID_VALUECURVE_DMX3|Type=Flat|Min=0.00|Max=255.00|P1=100.00|RV=TRUE|,E_SLIDER_DMX5=11,E_CHECKBOX_INVDMX5=1");
    std::vector<std::tuple<int, int, float, int, std::string>> maps = {
        { 1, 2, 0.5f, 10, "Check" },
        { 3, 4, 0.5f, 10, "" },
        { 6, 5, 1.0f, 0, "Uncheck" },
    };
    de->RemapSelectedDMXEffectValues(e, maps);
    const SettingsMap& s = e->GetSettings();
    CHECK(s.Get("E_SLIDER_DMX2", "") == "60");
    CHECK(s.Get("E_CHECKBOX_INVDMX2", "") == "1");
    CHECK(s.Contains("E_VALUECURVE_DMX4"));
    CHECK_FALSE(s.Contains("E_SLIDER_DMX4"));
    CHECK_FALSE(s.Contains("E_SLIDER_DMX5"));
    CHECK(s.Get("E_CHECKBOX_INVDMX5", "") == "0");
    CHECK_FALSE(s.Contains("E_VALUECURVE_DMX5"));
    CHECK_FALSE(s.Contains("E_CHECKBOX_INVDMX4"));

    // The remapped values render: channel 2 inverted, channel 4 carries the scaled curve.
    Frames fr = f.RunDmx(e, { 0, 10 });
    CHECK(fr.ch(0, 2) == 255 - 60);
    CHECK(std::abs(fr.ch(0, 4) - 60) <= 1);
    CHECK(fr.ch(1, 4) == fr.ch(0, 4));
}

// ================================================================== Servo

TEST_CASE("Servo writes a 16 bit position scaled into the servo limits", "[effects][media]") {
    Fx f(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\" Bits16=\"1\"", "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"1000\"/>"));
    const std::string chan = f.model->GetNodeName(0);
    Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0", { { "E_CHOICE_Channel", chan } });
    Frames fr = f.RunDmx(e, { 0, 10 });
    for (size_t i = 0; i < 2; ++i) {
        CHECK(fr.ch(i, 1) == 1);   // 500 >> 8
        CHECK(fr.ch(i, 2) == 244); // 500 & 0xFF
    }
}

TEST_CASE("Servo ramps from start to end value", "[effects][media]") {
    Fx f(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"2\" Bits16=\"1\"", "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"1000\"/>"));
    const std::string chan = f.model->GetNodeName(0);
    Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=0.0,E_TEXTCTRL_EndValue=100.0", { { "E_CHOICE_Channel", chan } });
    Frames fr = f.RunDmx(e, Range(0, 19));
    int prev = -1;
    for (size_t i = 0; i < fr.f.size(); ++i) {
        int v = fr.ch(i, 1) * 256 + fr.ch(i, 2);
        CHECK(v >= prev);
        prev = v;
    }
    CHECK(fr.ch(0, 1) * 256 + fr.ch(0, 2) == 0);
    CHECK(prev > 900);
    CHECK(prev <= 1000);

    // A value curve takes over from the start/end interpolation.
    Effect* vc = f.Add("Servo", "E_VALUECURVE_Servo=Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Flat|Min=0.00|Max=1000.00|P1=250.00|RV=TRUE|,E_TEXTCTRL_EndValue=100.0",
                       { { "E_CHOICE_Channel", chan } });
    Frames fv = f.RunDmx(vc, { 0, 19 });
    CHECK(fv.ch(0, 1) * 256 + fv.ch(0, 2) == 250);
    CHECK(fv.ch(1, 1) * 256 + fv.ch(1, 2) == 250);
}

TEST_CASE("Servo 8 bit writes one channel", "[effects][media]") {
    Fx f(Dmx("S", "DmxServo", "NumServos=\"2\" DmxChannelCount=\"2\" Bits16=\"0\"", "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"200\"/><Servo2 Channel=\"2\" MinLimit=\"0\" MaxLimit=\"200\"/>"));
    const std::string chan = f.model->GetNodeName(1);
    Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=25.0,E_TEXTCTRL_EndValue=25.0,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", chan } });
    Frames fr = f.RunDmx(e, { 0 });
    CHECK(fr.ch(0, 1) == 0);
    CHECK(fr.ch(0, 2) == 50);
}

TEST_CASE("Servo on a plain DMX model uses 0 to 100 and an unknown channel writes nothing", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"6\" NodeNames=\"a,b,c,d,e,f\""));
    REQUIRE(f.model->GetNodeName(4) == "e");
    Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=40.0,E_TEXTCTRL_EndValue=40.0,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", "e" } });
    Frames fr = f.RunDmx(e, { 0 });
    CHECK(fr.ch(0, 5) == 40);
    CHECK(fr.lit(0) == 1);
    // fractions truncate
    Effect* frac = f.Add("Servo", "E_TEXTCTRL_Servo=40.6,E_TEXTCTRL_EndValue=40.6,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", "e" } });
    CHECK(f.RunDmx(frac, { 0 }).ch(0, 5) == 40);
    // every whole percentage lands on itself, however the build evaluates position / 100
    for (int pct = 0; pct <= 100; ++pct) {
        INFO("percent " << pct);
        Effect* w = f.Add("Servo", "E_TEXTCTRL_Servo=" + std::to_string(pct) + ".0,E_TEXTCTRL_EndValue=" + std::to_string(pct) + ".0,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", "e" } });
        CHECK(f.RunDmx(w, { 0 }).ch(0, 5) == pct);
    }
    Effect* none = f.Add("Servo", "E_TEXTCTRL_Servo=40.0", { { "E_CHOICE_Channel", "No Such Channel" } });
    Frames fn = f.RunDmx(none, { 0 });
    CHECK(fn.lit(0) == 0);

    // Colour channels are full range 0..255.
    Fx fl(Dmx("F", "DmxFloodlight", "DmxChannelCount=\"4\" DmxRedChannel=\"1\" DmxGreenChannel=\"2\" DmxBlueChannel=\"3\" NodeNames=\"r,g,b,x\""));
    Effect* c = fl.Add("Servo", "E_TEXTCTRL_Servo=60.0,E_TEXTCTRL_EndValue=60.0,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", "g" } });
    Frames fc = fl.RunDmx(c, { 0 });
    CHECK(fc.ch(0, 2) == 153);
    Effect* x = fl.Add("Servo", "E_TEXTCTRL_Servo=60.0,E_TEXTCTRL_EndValue=60.0,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", "x" } });
    Frames fx = fl.RunDmx(x, { 0 });
    CHECK(fx.ch(0, 4) == 60);
}

TEST_CASE("Servo uses skull and advanced moving head limits", "[effects][media]") {
    Fx f(Dmx("S", "DmxSkull", "DmxChannelCount=\"26\" Bits16=\"1\" HasJaw=\"1\" HasPan=\"1\" HasTilt=\"1\" HasNod=\"1\" HasEyeUD=\"1\" HasEyeLR=\"1\" HasColor=\"1\" "
                              "DmxRedChannel=\"24\" DmxGreenChannel=\"25\" DmxBlueChannel=\"26\" DmxEyeBrtChannel=\"23\""));
    auto* sk = dynamic_cast<DmxSkull*>(f.model);
    REQUIRE(sk != nullptr);
    struct Axis {
        int channel;
        int lo;
        int hi;
    };
    const Axis axes[] = {
        { sk->GetJawChannel(), sk->GetJawMinLimit(), sk->GetJawMaxLimit() },
        { sk->GetPanChannel(), sk->GetPanMinLimit(), sk->GetPanMaxLimit() },
        { sk->GetTiltChannel(), sk->GetTiltMinLimit(), sk->GetTiltMaxLimit() },
        { sk->GetNodChannel(), sk->GetNodMinLimit(), sk->GetNodMaxLimit() },
        { sk->GetEyeUDChannel(), sk->GetEyeUDMinLimit(), sk->GetEyeUDMaxLimit() },
        { sk->GetEyeLRChannel(), sk->GetEyeLRMinLimit(), sk->GetEyeLRMaxLimit() },
    };
    for (const auto& a : axes) {
        INFO("channel " << a.channel);
        REQUIRE(a.channel >= 1);
        Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0", { { "E_CHOICE_Channel", f.model->GetNodeName(a.channel - 1) } });
        Frames fr = f.RunDmx(e, { 0 });
        int expect = (int)(uint16_t)(a.lo + (a.hi - a.lo) * 0.5f);
        CHECK(fr.ch(0, a.channel) * 256 + fr.ch(0, a.channel + 1) == expect);
    }

    Fx m(Dmx("MH", "DmxMovingHeadAdv", "DmxChannelCount=\"8\"", "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"4\"/>"));
    auto* adv = dynamic_cast<DmxMovingHeadAdv*>(m.model);
    REQUIRE(adv != nullptr);
    Effect* e = m.Add("Servo", "E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", m.model->GetNodeName(2) } });
    Frames fr = m.RunDmx(e, { 0 });
    DmxMotor* tilt = adv->GetTiltMotor();
    int expect = (int)(uint16_t)(tilt->GetMinValue() + (tilt->GetMaxValue() - tilt->GetMinValue()) * 0.5f);
    // Motor axes are always written as 16 bit to their coarse and fine channels.
    CHECK(fr.ch(0, 3) == (expect >> 8));
    CHECK(fr.ch(0, 4) == (expect & 0xFF));
}

TEST_CASE("Servo follows phonemes on a lyric timing track", "[effects][media]") {
    Fx f(Dmx("S", "DmxServo", "NumServos=\"1\" DmxChannelCount=\"1\" Bits16=\"0\"", "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"100\"/>"));
    Element* t = f.ctx._sequenceElements.AddElement("Lyrics", "timing", true, false, true, false, false);
    while (t->GetEffectLayerCount() < 3) t->AddEffectLayer();
    EffectLayer* ph = t->GetEffectLayer(2);
    ph->AddEffect(0, "AI", "", "", 0, 300, 0, false);
    ph->AddEffect(0, "O-shimmer", "", "", 300, 600, 0, false);
    ph->AddEffect(0, "nonsense", "", "", 600, 800, 0, false);
    Effect* e = f.Add("Servo", "E_CHECKBOX_Timing_Track=1,E_CHOICE_Servo_TimingTrack=Lyrics,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", f.model->GetNodeName(0) } });
    Frames fr = f.RunDmx(e, { 0, 7, 13, 17 });
    CHECK(fr.ch(0, 1) == 90);
    CHECK(fr.ch(1, 1) == 50);
    CHECK(fr.ch(2, 1) == 0);
    CHECK(fr.ch(3, 1) == 0);

    Effect* wrong = f.Add("Servo", "E_CHECKBOX_Timing_Track=1,E_CHOICE_Servo_TimingTrack=Nope,E_CHECKBOX_16bit=0", { { "E_CHOICE_Channel", f.model->GetNodeName(0) } });
    Frames fw = f.RunDmx(wrong, { 0 });
    CHECK(fw.ch(0, 1) == 0);

    auto* se = f.Get("Servo");
    auto has = [](const std::list<std::string>& l, const std::string& s) {
        return std::any_of(l.begin(), l.end(), [&](const std::string& x) { return x.find(s) != std::string::npos; });
    };
    CHECK_FALSE(has(se->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false), "ERR"));
    CHECK(has(se->CheckEffectSettings(wrong->GetSettings(), nullptr, f.model, wrong, false), "unknown timing"));
    Effect* noTrack = f.Add("Servo", "E_CHECKBOX_Timing_Track=1");
    CHECK(has(se->CheckEffectSettings(noTrack->GetSettings(), nullptr, f.model, noTrack, false), "no timing selected"));

    se->RenameTimingTrack("Lyrics", "Words", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "") == "Words");
    se->RenameTimingTrack("Other", "X", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "") == "Words");
}

TEST_CASE("Servo adjust settings converts old value curves", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"2\""));
    auto* se = f.Get("Servo");
    CHECK(se->needToAdjustSettings("2024.1"));
    Effect* flat = f.Add("Servo", "E_VALUECURVE_Servo=Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Flat|Min=0.00|Max=1000.00|P1=250.00|RV=TRUE|");
    se->adjustSettings("2024.1", flat, false);
    CHECK_FALSE(flat->GetSettings().Contains("E_VALUECURVE_Servo"));
    CHECK(flat->GetSettings().Get("E_TEXTCTRL_Servo", "") == "25.0");
    CHECK(flat->GetSettings().Get("E_TEXTCTRL_EndValue", "") == "25.0");

    Effect* ramp = f.Add("Servo", "E_VALUECURVE_Servo=Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Ramp|Min=0.00|Max=1000.00|P1=100.00|P2=900.00|RV=TRUE|");
    se->adjustSettings("2024.1", ramp, false);
    CHECK(ramp->GetSettings().Get("E_TEXTCTRL_Servo", "") == "10.0");
    CHECK(ramp->GetSettings().Get("E_TEXTCTRL_EndValue", "") == "90.0");

    Effect* sine = f.Add("Servo", "E_VALUECURVE_Servo=Active=TRUE|Id=ID_VALUECURVE_Servo|Type=Sine|Min=0.00|Max=1000.00|P1=100.00|P2=900.00|RV=TRUE|");
    se->adjustSettings("2024.1", sine, false);
    CHECK(sine->GetSettings().Contains("E_VALUECURVE_Servo"));

    Effect* startOnly = f.Add("Servo", "E_TEXTCTRL_Servo=33.0");
    se->adjustSettings("2024.1", startOnly, false);
    CHECK(startOnly->GetSettings().Get("E_TEXTCTRL_EndValue", "") == "33.0");
    Effect* neither = f.Add("Servo", "");
    se->adjustSettings("2024.1", neither, false);
    CHECK(neither->GetSettings().Get("E_TEXTCTRL_EndValue", "") == "0");
}

TEST_CASE("Servo split hands the midpoint to both halves", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"2\""));
    auto* se = f.Get("Servo");
    Effect* a = f.Add("Servo", "E_TEXTCTRL_Servo=0.0,E_TEXTCTRL_EndValue=100.0", {}, 0, 250);
    Effect* b = f.Add("Servo", "E_TEXTCTRL_Servo=0.0,E_TEXTCTRL_EndValue=100.0", {}, 250, 1000);
    se->AdjustSettingsAfterSplit(a, b);
    CHECK(std::strtod(a->GetSettings().Get("E_TEXTCTRL_EndValue", "").c_str(), nullptr) == Catch::Approx(25.0));
    CHECK(std::strtod(b->GetSettings().Get("E_TEXTCTRL_Servo", "").c_str(), nullptr) == Catch::Approx(25.0));
    CHECK(a->GetSettings().Get("E_TOGGLEBUTTON_End", "") == "1");
    CHECK(b->GetSettings().Get("E_TOGGLEBUTTON_Start", "") == "1");
}

TEST_CASE("Servo draws its position into the background display list", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"2\""));
    Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0", { { "E_CHOICE_Channel", f.model->GetNodeName(0) } });
    const bool was = Effect::IsBackgroundDisplayListEnabled();
    Effect::EnableBackgroundDisplayLists(true);
    f.RunDmx(e, Range(0, 19));
    Effect::EnableBackgroundDisplayLists(was);
    CHECK(e->GetBackgroundDisplayList().size() == 20 * 6);
}

// ================================================================== Moving Head

namespace {
// Home is mid travel so both positive and negative angles map inside the motor range.
const char* kMhChildren = "<PanMotor ChannelCoarse=\"1\" ChannelFine=\"2\" RangeOfMotion=\"540\" OrientHome=\"270\"/><TiltMotor ChannelCoarse=\"3\" ChannelFine=\"4\" RangeOfMotion=\"270\" OrientHome=\"135\"/>";

std::string MovingHead(const std::string& fixture = "MH1") {
    return Dmx("MH", "DmxMovingHead",
               "DmxChannelCount=\"12\" DmxFixture=\"" + fixture + "\" DmxColorType=\"0\" DmxRedChannel=\"5\" DmxGreenChannel=\"6\" DmxBlueChannel=\"7\" MhDimmerChannel=\"8\" DmxShutterChannel=\"9\" DmxShutterOnValue=\"200\"",
               kMhChildren);
}

int Cmd16(const Frames& fr, size_t i, int coarse) { return fr.ch(i, coarse) * 256 + fr.ch(i, coarse + 1); }
} // namespace

TEST_CASE("Moving head pan and tilt become motor commands", "[effects][media]") {
    Fx f(MovingHead());
    auto* mh = dynamic_cast<DmxMovingHead*>(f.model);
    REQUIRE(mh != nullptr);
    Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Pan: 45.0;Tilt: -20.0;Heads: 1" } });
    Frames fr = f.RunDmx(e, { 0, 10 });
    const int pan = mh->GetPanMotor()->ConvertPostoCmd(-45.0f);
    const int tilt = mh->GetTiltMotor()->ConvertPostoCmd(20.0f);
    for (size_t i = 0; i < 2; ++i) {
        CHECK(Cmd16(fr, i, 1) == pan);
        CHECK(Cmd16(fr, i, 3) == tilt);
        CHECK(fr.ch(i, 5) == 0); // no colour command
    }
    Effect* neg = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Pan: -45.0;Tilt: 0.0;Heads: 1" } });
    Frames fn = f.RunDmx(neg, { 0 });
    CHECK(Cmd16(fn, 0, 1) == mh->GetPanMotor()->ConvertPostoCmd(45.0f));
    CHECK(Cmd16(fn, 0, 1) != Cmd16(fr, 0, 1));
    // Opposite pans land symmetrically either side of home.
    const int home = mh->GetPanMotor()->ConvertPostoCmd(0.0f);
    CHECK(home > 30000);
    CHECK(home < 35000);
    CHECK(std::abs((Cmd16(fn, 0, 1) - home) + (Cmd16(fr, 0, 1) - home)) <= 2);
    CHECK(Cmd16(fn, 0, 3) == mh->GetTiltMotor()->ConvertPostoCmd(0.0f));

    // Settings for another fixture slot are ignored by this head.
    Effect* other = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH2_Settings", "Pan: 45.0;Tilt: 10.0;Heads: 2" } });
    Frames fo = f.RunDmx(other, { 0 });
    CHECK(fo.lit(0) == 0);
}

TEST_CASE("Moving head colour, dimmer and shutter channels", "[effects][media]") {
    Fx f(MovingHead());
    Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Color: 0.0,1.0,1.0;Dimmer: 0.0,0.0,1.0,1.0;Shutter: On;Heads: 1" } });
    Frames fr = f.RunDmx(e, Range(0, 19));
    for (size_t i = 0; i < fr.f.size(); ++i) {
        CHECK(fr.ch(i, 5) == 255);
        CHECK(fr.ch(i, 6) == 0);
        CHECK(fr.ch(i, 7) == 0);
        CHECK(fr.ch(i, 9) == 200);
        if (i > 0) CHECK(fr.ch(i, 8) >= fr.ch(i - 1, 8));
    }
    CHECK(fr.ch(0, 8) <= 5);
    CHECK(fr.ch(19, 8) >= 240);
    // No pan or tilt command means the motor channels are left alone.
    CHECK(Cmd16(fr, 0, 1) == 0);

    // Two colours blend from the first to the second across the effect.
    Effect* blend = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Color: 0.0,1.0,1.0,0.6666667,1.0,1.0;Heads: 1" } });
    Frames fb = f.RunDmx(blend, { 0, 10, 19 });
    CHECK(fb.ch(0, 5) == 255);
    CHECK(fb.ch(0, 7) == 0);
    CHECK(fb.ch(1, 5) > 50);
    CHECK(fb.ch(1, 7) > 50);
    CHECK(fb.ch(2, 5) < 30);
    CHECK(fb.ch(2, 7) > 225);
}

TEST_CASE("Moving head colour wheel picks wheel slots and pulses the shutter", "[effects][media]") {
    Fx f(Dmx("MH", "DmxMovingHeadAdv",
             "DmxChannelCount=\"20\" DmxFixture=\"MH1\" DmxColorType=\"1\" DmxColorWheelChannel=\"9\" DmxDimmerChannel=\"10\" DmxColorWheelDelay=\"100\" "
             "DmxColorWheelDMX0=\"5\" DmxColorWheelColor0=\"#FF0000\" DmxColorWheelDMX1=\"15\" DmxColorWheelColor1=\"#00FF00\" DmxColorWheelDMX2=\"25\" DmxColorWheelColor2=\"#0000FF\" "
             "DmxShutterChannel=\"11\" DmxShutterOnValue=\"255\"",
             kMhChildren));
    REQUIRE(f.model->GetNodeCount() == 20);
    Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Wheel: 0.3333333,1.0,1.0,0.6666667,1.0,1.0;AutoShutter: On;Heads: 1" } });
    Frames fr = f.RunDmx(e, Range(0, 19));
    // First half green, second half blue.
    CHECK(fr.ch(2, 9) == 15);
    CHECK(fr.ch(8, 9) == 15);
    CHECK(fr.ch(12, 9) == 25);
    CHECK(fr.ch(19, 9) == 25);
    CHECK(fr.ch(5, 10) == 255);
    // Shutter starts closed, opens, closes while the wheel turns, and ends open.
    CHECK(fr.ch(0, 11) == 0);
    CHECK(fr.ch(5, 11) == 255);
    CHECK(fr.ch(10, 11) < 128);
    CHECK(fr.ch(19, 11) == 255);

    // Without Auto Shutter the shutter channel is left alone.
    Effect* plain = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Wheel: 0.0,1.0,1.0;Heads: 1" } });
    Frames fp = f.RunDmx(plain, { 0, 10 });
    CHECK(fp.ch(0, 9) == 5);
    CHECK(fp.ch(1, 9) == 5);
    CHECK(fp.ch(0, 11) == 0);
}

TEST_CASE("Moving head pattern moves the head and repeats per cycle", "[effects][media]") {
    Fx f(MovingHead());
    Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Pattern: Circle;PatternWidth: 30;PatternHeight: 20;Cycles: 2;Heads: 1" } });
    Frames fr = f.RunDmx(e, Range(0, 19));
    std::set<int> pans;
    for (size_t i = 0; i < fr.f.size(); ++i) pans.insert(Cmd16(fr, i, 1));
    CHECK(pans.size() > 5);
    CHECK(fr.f[0] == fr.f[10]); // two cycles over 20 frames
    CHECK(fr.f[3] == fr.f[13]);

    // Ignoring tilt keeps the tilt motor still while the pattern pans.
    Effect* ig = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Pattern: Circle;Tilt: 5.0;IgnoreTilt: 1;Heads: 1" } });
    Frames fi = f.RunDmx(ig, { 0, 5, 10, 15 });
    CHECK(Cmd16(fi, 0, 3) == Cmd16(fi, 1, 3));
    CHECK(Cmd16(fi, 0, 3) == Cmd16(fi, 3, 3));
    CHECK(Cmd16(fi, 0, 1) != Cmd16(fi, 1, 1));
}

TEST_CASE("Moving head accepts every command type", "[effects][media]") {
    Fx f(MovingHead());
    const std::string ramp = "Active=TRUE|Id=ID_VALUECURVE_MHPan|Type=Ramp|Min=-1800.00|Max=1800.00|P1=-450.00|P2=450.00|RV=TRUE|";
    const std::string flat = "Active=TRUE|Id=ID_X|Type=Flat|Min=0.00|Max=100.00|P1=10.00|RV=TRUE|";
    const std::vector<std::string> cmds = {
        "Pan VC: " + ramp + ";Tilt VC: " + ramp + ";Heads: 1",
        "Pan: 10;PanOffset VC: " + ramp + ";TiltOffset VC: " + ramp + ";Heads: 1,2,3",
        "Pan: 10;Tilt: 5;PanOffset: 10;TiltOffset: 5;Groupings: 2;Heads: 1,2,3,4",
        "Pan: 10;Groupings VC: Active=TRUE|Id=ID_G|Type=Flat|Min=1.00|Max=6.00|P1=2.00|RV=TRUE|;Heads: 1,2",
        "Path: 0.5,0.5@L1.0,0.5;TimeOffset: 10;PathScale: 1;Heads: 1",
        "Path: 0.5,0.5@L1.0,0.5;TimeOffset VC: " + flat + ";PathScale VC: " + flat + ";IgnorePan: 1;Heads: 1",
        "Pattern: Lissajous;PatternXFreq: 3;PatternYFreq: 2;PatternXPhase: 45;PatternYPhase: 10;PatternRotation: 30;PatternXOffset: 5;PatternYOffset: 5;PatternStartOffset: 90;PatternPhaseOffset: 45;Heads: 1",
        "Pattern: Square;PatternRotation VC: Active=TRUE|Id=ID_R|Type=Ramp|Min=0.00|Max=360.00|P1=0.00|P2=360.00|RV=TRUE|;Heads: 1",
    };
    for (const auto& c : cmds) {
        INFO(c);
        Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", c } });
        Frames fr = f.RunDmx(e, { 0, 7, 19 });
        CHECK(fr.lit(0) + fr.lit(1) + fr.lit(2) > 0);
        Frames again = f.RunDmx(e, { 0, 7, 19 });
        CHECK(fr.f == again.f);
    }
    // A pan value curve sweeps across the effect.
    Effect* sweep = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", cmds[0] } });
    Frames fs = f.RunDmx(sweep, { 0, 19 });
    CHECK(Cmd16(fs, 0, 1) != Cmd16(fs, 1, 1));
}

TEST_CASE("Moving head effect only renders on moving head models", "[effects][media]") {
    Fx f(Dmx("G", "DmxGeneral", "DmxChannelCount=\"8\""));
    Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Pan: 45.0;Tilt: 10.0;Heads: 1" } });
    Frames fr = f.RunDmx(e, { 0 });
    CHECK(fr.lit(0) == 0);
}

TEST_CASE("Moving head head position spreads heads around the centre", "[effects][media]") {
    float pan = 0, tilt = 0;
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pan: 10;Tilt: 20;PanOffset: 6;TiltOffset: 2;Heads: 1,2,3", 1, 0, 1000, pan, tilt));
    // Three heads centre on slot 2: slot 1 is one step left.
    CHECK(pan == Catch::Approx(4.0f));
    CHECK(tilt == Catch::Approx(18.0f));
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pan: 10;Tilt: 20;PanOffset: 6;TiltOffset: 2;Heads: 1,2,3", 3, 0, 1000, pan, tilt));
    CHECK(pan == Catch::Approx(16.0f));
    CHECK(tilt == Catch::Approx(22.0f));
    // Groupings of two: heads 1 and 3 are both slot 1.
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pan: 0;PanOffset: 10;Groupings: 2;Heads: 1,2,3,4", 3, 0, 1000, pan, tilt));
    CHECK(pan == Catch::Approx(-5.0f));
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pan: 0;PanOffset: 10;Groupings: 2;Heads: 1,2,3,4", 4, 0, 1000, pan, tilt));
    CHECK(pan == Catch::Approx(5.0f));

    CHECK_FALSE(MovingHeadEffect::GetHeadStartPosition("Color: 0,1,1;Heads: 1", 1, 0, 1000, pan, tilt));
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pattern: Circle;PatternWidth: 30;PatternHeight: 20;Heads: 1", 1, 0, 1000, pan, tilt));
    // Circle starts at the top: x = cos(pi/2) = 0, y = cos(0) = 1.
    CHECK(pan == Catch::Approx(0.0f).margin(0.01));
    CHECK(tilt == Catch::Approx(20.0f));
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Path: 0.5,0.5;L1.0,0.5;Heads: 1", 1, 0, 1000, pan, tilt) == false);
    // The '@' separator stands in for ';' inside a path.
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pan: 0;Path: 1.0,0.5@L0.5,0.5;Heads: 1", 1, 0, 1000, pan, tilt));
    CHECK(pan == Catch::Approx(90.0f).margin(0.01));
    CHECK(tilt == Catch::Approx(45.0f).margin(0.01));
    const std::string ramp = "Active=TRUE|Id=ID_X|Type=Ramp|Min=-1800.00|Max=1800.00|P1=-300.00|P2=300.00|RV=TRUE|";
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pan VC: " + ramp + ";Tilt VC: " + ramp + ";PanOffset VC: " + ramp + ";TiltOffset VC: " + ramp +
                                                       ";Groupings VC: Active=TRUE|Id=ID_G|Type=Flat|Min=1.00|Max=6.00|P1=1.00|RV=TRUE|;TimeOffset VC: " + ramp +
                                                       ";PathScale VC: " + ramp + ";PatternRotation VC: " + ramp + ";Heads: 1",
                                                   1, 0, 1000, pan, tilt));
    CHECK(pan == Catch::Approx(-30.0f));
    CHECK(tilt == Catch::Approx(-30.0f));
    REQUIRE(MovingHeadEffect::GetHeadStartPosition("Pattern: Eight;PatternWidth: 10;PatternHeight: 10;PatternXOffset: 1;PatternYOffset: 2;PatternRotation: 90;"
                                                   "PatternStartOffset: 0;PatternPhaseOffset: 0;PatternXFreq: 1;PatternYFreq: 1;PatternXPhase: 0;PatternYPhase: 0;"
                                                   "IgnorePan: 1;IgnoreTilt: 1;TimeOffset: 0;PathScale: 0;Pan: 3;Tilt: 4;Heads: 1",
                                                   1, 0, 1000, pan, tilt));
    CHECK(pan == Catch::Approx(3.0f));
    CHECK(tilt == Catch::Approx(4.0f));
}

TEST_CASE("Moving head pattern shapes", "[effects][media]") {
    const float pi = 3.14159265f;
    float x = 0, y = 0;
    MovingHeadEffect::CalculatePatternPoint("Square", 0.0f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(-1.0f));
    CHECK(y == Catch::Approx(1.0f));
    MovingHeadEffect::CalculatePatternPoint("Square", pi * 0.75f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(1.0f));
    CHECK(y == Catch::Approx(0.0f).margin(1e-4));
    MovingHeadEffect::CalculatePatternPoint("Square", pi * 1.25f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(0.0f).margin(1e-4));
    CHECK(y == Catch::Approx(-1.0f));
    MovingHeadEffect::CalculatePatternPoint("Square", pi * 1.75f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(-1.0f));
    CHECK(y == Catch::Approx(0.0f).margin(1e-4));
    const float quads[4] = { 0.25f, 0.75f, 1.25f, 1.75f };
    const float tx[4] = { 1, 1, -1, -1 };
    const float ty[4] = { 1, -1, -1, 1 };
    for (int i = 0; i < 4; ++i) {
        MovingHeadEffect::CalculatePatternPoint("SquareTrue", pi * quads[i], 0, 0, 0, 0, x, y);
        CHECK(x == tx[i]);
        CHECK(y == ty[i]);
    }
    MovingHeadEffect::CalculatePatternPoint("SquareChoppy", 0.1f, 0, 0, 0, 0, x, y);
    CHECK(x == 1.0f);
    CHECK(y == 0.0f);
    MovingHeadEffect::CalculatePatternPoint("Line2", pi, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(0.0f).margin(1e-5));
    CHECK(y == x);
    MovingHeadEffect::CalculatePatternPoint("Line", 0.5f, 0, 0, 0, 0, x, y);
    CHECK(x == y);
    MovingHeadEffect::CalculatePatternPoint("Diamond", 0.0f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(0.0f).margin(1e-5));
    CHECK(y == Catch::Approx(1.0f));
    MovingHeadEffect::CalculatePatternPoint("Leaf", 0.0f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(0.0f).margin(1e-5));
    CHECK(y == Catch::Approx(1.0f));
    MovingHeadEffect::CalculatePatternPoint("Eight", 0.0f, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(0.0f).margin(1e-5));
    CHECK(y == Catch::Approx(1.0f));
    MovingHeadEffect::CalculatePatternPoint("Lissajous", 0.0f, 2, 3, pi / 2, 0, x, y);
    CHECK(x == Catch::Approx(0.0f).margin(1e-5));
    CHECK(y == Catch::Approx(1.0f));
    MovingHeadEffect::CalculatePatternPoint("Circle", pi / 2, 0, 0, 0, 0, x, y);
    CHECK(x == Catch::Approx(-1.0f));
    CHECK(y == Catch::Approx(0.0f).margin(1e-5));
}

TEST_CASE("Moving head path positions aim at the sketch", "[effects][media]") {
    float pan = 0, tilt = 0;
    MovingHeadEffect::CalculatePathPositions(true, true, pan, tilt, 0, 0, 0, 1.0, "0.5,0.5;L1.0,0.5");
    CHECK(pan == Catch::Approx(90.0f).margin(0.01));
    CHECK(tilt == Catch::Approx(45.0f).margin(0.01));
    pan = 0;
    tilt = 0;
    MovingHeadEffect::CalculatePathPositions(true, true, pan, tilt, 0, 0, 0, 1.0, "0.5,0.5;L0.0,0.5");
    CHECK(pan == Catch::Approx(-90.0f).margin(0.01));
    CHECK(tilt == Catch::Approx(45.0f).margin(0.01));
    pan = 0;
    tilt = 0;
    MovingHeadEffect::CalculatePathPositions(true, true, pan, tilt, 0, 0, 0, 1.0, "0.5,0.5;L0.5,0.0");
    CHECK(pan == Catch::Approx(0.0f).margin(0.01));
    CHECK(tilt == Catch::Approx(45.0f).margin(0.01));
    // Pointing behind the fixture folds the pan back to the front and flips the tilt.
    pan = 0;
    tilt = 0;
    MovingHeadEffect::CalculatePathPositions(true, true, pan, tilt, 0, 0, 0, 1.0, "0.5,0.5;L0.75,1.0");
    CHECK(pan < 90.0f);
    CHECK(pan > -90.0f);
    CHECK(tilt < 0.0f);
    // A negative scale shrinks the path; ignoring both axes leaves the inputs alone.
    float p2 = 0, t2 = 0;
    MovingHeadEffect::CalculatePathPositions(true, true, p2, t2, 0, -2, 0, 1.0, "0.5,0.5;L1.0,0.5");
    CHECK(t2 < 45.0f);
    CHECK(t2 > 0.0f);
    float p3 = 7, t3 = 8;
    MovingHeadEffect::CalculatePathPositions(false, false, p3, t3, 50, 0, 3, 0.5, "0.5,0.5;L1.0,0.5");
    CHECK(p3 == 7.0f);
    CHECK(t3 == 8.0f);
    float p4 = 7, t4 = 8;
    MovingHeadEffect::CalculatePathPositions(true, true, p4, t4, 0, 0, 0, 0.5, "");
    CHECK(p4 == 7.0f);
    CHECK(t4 == 8.0f);
}

TEST_CASE("Moving head remap moves settings to the model's own fixture slot", "[effects][media]") {
    Fx f(MovingHead("MH3"));
    SettingsMap s;
    s["E_TEXTCTRL_MH1_Settings"] = "Pan: 10.0;Heads: 1;Tilt: 5.0";
    s["E_TEXTCTRL_MH6_Settings"] = "Pan: 99.0;Heads: 6";
    MovingHeadEffect::RemapSingleFixtureSettings(s, f.model);
    CHECK(s.Get("E_TEXTCTRL_MH3_Settings", "") == "Pan: 10.0;Heads: 3;Tilt: 5.0");
    CHECK(s.Get("E_TEXTCTRL_MH1_Settings", "x") == "");
    CHECK(s.Get("E_TEXTCTRL_MH6_Settings", "x") == "");

    SettingsMap ok;
    ok["E_TEXTCTRL_MH3_Settings"] = "Pan: 1.0;Heads: 3";
    ok["E_TEXTCTRL_MH5_Settings"] = "Pan: 2.0;Heads: 5";
    MovingHeadEffect::RemapSingleFixtureSettings(ok, f.model);
    CHECK(ok.Get("E_TEXTCTRL_MH3_Settings", "") == "Pan: 1.0;Heads: 3");
    CHECK(ok.Get("E_TEXTCTRL_MH5_Settings", "x") == "");
    CHECK_FALSE(ok.Contains("E_TEXTCTRL_MH2_Settings"));

    SettingsMap untouched;
    untouched["E_TEXTCTRL_MH1_Settings"] = "Pan: 1.0;Heads: 1";
    MovingHeadEffect::RemapSingleFixtureSettings(untouched, nullptr);
    Fx g(Dmx("G", "DmxGeneral", "DmxChannelCount=\"4\""));
    MovingHeadEffect::RemapSingleFixtureSettings(untouched, g.model);
    CHECK(untouched.Get("E_TEXTCTRL_MH1_Settings", "") == "Pan: 1.0;Heads: 1");

    // Loading an old file runs the remap through adjustSettings.
    auto* me = f.Get("Moving Head");
    CHECK(me->needToAdjustSettings("2026.1"));
    Effect* e = f.Add("Moving Head", "", { { "E_TEXTCTRL_MH1_Settings", "Pan: 10.0;Heads: 1" } });
    me->adjustSettings("2026.1", e, false);
    CHECK(e->GetSettings().Get("E_TEXTCTRL_MH3_Settings", "") == "Pan: 10.0;Heads: 3");
    Frames fr = f.RunDmx(e, { 0 });
    auto* mh = dynamic_cast<DmxMovingHead*>(f.model);
    CHECK(Cmd16(fr, 0, 1) == mh->GetPanMotor()->ConvertPostoCmd(-10.0f));

    CHECK(me->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false).empty());
    e->GetSettings()["E_CHOICE_Servo_TimingTrack"] = "T1";
    me->RenameTimingTrack("T1", "T2", e);
    CHECK(e->GetSettings().Get("E_CHOICE_Servo_TimingTrack", "") == "T2");
    auto* mhe = dynamic_cast<MovingHeadEffect*>(me);
    REQUIRE(mhe != nullptr);
    CHECK(mhe->GetSettingVCMin("E_VALUECURVE_MHPan") == MOVING_HEAD_MIN);
    CHECK(mhe->GetSettingVCMin("E_VALUECURVE_MHGroupings") == MOVING_HEAD_GROUP_MIN);
    CHECK(mhe->GetSettingVCMax("E_VALUECURVE_MHGroupings") == MOVING_HEAD_GROUP_MAX);
    CHECK(mhe->GetSettingVCMax("E_VALUECURVE_MHPathScale") == MOVING_HEAD_SCALE_MAX);
    CHECK(mhe->GetSettingVCDivisor("E_VALUECURVE_MHTimeOffset") == MOVING_HEAD_DIVISOR);
}

TEST_CASE("Moving head effect renders through the engine on each head", "[effects][media]") {
    TempDir dir;
    FixFileReset reset;
    EnsureLoggers();
    auto head = [](const std::string& name, int start, const std::string& fixture) {
        return "<model name=\"" + name + "\" DisplayAs=\"DmxMovingHead\" StartChannel=\"" + std::to_string(start) +
               "\" StringType=\"Single Color White\" DmxChannelCount=\"8\" DmxFixture=\"" + fixture + "\" DmxRedChannel=\"5\" DmxGreenChannel=\"6\" DmxBlueChannel=\"7\">" +
               kMhChildren + "</model>";
    };
    dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
    dir.Write("xlights_rgbeffects.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n" + head("A", 1, "MH1") + head("B", 9, "MH2") +
                                            "\n</models>\n<modelGroups>\n<modelGroup name=\"Heads\" models=\"A,B\" layout=\"minimalGrid\" GridSize=\"400\"/>\n</modelGroups>\n</xrgb>\n");
    // Commas inside a setting value are stored escaped in the EffectDB.
    const std::string mh = "Pan: 20.0;Tilt: 0.0;Color: 0.0&comma;1.0&comma;1.0;Heads: 1&comma;2";
    const std::string settings = "B_CHOICE_BufferStyle=Per Model Default,E_TEXTCTRL_MH1_Settings=" + mh + ",E_TEXTCTRL_MH2_Settings=" +
                           "Pan: -20.0;Tilt: 0.0;Color: 0.6666667&comma;1.0&comma;1.0;Heads: 2";
    std::string xsq = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
                      "<head><version>2026.20</version><mediaFile></mediaFile><sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>0.500</sequenceDuration></head>"
                      "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Heads\" visible=\"1\"/></DisplayElements>"
                      "<EffectDB><Effect>" + settings + "</Effect></EffectDB><ColorPalettes><ColorPalette>C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1</ColorPalette></ColorPalettes>"
                      "<ElementEffects><Element type=\"model\" name=\"Heads\"><EffectLayer><Effect ref=\"0\" name=\"Moving Head\" startTime=\"0\" endTime=\"500\" palette=\"0\"/></EffectLayer></Element></ElementEffects></xsequence>";
    std::string path = dir.Write("seq.xsq", xsq);
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    REQUIRE(ctx.OpenSequence(path));
    REQUIRE(ctx.RenderAndWait(60000));
    auto* a = dynamic_cast<DmxMovingHead*>(ctx.AllModels["A"]);
    auto* b = dynamic_cast<DmxMovingHead*>(ctx.AllModels["B"]);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    auto& sd = ctx._seqData;
    REQUIRE(sd.NumFrames() >= 5);
    REQUIRE(sd.NumChannels() >= 16);
    const int panB = b->GetPanMotor()->ConvertPostoCmd(20.0f);
    for (unsigned int fidx = 0; fidx < 5; ++fidx) {
        INFO("frame " << fidx);
        // Each head renders its own fixture slot: A red at pan 20, B blue at pan -20.
        CHECK(sd[fidx][0] * 256 + sd[fidx][1] == a->GetPanMotor()->ConvertPostoCmd(-20.0f));
        CHECK(sd[fidx][4] == 255);
        CHECK(sd[fidx][6] == 0);
        CHECK(sd[fidx][8] * 256 + sd[fidx][9] == panB);
        CHECK(sd[fidx][12] == 0);
        CHECK(sd[fidx][14] == 255);
    }
}

TEST_CASE("Media effects move files from outside the show folder", "[effects][media]") {
    TempDir show;
    TempDir outside;
    FixFileReset reset;
    EnsureLoggers();
    show.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
    show.Write("xlights_rgbeffects.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models/><modelGroups/></xrgb>\n");
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(show.path.string()));
    const std::string out = outside.Write("f.bin", "x");
    const std::string in = show.Write("g.bin", "x");
    struct Case {
        const char* effect;
        const char* key;
    };
    for (const Case& c : { Case{ "Pictures", "E_TEXTCTRL_Pictures_Filename" }, Case{ "Glediator", "E_FILEPICKERCTRL_Glediator_Filename" }, Case{ "Video", "E_FILEPICKERCTRL_Video_Filename" } }) {
        INFO(c.effect);
        RenderableEffect* fx = ctx.effectManager.GetEffect(c.effect);
        REQUIRE(fx != nullptr);
        SettingsMap a;
        a[c.key] = out;
        CHECK(fx->CleanupFileLocations(&ctx, a));
        CHECK_FALSE(a[c.key].empty());
        SettingsMap b;
        b[c.key] = in;
        CHECK_FALSE(fx->CleanupFileLocations(&ctx, b));
        CHECK(b[c.key] == in);
        SettingsMap m;
        m[c.key] = outside.File("missing.bin");
        CHECK_FALSE(fx->CleanupFileLocations(&ctx, m));
    }
}

TEST_CASE("Servo uses Servo3d axis limits", "[effects][media]") {
    Fx f(Dmx("S", "DmxServo3d", "NumServos=\"2\" NumStatic=\"1\" NumMotion=\"2\" DmxChannelCount=\"4\" Bits16=\"1\"",
             "<Servo1 Channel=\"1\" MinLimit=\"100\" MaxLimit=\"300\"/><Servo2 Channel=\"3\" MinLimit=\"0\" MaxLimit=\"1000\"/>"));
    auto* sv = dynamic_cast<DmxServo3d*>(f.model);
    REQUIRE(sv != nullptr);
    Effect* e = f.Add("Servo", "E_TEXTCTRL_Servo=50.0,E_TEXTCTRL_EndValue=50.0", { { "E_CHOICE_Channel", f.model->GetNodeName(2) } });
    Frames fr = f.RunDmx(e, { 0 });
    const int lo = sv->GetAxis(1)->GetMinLimit();
    const int hi = sv->GetAxis(1)->GetMaxLimit();
    CHECK(Cmd16(fr, 0, 3) == (int)(uint16_t)(lo + (hi - lo) * 0.5f));
    CHECK(Cmd16(fr, 0, 1) == 0);
}

TEST_CASE("Video with an undecodable file reports it and paints red", "[effects][media]") {
    TempDir d;
    const std::string bad = d.Write("bad.mp4", std::string(4096, 'z'));
    auto has = [](const std::list<std::string>& l, const std::string& s) {
        return std::any_of(l.begin(), l.end(), [&](const std::string& x) { return x.find(s) != std::string::npos; });
    };
    Fx f("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"64\" StrandsPerString=\"8\" StringType=\"RGB Nodes\"/>");
    auto* ve = f.Get("Video");
    Effect* e = f.Add("Video", "E_CHOICE_Video_DurationTreatment=Slow/Accelerate,E_TEXTCTRL_Video_Starttime=0.5", { { "E_FILEPICKERCTRL_Video_Filename", bad } });
    auto r = ve->CheckEffectSettings(e->GetSettings(), nullptr, f.model, e, false);
    CHECK(has(r, "could not be understood"));
    CHECK(has(r, "not under show directory"));
    Frames fr = f.Run(e, 4, 4, { 0, 1 });
    CHECK(AllColour(fr, 0, 255, 0, 0));
    CHECK(AllColour(fr, 1, 255, 0, 0));
    // Equal crops are widened by one percent rather than collapsing to nothing.
    Effect* crop = f.Add("Video", "E_SLIDER_Video_CropLeft=0,E_SLIDER_Video_CropRight=0,E_SLIDER_Video_CropTop=50,E_SLIDER_Video_CropBottom=50,E_TEXTCTRL_SampleSpacing=2",
                         { { "E_FILEPICKERCTRL_Video_Filename", bad } });
    Frames fc = f.Run(crop, 4, 4, { 0 });
    CHECK(AllColour(fc, 0, 255, 0, 0));

    ve->loadFiles(e);
    CHECK_FALSE(e->GetSettings().Get("E_FILEPICKERCTRL_Video_Filename", "").empty());
}

TEST_CASE("Video decode sizes skip models that cannot take video", "[effects][media]") {
    Fx f(Line("L", 10));
    const std::string file = "/no/such/dir/thin.mp4";
    f.Add("Video", "E_TEXTCTRL_Video_CropLeft=10", { { "E_FILEPICKERCTRL_Video_Filename", file } });
    f.Add("On", "");
    Fx other(Line("Other", 10));
    std::list<Model*> models{ nullptr, f.model, other.model };
    VideoDecodeSizeRegistry::Clear();
    VideoEffect::PrepareDecodeSizes(f.ctx._sequenceElements, models);
    int w = 0, h = 0;
    CHECK_FALSE(VideoDecodeSizeRegistry::GetMaxDecodeSize(file, w, h));
    VideoDecodeSizeRegistry::Clear();
}

// ================================================================== Moving Head song generator

TEST_CASE("Song generator needs bars and heads", "[effects][media]") {
    CHECK(MovingHeadSong::Generate({}, {}, 4).empty());
    CHECK(MovingHeadSong::Generate({ 0 }, {}, 4).empty());
    CHECK(MovingHeadSong::Generate({ 0, 1000 }, {}, 0).empty());
}

TEST_CASE("Song generator pools quiet bars into fanned beams with dark links", "[effects][media]") {
    MovingHeadSong::Params p;
    p.poolBars = 2;
    std::vector<long> bars = { 0, 2000, 4000, 6000, 8000 };
    auto out = MovingHeadSong::Generate(bars, {}, 3, p);
    // Four bars < minBarsForOutro: two pooled beams per head, each preceded by a dark link.
    std::vector<MovingHeadSong::HeadEffect> head0, head2;
    for (const auto& e : out) {
        if (e.head == 0) head0.push_back(e);
        if (e.head == 2) head2.push_back(e);
    }
    REQUIRE(head0.size() == 2);
    // The first beam starts at 0 so no link is needed; the second touches the first with the same pose.
    CHECK_FALSE(head0[0].linked);
    CHECK(head0[0].startMS == 0);
    CHECK(head0[0].endMS == 4000);
    CHECK(head0[1].startMS == 4000);
    CHECK(head0[1].endMS == 8000);
    CHECK(head0[0].tilt0 == Catch::Approx(-p.poolTilt));
    CHECK(head2[0].tilt0 == Catch::Approx(p.poolTilt));
    CHECK(head0[0].dimmer.size() == 6);

    // Only the requested heads are emitted.
    auto some = MovingHeadSong::Generate(bars, {}, 3, p, { 1 });
    CHECK(!some.empty());
    for (const auto& e : some) CHECK(e.head == 1);
    for (const auto& e : some) CHECK(e.tilt0 == Catch::Approx(0.0f));
}

TEST_CASE("Song generator alternates chorus banks and sweeps section starts", "[effects][media]") {
    std::vector<long> bars;
    for (int i = 0; i <= 8; ++i) bars.push_back(i * 2000);
    MovingHeadSong::Section verse{ 0, 4000, false, false };
    MovingHeadSong::Section chorus{ 4000, 12000, true, false };
    MovingHeadSong::Params p;
    auto out = MovingHeadSong::Generate(bars, { chorus, verse }, 4, p);
    REQUIRE(!out.empty());
    // Bar 2 is the first bar of the second section: a sweep on every head. Heads that must
    // move before the next beam have the sweep's tail trimmed for travel time.
    int sweeps = 0;
    for (const auto& e : out) {
        if (!e.linked && e.startMS == 4000 && e.dimmer.size() == 8) {
            ++sweeps;
            CHECK(e.endMS <= 6000);
            CHECK(e.endMS >= 4000 + (long)(2000 * p.minBeamFraction));
            CHECK(std::fabs(e.tilt0) > std::fabs(e.tilt1));
        }
    }
    CHECK(sweeps == 4);
    // Chorus bar 3 (6000..8000) flashes on half bars: even heads first, odd heads second.
    int bank0 = 0, bank1 = 0;
    for (const auto& e : out) {
        if (e.linked || e.startMS < 6000 || e.endMS > 8000) continue;
        if (e.head % 2 == 0) {
            CHECK(e.startMS == 6000);
            ++bank0;
        } else {
            CHECK(e.startMS == 7000);
            ++bank1;
        }
        CHECK(std::fabs(e.tilt1 - e.tilt0) == Catch::Approx(2 * p.chorusSwing));
    }
    CHECK(bank0 == 2);
    CHECK(bank1 == 2);
    // The last two bars are the outro, one long beam per head ending at the last bar.
    int outro = 0;
    for (const auto& e : out) {
        if (!e.linked && e.startMS == 12000 && e.endMS == 16000) {
            ++outro;
            CHECK(e.tilt1 == 0.0f);
        }
    }
    CHECK(outro == 4);

    // Left/right pairing groups the halves instead.
    p.chorusPairing = MovingHeadSong::ChorusPairing::LeftRight;
    p.chorusBeats = 1;
    auto lr = MovingHeadSong::Generate(bars, { chorus, verse }, 4, p);
    int lrBeams = 0;
    for (const auto& e : lr) {
        if (e.linked || e.startMS < 6000 || e.endMS > 8000) continue;
        long beat = (e.startMS - 6000) / 500;
        int bank = e.head < 2 ? 0 : 1;
        CHECK((beat % 2) == bank);
        ++lrBeams;
    }
    CHECK(lrBeams == 8); // four one-beat flashes, two heads each
}

TEST_CASE("Song generator bursts on peak sections and trims beams for travel", "[effects][media]") {
    std::vector<long> bars;
    for (int i = 0; i <= 4; ++i) bars.push_back(i * 2000);
    MovingHeadSong::Section peak{ 0, 8000, true, true };
    MovingHeadSong::Params p;
    p.burstEveryBars = 1;
    p.minBarsForOutro = 100;
    p.tiltSlewDegPerSec = { 10.0f, 10.0f };
    auto out = MovingHeadSong::Generate(bars, { peak }, 2, p);
    int bursts = 0;
    for (const auto& e : out) {
        if (!e.linked && e.dimmer.size() == 16) {
            ++bursts;
            CHECK(e.startMS % 2000 == 0);
            CHECK(e.endMS - e.startMS <= p.burstMS);
            CHECK(e.endMS - e.startMS >= 100);
            CHECK(e.tilt0 == 0.0f);
        }
    }
    CHECK(bursts == 2 * 4);
    // Effects for one head never overlap and links are dark.
    for (int h = 0; h < 2; ++h) {
        long cursor = -1;
        for (const auto& e : out) {
            if (e.head != h) continue;
            CHECK(e.startMS >= cursor);
            CHECK(e.endMS > e.startMS);
            cursor = e.endMS;
            if (e.linked) CHECK(e.dimmer.empty());
        }
    }
    // A slow head needs travel time, so some beam ends before the next one starts.
    bool trimmed = false;
    for (size_t i = 0; i + 1 < out.size(); ++i) {
        if (out[i].head == out[i + 1].head && out[i + 1].linked && out[i + 1].startMS > 0) trimmed = true;
    }
    CHECK(trimmed);
}

TEST_CASE("Song generator settings strings drive the moving head effect", "[effects][media]") {
    MovingHeadSong::HeadEffect beam;
    beam.head = 0;
    beam.startMS = 0;
    beam.endMS = 1000;
    beam.tilt0 = 10.0f;
    beam.tilt1 = 30.0f;
    beam.dimmer = { 0, 0, 0.5f, 1, 1, 0 };
    const std::string s = MovingHeadSong::BuildSettings(beam, 2);
    CHECK(s.find("E_TEXTCTRL_MH2_Settings=Color: 0.000000&comma;0.000000&comma;1.000000;Dimmer: 0&comma;0&comma;0.5&comma;1&comma;1&comma;0;Pan: 90.0;Tilt VC: ") != std::string::npos);
    CHECK(s.find("P1=100.00|P2=300.00") != std::string::npos);
    CHECK(s.find("E_CHECKBOX_MHLinkToNext=0") != std::string::npos);
    CHECK(s.find("E_SLIDER_MHTilt=0") != std::string::npos);
    CHECK(s.find("E_TEXTCTRL_MH1_Settings=,") != std::string::npos);
    CHECK(MovingHeadSong::BuildPalette() == "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1");

    MovingHeadSong::HeadEffect link;
    link.linked = true;
    link.tilt0 = link.tilt1 = -12.5f;
    const std::string l = MovingHeadSong::BuildSettings(link, 1);
    CHECK(l.find("E_TEXTCTRL_MH1_Settings=Pan: 90.0;Tilt: -12.5;Dimmer: 0.0&comma;0.0&comma;1.0&comma;0.0;") != std::string::npos);
    CHECK(l.find("E_CHECKBOX_MHLinkToNext=1") != std::string::npos);
    CHECK(l.find("E_SLIDER_MHTilt=-125") != std::string::npos);

    MovingHeadSong::HeadEffect still = beam;
    still.tilt1 = still.tilt0;
    const std::string st = MovingHeadSong::BuildSettings(still, 1);
    CHECK(st.find("Tilt: 10.0;PanOffset") != std::string::npos);
    CHECK(st.find("E_SLIDER_MHTilt=100") != std::string::npos);

    // The generated beam renders: white, dimmer up then down, tilt ramping.
    Fx f(Dmx("MH", "DmxMovingHead",
             "DmxChannelCount=\"12\" DmxFixture=\"MH2\" DmxRedChannel=\"5\" DmxGreenChannel=\"6\" DmxBlueChannel=\"7\" MhDimmerChannel=\"8\" DmxShutterChannel=\"9\" DmxShutterOnValue=\"200\"",
             kMhChildren));
    Effect* e = f.Add("Moving Head", s, {}, 0, 1000, MovingHeadSong::BuildPalette());
    Frames fr = f.RunDmx(e, { 0, 10, 19 });
    CHECK(fr.ch(1, 5) == 255);
    CHECK(fr.ch(1, 6) == 255);
    CHECK(fr.ch(1, 7) == 255);
    CHECK(fr.ch(1, 8) > 200);
    CHECK(fr.ch(0, 8) < 30);
    CHECK(fr.ch(1, 9) == 200);
    auto* mh = dynamic_cast<DmxMovingHead*>(f.model);
    REQUIRE(mh != nullptr);
    CHECK(std::abs(Cmd16(fr, 0, 3) - mh->GetTiltMotor()->ConvertPostoCmd(-10.0f)) <= 2);
    CHECK(std::abs(Cmd16(fr, 2, 3) - mh->GetTiltMotor()->ConvertPostoCmd(-30.0f)) <= 2);
    CHECK(Cmd16(fr, 0, 1) == mh->GetPanMotor()->ConvertPostoCmd(-90.0f));
}

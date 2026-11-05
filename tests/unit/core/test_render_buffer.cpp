#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/ColorCurve.h"
#include "render/HeadlessRenderContext.h"
#include "render/RenderBuffer.h"
#include "utils/Color.h"

namespace {

struct BufferFixture {
    HeadlessRenderContext ctx;
    std::unique_ptr<Model> model;

    BufferFixture() { model.reset(ctx.AllModels.CreateDefaultModel("Single Line")); }

    std::unique_ptr<RenderBuffer> Make(int w, int h) {
        auto rb = std::make_unique<RenderBuffer>(&ctx, nullptr, model.get());
        rb->InitBuffer(h, w, "None");
        rb->Clear();
        return rb;
    }
};

bool Same(const xlColor& a, const xlColor& b) {
    return a.red == b.red && a.green == b.green && a.blue == b.blue && a.alpha == b.alpha;
}

void SetPal(RenderBuffer& rb, std::vector<xlColor> colors) {
    xlColorVector cv(colors.begin(), colors.end());
    xlColorCurveVector curves(colors.size());
    rb.SetPalette(cv, curves);
}

int CountColor(RenderBuffer& rb, const xlColor& c) {
    int n = 0;
    for (int y = 0; y < rb.BufferHt; ++y) {
        for (int x = 0; x < rb.BufferWi; ++x) {
            if (Same(rb.GetPixel(x, y), c)) ++n;
        }
    }
    return n;
}

const xlColor kRed(255, 0, 0, 255);
const xlColor kBlue(0, 0, 255, 255);
const xlColor kClear(0, 0, 0, 0);

} // namespace

TEST_CASE("InitBuffer sizes the pixel store", "[render][buffer]") {
    BufferFixture f;
    struct Dim { int w, h; };
    for (Dim d : { Dim{ 1, 1 }, Dim{ 1, 9 }, Dim{ 9, 1 }, Dim{ 7, 3 }, Dim{ 3, 7 }, Dim{ 64, 64 } }) {
        INFO(d.w << "x" << d.h);
        auto rb = f.Make(d.w, d.h);
        CHECK(rb->BufferWi == d.w);
        CHECK(rb->BufferHt == d.h);
        CHECK(rb->GetPixelCount() == (uint32_t)(d.w * d.h));
        CHECK(CountColor(*rb, kClear) == d.w * d.h);
    }
}

TEST_CASE("Zero sized buffer is safe to draw on", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(0, 0);
    CHECK(rb->GetPixelCount() == 0);
    rb->SetPixel(0, 0, kRed);
    rb->Fill(kRed);
    rb->DrawHLine(0, 0, 5, kRed);
    rb->DrawBox(0, 0, 3, 3, kRed);
    rb->Clear();
    CHECK(Same(rb->GetPixel(0, 0), xlBLACK));
}

TEST_CASE("SetPixel and GetPixel round trip with origin at lower left", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(4, 3);
    rb->SetPixel(0, 0, kRed);
    rb->SetPixel(3, 2, kBlue);
    CHECK(Same(rb->GetPixel(0, 0), kRed));
    CHECK(Same(rb->GetPixel(3, 2), kBlue));
    CHECK(Same(rb->GetPixel(1, 0), kClear));
    CHECK(Same(rb->GetPixels()[0], kRed));
    CHECK(Same(rb->GetPixels()[2 * 4 + 3], kBlue));
    xlColor c;
    rb->GetPixel(3, 2, c);
    CHECK(Same(c, kBlue));
}

TEST_CASE("Out of range pixels are ignored on write and black on read", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(4, 3);
    rb->SetPixel(-1, 0, kRed);
    rb->SetPixel(0, -1, kRed);
    rb->SetPixel(4, 0, kRed);
    rb->SetPixel(0, 3, kRed);
    rb->SetPixel(100, 100, kRed);
    CHECK(CountColor(*rb, kRed) == 0);
    CHECK(Same(rb->GetPixel(-1, 0), xlBLACK));
    CHECK(Same(rb->GetPixel(4, 0), xlBLACK));
    CHECK(Same(rb->GetPixel(0, 3), xlBLACK));
    rb->SetAllowAlphaChannel(true);
    CHECK(Same(rb->GetPixel(-1, -1), xlCLEAR));
}

TEST_CASE("SetPixel with wrap folds negative coordinates back in", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(4, 3);
    rb->SetPixel(-1, 0, kRed, true);
    rb->SetPixel(0, -1, kBlue, true);
    CHECK(Same(rb->GetPixel(3, 0), kRed));
    CHECK(Same(rb->GetPixel(0, 2), kBlue));
}

TEST_CASE("SetPixel with wrap folds a coordinate equal to the size to zero", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(4, 3);
    rb->SetPixel(4, 1, kRed, true);
    rb->SetPixel(1, 3, kBlue, true);
    CHECK(Same(rb->GetPixel(0, 1), kRed));
    CHECK(Same(rb->GetPixel(1, 0), kBlue));
}

TEST_CASE("SetPixel HSV overload with wrap folds a coordinate equal to the size to zero", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(4, 3);
    HSVValue hsv(0.0, 1.0, 1.0);
    rb->SetPixel(4, 3, hsv, true);
    CHECK(Same(rb->GetPixel(0, 0), xlColor(hsv)));
}

TEST_CASE("SetPixel with alpha blends over the existing pixel", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetPixel(0, 0, kBlue);
    rb->SetPixel(0, 0, xlColor(255, 0, 0, 128), false, true);
    xlColor c = rb->GetPixel(0, 0);
    CHECK(c.red == 255);
    CHECK(std::abs(c.blue - 127) <= 1);
    CHECK(c.alpha == 255);
}

TEST_CASE("SetPixel without the alpha flag replaces the pixel", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetPixel(0, 0, kBlue);
    rb->SetPixel(0, 0, xlColor(255, 0, 0, 128), false, false);
    CHECK(Same(rb->GetPixel(0, 0), xlColor(255, 0, 0, 128)));
}

TEST_CASE("Fill and Clear touch every pixel", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(5, 4);
    rb->Fill(kRed);
    CHECK(CountColor(*rb, kRed) == 20);
    rb->Clear();
    CHECK(CountColor(*rb, kClear) == 20);
}

TEST_CASE("SetPixelColumn fills one column", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(5, 4);
    rb->SetPixelColumn(2, kRed);
    CHECK(CountColor(*rb, kRed) == 4);
    for (int y = 0; y < 4; ++y) CHECK(Same(rb->GetPixel(2, y), kRed));
    rb->SetPixelColumn(-1, kBlue);
    rb->SetPixelColumn(5, kBlue);
    CHECK(CountColor(*rb, kBlue) == 0);
}

TEST_CASE("Horizontal and vertical lines are inclusive and clipped", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(6, 5);
    rb->DrawHLine(1, 1, 3, kRed);
    CHECK(CountColor(*rb, kRed) == 3);
    rb->Clear();
    rb->DrawHLine(1, 4, 1, kRed);
    CHECK(CountColor(*rb, kRed) == 4);
    rb->Clear();
    rb->DrawHLine(1, -3, 20, kRed);
    CHECK(CountColor(*rb, kRed) == 6);
    rb->Clear();
    rb->DrawVLine(2, 0, 4, kBlue);
    CHECK(CountColor(*rb, kBlue) == 5);
    rb->Clear();
    rb->DrawVLine(2, 3, 1, kBlue);
    CHECK(CountColor(*rb, kBlue) == 3);
    rb->Clear();
    rb->DrawVLine(20, 0, 4, kBlue);
    CHECK(CountColor(*rb, kBlue) == 0);
}

TEST_CASE("DrawBox fills the inclusive rectangle in either corner order", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(8, 8);
    rb->DrawBox(1, 1, 3, 2, kRed);
    CHECK(CountColor(*rb, kRed) == 6);
    rb->Clear();
    rb->DrawBox(3, 2, 1, 1, kRed);
    CHECK(CountColor(*rb, kRed) == 6);
    rb->Clear();
    rb->DrawBox(-5, -5, 100, 100, kRed);
    CHECK(CountColor(*rb, kRed) == 64);
}

TEST_CASE("DrawLine reaches both end points", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(10, 10);
    rb->DrawLine(1, 1, 8, 6, kRed);
    CHECK(Same(rb->GetPixel(1, 1), kRed));
    CHECK(Same(rb->GetPixel(8, 6), kRed));
    CHECK(CountColor(*rb, kRed) >= 8);
}

TEST_CASE("DrawLine of a single point lights that point", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(5, 5);
    rb->DrawLine(2, 2, 2, 2, kRed);
    CHECK(Same(rb->GetPixel(2, 2), kRed));
    CHECK(CountColor(*rb, kRed) == 1);
}

TEST_CASE("Filled circle covers its centre and stays inside its radius", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(21, 21);
    rb->DrawCircle(10, 10, 5, kRed, true);
    CHECK(Same(rb->GetPixel(10, 10), kRed));
    CHECK(Same(rb->GetPixel(10, 15), kRed));
    CHECK(Same(rb->GetPixel(10, 5), kRed));
    CHECK(Same(rb->GetPixel(10, 17), kClear));
    CHECK(Same(rb->GetPixel(0, 0), kClear));
    int n = CountColor(*rb, kRed);
    CHECK(n > 60);
    CHECK(n < 110);
}

TEST_CASE("Outline circle leaves the centre empty", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(21, 21);
    rb->DrawCircle(10, 10, 5, kRed, false);
    CHECK(Same(rb->GetPixel(10, 10), kClear));
    CHECK(Same(rb->GetPixel(15, 10), kRed));
    CHECK(Same(rb->GetPixel(5, 10), kRed));
}

TEST_CASE("CopyPixel copies inside the buffer and ignores bad coordinates", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(4, 4);
    rb->SetPixel(0, 0, kRed);
    rb->CopyPixel(0, 0, 3, 3);
    CHECK(Same(rb->GetPixel(3, 3), kRed));
    rb->CopyPixel(-1, 0, 2, 2);
    rb->CopyPixel(0, 0, 9, 9);
    CHECK(Same(rb->GetPixel(2, 2), kClear));
    CHECK(CountColor(*rb, kRed) == 2);
}

TEST_CASE("Temp buffer is independent of the pixel buffer", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(3, 3);
    rb->ClearTempBuf();
    rb->SetTempPixel(1, 1, kRed);
    CHECK(Same(rb->GetTempPixel(1, 1), kRed));
    CHECK(Same(rb->GetPixel(1, 1), kClear));
    rb->CopyTempBufToPixels();
    CHECK(Same(rb->GetPixel(1, 1), kRed));
    rb->ClearTempBuf();
    CHECK(Same(rb->GetTempPixel(1, 1), kClear));
    CHECK(Same(rb->GetPixel(1, 1), kRed));
    rb->SetTempPixel(7, 7, kBlue);
    rb->SetTempPixel(0, 0, xlColor(10, 20, 30), 40);
    CHECK(Same(rb->GetTempPixel(0, 0), xlColor(10, 20, 30, 40)));
}

TEST_CASE("CopyPixelsToTempBuf snapshots the pixels", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(3, 3);
    rb->SetPixel(2, 1, kBlue);
    rb->CopyPixelsToTempBuf();
    rb->Clear();
    CHECK(Same(rb->GetTempPixel(2, 1), kBlue));
    CHECK(Same(rb->GetPixel(2, 1), kClear));
}

TEST_CASE("AlphaBlend with an opaque source replaces and with a clear source keeps", "[render][buffer]") {
    BufferFixture f;
    auto dst = f.Make(2, 2);
    auto src = f.Make(2, 2);
    dst->Fill(kBlue);
    src->SetPixel(0, 0, kRed);
    dst->AlphaBlend(*src);
    CHECK(Same(dst->GetPixel(0, 0), kRed));
    CHECK(Same(dst->GetPixel(1, 1), kBlue));
}

TEST_CASE("AlphaBlend adds a translucent source over the destination", "[render][buffer]") {
    BufferFixture f;
    auto dst = f.Make(1, 1);
    auto src = f.Make(1, 1);
    dst->Fill(xlColor(0, 0, 200, 255));
    src->Fill(xlColor(100, 0, 0, 128));
    dst->AlphaBlend(*src);
    xlColor c = dst->GetPixel(0, 0);
    CHECK(c.red == 100);
    CHECK(std::abs(c.blue - 100) <= 1);
    CHECK(c.alpha == 255);
}

TEST_CASE("AlphaBlend between different sized buffers does nothing", "[render][buffer]") {
    BufferFixture f;
    auto dst = f.Make(2, 2);
    auto src = f.Make(3, 3);
    dst->Fill(kBlue);
    src->Fill(kRed);
    dst->AlphaBlend(*src);
    CHECK(CountColor(*dst, kBlue) == 4);
}

TEST_CASE("ChannelBlend interpolates and rounds to nearest", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    CHECK(rb->ChannelBlend(0, 255, 0.0f) == 0);
    CHECK(rb->ChannelBlend(0, 255, 1.0f) == 255);
    CHECK(rb->ChannelBlend(0, 100, 0.5f) == 50);
    CHECK(rb->ChannelBlend(200, 100, 0.5f) == 150);
    CHECK(rb->ChannelBlend(10, 10, 0.7f) == 10);
    CHECK(rb->ChannelBlend(0, 255, 0.5f) == 128);
}

TEST_CASE("Palette colour count and 2 colour blends", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    SetPal(*rb, { kRed, kBlue });
    CHECK(rb->GetColorCount() == 2);
    xlColor c;
    rb->Get2ColorBlend(0, 1, 0.0f, c);
    CHECK(Same(c, kRed));
    rb->Get2ColorBlend(0, 1, 1.0f, c);
    CHECK(Same(c, kBlue));
    rb->Get2ColorBlend(0, 1, 0.5f, c);
    CHECK(std::abs(c.red - 128) <= 1);
    CHECK(c.green == 0);
    CHECK(std::abs(c.blue - 128) <= 1);
    xlColor d = kRed;
    rb->Get2ColorBlend(d, xlColor(0, 255, 0), 1.0f);
    CHECK(d.red == 0);
    CHECK(d.green == 255);
}

TEST_CASE("Multi colour blend walks the palette", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    SetPal(*rb, { kRed, xlColor(0, 255, 0), kBlue });
    xlColor c;
    rb->GetMultiColorBlend(0.0f, false, c);
    CHECK(Same(c, kRed));
    rb->GetMultiColorBlend(0.5f, false, c);
    CHECK(c.red == 0);
    CHECK(c.green == 255);
    CHECK(c.blue == 0);
    rb->GetMultiColorBlend(1.0f, false, c);
    CHECK(c.blue >= 254);
    CHECK(c.red == 0);
    rb->GetMultiColorBlend(5.0f, false, c);
    CHECK(c.blue >= 254);
    rb->GetMultiColorBlend(-1.0f, false, c);
    CHECK(Same(c, kRed));
}

TEST_CASE("Circular multi colour blend returns to the first colour", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    SetPal(*rb, { kRed, kBlue });
    xlColor c;
    rb->GetMultiColorBlend(0.0f, true, c);
    CHECK(Same(c, kRed));
    rb->GetMultiColorBlend(0.5f, true, c);
    CHECK(Same(c, kBlue));
    rb->GetMultiColorBlend(0.99999f, true, c);
    CHECK(c.red >= 250);
}

TEST_CASE("Multi colour blend with a single colour returns it", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    SetPal(*rb, { kBlue });
    xlColor c;
    rb->GetMultiColorBlend(0.7f, false, c);
    CHECK(Same(c, kBlue));
}

TEST_CASE("Empty palette still reports one colour", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    CHECK(rb->GetColorCount() == 1);
}

TEST_CASE("Effect periods follow the frame time", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetFrameTimeInMs(50);
    rb->SetEffectDuration(100, 1000);
    int s = 0, e = 0;
    rb->GetEffectPeriods(s, e);
    CHECK(s == 2);
    CHECK(e == 19);
    CHECK(rb->GetStartTimeMS() == 100);
    CHECK(rb->GetEndTimeMS() == 950);
    rb->SetFrameTimeInMs(25);
    rb->SetEffectDuration(100, 1000);
    rb->GetEffectPeriods(s, e);
    CHECK(s == 4);
    CHECK(e == 39);
}

TEST_CASE("Effect time position runs from 0 at the first frame to 1 at the last", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetFrameTimeInMs(50);
    rb->SetEffectDuration(0, 1000);
    rb->SetState(0, true);
    CHECK(rb->GetEffectTimeIntervalPosition() == 0.0f);
    rb->SetState(19, false);
    CHECK(std::abs(rb->GetEffectTimeIntervalPosition() - 1.0f) < 0.0001f);
    rb->SetState(10, false);
    CHECK(std::abs(rb->GetEffectTimeIntervalPosition() - 10.0f / 19.0f) < 0.0001f);
}

TEST_CASE("Effect time position is zero for a one frame effect", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetFrameTimeInMs(50);
    rb->SetEffectDuration(500, 550);
    rb->SetState(10, true);
    CHECK(rb->GetEffectTimeIntervalPosition() == 0.0f);
}

TEST_CASE("SetState requests re-initialisation only when asked", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->needToInit = false;
    rb->SetState(3, false);
    CHECK_FALSE(rb->needToInit);
    CHECK(rb->curPeriod == 3);
    rb->SetState(4, true);
    CHECK(rb->needToInit);
}

TEST_CASE("Colour curves in the palette follow effect progress", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(1, 1);
    rb->SetFrameTimeInMs(50);
    rb->SetEffectDuration(0, 1000);
    xlColorVector cv{ kRed };
    xlColorCurveVector curves;
    ColorCurve cc("Active=TRUE|Id=ID_A|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF|");
    curves.push_back(cc);
    rb->SetPalette(cv, curves);
    xlColor first, last;
    rb->SetState(0, true);
    rb->GetPalette().GetColor(0, first);
    rb->SetState(19, false);
    rb->GetPalette().GetColor(0, last);
    CHECK(first.red > first.blue);
    CHECK(last.blue > last.red);
}

TEST_CASE("Random numbers are reproducible for a model layer and effect start", "[render][buffer][rng]") {
    BufferFixture f;
    auto a = f.Make(4, 4);
    auto b = f.Make(4, 4);
    for (auto* rb : { a.get(), b.get() }) {
        rb->SetFrameTimeInMs(50);
        rb->SetEffectDuration(0, 1000);
        rb->SetState(7, true);
    }
    for (int i = 0; i < 20; ++i) {
        CHECK(a->rand01() == b->rand01());
    }
    for (int i = 0; i < 20; ++i) {
        CHECK(a->randInt(0, 1000) == b->randInt(0, 1000));
    }
}

TEST_CASE("rand01 stays within the half open unit interval", "[render][buffer][rng]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetEffectDuration(0, 1000);
    rb->SetState(1, true);
    double mn = 1, mx = 0;
    for (int i = 0; i < 2000; ++i) {
        double v = rb->rand01();
        mn = std::min(mn, v);
        mx = std::max(mx, v);
        REQUIRE(v >= 0.0);
        REQUIRE(v < 1.0);
    }
    CHECK(mx - mn > 0.9);
}

TEST_CASE("randInt is inclusive and collapses an empty range", "[render][buffer][rng]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetEffectDuration(0, 1000);
    rb->SetState(1, true);
    std::set<int> seen;
    for (int i = 0; i < 500; ++i) {
        int v = rb->randInt(3, 6);
        REQUIRE(v >= 3);
        REQUIRE(v <= 6);
        seen.insert(v);
    }
    CHECK(seen == std::set<int>{ 3, 4, 5, 6 });
    CHECK(rb->randInt(5, 5) == 5);
    CHECK(rb->randInt(9, 2) == 9);
}

TEST_CASE("The random stream restarts each frame", "[render][buffer][rng]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetEffectDuration(0, 1000);
    rb->SetState(4, true);
    double a1 = rb->rand01();
    double a2 = rb->rand01();
    CHECK(a1 != a2);
    rb->SetState(5, false);
    double b1 = rb->rand01();
    CHECK(b1 != a1);
    rb->SetState(4, false);
    CHECK(rb->rand01() == a1);
}

TEST_CASE("Hash random values depend on index frame and layer", "[render][buffer][rng]") {
    BufferFixture f;
    auto a = f.Make(2, 2);
    auto b = f.Make(2, 2);
    a->SetEffectDuration(0, 1000);
    b->SetEffectDuration(0, 1000);
    a->SetState(3, true);
    b->SetState(3, true);
    CHECK(a->hashRandom(5) == b->hashRandom(5));
    CHECK(a->hashRand01(5) == b->hashRand01(5));
    CHECK(a->hashRandom(5) != a->hashRandom(6));
    b->SetState(4, false);
    CHECK(a->hashRandom(5) != b->hashRandom(5));
    b->SetState(3, false);
    b->SetLayerIndex(2);
    b->SetEffectDuration(0, 1000);
    CHECK(a->hashRandom(5) != b->hashRandom(5));
    double v = a->hashRand01(123);
    CHECK(v >= 0.0);
    CHECK(v < 1.0);
}

TEST_CASE("Stable hash random ignores the frame", "[render][buffer][rng]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    rb->SetEffectDuration(0, 1000);
    rb->SetState(2, true);
    uint32_t a = rb->hashRandomStable(9);
    uint32_t m = rb->hashModelStable(9);
    rb->SetState(11, false);
    CHECK(rb->hashRandomStable(9) == a);
    CHECK(rb->hashModelStable(9) == m);
    CHECK(rb->hashRandomStable(9) != rb->hashRandomStable(10));
}

TEST_CASE("RandomRange stays within its bounds", "[render][buffer][rng]") {
    BufferFixture f;
    auto rb = f.Make(2, 2);
    for (int i = 0; i < 500; ++i) {
        double v = rb->RandomRange(2.0, 5.0);
        REQUIRE(v >= 2.0);
        REQUIRE(v <= 5.0);
    }
}

TEST_CASE("Memory estimate grows with buffer size", "[render][buffer]") {
    BufferFixture f;
    auto small = f.Make(2, 2);
    auto big = f.Make(100, 100);
    CHECK(big->GetApproxMemoryBytes() > small->GetApproxMemoryBytes());
    CHECK(big->GetApproxMemoryBytes() >= 100u * 100u * sizeof(xlColor));
}

TEST_CASE("Shrinking and regrowing a buffer keeps it consistent", "[render][buffer]") {
    BufferFixture f;
    auto rb = f.Make(10, 10);
    rb->Fill(kRed);
    rb->InitBuffer(2, 3, "None");
    CHECK(rb->GetPixelCount() == 6);
    rb->Clear();
    rb->Fill(kBlue);
    CHECK(CountColor(*rb, kBlue) == 6);
    rb->InitBuffer(20, 30, "None");
    CHECK(rb->GetPixelCount() == 600);
    rb->Clear();
    CHECK(CountColor(*rb, kClear) == 600);
}

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "graphics/xlFontInfo.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "graphics/xlGraphicsCapability.h"

using Catch::Matchers::WithinAbs;

namespace {

struct V3 {
    float x, y, z;
};

struct RecordingVA : xlVertexAccumulator {
    std::vector<V3> v;
    unsigned prealloc = 0;
    void PreAlloc(unsigned int i) override { prealloc += i; }
    void AddVertex(float x, float y, float z) override { v.push_back({ x, y, z }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
};

struct CV {
    V3 p;
    xlColor c;
};

struct RecordingCVA : xlVertexColorAccumulator {
    std::vector<CV> v;
    unsigned prealloc = 0;
    int finalizeCalls = 0;
    void PreAlloc(unsigned int i) override { prealloc += i; }
    void AddVertex(float x, float y, float z, const xlColor& c) override { v.push_back({ { x, y, z }, c }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void Finalize(bool, bool) override { ++finalizeCalls; }
    void SetVertex(uint32_t i, float x, float y, float z, const xlColor& c) override { v[i] = { { x, y, z }, c }; }
    void SetVertex(uint32_t i, float x, float y, float z) override { v[i].p = { x, y, z }; }
    void SetVertex(uint32_t i, const xlColor& c) override { v[i].c = c; }
};

struct RecordingIVA : xlVertexIndexedColorAccumulator {
    struct E {
        V3 p;
        uint32_t idx;
    };
    std::vector<E> v;
    void AddVertex(float x, float y, float z, uint32_t i) override { v.push_back({ { x, y, z }, i }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t, float, float, float, uint32_t) override {}
    void SetVertex(uint32_t, float, float, float) override {}
    void SetVertex(uint32_t, uint32_t) override {}
};

struct TV {
    float x, y, z, tx, ty;
};

struct RecordingTVA : xlVertexTextureAccumulator {
    std::vector<TV> v;
    void AddVertex(float x, float y, float z, float tx, float ty) override { v.push_back({ x, y, z, tx, ty }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
};

} // namespace

TEST_CASE("xlVertexAccumulator rect as lines emits four closed edges", "[misc][graphics]") {
    RecordingVA a;
    a.AddRectAsLines(1, 2, 5, 8);
    REQUIRE(a.v.size() == 8);
    CHECK(a.prealloc == 8);
    CHECK(a.v[0].x == 1);
    CHECK(a.v[0].y == 2);
    CHECK(a.v[1].y == 8);
    CHECK(a.v[7].x == 1);
    CHECK(a.v[7].y == 2);
    for (auto const& p : a.v) {
        CHECK(p.z == 0);
        CHECK((p.x == 1 || p.x == 5));
        CHECK((p.y == 2 || p.y == 8));
    }
}

TEST_CASE("xlVertexAccumulator rect as triangles emits two triangles", "[misc][graphics]") {
    RecordingVA a;
    a.AddRectAsTriangles(0, 0, 10, 4);
    REQUIRE(a.v.size() == 6);
    CHECK(a.v[2].x == 10);
    CHECK(a.v[2].y == 4);
    CHECK(a.v[3].x == a.v[2].x);
    CHECK(a.v[3].y == a.v[2].y);
    CHECK(a.v[5].x == a.v[0].x);
    CHECK(a.v[5].y == a.v[0].y);
}

TEST_CASE("xlVertexAccumulator circle as lines closes the loop", "[misc][graphics]") {
    RecordingVA a;
    a.AddCircleAsLines(10, 20, 5);
    REQUIRE(a.v.size() == 25);
    for (auto const& p : a.v) {
        float d = std::hypot(p.x - 10, p.y - 20);
        CHECK_THAT(d, WithinAbs(5.0f, 1e-3));
    }
    CHECK_THAT(a.v.front().x, WithinAbs(15.0f, 1e-3));
    CHECK_THAT(a.v.back().x, WithinAbs(a.v.front().x, 1e-2));
    CHECK_THAT(a.v.back().y, WithinAbs(a.v.front().y, 1e-2));
}

TEST_CASE("xlVertexAccumulator dashed rect stays within the rect extents", "[misc][graphics]") {
    RecordingVA a;
    a.AddRectAsDashedLines(0, 0, 20, 10, 4);
    CHECK(a.v.size() % 2 == 0);
    CHECK(a.v.size() >= 16);
    for (auto const& p : a.v) {
        CHECK(p.x >= 0);
        CHECK(p.x <= 20);
        CHECK(p.y >= 0);
        CHECK(p.y <= 10);
    }
}

TEST_CASE("xlVertexAccumulator dashed rect accepts reversed corners", "[misc][graphics]") {
    RecordingVA a;
    RecordingVA b;
    a.AddRectAsDashedLines(0, 0, 20, 10, 4);
    b.AddRectAsDashedLines(20, 10, 0, 0, 4);
    CHECK(a.v.size() == b.v.size());
}

TEST_CASE("xlVertexColorAccumulator rect as triangles carries colour and depth", "[misc][graphics]") {
    RecordingCVA a;
    xlColor red(255, 0, 0);
    a.AddRectAsTriangles(0, 0, 3, 3, red);
    REQUIRE(a.v.size() == 6);
    CHECK(a.prealloc == 6);
    for (auto const& p : a.v) {
        CHECK(p.p.z == 0);
        CHECK(p.c.red == 255);
    }

    RecordingCVA b;
    b.AddRectAsTriangles(0, 0, 3, 3, 7.0f, red);
    for (auto const& p : b.v) {
        CHECK(p.p.z == 7.0f);
    }
}

TEST_CASE("xlVertexColorAccumulator rect as lines", "[misc][graphics]") {
    RecordingCVA a;
    a.AddRectAsLines(0, 0, 4, 2, xlWHITE);
    REQUIRE(a.v.size() == 8);
    CHECK(a.v[0].p.x == 0);
    CHECK(a.v[7].p.x == 0);
    CHECK(a.v[7].p.y == 0);
    CHECK(a.v[3].p.x == 4);
    CHECK(a.v[3].p.y == 2);
}

TEST_CASE("xlVertexColorAccumulator dashed rect stays in range", "[misc][graphics]") {
    RecordingCVA a;
    a.AddRectAsDashedLines(2, 3, 30, 12, 5, xlWHITE);
    CHECK(a.v.size() > 0);
    CHECK(a.v.size() % 2 == 0);
    for (auto const& p : a.v) {
        CHECK(p.p.x >= 2);
        CHECK(p.p.x <= 30);
        CHECK(p.p.y >= 3);
        CHECK(p.p.y <= 12);
    }
}

TEST_CASE("xlVertexColorAccumulator circle as lines", "[misc][graphics]") {
    RecordingCVA a;
    a.AddCircleAsLines(0, 0, 10, xlWHITE);
    REQUIRE(a.v.size() == 25);
    for (auto const& p : a.v) {
        CHECK_THAT(std::hypot(p.p.x, p.p.y), WithinAbs(10.0f, 1e-3));
    }
}

TEST_CASE("xlVertexColorAccumulator circle as triangles uses at least 16 segments", "[misc][graphics]") {
    RecordingCVA a;
    a.AddCircleAsTriangles(5, 6, 3, xlWHITE);
    CHECK(a.v.size() == 16 * 3);
    CHECK(a.prealloc == 16 * 4);
}

TEST_CASE("xlVertexColorAccumulator circle as triangles scales segments with radius", "[misc][graphics]") {
    RecordingCVA a;
    a.AddCircleAsTriangles(0, 0, 0, 40, xlWHITE, xlWHITE, 0.0f);
    CHECK(a.v.size() == 40 * 3);

    RecordingCVA b;
    b.AddCircleAsTriangles(0, 0, 0, 40, xlWHITE, xlWHITE, 0.0f, 24);
    CHECK(b.v.size() == 24 * 3);

    RecordingCVA c;
    c.AddCircleAsTriangles(0, 0, 0, 40, xlWHITE, xlWHITE, 0.0f, 2);
    CHECK(c.v.size() == 16 * 3);
}

TEST_CASE("xlVertexColorAccumulator circle triangles put the centre colour at the centre", "[misc][graphics]") {
    RecordingCVA a;
    xlColor centre(10, 20, 30);
    xlColor edge(200, 100, 50);
    a.AddCircleAsTriangles(4, 5, 6, 10, centre, edge);
    REQUIRE(a.v.size() == 48);
    for (size_t i = 0; i < a.v.size(); i += 3) {
        CHECK(a.v[i].c.red == 200);
        CHECK(a.v[i + 1].c.red == 200);
        CHECK(a.v[i + 2].c.red == 10);
        CHECK(a.v[i + 2].p.x == 4);
        CHECK(a.v[i + 2].p.y == 5);
        CHECK(a.v[i + 2].p.z == 6);
        CHECK_THAT(std::hypot(a.v[i].p.x - 4, a.v[i].p.y - 5), WithinAbs(10.0f, 1e-2));
    }
}

TEST_CASE("xlVertexColorAccumulator circle depth ratio lifts the rim", "[misc][graphics]") {
    RecordingCVA a;
    a.AddCircleAsTriangles(0, 0, 1, 10, xlWHITE, xlWHITE, 0.5f);
    REQUIRE(a.v.size() >= 3);
    CHECK_THAT(a.v[0].p.z, WithinAbs(6.0f, 1e-4));
    CHECK(a.v[2].p.z == 1);
}

TEST_CASE("xlVertexColorAccumulator circle with zero radius does not blow up", "[misc][graphics]") {
    RecordingCVA a;
    a.AddCircleAsTriangles(0, 0, 0, xlWHITE);
    CHECK(a.v.size() == 48);
    for (auto const& p : a.v) {
        CHECK(p.p.x == 0);
        CHECK(p.p.y == 0);
    }
}

TEST_CASE("xlVertexColorAccumulator cube has 12 triangles within its extents", "[misc][graphics]") {
    RecordingCVA a;
    a.AddCubeAsTriangles(10, 20, 30, 4, xlWHITE);
    REQUIRE(a.v.size() == 36);
    for (auto const& p : a.v) {
        CHECK_THAT(std::fabs(p.p.x - 10), WithinAbs(2.0f, 1e-4));
        CHECK_THAT(std::fabs(p.p.y - 20), WithinAbs(2.0f, 1e-4));
        CHECK_THAT(std::fabs(p.p.z - 30), WithinAbs(2.0f, 1e-4));
    }
}

TEST_CASE("xlVertexColorAccumulator sphere is drawn as a cube of twice the radius", "[misc][graphics]") {
    RecordingCVA s;
    RecordingCVA c;
    s.AddSphereAsTriangles(1, 2, 3, 5, xlWHITE);
    c.AddCubeAsTriangles(1, 2, 3, 10, xlWHITE);
    REQUIRE(s.v.size() == c.v.size());
    for (size_t i = 0; i < s.v.size(); ++i) {
        CHECK(s.v[i].p.x == c.v[i].p.x);
        CHECK(s.v[i].p.y == c.v[i].p.y);
    }
}

TEST_CASE("xlVertexColorAccumulator horizontal blend with no colours is a no-op", "[misc][graphics]") {
    RecordingCVA a;
    xlColorVector none;
    a.AddHBlendedRectangleAsTriangles(0, 0, 10, 10, nullptr, 0, none);
    CHECK(a.v.empty());
}

TEST_CASE("xlVertexColorAccumulator horizontal blend with one colour is a flat rect", "[misc][graphics]") {
    RecordingCVA a;
    xlColorVector one = { xlColor(9, 8, 7) };
    a.AddHBlendedRectangleAsTriangles(0, 0, 10, 10, nullptr, 0, one);
    REQUIRE(a.v.size() == 6);
    for (auto const& p : a.v) {
        CHECK(p.c.red == 9);
        CHECK(p.c.green == 8);
    }
}

TEST_CASE("xlVertexColorAccumulator horizontal blend splits evenly between colours", "[misc][graphics]") {
    RecordingCVA a;
    xlColorVector cols = { xlColor(255, 0, 0), xlColor(0, 255, 0), xlColor(0, 0, 255) };
    a.AddHBlendedRectangleAsTriangles(0, 0, 100, 10, nullptr, 0, cols);
    REQUIRE(a.v.size() == 12);
    CHECK(a.v[0].c.red == 255);
    CHECK(a.v[2].c.green == 255);
    CHECK(a.v[2].p.x == 50);
    CHECK(a.v[6].p.x == 50);
    CHECK(a.v[8].c.blue == 255);
    CHECK(a.v[8].p.x == 100);
}

TEST_CASE("xlVertexColorAccumulator horizontal blend applies the colour mask", "[misc][graphics]") {
    RecordingCVA a;
    xlColor mask(255, 0, 0);
    xlColorVector cols = { xlColor(200, 200, 200), xlColor(100, 100, 100) };
    a.AddHBlendedRectangleAsTriangles(0, 0, 10, 10, &mask, 0, cols);
    REQUIRE(a.v.size() == 6);
    CHECK(a.v[0].c.red == 200);
    CHECK(a.v[0].c.green == 0);
    CHECK(a.v[2].c.red == 100);
    CHECK(a.v[2].c.blue == 0);
}

TEST_CASE("xlVertexIndexedColorAccumulator circle uses colour indices", "[misc][graphics]") {
    RecordingIVA a;
    a.AddCircleAsTriangles(0, 0, 0, 10, 3, 5);
    REQUIRE(a.v.size() == 48);
    for (size_t i = 0; i < a.v.size(); i += 3) {
        CHECK(a.v[i].idx == 5);
        CHECK(a.v[i + 1].idx == 5);
        CHECK(a.v[i + 2].idx == 3);
    }
    RecordingIVA b;
    b.AddCircleAsTriangles(0, 0, 0, 10, 2);
    for (auto const& e : b.v) {
        CHECK(e.idx == 2);
    }
}

TEST_CASE("xlVertexTextureAccumulator AddTexture emits a textured quad", "[misc][graphics]") {
    RecordingTVA a;
    a.AddTexture(0, 0, 8, 4, 0.25f, 0.5f, 0.75f, 1.0f);
    REQUIRE(a.v.size() == 6);
    CHECK(a.v[0].tx == 0.25f);
    CHECK(a.v[0].ty == 0.5f);
    CHECK(a.v[2].x == 8);
    CHECK(a.v[2].tx == 0.75f);
    CHECK(a.v[2].ty == 1.0f);

    RecordingTVA f;
    f.AddFullTexture(0, 0, 1, 1);
    REQUIRE(f.v.size() == 6);
    CHECK(f.v[0].tx == 0);
    CHECK(f.v[2].tx == 1);
    CHECK(f.v[2].ty == 1);
}

TEST_CASE("accumulators keep the name they are given", "[misc][graphics]") {
    RecordingVA a;
    RecordingCVA b;
    CHECK(a.SetName("lines") == &a);
    CHECK(a.GetName() == "lines");
    CHECK(b.SetName("tri") == &b);
    CHECK(b.GetName() == "tri");
    CHECK(a.Flush() == &a);
}

TEST_CASE("xlDisplayList maps normalised items into the target rect", "[misc][graphics]") {
    xlDisplayList dl;
    RecordingCVA acc;
    dl.addToAccumulator(0, 0, 100, 100, acc);
    CHECK(acc.v.empty());

    xlDisplayListItem i1;
    i1.x = 0.5f;
    i1.y = 0.25f;
    i1.color = xlColor(1, 2, 3);
    dl.push_back(i1);
    dl.addToAccumulator(10, 20, 200, 100, acc);
    REQUIRE(acc.v.size() == 1);
    CHECK_THAT(acc.v[0].p.x, WithinAbs(110.0f, 1e-4));
    CHECK_THAT(acc.v[0].p.y, WithinAbs(45.0f, 1e-4));
    CHECK(acc.v[0].c.blue == 3);

    dl.LockedClear();
    CHECK(dl.empty());
    CHECK(dl.iconSize == 2);
}

TEST_CASE("xlGraphicsProgram finalizes the accumulator then runs steps in order", "[misc][graphics]") {
    auto* acc = new RecordingCVA();
    std::vector<int> order;
    {
        xlGraphicsProgram prog(acc);
        CHECK(prog.getAccumulator() == acc);
        prog.addStep([&](xlGraphicsContext*) { order.push_back(1); });
        prog.addStep([&](xlGraphicsContext*) { order.push_back(acc->finalizeCalls); });
        prog.addStep([&](xlGraphicsContext*) { order.push_back(3); });
        prog.runSteps(nullptr);
    }
    REQUIRE(order.size() == 3);
    CHECK(order[0] == 1);
    CHECK(order[1] == 1);
    CHECK(order[2] == 3);
}

TEST_CASE("xlGraphicsProgram without an accumulator still runs", "[misc][graphics]") {
    xlGraphicsProgram prog(nullptr);
    int n = 0;
    prog.addStep([&](xlGraphicsContext*) { ++n; });
    prog.runSteps(nullptr);
    CHECK(n == 1);
    CHECK(prog.getAccumulator() == nullptr);
}

TEST_CASE("xlFontInfo FindFont returns the exact size when it exists", "[misc][graphics][font]") {
    for (int sz : { 8, 12, 16, 24, 88 }) {
        auto const& f = xlFontInfo::FindFont(sz);
        INFO("size " << sz);
        CHECK(f.getSize() == sz);
        CHECK(f.getImage().IsOk());
    }
}

TEST_CASE("xlFontInfo FindFont falls back to a neighbouring stock size", "[misc][graphics][font]") {
    auto const& f13 = xlFontInfo::FindFont(13);
    CHECK((f13.getSize() == 12 || f13.getSize() == 14));
    CHECK(f13.getImage().IsOk());
    CHECK(xlFontInfo::FindFont(1).getSize() == 8);
    CHECK(xlFontInfo::FindFont(0).getSize() == 8);
    CHECK(xlFontInfo::FindFont(-4).getSize() == 8);
}

TEST_CASE("xlFontInfo FindFont gives a usable font for every size up to 88", "[misc][graphics][font]") {
    for (int sz = 1; sz <= 88; ++sz) {
        INFO("size " << sz);
        CHECK(xlFontInfo::FindFont(sz).getImage().IsOk());
    }
}

// FindFont's downward search stops without returning when the stock size it reaches (88) was already
// loaded, then the upward search never runs for sizes above 88, so an unloaded font comes back.
TEST_CASE("xlFontInfo FindFont above the largest stock size uses the largest font", "[misc][graphics][font][!mayfail]") {
    (void)xlFontInfo::FindFont(88);
    auto const& big = xlFontInfo::FindFont(100);
    CHECK(big.getImage().IsOk());
    CHECK(big.getSize() == 88);
}

TEST_CASE("xlFontInfo FindFont returns a stable object per size", "[misc][graphics][font]") {
    CHECK(&xlFontInfo::FindFont(16) == &xlFontInfo::FindFont(16));
    CHECK(xlFontInfo::FindFont(16).getID() == xlFontInfo::FindFont(16).getID());
    CHECK(xlFontInfo::FindFont(16).getID() != xlFontInfo::FindFont(18).getID());
}

TEST_CASE("xlFontInfo widthOf is additive and scales with the factor", "[misc][graphics][font]") {
    auto const& f = xlFontInfo::FindFont(16);
    float a = f.widthOf("A");
    float b = f.widthOf("B");
    CHECK(a > 0);
    CHECK_THAT(f.widthOf("AB"), WithinAbs(a + b, 1e-3));
    CHECK_THAT(f.widthOf("AB", 2.0f), WithinAbs((a + b) / 2, 1e-3));
    CHECK(f.widthOf("") == 0);
    CHECK(f.widthOf(" ") > 0);
}

TEST_CASE("xlFontInfo widthOf treats unprintable characters as question marks", "[misc][graphics][font]") {
    auto const& f = xlFontInfo::FindFont(16);
    float q = f.widthOf("?");
    CHECK_THAT(f.widthOf("\x01"), WithinAbs(q, 1e-5));
    CHECK_THAT(f.widthOf("\n"), WithinAbs(q, 1e-5));
    CHECK_THAT(f.widthOf("\xC3\xA9"), WithinAbs(2 * q, 1e-5));
    CHECK_THAT(f.widthOf(std::string(1, '\x7f')), WithinAbs(q, 1e-5));
}

TEST_CASE("xlFontInfo populate emits six vertices per visible glyph", "[misc][graphics][font]") {
    auto const& f = xlFontInfo::FindFont(16);
    RecordingTVA va;
    f.populate(va, 10, 50, "Ab");
    REQUIRE(va.v.size() == 12);
    for (auto const& p : va.v) {
        CHECK(p.tx >= 0);
        CHECK(p.tx <= 1);
        CHECK(p.ty >= 0);
        CHECK(p.ty <= 1);
    }
    CHECK(va.v[0].x >= 10);
    CHECK(va.v[6].x > va.v[0].x);
    float right = 0;
    for (auto const& p : va.v) {
        right = std::max(right, p.x);
    }
    CHECK_THAT(right, WithinAbs(10 + f.widthOf("Ab"), 0.5f));
}

TEST_CASE("xlFontInfo populate skips spaces but advances", "[misc][graphics][font]") {
    auto const& f = xlFontInfo::FindFont(16);
    RecordingTVA a;
    f.populate(a, 0, 20, "A");
    RecordingTVA b;
    f.populate(b, 0, 20, " A");
    REQUIRE(a.v.size() == 6);
    REQUIRE(b.v.size() == 6);
    CHECK_THAT(b.v[0].x - a.v[0].x, WithinAbs(f.widthOf(" "), 1e-3));
}

TEST_CASE("xlFontInfo populate with empty text adds nothing", "[misc][graphics][font]") {
    auto const& f = xlFontInfo::FindFont(16);
    RecordingTVA va;
    f.populate(va, 0, 0, "");
    CHECK(va.v.empty());
}

TEST_CASE("xlFontInfo populate scales by the factor", "[misc][graphics][font]") {
    auto const& f = xlFontInfo::FindFont(16);
    RecordingTVA a;
    RecordingTVA b;
    f.populate(a, 0, 0, "W", 1.0f);
    f.populate(b, 0, 0, "W", 2.0f);
    REQUIRE(a.v.size() == 6);
    REQUIRE(b.v.size() == 6);
    float wa = a.v[2].x - a.v[0].x;
    float wb = b.v[2].x - b.v[0].x;
    CHECK_THAT(wb, WithinAbs(wa / 2, 1e-3));
}

// widthOf indexes widths[] without the emptiness guard populate() has.
TEST_CASE("xlFontInfo widthOf on an unloaded font must not crash", "[graphics][font][known-crash][.]") {
    xlFontInfo f;
    CHECK(f.widthOf("abc") == 0);
}

TEST_CASE("xlGraphicsCapability health follows the recorded GL version", "[misc][graphics][capability]") {
    auto& cap = xlGraphicsCapability::Instance();
    cap.RecordGL("4.6.0 Mesa", "llvmpipe", "Mesa");
    auto good = cap.Health();
    CHECK(good != xlGraphicsHealth::NoUsableGL);

    cap.RecordGL("1.4 Software", "GDI Generic", "Microsoft");
    auto bad = cap.Health();
    CHECK(bad != xlGraphicsHealth::OK);
    CHECK(bad != xlGraphicsHealth::Unknown);

    cap.RecordGL("4.1 Metal", "Apple", "Apple");
}

TEST_CASE("xlGraphicsCapability tolerates null and odd GL strings", "[misc][graphics][capability]") {
    auto& cap = xlGraphicsCapability::Instance();
    cap.RecordGL(nullptr, nullptr, nullptr);
    cap.RecordGL("", "", "");
    cap.RecordGL("x.y", "r", "v");
    cap.RecordGL("4.1", "Apple", "Apple");
    (void)cap.Health();
    SUCCEED();
}

TEST_CASE("xlGraphicsCapability probing is consistent and describes each adapter once", "[misc][graphics][capability]") {
    auto& cap = xlGraphicsCapability::Instance();
    auto const& adapters = cap.Adapters();
    auto desc = cap.DescribeAdapters();
    CHECK(desc.size() <= adapters.size());
    auto sorted = desc;
    std::sort(sorted.begin(), sorted.end());
    CHECK(std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end());
    for (auto const& d : desc) {
        CHECK(d.rfind("name=", 0) == 0);
        CHECK(d.find(" | pci=") != std::string::npos);
    }
    if (adapters.empty()) {
        CHECK(cap.HasHardwareAdapter());
    }
    CHECK(cap.IsRemoteSession() == cap.IsRemoteSession());
}

TEST_CASE("xlGraphicsCapability Record helpers do not throw", "[misc][graphics][capability]") {
    auto& cap = xlGraphicsCapability::Instance();
    cap.RecordVulkan("Fake GPU", 2, true);
    cap.RecordVulkan("llvmpipe", 4, false);
    cap.RecordNoVulkanDevice();
    SUCCEED();
}

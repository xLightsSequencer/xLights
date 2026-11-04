#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "support/TempDir.h"
#include "utils/VectorMath.h"
#include "utils/ZipUtils.h"
#include "utils/xlImage.h"

using Catch::Matchers::WithinAbs;

namespace {

uint32_t Crc32(const std::string& s) {
    uint32_t crc = 0xFFFFFFFFu;
    for (unsigned char c : s) {
        crc ^= c;
        for (int k = 0; k < 8; ++k) {
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
        }
    }
    return ~crc;
}

void Put16(std::string& o, uint32_t v) {
    o += static_cast<char>(v & 0xFF);
    o += static_cast<char>((v >> 8) & 0xFF);
}
void Put32(std::string& o, uint32_t v) {
    Put16(o, v & 0xFFFF);
    Put16(o, v >> 16);
}

std::string BuildStoredZip(const std::vector<std::pair<std::string, std::string>>& entries) {
    std::string out;
    std::string central;
    for (const auto& e : entries) {
        const uint32_t offset = static_cast<uint32_t>(out.size());
        const uint32_t crc = Crc32(e.second);
        Put32(out, 0x04034b50);
        Put16(out, 20);
        Put16(out, 0);
        Put16(out, 0);
        Put16(out, 0);
        Put16(out, 0x21);
        Put32(out, crc);
        Put32(out, static_cast<uint32_t>(e.second.size()));
        Put32(out, static_cast<uint32_t>(e.second.size()));
        Put16(out, static_cast<uint32_t>(e.first.size()));
        Put16(out, 0);
        out += e.first;
        out += e.second;

        Put32(central, 0x02014b50);
        Put16(central, 20);
        Put16(central, 20);
        Put16(central, 0);
        Put16(central, 0);
        Put16(central, 0);
        Put16(central, 0x21);
        Put32(central, crc);
        Put32(central, static_cast<uint32_t>(e.second.size()));
        Put32(central, static_cast<uint32_t>(e.second.size()));
        Put16(central, static_cast<uint32_t>(e.first.size()));
        Put16(central, 0);
        Put16(central, 0);
        Put16(central, 0);
        Put16(central, 0);
        Put32(central, 0);
        Put32(central, offset);
        central += e.first;
    }
    const uint32_t cdOffset = static_cast<uint32_t>(out.size());
    out += central;
    Put32(out, 0x06054b50);
    Put16(out, 0);
    Put16(out, 0);
    Put16(out, static_cast<uint32_t>(entries.size()));
    Put16(out, static_cast<uint32_t>(entries.size()));
    Put32(out, static_cast<uint32_t>(central.size()));
    Put32(out, cdOffset);
    Put16(out, 0);
    return out;
}

xlImage MakePattern(int w, int h) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.SetRGBA(x, y, static_cast<uint8_t>(x * 40 + 10), static_cast<uint8_t>(y * 50 + 20), static_cast<uint8_t>(x + y), static_cast<uint8_t>(255 - x - y));
        }
    }
    return img;
}

bool SameImage(const xlImage& a, const xlImage& b) {
    if (a.GetWidth() != b.GetWidth() || a.GetHeight() != b.GetHeight()) return false;
    return std::memcmp(a.GetData(), b.GetData(), static_cast<size_t>(a.GetWidth()) * a.GetHeight() * 4) == 0;
}

std::vector<uint8_t> Bytes(const std::string& s) {
    return std::vector<uint8_t>(s.begin(), s.end());
}

std::string TinyGif() {
    std::string g = "GIF89a";
    Put16(g, 1);
    Put16(g, 1);
    g += static_cast<char>(0x80);
    g += '\0';
    g += '\0';
    g += std::string("\xFF\x00\x00\x00\x00\x00", 6);
    return g;
}

void AppendGifFrame(std::string& g, int delayCs, const std::string& lzw) {
    g += std::string("\x21\xF9\x04\x00", 4);
    Put16(g, static_cast<uint32_t>(delayCs));
    g += std::string("\x00\x00", 2);
    g += '\x2C';
    Put16(g, 0);
    Put16(g, 0);
    Put16(g, 1);
    Put16(g, 1);
    g += '\0';
    g += '\x02';
    g += static_cast<char>(lzw.size());
    g += lzw;
    g += '\0';
}

} // namespace

TEST_CASE("ZipUtils reads entries from a stored archive", "[utils]") {
    TempDir d;
    const std::string zip = d.Write("t.zip", BuildStoredZip({ { "first.txt", "hello" }, { "dir/second.bin", std::string("a\0b", 3) } }));

    auto first = ZipUtils::ReadFirstEntry(zip);
    REQUIRE(first.has_value());
    CHECK(*first == "hello");

    auto second = ZipUtils::ReadEntry(zip, "dir/second.bin");
    REQUIRE(second.has_value());
    CHECK(*second == std::string("a\0b", 3));

    CHECK(ZipUtils::ListEntries(zip) == std::vector<std::string>{ "first.txt", "dir/second.bin" });
}

TEST_CASE("ZipUtils reports missing entries and archives", "[utils]") {
    TempDir d;
    const std::string zip = d.Write("t.zip", BuildStoredZip({ { "a.txt", "x" } }));
    CHECK_FALSE(ZipUtils::ReadEntry(zip, "nope.txt").has_value());
    CHECK_FALSE(ZipUtils::ReadEntry(d.File("absent.zip"), "a.txt").has_value());
    CHECK_FALSE(ZipUtils::ReadFirstEntry(d.File("absent.zip")).has_value());
    CHECK(ZipUtils::ListEntries(d.File("absent.zip")).empty());

    const std::string bad = d.Write("bad.zip", "this is not a zip file");
    CHECK_FALSE(ZipUtils::ReadFirstEntry(bad).has_value());
    CHECK(ZipUtils::ListEntries(bad).empty());
}

TEST_CASE("ZipUtils reads an empty and a large entry", "[utils]") {
    TempDir d;
    const std::string big(100000, 'z');
    const std::string zip = d.Write("t.zip", BuildStoredZip({ { "empty", "" }, { "big", big } }));
    auto e = ZipUtils::ReadEntry(zip, "empty");
    REQUIRE(e.has_value());
    CHECK(e->empty());
    auto b = ZipUtils::ReadEntry(zip, "big");
    REQUIRE(b.has_value());
    CHECK(*b == big);
}

TEST_CASE("xlImage default state and construction", "[utils]") {
    xlImage none;
    CHECK_FALSE(none.IsOk());
    CHECK(none.GetWidth() == 0);
    CHECK(none.GetData() == nullptr);

    xlImage blank(3, 2);
    CHECK(blank.IsOk());
    CHECK(blank.GetWidth() == 3);
    CHECK(blank.GetHeight() == 2);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 3; ++x) {
            CHECK(blank.GetRed(x, y) == 0);
            CHECK(blank.GetAlpha(x, y) == 0);
            CHECK(blank.IsTransparent(x, y));
        }
    CHECK_FALSE(xlImage(0, 5).IsOk());
    CHECK_FALSE(xlImage(-1, 5).IsOk());
}

TEST_CASE("xlImage pixel setters and getters", "[utils]") {
    xlImage img(2, 2);
    img.SetRGBA(1, 0, 10, 20, 30, 40);
    CHECK(img.GetRed(1, 0) == 10);
    CHECK(img.GetGreen(1, 0) == 20);
    CHECK(img.GetBlue(1, 0) == 30);
    CHECK(img.GetAlpha(1, 0) == 40);
    img.SetRGB(0, 1, 1, 2, 3);
    CHECK(img.GetBlue(0, 1) == 3);
    CHECK(img.GetAlpha(0, 1) == 0);
    img.SetAlpha(0, 1, 200);
    CHECK(img.GetAlpha(0, 1) == 200);
    CHECK_FALSE(img.IsTransparent(0, 1));
    CHECK(img.IsTransparent(0, 1, 200));
    img.Clear();
    CHECK(img.GetBlue(0, 1) == 0);
    CHECK(img.GetAlpha(1, 0) == 0);
}

TEST_CASE("xlImage copy is deep and move leaves source empty", "[utils]") {
    xlImage a = MakePattern(3, 3);
    xlImage b(a);
    b.SetRGBA(0, 0, 1, 1, 1, 1);
    CHECK(a.GetRed(0, 0) == 10);
    CHECK(b.GetRed(0, 0) == 1);

    xlImage c;
    c = a;
    CHECK(SameImage(a, c));
    xlImage m(std::move(c));
    CHECK_FALSE(c.IsOk());
    CHECK(SameImage(a, m));
    xlImage n;
    n = std::move(m);
    CHECK_FALSE(m.IsOk());
    CHECK(SameImage(a, n));
    CHECK(SameImage(a, a.Copy()));
}

TEST_CASE("xlImage Create reallocates and zeroes", "[utils]") {
    xlImage img = MakePattern(2, 2);
    img.Create(4, 1);
    CHECK(img.GetWidth() == 4);
    CHECK(img.GetHeight() == 1);
    CHECK(img.GetRed(3, 0) == 0);
    img.Create(0, 0);
    CHECK_FALSE(img.IsOk());
}

TEST_CASE("xlImage PNG round trip is lossless", "[utils]") {
    xlImage src = MakePattern(5, 4);
    std::vector<uint8_t> png;
    REQUIRE(src.SaveAsPNG(png));
    REQUIRE(png.size() > 8);
    CHECK(png[1] == 'P');
    CHECK(png[2] == 'N');
    CHECK(png[3] == 'G');

    xlImage back;
    REQUIRE(back.LoadFromMemory(png.data(), png.size()));
    CHECK(SameImage(src, back));
}

TEST_CASE("xlImage LoadFromFile reads a saved PNG", "[utils]") {
    TempDir d;
    xlImage src = MakePattern(3, 2);
    std::vector<uint8_t> png;
    REQUIRE(src.SaveAsPNG(png));
    const std::string path = d.Write("img.png", std::string(png.begin(), png.end()));

    xlImage back;
    REQUIRE(back.LoadFromFile(path));
    CHECK(SameImage(src, back));
}

TEST_CASE("xlImage loading failures", "[utils]") {
    TempDir d;
    xlImage img;
    CHECK_FALSE(img.LoadFromFile(d.File("missing.png")));
    CHECK_FALSE(img.LoadFromFile(d.Write("empty.png", "")));
    CHECK_FALSE(img.LoadFromFile(d.Write("junk.png", "definitely not an image")));
    const auto junk = Bytes("junk");
    CHECK_FALSE(img.LoadFromMemory(junk.data(), junk.size()));
    CHECK_FALSE(img.IsOk());

    std::vector<uint8_t> out;
    CHECK_FALSE(xlImage().SaveAsPNG(out));
}

TEST_CASE("xlImage Rescale changes dimensions and keeps flat colour", "[utils]") {
    xlImage img(2, 2);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x) img.SetRGBA(x, y, 100, 150, 200, 255);
    img.Rescale(8, 6);
    CHECK(img.GetWidth() == 8);
    CHECK(img.GetHeight() == 6);
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 8; ++x) {
            CHECK(std::abs(img.GetRed(x, y) - 100) <= 1);
            CHECK(std::abs(img.GetGreen(x, y) - 150) <= 1);
            CHECK(std::abs(img.GetBlue(x, y) - 200) <= 1);
            CHECK(img.GetAlpha(x, y) == 255);
        }

    xlImage same = MakePattern(3, 3);
    xlImage ref = same;
    same.Rescale(3, 3);
    CHECK(SameImage(same, ref));
    same.Rescale(0, 5);
    CHECK(same.GetWidth() == 3);
}

TEST_CASE("xlImage Rescale of a large image takes the threaded path", "[utils]") {
    xlImage img(600, 4);
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 600; ++x) img.SetRGBA(x, y, 50, 60, 70, 255);
    img.Rescale(300, 2);
    CHECK(img.GetWidth() == 300);
    CHECK(img.GetHeight() == 2);
    CHECK(std::abs(img.GetRed(150, 1) - 50) <= 1);
    CHECK(std::abs(img.GetBlue(299, 0) - 70) <= 1);
}

TEST_CASE("xlImage mirror and rotate", "[utils]") {
    xlImage src = MakePattern(3, 2);

    xlImage h = src.Mirror(true);
    CHECK(h.GetWidth() == 3);
    CHECK(h.GetRed(0, 0) == src.GetRed(2, 0));
    CHECK(h.GetGreen(2, 1) == src.GetGreen(0, 1));
    CHECK(SameImage(h.Mirror(true), src));

    xlImage v = src.Mirror(false);
    CHECK(v.GetRed(1, 0) == src.GetRed(1, 1));
    CHECK(SameImage(v.Mirror(false), src));

    xlImage r180 = src.Rotate180();
    CHECK(r180.GetRed(0, 0) == src.GetRed(2, 1));
    CHECK(SameImage(r180.Rotate180(), src));
    CHECK(SameImage(r180, src.Mirror(true).Mirror(false)));
}

TEST_CASE("xlImage Rotate90 swaps dimensions and inverts", "[utils]") {
    xlImage src = MakePattern(3, 2);
    xlImage cw = src.Rotate90(true);
    CHECK(cw.GetWidth() == 2);
    CHECK(cw.GetHeight() == 3);
    CHECK(cw.GetRed(1, 0) == src.GetRed(0, 0));
    CHECK(cw.GetRed(0, 0) == src.GetRed(0, 1));
    CHECK(cw.GetRed(1, 2) == src.GetRed(2, 0));

    xlImage ccw = src.Rotate90(false);
    CHECK(ccw.GetRed(0, 2) == src.GetRed(0, 0));
    CHECK(SameImage(cw.Rotate90(false), src));
    CHECK(SameImage(ccw.Rotate90(true), src));
    CHECK(SameImage(cw.Rotate90(true), src.Rotate180()));
}

TEST_CASE("xlImage ApplyOrientation follows EXIF codes", "[utils]") {
    xlImage src = MakePattern(3, 2);
    CHECK(SameImage(src.ApplyOrientation(1), src));
    CHECK(SameImage(src.ApplyOrientation(0), src));
    CHECK(SameImage(src.ApplyOrientation(99), src));
    CHECK(SameImage(src.ApplyOrientation(2), src.Mirror(true)));
    CHECK(SameImage(src.ApplyOrientation(3), src.Rotate180()));
    CHECK(SameImage(src.ApplyOrientation(4), src.Mirror(false)));
    CHECK(SameImage(src.ApplyOrientation(6), src.Rotate90(true)));
    CHECK(SameImage(src.ApplyOrientation(8), src.Rotate90(false)));
    CHECK(src.ApplyOrientation(5).GetWidth() == 2);
    CHECK(src.ApplyOrientation(7).GetHeight() == 3);
    CHECK_FALSE(xlImage().Mirror(true).IsOk());
    CHECK_FALSE(xlImage().Rotate90(true).IsOk());
}

TEST_CASE("xlImage builds from XPM data", "[utils]") {
    static const char* const xpm[] = {
        "3 2 4 1",
        "a c #FF0000",
        "b c #0f0",
        "c c None",
        "d c blue",
        "abc",
        "dda"
    };
    xlImage img(xpm);
    REQUIRE(img.IsOk());
    CHECK(img.GetWidth() == 3);
    CHECK(img.GetHeight() == 2);
    CHECK(img.GetRed(0, 0) == 255);
    CHECK(img.GetAlpha(0, 0) == 255);
    CHECK(img.GetGreen(1, 0) == 255);
    CHECK(img.GetRed(1, 0) == 0);
    CHECK(img.GetAlpha(2, 0) == 0);
    CHECK(img.GetBlue(0, 1) == 255);
    CHECK(img.GetBlue(1, 1) == 255);
    CHECK(img.GetRed(2, 1) == 255);
}

TEST_CASE("xlImage rejects malformed XPM", "[utils]") {
    static const char* const bad[] = { "0 0 0 0" };
    CHECK_FALSE(xlImage(bad).IsOk());
    CHECK_FALSE(xlImage(static_cast<const char* const*>(nullptr)).IsOk());
}

TEST_CASE("LoadAnimatedGIFFromMemory decodes a one pixel frame", "[utils]") {
    std::string g = TinyGif();
    AppendGifFrame(g, 10, std::string("\x44\x01", 2));
    g += '\x3B';

    AnimatedImageData a = LoadAnimatedGIFFromMemory(reinterpret_cast<const uint8_t*>(g.data()), g.size());
    REQUIRE(a.frames.size() == 1);
    CHECK(a.width == 1);
    CHECK(a.height == 1);
    CHECK(a.frameTimes[0] == 100);
    CHECK(a.frames[0].GetRed(0, 0) == 255);
    CHECK(a.frames[0].GetGreen(0, 0) == 0);
    CHECK(a.frames[0].GetAlpha(0, 0) == 255);
}

TEST_CASE("LoadAnimatedGIFFromMemory decodes several frames with delays", "[utils]") {
    std::string g = TinyGif();
    AppendGifFrame(g, 10, std::string("\x44\x01", 2));
    AppendGifFrame(g, 25, std::string("\x4C\x01", 2));
    g += '\x3B';

    AnimatedImageData a = LoadAnimatedGIFFromMemory(reinterpret_cast<const uint8_t*>(g.data()), g.size());
    REQUIRE(a.frames.size() == 2);
    REQUIRE(a.frameTimes.size() == 2);
    CHECK(a.frameTimes[0] == 100);
    CHECK(a.frameTimes[1] == 250);
    CHECK(a.frames[0].GetRed(0, 0) == 255);
    CHECK(a.frames[1].GetRed(0, 0) == 0);
    CHECK(a.frames[1].GetAlpha(0, 0) == 255);
}

TEST_CASE("LoadAnimatedGIFFromMemory rejects bad input", "[utils]") {
    CHECK(LoadAnimatedGIFFromMemory(nullptr, 0).frames.empty());
    const auto notGif = Bytes("PNG is not a gif at all");
    CHECK(LoadAnimatedGIFFromMemory(notGif.data(), notGif.size()).frames.empty());
    const std::string headerOnly = TinyGif() + '\x3B';
    CHECK(LoadAnimatedGIFFromMemory(reinterpret_cast<const uint8_t*>(headerOnly.data()), headerOnly.size()).frames.empty());
}

TEST_CASE("GIF bytes are also readable through xlImage::LoadFromMemory", "[utils]") {
    std::string g = TinyGif();
    AppendGifFrame(g, 10, std::string("\x44\x01", 2));
    g += '\x3B';
    xlImage img;
    REQUIRE(img.LoadFromMemory(reinterpret_cast<const uint8_t*>(g.data()), g.size()));
    CHECK(img.GetWidth() == 1);
    CHECK(img.GetRed(0, 0) == 255);
}

TEST_CASE("VectorMath GetPlaneIntersect", "[utils]") {
    glm::vec3 hit;
    REQUIRE(VectorMath::GetPlaneIntersect({ 0, 0, 5 }, { 0, 0, -1 }, { 0, 0, 0 }, { 0, 0, 1 }, hit));
    CHECK_THAT(hit.z, WithinAbs(0.0, 1e-5));
    REQUIRE(VectorMath::GetPlaneIntersect({ 1, 2, 10 }, { 0, 0, -2 }, { 0, 0, 3 }, { 0, 0, 1 }, hit));
    CHECK_THAT(hit.x, WithinAbs(1.0, 1e-5));
    CHECK_THAT(hit.y, WithinAbs(2.0, 1e-5));
    CHECK_THAT(hit.z, WithinAbs(3.0, 1e-5));
    CHECK_FALSE(VectorMath::GetPlaneIntersect({ 0, 0, 5 }, { 1, 0, 0 }, { 0, 0, 0 }, { 0, 0, 1 }, hit));
}

TEST_CASE("VectorMath ray against a box", "[utils]") {
    const glm::vec3 mn(-0.5f), mx(0.5f);
    const glm::mat4 model(1.0f);
    float dist = -1.0f;
    CHECK(VectorMath::TestRayOBBIntersection({ 0, 0, -10 }, { 0, 0, 1 }, mn, mx, model, dist));
    CHECK_THAT(dist, WithinAbs(9.5, 1e-4));

    CHECK_FALSE(VectorMath::TestRayOBBIntersection({ 5, 0, -10 }, { 0, 0, 1 }, mn, mx, model, dist));
    CHECK_FALSE(VectorMath::TestRayOBBIntersection({ 0, 0, -10 }, { 0, 0, -1 }, mn, mx, model, dist));

    const glm::mat4 moved = glm::translate(glm::mat4(1.0f), glm::vec3(5, 0, 0));
    CHECK_FALSE(VectorMath::TestRayOBBIntersection({ 0, 0, -10 }, { 0, 0, 1 }, mn, mx, moved, dist));
    CHECK(VectorMath::TestRayOBBIntersection({ 5, 0, -10 }, { 0, 0, 1 }, mn, mx, moved, dist));
    CHECK_THAT(dist, WithinAbs(9.5, 1e-4));
}

TEST_CASE("VectorMath ray against a rotated box", "[utils]") {
    const glm::vec3 mn(-2.0f, -0.5f, -0.5f), mx(2.0f, 0.5f, 0.5f);
    float dist = 0.0f;
    CHECK(VectorMath::TestRayOBBIntersection({ 1.5f, 0, -10 }, { 0, 0, 1 }, mn, mx, glm::mat4(1.0f), dist));
    const glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
    CHECK_FALSE(VectorMath::TestRayOBBIntersection({ 1.5f, 0, -10 }, { 0, 0, 1 }, mn, mx, rot, dist));
    CHECK(VectorMath::TestRayOBBIntersection({ 0, 1.5f, -10 }, { 0, 0, 1 }, mn, mx, rot, dist));
}

TEST_CASE("VectorMath 2D box point test", "[utils]") {
    const glm::vec3 mn(-1, -1, 0), mx(1, 1, 0);
    CHECK(VectorMath::TestRayOBBIntersection2D({ 0, 0, 0 }, mn, mx, glm::mat4(1.0f)));
    CHECK(VectorMath::TestRayOBBIntersection2D({ 0.9f, -0.9f, 0 }, mn, mx, glm::mat4(1.0f)));
    CHECK_FALSE(VectorMath::TestRayOBBIntersection2D({ 1.5f, 0, 0 }, mn, mx, glm::mat4(1.0f)));
    CHECK_FALSE(VectorMath::TestRayOBBIntersection2D({ 0, -1.5f, 0 }, mn, mx, glm::mat4(1.0f)));
    const glm::mat4 moved = glm::translate(glm::mat4(1.0f), glm::vec3(10, 10, 0));
    CHECK(VectorMath::TestRayOBBIntersection2D({ 10, 10, 0 }, mn, mx, moved));
    CHECK_FALSE(VectorMath::TestRayOBBIntersection2D({ 0, 0, 0 }, mn, mx, moved));
}

TEST_CASE("VectorMath screen coordinate mapping", "[utils]") {
    const glm::mat4 id(1.0f);
    glm::vec2 c = VectorMath::GetScreenCoord(200, 100, { 0, 0, 0 }, id, id);
    CHECK_THAT(c.x, WithinAbs(100.0, 1e-3));
    CHECK_THAT(c.y, WithinAbs(50.0, 1e-3));
    glm::vec2 tr = VectorMath::GetScreenCoord(200, 100, { 1, 1, 0 }, id, id);
    CHECK_THAT(tr.x, WithinAbs(200.0, 1e-3));
    CHECK_THAT(tr.y, WithinAbs(0.0, 1e-3));
    glm::vec2 bl = VectorMath::GetScreenCoord(200, 100, { -1, -1, 0 }, id, id);
    CHECK_THAT(bl.x, WithinAbs(0.0, 1e-3));
    CHECK_THAT(bl.y, WithinAbs(100.0, 1e-3));

    const glm::mat4 shifted = glm::translate(id, glm::vec3(0.5f, 0, 0));
    glm::vec2 s = VectorMath::GetScreenCoord(200, 100, { 0, 0, 0 }, id, shifted);
    CHECK_THAT(s.x, WithinAbs(150.0, 1e-3));
}

TEST_CASE("VectorMath ScreenPosToWorldRay through the centre", "[utils]") {
    const glm::mat4 proj = glm::perspective(glm::radians(60.0f), 2.0f, 0.1f, 100.0f);
    const glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::vec3 origin, dir;
    VectorMath::ScreenPosToWorldRay(100, 50, 200, 100, proj * view, origin, dir);
    CHECK_THAT(glm::length(dir), WithinAbs(1.0, 1e-4));
    CHECK_THAT(dir.x, WithinAbs(0.0, 1e-4));
    CHECK_THAT(dir.y, WithinAbs(0.0, 1e-4));
    CHECK_THAT(dir.z, WithinAbs(-1.0, 1e-4));
    CHECK_THAT(origin.z, WithinAbs(4.9, 1e-2));
}

TEST_CASE("VectorMath volume selection contains the projected box", "[utils]") {
    const glm::mat4 id(1.0f);
    const glm::vec3 mn(-0.5f, -0.5f, 0), mx(0.5f, 0.5f, 0);
    CHECK(VectorMath::TestVolumeOBBIntersection(0, 0, 200, 100, 200, 100, mn, mx, id, id));
    CHECK_FALSE(VectorMath::TestVolumeOBBIntersection(0, 0, 100, 100, 200, 100, mn, mx, id, id));
    CHECK_FALSE(VectorMath::TestVolumeOBBIntersection(120, 0, 200, 100, 200, 100, mn, mx, id, id));
}

TEST_CASE("VectorMath rotationBetweenVectors rotates start onto dest for quarter turns", "[utils]") {
    const glm::vec3 pairs[][2] = { { { 1, 0, 0 }, { 0, 1, 0 } }, { { 1, 0, 0 }, { 0, 0, 1 } }, { { 0, 0, 2 }, { 3, 0, 0 } } };
    for (const auto& p : pairs) {
        glm::quat q = VectorMath::rotationBetweenVectors(p[0], p[1]);
        glm::vec3 r = q * glm::normalize(p[0]);
        glm::vec3 want = glm::normalize(p[1]);
        CHECK_THAT(r.x, WithinAbs(want.x, 1e-4));
        CHECK_THAT(r.y, WithinAbs(want.y, 1e-4));
        CHECK_THAT(r.z, WithinAbs(want.z, 1e-4));
    }
}

TEST_CASE("VectorMath rotationBetweenVectors rotates start onto dest for any angle", "[utils]") {
    // xyz is built from the normalised axis times 1/s instead of the unnormalised cross
    // product, so the result is only right when the angle is exactly 90 degrees.
    const glm::vec3 pairs[][2] = { { { 0, 1, 0 }, { 1, 1, 1 } }, { { 1, 2, 3 }, { -3, 2, 1 } } };
    for (const auto& p : pairs) {
        glm::quat q = VectorMath::rotationBetweenVectors(p[0], p[1]);
        glm::vec3 r = q * glm::normalize(p[0]);
        glm::vec3 want = glm::normalize(p[1]);
        CHECK_THAT(r.x, WithinAbs(want.x, 1e-4));
        CHECK_THAT(r.y, WithinAbs(want.y, 1e-4));
        CHECK_THAT(r.z, WithinAbs(want.z, 1e-4));
        CHECK_THAT(glm::length(q), WithinAbs(1.0, 1e-4));
    }
}

TEST_CASE("VectorMath rotationBetweenVectors parallel vectors give identity", "[utils]") {
    glm::quat q = VectorMath::rotationBetweenVectors({ 2, 0, 0 }, { 5, 0, 0 });
    CHECK_THAT(q.w, WithinAbs(1.0, 1e-6));
    CHECK_THAT(q.x, WithinAbs(0.0, 1e-6));
}

TEST_CASE("VectorMath rotationBetweenVectors handles opposite vectors", "[utils]") {
    // The 180 degree branch builds glm::quat(180.0f, axis), which is a (w, xyz) constructor
    // and not an angle/axis one, so the result is neither unit length nor a half turn.
    glm::quat q = VectorMath::rotationBetweenVectors({ 1, 0, 0 }, { -1, 0, 0 });
    CHECK_THAT(glm::length(q), WithinAbs(1.0, 1e-4));
    glm::vec3 r = q * glm::vec3(1, 0, 0);
    CHECK_THAT(r.x, WithinAbs(-1.0, 1e-3));

    glm::quat q2 = VectorMath::rotationBetweenVectors({ 1, 2, 3 }, { -2, -4, -6 });
    CHECK_THAT(glm::length(q2), WithinAbs(1.0, 1e-4));
    glm::vec3 r2 = q2 * glm::normalize(glm::vec3(1, 2, 3));
    glm::vec3 want2 = glm::normalize(glm::vec3(-1, -2, -3));
    CHECK_THAT(r2.x, WithinAbs(want2.x, 1e-4));
    CHECK_THAT(r2.y, WithinAbs(want2.y, 1e-4));
    CHECK_THAT(r2.z, WithinAbs(want2.z, 1e-4));
}

TEST_CASE("VectorMath rotation matrices from the X axis", "[utils]") {
    const glm::vec3 targets[] = { { 0, 1, 0 }, { 0, 0, 1 }, { 1, 1, 0 }, { 1, 2, 3 }, { 0, -1, 0 } };
    for (const auto& t : targets) {
        glm::mat4 m = VectorMath::rotationMatrixFromXAxisToVector(t);
        glm::vec3 r = glm::vec3(m * glm::vec4(1, 0, 0, 0));
        glm::vec3 want = glm::normalize(t);
        CHECK_THAT(r.x, WithinAbs(want.x, 1e-4));
        CHECK_THAT(r.y, WithinAbs(want.y, 1e-4));
        CHECK_THAT(r.z, WithinAbs(want.z, 1e-4));
    }
}

TEST_CASE("VectorMath rotMatrixFromXAxisToVector quarter turns", "[utils]") {
    const glm::vec3 targets[] = { { 0, 1, 0 }, { 0, 0, 1 } };
    for (const auto& t : targets) {
        glm::mat4 m = VectorMath::rotMatrixFromXAxisToVector(t);
        glm::vec3 r = glm::vec3(m * glm::vec4(1, 0, 0, 0));
        CHECK_THAT(r.x, WithinAbs(t.x, 1e-4));
        CHECK_THAT(r.y, WithinAbs(t.y, 1e-4));
        CHECK_THAT(r.z, WithinAbs(t.z, 1e-4));
    }
}

TEST_CASE("VectorMath rotMatrixFromXAxisToVector arbitrary and opposite directions", "[utils]") {
    // Inherits the rotationBetweenVectors problems (wrong half angle, bad 180 degree quaternion).
    const glm::vec3 targets[] = { { 1, 1, 0 }, { 1, 2, 3 }, { -1, 0, 0 } };
    for (const auto& t : targets) {
        glm::mat4 m = VectorMath::rotMatrixFromXAxisToVector(t);
        glm::vec3 r = glm::vec3(m * glm::vec4(1, 0, 0, 0));
        glm::vec3 want = glm::normalize(t);
        CHECK_THAT(r.x, WithinAbs(want.x, 1e-4));
        CHECK_THAT(r.y, WithinAbs(want.y, 1e-4));
        CHECK_THAT(r.z, WithinAbs(want.z, 1e-4));
    }
}

TEST_CASE("VectorMath X axis special cases", "[utils]") {
    glm::mat4 same = VectorMath::rotationMatrixFromXAxisToVector({ 3, 0, 0 });
    CHECK_THAT(same[0][0], WithinAbs(1.0, 1e-6));
    CHECK_THAT(same[1][1], WithinAbs(1.0, 1e-6));

    glm::mat4 flip = VectorMath::rotationMatrixFromXAxisToVector({ -3, 0, 0 });
    glm::vec3 r = glm::vec3(flip * glm::vec4(1, 0, 0, 0));
    CHECK_THAT(r.x, WithinAbs(-1.0, 1e-4));
}

TEST_CASE("VectorMath rotationMatrixFromXAxisToVector2 uses the segment direction", "[utils]") {
    glm::mat4 m = VectorMath::rotationMatrixFromXAxisToVector2({ 1, 1, 1 }, { 1, 3, 1 });
    glm::vec3 r = glm::vec3(m * glm::vec4(1, 0, 0, 0));
    CHECK_THAT(r.x, WithinAbs(0.0, 1e-4));
    CHECK_THAT(r.y, WithinAbs(1.0, 1e-4));

    glm::mat4 same = VectorMath::rotationMatrixFromXAxisToVector2({ 0, 0, 0 }, { 5, 0, 0 });
    CHECK_THAT(same[0][0], WithinAbs(1.0, 1e-6));
    glm::mat4 back = VectorMath::rotationMatrixFromXAxisToVector2({ 5, 0, 0 }, { 0, 0, 0 });
    glm::vec3 rb = glm::vec3(back * glm::vec4(1, 0, 0, 0));
    CHECK_THAT(rb.x, WithinAbs(-1.0, 1e-4));
}

TEST_CASE("VectorMath rotationMatrixBetweenVectors maps b direction onto a", "[utils]") {
    const glm::vec3 a(0, 1, 0), b(1, 0, 0);
    glm::mat4 m = VectorMath::rotationMatrixBetweenVectors(a, b);
    glm::vec3 r = glm::vec3(m * glm::vec4(b, 0));
    CHECK_THAT(glm::length(r), WithinAbs(1.0, 1e-4));
    const float dotWithA = glm::dot(glm::normalize(r), a);
    const float dotWithNegA = glm::dot(glm::normalize(r), -a);
    CHECK((dotWithA > 0.999f || dotWithNegA > 0.999f));
}

#include <catch2/catch_test_macros.hpp>

#include "render/SequenceMedia.h"
#include "support/TempDir.h"
#include "utils/Base64.h"
#include "utils/FileUtils.h"
#include "utils/JobPool.h"
#include "utils/xlImage.h"

#include <pugixml.hpp>

#include <chrono>
#include <filesystem>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct FixFileGuard {
    explicit FixFileGuard(const std::string& dir) { FileUtils::SetFixFileShowDir(dir); }
    ~FixFileGuard() { FileUtils::SetFixFileShowDir(""); }
};

std::string B64(const std::string& s) {
    return Base64::Encode(reinterpret_cast<const uint8_t*>(s.data()), s.size());
}

std::string Png(int w, int h, uint8_t r = 10) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            img.SetRGBA(x, y, r, static_cast<uint8_t>(x * 10), static_cast<uint8_t>(y * 10), 255);
    std::vector<uint8_t> out;
    img.SaveAsPNG(out);
    return std::string(out.begin(), out.end());
}

std::vector<uint8_t> Hex(const std::string& h) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i + 1 < h.size(); i += 2) out.push_back(static_cast<uint8_t>(std::strtol(h.substr(i, 2).c_str(), nullptr, 16)));
    return out;
}

// A 4x2 baseline JPEG (no orientation tag).
const std::vector<uint8_t>& Jpeg4x2() {
    static const std::vector<uint8_t> j = Hex(
        "ffd8ffe000104a46494600010100004800480000ffe1004c4578696600004d4d002a00000008000187690004000000010000001a000000000003a001"
        "00030000000100010000a00200040000000100000004a0030004000000010000000200000000ffed003850686f746f73686f7020332e30003842494d"
        "04040000000000003842494d0425000000000010d41d8cd98f00b204e9800998ecf8427effc00011080002000403012200021101031101ffc4001f00"
        "00010501010101010100000000000000000102030405060708090a0bffc400b5100002010303020403050504040000017d0102030004110512213141"
        "0613516107227114328191a1082342b1c11552d1f02433627282090a161718191a25262728292a3435363738393a434445464748494a535455565758"
        "595a636465666768696a737475767778797a838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8"
        "c9cad2d3d4d5d6d7d8d9dae1e2e3e4e5e6e7e8e9eaf1f2f3f4f5f6f7f8f9faffc4001f01000301010101010101010100000000000001020304050607"
        "08090a0bffc400b51100020102040403040705040400010277000102031104052131061241510761711322328108144291a1b1c109233352f0156272"
        "d10a162434e125f11718191a262728292a35363738393a434445464748494a535455565758595a636465666768696a737475767778797a8283848586"
        "8788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae2e3e4e5e6e7e8e9eaf2f3"
        "f4f5f6f7f8f9faffdb0043000606060606060a06060a0e0a0a0a0e120e0e0e0e12171212121212171c1717171717171c1c1c1c1c1c1c1c2222222222"
        "2227272727272c2c2c2c2c2c2c2c2c2cffdb0043010707070b0a0b130a0a132e1f1a1f2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e"
        "2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2e2effdd00040001ffda000c03010002110311003f00e4e8a28af08fd58fffd9");
    return j;
}

// The JPEG above with an APP1 Exif segment carrying the given orientation inserted after SOI.
std::string JpegWithOrientation(int orientation, bool littleEndian, bool validTiff = true) {
    std::vector<uint8_t> tiff;
    auto p16 = [&](int v) {
        if (littleEndian) { tiff.push_back(v & 0xFF); tiff.push_back((v >> 8) & 0xFF); }
        else { tiff.push_back((v >> 8) & 0xFF); tiff.push_back(v & 0xFF); }
    };
    auto p32 = [&](uint32_t v) {
        if (littleEndian) { p16(v & 0xFFFF); p16(v >> 16); }
        else { p16(v >> 16); p16(v & 0xFFFF); }
    };
    tiff.push_back(littleEndian ? 'I' : 'M');
    tiff.push_back(littleEndian ? 'I' : 'M');
    p16(validTiff ? 42 : 43);
    p32(8);
    p16(2);                 // two IFD entries
    p16(0x010F); p16(2); p32(1); p32(0);       // Make, not orientation
    p16(0x0112); p16(3); p32(1); p16(orientation); p16(0);
    p32(0);
    std::vector<uint8_t> seg = { 'E', 'x', 'i', 'f', 0, 0 };
    seg.insert(seg.end(), tiff.begin(), tiff.end());
    const auto& j = Jpeg4x2();
    std::string out(j.begin(), j.begin() + 2);
    const size_t len = seg.size() + 2;
    out += static_cast<char>(0xFF);
    out += static_cast<char>(0xE1);
    out += static_cast<char>((len >> 8) & 0xFF);
    out += static_cast<char>(len & 0xFF);
    out.append(seg.begin(), seg.end());
    out.append(j.begin() + 2, j.end());
    return out;
}

std::string Svg(int w, int h) {
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" + std::to_string(w) + "\" height=\"" + std::to_string(h) +
           "\"><rect x=\"0\" y=\"0\" width=\"" + std::to_string(w) + "\" height=\"" + std::to_string(h) + "\" fill=\"#ff0000\"/></svg>";
}

void Touch(const fs::path& p, int minutes) {
    fs::last_write_time(p, fs::file_time_type::clock::now() + std::chrono::minutes(minutes));
}

pugi::xml_document SaveDoc(const SequenceMedia& m) {
    pugi::xml_document doc;
    auto root = doc.append_child("root");
    m.SaveToXml(root);
    return doc;
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Images: SVG, JPEG orientation, webp, animation timing
// ---------------------------------------------------------------------------------------------

TEST_CASE("An svg file loads as a vector image that redraws at any size", "[seq2][media]") {
    TempDir d;
    auto path = d.Write("shape.svg", "\xEF\xBB\xBF  \n" + Svg(20, 10));
    SequenceMedia m;
    auto e = m.GetImage(path);
    REQUIRE(e->IsOk());
    CHECK(e->IsVector());
    CHECK(e->GetImageWidth() == 20);
    CHECK(e->GetImageHeight() == 10);
    auto big = e->GetScaledImage(0, 80, 40, false);
    REQUIRE(big);
    CHECK(big->GetWidth() == 80);
    CHECK(big->GetHeight() == 40);
    CHECK(big->GetRed(40, 20) == 255);
    CHECK(big->GetGreen(40, 20) == 0);
    CHECK(e->GetScaledImage(0, 80, 40, false) == big);
    e->ClearScaledImageCache();
    CHECK(e->GetScaledImage(0, 80, 40, false) != big);

    e->GeneratePreview(10, 10);
    REQUIRE(e->GetPreviewFrameCount() == 1);
    CHECK(e->GetPreviewFrame(0)->GetWidth() == 10);
    CHECK(e->GetPreviewFrame(0)->GetHeight() == 5);
    CHECK(e->GetPreviewFrameTime(0) == 0);
    CHECK(e->GetPreviewFrame(3) == nullptr);
    CHECK(e->GetPreviewFrameTime(3) == 50);
}

TEST_CASE("A huge svg is capped when rasterized at its own size", "[seq2][media]") {
    SequenceMedia m;
    m.AddEmbeddedImage("big.svg", B64(Svg(8000, 4000)));
    auto e = m.GetImage("big.svg");
    REQUIRE(e->IsOk());
    CHECK(e->GetImageWidth() == 2048);
    CHECK(e->GetImageHeight() == 1024);
}

TEST_CASE("An svg with no size becomes a placeholder raster", "[seq2][media]") {
    SequenceMedia m;
    m.AddEmbeddedImage("empty.svg", B64("<svg xmlns=\"http://www.w3.org/2000/svg\"></svg>"));
    auto e = m.GetImage("empty.svg");
    CHECK_FALSE(e->IsVector());
    CHECK(e->GetImageWidth() == 5);
    CHECK(e->GetImageHeight() == 5);
}

TEST_CASE("JPEG EXIF orientation is applied on load", "[seq2][media]") {
    SequenceMedia m;
    const auto& plain = Jpeg4x2();
    m.AddEmbeddedImage("plain.jpg", Base64::Encode(plain.data(), plain.size()));
    CHECK(m.GetImage("plain.jpg")->GetImageWidth() == 4);
    CHECK(m.GetImage("plain.jpg")->GetImageHeight() == 2);

    for (bool le : { false, true }) {
        INFO("little endian " << le);
        SequenceMedia mm;
        mm.AddEmbeddedImage("r6.jpg", B64(JpegWithOrientation(6, le)));
        mm.AddEmbeddedImage("r8.jpg", B64(JpegWithOrientation(8, le)));
        mm.AddEmbeddedImage("r3.jpg", B64(JpegWithOrientation(3, le)));
        auto r6 = mm.GetImage("r6.jpg");
        REQUIRE(r6->IsOk());
        CHECK(r6->GetImageWidth() == 2);
        CHECK(r6->GetImageHeight() == 4);
        CHECK(mm.GetImage("r8.jpg")->GetImageWidth() == 2);
        CHECK(mm.GetImage("r3.jpg")->GetImageWidth() == 4);
    }
    // a TIFF header that is not 42 means no orientation
    SequenceMedia bad;
    bad.AddEmbeddedImage("bad.jpg", B64(JpegWithOrientation(6, true, false)));
    CHECK(bad.GetImage("bad.jpg")->GetImageWidth() == 4);
}

TEST_CASE("Undecodable image data becomes a placeholder", "[seq2][media]") {
    SequenceMedia m;
    m.AddEmbeddedImage("junk.png", B64(std::string("\xFF\xD8\xFF\xE1\x00\x04" "ab" "\xFF\xD9" " not really a jpeg", 26)));
    auto e = m.GetImage("junk.png");
    CHECK(e->GetImageWidth() == 5);
    CHECK(e->GetImageHeight() == 5);
}

TEST_CASE("WebP data goes through the registered loader", "[seq2][media]") {
    struct Restore {
        ~Restore() { ImageCacheEntry::SetWebPLoader(nullptr); }
    } restore;
    std::string webp = std::string("RIFF\x10\x00\x00\x00WEBPVP8 ", 16) + "payload";

    ImageCacheEntry::SetWebPLoader(nullptr);
    SequenceMedia none;
    none.AddEmbeddedImage("a.webp", B64(webp));
    CHECK_FALSE(none.GetImage("a.webp")->IsOk());

    int calls = 0;
    ImageCacheEntry::SetWebPLoader([&](const uint8_t* data, size_t len, const std::string& name) {
        ++calls;
        AnimatedImageData d;
        d.width = 3;
        d.height = 2;
        for (int i = 0; i < 3; ++i) {
            d.frames.emplace_back(3, 2);
            d.frameTimes.push_back(100 * (i + 1));
        }
        d.framesNoBG.emplace_back(3, 2);
        d.framesNoBG.emplace_back(3, 2);
        d.framesNoBG.emplace_back(3, 2);
        CHECK(len == webp.size());
        return d;
    });
    SequenceMedia m;
    m.AddEmbeddedImage("b.webp", B64(webp));
    auto e = m.GetImage("b.webp");
    CHECK(calls == 1);
    REQUIRE(e->IsOk());
    CHECK(e->GetImageCount() == 3);
    CHECK_FALSE(e->IsFrameBasedAnimation());
    CHECK(e->GetFrame(1, true) != e->GetFrame(1, false));
    // frame times 100, 200, 300 (total 600)
    CHECK(e->GetFrameForTime(0, false) == 0);
    CHECK(e->GetFrameForTime(99, false) == 0);
    CHECK(e->GetFrameForTime(100, false) == 1);
    CHECK(e->GetFrameForTime(350, false) == 2);
    CHECK(e->GetFrameForTime(600, false) == 2);
    CHECK(e->GetFrameForTime(900, false) == 2);
    CHECK(e->GetFrameForTime(650, true) == 0);
    CHECK(e->GetFrameForTime(750, true) == 1);
    CHECK(e->GetFrameForTime(1250, true) == 0);

    e->GeneratePreview(6, 6);
    CHECK(e->GetPreviewFrameCount() == 3);
    CHECK(e->GetPreviewFrame(0)->GetWidth() == 6);
    CHECK(e->GetPreviewFrame(0)->GetHeight() == 4);
    CHECK(e->GetPreviewFrameTime(2) == 300);
    // asking for the same size again keeps the cached preview
    auto first = e->GetPreviewFrame(0);
    e->GeneratePreview(6, 6);
    CHECK(e->GetPreviewFrame(0) == first);
}

TEST_CASE("A single frame animation has no running time", "[seq2][media]") {
    struct Restore {
        ~Restore() { ImageCacheEntry::SetWebPLoader(nullptr); }
    } restore;
    ImageCacheEntry::SetWebPLoader([](const uint8_t*, size_t, const std::string&) {
        AnimatedImageData d;
        d.width = 1;
        d.height = 1;
        d.frames.emplace_back(1, 1);
        d.frameTimes.push_back(500);
        return d;
    });
    SequenceMedia m;
    m.AddEmbeddedImage("one.webp", B64(std::string("RIFF\x04\x00\x00\x00WEBP", 12)));
    auto e = m.GetImage("one.webp");
    CHECK(e->GetImageCount() == 1);
    CHECK(e->IsFrameBasedAnimation());
    CHECK(e->GetFrameForTime(10000, true) == 0);
    CHECK(e->GetFrame(0, true) == e->GetFrame(0, false));
}

TEST_CASE("Unloading cached image data re-decodes embedded images but keeps frame-only ones", "[seq2][media]") {
    SequenceMedia m;
    m.AddEmbeddedImage("data.png", B64(Png(3, 3)));
    std::vector<xlImage> frames;
    frames.emplace_back(2, 2);
    frames.emplace_back(2, 2);
    m.AddEmbeddedImage("frames.gif", frames, 40);

    auto d = m.GetImage("data.png");
    d->GeneratePreview(3, 3);
    CHECK(d->HasPreview());
    d->UnloadCachedData();
    CHECK_FALSE(d->HasPreview());
    CHECK_FALSE(d->isLoaded());
    auto again = m.GetImage("data.png");
    CHECK(again->isLoaded());
    CHECK(again->GetFrame(0, false)->GetWidth() == 3);

    auto f = m.GetImage("frames.gif");
    f->UnloadCachedData();
    CHECK(f->GetImageCount() == 2);
    CHECK(f->GetFrame(1, false)->IsOk());
    CHECK(f->GetFrameForTime(45, true) == 1);
}

TEST_CASE("RemoveUnusedMedia drops unused external entries and keeps embedded and metadata ones", "[seq2][media]") {
    TempDir d;
    auto ext = d.Write("ext.png", Png(2, 2));
    auto meta = d.Write("meta.png", Png(2, 2));
    auto used = d.Write("used.png", Png(2, 2));
    auto txt = d.Write("notes.txt", "hello");
    SequenceMedia m;
    m.GetImage(ext);
    m.GetImage(meta);
    m.GetImage(used)->MarkIsUsed();
    m.GetTextFile(txt);
    m.AddEmbeddedImage("emb.png", B64(Png(2, 2)));
    m.MarkUsedByMetadata(meta);
    m.RemoveUnusedMedia();
    CHECK_FALSE(m.HasImage(ext));
    CHECK_FALSE(m.HasMedia(txt));
    CHECK(m.HasImage(meta));
    CHECK(m.HasImage(used));
    CHECK(m.HasImage("emb.png"));
    // embedded entry was only unloaded and decodes again on demand
    CHECK(m.GetImage("emb.png")->IsOk());

    m.MarkAllUnused();
    CHECK_FALSE(m.GetImage(used)->IsUsed());
}

TEST_CASE("PurgePreviewCaches clears every preview", "[seq2][media]") {
    TempDir d;
    auto svg = d.Write("a.svg", Svg(4, 4));
    SequenceMedia m;
    m.AddEmbeddedImage("i.png", B64(Png(4, 4)));
    auto img = m.GetImage("i.png");
    img->GeneratePreview(2, 2);
    auto s = m.GetSVG(svg);
    s->GeneratePreview(2, 2);
    m.GetTextFile(d.Write("t.txt", "x"));
    m.GetShader(d.Write("s.fs", "void main(){}"));
    m.GetBinaryFile(d.Write("g.gled", "bin"));
    m.GetVideo(d.Write("v.mp4", "vid"));
    REQUIRE(img->HasPreview());
    REQUIRE(s->HasPreview());
    m.PurgePreviewCaches();
    CHECK_FALSE(img->HasPreview());
    CHECK_FALSE(s->HasPreview());
}

TEST_CASE("Preview frames supplied by the caller are kept until cleared", "[seq2][media]") {
    TextMediaCacheEntry e("x.txt");
    e.GeneratePreview(4, 4); // base class: no preview
    CHECK_FALSE(e.HasPreview());
    std::vector<std::shared_ptr<xlImage>> frames{ std::make_shared<xlImage>(3, 2), std::make_shared<xlImage>(3, 2) };
    e.SetPreviewFrames(std::move(frames), 75);
    CHECK(e.GetPreviewFrameCount() == 2);
    CHECK(e.GetPreviewFrameTime(1) == 75);
    std::vector<std::shared_ptr<xlImage>> other{ std::make_shared<xlImage>(1, 1) };
    e.SetPreviewFrames(std::move(other), 10);
    CHECK(e.GetPreviewFrameCount() == 2);
    e.ClearPreview();
    CHECK_FALSE(e.HasPreview());
}

// ---------------------------------------------------------------------------------------------
// Text, SVG, shader, binary, video and audio entries
// ---------------------------------------------------------------------------------------------

TEST_CASE("Text files load from disk, embed and round trip through xml", "[seq2][media]") {
    TempDir d;
    auto path = d.Write("lyrics.txt", "line one\nline two");
    SequenceMedia a;
    auto t = a.GetTextFile(path);
    REQUIRE(t->isLoaded());
    CHECK(t->GetContent() == "line one\nline two");
    CHECK(a.GetTextFile(path) == t);
    CHECK(a.GetTextFile("") == nullptr);
    CHECK(a.GetMediaEmbedState(path) == std::pair<bool, bool>{ false, true });

    a.EmbedMedia(path);
    CHECK(a.GetMediaEmbedState(path).first);
    CHECK(a.FindEmbeddedKey(path) == path);
    auto doc = SaveDoc(a);
    auto node = doc.child("root").child("SequenceMedia").child("TextFile");
    REQUIRE(node);
    CHECK(std::string(node.attribute("path").as_string()) == path);

    fs::remove(path);
    SequenceMedia b;
    REQUIRE(b.LoadFromXml(doc.child("root").child("SequenceMedia")));
    auto tb = b.GetTextFile(path);
    REQUIRE(tb != nullptr);
    CHECK(tb->GetContent() == "line one\nline two");
    CHECK(tb->IsEmbedded());

    b.ExtractMedia(path);
    CHECK_FALSE(b.GetMediaEmbedState(path).first);
    CHECK(SaveDoc(b).child("root").child("SequenceMedia").child("TextFile").empty());
}

TEST_CASE("A missing text file is not loaded and an empty one stays unloaded", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto missing = m.GetTextFile(d.File("absent.txt"));
    REQUIRE(missing != nullptr);
    CHECK_FALSE(missing->isLoaded());
    CHECK(missing->GetContent().empty());
    auto empty = m.GetTextFile(d.Write("empty.txt", ""));
    CHECK_FALSE(empty->isLoaded());
}

TEST_CASE("Text files reload when changed on disk", "[seq2][media]") {
    TempDir d;
    auto path = d.Write("t.txt", "first");
    SequenceMedia m;
    CHECK(m.GetTextFile(path)->GetContent() == "first");
    d.Write("t.txt", "second version");
    Touch(path, 5);
    CHECK(m.GetTextFile(path)->GetContent() == "second version");
}

TEST_CASE("SVG entries give thumbnails that keep the aspect ratio", "[seq2][media]") {
    TempDir d;
    auto path = d.Write("s.svg", Svg(40, 20));
    SequenceMedia m;
    auto s = m.GetSVG(path);
    REQUIRE(s->isLoaded());
    CHECK(s->GetSVGContent().find("<svg") != std::string::npos);
    auto th = s->GetThumbnail(10, 10);
    REQUIRE(th);
    CHECK(th->GetWidth() == 10);
    CHECK(th->GetHeight() == 5);
    CHECK(s->GetThumbnail(10, 10) == th);
    CHECK(s->GetThumbnail(20, 20)->GetWidth() == 20);
    s->GeneratePreview(8, 8);
    CHECK(s->GetPreviewFrameCount() == 1);

    // changed on disk
    d.Write("s.svg", Svg(10, 30));
    Touch(path, 5);
    auto again = m.GetSVG(path);
    CHECK(again->GetSVGContent().find("height=\"30\"") != std::string::npos);
    CHECK_FALSE(again->HasPreview());
    auto th2 = again->GetThumbnail(10, 10);
    REQUIRE(th2);
    CHECK(th2->GetHeight() == 10);
    CHECK(th2->GetWidth() == 3);
}

TEST_CASE("SVG entries without usable content have no thumbnail", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    CHECK(m.GetSVG(d.File("none.svg"))->GetThumbnail(4, 4) == nullptr);
    auto bad = m.GetSVG(d.Write("bad.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\"/>"));
    CHECK(bad->GetThumbnail(4, 4) == nullptr);
    bad->GeneratePreview(4, 4);
    CHECK_FALSE(bad->HasPreview());
}

TEST_CASE("SVG, shader and binary entries round trip through xml when embedded", "[seq2][media]") {
    TempDir d;
    auto svg = d.Write("a.svg", Svg(4, 4));
    auto fs1 = d.Write("w.fs", "/*{}*/ void main() { gl_FragColor = vec4(1.0); }");
    SequenceMedia a;
    a.GetSVG(svg);
    a.GetShader(fs1);
    a.EmbedAllMedia();
    auto doc = SaveDoc(a);
    auto sm = doc.child("root").child("SequenceMedia");
    CHECK(sm.child("SVG"));
    CHECK(sm.child("Shader"));
    // add a hand written binary entry and a bogus node
    auto bin = sm.append_child("BinaryFile");
    bin.append_attribute("path") = "data.gled";
    bin.append_attribute("subtype") = "glediator";
    bin.append_child("Data").text().set(B64("\x01\x02\x03").c_str());
    sm.append_child("Unknown");

    SequenceMedia b;
    REQUIRE(b.LoadFromXml(sm));
    CHECK(b.GetSVG(svg)->GetSVGContent() == Svg(4, 4));
    CHECK(b.GetShader(fs1)->GetShaderSource().find("gl_FragColor") != std::string::npos);
    auto bf = b.GetBinaryFile("data.gled");
    REQUIRE(bf);
    CHECK(bf->GetSubtype() == "glediator");
    CHECK(bf->GetData() == std::vector<uint8_t>{ 1, 2, 3 });
    CHECK(b.GetShaderPaths() == std::vector<std::string>{ fs1 });

    auto again = SaveDoc(b);
    auto bnode = again.child("root").child("SequenceMedia").child("BinaryFile");
    REQUIRE(bnode);
    CHECK(std::string(bnode.attribute("subtype").as_string()) == "glediator");
}

TEST_CASE("Shader entries load and reload from disk", "[seq2][media]") {
    TempDir d;
    auto path = d.Write("x.fs", "void main(){}");
    SequenceMedia m;
    auto s = m.GetShader(path);
    REQUIRE(s->isLoaded());
    CHECK(s->GetShaderSource() == "void main(){}");
    CHECK(m.GetShader(path) == s);
    CHECK(m.GetShader("") == nullptr);
    d.Write("x.fs", "void main(){ /*v2*/ }");
    Touch(path, 5);
    CHECK(m.GetShader(path)->GetShaderSource() == "void main(){ /*v2*/ }");
    auto missing = m.GetShader(d.File("gone.fs"));
    CHECK_FALSE(missing->isLoaded());
    CHECK(missing->GetShaderConfig(nullptr) == nullptr);
}

TEST_CASE("Binary files only check that the file exists", "[seq2][media]") {
    TempDir d;
    auto real = d.Write("y.gled", std::string("\x00\x01\x02", 3));
    SequenceMedia m;
    auto b = m.GetBinaryFile(real, "glediator");
    REQUIRE(b);
    CHECK(b->isLoaded());
    CHECK(b->GetData().empty());
    CHECK(b->GetSubtype() == "glediator");
    CHECK_FALSE(b->IsEmbeddable());
    CHECK(m.GetBinaryFile(real) == b);
    CHECK(m.GetBinaryFile("") == nullptr);
    CHECK_FALSE(m.GetBinaryFile(d.File("none.gled"))->isLoaded());
    m.EmbedMedia(real);
    CHECK_FALSE(m.GetMediaEmbedState(real).first);
    CHECK(m.GetMediaEmbedState(real).second == false);
}

TEST_CASE("Video entries are path only", "[seq2][media]") {
    TempDir d;
    FixFileGuard g(d.path.string());
    auto path = d.Write("clip.mp4", "not really a video");
    SequenceMedia m;
    auto v = m.GetVideo(path);
    REQUIRE(v);
    CHECK(v->IsOk());
    CHECK_FALSE(v->IsEmbeddable());
    CHECK(fs::equivalent(v->GetResolvedPath(), path));
    CHECK(v->GetDurationMS() == 0);
    CHECK(v->GetThumbnail(8, 8) == nullptr);
    v->GeneratePreview(8, 8);
    CHECK_FALSE(v->HasPreview());
    CHECK(m.GetVideo(path) == v);
    CHECK(m.GetVideo("") == nullptr);
    REQUIRE(m.GetVideoFilePaths().size() == 1);
    CHECK(fs::equivalent(m.GetVideoFilePaths()[0], path));

    auto missing = m.GetVideo(d.File("absent.mp4"));
    CHECK_FALSE(missing->IsOk());
    CHECK(missing->GetDurationMS() == 0);
    CHECK(missing->GetThumbnail(8, 8) == nullptr);
    missing->GeneratePreview(8, 8);
    CHECK_FALSE(missing->HasPreview());

    VideoMediaCacheEntry unloaded("x.mp4");
    CHECK_FALSE(unloaded.IsOk());
}

TEST_CASE("Video and audio entries load from xml by path", "[seq2][media]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<SequenceMedia><Video path=\"v/one.mp4\"/><Audio path=\"a/song.mp3\"/><Audio path=\"\"/><Image/></SequenceMedia>"));
    SequenceMedia m;
    REQUIRE(m.LoadFromXml(doc.child("SequenceMedia")));
    CHECK(m.HasMedia("v/one.mp4"));
    CHECK(m.HasMedia("a/song.mp3"));
    CHECK(m.HasImage(""));
    auto paths = m.GetAllMediaPaths();
    CHECK(paths.size() == 3);
    CHECK(m.GetMediaCount() == 3);
    auto a = m.GetAudio("a/song.mp3");
    REQUIRE(a);
    CHECK(a->isLoaded());
    CHECK(m.GetAudio("") == nullptr);
}

TEST_CASE("MarkUsedByMetadata reaches every media type", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto svg = d.Write("a.svg", Svg(2, 2));
    auto txt = d.Write("a.txt", "x");
    auto sh = d.Write("a.fs", "x");
    auto bin = d.Write("a.gled", "x");
    auto vid = d.Write("a.mp4", "x");
    auto aud = d.Write("a.mp3", "x");
    m.GetSVG(svg);
    m.GetTextFile(txt);
    m.GetShader(sh);
    m.GetBinaryFile(bin);
    m.GetVideo(vid);
    m.GetAudio(aud);
    for (const auto& p : { svg, txt, sh, bin, vid, aud }) {
        INFO(p);
        CHECK_FALSE(m.IsUsedByMetadata(p));
        m.MarkUsedByMetadata(p);
        CHECK(m.IsUsedByMetadata(p));
        m.MarkUsedByMetadata(p, false);
        CHECK_FALSE(m.IsUsedByMetadata(p));
    }
    m.MarkUsedByMetadata("nothing");
    CHECK_FALSE(m.IsUsedByMetadata("nothing"));
}

// ---------------------------------------------------------------------------------------------
// Cross type operations
// ---------------------------------------------------------------------------------------------

TEST_CASE("Embed, extract and remove work across every media type", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto img = d.Write("i.png", Png(2, 2));
    auto svg = d.Write("s.svg", Svg(2, 2));
    auto txt = d.Write("t.txt", "x");
    auto sh = d.Write("s.fs", "x");
    auto vid = d.Write("v.mp4", "x");
    m.GetImage(img);
    m.GetSVG(svg);
    m.GetTextFile(txt);
    m.GetShader(sh);
    m.GetVideo(vid);
    for (const auto& p : { img, svg, txt, sh }) {
        INFO(p);
        m.EmbedMedia(p);
        CHECK(m.GetMediaEmbedState(p).first);
        m.ExtractMedia(p);
        CHECK_FALSE(m.GetMediaEmbedState(p).first);
    }
    m.EmbedMedia(vid);
    CHECK(m.GetMediaEmbedState(vid) == std::pair<bool, bool>{ false, false });
    CHECK(m.GetMediaEmbedState("unknown") == std::pair<bool, bool>{ false, false });

    m.EmbedAllMedia();
    CHECK(m.GetMediaEmbedState(svg).first);
    CHECK(m.GetMediaEmbedState(img).first);
    m.ExtractAllMedia();
    CHECK_FALSE(m.GetMediaEmbedState(svg).first);
    CHECK_FALSE(m.GetMediaEmbedState(txt).first);

    CHECK(m.GetMediaCount() == 5);
    m.RemoveMedia(svg);
    m.RemoveMedia(vid);
    CHECK(m.GetMediaCount() == 3);
    CHECK_FALSE(m.HasMedia(svg));
}

TEST_CASE("Embedded media is looked up by the stored key and by the resolved path", "[seq2][media]") {
    TempDir d;
    FixFileGuard g(d.path.string());
    auto abs = d.Write("pic.png", Png(2, 2));
    SequenceMedia m;
    m.AddEmbeddedImage("pic.png", B64(Png(2, 2)));
    CHECK(m.FindEmbeddedKey("pic.png") == "pic.png");
    CHECK(m.FindEmbeddedKey("") == "");
    // the absolute form of the same file finds the relative key
    CHECK(m.FindEmbeddedKey(abs) == "pic.png");
    // an external entry is not an embedded key
    auto other = d.Write("other.png", Png(2, 2));
    m.GetImage(other);
    CHECK(m.FindEmbeddedKey(other).empty());
    // separators are interchangeable for the exact lookup
    m.AddEmbeddedImage("sub\\x.png", B64(Png(1, 1)));
    CHECK(m.FindEmbeddedKey("sub/x.png") == "sub\\x.png");
}

TEST_CASE("CopyEmbeddedMedia copies text, svg and shader entries", "[seq2][media]") {
    TempDir d;
    auto txt = d.Write("t.txt", "words");
    auto svg = d.Write("s.svg", Svg(3, 3));
    auto sh = d.Write("x.fs", "void main(){}");
    SequenceMedia src;
    src.GetTextFile(txt);
    src.GetSVG(svg);
    src.GetShader(sh);
    src.EmbedAllMedia();

    SequenceMedia dst;
    CHECK(dst.CopyEmbeddedMedia(src, txt));
    CHECK(dst.CopyEmbeddedMedia(src, svg));
    CHECK(dst.CopyEmbeddedMedia(src, sh));
    CHECK(dst.GetMediaEmbedState(txt).first);
    CHECK(dst.GetTextFile(txt)->GetContent() == "words");
    CHECK(dst.GetSVG(svg)->GetSVGContent() == Svg(3, 3));
    CHECK(dst.GetShader(sh)->GetShaderSource() == "void main(){}");
    // copying again keeps the existing entry
    CHECK(dst.CopyEmbeddedMedia(src, txt));
    CHECK(dst.GetMediaCount() == 3);
}

TEST_CASE("RenameMedia rekeys text, svg, shader, binary, video and audio entries", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto txt = d.Write("t.txt", "x");
    auto svg = d.Write("s.svg", Svg(2, 2));
    auto sh = d.Write("s.fs", "x");
    auto bin = d.Write("b.gled", "x");
    auto vid = d.Write("v.mp4", "x");
    auto aud = d.Write("a.mp3", "x");
    m.GetTextFile(txt);
    m.GetSVG(svg);
    m.GetShader(sh);
    m.GetBinaryFile(bin);
    m.GetVideo(vid);
    m.GetAudio(aud);
    for (const auto& p : { txt, svg, sh, bin, vid, aud }) {
        INFO(p);
        CHECK(m.RenameMedia(p, p + ".new"));
        CHECK_FALSE(m.HasMedia(p));
        CHECK(m.HasMedia(p + ".new"));
    }
    CHECK_FALSE(m.RenameMedia("nothing", "x"));
}

TEST_CASE("ReloadMedia re-reads external entries but not embedded ones", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto img = d.Write("i.png", Png(2, 2));
    auto txt = d.Write("t.txt", "one");
    auto svg = d.Write("s.svg", Svg(2, 2));
    auto sh = d.Write("s.fs", "one");
    auto bin = d.Write("b.gled", "x");
    auto vid = d.Write("v.mp4", "x");
    auto aud = d.Write("a.mp3", "x");
    m.GetImage(img);
    m.GetTextFile(txt);
    m.GetSVG(svg);
    m.GetShader(sh);
    m.GetBinaryFile(bin, "glediator");
    m.GetVideo(vid);
    m.GetAudio(aud);

    d.Write("i.png", Png(5, 1));
    d.Write("t.txt", "two");
    d.Write("s.fs", "two");
    for (const auto& p : { img, txt, svg, sh, bin, vid, aud }) {
        INFO(p);
        CHECK(m.ReloadMedia(p));
    }
    CHECK(m.GetImage(img)->GetImageWidth() == 5);
    CHECK(m.GetTextFile(txt)->GetContent() == "two");
    CHECK(m.GetShader(sh)->GetShaderSource() == "two");
    CHECK(m.GetBinaryFile(bin)->GetSubtype() == "glediator");

    m.AddEmbeddedImage("emb.png", B64(Png(1, 1)));
    CHECK_FALSE(m.ReloadMedia("emb.png"));
    CHECK_FALSE(m.ReloadMedia("unknown"));
}

TEST_CASE("ForceRefreshEntry replaces entries that collide on the resolved path", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto img = d.Write("i.png", Png(2, 2));
    auto img2 = d.Write("i2.png", Png(6, 1));
    auto txt = d.Write("t.txt", "text");
    auto svg = d.Write("s.svg", Svg(2, 2));
    auto sh = d.Write("s.fs", "shader");
    auto bin = d.Write("b.gled", "x");
    auto vid = d.Write("v.mp4", "x");

    m.GetImage(img);
    // re-pointing the key at another file loads that file
    m.ForceRefreshEntry(img, img2, MediaType::Image);
    CHECK(m.GetImage(img)->GetImageWidth() == 6);

    m.GetTextFile(txt);
    m.ForceRefreshEntry("lyrics", txt, MediaType::TextFile);
    CHECK_FALSE(m.HasMedia(txt)); // old key resolved to the same file and was dropped
    CHECK(m.GetTextFile("lyrics")->GetContent() == "text");

    m.ForceRefreshEntry(svg, "", MediaType::SVG);
    CHECK(m.GetSVG(svg)->isLoaded());
    m.ForceRefreshEntry(sh, "", MediaType::Shader);
    CHECK(m.GetShader(sh)->GetShaderSource() == "shader");
    m.GetBinaryFile(bin, "glediator");
    m.ForceRefreshEntry(bin, bin, MediaType::BinaryFile);
    CHECK(m.GetBinaryFile(bin)->GetSubtype() == "glediator");
    m.ForceRefreshEntry(vid, vid, MediaType::Video);
    CHECK(m.GetVideo(vid)->IsOk());

    auto before = m.GetMediaCount();
    m.ForceRefreshEntry("", img, MediaType::Image);
    m.ForceRefreshEntry("a.mp3", "", MediaType::Audio);
    CHECK(m.GetMediaCount() == before);
}

TEST_CASE("Queued image loads decode on a pool and can be waited on", "[seq2][media]") {
    TempDir d;
    std::vector<std::string> paths;
    for (int i = 0; i < 6; ++i) paths.push_back(d.Write("q" + std::to_string(i) + ".png", Png(i + 1, 2)));
    SequenceMedia m;
    m.AddEmbeddedImage("emb.png", B64(Png(7, 7)));
    m.GetImage("emb.png")->UnloadCachedData();
    {
        JobPool pool("seq2media", 2, 2);
        for (const auto& p : paths) {
            m.QueueImageLoad(p, p, pool);
            m.QueueImageLoad(p, p, pool); // duplicates are ignored
        }
        m.QueueImageLoad("", "", pool);
        m.QueueUnloadedImages(pool);
        m.WaitForImageLoads();
        pool.Stop();
    }
    for (int i = 0; i < 6; ++i) {
        auto e = m.GetImage(paths[i]);
        CHECK(e->isLoaded());
        CHECK(e->GetImageWidth() == i + 1);
    }
    CHECK(m.GetImage("emb.png")->isLoaded());
}

TEST_CASE("RegisterImage defers decoding until first use", "[seq2][media]") {
    TempDir d;
    FixFileGuard g(d.path.string());
    auto p = d.Write("reg.png", Png(3, 1));
    SequenceMedia m;
    m.RegisterImage("");
    m.RegisterImage("reg.png");
    REQUIRE(m.HasImage("reg.png"));
    auto r = m.ResolveImagePath("reg.png");
    CHECK(r.settingsPath == "reg.png");
    CHECK(fs::equivalent(r.loadPath, p));
    CHECK(m.ResolveImagePath("").loadPath.empty());
    auto e = m.GetImage("reg.png");
    CHECK(e->GetImageWidth() == 3);
    // the same file under its absolute name shares the entry
    m.RegisterImage(p);
    CHECK(m.GetImage(p) == e);
}

TEST_CASE("Missing image tracking", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto absent = d.File("nope.png");
    CHECK(m.IsImageMissing(absent));
    CHECK(m.IsImageMissing(absent));
    auto present = d.Write("yes.png", Png(1, 1));
    CHECK_FALSE(m.IsImageMissing(present));
    m.AddEmbeddedImage("emb.png", B64(Png(1, 1)));
    CHECK_FALSE(m.IsImageMissing("emb.png"));
    // registering clears the missing mark
    d.Write("nope.png", Png(1, 1));
    m.RegisterImage(absent);
    CHECK_FALSE(m.IsImageMissing(absent));
}

TEST_CASE("AddEmbeddedImageFromFile ignores bad input", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    m.AddEmbeddedImageFromFile("", d.File("x.png"));
    m.AddEmbeddedImageFromFile("x.png", "");
    m.AddEmbeddedImageFromFile("x.png", d.File("missing.png"));
    m.AddEmbeddedImageFromFile("x.png", d.Write("empty.png", ""));
    CHECK(m.GetImageCount() == 0);
    m.AddEmbeddedImageFromFile("x.png", d.Write("real.png", Png(2, 3)));
    REQUIRE(m.HasImage("x.png"));
    m.AddEmbeddedImageFromFile("x.png", d.Write("other.png", Png(9, 9)));
    CHECK(m.GetImage("x.png")->GetImageHeight() == 3);
    // an empty xlImage cannot be encoded and is skipped
    m.AddEmbeddedImage("blank.png", xlImage());
    CHECK_FALSE(m.HasImage("blank.png"));
    m.AddEmbeddedImage("noframes.gif", std::vector<xlImage>{}, 50);
    CHECK_FALSE(m.HasImage("noframes.gif"));
}

TEST_CASE("Embedded entries can be saved back to a file", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    m.AddEmbeddedImage("e.png", B64(Png(2, 2)));
    auto e = m.GetImage("e.png");
    auto out = d.File("out.png");
    REQUIRE(e->SaveToFile(out));
    xlImage back;
    REQUIRE(back.LoadFromFile(out));
    CHECK(back.GetWidth() == 2);
    TextMediaCacheEntry empty("x");
    CHECK_FALSE(empty.SaveToFile(d.File("never.txt")));
    CHECK_FALSE(fs::exists(d.File("never.txt")));
}

TEST_CASE("ReloadMedia re-reads an image whose modification time did not change", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto img = d.Write("i.png", Png(2, 2));
    REQUIRE(m.GetImage(img)->GetImageWidth() == 2);
    auto when = std::filesystem::last_write_time(img);
    d.Write("i.png", Png(5, 1));
    // a rewrite within the file system's timestamp granularity looks unchanged
    std::filesystem::last_write_time(img, when);
    REQUIRE(m.ReloadMedia(img));
    CHECK(m.GetImage(img)->GetImageWidth() == 5);
}

TEST_CASE("A removed image is loaded again rather than returned from the path cache", "[seq2][media]") {
    TempDir d;
    SequenceMedia m;
    auto img = d.Write("i.png", Png(2, 2));
    auto first = m.GetImage(img);
    REQUIRE(first);
    m.RemoveImage(img);
    auto second = m.GetImage(img);
    REQUIRE(second);
    CHECK(second != first);
    m.RemoveMedia(img);
    CHECK(m.GetImage(img) != second);
}

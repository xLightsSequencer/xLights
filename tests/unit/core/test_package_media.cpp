#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SeqMediaMigration.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "support/TempDir.h"
#include "utils/Base64.h"
#include "utils/FileUtils.h"
#include "utils/xlImage.h"

#include <pugixml.hpp>

#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

xlImage Pattern(int w, int h, int seed = 0) {
    xlImage img(w, h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            img.SetRGBA(x, y, static_cast<uint8_t>(seed + x * 30 + 5), static_cast<uint8_t>(y * 40 + 9), static_cast<uint8_t>(x + y + 3), static_cast<uint8_t>(255 - x));
        }
    }
    return img;
}

bool Same(const xlImage& a, const xlImage& b) {
    return a.GetWidth() == b.GetWidth() && a.GetHeight() == b.GetHeight() && std::memcmp(a.GetData(), b.GetData(), static_cast<size_t>(a.GetWidth()) * a.GetHeight() * 4) == 0;
}

std::string PngB64(const xlImage& img) {
    std::vector<uint8_t> png;
    img.SaveAsPNG(png);
    return Base64::Encode(png.data(), png.size());
}

std::string WritePng(const TempDir& d, const std::string& name, const xlImage& img) {
    std::vector<uint8_t> png;
    img.SaveAsPNG(png);
    std::ofstream(d.path / name, std::ios::binary).write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
    return d.File(name);
}

pugi::xml_document Save(const SequenceMedia& m) {
    pugi::xml_document doc;
    auto root = doc.append_child("root");
    m.SaveToXml(root);
    return doc;
}

std::string Xml(const pugi::xml_document& d) {
    struct W : pugi::xml_writer {
        std::string s;
        void write(const void* data, size_t n) override { s.append(static_cast<const char*>(data), n); }
    } w;
    d.save(w);
    return w.s;
}

std::string RoundTrip(const SequenceMedia& m, SequenceMedia& out) {
    auto doc = Save(m);
    auto xml = Xml(doc);
    pugi::xml_document again;
    again.load_string(xml.c_str());
    REQUIRE(out.LoadFromXml(again.child("root").child("SequenceMedia")));
    return xml;
}

}

TEST_CASE("Embedded image survives an xml save and load with identical pixels", "[impexp][media][embed]") {
    SequenceMedia a;
    auto img = Pattern(5, 4);
    a.AddEmbeddedImage("pics/tree.png", img);
    REQUIRE(a.GetImageCount() == 1);
    auto e = a.GetImage("pics/tree.png");
    REQUIRE(e->IsOk());
    CHECK(e->IsEmbedded());
    CHECK(Same(*e->GetFrame(0, false), img));

    SequenceMedia b;
    RoundTrip(a, b);
    REQUIRE(b.GetImageCount() == 1);
    auto e2 = b.GetImage("pics/tree.png");
    REQUIRE(e2->IsOk());
    CHECK(e2->IsEmbedded());
    CHECK(e2->GetImageWidth() == 5);
    CHECK(e2->GetImageHeight() == 4);
    CHECK(Same(*e2->GetFrame(0, false), img));
}

TEST_CASE("Saved xml carries the image path and base64 data", "[impexp][media][embed]") {
    SequenceMedia a;
    auto img = Pattern(2, 2);
    a.AddEmbeddedImage("x.png", img);
    auto doc = Save(a);
    auto node = doc.child("root").child("SequenceMedia").child("Image");
    REQUIRE(node);
    CHECK(std::string(node.attribute("path").as_string()) == "x.png");
    auto data = Base64::Decode(node.child("Data").text().as_string());
    xlImage decoded;
    REQUIRE(decoded.LoadFromMemory(data.data(), data.size()));
    CHECK(Same(decoded, img));
}

TEST_CASE("Only embedded entries are written to the media block", "[impexp][media][embed]") {
    TempDir d;
    SequenceMedia a;
    auto path = WritePng(d, "ext.png", Pattern(2, 2));
    a.GetImage(path);
    a.AddEmbeddedImage("emb.png", Pattern(3, 3));
    auto doc = Save(a);
    int count = 0;
    for (auto n : doc.child("root").child("SequenceMedia").children("Image")) {
        ++count;
        CHECK(std::string(n.attribute("path").as_string()) == "emb.png");
    }
    CHECK(count == 1);
}

TEST_CASE("Embedding an external image keeps its pixels and survives deleting the file", "[impexp][media][embed]") {
    TempDir d;
    auto img = Pattern(4, 4, 7);
    auto path = WritePng(d, "ext.png", img);
    SequenceMedia a;
    auto e = a.GetImage(path);
    REQUIRE(e->IsOk());
    CHECK_FALSE(e->IsEmbedded());
    a.EmbedImage(path);
    CHECK(e->IsEmbedded());
    CHECK(a.GetMediaEmbedState(path).first);
    CHECK(a.GetMediaEmbedState(path).second);

    SequenceMedia b;
    RoundTrip(a, b);
    std::filesystem::remove(path);
    auto e2 = b.GetImage(path);
    REQUIRE(e2->IsOk());
    CHECK(Same(*e2->GetFrame(0, false), img));
}

TEST_CASE("Extracting an embedded image stops it being written into the xml", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("e.png", Pattern(2, 2));
    a.ExtractImage("e.png");
    CHECK_FALSE(a.GetImage("e.png")->IsEmbedded());
    CHECK(Save(a).child("root").child("SequenceMedia").child("Image").empty());
    a.EmbedAllImages();
    CHECK_FALSE(Save(a).child("root").child("SequenceMedia").child("Image").empty());
    a.ExtractAllImages();
    CHECK(Save(a).child("root").child("SequenceMedia").child("Image").empty());
}

TEST_CASE("ExtractImageToFile writes the image to disk and rekeys the entry", "[impexp][media][embed]") {
    TempDir d;
    SequenceMedia a;
    auto img = Pattern(3, 5, 2);
    a.AddEmbeddedImage("old.png", img);
    auto target = d.File("saved.png");
    REQUIRE(a.ExtractImageToFile("old.png", target));
    CHECK_FALSE(a.HasImage("old.png"));
    CHECK(a.HasImage(target));
    CHECK_FALSE(a.GetMediaEmbedState(target).first);
    xlImage onDisk;
    REQUIRE(onDisk.LoadFromFile(target));
    CHECK(Same(onDisk, img));
    CHECK_FALSE(a.ExtractImageToFile("missing.png", d.File("never.png")));
    CHECK_FALSE(std::filesystem::exists(d.File("never.png")));
}

TEST_CASE("AddEmbeddedImage does not replace an existing entry", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("same.png", Pattern(2, 2, 0));
    a.AddEmbeddedImage("same.png", Pattern(6, 6, 50));
    CHECK(a.GetImageCount() == 1);
    CHECK(a.GetImage("same.png")->GetImageWidth() == 2);
}

TEST_CASE("AddEmbeddedImage with raw base64 and with a file", "[impexp][media][embed]") {
    TempDir d;
    auto img = Pattern(3, 3, 9);
    SequenceMedia a;
    a.AddEmbeddedImage("b64.png", PngB64(img));
    CHECK(Same(*a.GetImage("b64.png")->GetFrame(0, false), img));
    auto path = WritePng(d, "src.png", img);
    a.AddEmbeddedImageFromFile("fromfile.png", path);
    REQUIRE(a.HasImage("fromfile.png"));
    CHECK(Same(*a.GetImage("fromfile.png")->GetFrame(0, false), img));
    a.AddEmbeddedImageFromFile("", path);
    a.AddEmbeddedImageFromFile("n.png", d.File("missing.png"));
    a.AddEmbeddedImageFromFile("n2.png", "");
    d.Write("empty.png", "");
    a.AddEmbeddedImageFromFile("n3.png", d.File("empty.png"));
    CHECK(a.GetImageCount() == 2);
}

TEST_CASE("Embedded image entries with garbage data load as placeholders and do not crash", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("bad.png", std::string("!!!not base64!!!"));
    a.AddEmbeddedImage("empty.png", std::string());
    a.AddEmbeddedImage("trunc.png", PngB64(Pattern(8, 8)).substr(0, 30));
    a.AddEmbeddedImage("text.png", Base64::Encode(reinterpret_cast<const uint8_t*>("hello world"), 11));
    for (auto name : { "bad.png", "empty.png", "trunc.png", "text.png" }) {
        auto e = a.GetImage(name);
        REQUIRE(e != nullptr);
        CHECK(e->GetFrame(0, false) != nullptr);
    }
    SequenceMedia b;
    RoundTrip(a, b);
    CHECK(b.GetImageCount() == 4);
}

TEST_CASE("Multi frame embedded image keeps frames and timing through xml", "[impexp][media][embed]") {
    SequenceMedia a;
    std::vector<xlImage> frames{ Pattern(2, 2, 0), Pattern(2, 2, 60), Pattern(2, 2, 120) };
    a.AddEmbeddedImage("anim.gif", frames, 100);
    auto e = a.GetImage("anim.gif");
    CHECK(e->GetImageCount() == 3);

    SequenceMedia b;
    RoundTrip(a, b);
    auto e2 = b.GetImage("anim.gif");
    REQUIRE(e2->IsOk());
    REQUIRE(e2->GetImageCount() == 3);
    for (int i = 0; i < 3; ++i) {
        CHECK(Same(*e2->GetFrame(i, false), frames[i]));
    }
    CHECK(e2->GetFrameForTime(0, true) == 0);
    CHECK(e2->GetFrameForTime(150, true) == 1);
    CHECK(e2->GetFrameForTime(250, true) == 2);
    CHECK(e2->GetFrameForTime(300, true) == 0);
    CHECK(e2->GetFrameForTime(10000, false) == 2);
}

TEST_CASE("Out of range frame requests return the invalid image placeholder", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("one.png", Pattern(2, 2));
    auto e = a.GetImage("one.png");
    auto good = e->GetFrame(0, false);
    REQUIRE(good != nullptr);
    CHECK(good->IsOk());
    auto bad = e->GetFrame(5, false);
    CHECK((bad == nullptr || !bad->IsOk()));
    CHECK(e->GetFrame(-1, false) == bad);
}

TEST_CASE("Media block loading ignores bad nodes and unknown children", "[impexp][media][embed]") {
    pugi::xml_document doc;
    doc.load_string("<SequenceMedia><Image path=\"a.png\"><Data>@@@</Data></Image><Image/><Mystery path=\"x\"/><Image path=\"b.png\"/><Image path=\"c.png\"><Frame time=\"abc\">@@</Frame></Image></SequenceMedia>");
    SequenceMedia m;
    REQUIRE(m.LoadFromXml(doc.child("SequenceMedia")));
    CHECK(m.HasImage("a.png"));
    CHECK(m.HasImage("b.png"));
    CHECK(m.HasImage("c.png"));
    CHECK_FALSE(m.LoadFromXml(doc.child("Nope")));
    pugi::xml_document empty;
    CHECK_FALSE(m.LoadFromXml(empty.document_element()));
}

TEST_CASE("Loading a media block replaces the previous contents", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("first.png", Pattern(2, 2));
    SequenceMedia b;
    b.AddEmbeddedImage("second.png", Pattern(2, 2));
    RoundTrip(b, a);
    CHECK_FALSE(a.HasImage("first.png"));
    CHECK(a.HasImage("second.png"));
}

TEST_CASE("Text and svg media embed and round trip", "[impexp][media][embed]") {
    TempDir d;
    auto txt = d.Write("lines.txt", "line one\nline two\n");
    auto svg = d.Write("shape.svg", "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"10\" height=\"10\"><rect width=\"10\" height=\"10\" fill=\"red\"/></svg>");
    SequenceMedia a;
    auto t = a.GetTextFile(txt);
    REQUIRE(t != nullptr);
    CHECK(t->GetContent() == "line one\nline two\n");
    auto s = a.GetSVG(svg);
    REQUIRE(s != nullptr);
    CHECK(s->GetSVGContent().find("<rect") != std::string::npos);
    a.EmbedMedia(txt);
    a.EmbedMedia(svg);

    SequenceMedia b;
    RoundTrip(a, b);
    std::filesystem::remove(txt);
    std::filesystem::remove(svg);
    auto t2 = b.GetTextFile(txt);
    REQUIRE(t2 != nullptr);
    CHECK(t2->GetContent() == "line one\nline two\n");
    auto s2 = b.GetSVG(svg);
    REQUIRE(s2 != nullptr);
    CHECK(s2->GetSVGContent() == s->GetSVGContent());
    CHECK(b.FindEmbeddedKey(txt) == txt);
    CHECK(b.FindEmbeddedKey("unknown").empty());
    CHECK(b.FindEmbeddedKey("").empty());
}

TEST_CASE("Videos, audio and binary files are never embeddable", "[impexp][media][embed]") {
    TempDir d;
    auto vid = d.Write("clip.mp4", "VID");
    auto aud = d.Write("song.mp3", "AUD");
    auto bin = d.Write("data.gled", "BIN");
    SequenceMedia a;
    a.GetVideo(vid);
    a.GetAudio(aud);
    a.GetBinaryFile(bin, "glediator");
    CHECK(a.GetMediaEmbedState(vid) == std::make_pair(false, false));
    CHECK(a.GetMediaEmbedState(aud) == std::make_pair(false, false));
    CHECK(a.GetMediaEmbedState(bin) == std::make_pair(false, false));
    a.EmbedAllMedia();
    CHECK_FALSE(a.GetMediaEmbedState(vid).first);
    CHECK_FALSE(a.GetMediaEmbedState(aud).first);
    CHECK(a.GetMediaCount() == 3);
}

TEST_CASE("Video and audio entries are only saved once marked used", "[impexp][media][embed]") {
    TempDir d;
    auto vid = d.Write("clip.mp4", "VID");
    auto aud = d.Write("song.mp3", "AUD");
    SequenceMedia a;
    auto v = a.GetVideo(vid);
    auto s = a.GetAudio(aud);
    auto doc = Save(a);
    CHECK(doc.child("root").child("SequenceMedia").child("Video").empty());
    CHECK(doc.child("root").child("SequenceMedia").child("Audio").empty());
    v->MarkIsUsed(true);
    s->MarkIsUsed(true);
    doc = Save(a);
    CHECK(std::string(doc.child("root").child("SequenceMedia").child("Video").attribute("path").as_string()) == vid);
    CHECK(std::string(doc.child("root").child("SequenceMedia").child("Audio").attribute("path").as_string()) == aud);

    SequenceMedia b;
    RoundTrip(a, b);
    CHECK(b.HasMedia(vid));
    CHECK(b.HasMedia(aud));
    CHECK(b.GetVideoFilePaths() == std::vector<std::string>{ vid });
}

TEST_CASE("Unused media can be pruned", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("keep.png", Pattern(2, 2));
    a.AddEmbeddedImage("drop.png", Pattern(2, 2));
    a.MarkAllUnused();
    a.GetImage("keep.png")->MarkIsUsed(true);
    a.RemoveUnusedImages();
    CHECK(a.HasImage("keep.png"));
    CHECK_FALSE(a.HasImage("drop.png"));
    TempDir d;
    auto ext = WritePng(d, "ext.png", Pattern(2, 2));
    a.GetImage(ext);
    a.MarkAllUnused();
    a.RemoveUnusedMedia();
    CHECK_FALSE(a.HasImage(ext));
    CHECK(a.HasImage("keep.png"));
}

TEST_CASE("Renaming media refuses to merge entries", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("one.png", Pattern(2, 2));
    a.AddEmbeddedImage("two.png", Pattern(3, 3));
    CHECK_FALSE(a.RenameImage("one.png", "two.png"));
    CHECK_FALSE(a.RenameImage("nothing.png", "new.png"));
    CHECK(a.RenameImage("one.png", "one.png"));
    CHECK(a.RenameImage("one.png", "uno.png"));
    CHECK_FALSE(a.HasImage("one.png"));
    CHECK(a.GetImage("uno.png")->GetFilePath() == "uno.png");
    CHECK_FALSE(a.RenameMedia("uno.png", "two.png"));
    CHECK(a.RenameMedia("uno.png", "eins.png"));
    CHECK(a.HasMedia("eins.png"));
    CHECK_FALSE(a.RenameMedia("ghost", "other"));
}

TEST_CASE("CopyEmbeddedMedia copies the data and refuses unknown or self copies", "[impexp][media][embed]") {
    SequenceMedia src;
    auto img = Pattern(3, 2, 4);
    src.AddEmbeddedImage("i.png", img);
    SequenceMedia dst;
    CHECK(dst.CopyEmbeddedMedia(src, "i.png"));
    CHECK(Same(*dst.GetImage("i.png")->GetFrame(0, false), img));
    CHECK(dst.GetMediaEmbedState("i.png").first);
    CHECK_FALSE(dst.CopyEmbeddedMedia(src, "nope.png"));
    CHECK_FALSE(dst.CopyEmbeddedMedia(src, ""));
    CHECK_FALSE(dst.CopyEmbeddedMedia(dst, "i.png"));
}

TEST_CASE("Missing images are remembered and can be cleared", "[impexp][media][embed]") {
    TempDir d;
    SequenceMedia a;
    auto missing = d.File("absent.png");
    auto e = a.GetImage(missing);
    REQUIRE(e != nullptr);
    CHECK_FALSE(e->IsOk());
    CHECK(a.GetImage("") == nullptr);
    a.ClearMissingImages();
    a.RemoveImage(missing);
    CHECK_FALSE(a.HasImage(missing));
}

TEST_CASE("Image paths are listed and Clear empties everything", "[impexp][media][embed]") {
    SequenceMedia a;
    a.AddEmbeddedImage("b.png", Pattern(2, 2));
    a.AddEmbeddedImage("a.png", Pattern(2, 2));
    CHECK(a.GetImagePaths() == std::vector<std::string>{ "a.png", "b.png" });
    CHECK(a.GetAllMediaPaths().size() == 2);
    a.Clear();
    CHECK(a.GetImageCount() == 0);
    CHECK(a.GetMediaCount() == 0);
}

TEST_CASE("Relocation records can be queued and cleared", "[impexp][media][embed]") {
    SequenceMedia a;
    a.RecordRelocation("a", "b");
    a.RecordRelocation("c", "d");
    CHECK(a.GetImageRelocations().size() == 2);
    a.ClearRelocations();
    CHECK(a.GetImageRelocations().empty());
}

TEST_CASE("Image files reload when they change on disk", "[impexp][media][embed]") {
    TempDir d;
    auto path = WritePng(d, "chg.png", Pattern(2, 2));
    const auto before = std::filesystem::file_time_type::clock::now() - std::chrono::hours(1);
    std::filesystem::last_write_time(path, before);
    SequenceMedia a;
    auto e = a.GetImage(path);
    REQUIRE(e->IsOk());
    CHECK(e->GetImageWidth() == 2);
    WritePng(d, "chg.png", Pattern(7, 3));
    std::filesystem::last_write_time(path, before + std::chrono::minutes(30));
    auto again = a.GetImage(path);
    CHECK(again->IsOk());
    CHECK(again->GetImageWidth() == 7);
    CHECK(again->GetImageHeight() == 3);
}

TEST_CASE("Resolving relative and absolute media paths", "[impexp][media][resolve]") {
    TempDir d;
    FileUtils::SetFixFileShowDir(d.path.string());
    struct Reset {
        ~Reset() { FileUtils::SetFixFileShowDir(""); }
    } reset;
    auto p = WritePng(d, "here.png", Pattern(2, 2));

    CHECK(SequenceMedia::ResolveFilePath("").loadPath.empty());
    auto rel = SequenceMedia::ResolveFilePath("here.png");
    CHECK(rel.settingsPath == "here.png");
    CHECK(std::filesystem::equivalent(rel.loadPath, p));
    auto abs = SequenceMedia::ResolveFilePath(p);
    CHECK(std::filesystem::equivalent(abs.loadPath, p));
    CHECK_FALSE(abs.settingsPath.empty());
    auto none = SequenceMedia::ResolveFilePath("not_there.png");
    CHECK(none.settingsPath == "not_there.png");
    CHECK_FALSE(none.loadPath.empty());
}

TEST_CASE("A relative image path is loaded from the show folder", "[impexp][media][resolve]") {
    TempDir d;
    FileUtils::SetFixFileShowDir(d.path.string());
    struct Reset {
        ~Reset() { FileUtils::SetFixFileShowDir(""); }
    } reset;
    WritePng(d, "rel.png", Pattern(4, 2));
    SequenceMedia a;
    auto e = a.GetImage("rel.png");
    REQUIRE(e->IsOk());
    CHECK(e->GetImageWidth() == 4);
    CHECK(a.HasImage("rel.png"));
    CHECK(a.GetImage("rel.png") == e);
}

namespace {
struct GifFixture {
    HeadlessRenderContext ctx;
    EffectLayer* layer = nullptr;
    GifFixture() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        layer = se.AddElement("Model", "model", true, false, true, false, false)->AddEffectLayer();
    }
};
}

TEST_CASE("Animated gif video effects become pictures effects", "[impexp][media][migration]") {
    GifFixture f;
    f.layer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=/shows/anim.GIF,E_TEXTCTRL_Video_Speed=-2.0,E_CHECKBOX_Video_Crop=1,B_CHOICE_BufferStyle=Default", "C_BUTTON_Palette1=#ff0000", 0, 1000, 0, false);
    f.layer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=/shows/clip.mp4", "", 1000, 2000, 0, false);
    f.layer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=/shows/other.gif", "", 2000, 3000, 0, false);
    f.layer->AddEffect(0, "Bars", "E_FILEPICKERCTRL_Video_Filename=/shows/anim.GIF", "", 3000, 4000, 0, false);

    std::vector<MediaCompatibilityIssue> issues{ { "/shows/anim.GIF", "gif", true } };
    CHECK(seqmedia::ConvertGifVideoEffectsToPictures(f.ctx.GetSequenceElements(), issues) == 1);

    Effect* a = f.layer->GetEffect(0);
    CHECK(a->GetEffectName() == "Pictures");
    auto& sm = a->GetSettings();
    CHECK(sm.Get("E_TEXTCTRL_Pictures_Filename", "") == "/shows/anim.GIF");
    CHECK(sm.Get("E_TEXTCTRL_Pictures_FrameRateAdj", "") == "1.0");
    CHECK(sm.Get("E_CHECKBOX_LoopGIF", "") == "1");
    CHECK_FALSE(sm.Contains("E_CHECKBOX_Video_Crop"));
    CHECK_FALSE(sm.Contains("E_FILEPICKERCTRL_Video_Filename"));
    CHECK(sm.Get("B_CHOICE_BufferStyle", "") == "Default");
    CHECK(f.layer->GetEffect(1)->GetEffectName() == "Video");
    CHECK(f.layer->GetEffect(2)->GetEffectName() == "Video");
    CHECK(f.layer->GetEffect(3)->GetEffectName() == "Bars");
}

TEST_CASE("Gif conversion keeps a positive video speed", "[impexp][media][migration]") {
    GifFixture f;
    f.layer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=g.gif,E_TEXTCTRL_Video_Speed=2.5", "", 0, 1000, 0, false);
    std::vector<MediaCompatibilityIssue> issues{ { "g.gif", "gif", true } };
    CHECK(seqmedia::ConvertGifVideoEffectsToPictures(f.ctx.GetSequenceElements(), issues) == 1);
    CHECK(f.layer->GetEffect(0)->GetSettings().Get("E_TEXTCTRL_Pictures_FrameRateAdj", "") == "2.5");
}

TEST_CASE("Gif conversion does nothing without matching issues", "[impexp][media][migration]") {
    GifFixture f;
    f.layer->AddEffect(0, "Video", "E_FILEPICKERCTRL_Video_Filename=g.gif", "", 0, 1000, 0, false);
    CHECK(seqmedia::ConvertGifVideoEffectsToPictures(f.ctx.GetSequenceElements(), {}) == 0);
    CHECK(seqmedia::ConvertGifVideoEffectsToPictures(f.ctx.GetSequenceElements(), { { "other.gif", "gif", true } }) == 0);
    CHECK(f.layer->GetEffect(0)->GetEffectName() == "Video");
}

TEST_CASE("Video conversion ignores audio and unreadable files", "[impexp][media][migration]") {
    GifFixture f;
    std::vector<MediaCompatibilityIssue> issues{ { "a.mp3", "Cannot open audio file", false }, { "v.avi", "Cannot open file", true }, { "w.avi", "Cannot read stream info", true }, { "x.wav", "Unsupported audio codec", false } };
    auto r = seqmedia::ConvertIncompatibleVideos(f.ctx.GetSequenceElements(), issues);
    CHECK(r.attempted == 0);
    CHECK(r.converted == 0);
    CHECK(r.effectsUpdated == 0);
    CHECK_FALSE(r.cancelled);
    CHECK(r.failures.empty());
    auto r2 = seqmedia::ConvertIncompatibleVideos(f.ctx.GetSequenceElements(), {});
    CHECK(r2.attempted == 0);
}

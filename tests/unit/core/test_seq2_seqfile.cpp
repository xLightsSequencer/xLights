#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "media/AudioManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "render/SongStructureManager.h"
#include "support/TempDir.h"
#include "utils/Base64.h"

#include <pugixml.hpp>
#include <zstd.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// HeadlessRenderContext does not create timing tracks; the desktop and iPad hosts do.
struct TimingCtx : HeadlessRenderContext {
    TimingCtx() { GetSequenceElements().SetViewsManager(&_sequenceViewManager); }
    TimingElement* AddTimingElement(const std::string& name, const std::string& subType = "") override {
        auto* t = dynamic_cast<TimingElement*>(_sequenceElements.AddElement(name, "timing", true, false, true, false, false));
        if (t == nullptr) return nullptr;
        if (t->GetEffectLayerCount() == 0) t->AddEffectLayer();
        t->SetSubType(subType);
        return t;
    }
};

struct Mark {
    std::string label;
    int start, end;
    bool operator==(const Mark& o) const { return label == o.label && start == o.start && end == o.end; }
};

std::vector<Mark> Marks(SequenceElements& se, const std::string& name) {
    std::vector<Mark> out;
    auto* t = se.GetTimingElement(name);
    if (t == nullptr) return out;
    auto* l = t->GetEffectLayer(0);
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        auto* e = l->GetEffect(i);
        out.push_back({ e->GetEffectName(), e->GetStartTimeMS(), e->GetEndTimeMS() });
    }
    return out;
}

std::string Read(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

struct Loaded {
    HeadlessRenderContext ctx;
    std::unique_ptr<SequenceFile> file;
    bool ok = false;
    Loaded(const std::string& path, const std::string& showDir) {
        ctx.showDirectory = showDir;
        file = std::make_unique<SequenceFile>(path);
        auto doc = file->Open(showDir, true, path);
        if (doc) ok = ctx.LoadSequenceElements(*file, *doc);
    }
    SequenceElements& se() { return ctx.GetSequenceElements(); }
};

std::string Doc(const std::string& body) {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
           "<head><version>2024.01</version><sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>10.000</sequenceDuration></head>" +
           body + "</xsequence>";
}

} // namespace

TEST_CASE("Timing sections added before the sequence loads are queued then applied", "[seq2][xsq]") {
    TimingCtx ctx;
    SequenceFile sf("pending.xsq");
    sf.SetSequenceDurationMS(1000);
    std::vector<int> starts{ 0, 100, 300, 500, 500, 950, 1200 };
    std::vector<int> ends{ 200, 300, 300, 600, 600, 1100, 1300 };
    std::vector<std::string> labels{ "a", "b", "z", "c", "c", "last", "late" };
    sf.AddNewTimingSection("Beats", &ctx, starts, ends, labels);
    sf.AddFixedTimingSection("50ms", &ctx);
    sf.AddFixedTimingSection("Empty", &ctx);
    sf.AddFixedTimingSection("Custom", 250, &ctx);
    sf.AddNewTimingSection("Lyrics", &ctx, "Papagayo");
    sf.AddMetronomeLabelTimingSection("Metro", 300, { "A", "B" }, &ctx, -1, false);
    CHECK(sf.GetTimingList() == std::vector<std::string>{ "Beats", "50ms", "Empty", "Custom", "Lyrics", "Metro" });
    CHECK(sf.TimingAlreadyExists("Custom", &ctx));
    CHECK_FALSE(sf.TimingAlreadyExists("Other", &ctx));
    CHECK_FALSE(sf.TimingMatchesModelName("Custom", &ctx));
    CHECK(ctx.GetSequenceElements().GetElementCount() == 0);

    sf.SetSequenceLoaded(true);
    REQUIRE(sf.GetSequenceLoaded());
    sf.ApplyPendingTimings(&ctx);
    auto& se = ctx.GetSequenceElements();
    CHECK(se.GetNumberOfTimingElements() == 6);

    // overlapping marks are trimmed, duplicates and marks after the end dropped,
    // zero length marks stretched to the next start
    CHECK(Marks(se, "Beats") == std::vector<Mark>{ { "a", 0, 100 }, { "b", 100, 300 }, { "z", 300, 500 }, { "c", 500, 600 }, { "last", 950, 1100 } });

    auto fifty = Marks(se, "50ms");
    REQUIRE(fifty.size() >= 20);
    for (int i = 0; i < 20; ++i) {
        CHECK(fifty[i].start == i * 50);
        CHECK(fifty[i].end == i * 50 + 50);
    }
    CHECK(se.GetTimingElement("50ms")->GetFixedTiming() == 50);
    CHECK(Marks(se, "Empty").empty());
    auto custom = Marks(se, "Custom");
    REQUIRE(custom.size() >= 4);
    CHECK(custom[3] == Mark{ "", 750, 1000 });
    CHECK(se.GetTimingElement("Lyrics")->GetSubType() == "Papagayo");
    CHECK(Marks(se, "Metro") == std::vector<Mark>{ { "A", 0, 300 }, { "B", 300, 600 }, { "A", 600, 900 }, { "B", 900, 1000 } });

    // nothing left pending
    sf.ApplyPendingTimings(&ctx);
    CHECK(se.GetNumberOfTimingElements() == 6);
    CHECK(sf.TimingAlreadyExists("Metro", &ctx));
    CHECK(sf.TimingMatchesModelName("Metro", &ctx));
}

TEST_CASE("A zero length final timing mark runs to the end of the sequence", "[seq2][xsq]") {
    TimingCtx ctx;
    SequenceFile sf("z.xsq");
    sf.SetSequenceDurationMS(1000);
    sf.SetSequenceLoaded(true);
    std::vector<int> starts{ 0, 400 };
    std::vector<int> ends{ 100, 400 };
    std::vector<std::string> labels{ "x", "y" };
    sf.AddNewTimingSection("T", &ctx, starts, ends, labels);
    CHECK(Marks(ctx.GetSequenceElements(), "T") == std::vector<Mark>{ { "x", 0, 100 }, { "y", 400, 1000 } });

    // a zero length mark at the very end is dropped
    std::vector<int> s2{ 0, 1000 };
    std::vector<int> e2{ 100, 1000 };
    std::vector<std::string> l2{ "x", "y" };
    sf.AddNewTimingSection("T2", &ctx, s2, e2, l2);
    CHECK(Marks(ctx.GetSequenceElements(), "T2") == std::vector<Mark>{ { "x", 0, 100 } });
}

TEST_CASE("Metronome timing with random labels and intervals", "[seq2][xsq]") {
    TimingCtx ctx;
    SequenceFile sf("m.xsq");
    sf.SetSequenceDurationMS(5000);
    sf.SetSequenceLoaded(true);
    sf.AddMetronomeLabelTimingSection("R", 400, { "P", "Q", "R" }, &ctx, 100, true);
    auto m = Marks(ctx.GetSequenceElements(), "R");
    REQUIRE(m.size() >= 12);
    CHECK(m.front().start == 0);
    CHECK(m.back().end == 5000);
    for (size_t i = 0; i < m.size(); ++i) {
        CHECK(std::set<std::string>{ "P", "Q", "R" }.count(m[i].label) == 1);
        CHECK(m[i].end - m[i].start <= 400);
        if (i > 0) {
            CHECK(m[i].label != m[i - 1].label);
            CHECK(m[i].start == m[i - 1].end);
        }
    }
    // no tags: numbered 1..10
    sf.AddMetronomeLabelTimingSection("N", 100, {}, &ctx, -1, false);
    auto n = Marks(ctx.GetSequenceElements(), "N");
    REQUIRE(n.size() == 50);
    CHECK(n[0].label == "1");
    CHECK(n[9].label == "10");
    CHECK(n[10].label == "1");
}

TEST_CASE("Fixed timing tracks have no zero length mark at the end", "[seq2]") {
    TimingCtx ctx;
    SequenceFile sf("f.xsq");
    sf.SetSequenceDurationMS(1000);
    sf.SetSequenceLoaded(true);
    sf.AddFixedTimingSection("50ms", &ctx);
    auto m = Marks(ctx.GetSequenceElements(), "50ms");
    CHECK(m.size() == 20);
    for (const auto& k : m) CHECK(k.end > k.start);
}

TEST_CASE("Alternate audio track names stay unique and follow removals", "[seq2][xsq]") {
    TempDir td;
    SequenceFile sf(td.File("alt.xsq"));
    sf.AddAltTrack(td.path.string(), td.File("one.mp3"), "Drums");
    sf.AddAltTrack(td.path.string(), td.File("two.mp3"), "");
    sf.AddAltTrack(td.path.string(), td.File("three.mp3"), "Vocals");
    REQUIRE(sf.GetAltTrackCount() == 3);
    CHECK(sf.GetAltTrackDisplayName(1) == "Track2");
    CHECK(sf.GetAltTrackMedia(0) == nullptr);

    sf.SetAltTrackShortname(1, "Drums");
    CHECK(sf.GetAltTrackDisplayName(1) == "Drums_2");
    sf.SetAltTrackShortname(2, "Drums");
    CHECK(sf.GetAltTrackDisplayName(2) == "Drums_3");
    sf.SetAltTrackShortname(0, "Drums");
    CHECK(sf.GetAltTrackDisplayName(0) == "Drums");
    sf.SetAltTrackShortname(7, "x");

    sf.SetAltTrackPath(td.path.string(), 1, td.File("moved.mp3"));
    CHECK(sf.GetAltTrack(1).path.find("moved.mp3") != std::string::npos);
    sf.SetAltTrackPath(td.path.string(), -1, td.File("x.mp3"));

    sf.RemoveAltTrack(0);
    REQUIRE(sf.GetAltTrackCount() == 2);
    CHECK(sf.GetAltTrackDisplayName(0) == "Drums_2");
    sf.RemoveAltTrack(5);
    CHECK(sf.GetAltTrackCount() == 2);
}

namespace {
std::string SilentWav() {
    auto put = [](std::string& s, uint32_t v, int bytes) {
        for (int i = 0; i < bytes; i++) s.push_back(char((v >> (8 * i)) & 0xFF));
    };
    const uint32_t rate = 8000, dataBytes = rate * 2; // one second, mono 16-bit
    std::string s = "RIFF";
    put(s, 36 + dataBytes, 4);
    s += "WAVEfmt ";
    put(s, 16, 4); put(s, 1, 2); put(s, 1, 2); put(s, rate, 4); put(s, rate * 2, 4); put(s, 2, 2); put(s, 16, 2);
    s += "data";
    put(s, dataBytes, 4);
    s.append(dataBytes, '\0');
    return s;
}
} // namespace

TEST_CASE("The host is told before the sequence deletes an AudioManager", "[seq2][xsq][audio]") {
    TempDir td;
    std::string const show = td.path.string();
    std::string const a = td.Write("a.wav", SilentWav());
    std::string const b = td.Write("b.wav", SilentWav());

    SequenceFile sf(td.File("audio.xsq"), 50);
    std::vector<AudioManager*> released;
    // Every pointer handed over must still be owned by the sequence, i.e. not yet deleted.
    std::vector<AudioManager*> owned;
    sf.SetAudioReleasingCallback([&](AudioManager* am) {
        bool stillOwned = am == sf.GetMedia();
        for (int i = 0; i < sf.GetAltTrackCount(); i++) stillOwned = stillOwned || am == sf.GetAltTrackMedia(i);
        CHECK(stillOwned);
        released.push_back(am);
    });

    sf.SetMediaFile(show, a, false);
    AudioManager* first = sf.GetMedia();
    REQUIRE(first != nullptr);
    CHECK(released.empty());

    sf.SetMediaFile(show, b, false);
    REQUIRE(released.size() == 1);
    CHECK(released.back() == first);
    REQUIRE(sf.GetMedia() != nullptr);

    AudioManager* second = sf.GetMedia();
    sf.ClearMediaFile();
    REQUIRE(released.size() == 2);
    CHECK(released.back() == second);
    CHECK(sf.GetMedia() == nullptr);

    sf.SetMediaFile(show, a, false);
    AudioManager* third = sf.GetMedia();
    sf.SetSequenceType("Animation");
    REQUIRE(released.size() == 3);
    CHECK(released.back() == third);

    sf.AddAltTrack(show, a, "Drums");
    AudioManager* alt = sf.GetAltTrackMedia(0);
    REQUIRE(alt != nullptr);
    sf.SetAltTrackPath(show, 0, b);
    REQUIRE(released.size() == 4);
    CHECK(released.back() == alt);

    AudioManager* alt2 = sf.GetAltTrackMedia(0);
    REQUIRE(alt2 != nullptr);
    sf.RemoveAltTrack(0);
    REQUIRE(released.size() == 5);
    CHECK(released.back() == alt2);
}

TEST_CASE("ClearMediaFile resets the song tags", "[seq2][xsq]") {
    SequenceFile sf("c.xsq");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::SONG, "Song");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, "Artist");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, "Album");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::URL, "http://example.com");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, "Me");
    sf.ClearMediaFile();
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::SONG).empty());
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::ARTIST).empty());
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::ALBUM).empty());
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::URL).empty());
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR) == "Me");
    CHECK(sf.GetMediaFile().empty());
}

TEST_CASE("Image directory is remembered", "[seq2][xsq]") {
    SequenceFile sf("i.xsq");
    CHECK(sf.GetImageDir(nullptr).empty());
    sf.SetImageDir("/some/images");
    CHECK(sf.GetImageDir(nullptr) == "/some/images");
}

TEST_CASE("GetMediaForXSQ finds the audio in media folders or the show", "[seq2][xsq]") {
    TempDir show;
    TempDir media;
    auto inMedia = media.Write("song.mp3", "x");
    auto xsq = show.Write("a.xsq", Doc("").replace(Doc("").find("</head>"), 7, "<mediaFile>/gone/song.mp3</mediaFile></head>"));
    CHECK(SequenceFile::GetMediaForXSQ(xsq, show.path.string(), { media.path.string() }) == inMedia);

    auto direct = show.Write("direct.mp3", "x");
    auto xsq2 = show.Write("b.xsq", Doc("").replace(Doc("").find("</head>"), 7, "<mediaFile>" + direct + "</mediaFile></head>"));
    CHECK(SequenceFile::GetMediaForXSQ(xsq2, show.path.string(), {}) == direct);

    auto xsq3 = show.Write("c.xsq", Doc("").replace(Doc("").find("</head>"), 7, "<mediaFile>/gone/nothing.mp3</mediaFile></head>"));
    CHECK(SequenceFile::GetMediaForXSQ(xsq3, show.path.string(), { media.path.string() }).empty());
    CHECK(SequenceFile::GetMediaForXSQ(show.File("none.xsq"), show.path.string(), {}).empty());
    auto xsq4 = show.Write("d.xsq", Doc(""));
    CHECK(SequenceFile::GetMediaForXSQ(xsq4, show.path.string(), {}).empty());
}

TEST_CASE("Effects on submodels, strands and nodes survive a save and load", "[seq2][xsq]") {
    TempDir td;
    auto p = td.File("deep.xsq");
    {
        HeadlessRenderContext ctx;
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* m = dynamic_cast<ModelElement*>(se.AddElement("Tree", "model", true, false, true, false, false));
        if (m->GetEffectLayerCount() == 0) m->AddEffectLayer();
        m->GetEffectLayer(0)->AddEffect(0, "On", "", "", 0, 100, 0, false);
        auto* sub = m->GetSubModel("Left", true);
        sub->GetEffectLayer(0)->AddEffect(0, "Bars", "", "", 100, 200, 0, false);
        auto* subL1 = sub->AddEffectLayer();
        subL1->SetLayerName("Upper");
        subL1->AddEffect(0, "Twinkle", "", "", 300, 400, 0, false);
        auto* s1 = m->GetStrand(1, true);
        s1->GetEffectLayer(0)->AddEffect(0, "Off", "", "", 0, 50, 0, false);
        s1->GetNodeLayer(2, true)->AddEffect(0, "On", "", "", 50, 150, 0, false);
        auto* s2 = m->GetStrand(2, true);
        s2->GetNodeLayer(0, true)->AddEffect(0, "Fire", "", "", 0, 500, 0, false);
        s2->GetNodeLayer(0)->SetNodeName("first");
        se.GetSongStructureManager().AddBoundary(5000, 10000);

        SequenceFile sf(p);
        REQUIRE(sf.Save(se));
    }
    auto xml = Read(p);
    CHECK(xml.find("SubModelEffectLayer") != std::string::npos);
    CHECK(xml.find("<Strand") != std::string::npos);
    CHECK(xml.find("<Node") != std::string::npos);
    CHECK(xml.find("layerName=\"Upper\"") != std::string::npos);

    Loaded l(p, td.path.string());
    REQUIRE(l.ok);
    auto* m = dynamic_cast<ModelElement*>(l.se().GetElement("Tree"));
    REQUIRE(m != nullptr);
    auto* sub = m->GetSubModel("Left");
    REQUIRE(sub != nullptr);
    REQUIRE(sub->GetEffectLayerCount() == 2);
    CHECK(sub->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "Bars");
    CHECK(sub->GetEffectLayer(1)->GetLayerName() == "Upper");
    CHECK(sub->GetEffectLayer(1)->GetEffect(0)->GetEffectName() == "Twinkle");
    auto* s1 = m->GetStrand(1);
    REQUIRE(s1 != nullptr);
    CHECK(s1->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "Off");
    REQUIRE(s1->GetNodeLayer(2) != nullptr);
    CHECK(s1->GetNodeLayer(2)->GetEffect(0)->GetEffectName() == "On");
    auto* s2 = m->GetStrand(2);
    REQUIRE(s2 != nullptr);
    REQUIRE(s2->GetNodeLayer(0) != nullptr);
    CHECK(s2->GetNodeLayer(0)->GetEffect(0)->GetEndTimeMS() == 500);
    CHECK(l.se().GetSongStructureManager().AnyViewHasRegions());
    CHECK(l.se().GetSongStructureManager().GetViewCount() == 1);
}

TEST_CASE("CompressedData sections in an xsq are expanded on load", "[seq2][xsq]") {
    TempDir td;
    const std::string inner = "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Packed\" visible=\"1\"/></DisplayElements>"
                              "<ElementEffects><Element type=\"model\" name=\"Packed\"><EffectLayer>"
                              "<Effect ref=\"0\" name=\"On\" startTime=\"0\" endTime=\"100\" palette=\"0\"/></EffectLayer></Element></ElementEffects>";
    std::vector<uint8_t> comp(ZSTD_compressBound(inner.size()));
    size_t n = ZSTD_compress(comp.data(), comp.size(), inner.data(), inner.size(), 3);
    REQUIRE_FALSE(ZSTD_isError(n));
    std::string b64 = Base64::Encode(comp.data(), n);
    auto p = td.Write("packed.xsq", Doc("<ColorPalettes><ColorPalette>C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1</ColorPalette></ColorPalettes>"
                                        "<EffectDB><Effect></Effect></EffectDB>"
                                        "<CompressedData size=\"" + std::to_string(inner.size()) + "\">" + b64 + "</CompressedData>"));
    Loaded l(p, td.path.string());
    REQUIRE(l.ok);
    auto* e = l.se().GetElement("Packed");
    REQUIRE(e != nullptr);
    REQUIRE(e->GetEffectLayerCount() >= 1);
    REQUIRE(e->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(e->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "On");
}

TEST_CASE("A CompressedData section that does not decompress is skipped", "[seq2][xsq]") {
    TempDir td;
    const std::string inner = "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Packed\" visible=\"1\"/></DisplayElements>";
    std::vector<uint8_t> comp(ZSTD_compressBound(inner.size()));
    size_t n = ZSTD_compress(comp.data(), comp.size(), inner.data(), inner.size(), 3);
    REQUIRE_FALSE(ZSTD_isError(n));
    // a size attribute too small for the content makes zstd fail
    std::string tooSmall = Base64::Encode(comp.data(), n);
    std::string garbage = Base64::Encode(reinterpret_cast<const uint8_t*>("not zstd at all"), 15);
    auto p = td.Write("bad.xsq", Doc("<ColorPalettes></ColorPalettes><EffectDB></EffectDB>"
                                     "<CompressedData size=\"4\">" + tooSmall + "</CompressedData>"
                                     "<CompressedData size=\"100\">" + garbage + "</CompressedData>"));
    Loaded l(p, td.path.string());
    REQUIRE(l.ok);
    CHECK(l.se().GetElement("Packed") == nullptr);
}

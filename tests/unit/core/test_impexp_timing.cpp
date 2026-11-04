#include <catch2/catch_test_macros.hpp>

#include "import_export/Vixen3.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "render/UICallbacks.h"
#include "support/TempDir.h"
#include "utils/AppCallbacks.h"
#include "utils/ZipUtils.h"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct TimingCtx : HeadlessRenderContext {
    TimingCtx() { GetSequenceElements().SetViewsManager(&_sequenceViewManager); }
    TimingElement* AddTimingElement(const std::string& name, const std::string& subType = "") override {
        auto* e = dynamic_cast<TimingElement*>(GetSequenceElements().AddElement(name, "timing", true, false, true, false, false));
        if (e != nullptr && e->GetEffectLayerCount() == 0) {
            e->AddEffectLayer();
        }
        if (e != nullptr && !subType.empty()) {
            e->SetSubType(subType);
        }
        return e;
    }
    SequenceElements& se() { return GetSequenceElements(); }
};

struct StubUI : UICallbacks {
    std::vector<std::string> choice;
    long number = 0;
    void ShowMessage(const std::string&, const std::string&) const override {}
    bool PromptYesNo(const std::string&, const std::string&) const override { return true; }
    std::string PromptForDirectory(const std::string&, const std::string&) const override { return ""; }
    std::string PromptForFile(const std::string&, const std::string&, const std::string&) const override { return ""; }
    long PromptForNumber(const std::string&, const std::string&, long, long, long) const override { return number; }
    std::string PromptForText(const std::string&, const std::string&, const std::string&) const override { return ""; }
    std::vector<std::string> ChooseFromList(const std::string&, const std::vector<std::string>&) const override { return choice; }
    ProgressToken BeginProgress(const std::string&, int) override { return 0; }
    void UpdateProgress(ProgressToken, int, const std::string&) override {}
    void EndProgress(ProgressToken) override {}
};

struct UICtx : TimingCtx {
    StubUI ui;
    UICallbacks* GetUICallbacks() override { return &ui; }
};

struct Silence {
    int errors = 0;
    Silence() {
        AppCallbacks::SetDisplayMessageCallback([this](AppCallbacks::DisplayMessageLevel level, const std::string&) {
            if (level == AppCallbacks::DisplayMessageLevel::Error) {
                ++errors;
            }
        });
    }
    ~Silence() { AppCallbacks::SetDisplayMessageCallback(nullptr); }
};

bool KnownCrashEnabled() { return std::getenv("XL_RUN_KNOWN_CRASH") != nullptr; }

using Mark = std::tuple<int, int, std::string>;

std::vector<Mark> Marks(TimingCtx& c, const std::string& name, int layer = 0) {
    std::vector<Mark> out;
    Element* e = c.se().GetElement(name);
    if (e == nullptr || layer >= static_cast<int>(e->GetEffectLayerCount())) {
        return out;
    }
    EffectLayer* l = e->GetEffectLayer(layer);
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        Effect* ef = l->GetEffect(i);
        out.emplace_back(ef->GetStartTimeMS(), ef->GetEndTimeMS(), ef->GetEffectName());
    }
    return out;
}

std::vector<std::string> TimingNames(TimingCtx& c) {
    std::vector<std::string> n;
    for (size_t i = 0; i < c.se().GetElementCount(); ++i) {
        if (c.se().GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            n.push_back(c.se().GetElement(i)->GetName());
        }
    }
    return n;
}

std::string XTimingDoc(const std::string& inner) {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + inner;
}

}

TEST_CASE("Audacity import reads tab delimited labels", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("beats.txt", "0.000000\t0.500000\tone\n0.500000\t1.000000\ttwo words\n1.000000\t1.500000\tthree\n");
    sf.ProcessAudacityTimingFiles({ f }, &c);
    CHECK(Marks(c, "beats") == std::vector<Mark>{ { 0, 500, "one" }, { 500, 1000, "two words" }, { 1000, 1500, "three" } });
}

TEST_CASE("Audacity import reads space delimited labels and joins extra words", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("s.txt", "0.0 1.0 hello big world\n1.0 2.0 bye\n");
    sf.ProcessAudacityTimingFiles({ f }, &c);
    CHECK(Marks(c, "s") == std::vector<Mark>{ { 0, 1000, "hello big world" }, { 1000, 2000, "bye" } });
}

TEST_CASE("Audacity import rounds times to the frame grid", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("r.txt", "0.012\t1.234\ta\n1.234\t2.490\tb\n");
    sf.ProcessAudacityTimingFiles({ f }, &c);
    CHECK(Marks(c, "r") == std::vector<Mark>{ { 0, 1250, "a" }, { 1250, 2500, "b" } });
}

TEST_CASE("Audacity import strips comments and blank lines", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("c.txt", "# a comment line\n\n0.0\t0.5\tok # trailing comment\n   \n0.5\t1.0\tnext\n");
    sf.ProcessAudacityTimingFiles({ f }, &c);
    auto m = Marks(c, "c");
    REQUIRE(m.size() == 2);
    CHECK(std::get<0>(m[0]) == 0);
    CHECK(std::get<1>(m[0]) == 500);
    CHECK(std::get<2>(m[0]) == "ok");
    CHECK(std::get<2>(m[1]) == "next");
}

TEST_CASE("Audacity import with a missing end time borrows the next start", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("z.txt", "0.0\t0.0\tA\n1.0\t1.0\tB\n2.0\t3.0\tC\n");
    sf.ProcessAudacityTimingFiles({ f }, &c);
    auto m = Marks(c, "z");
    REQUIRE(m.size() == 3);
    CHECK(m[0] == Mark{ 0, 1000, "A" });
    CHECK(m[1] == Mark{ 1000, 2000, "B" });
    CHECK(m[2] == Mark{ 2000, 3000, "C" });
}

TEST_CASE("Audacity import gives a trailing zero length mark one frame", "[impexp][timing][audacity][!mayfail]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("z.txt", "0.0\t1.0\tA\n2.0\t2.0\tLast\n");
    sf.ProcessAudacityTimingFiles({ f }, &c);
    auto m = Marks(c, "z");
    REQUIRE(m.size() == 2);
    // endTime = startTime + GetFrequency() adds 20 (Hz) rather than the 50 ms frame
    CHECK(std::get<1>(m[1]) - std::get<0>(m[1]) == sf.GetFrameMS());
}

TEST_CASE("Audacity import names clash with an existing timing track", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("dup.txt", "0.0\t1.0\ta\n");
    sf.ProcessAudacityTimingFiles({ f, f }, &c);
    CHECK(TimingNames(c) == std::vector<std::string>{ "dup", "dup_1" });
}

TEST_CASE("Audacity import tolerates junk input", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessAudacityTimingFiles({ d.Write("empty.txt", "") }, &c);
    CHECK(Marks(c, "empty").empty());
    sf.ProcessAudacityTimingFiles({ d.Write("junk.txt", "abc\tdef\tghi\nnot numbers at all\n") }, &c);
    CHECK(Marks(c, "junk").size() <= 2);
    std::string big;
    for (int i = 0; i < 2000; ++i) {
        big += "9999999999999\t99999999999999\tx\n";
    }
    sf.ProcessAudacityTimingFiles({ d.Write("huge.txt", big) }, &c);
    std::string bin = std::string("\xff\xfe\x00\x80\xc3\x28\t1\t2\n", 9);
    sf.ProcessAudacityTimingFiles({ d.Write("bin.txt", bin) }, &c);
    SUCCEED();
}

TEST_CASE("Audacity import of a missing file adds nothing", "[impexp][timing][audacity]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessAudacityTimingFiles({ d.File("nope.txt") }, &c);
    CHECK(TimingNames(c).empty());
}

TEST_CASE("xTiming single timing import", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("one.xtiming", XTimingDoc(
        "<timing name=\"Beats\" SourceVersion=\"2024.01\">"
        "<EffectLayer><Effect label=\"a\" starttime=\"0\" endtime=\"500\"/><Effect label=\"b\" starttime=\"500\" endtime=\"1000\"/></EffectLayer>"
        "</timing>"));
    sf.ProcessXTiming({ f }, &c);
    CHECK(TimingNames(c) == std::vector<std::string>{ "Beats" });
    CHECK(Marks(c, "Beats") == std::vector<Mark>{ { 0, 500, "a" }, { 500, 1000, "b" } });
}

TEST_CASE("xTiming multiple timings and layers", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("many.xtiming", XTimingDoc(
        "<timings>"
        "<timing name=\"Words\"><EffectLayer><Effect label=\"hi\" starttime=\"0\" endtime=\"1000\"/></EffectLayer>"
        "<EffectLayer><Effect label=\"h\" starttime=\"0\" endtime=\"500\"/><Effect label=\"i\" starttime=\"500\" endtime=\"1000\"/></EffectLayer></timing>"
        "<timing name=\"Other\"><EffectLayer><Effect label=\"x\" starttime=\"100\" endtime=\"200\"/></EffectLayer></timing>"
        "<notatiming name=\"Ignored\"/>"
        "</timings>"));
    sf.ProcessXTiming({ f }, &c);
    CHECK(TimingNames(c) == std::vector<std::string>{ "Words", "Other" });
    CHECK(Marks(c, "Words", 0) == std::vector<Mark>{ { 0, 1000, "hi" } });
    CHECK(Marks(c, "Words", 1) == std::vector<Mark>{ { 0, 500, "h" }, { 500, 1000, "i" } });
    CHECK(Marks(c, "Other") == std::vector<Mark>{ { 100, 200, "x" } });
}

TEST_CASE("xTiming snaps times down to the frame and unescapes labels", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("snap.xtiming", XTimingDoc(
        "<timing name=\"A&amp;B\"><EffectLayer><Effect label=\"x&amp;y\" starttime=\"123\" endtime=\"477\"/></EffectLayer></timing>"));
    sf.ProcessXTiming({ f }, &c);
    auto names = TimingNames(c);
    REQUIRE(names.size() == 1);
    auto m = Marks(c, names[0]);
    REQUIRE(m.size() == 1);
    CHECK(std::get<0>(m[0]) == 100);
    CHECK(std::get<1>(m[0]) == 450);
}

TEST_CASE("xTiming keeps the sub type", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("sub.xtiming", XTimingDoc(
        "<timing name=\"Lyr\" subType=\"Lyric\"><EffectLayer><Effect label=\"x\" starttime=\"0\" endtime=\"50\"/></EffectLayer></timing>"));
    sf.ProcessXTiming({ f }, &c);
    auto* t = dynamic_cast<TimingElement*>(c.se().GetElement("Lyr"));
    REQUIRE(t != nullptr);
    CHECK(t->GetSubType() == "Lyric");
}

TEST_CASE("xTiming importing the same file twice makes unique names", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("t.xtiming", XTimingDoc("<timing name=\"T\"><EffectLayer/></timing>"));
    sf.ProcessXTiming({ f, f, f }, &c);
    CHECK(TimingNames(c) == std::vector<std::string>{ "T", "T_1", "T_2" });
}

TEST_CASE("xTiming rejects bad files without adding tracks", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessXTiming({ d.File("absent.xtiming") }, &c);
    CHECK(quiet.errors == 1);
    sf.ProcessXTiming({ d.Write("bad.xtiming", "<timing name=\"x\"><EffectLayer>") }, &c);
    CHECK(quiet.errors == 2);
    sf.ProcessXTiming({ d.Write("other.xtiming", XTimingDoc("<something/>")) }, &c);
    sf.ProcessXTiming({ d.Write("empty.xtiming", "") }, &c);
    CHECK(TimingNames(c).size() <= 1);
}

TEST_CASE("xTiming copes with odd attribute values", "[impexp][timing][xtiming]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("odd.xtiming", XTimingDoc(
        "<timing><EffectLayer><Effect starttime=\"abc\" endtime=\"99999999999999999999\"/>"
        "<Effect label=\"neg\" starttime=\"-50\" endtime=\"-10\"/></EffectLayer></timing>"));
    sf.ProcessXTiming({ f }, &c);
    SUCCEED();
}

TEST_CASE("Timing sections survive an xsq save and the xLights timing import", "[impexp][timing][roundtrip]") {
    TempDir d;
    std::string path = d.File("src.xsq");
    {
        TimingCtx c;
        auto* t = c.AddTimingElement("Beats");
        auto* l0 = t->GetEffectLayer(0);
        l0->AddEffect(0, "one", "", "", 0, 500, 0, false);
        l0->AddEffect(0, "two", "", "", 500, 1250, 0, false);
        auto* t2 = c.AddTimingElement("Words");
        t2->GetEffectLayer(0)->AddEffect(0, "w", "", "", 100, 200, 0, false);
        t2->AddEffectLayer()->AddEffect(0, "p", "", "", 100, 150, 0, false);
        SequenceFile sf(path);
        sf.SetSequenceDurationMS(5000);
        REQUIRE(sf.Save(c.se()));
    }
    TimingCtx dest;
    dest.showDirectory = d.path.string();
    SequenceFile sf("dest.xsq");
    auto names = sf.GetXLightsTimingTrackNames(path, &dest);
    CHECK(names == std::vector<std::string>{ "Beats", "Words" });

    sf.SetSequenceLoaded(true);
    sf.ProcessXLightsTiming(path, {}, &dest);
    CHECK(Marks(dest, "Beats") == std::vector<Mark>{ { 0, 500, "one" }, { 500, 1250, "two" } });
    CHECK(Marks(dest, "Words", 0) == std::vector<Mark>{ { 100, 200, "w" } });
    CHECK(Marks(dest, "Words", 1) == std::vector<Mark>{ { 100, 150, "p" } });
}

TEST_CASE("xLights timing import honours the selected indices", "[impexp][timing][roundtrip]") {
    TempDir d;
    std::string path = d.File("src.xsq");
    {
        TimingCtx c;
        c.AddTimingElement("A")->GetEffectLayer(0)->AddEffect(0, "a", "", "", 0, 100, 0, false);
        c.AddTimingElement("B")->GetEffectLayer(0)->AddEffect(0, "b", "", "", 0, 100, 0, false);
        c.AddTimingElement("C")->GetEffectLayer(0)->AddEffect(0, "c", "", "", 0, 100, 0, false);
        SequenceFile sf(path);
        sf.SetSequenceDurationMS(1000);
        REQUIRE(sf.Save(c.se()));
    }
    TimingCtx dest;
    dest.showDirectory = d.path.string();
    SequenceFile sf("dest.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessXLightsTiming(path, { 2, 0, 7, -1 }, &dest);
    CHECK(TimingNames(dest) == std::vector<std::string>{ "C", "A" });
}

TEST_CASE("xLights timing import before a sequence is loaded stores pending timings", "[impexp][timing][roundtrip]") {
    TempDir d;
    std::string path = d.File("src.xsq");
    {
        TimingCtx c;
        c.AddTimingElement("A")->GetEffectLayer(0)->AddEffect(0, "a", "", "", 0, 100, 0, false);
        SequenceFile sf(path);
        sf.SetSequenceDurationMS(1000);
        REQUIRE(sf.Save(c.se()));
    }
    TimingCtx dest;
    dest.showDirectory = d.path.string();
    SequenceFile sf("dest.xsq");
    sf.ProcessXLightsTiming(path, {}, &dest);
    CHECK(TimingNames(dest).empty());
    CHECK(sf.GetTimingList() == std::vector<std::string>{ "A" });
    sf.SetSequenceLoaded(true);
    sf.ApplyPendingTimings(&dest);
    CHECK(TimingNames(dest) == std::vector<std::string>{ "A" });
}

TEST_CASE("xLights timing names of a missing or corrupt file are empty", "[impexp][timing][roundtrip]") {
    TempDir d;
    TimingCtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    CHECK(sf.GetXLightsTimingTrackNames(d.File("nope.xsq"), &c).empty());
    CHECK(sf.GetXLightsTimingTrackNames(d.Write("bad.xsq", "garbage <<<"), &c).empty());
}

TEST_CASE("SRT import reads times and joined text lines", "[impexp][timing][srt]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("subs.srt",
        "1\n00:00:06,580 --> 00:00:08,580\nHello there\nsecond line\n\n"
        "2\n00:01:00,000 --> 00:01:02,500\nNext\n\n"
        "3\n01:00:00,000 --> 01:00:01,000\nHour\n\n"
        "4\n01:00:02,000 --> 01:00:03,000\nPadding\n\n");
    sf.ProcessSRT({ f }, &c);
    auto m = Marks(c, "subs");
    REQUIRE(m.size() >= 2);
    CHECK(m[0] == Mark{ 6580, 8580, "Hello there second line" });
    CHECK(m[1] == Mark{ 60000, 62500, "Next" });
    REQUIRE(m.size() >= 3);
    CHECK(m[2] == Mark{ 3600000, 3601000, "Hour" });
}

TEST_CASE("SRT import keeps the final subtitle", "[impexp][timing][srt][!mayfail]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("two.srt",
        "1\n00:00:01,000 --> 00:00:02,000\nA\n\n"
        "2\n00:00:03,000 --> 00:00:04,000\nB\n\n");
    sf.ProcessSRT({ f }, &c);
    // the do/while in ProcessSRT stops once the index reaches the end, before adding the last entry it just read
    CHECK(Marks(c, "two").size() == 2);
}

TEST_CASE("SRT import drops entries with a non positive duration and tolerates junk", "[impexp][timing][srt]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("bad.srt",
        "1\n00:00:05,000 --> 00:00:04,000\nBackwards\n\n"
        "2\nnot a time line\nText\n\n"
        "3\n00:00:01,000 --> 00:00:02,000\nGood\n\n"
        "4\n00:00:03,000 --> 00:00:04,000\nPad\n\n");
    sf.ProcessSRT({ f }, &c);
    for (const auto& mk : Marks(c, "bad")) {
        CHECK(std::get<1>(mk) > std::get<0>(mk));
        CHECK(std::get<2>(mk) != "Backwards");
    }
    sf.ProcessSRT({ d.Write("empty.srt", "") }, &c);
    sf.ProcessSRT({ d.Write("one.srt", "1\n") }, &c);
    sf.ProcessSRT({ d.Write("noeol.srt", "1\n00:00:01,000 --> 00:00:02,000") }, &c);
    SUCCEED();
}

TEST_CASE("SRT import of a missing file reports an error", "[impexp][timing][srt]") {
    TempDir d;
    TimingCtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessSRT({ d.File("none.srt") }, &c);
    CHECK(quiet.errors == 1);
    CHECK(TimingNames(c).empty());
}

namespace {
std::string PapagayoFile() {
    return "LipSync Version 1\n"
           "song.wav\n"
           "20\n"
           "400\n"
           "1\n"
           "Voice A\n"
           "Hello world\n"
           "\t1\n"
           "\t\tHello world\n"
           "\t\t0\n"
           "\t\t20\n"
           "\t\t2\n"
           "\t\t\tHello 0 10 2\n"
           "\t\t\t\t0 HH\n"
           "\t\t\t\t5 EH\n"
           "\t\t\tworld 10 20 1\n"
           "\t\t\t\t10 W\n";
}
}

TEST_CASE("Papagayo import builds phrase, word and phoneme layers", "[impexp][timing][papagayo]") {
    TempDir d;
    UICtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessPapagayo({ d.Write("lip.pgo", PapagayoFile()) }, &c);
    REQUIRE(TimingNames(c) == std::vector<std::string>{ "Voice 1" });
    CHECK(Marks(c, "Voice 1", 0) == std::vector<Mark>{ { 0, 1000, "Hello world" } });
    CHECK(Marks(c, "Voice 1", 1) == std::vector<Mark>{ { 0, 500, "Hello" }, { 500, 1000, "world" } });
    CHECK(Marks(c, "Voice 1", 2) == std::vector<Mark>{ { 0, 250, "HH" }, { 250, 500, "EH" }, { 500, 1000, "W" } });
}

TEST_CASE("Papagayo import applies the frame offset from the UI", "[impexp][timing][papagayo]") {
    TempDir d;
    UICtx c;
    c.ui.number = 4;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessPapagayo({ d.Write("lip.pgo", PapagayoFile()) }, &c);
    CHECK(Marks(c, "Voice 1", 0) == std::vector<Mark>{ { 200, 1200, "Hello world" } });
    CHECK(std::get<0>(Marks(c, "Voice 1", 1).front()) == 200);
}

TEST_CASE("Papagayo header is case insensitive and wrong headers are rejected", "[impexp][timing][papagayo]") {
    TempDir d;
    UICtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    std::string lower = PapagayoFile();
    lower.replace(0, 17, "LIPSYNC VERSION 1");
    sf.ProcessPapagayo({ d.Write("a.pgo", lower) }, &c);
    CHECK(quiet.errors == 0);
    CHECK(TimingNames(c).size() == 1);
    sf.ProcessPapagayo({ d.Write("b.pgo", "Not A Papagayo File\n") }, &c);
    CHECK(quiet.errors == 1);
    CHECK(TimingNames(c).size() == 1);
}

TEST_CASE("Papagayo import survives truncated files", "[impexp][timing][papagayo]") {
    TempDir d;
    UICtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    std::string full = PapagayoFile();
    for (size_t cut = 0; cut < full.size(); cut += 7) {
        sf.ProcessPapagayo({ d.Write("t" + std::to_string(cut) + ".pgo", full.substr(0, cut)) }, &c);
    }
    sf.ProcessPapagayo({ d.File("missing.pgo") }, &c);
    SUCCEED();
}

TEST_CASE("Papagayo import handles zero sample rate without crashing", "[impexp][timing][papagayo][known-crash]") {
    if (!KnownCrashEnabled()) { SKIP("crashes the process; set XL_RUN_KNOWN_CRASH to run"); }
    TempDir d;
    UICtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    // samples per second 0: DisplayError is raised but parsing continues into 1000 / 0
    sf.ProcessPapagayo({ d.Write("z.pgo", "LipSync Version 1\nx.wav\n0\n10\n1\n") }, &c);
    SUCCEED();
}

TEST_CASE("ElevenLabs import creates phrase and word layers", "[impexp][timing][elevenlabs]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("speech.json",
        R"({"segments":[{"text":"Hello world","start_time":0.0,"end_time":1.0,"words":[
            {"text":"Hello","start_time":0.0,"end_time":0.5},
            {"text":" ","start_time":0.5,"end_time":0.5},
            {"text":"","start_time":0.5,"end_time":0.5},
            {"text":"world","start_time":0.5,"end_time":1.0}]},
          {"text":"Again","start_time":2.0,"end_time":2.4,"words":[{"text":"Again","start_time":2.0,"end_time":2.4}]}]})");
    sf.ProcessElevenLabsTimingFiles({ f }, &c);
    CHECK(Marks(c, "speech", 0) == std::vector<Mark>{ { 0, 1000, "Hello world" }, { 2000, 2400, "Again" } });
    CHECK(Marks(c, "speech", 1) == std::vector<Mark>{ { 0, 500, "Hello" }, { 500, 1000, "world" }, { 2000, 2400, "Again" } });
}

TEST_CASE("ElevenLabs zero length items are widened by one frame", "[impexp][timing][elevenlabs][!mayfail]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto f = d.Write("zero.json",
        R"({"segments":[{"text":"Hm","start_time":1.0,"end_time":1.0,"words":[{"text":"Hm","start_time":1.0,"end_time":1.0}]}]})");
    sf.ProcessElevenLabsTimingFiles({ f }, &c);
    auto m = Marks(c, "zero", 0);
    REQUIRE(m.size() == 1);
    // startTime + GetFrequency() (20) rounds back to startTime so the mark stays zero length
    CHECK(std::get<1>(m[0]) > std::get<0>(m[0]));
}

TEST_CASE("ElevenLabs import with bad json adds at most an empty track", "[impexp][timing][elevenlabs]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessElevenLabsTimingFiles({ d.Write("bad.json", "{not json") }, &c);
    sf.ProcessElevenLabsTimingFiles({ d.Write("wrongtype.json", R"({"segments":[{"text":5,"start_time":"a"}]})") }, &c);
    sf.ProcessElevenLabsTimingFiles({ d.Write("empty.json", "{}") }, &c);
    sf.ProcessElevenLabsTimingFiles({ d.Write("nothing.json", "") }, &c);
    for (const auto& n : TimingNames(c)) {
        CHECK(Marks(c, n).empty());
    }
}

TEST_CASE("ElevenLabs import of a missing file is a no-op", "[impexp][timing][elevenlabs]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessElevenLabsTimingFiles({ d.File("none.json") }, &c);
    CHECK(TimingNames(c).empty());
}

namespace {
std::string LorTimingDoc() {
    return "<?xml version=\"1.0\"?>\n<sequence>"
           "<timingGrids>"
           "<timingGrid saveID=\"0\" name=\"Fixed\" type=\"fixed\" spacing=\"500\"/>"
           "<timingGrid saveID=\"1\" name=\"Free One\" type=\"freeform\">"
           "<timing centisecond=\"0\"/><timing centisecond=\"50\"/><timing centisecond=\"125\"/><timing centisecond=\"200\"/></timingGrid>"
           "<timingGrid saveID=\"2\" type=\"freeform\"><timing centisecond=\"10\"/><timing centisecond=\"60\"/></timingGrid>"
           "</timingGrids></sequence>";
}
}

TEST_CASE("LOR timing import converts the chosen freeform grid", "[impexp][timing][lor]") {
    TempDir d;
    UICtx c;
    c.ui.choice = { "Free One" };
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessLorTiming({ d.Write("show.lms", LorTimingDoc()) }, &c);
    REQUIRE(TimingNames(c).size() == 1);
    auto name = TimingNames(c)[0];
    CHECK(name == "show: Free One");
    CHECK(Marks(c, name) == std::vector<Mark>{ { 0, 500, "" }, { 500, 1250, "" }, { 1250, 2000, "" } });
}

TEST_CASE("LOR timing import names unnamed grids by their save id", "[impexp][timing][lor]") {
    TempDir d;
    UICtx c;
    c.ui.choice = { "Unnamed2" };
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessLorTiming({ d.Write("show.lms", LorTimingDoc()) }, &c);
    REQUIRE(TimingNames(c).size() == 1);
    CHECK(Marks(c, TimingNames(c)[0]) == std::vector<Mark>{ { 100, 600, "" } });
}

TEST_CASE("LOR timing import with a cancelled chooser adds nothing", "[impexp][timing][lor]") {
    TempDir d;
    UICtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessLorTiming({ d.Write("show.lms", LorTimingDoc()) }, &c);
    CHECK(TimingNames(c).empty());
}

TEST_CASE("LOR timing import without a UI adds nothing", "[impexp][timing][lor]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessLorTiming({ d.Write("show.lms", LorTimingDoc()) }, &c);
    CHECK(TimingNames(c).empty());
}

TEST_CASE("LOR timing import reports missing and malformed files", "[impexp][timing][lor]") {
    TempDir d;
    UICtx c;
    Silence quiet;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessLorTiming({ d.File("none.lms") }, &c);
    CHECK(quiet.errors == 1);
    sf.ProcessLorTiming({ d.Write("bad.lms", "<sequence><timingGrids>") }, &c);
    CHECK(quiet.errors == 2);
}

TEST_CASE("LOR timing import of an empty freeform grid must not run off the end", "[impexp][timing][lor][known-crash]") {
    if (!KnownCrashEnabled()) { SKIP("crashes the process; set XL_RUN_KNOWN_CRASH to run"); }
    TempDir d;
    UICtx c;
    c.ui.choice = { "Empty" };
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    // grid_times.size() - 1 underflows for a grid with no <timing> children
    sf.ProcessLorTiming({ d.Write("e.lms", "<sequence><timingGrids><timingGrid saveID=\"0\" name=\"Empty\" type=\"freeform\"/></timingGrids></sequence>") }, &c);
    SUCCEED();
}

TEST_CASE("Vixen 3 timing import with generic mark collections", "[impexp][timing][vixen3]") {
    TempDir d;
    TimingCtx c;
    auto f = d.Write("seq.tim",
        "<TimedSequenceData><MarkCollections><MarkCollection><Name>Beat</Name><Marks>"
        "<d3p1:duration>PT0.5S</d3p1:duration><d3p1:duration>PT1M1.5S</d3p1:duration>"
        "</Marks></MarkCollection></MarkCollections></TimedSequenceData>");
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    CHECK(sf.GetVixen3TimingTrackNames(f) == std::vector<std::string>{ "Beat" });
    sf.ProcessVixen3Timing(f, {}, &c);
    CHECK(Marks(c, "Beat") == std::vector<Mark>{ { 0, 500, "" }, { 500, 61500, "" } });
}

TEST_CASE("Vixen 3 timing import with phrase collections builds three layers", "[impexp][timing][vixen3]") {
    TempDir d;
    TimingCtx c;
    auto mark = [](const std::string& start, const std::string& dur, const std::string& text) {
        return "<d1p1:anyType><d2p1:StartTime>" + start + "</d2p1:StartTime><d2p1:Duration>" + dur + "</d2p1:Duration><d2p1:Text>" + text + "</d2p1:Text></d1p1:anyType>";
    };
    auto coll = [&](const std::string& name, const std::string& type, const std::string& marks) {
        return "<d1p1:anyType><d2p1:Name>" + name + "</d2p1:Name><d2p1:CollectionType>" + type + "</d2p1:CollectionType><d2p1:Marks>" + marks + "</d2p1:Marks></d1p1:anyType>";
    };
    auto f = d.Write("lyr.tim",
        "<TimedSequenceData><LabeledMarkCollections>" +
        coll("Song", "Phrase", mark("PT1S", "PT2S", "Hello world")) +
        coll("Song Word", "Word", mark("PT1S", "PT1S", "Hello") + mark("PT2S", "PT1S", "world")) +
        coll("Song Phoneme", "Phoneme", mark("PT1S", "PT0.5S", "HH")) +
        "</LabeledMarkCollections></TimedSequenceData>");
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    CHECK(sf.GetVixen3TimingTrackNames(f) == std::vector<std::string>{ "Song" });
    sf.ProcessVixen3Timing(f, {}, &c);
    CHECK(Marks(c, "Song", 0) == std::vector<Mark>{ { 1000, 3000, "Hello world" } });
    CHECK(Marks(c, "Song", 1) == std::vector<Mark>{ { 1000, 2000, "Hello" }, { 2000, 3000, "world" } });
    CHECK(Marks(c, "Song", 2) == std::vector<Mark>{ { 1000, 1500, "HH" } });
}

TEST_CASE("Vixen 3 timing import tolerates garbage", "[impexp][timing][vixen3]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    CHECK(sf.GetVixen3TimingTrackNames(d.File("none.tim")).empty());
    CHECK(sf.GetVixen3TimingTrackNames(d.Write("junk.tim", "not xml")).empty());
    CHECK(sf.GetVixen3TimingTrackNames(d.Write("other.tim", "<Other/>")).empty());
    sf.ProcessVixen3Timing(d.Write("junk2.tim", "\xff\xfe"), { 3, -2 }, &c);
    CHECK(TimingNames(c).empty());
}

TEST_CASE("Vixen3 ConvertTiming rounds to the frame", "[impexp][timing][vixen3]") {
    CHECK(Vixen3::ConvertTiming(1.0f, 50) == 1000);
    CHECK(Vixen3::ConvertTiming(0.026f, 50) == 50);
    CHECK(Vixen3::ConvertTiming(0.024f, 50) == 0);
    CHECK(Vixen3::ConvertTiming(0.0f, 25) == 0);
}

TEST_CASE("Timing sections added through SequenceFile helpers", "[impexp][timing][sections]") {
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.SetSequenceDurationMS(1000);
    sf.SetSequenceLoaded(true);

    SECTION("fixed interval track covers the whole sequence") {
        sf.AddFixedTimingSection("100ms", &c);
        auto m = Marks(c, "100ms");
        REQUIRE(m.size() >= 10);
        CHECK(m[0] == Mark{ 0, 100, "" });
        CHECK(m[9] == Mark{ 900, 1000, "" });
        auto* t = dynamic_cast<TimingElement*>(c.se().GetElement("100ms"));
        REQUIRE(t != nullptr);
        CHECK(t->GetFixedTiming() == 100);
    }
    SECTION("non fixed names give an empty track") {
        sf.AddFixedTimingSection("Mine", &c);
        CHECK(Marks(c, "Mine").empty());
    }
    SECTION("explicit interval") {
        sf.AddFixedTimingSection("quarter", 250, &c);
        auto m = Marks(c, "quarter");
        REQUIRE(m.size() >= 4);
        CHECK(m[0] == Mark{ 0, 250, "" });
        CHECK(m[3] == Mark{ 750, 1000, "" });
    }
    SECTION("metronome cycles through tags") {
        sf.AddMetronomeLabelTimingSection("Metro", 250, { "A", "B", "C" }, &c);
        auto m = Marks(c, "Metro");
        REQUIRE(m.size() == 4);
        CHECK(std::get<2>(m[0]) == "A");
        CHECK(std::get<2>(m[1]) == "B");
        CHECK(std::get<2>(m[2]) == "C");
        CHECK(std::get<2>(m[3]) == "A");
        CHECK(std::get<1>(m[3]) == 1000);
    }
    SECTION("metronome defaults to numeric labels") {
        sf.AddMetronomeLabelTimingSection("Metro", 500, {}, &c);
        auto m = Marks(c, "Metro");
        REQUIRE(m.size() == 2);
        CHECK(std::get<2>(m[0]) == "1");
        CHECK(std::get<2>(m[1]) == "2");
    }
    SECTION("random labels never repeat back to back") {
        sf.SetSequenceDurationMS(5000);
        sf.AddMetronomeLabelTimingSection("Rnd", 100, { "A", "B", "C" }, &c, -1, true);
        auto m = Marks(c, "Rnd");
        REQUIRE(m.size() == 50);
        for (size_t i = 1; i < m.size(); ++i) {
            CHECK(std::get<2>(m[i]) != std::get<2>(m[i - 1]));
        }
    }
}

TEST_CASE("AddNewTimingSection sorts, trims overlaps and drops marks past the end", "[impexp][timing][sections]") {
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.SetSequenceDurationMS(2000);
    sf.SetSequenceLoaded(true);

    std::vector<int> starts{ 1000, 0, 500, 5000 };
    std::vector<int> ends{ 1500, 600, 1200, 6000 };
    std::vector<std::string> labels{ "c", "a", "b", "late" };
    sf.AddNewTimingSection("vamp", &c, starts, ends, labels);
    auto m = Marks(c, "vamp");
    REQUIRE(m.size() == 3);
    CHECK(m[0] == Mark{ 0, 500, "a" });
    CHECK(m[1] == Mark{ 500, 1000, "b" });
    CHECK(m[2] == Mark{ 1000, 1500, "c" });
}

TEST_CASE("AddNewTimingSection before load is deferred until applied", "[impexp][timing][sections]") {
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceDurationMS(2000);
    std::vector<int> starts{ 0 };
    std::vector<int> ends{ 500 };
    std::vector<std::string> labels{ "a" };
    sf.AddNewTimingSection("later", &c, starts, ends, labels);
    CHECK(TimingNames(c).empty());
    CHECK(sf.GetTimingList() == std::vector<std::string>{ "later" });
    sf.SetSequenceLoaded(true);
    sf.ApplyPendingTimings(&c);
    CHECK(Marks(c, "later") == std::vector<Mark>{ { 0, 500, "a" } });
    sf.ApplyPendingTimings(&c);
    CHECK(TimingNames(c).size() == 1);
}

TEST_CASE("UniqueTimingName strips unsafe xml characters and numbers clashes", "[impexp][timing][sections]") {
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    c.AddTimingElement("Name");
    c.AddTimingElement("Name_1");
    CHECK(sf.UniqueTimingName(&c, "Name") == "Name_2");
    CHECK(sf.UniqueTimingName(&c, "Fresh") == "Fresh");
    CHECK(sf.TimingAlreadyExists("Name", &c));
    CHECK_FALSE(sf.TimingAlreadyExists("Other", &c));
}

TEST_CASE("LSP timing import needs a zip with a Sequence entry", "[impexp][timing][lsp]") {
    TempDir d;
    TimingCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessLSPTiming({ d.Write("notzip.lsp", "not a zip"), d.File("missing.lsp") }, &c);
    CHECK(TimingNames(c).empty());
}

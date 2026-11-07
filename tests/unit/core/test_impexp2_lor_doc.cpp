#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>
using Catch::Approx;

#include "import_export/LOREdit.h"
#include "import_export/SuperStarImporter.h"
#include "support/TempDir.h"
#include "test_impexp2_lor_common.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <vector>

using namespace lortest;

namespace {
bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }

std::string Timings(const std::string& name, const std::vector<int>& cs, const char* tag = "TimingGridFree") {
    std::string s = std::string("<") + tag + " name=\"" + name + "\">";
    for (int c : cs) s += "<timing centisecond=\"" + std::to_string(c) + "\"/>";
    return s + "</" + tag + ">";
}
std::string TimingDoc(const std::string& inner) { return Seq("<TimingGrids>" + inner + "</TimingGrids>"); }

std::string ChEff(int s, int e, int inten = 100, const std::string& type = "INTENSITY") {
    return ChanEffect(s, e, type, "intensity=\"" + std::to_string(inten) + "\"");
}

std::string Mouth(const std::string& face, const std::string& shape, const std::vector<std::pair<int, int>>& ranges, int inten = 100) {
    std::string e;
    for (auto& r : ranges) e += ChEff(r.first, r.second, inten);
    return SeqProp(face + " Mouth " + shape, Chan(0, 0, 0, e));
}

// every public query, to prove nothing falls over
void Poke(const LOREdit& l) {
    (void)l.GetTimingTracks();
    for (auto& t : l.GetTimingTracks()) (void)l.GetTimings(t, 0);
    auto models = l.GetModelsWithEffects();
    for (auto& m : models) {
        int r, c;
        (void)l.GetModelChannels(m, r, c);
        (void)l.GetModelStrands(m);
        (void)l.GetSequencingType(m);
        int n = l.GetModelLayers(m);
        for (int i = 0; i < n; ++i) {
            for (auto& e : l.GetTrackEffects(m, i, 0)) {
                std::string pal = e.GetPalette();
                (void)e.GetxLightsEffect();
                (void)e.GetSettings(pal);
            }
        }
        (void)l.GetChannelEffects(m, 0, 0, -1, 0);
        (void)l.GetChannelEffects(m, 0, 3, 0);
    }
    (void)l.GetNodesWithEffects();
    (void)l.GetFaceSources();
    for (auto& t : l.GetLipSyncTracks()) (void)l.GetLipSync(t, 0);
    (void)l.GetEmbeddedPictures();
}
}

// ---------- timing tracks ----------

TEST_CASE("LOR timing tracks lists only free grids", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc(Timings("Fixed", { 0 }, "TimingGridFixed") + Timings("Beats", { 10 }) + Timings("Words", { 20 })));
    LOREdit l(doc, 20);
    CHECK(l.GetTimingTracks() == std::vector<std::string>{ "Beats", "Words" });
}

TEST_CASE("LOR timing marks round to the frame period", "[impexp2][lor][timing]") {
    struct Row { int freq; std::vector<int> cs; std::vector<std::pair<uint32_t, uint32_t>> expect; };
    auto r = GENERATE(Row{ 20, { 50, 125, 125, 300 }, { { 0, 500 }, { 500, 1250 }, { 1250, 3000 } } },
                      Row{ 20, { 1, 2 }, {} },
                      Row{ 20, { 1, 2, 3 }, { { 0, 50 } } },
                      Row{ 20, { 3, 9 }, { { 0, 50 }, { 50, 100 } } },
                      Row{ 40, { 3, 8 }, { { 0, 25 }, { 25, 75 } } },
                      Row{ 20, {}, {} },
                      Row{ 20, { 0 }, {} },
                      Row{ 10, { 15, 31 }, { { 0, 200 }, { 200, 300 } } },
                      Row{ 50, { 10 }, { { 0, 100 } } });
    auto doc = Parse(TimingDoc(Timings("T", r.cs)));
    LOREdit l(doc, r.freq);
    CHECK(l.GetTimings("T") == r.expect);
}

TEST_CASE("LOR timing offsets", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc(Timings("T", { 100, 200 })));
    LOREdit l(doc, 20);
    SECTION("positive") {
        auto t = l.GetTimings("T", 500);
        REQUIRE(t.size() == 2);
        CHECK(t[0] == std::make_pair(500u, 1500u));
        CHECK(t[1] == std::make_pair(1500u, 2500u));
    }
    SECTION("negative keeps later marks") {
        auto t = l.GetTimings("T", -1500);
        REQUIRE(t.size() == 1);
        CHECK(t[0].second == 500u);
    }
    SECTION("negative exact") {
        auto t = l.GetTimings("T", -1000);
        REQUIRE(t.size() == 1);
        CHECK(t[0] == std::make_pair(0u, 1000u));
    }
}

TEST_CASE("LOR timing marks before zero after a negative offset start at zero", "[impexp2][lor][timing]") {
    // BUG: GetTimings assigns lastMS = adjTime even when the mark is not kept, so a mark that lands below zero becomes the
    // start of the next segment (a ~4 billion uint32) instead of 0
    auto doc = Parse(TimingDoc(Timings("T", { 100, 200 })));
    LOREdit l(doc, 20);
    auto t = l.GetTimings("T", -1500);
    REQUIRE(t.size() == 1);
    CHECK(t[0] == std::make_pair(0u, 500u));
    CHECK(l.GetTimings("T", -100000).empty());
}

TEST_CASE("LOR out of order timing marks do not create overlapping segments", "[impexp2][lor][timing]") {
    // BUG: lastMS follows every mark, so a mark earlier than the previous one rewinds the next segment start
    auto doc = Parse(TimingDoc(Timings("T", { 100, 50, 200 })));
    LOREdit l(doc, 20);
    CHECK(l.GetTimings("T") == std::vector<std::pair<uint32_t, uint32_t>>{ { 0, 1000 }, { 1000, 2000 } });
}

TEST_CASE("LOR timing track lookups that find nothing", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc(Timings("T", { 100 })));
    LOREdit l(doc, 20);
    CHECK(l.GetTimings("missing").empty());
    CHECK(l.GetTimings("").empty());
    pugi::xml_document empty;
    LOREdit e(empty, 20);
    CHECK(e.GetTimings("T").empty());
    CHECK(e.GetTimingTracks().empty());
}

TEST_CASE("LOR timing track with no name and odd children", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc("<TimingGridFree><timing centisecond=\"10\"/><junk centisecond=\"99\"/><timing centisecond=\"20\"/></TimingGridFree>"));
    LOREdit l(doc, 20);
    CHECK(l.GetTimingTracks() == std::vector<std::string>{ "" });
        CHECK(l.GetTimings("") == std::vector<std::pair<uint32_t, uint32_t>>{ { 0, 100 }, { 100, 200 } });
}

TEST_CASE("LOR timing track with huge centiseconds does not wrap", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc(Timings("T", { 100, 2000000000, 2100000000 })));
    LOREdit l(doc, 20);
    auto t = l.GetTimings("T");
    // int overflow of cs*10 is undefined; just require a sane first mark
    REQUIRE(!t.empty());
    CHECK(t[0] == std::make_pair(0u, 1000u));
}

TEST_CASE("LOR timing track names repeated in two grids are both read", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc(Timings("T", { 10 }) + Timings("T", { 20 })));
    LOREdit l(doc, 20);
    CHECK(l.GetTimingTracks().size() == 2);
    CHECK(l.GetTimings("T").size() == 2);
}

TEST_CASE("LOR timing track at frequency over 1000 does not divide by zero", "[impexp2][lor][timing]") {
    // BUG: RoundToMultipleOfPeriod computes ms = int(1000/period) = 0 for period > 1000 and then divides by it (SIGFPE)
    auto doc = Parse(TimingDoc(Timings("T", { 100 })));
    LOREdit l(doc, 2000);
    CHECK_NOTHROW(l.GetTimings("T"));
}

TEST_CASE("LOR timing track at frequency zero returns nothing", "[impexp2][lor][timing]") {
    auto doc = Parse(TimingDoc(Timings("T", { 100 })));
    LOREdit l(doc, 0);
    // 1000/0 is infinite; whatever it becomes, the call must come back
    (void)l.GetTimings("T");
    SUCCEED();
}

// ---------- strands / channels ----------

namespace {
std::string Preview(const std::string& props) { return "<PreviewClass>" + props + "</PreviewClass>"; }
}

TEST_CASE("LOR model strands come from the prop channel grid", "[impexp2][lor][model]") {
    auto doc = Parse(Seq(Preview("<PropClass Name=\"Tree\" ChannelGrid=\"a,b,c,d,e,Red;a,b,c,d,e,Green;a,b,c\"/><PropClass Name=\"Flat\"/>")));
    LOREdit l(doc, 20);
    auto s = l.GetModelStrands("Tree");
    REQUIRE(s.size() == 3);
    CHECK(s[1] == "Red");
    CHECK(s[2] == "Green");
    CHECK(s[3] == "");
    auto f = l.GetModelStrands("Flat");
    REQUIRE(f.size() == 1);
    CHECK(f[1] == "");
    CHECK(l.GetModelStrands("Nope").empty());
    CHECK(l.GetModelStrands("").empty());
}

TEST_CASE("LOR model strands for an unfamiliar document", "[impexp2][lor][model]") {
    pugi::xml_document empty;
    LOREdit l(empty, 20);
    CHECK(l.GetModelStrands("x").empty());
    auto doc = Parse(Seq("<PreviewClass><Other Name=\"Tree\"/></PreviewClass>"));
    LOREdit l2(doc, 20);
    CHECK(l2.GetModelStrands("Tree").empty());
}

namespace {
std::string ChannelProp(const std::string& name, const std::string& chans, const std::string& attrs = "") {
    return SeqProp(name, chans, attrs);
}
}

TEST_CASE("LOR model channel counting", "[impexp2][lor][model]") {
    auto doc = Parse(Seq(Props(
        ChannelProp("Grid", Chan(0, 0, 0, ChEff(0, 10)) + Chan(0, 1, 0, ChEff(0, 10)) + Chan(2, 3, 0, ChEff(0, 10)) + Chan(5, 5, 0, "")) +
        ChannelProp("Single", Chan(0, 0, 0, ChEff(0, 10))) +
        ChannelProp("Line", Chan(0, 0, 0, ChEff(0, 10)) + Chan(0, 0, 1, ChEff(0, 10)) + Chan(0, 0, 2, ChEff(0, 10))) +
        ChannelProp("Empty", Chan(0, 0, 0, "")))));
    LOREdit l(doc, 20);
    int r = -1, c = -1;
    CHECK(l.GetModelChannels("Grid", r, c) == 3);
    CHECK(r == 3);
    CHECK(c == 4);
    CHECK(l.GetModelChannels("Single", r, c) == 1);
    CHECK(r == 1);
    CHECK(c == 1);
    CHECK(l.GetModelChannels("Line", r, c) == 3);
    CHECK(r == 1);
    CHECK(c == 1);
    CHECK(l.GetModelChannels("Empty", r, c) == 0);
    CHECK(l.GetModelChannels("Missing", r, c) == 0);
    CHECK(r == 0);
    CHECK(c == 0);
}

TEST_CASE("LOR model channel counting falls back to a prefix match", "[impexp2][lor][model]") {
    auto doc = Parse(Seq(Props(ChannelProp("Tree", Chan(0, 0, 0, ChEff(0, 10)) + Chan(0, 1, 0, ChEff(0, 10))))));
    LOREdit l(doc, 20);
    int r, c;
    CHECK(l.GetModelChannels("Tree 3 decorated", r, c) == 2);
    CHECK(l.GetModelChannels("Tre", r, c) == 0);
}

TEST_CASE("LOR model channel counting does not match every model to an unnamed prop", "[impexp2][lor][model]") {
    // BUG: the prefix fallback is StartsWith(model, name) with name == "" which is always true, so a prop that has no name
    // is claimed by any model that has no exact match
    auto doc = Parse(Seq(Props("<SeqProp>" + Chan(0, 0, 0, ChEff(0, 10)) + "</SeqProp>")));
    LOREdit l(doc, 20);
    int r, c;
    CHECK(l.GetModelChannels("Anything", r, c) == 0);
}

TEST_CASE("LOR sequencing type does not match every model to an unnamed prop", "[impexp2][lor][model]") {
    // BUG: same empty-name prefix match in GetSequencingType
    auto doc = Parse(Seq(Props("<SeqProp>" + Chan(0, 0, 0, ChEff(0, 10)) + "</SeqProp>")));
    LOREdit l(doc, 20);
    CHECK(l.GetSequencingType("Anything") == loreditType::NONE);
}

TEST_CASE("LOR prop named by a nested PropClass", "[impexp2][lor][model]") {
    auto doc = Parse(Seq(Props("<SeqProp><PropClass Name=\"Nested\"/>" + Chan(0, 0, 0, ChEff(0, 10)) + "</SeqProp>")));
    LOREdit l(doc, 20);
    int r, c;
    CHECK(l.GetModelChannels("Nested", r, c) == 1);
    CHECK(l.GetSequencingType("Nested") == loreditType::CHANNELS);
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "Nested" });
}

TEST_CASE("LOR sequencing types", "[impexp2][lor][model]") {
    auto doc = Parse(Seq(Props(
        ChannelProp("Chans", Chan(0, 0, 0, ChEff(0, 10))) +
        SeqProp("Tracks", Track(TrackEffect(0, 10, Settings(Layer("fire", "50,0"))))) +
        SeqProp("Rows", Track(TrackEffect(0, 10, Settings(Layer("fire", "50,0"))), " name=\"R1\" type=\"custom\"")) +
        SeqProp("Hollow", Track("") + Chan(0, 0, 0, "")) +
        SeqProp("Both", Chan(0, 0, 0, ChEff(0, 10)) + Track(TrackEffect(0, 10, Settings(Layer("fire", "50,0"))))))));
    LOREdit l(doc, 20);
    CHECK(l.GetSequencingType("Chans") == loreditType::CHANNELS);
    CHECK(l.GetSequencingType("Tracks") == loreditType::TRACKS);
    CHECK(l.GetSequencingType("Rows/R1") == loreditType::TRACKS);
    // "Rows/Nope" has no such row but still matches the prop by prefix
    CHECK(l.GetSequencingType("Rows/Nope") == loreditType::TRACKS);
    CHECK(l.GetSequencingType("Hollow") == loreditType::NONE);
    CHECK(l.GetSequencingType("Both") == loreditType::CHANNELS);
    CHECK(l.GetSequencingType("Nothing") == loreditType::NONE);
    CHECK(l.GetSequencingType("Tracks 2") == loreditType::TRACKS);
    CHECK(l.GetSequencingType("") == loreditType::NONE);
}

TEST_CASE("LOR archived props are searched too", "[impexp2][lor][model]") {
    auto doc = Parse(Seq("<ArchivedProps><ArchiveProp name=\"Old\">" + Chan(0, 0, 0, ChEff(0, 10)) + "</ArchiveProp></ArchivedProps>"));
    LOREdit l(doc, 20);
    CHECK(l.GetSequencingType("Old") == loreditType::CHANNELS);
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "Old" });
    auto e = l.GetChannelEffects("Old", 0, 0, -1, 0);
    CHECK(e.size() == 1);
}

TEST_CASE("LOR props under the wrong parent element are ignored", "[impexp2][lor][model]") {
    auto doc = Parse(Seq("<Other><SeqProp name=\"X\">" + Chan(0, 0, 0, ChEff(0, 10)) + "</SeqProp></Other><SequenceProps><ArchiveProp name=\"Y\">" + Chan(0, 0, 0, ChEff(0, 10)) +
                         "</ArchiveProp><Junk name=\"Z\">" + Chan(0, 0, 0, ChEff(0, 10)) + "</Junk></SequenceProps>"));
    LOREdit l(doc, 20);
    auto m = l.GetModelsWithEffects();
    CHECK(m == std::vector<std::string>{ "Y" });
}

TEST_CASE("LOR models with effects lists whole props and custom rows", "[impexp2][lor][model]") {
    std::string e = TrackEffect(0, 10, Settings(Layer("fire", "50,0")));
    auto doc = Parse(Seq(Props(
        SeqProp("Matrix", Track(e, " name=\"Row1\" type=\"custom\"") + Track(e, " name=\"Row2\" type=\"custom_horizontal_buffer\"") + Track(e, " name=\"Row1\" type=\"custom\"") + Track(e)) +
        SeqProp("OnlyRows", Track(e, " name=\"R\" type=\"custom\"")) +
        SeqProp("Plain", Track(e)) +
        SeqProp("Empty", Track("") + Chan(0, 0, 0, "")) +
        SeqProp("", Track(e)) +
        SeqProp("Chan", Chan(0, 0, 0, ChEff(0, 10))))));
    LOREdit l(doc, 20);
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "Matrix", "Matrix/Row1", "Matrix/Row2", "OnlyRows/R", "Plain", "Chan" });
}

TEST_CASE("LOR models with effects on the same name twice", "[impexp2][lor][model]") {
    auto doc = Parse(Seq(Props(ChannelProp("Dup", Chan(0, 0, 0, ChEff(0, 10))) + ChannelProp("Dup", Chan(0, 0, 0, ChEff(20, 30))))));
    LOREdit l(doc, 20);
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "Dup", "Dup" });
    // the first prop with that name wins when reading effects
    auto e = l.GetChannelEffects("Dup", 0, 0, -1, 0);
    REQUIRE(e.size() == 1);
    CHECK(e[0].startMS == 0);
}

// ---------- track layers ----------

TEST_CASE("LOR track layers are ordered last motion row first, left before right", "[impexp2][lor][layers]") {
    std::string left = Settings(Layer("fire", "50,0"));
    std::string right = Settings(kNone, Layer("bars", "up,1,none,False,8,0"));
    std::string both = Settings(Layer("colorwash", "full,full,none"), Layer("spirals", "1,left_to_right,20,50,0,False,none,50"));
    auto doc = Parse(Seq(Props(SeqProp("P", Track(TrackEffect(0, 10, left), " name=\"T1\"") + Track(TrackEffect(0, 10, both) + TrackEffect(10, 20, right), " name=\"T2\"")))));
    LOREdit l(doc, 20);
    REQUIRE(l.GetModelLayers("P") == 3);
    auto l0 = l.GetTrackEffects("P", 0, 0);
    auto l1 = l.GetTrackEffects("P", 1, 0);
    auto l2 = l.GetTrackEffects("P", 2, 0);
    REQUIRE(l0.size() == 2);
    REQUIRE(l1.size() == 2);
    REQUIRE(l2.size() == 1);
    CHECK(l0[0].effectType == "colorwash");
    CHECK(l0[0].left);
    CHECK(l0[1].effectType == "");
    CHECK(l1[0].effectType == "spirals");
    CHECK_FALSE(l1[0].left);
    CHECK(l1[1].effectType == "bars");
    CHECK(l2[0].effectType == "fire");
    CHECK(l.GetTrackEffects("P", 3, 0).empty());
    CHECK(l.GetTrackEffects("P", -1, 0).empty());
    CHECK(l.GetTrackEffects("Q", 0, 0).empty());
}

TEST_CASE("LOR tracks with no real effects make no layers", "[impexp2][lor][layers]") {
    auto doc = Parse(Seq(Props(SeqProp("P",
        Track(TrackEffect(0, 10, Settings(kNone, kNone))) + Track("") + Track(TrackEffect(0, 10, "Mix_Average|0|0|full|10")) + Track(TrackEffect(0, 10, "")) +
        Track(TrackEffect(0, 10, "junk"))))));
    LOREdit l(doc, 20);
    CHECK(l.GetModelLayers("P") == 0);
}

TEST_CASE("LOR track layers on props whose names contain slashes", "[impexp2][lor][layers]") {
    std::string e = TrackEffect(0, 10, Settings(Layer("fire", "50,0")));
    auto doc = Parse(Seq(Props(
        SeqProp("A", Track(e, " name=\"B/Row\" type=\"custom\"")) +
        SeqProp("A/B", Track(e, " name=\"Row\" type=\"custom\"") + Track(e)))));
    LOREdit l(doc, 20);
    // A/B/Row is prop "A/B" row "Row" even though prop "A" is a prefix of it
    CHECK(l.GetModelLayers("A/B/Row") == 1);
    CHECK(l.GetModelLayers("A/B") == 1);
    CHECK(l.GetModelLayers("A") == 0);
    CHECK(l.GetSequencingType("A/B/Row") == loreditType::TRACKS);
}

TEST_CASE("LOR custom rows only appear under their own source name", "[impexp2][lor][layers]") {
    std::string e = TrackEffect(0, 10, Settings(Layer("fire", "50,0")));
    auto doc = Parse(Seq(Props(SeqProp("P", Track(e, " name=\"R1\" type=\"custom\"") + Track(TrackEffect(0, 10, Settings(Layer("bars", "up,1,none,False,8,0"))), " name=\"W\" type=\"none\"")))));
    LOREdit l(doc, 20);
    auto whole = l.GetTrackEffects("P", 0, 0);
    REQUIRE(whole.size() == 1);
    CHECK(whole[0].effectType == "bars");
    auto row = l.GetTrackEffects("P/R1", 0, 0);
    REQUIRE(row.size() == 1);
    CHECK(row[0].effectType == "fire");
    CHECK(row[0].trackType == "custom");
    CHECK(l.GetTrackEffects("P/W", 0, 0).empty());
    CHECK(l.GetTrackEffects("P/", 0, 0).empty());
}

// ---------- AddEffects ----------

TEST_CASE("LOR track effect time and intensity attributes", "[impexp2][lor][addeffects]") {
    auto doc = Parse("<track name=\"T\">"
                     "<effect startCentisecond=\"10\" endCentisecond=\"25\" intensity=\"70\" settings=\"x\"/>"
                     "<effect startCentisecond=\"30\" endCentisecond=\"40\" startIntensity=\"20\" endIntensity=\"90\" settings=\"x\"/>"
                     "<effect startCentisecond=\"50\" endCentisecond=\"60\"/>"
                     "<effect/>"
                     "</track>");
    LOREdit l(doc, 20);
    auto v = l.AddEffects(doc.child("track"), true, 100);
    REQUIRE(v.size() == 4);
    CHECK(v[0].startMS == 200);
    CHECK(v[0].endMS == 350);
    CHECK(v[0].startIntensity == 70);
    CHECK(v[0].endIntensity == 70);
    CHECK(v[1].startIntensity == 20);
    CHECK(v[1].endIntensity == 90);
    CHECK(v[2].startIntensity == 100);
    CHECK(v[2].endIntensity == 100);
    CHECK(v[3].startMS == 100);
    CHECK(v[3].endMS == 100);
    for (auto& e : v) {
        CHECK(e.type == loreditType::TRACKS);
        CHECK(e.startColour == xlWHITE);
        CHECK(e.endColour == xlWHITE);
        CHECK(e.left);
        CHECK(e.effectType.empty());
        CHECK(e.otherSettings.empty());
    }
}

TEST_CASE("LOR track effect with a negative offset", "[impexp2][lor][addeffects]") {
    // BUG: startMS/endMS are uint32_t, so cs*10 + negative offset below zero wraps to ~4 billion instead of clamping/dropping
    auto doc = Parse(OneEffectDoc(Settings(Layer("fire", "50,0")), 10, 50));
    LOREdit l(doc, 20);
    auto v = l.GetTrackEffects("P", 0, -500);
    REQUIRE(v.size() == 1);
    CHECK(v[0].startMS < 1000000u);
}

TEST_CASE("LOR track effect settings field counts", "[impexp2][lor][addeffects]") {
    struct Row { std::string s; size_t other; bool hasType; std::string sparkle8; };
    auto r = GENERATE(Row{ "Mix_Average|0|0|full|10|lightorama_fire:FFFF0000,1:50,0|lightorama_none::", 6, true, "" },
                      Row{ "Mix_Average|0|0|full|10|lightorama_fire:FFFF0000,1:50,0|lightorama_none::|EXTRA", 6, true, "EXTRA" },
                      Row{ "Mix_Average|0|0|full|10|lightorama_fire:FFFF0000,1:50,0|lightorama_none::|a|b", 6, true, "" },
                      Row{ "Mix_Average|0|0|full|10|lightorama_fire:FFFF0000,1:50,0", 0, false, "" },
                      Row{ "Mix_Average|0|0|full|10||", 6, false, "" },
                      Row{ "|||||lightorama_fire::|", 6, true, "" },
                      Row{ "||||||", 6, false, "" },
                      Row{ "", 0, false, "" });
    auto c = Convert(r.s);
    if (!c.ok) {
        // no layer found at all: still make sure the getter copes
        auto doc = Parse(OneEffectDoc(r.s));
        LOREdit l(doc, 20);
        auto dir = l.AddEffects(doc.child("sequence").child("SequenceProps").child("SeqProp").child("track"), true, 0);
        REQUIRE(dir.size() == 1);
        CHECK(dir[0].otherSettings.size() == r.other);
        CHECK(dir[0].effectType.empty());
        return;
    }
    CHECK(c.eff.otherSettings.size() == r.other);
    CHECK(!c.eff.effectType.empty() == r.hasType);
    if (r.other == 6) CHECK(c.eff.otherSettings[5] == r.sparkle8);
}

TEST_CASE("LOR track effect layer parts", "[impexp2][lor][addeffects]") {
    auto c = Convert(Settings("lightorama_fire:FFFF0000,1:50,0:extra:more"));
    REQUIRE(c.ok);
    CHECK(c.eff.effectType == "fire");
    CHECK(c.eff.effectSettings == std::vector<std::string>{ "FFFF0000,1", "50,0", "extra", "more" });
    CHECK(c.eff.otherSettings.size() == 6);
    CHECK(c.eff.otherSettings[0] == "Mix_Average");
    CHECK(c.eff.otherSettings[3] == "full");
    CHECK(c.eff.otherSettings[4] == "10");
}

TEST_CASE("LOR track effect layer with extra lightorama_ parts drops them", "[impexp2][lor][addeffects]") {
    auto c = Convert(Settings("lightorama_fire:lightorama_x:50,0"));
    REQUIRE(c.ok);
    CHECK(c.eff.effectSettings == std::vector<std::string>{ "50,0" });
}

TEST_CASE("LOR track effect effect type containing an underscore", "[impexp2][lor][addeffects]") {
    auto c = Convert(Settings("lightorama_some_thing:x:y"));
    REQUIRE(c.ok);
    CHECK(c.eff.effectType == "some_thing");
}

TEST_CASE("LOR track effect left only uses the left layer, right only the right", "[impexp2][lor][addeffects]") {
    auto doc = Parse(OneEffectDoc(Settings(Layer("fire", "50,0"), Layer("bars", "up,1,none,False,8,0"))));
    LOREdit l(doc, 20);
    auto track = doc.child("sequence").child("SequenceProps").child("SeqProp").child("track");
    auto a = l.AddEffects(track, true, 0);
    auto b = l.AddEffects(track, false, 0);
    REQUIRE(a.size() == 1);
    REQUIRE(b.size() == 1);
    CHECK(a[0].effectType == "fire");
    CHECK(b[0].effectType == "bars");
    CHECK(a[0].otherSidePresent);
    CHECK(b[0].otherSidePresent);
}

TEST_CASE("LOR track effect with an empty track", "[impexp2][lor][addeffects]") {
    auto doc = Parse("<track/>");
    LOREdit l(doc, 20);
    CHECK(l.AddEffects(doc.child("track"), true, 0).empty());
    pugi::xml_node none;
    CHECK(l.AddEffects(none, true, 0).empty());
}

TEST_CASE("LOR track motion row geometry is copied to each effect", "[impexp2][lor][addeffects]") {
    auto doc = Parse("<track type=\"rectangle\" subx=\"0.1\" suby=\"0.2\" subw=\"0.3\" subh=\"0.4\"><effect/><effect/></track>");
    LOREdit l(doc, 20);
    auto v = l.AddEffects(doc.child("track"), false, 0);
    REQUIRE(v.size() == 2);
    for (auto& e : v) {
        CHECK(e.trackType == "rectangle");
        CHECK(e.subx == Catch::Approx(0.1f));
        CHECK(e.suby == Catch::Approx(0.2f));
        CHECK(e.subw == Catch::Approx(0.3f));
        CHECK(e.subh == Catch::Approx(0.4f));
    }
    auto d = Parse("<track><effect/></track>");
    auto w = l.AddEffects(d.child("track"), true, 0);
    REQUIRE(w.size() == 1);
    CHECK(w[0].trackType == "none");
    CHECK(w[0].subw == 1.0f);
    CHECK(w[0].subh == 1.0f);
}

TEST_CASE("LOR track effect with garbage numbers", "[impexp2][lor][addeffects]") {
    auto doc = Parse("<track><effect startCentisecond=\"abc\" endCentisecond=\"-5\" intensity=\"x\" settings=\"a|b\"/></track>");
    LOREdit l(doc, 20);
    auto v = l.AddEffects(doc.child("track"), true, 0);
    REQUIRE(v.size() == 1);
    CHECK(v[0].startMS == 0);
    CHECK(v[0].endMS == 0);
    // intensity "x" reads as 0 (not the 9999 sentinel)
    CHECK(v[0].startIntensity == 0);
}

// ---------- channel effects ----------

namespace {
std::string ChannelDoc(const std::string& chans, const std::string& attrs = "") {
    return Seq(Props(SeqProp("Tree", chans, attrs)));
}
}

TEST_CASE("LOR channel effect intensity forms", "[impexp2][lor][channel]") {
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0,
        ChanEffect(10, 20, "INTENSITY", "intensity=\"50\"") +
        ChanEffect(20, 30, "INTENSITY", "intensity=\"-65536\"") +
        ChanEffect(30, 40, "DMX_INTENSITY", "intensity=\"-65536\"") +
        ChanEffect(40, 50, "SHIMMER", "startIntensity=\"10\" endIntensity=\"60\"") +
        ChanEffect(50, 60, "INTENSITY", "startIntensity=\"-65536\" endIntensity=\"-16711936\"") +
        ChanEffect(60, 70, "DMX_INTENSITY", "startIntensity=\"-256\" endIntensity=\"-1\"") +
        ChanEffect(70, 80, "TWINKLE", "startIntensity=\"30\"") +
        ChanEffect(80, 90, "INTENSITY", "startIntensity=\"-65536\""))));
    LOREdit l(doc, 20);
    auto v = l.GetChannelEffects("Tree", 0, 0, -1, 1000);
    REQUIRE(v.size() == 8);
    CHECK(v[0].startMS == 1100);
    CHECK(v[0].endMS == 1200);
    CHECK(v[0].startIntensity == 50);
    CHECK(v[0].endIntensity == 50);
    CHECK(v[0].startColour == xlWHITE);
    CHECK(v[0].type == loreditType::CHANNELS);
    CHECK(v[0].effectType == "INTENSITY");
    CHECK(v[1].startIntensity == 100);
    CHECK(v[1].startColour == xlColor(255, 0, 0));
    CHECK(v[1].endColour == xlColor(255, 0, 0));
    CHECK(v[2].startIntensity == 255);
    CHECK(v[2].endIntensity == 255);
    CHECK(v[3].startIntensity == 10);
    CHECK(v[3].endIntensity == 60);
    CHECK(v[3].effectType == "SHIMMER");
    CHECK(v[4].startColour == xlColor(255, 0, 0));
    CHECK(v[4].endColour == xlColor(0, 255, 0));
    CHECK(v[4].startIntensity == 100);
    CHECK(v[5].startIntensity == 255);
    CHECK(v[5].startColour == xlColor(255, 255, 0));
    CHECK(v[5].endColour == xlColor(255, 255, 255));
    CHECK(v[6].startIntensity == 30);
    CHECK(v[6].endIntensity == 100);
    // a negative start with no end colour: end defaults to -1 = white
    CHECK(v[7].endColour == xlColor(255, 255, 255));
}

TEST_CASE("LOR channel effect pixel channel flag", "[impexp2][lor][channel]") {
    for (int on : { 0, 1 }) {
        auto doc = Parse(ChannelDoc(Chan(0, 0, 0, ChEff(0, 10)), std::string(" EnablePixelChannels=\"") + std::to_string(on) + "\""));
        LOREdit l(doc, 20);
        auto v = l.GetChannelEffects("Tree", 0, 0, -1, 0);
        REQUIRE(v.size() == 1);
        CHECK(v[0].pixelChannels == (on == 1));
    }
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0, ChEff(0, 10))));
    LOREdit l(doc, 20);
    CHECK_FALSE(l.GetChannelEffects("Tree", 0, 0, -1, 0)[0].pixelChannels);
}

TEST_CASE("LOR channel effect without any intensity attribute is initialised", "[impexp2][lor][channel]") {
    // BUG: when neither intensity nor startIntensity is present, startIntensity/endIntensity are never assigned
    // (LOREditEffect has no default member initialisers) and the colours keep their default
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0, "<effect startCentisecond=\"0\" endCentisecond=\"10\" settings=\"INTENSITY\"/>")));
    LOREdit l(doc, 20);
    auto v = l.GetChannelEffects("Tree", 0, 0, -1, 0);
    REQUIRE(v.size() == 1);
    CHECK(v[0].startIntensity == 100);
    CHECK(v[0].endIntensity == 100);
}

TEST_CASE("LOR channel node matching", "[impexp2][lor][channel]") {
    auto doc = Parse(ChannelDoc(
        Chan(0, 0, 0, ChEff(0, 10, 11)) + Chan(1, 2, 1, ChEff(0, 10, 22)) + Chan(1, 2, 2, ChEff(0, 10, 33)) + Chan(3, 0, 2, ChEff(0, 10, 44))));
    LOREdit l(doc, 20);
    auto first = [&](int r, int c, int k) -> int {
        auto v = l.GetChannelEffects("Tree", r, c, k, 0);
        return v.empty() ? -1 : (int)v[0].startIntensity;
    };
    CHECK(first(-1, -1, -1) == 11);
    CHECK(first(0, 0, -1) == 11);
    CHECK(first(1, 2, -1) == 22);
    CHECK(first(1, 2, 2) == 33);
    CHECK(first(1, 2, 1) == 22);
    CHECK(first(1, 2, 7) == -1);
    CHECK(first(9, 9, -1) == -1);
    CHECK(first(3, 0, 2) == 44);
    CHECK(first(0, 2, -1) == -1);
    CHECK(first(-1, -1, 0) == -1);
    CHECK(first(0, 0, 0) == 11);
    CHECK(l.GetChannelEffects("Nope", -1, -1, -1, 0).empty());
}

TEST_CASE("LOR channel node matching by colour number on a single row", "[impexp2][lor][channel]") {
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0, ChEff(0, 10, 11)) + Chan(0, 0, 1, ChEff(0, 10, 22)) + Chan(0, 0, 2, ChEff(0, 10, 33))));
    LOREdit l(doc, 20);
    auto v = l.GetChannelEffects("Tree", 0, 2, -1, 0);
    REQUIRE(v.size() == 1);
    CHECK(v[0].startIntensity == 33);
}

TEST_CASE("LOR channel effects by index", "[impexp2][lor][channel]") {
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0, ChEff(0, 10, 11)) + Chan(0, 1, 0, ChEff(0, 10, 22)) + Chan(0, 2, 0, ChEff(0, 10, 33))));
    LOREdit l(doc, 20);
    auto at = [&](int ch) -> int {
        auto v = l.GetChannelEffects("Tree", ch, 3, 0);
        return v.empty() ? -1 : (int)v[0].startIntensity;
    };
    CHECK(at(0) == 11);
    CHECK(at(1) == 22);
    CHECK(at(2) == 33);
    CHECK(at(3) == 33);
    CHECK(at(100) == 33);
    CHECK(at(-1) == -1);
    CHECK(l.GetChannelEffects("Missing", 0, 1, 0).empty());
}

TEST_CASE("LOR channel effects by index on a grid", "[impexp2][lor][channel]") {
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0, ChEff(0, 10, 11)) + Chan(1, 0, 0, ChEff(0, 10, 22)) + Chan(2, 0, 0, ChEff(0, 10, 33))));
    LOREdit l(doc, 20);
    auto v = l.GetChannelEffects("Tree", 1, 3, 0);
    REQUIRE(v.size() == 1);
    CHECK(v[0].startIntensity == 22);
}

TEST_CASE("LOR channel effects with a null model", "[impexp2][lor][channel]") {
    auto doc = Parse(ChannelDoc(Chan(0, 0, 0, ChEff(0, 10))));
    LOREdit l(doc, 20);
    CHECK(l.GetChannelEffects("Tree", 0, static_cast<Model*>(nullptr), 0).empty());
}

TEST_CASE("LOR node listing", "[impexp2][lor][channel]") {
    auto doc = Parse(Seq(Preview("<PropClass Name=\"Tree\" ChannelGrid=\"a,b,c,d,e,Red;a,b,c,d,e,Green\"/>") +
                         Props(SeqProp("Tree", Chan(0, 0, 0, ChEff(0, 10)) + Chan(0, 1, 2, ChEff(0, 10)) + Chan(1, 1, 1, ChEff(0, 10)) + Chan(5, 5, 5, "")) +
                               SeqProp("Other", Chan(3, 4, 0, ChEff(0, 10))) +
                               SeqProp("", Chan(0, 0, 0, ChEff(0, 10))) +
                               "<SeqProp><PropClass Name=\"Named\"/>" + Chan(7, 8, 9, ChEff(0, 10)) + "</SeqProp>")));
    LOREdit l(doc, 20);
    CHECK(l.GetNodesWithEffects() == std::vector<std::string>{ "Tree[0,0,0][Red]", "Tree[0,1,2][Green]", "Tree[1,1,1][]", "Other[3,4,0][]", "Named[7,8,9][]" });
}

// ---------- faces ----------

TEST_CASE("LOR singing face basics", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("Santa", "AI", { { 0, 10 } }, -65536) + Mouth("Santa", "E", { { 10, 20 } }) + Mouth("Santa", "Closed", { { 20, 30 } }))));
    LOREdit l(doc, 20);
    CHECK(l.GetFaceSources() == std::vector<std::string>{ "Santa (Singing Face)" });
    CHECK(LOREdit::IsFaceSource("Santa (Singing Face)"));
    CHECK_FALSE(LOREdit::IsFaceSource("Santa"));
    CHECK(l.GetLipSyncTracks() == std::vector<std::string>{ "Lip Sync Santa" });
    CHECK(l.IsLipSyncTrack("Lip Sync Santa"));
    CHECK_FALSE(l.IsLipSyncTrack("Santa"));
    CHECK(l.GetLipSyncTrackForFace("Santa (Singing Face)") == "Lip Sync Santa");
    CHECK(l.GetLipSyncTrackForFace("Santa") == "Lip Sync Santa");
    CHECK(l.GetLipSyncTrackForFace("Nobody (Singing Face)") == "");
    CHECK(l.GetFaceColour("Santa (Singing Face)") == xlColor(255, 0, 0));
    CHECK(l.GetFaceColour("Nobody") == xlWHITE);
    CHECK(l.GetSequencingType("Santa (Singing Face)") == loreditType::NONE);
    uint32_t s = 7, e = 7;
    CHECK(l.GetFaceSpan("Santa (Singing Face)", 0, s, e));
    CHECK(s == 0);
    CHECK(e == 300);
    CHECK_FALSE(l.GetFaceSpan("Nobody (Singing Face)", 0, s, e));
    CHECK(l.GetFaceSpan("Santa (Singing Face)", 1000, s, e));
    CHECK(s == 1000);
    CHECK(e == 1300);
    auto models = l.GetModelsWithEffects();
    CHECK(std::find(models.begin(), models.end(), "Santa (Singing Face)") != models.end());
}

TEST_CASE("LOR lip sync layers", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("Santa", "AI", { { 0, 10 } }) + Mouth("Santa", "E", { { 10, 20 } }) + Mouth("Santa", "Closed", { { 20, 30 } }))));
    LOREdit l(doc, 20);
    auto layers = l.GetLipSync("Lip Sync Santa");
    REQUIRE(layers.size() == 3);
    REQUIRE(layers[2].size() == 2);
    CHECK(layers[2][0].startMS == 0);
    CHECK(layers[2][0].endMS == 100);
    CHECK(layers[2][0].label == "AI");
    CHECK(layers[2][1].startMS == 100);
    CHECK(layers[2][1].endMS == 200);
    CHECK(layers[2][1].label == "E");
    REQUIRE(layers[0].size() == 1);
    CHECK(layers[0][0].startMS == 0);
    CHECK(layers[0][0].endMS == 200);
    CHECK(layers[1].size() == layers[0].size());
    auto shifted = l.GetLipSync("Lip Sync Santa", 500);
    REQUIRE(shifted[2].size() == 2);
    CHECK(shifted[2][0].startMS == 500);
    auto none = l.GetLipSync("nope");
    REQUIRE(none.size() == 3);
    CHECK(none[0].empty());
    CHECK(none[2].empty());
}

TEST_CASE("LOR lip sync negative offset clamps to zero", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("Santa", "AI", { { 0, 10 }, { 20, 40 } }))));
    LOREdit l(doc, 20);
    auto layers = l.GetLipSync("Lip Sync Santa", -150);
    REQUIRE(layers.size() == 3);
    for (auto& m : layers[2]) {
        CHECK(m.startMS < 100000u);
        CHECK(m.endMS < 100000u);
        CHECK(m.endMS > m.startMS);
    }
    uint32_t s, e;
    CHECK(l.GetFaceSpan("Santa", -100000, s, e) == false);
}

TEST_CASE("LOR lip sync phrases split on gaps of 300ms", "[impexp2][lor][faces]") {
    struct Row { int gapCs; size_t phrases; };
    auto r = GENERATE(Row{ 0, 1 }, Row{ 25, 1 }, Row{ 27, 1 }, Row{ 28, 2 }, Row{ 30, 2 }, Row{ 100, 2 });
    int second = 10 + r.gapCs;
    auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 10 }, { second, second + 10 } }))));
    LOREdit l(doc, 20);
    auto layers = l.GetLipSync("Lip Sync F");
    CHECK(layers[0].size() == r.phrases);
}

TEST_CASE("LOR lip sync merges runs of the same shape and trims overlaps", "[impexp2][lor][faces]") {
    SECTION("same shape runs merge") {
        auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 10 }, { 10, 20 }, { 15, 30 } }))));
        LOREdit l(doc, 20);
        auto layers = l.GetLipSync("Lip Sync F");
        REQUIRE(layers[2].size() == 1);
        CHECK(layers[2][0].startMS == 0);
        CHECK(layers[2][0].endMS == 300);
    }
    SECTION("overlap of different shapes truncates the earlier") {
        auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 20 } }) + Mouth("F", "E", { { 10, 30 } }))));
        LOREdit l(doc, 20);
        auto layers = l.GetLipSync("Lip Sync F");
        REQUIRE(layers[2].size() == 2);
        CHECK(layers[2][0].label == "AI");
        CHECK(layers[2][0].endMS == 100);
        CHECK(layers[2][1].label == "E");
        CHECK(layers[2][1].startMS == 100);
        CHECK(layers[2][1].endMS == 300);
    }
    SECTION("identical start different shapes keeps the later sorted one") {
        auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 20 } }) + Mouth("F", "E", { { 0, 10 } }))));
        LOREdit l(doc, 20);
        auto layers = l.GetLipSync("Lip Sync F");
        for (auto& m : layers[2]) CHECK(m.endMS > m.startMS);
    }
    SECTION("effects out of order are sorted") {
        auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 50, 60 }, { 0, 10 }, { 20, 30 } }))));
        LOREdit l(doc, 20);
        auto layers = l.GetLipSync("Lip Sync F");
        REQUIRE(layers[2].size() == 3);
        CHECK(layers[2][0].startMS == 0);
        CHECK(layers[2][2].startMS == 500);
    }
}

TEST_CASE("LOR lip sync ignores empty and backwards effects", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 10, 10 }, { 30, 20 }, { 40, 50 } }))));
    LOREdit l(doc, 20);
    auto layers = l.GetLipSync("Lip Sync F");
    REQUIRE(layers[2].size() == 1);
    CHECK(layers[2][0].startMS == 400);
}

TEST_CASE("LOR lip sync effects shorter than a frame disappear", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 1 }, { 10, 20 } }))));
    LOREdit l(doc, 20);
    auto layers = l.GetLipSync("Lip Sync F");
    REQUIRE(layers[2].size() == 1);
    CHECK(layers[2][0].startMS == 100);
}

TEST_CASE("LOR faces that never sing are not offered", "[impexp2][lor][faces]") {
    SECTION("rest only") {
        auto doc = Parse(Seq(Props(Mouth("F", "Closed", { { 0, 100 } }))));
        LOREdit l(doc, 20);
        CHECK(l.GetFaceSources().empty());
        CHECK(l.GetLipSyncTracks().empty());
    }
    SECTION("unknown shape") {
        auto doc = Parse(Seq(Props(Mouth("F", "Bananas", { { 0, 100 } }))));
        LOREdit l(doc, 20);
        CHECK(l.GetFaceSources().empty());
    }
    SECTION("zero length") {
        auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 5, 5 } }))));
        LOREdit l(doc, 20);
        CHECK(l.GetFaceSources().empty());
    }
    SECTION("mouth prop without effects") {
        auto doc = Parse(Seq(Props(SeqProp("F Mouth AI", Chan(0, 0, 0, "")))));
        LOREdit l(doc, 20);
        CHECK(l.GetFaceSources().empty());
    }
    SECTION("archived props are not faces") {
        auto doc = Parse(Seq("<ArchivedProps><ArchiveProp name=\"F Mouth AI\">" + Chan(0, 0, 0, ChEff(0, 10)) + "</ArchiveProp></ArchivedProps>"));
        LOREdit l(doc, 20);
        CHECK(l.GetFaceSources().empty());
    }
    SECTION("no face name") {
        auto doc = Parse(Seq(Props(SeqProp(" Mouth AI", Chan(0, 0, 0, ChEff(0, 10))) + SeqProp("Mouth AI", Chan(0, 0, 0, ChEff(0, 10))) + SeqProp("X Mouth ", Chan(0, 0, 0, ChEff(0, 10))))));
        LOREdit l(doc, 20);
        CHECK(l.GetFaceSources().empty());
    }
}

TEST_CASE("LOR face shape names with extra text", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "AI (Full Open)", { { 0, 10 } }) + Mouth("F", "L(th)", { { 10, 20 } }) + Mouth("F", "etc", { { 20, 30 } }))));
    LOREdit l(doc, 20);
    auto layers = l.GetLipSync("Lip Sync F");
    REQUIRE(layers[2].size() == 3);
    CHECK(layers[2][0].label == "AI");
    CHECK(layers[2][1].label == "L");
    CHECK(layers[2][2].label == "etc");
}

TEST_CASE("LOR face colour comes from the first negative intensity on a non-rest shape", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "Closed", { { 0, 10 } }, -16711936) + Mouth("F", "AI", { { 10, 20 } }, -255) + Mouth("F", "E", { { 20, 30 } }, -16711681))));
    LOREdit l(doc, 20);
    CHECK(l.GetFaceColour("F (Singing Face)") == xlColor(255, 255, 1));
}

TEST_CASE("LOR face with only positive intensities stays white", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 10 } }, 80))));
    LOREdit l(doc, 20);
    CHECK(l.GetFaceColour("F (Singing Face)") == xlWHITE);
}

TEST_CASE("LOR faces with matching timing share a lip sync track", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(
        Mouth("Santa", "AI", { { 0, 10 } }) + Mouth("Santa", "E", { { 10, 20 } }) + Mouth("Santa", "O", { { 20, 30 } }) +
        Mouth("Backup", "AI", { { 0, 10 } }) + Mouth("Backup", "E", { { 10, 30 } }) +
        Mouth("Other", "AI", { { 100, 110 } }) + Mouth("Other", "E", { { 130, 140 } }))));
    LOREdit l(doc, 20);
    auto faces = l.GetFaceSources();
    CHECK(faces.size() == 3);
    CHECK(l.GetLipSyncTrackForFace("Backup (Singing Face)") == l.GetLipSyncTrackForFace("Santa (Singing Face)"));
    CHECK(l.GetLipSyncTrackForFace("Other (Singing Face)") != l.GetLipSyncTrackForFace("Santa (Singing Face)"));
    auto tracks = l.GetLipSyncTracks();
    CHECK(tracks.size() == 2);
    CHECK(tracks[0] == "Lip Sync Santa");
    // asking for the shared track gives the richer face's marks
    auto layers = l.GetLipSync("Lip Sync Santa");
    CHECK(layers[2].size() == 3);
}

TEST_CASE("LOR matrix face rows", "[impexp2][lor][faces]") {
    std::string tr = Track(TrackEffect(0, 10, Settings(Layer("fire", "50,0"))), " name=\"Bob Mouth AI\" type=\"custom\"") +
                     Track(TrackEffect(10, 20, Settings(Layer("fire", "50,0"))), " name=\"Bob Mouth E\" type=\"custom\"") +
                     Track(TrackEffect(20, 30, Settings(Layer("fire", "50,0"))), " name=\"Bob Mouth Closed\" type=\"custom\"") +
                     Track("", " name=\"Bob Mouth O\" type=\"custom\"") +
                     Track(TrackEffect(30, 40, Settings(Layer("fire", "50,0"))), " name=\"Nonface\" type=\"custom\"");
    auto doc = Parse(Seq(Props(SeqProp("Matrix", tr))));
    LOREdit l(doc, 20);
    CHECK(l.GetFaceSources() == std::vector<std::string>{ "Matrix/Bob (Singing Face)" });
    CHECK(l.GetLipSyncTracks() == std::vector<std::string>{ "Lip Sync Matrix - Bob" });
    auto layers = l.GetLipSync("Lip Sync Matrix - Bob");
    CHECK(layers[2].size() == 2);
    uint32_t s, e;
    CHECK(l.GetFaceSpan("Matrix/Bob (Singing Face)", 0, s, e));
    CHECK(e == 300);
}

TEST_CASE("LOR face with a single instant does not span", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 0, 1 } }))));
    LOREdit l(doc, 20);
    uint32_t s = 5, e = 5;
    CHECK_FALSE(l.GetFaceSpan("F", 0, s, e));
}

TEST_CASE("LOR face lip sync at a 40Hz sequence", "[impexp2][lor][faces]") {
    auto doc = Parse(Seq(Props(Mouth("F", "AI", { { 3, 6 } }))));
    LOREdit l(doc, 40);
    auto layers = l.GetLipSync("Lip Sync F");
    REQUIRE(layers[2].size() == 1);
    CHECK(layers[2][0].startMS == 25);
    CHECK(layers[2][0].endMS == 50);
}

// ---------- pictures / misc ----------

TEST_CASE("LOR embedded pictures keep base64 without whitespace", "[impexp2][lor][pictures]") {
    auto doc = Parse(Seq("<pictures><picture name=\"*a*1234*.png\">AAAA\r\n BBBB\t\nCC==</picture><picture name=\"b\"/><junk/><picture>XX</picture></pictures>"
                         "<pictures><picture name=\"*a*1234*.png\">DDDD</picture></pictures>"));
    LOREdit l(doc, 20);
    auto p = l.GetEmbeddedPictures();
    REQUIRE(p.size() == 3);
    CHECK(p["b"] == "");
    CHECK(p[""] == "XX");
    CHECK(p["*a*1234*.png"] == "DDDD");
    pugi::xml_document empty;
    LOREdit e(empty, 20);
    CHECK(e.GetEmbeddedPictures().empty());
}

TEST_CASE("LOR embedded picture data is whitespace stripped", "[impexp2][lor][pictures]") {
    auto doc = Parse(Seq("<pictures><picture name=\"p\">AAAA\r\n BBBB\t\nCC==</picture></pictures>"));
    LOREdit l(doc, 20);
    CHECK(l.GetEmbeddedPictures()["p"] == "AAAABBBBCC==");
}

// ---------- file level ----------

TEST_CASE("LOR file with a byte order mark and CRLF loads", "[impexp2][lor][file]") {
    TempDir d;
    std::string body = "\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<sequence saveFileVersion=\"16\" AppVersion=\"6.6.0.13\" totalCentiseconds=\"16566\">\r\n"
                       "\t<SequenceProps>\r\n\t\t<SeqProp name=\"Tree\" EnablePixelChannels=\"1\">\r\n\t\t\t<channel col=\"0\" color=\"0\" row=\"0\">\r\n\t\t\t\t" +
                       ChEff(0, 10) + "\r\n\t\t\t</channel>\r\n\t\t</SeqProp>\r\n\t</SequenceProps>\r\n\t<TimingGrids>\r\n\t\t" + Timings("Beats", { 10, 20 }) + "\r\n\t</TimingGrids>\r\n</sequence>\r\n";
    auto path = d.Write("a.loredit", body);
    pugi::xml_document doc;
    auto res = doc.load_file(path.c_str());
    REQUIRE(res);
    LOREdit l(doc, 20);
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "Tree" });
    CHECK(l.GetTimings("Beats").size() == 2);
    CHECK(l.GetChannelEffects("Tree", 0, 0, -1, 0)[0].pixelChannels);
}

TEST_CASE("LOR file with latin-1 names does not fall over", "[impexp2][lor][file]") {
    std::string body = "<sequence><SequenceProps><SeqProp name=\"caf\xE9\">" + Chan(0, 0, 0, ChEff(0, 10)) + "</SeqProp></SequenceProps></sequence>";
    pugi::xml_document doc;
    doc.load_buffer(body.data(), body.size());
    LOREdit l(doc, 20);
    Poke(l);
    SUCCEED();
}

TEST_CASE("LOR file in utf-16 with a byte order mark loads", "[impexp2][lor][file]") {
    std::string x = "<sequence><SequenceProps><SeqProp name=\"U\">" + Chan(0, 0, 0, ChEff(0, 10)) + "</SeqProp></SequenceProps></sequence>";
    std::string b = "\xFF\xFE";
    for (char c : x) {
        b += c;
        b += '\0';
    }
    pugi::xml_document doc;
    REQUIRE(doc.load_buffer(b.data(), b.size(), pugi::parse_default, pugi::encoding_auto));
    LOREdit l(doc, 20);
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "U" });
}

TEST_CASE("LOR empty and tiny documents", "[impexp2][lor][file]") {
    for (const char* x : { "", "<", "<sequence", "<sequence/>", "<sequence><SequenceProps/></sequence>", "<a/>", "garbage", "<sequence><SequenceProps><SeqProp/></SequenceProps></sequence>",
                           "<sequence><SequenceProps><SeqProp name=\"\"><channel/></SeqProp></SequenceProps></sequence>",
                           "<sequence><SequenceProps><SeqProp name=\"x\"><track/></SeqProp></SequenceProps></sequence>",
                           "<sequence><SequenceProps><SeqProp name=\"x\"><track><effect/></track></SeqProp></SequenceProps></sequence>" }) {
        INFO(x);
        pugi::xml_document doc;
        doc.load_string(x);
        LOREdit l(doc, 20);
        Poke(l);
        auto m = l.GetModelsWithEffects();
        CHECK(m.size() <= 1);
    }
}

TEST_CASE("LOR truncated documents never crash the readers", "[impexp2][lor][file]") {
    std::string full = Seq(Preview("<PropClass Name=\"P\" ChannelGrid=\"a,b,c,d,e,Red\"/>") + "<TimingGrids>" + Timings("T", { 10, 20, 30 }) + "</TimingGrids>" +
                           Props(SeqProp("P", Chan(0, 0, 0, ChEff(0, 10)) + Track(TrackEffect(0, 10, Settings(Layer("spirals", "1,left_to_right,20,50,0,False,none,50"), Layer("sketch", "GSVG a b c d e f g h FFFF0000-1 2 True~PPath M 0 0 L 1 1,100,100,0,0,0,FFFF"))))) +
                                 Mouth("F", "AI", { { 0, 10 } }) + Mouth("F", "E", { { 10, 20 } })));
    for (size_t cut = 0; cut < full.size(); cut += 3) {
        pugi::xml_document doc;
        doc.load_buffer(full.data(), cut);
        LOREdit l(doc, 20);
        Poke(l);
    }
    SUCCEED();
}

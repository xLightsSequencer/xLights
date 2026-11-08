// Synthetic Vixen 3 sequences (.tim) and SystemConfig.xml documents.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "import_export/Vixen3.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {

std::string SysConfig(const std::string& nodes) { return "<SystemConfig><Nodes>" + nodes + "</Nodes></SystemConfig>"; }
std::string Node(const std::string& id, const std::string& name, const std::string& kids = "") { return "<Node name=\"" + name + "\" id=\"" + id + "\">" + kids + "</Node>"; }

std::string Seq(const std::string& body) { return "<TimedSequenceData xmlns:i=\"http://www.w3.org/2001/XMLSchema-instance\">" + body + "</TimedSequenceData>"; }

std::string Surrogate(const std::string& instance, const std::string& start, const std::string& span, const std::string& nodeId) {
    return "<EffectNodeSurrogate><InstanceId>" + instance + "</InstanceId><StartTime>" + start + "</StartTime><TimeSpan>" + span +
           "</TimeSpan><TargetNodes><ChannelNodeReferenceSurrogate><NodeId>" + nodeId + "</NodeId></ChannelNodeReferenceSurrogate></TargetNodes></EffectNodeSurrogate>";
}
std::string Surrogates(const std::string& s) { return "<_effectNodeSurrogates>" + s + "</_effectNodeSurrogates>"; }
std::string DataModel(const std::string& type, const std::string& id, const std::string& body) {
    return "<d1p1:anyType i:type=\"d2p1:" + type + "\"><ModuleInstanceId>" + id + "</ModuleInstanceId>" + body + "</d1p1:anyType>";
}
std::string DataModels(const std::string& s) { return "<_dataModels>" + s + "</_dataModels>"; }
std::string Setting(const std::string& k, const std::string& v) { return "<d2p1:" + k + ">" + v + "</d2p1:" + k + ">"; }

std::string ColorPoint(double x, double y, double z, double pos) {
    return "<d2p1:ColorPoint><d2p1:_color><d2p1:_x>" + std::to_string(x) + "</d2p1:_x><d2p1:_y>" + std::to_string(y) + "</d2p1:_y><d2p1:_z>" + std::to_string(z) +
           "</d2p1:_z></d2p1:_color><d2p1:_position>" + std::to_string(pos) + "</d2p1:_position></d2p1:ColorPoint>";
}
const double WX = 95.047, WY = 100.0, WZ = 108.883;
const double RX = 41.24, RY = 21.26, RZ = 1.93;

struct Dir {
    TempDir d;
    std::string seq;
    std::string sys;
    Dir(const std::string& seqBody, const std::string& sysBody, bool sysInSeqDir = true) {
        if (sysInSeqDir) sys = d.Write("SystemConfig.xml", sysBody);
        seq = d.Write("a.tim", seqBody);
    }
};

// A single node "N1"=Roof with one effect E1.
Dir OneEffect(const std::string& type, const std::string& settings, const std::string& start = "PT1S", const std::string& span = "PT2S") {
    return Dir(Seq(Surrogates(Surrogate("E1", start, span, "N1")) + DataModels(DataModel(type, "E1", settings))), SysConfig(Node("N1", "Roof")));
}

VixenEffect Eff(const std::string& type, std::map<std::string, std::string> s) {
    VixenEffect e(0, 1, "x");
    e.type = type;
    e.settings = std::move(s);
    return e;
}

} // namespace

// ---------------------------------------------------------------- ConvertTiming

TEST_CASE("Vixen3 ConvertTiming rounds to the frame grid", "[impexp2][vixen]") {
    struct C { float t; int f; long e; };
    auto c = GENERATE(values<C>({ { 1.0f, 50, 1000 }, { 0.012f, 50, 0 }, { 0.026f, 50, 50 }, { 0.025f, 50, 50 }, { 0.0f, 50, 0 }, { 1.5f, 25, 1500 }, { 1.51f, 25, 1500 },
                                  { 1.52f, 25, 1525 }, { -1.0f, 50, -1000 }, { 3600.0f, 20, 3600000 }, { 0.5f, 33, 495 }, { 2.0f, 1, 2000 }, { 1e6f, 50, 1000000000 } }));
    CAPTURE(c.t, c.f);
    CHECK(Vixen3::ConvertTiming(c.t, c.f) == c.e);
}

TEST_CASE("Vixen3 ConvertTiming with a zero frame time must not produce garbage", "[impexp2][vixen]") {
    // round(t*1000/0) = inf, inf*0 = NaN converted to long is undefined; expected 0
    CHECK(Vixen3::ConvertTiming(1.0f, 0) == 0);
}

// ---------------------------------------------------------------- constructor / files

TEST_CASE("Vixen3 with a missing file is empty", "[impexp2][vixen]") {
    TempDir d;
    Vixen3 v(d.File("none.tim"));
    CHECK(v.GetTimings().empty());
    CHECK(v.GetModelsWithEffects().empty());
    CHECK_FALSE(v.IsSystemFound());
    CHECK(v.GetFilename() == d.File("none.tim"));
}

TEST_CASE("Vixen3 survives junk empty and wrong root documents", "[impexp2][vixen]") {
    for (std::string body : { std::string(""), std::string("junk"), std::string("<<<>>>"), std::string("<Other/>"), std::string("<TimedSequenceData/>"), std::string("\xEF\xBB\xBF<TimedSequenceData/>"), std::string("\xFF\xFE\x00\x01", 4) }) {
        Dir d(body, SysConfig(Node("N1", "x")));
        Vixen3 v(d.seq);
        CAPTURE(body);
        CHECK(v.GetTimings().empty());
        CHECK(v.GetModelsWithEffects().empty());
    }
}

TEST_CASE("Vixen3 finds SystemConfig next to the sequence", "[impexp2][vixen]") {
    Dir d(Seq(""), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.IsSystemFound());
}

TEST_CASE("Vixen3 finds SystemConfig in a SystemData folder", "[impexp2][vixen]") {
    TempDir t;
    std::filesystem::create_directories(t.path / "SystemData");
    t.Write("SystemData/SystemConfig.xml", SysConfig(""));
    auto seq = t.Write("a.tim", Seq(""));
    Vixen3 v(seq);
    CHECK(v.IsSystemFound());
}

TEST_CASE("Vixen3 finds SystemConfig in a sibling SystemData folder of the parent", "[impexp2][vixen]") {
    TempDir t;
    std::filesystem::create_directories(t.path / "SystemData");
    std::filesystem::create_directories(t.path / "Sequences");
    t.Write("SystemData/SystemConfig.xml", SysConfig(Node("N1", "Roof")));
    auto seq = t.Write("Sequences/a.tim", Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1")) + DataModels(DataModel("SetLevelData", "E1", ""))));
    Vixen3 v(seq);
    CHECK(v.IsSystemFound());
    CHECK(v.GetModelsWithEffects().size() == 1);
}

TEST_CASE("Vixen3 explicit SystemConfig path is honoured", "[impexp2][vixen]") {
    TempDir t;
    std::filesystem::create_directories(t.path / "elsewhere");
    auto sys = t.Write("elsewhere/mine.xml", SysConfig(Node("N1", "Roof")));
    auto seq = t.Write("a.tim", Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1")) + DataModels(DataModel("SetLevelData", "E1", ""))));
    Vixen3 v(seq, sys);
    CHECK(v.IsSystemFound());
    REQUIRE(v.GetModelsWithEffects().size() == 1);
    CHECK(v.GetModelsWithEffects().front().first == "Roof");
}

TEST_CASE("Vixen3 explicit missing SystemConfig falls back to the search", "[impexp2][vixen]") {
    Dir d(Seq(""), SysConfig(""));
    Vixen3 v(d.seq, d.d.File("nope.xml"));
    CHECK(v.IsSystemFound());
    Dir e(Seq(""), "", false);
    Vixen3 w(e.seq, e.d.File("nope.xml"));
    CHECK_FALSE(w.IsSystemFound());
}

TEST_CASE("Vixen3 without a SystemConfig reads timings but no effects", "[impexp2][vixen]") {
    Dir d(Seq("<MarkCollections><MarkCollection><Name>M</Name><Marks><d3p1:duration>PT1S</d3p1:duration></Marks></MarkCollection></MarkCollections>" +
              Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1")) + DataModels(DataModel("SetLevelData", "E1", ""))),
          "", false);
    Vixen3 v(d.seq);
    CHECK_FALSE(v.IsSystemFound());
    CHECK(v.GetTimings().size() == 1);
    CHECK(v.GetModelsWithEffects().empty());
}

TEST_CASE("Vixen3 SystemConfig with the wrong root yields no models", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1")) + DataModels(DataModel("SetLevelData", "E1", ""))), "<Other><Nodes><Node name=\"R\" id=\"N1\"/></Nodes></Other>");
    Vixen3 v(d.seq);
    CHECK(v.GetModelsWithEffects().empty());
}

TEST_CASE("Vixen3 SystemConfig nested nodes are all registered", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "C") + Surrogate("E2", "PT0S", "PT1S", "A")) + DataModels(DataModel("SetLevelData", "E1", "") + DataModel("SetLevelData", "E2", ""))),
          SysConfig(Node("A", "Top", Node("B", "Mid", Node("C", "Leaf")))));
    Vixen3 v(d.seq);
    auto m = v.GetModelsWithEffects();
    REQUIRE(m.size() == 2);
    CHECK(m.front().first == "Leaf");
    CHECK(m.back().first == "Top");
}

TEST_CASE("Vixen3 effects pointing at unknown nodes are dropped", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "NOPE") + Surrogate("E2", "PT0S", "PT1S", "")) + DataModels(DataModel("SetLevelData", "E1", ""))), SysConfig(Node("N1", "Roof")));
    Vixen3 v(d.seq);
    CHECK(v.GetModelsWithEffects().empty());
}

TEST_CASE("Vixen3 effect section before models exist is ignored", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1"))), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.GetModelsWithEffects().empty());
}

TEST_CASE("Vixen3 effect times", "[impexp2][vixen]") {
    struct C { const char* start; const char* span; float s; float e; };
    auto c = GENERATE(values<C>({ { "PT1S", "PT2S", 1.0f, 3.0f }, { "PT0S", "PT0.5S", 0.0f, 0.5f }, { "PT1M30.5S", "PT2.25S", 90.5f, 92.75f }, { "PT2M", "PT1M", 120.0f, 180.0f },
                                  { "", "", 0.0f, 0.0f }, { "PT-1S", "PT3S", -1.0f, 2.0f }, { "PT10S", "PT-3S", 10.0f, 7.0f }, { "1S", "2S", 1.0f, 3.0f } }));
    auto d = OneEffect("SetLevelData", "", c.start, c.span);
    Vixen3 v(d.seq);
    auto effs = v.GetEffects("Roof");
    REQUIRE(effs.size() == 1);
    CAPTURE(c.start, c.span);
    CHECK(effs.front().start == c.s);
    CHECK(effs.front().end == c.e);
    CHECK(effs.front().effectSettingsId == "E1");
}

TEST_CASE("Vixen3 durations with hours are read correctly", "[impexp2][vixen]") {
    // time parsing handles only PT<m>M<s>S; PT1H2M3S is read as 1 minute 3 seconds
    auto d = OneEffect("SetLevelData", "", "PT1H2M3S", "PT1S");
    Vixen3 v(d.seq);
    auto effs = v.GetEffects("Roof");
    REQUIRE(effs.size() == 1);
    CHECK(effs.front().start == 3723.0f);
}

TEST_CASE("Vixen3 GetEffects of unknown models is empty and the model list counts effects", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1") + Surrogate("E2", "PT1S", "PT1S", "N1") + Surrogate("E3", "PT1S", "PT1S", "N2")) +
              DataModels(DataModel("SetLevelData", "E1", "") + DataModel("SetLevelData", "E2", "") + DataModel("PulseData", "E3", ""))),
          SysConfig(Node("N1", "Zed") + Node("N2", "Alpha")));
    Vixen3 v(d.seq);
    CHECK(v.GetEffects("Nope").empty());
    auto m = v.GetModelsWithEffects();
    REQUIRE(m.size() == 2);
    CHECK(m.front() == std::make_pair(std::string("Alpha"), 1));
    CHECK(m.back() == std::make_pair(std::string("Zed"), 2));
}

TEST_CASE("Vixen3 two nodes with one name share their effects", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1") + Surrogate("E2", "PT1S", "PT1S", "N2")) + DataModels(DataModel("SetLevelData", "E1", "") + DataModel("SetLevelData", "E2", ""))),
          SysConfig(Node("N1", "Same") + Node("N2", "Same")));
    Vixen3 v(d.seq);
    CHECK(v.GetEffects("Same").size() == 2);
}

TEST_CASE("Vixen3 effect without a data model has an empty type", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1"))), SysConfig(Node("N1", "Roof")));
    Vixen3 v(d.seq);
    // _dataModels missing; effect has no type
    auto effs = v.GetEffects("Roof");
    REQUIRE(effs.size() == 1);
    CHECK(effs.front().type.empty());
    CHECK(effs.front().GetXLightsType() == "Off");
    CHECK(effs.front().GetSettings().empty());
    CHECK(effs.front().GetPalette().empty());
}

TEST_CASE("Vixen3 data model without a type attribute or id is ignored", "[impexp2][vixen]") {
    Dir d(Seq(Surrogates(Surrogate("E1", "PT0S", "PT1S", "N1")) + DataModels("<d1p1:anyType><ModuleInstanceId>E1</ModuleInstanceId></d1p1:anyType><d1p1:anyType i:type=\"d2p1:PulseData\"/>")), SysConfig(Node("N1", "Roof")));
    Vixen3 v(d.seq);
    auto effs = v.GetEffects("Roof");
    REQUIRE(effs.size() == 1);
    CHECK(effs.front().type.empty());
}

TEST_CASE("Vixen3 effect type comes from the i:type attribute after the colon", "[impexp2][vixen]") {
    struct C { const char* t; const char* expect; };
    auto c = GENERATE(values<C>({ { "PulseData", "PulseData" }, { "SetLevelData", "SetLevelData" } }));
    auto d = OneEffect(c.t, "");
    Vixen3 v(d.seq);
    CHECK(v.GetEffects("Roof").front().type == c.expect);
}

// ---------------------------------------------------------------- mark collections

TEST_CASE("Vixen3 mark collections become timings", "[impexp2][vixen]") {
    Dir d(Seq("<MarkCollections><MarkCollection><Name>Beats</Name><Marks><d3p1:duration>PT0.5S</d3p1:duration><d3p1:duration>PT1S</d3p1:duration><d3p1:duration>PT1M2.5S</d3p1:duration></Marks></MarkCollection></MarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto names = v.GetTimings();
    REQUIRE(names.size() == 1);
    CHECK(names.front() == "Beats");
    CHECK(v.GetTimingType("Beats") == "Generic");
    auto t = v.GetTimings("Beats");
    REQUIRE(t.size() == 3);
    auto it = t.begin();
    CHECK(it->start == 0.0f);
    CHECK(it->end == 0.5f);
    ++it;
    CHECK(it->start == 0.5f);
    CHECK(it->end == 1.0f);
    ++it;
    CHECK(it->start == 1.0f);
    CHECK(it->end == 62.5f);
    CHECK(it->label.empty());
}

TEST_CASE("Vixen3 mark time formats", "[impexp2][vixen]") {
    struct C { const char* t; float e; };
    auto c = GENERATE(values<C>({ { "PT1S", 1.0f }, { "PT1.5S", 1.5f }, { "PT0S", 0.0f }, { "PT90S", 90.0f }, { "PT2M", 120.0f }, { "PT2M5S", 125.0f }, { "1.5S", 1.5f }, { "", 0.0f }, { "junk", 0.0f },
                                  { "PT.5S", 0.5f }, { "PTS", 0.0f }, { "PT-2S", -2.0f }, { "PT1e1S", 10.0f } }));
    Dir d(Seq("<MarkCollections><MarkCollection><Name>M</Name><Marks><d3p1:duration>" + std::string(c.t) + "</d3p1:duration></Marks></MarkCollection></MarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto t = v.GetTimings("M");
    REQUIRE(t.size() == 1);
    CAPTURE(c.t);
    CHECK(t.front().end == c.e);
}

TEST_CASE("Vixen3 mark collection with no name uses the empty name and overwrites", "[impexp2][vixen]") {
    Dir d(Seq("<MarkCollections><MarkCollection><Marks><d3p1:duration>PT1S</d3p1:duration></Marks></MarkCollection><MarkCollection><Marks><d3p1:duration>PT2S</d3p1:duration><d3p1:duration>PT3S</d3p1:duration></Marks></MarkCollection></MarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.GetTimings("").size() == 2);
}

TEST_CASE("Vixen3 mark collection ignores unrelated children", "[impexp2][vixen]") {
    Dir d(Seq("<MarkCollections><Other/><MarkCollection><Name>M</Name><Junk/><Marks><x/><d3p1:duration>PT1S</d3p1:duration></Marks></MarkCollection></MarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.GetTimings("M").size() == 1);
}

TEST_CASE("Vixen3 decreasing marks give reversed intervals", "[impexp2][vixen]") {
    Dir d(Seq("<MarkCollections><MarkCollection><Name>M</Name><Marks><d3p1:duration>PT2S</d3p1:duration><d3p1:duration>PT1S</d3p1:duration></Marks></MarkCollection></MarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto t = v.GetTimings("M");
    REQUIRE(t.size() == 2);
    CHECK(t.back().start == 2.0f);
    CHECK(t.back().end == 1.0f);
}

namespace {
std::string Labeled(const std::string& name, const std::string& type, const std::string& marks) {
    return "<d1p1:anyType><d2p1:Name>" + name + "</d2p1:Name><d2p1:CollectionType>" + type + "</d2p1:CollectionType><d2p1:Marks>" + marks + "</d2p1:Marks></d1p1:anyType>";
}
std::string LMark(const std::string& start, const std::string& dur, const std::string& text) {
    return "<d1p1:anyType><d2p1:StartTime>" + start + "</d2p1:StartTime><d2p1:Duration>" + dur + "</d2p1:Duration><d2p1:Text>" + text + "</d2p1:Text></d1p1:anyType>";
}
}

TEST_CASE("Vixen3 labelled marks use start and duration", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections>" + Labeled("Words", "Phrase", LMark("PT1S", "PT0.5S", "hello") + LMark("PT2S", "PT1S", "world")) + "</LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.GetTimingType("Words") == "Phrase");
    auto t = v.GetTimings("Words");
    REQUIRE(t.size() == 2);
    CHECK(t.front().start == 1.0f);
    CHECK(t.front().end == 1.5f);
    CHECK(t.front().label == "hello");
    CHECK(t.back().start == 2.0f);
    CHECK(t.back().end == 3.0f);
    CHECK(t.back().label == "world");
}

TEST_CASE("Vixen3 labelled marks that overlap are pushed after the previous one", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections>" + Labeled("W", "Phrase", LMark("PT1S", "PT2S", "a") + LMark("PT2S", "PT2S", "b") + LMark("PT0S", "PT1S", "c")) + "</LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto t = v.GetTimings("W");
    // the third mark starts before the end of the second and ends at 1s, so it cannot fit and is dropped
    REQUIRE(t.size() == 2);
    auto it = t.begin();
    CHECK(it->start == 1.0f);
    CHECK(it->end == 3.0f);
    ++it;
    CHECK(it->start == 3.0f);
    CHECK(it->end == 5.0f);
}

TEST_CASE("Vixen3 labelled marks with empty text ignore the duration", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections>" + Labeled("W", "Generic", LMark("PT1S", "PT9S", "") + LMark("PT3S", "PT0S", "") + LMark("PT2S", "PT1S", "") + LMark("PT0S", "PT1S", "")) + "</LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto t = v.GetTimings("W");
    REQUIRE(t.size() == 2);
    CHECK(t.front().start == 0.0f);
    CHECK(t.front().end == 1.0f);
    CHECK(t.back().start == 1.0f);
    CHECK(t.back().end == 3.0f);
}

TEST_CASE("Vixen3 labelled marks with zero or negative duration are dropped", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections>" + Labeled("W", "Phrase", LMark("PT1S", "PT0S", "a") + LMark("PT1S", "PT-1S", "b") + LMark("PT1S", "", "c")) + "</LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.GetTimings("W").empty());
}

TEST_CASE("Vixen3 unnamed labelled collections get numbered names", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections>" + Labeled("", "Phrase", LMark("PT1S", "PT1S", "a")) + Labeled("", "Generic", LMark("PT1S", "PT1S", "b")) + "</LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto names = v.GetTimings();
    REQUIRE(names.size() == 2);
    CHECK(names.front() == "Unnamed 1");
    CHECK(names.back() == "Unnamed 2");
}

TEST_CASE("Vixen3 only generic and phrase collections are listed and related word timings are found", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections>" + Labeled("Song", "Phrase", LMark("PT1S", "PT1S", "a")) + Labeled("Song Word", "Word", LMark("PT1S", "PT0.5S", "w")) +
              Labeled("Song Phoneme", "Phoneme", LMark("PT1S", "PT0.25S", "p")) + Labeled("Odd", "Strange", LMark("PT1S", "PT1S", "z")) + "</LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    auto names = v.GetTimings();
    REQUIRE(names.size() == 1);
    CHECK(names.front() == "Song");
    CHECK(v.GetRelatedTiming("Song", "Word").size() == 1);
    CHECK(v.GetRelatedTiming("Song", "Phoneme").front().label == "p");
    CHECK(v.GetRelatedTiming("Song", "Nothing").empty());
    CHECK(v.GetTimingType("Odd") == "Strange");
    CHECK(v.GetTimingType("missing") == "");
    CHECK(v.GetTimings("missing").empty());
}

TEST_CASE("Vixen3 labelled collection without a type is generic", "[impexp2][vixen]") {
    Dir d(Seq("<LabeledMarkCollections><d1p1:anyType><d2p1:Name>N</d2p1:Name><d2p1:Marks>" + LMark("PT1S", "PT1S", "a") + "</d2p1:Marks></d1p1:anyType></LabeledMarkCollections>"), SysConfig(""));
    Vixen3 v(d.seq);
    CHECK(v.GetTimingType("N") == "Generic");
}

// ---------------------------------------------------------------- effect settings from files

TEST_CASE("Vixen3 pulse effect level curve with two points sets start and end intensity", "[impexp2][vixen]") {
    std::string lc = "<d2p1:LevelCurve><d2p1:Points><d2p1:PointPair><X>0</X><Y>0</Y></d2p1:PointPair><d2p1:PointPair><X>100</X><Y>80</Y></d2p1:PointPair></d2p1:Points></d2p1:LevelCurve>";
    auto d = OneEffect("PulseData", lc);
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    CHECK(e.GetXLightsType() == "On");
    REQUIRE(e.levelCurve.size() == 2);
    CHECK(e.GetSettings() == "E_TEXTCTRL_Eff_On_Start=0,E_TEXTCTRL_Eff_On_End=80");
    CHECK(e.GetPalette().find("ValueCurve") == std::string::npos);
}

TEST_CASE("Vixen3 pulse effect level curve with several points makes a brightness curve", "[impexp2][vixen]") {
    std::string lc = "<d2p1:LevelCurve><d2p1:Points>";
    for (auto xy : { std::pair<int, int>{ 0, 0 }, { 50, 100 }, { 100, 20 } }) lc += "<d2p1:PointPair><X>" + std::to_string(xy.first) + "</X><Y>" + std::to_string(xy.second) + "</Y></d2p1:PointPair>";
    lc += "</d2p1:Points></d2p1:LevelCurve>";
    auto d = OneEffect("PulseData", lc);
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    CHECK(e.GetSettings().empty());
    CHECK(e.GetPalette() == ",C_VALUECURVE_Brightness=Active=TRUE|Id=ID_VALUECURVE_Brightness|Type=Custom|Min=0.00|Max=400.00|RV=TRUE|Values=0.00:0.00;0.50:0.25;1.00:0.05");
}

TEST_CASE("Vixen3 pulse effect with one or zero curve points has no settings", "[impexp2][vixen]") {
    auto d = OneEffect("PulseData", "<d2p1:LevelCurve><d2p1:Points><d2p1:PointPair><X>0</X><Y>5</Y></d2p1:PointPair></d2p1:Points></d2p1:LevelCurve>");
    Vixen3 v(d.seq);
    CHECK(v.GetEffects("Roof").front().GetSettings().empty());
    auto d2 = OneEffect("PulseData", "<d2p1:LevelCurve/>");
    Vixen3 w(d2.seq);
    CHECK(w.GetEffects("Roof").front().GetSettings().empty());
}

TEST_CASE("Vixen3 level curve points with junk numbers read as zero", "[impexp2][vixen]") {
    auto d = OneEffect("PulseData", "<d2p1:LevelCurve><d2p1:Points><d2p1:PointPair><X>abc</X><Y>1.9</Y></d2p1:PointPair><d2p1:PointPair/></d2p1:Points></d2p1:LevelCurve>");
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.levelCurve.size() == 2);
    CHECK(e.levelCurve[0].x == 0);
    CHECK(e.levelCurve[0].y == 1);
    CHECK(e.GetSettings() == "E_TEXTCTRL_Eff_On_Start=1,E_TEXTCTRL_Eff_On_End=0");
}

TEST_CASE("Vixen3 set level colour is scaled from 0-1 floats", "[impexp2][vixen]") {
    auto d = OneEffect("SetLevelData", "<d2p1:color><d2p1:_r>1</d2p1:_r><d2p1:_g>0.5</d2p1:_g><d2p1:_b>0</d2p1:_b></d2p1:color>");
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 1);
    REQUIRE(e.palette[0].size() == 1);
    CHECK(e.palette[0][0].color == xlColor(255, 127, 0));
    CHECK(e.GetPalette() == "C_BUTTON_Palette1=#ff7f00,C_CHECKBOX_Palette1=1");
}

TEST_CASE("Vixen3 set level colour out of range does not wrap", "[impexp2][vixen]") {
    // int r = 255 * 2.0 = 510 is cast to uint8_t, wrapping to 254
    auto d = OneEffect("SetLevelData", "<d2p1:color><d2p1:_r>2</d2p1:_r><d2p1:_g>-1</d2p1:_g><d2p1:_b>0</d2p1:_b></d2p1:color>");
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 1);
    CHECK(e.palette[0][0].color == xlColor(255, 0, 0));
}

TEST_CASE("Vixen3 gradients from XYZ colour points", "[impexp2][vixen]") {
    std::string g = "<d2p1:ColorGradient><d2p1:_colors>" + ColorPoint(RX, RY, RZ, 1.0) + ColorPoint(0, 0, 0, 0.0) + ColorPoint(WX, WY, WZ, 0.5) + "</d2p1:_colors></d2p1:ColorGradient>";
    auto d = OneEffect("ColorWashData", g);
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    CHECK(e.GetXLightsType() == "Color Wash");
    REQUIRE(e.palette.size() == 1);
    REQUIRE(e.palette[0].size() == 3);
    // sorted by position
    CHECK(e.palette[0][0].position == 0.0f);
    CHECK(e.palette[0][0].color == xlBLACK);
    CHECK(e.palette[0][1].position == 0.5f);
    auto w = e.palette[0][1].color;
    CHECK(w.Red() >= 254);
    CHECK(w.Green() >= 254);
    CHECK(w.Blue() >= 254);
    auto r = e.palette[0][2].color;
    CHECK(r.Red() >= 254);
    CHECK(r.Green() <= 2);
    CHECK(r.Blue() <= 2);
    auto p = e.GetPalette();
    CHECK(p.find("C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Values=x=0.000^c=#000000;x=0.500^c=#") == 0);
    CHECK(p.find("|,C_CHECKBOX_Palette1=1") != std::string::npos);
}

TEST_CASE("Vixen3 gradient names that carry colours", "[impexp2][vixen]") {
    auto name = GENERATE(values<std::string>({ "ColorGradient", "Gradient", "HeadColor", "FillColor", "MeterColorGradient" }));
    auto d = OneEffect("ColorWashData", "<d2p1:" + name + "><d2p1:_colors>" + ColorPoint(0, 0, 0, 0.0) + ColorPoint(WX, WY, WZ, 1.0) + "</d2p1:_colors></d2p1:" + name + ">");
    Vixen3 v(d.seq);
    CAPTURE(name);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 1);
    CHECK(e.palette[0].size() == 2);
}

TEST_CASE("Vixen3 nested colour containers", "[impexp2][vixen]") {
    std::string cols = "<d2p1:_colors>" + ColorPoint(0, 0, 0, 0.0) + ColorPoint(WX, WY, WZ, 1.0) + "</d2p1:_colors>";
    struct C { std::string body; size_t palettes; };
    auto c = GENERATE_COPY(values<C>({
        { "<d2p1:Colors>" + cols + "</d2p1:Colors>", 1 },
        { "<d2p1:Colors><d2p1:ColorGradient>" + cols + "</d2p1:ColorGradient></d2p1:Colors>", 1 },
        { "<d2p1:Colors><d2p1:GradientLevelPair><d2p1:ColorGradient>" + cols + "</d2p1:ColorGradient></d2p1:GradientLevelPair></d2p1:Colors>", 1 },
        { "<d2p1:GradientColors><d2p1:ColorGradient>" + cols + "</d2p1:ColorGradient></d2p1:GradientColors>", 1 },
        { "<d2p1:ColorGradients><d2p1:ColorGradient>" + cols + "</d2p1:ColorGradient><d2p1:ColorGradient>" + cols + "</d2p1:ColorGradient></d2p1:ColorGradients>", 2 },
        { "<d2p1:Colors><d2p1:Other>" + cols + "</d2p1:Other></d2p1:Colors>", 0 },
    }));
    auto d = OneEffect("BarsData", c.body);
    Vixen3 v(d.seq);
    CHECK(v.GetEffects("Roof").front().palette.size() == c.palettes);
}

TEST_CASE("Vixen3 static colour handling uses the static colour", "[impexp2][vixen]") {
    std::string body = Setting("ColorHandling", "StaticColor") + "<d2p1:StaticColor><d2p1:value>4294901760</d2p1:value></d2p1:StaticColor><d2p1:ColorGradient><d2p1:_colors>" + ColorPoint(0, 0, 0, 0) + "</d2p1:_colors></d2p1:ColorGradient>";
    auto d = OneEffect("TwinkleData", body + Setting("AveragePulseTime", "5") + Setting("AverageCoverage", "40"));
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 1);
    CHECK(e.palette[0][0].color == xlRED);
    CHECK(e.GetSettings() == "E_SLIDER_Twinkle_Steps=5,E_SLIDER_Twinkle_Count=40");
}

TEST_CASE("Vixen3 non static colour handling ignores the static colour", "[impexp2][vixen]") {
    std::string body = Setting("ColorHandling", "GradientThroughWholeEffect") + "<d2p1:StaticColor><d2p1:value>4294901760</d2p1:value></d2p1:StaticColor><d2p1:ColorGradient><d2p1:_colors>" + ColorPoint(0, 0, 0, 0) + "</d2p1:_colors></d2p1:ColorGradient>";
    auto d = OneEffect("ColorWashData", body);
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 1);
    CHECK(e.palette[0][0].color == xlBLACK);
}

TEST_CASE("Vixen3 static colour value extraction", "[impexp2][vixen]") {
    struct C { const char* v; xlColor c; };
    auto c = GENERATE(values<C>({ { "4278190335", xlColor(0, 0, 255) }, { "4278255360", xlColor(0, 255, 0) }, { "0", xlBLACK }, { "16777215", xlWHITE }, { "abc", xlBLACK } }));
    auto d = OneEffect("TwinkleData", Setting("ColorHandling", "StaticColor") + "<d2p1:StaticColor><d2p1:value>" + c.v + "</d2p1:value></d2p1:StaticColor>");
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 1);
    CAPTURE(c.v);
    CHECK(e.palette[0][0].color == c.c);
}

TEST_CASE("Vixen3 nutcracker palette uses known colours that are active", "[impexp2][vixen]") {
    std::string pal = "<d2p1:Palette><d2p1:_colors><d2p1:Color><d2p1:knownColor>37</d2p1:knownColor></d2p1:Color><d2p1:Color><d2p1:knownColor>141</d2p1:knownColor></d2p1:Color><d2p1:Color><d2p1:knownColor>104</d2p1:knownColor></d2p1:Color></d2p1:_colors>"
                      "<d2p1:_colorsActive><d2p1:boolean>true</d2p1:boolean><d2p1:boolean>false</d2p1:boolean><d2p1:boolean>true</d2p1:boolean></d2p1:_colorsActive></d2p1:Palette>";
    auto d = OneEffect("NutcrackerModuleData", "<d2p1:NutcrackerData>" + pal + "<d2p1:CurrentEffect>Fire</d2p1:CurrentEffect></d2p1:NutcrackerData>");
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    REQUIRE(e.palette.size() == 2);
    CHECK(e.palette[0][0].color == xlColor(0, 0, 255));
    CHECK(e.palette[1][0].color == xlColor(0, 255, 0));
    CHECK(e.settings.at("CurrentEffect") == "Fire");
    CHECK(e.GetXLightsType() == "Fire");
}

TEST_CASE("Vixen3 nutcracker palette with more active flags than colours must not throw", "[impexp2][vixen]") {
    // knownColor.at(index) throws std::out_of_range out of the constructor
    std::string pal = "<d2p1:Palette><d2p1:_colors><d2p1:Color><d2p1:knownColor>37</d2p1:knownColor></d2p1:Color></d2p1:_colors>"
                      "<d2p1:_colorsActive><d2p1:boolean>true</d2p1:boolean><d2p1:boolean>true</d2p1:boolean></d2p1:_colorsActive></d2p1:Palette>";
    auto d = OneEffect("NutcrackerModuleData", "<d2p1:NutcrackerData>" + pal + "</d2p1:NutcrackerData>");
    CHECK_NOTHROW(Vixen3(d.seq));
}

TEST_CASE("Vixen3 nutcracker palette active flags before colours must not throw", "[impexp2][vixen]") {
    std::string pal = "<d2p1:Palette><d2p1:_colorsActive><d2p1:boolean>true</d2p1:boolean></d2p1:_colorsActive><d2p1:_colors><d2p1:Color><d2p1:knownColor>37</d2p1:knownColor></d2p1:Color></d2p1:_colors></d2p1:Palette>";
    auto d = OneEffect("NutcrackerModuleData", "<d2p1:NutcrackerData>" + pal + "</d2p1:NutcrackerData>");
    CHECK_NOTHROW(Vixen3(d.seq));
}

TEST_CASE("Vixen3 settings elements without children are ignored", "[impexp2][vixen]") {
    auto d = OneEffect("BarsData", "<d2p1:Empty/><d2p1:Direction>Moves Down</d2p1:Direction><NotPrefixed>1</NotPrefixed><TargetPositioning>Strings</TargetPositioning><TargetPositioning/>");
    Vixen3 v(d.seq);
    auto e = v.GetEffects("Roof").front();
    CHECK(e.settings.count("Empty") == 0);
    CHECK(e.settings.count("NotPrefixed") == 0);
    CHECK(e.settings.at("Direction") == "Moves Down");
    CHECK(e.settings.at("TargetPositioning") == "Strings");
}

// ---------------------------------------------------------------- GetXLightsType table

TEST_CASE("Vixen3 effect type mapping", "[impexp2][vixen]") {
    struct C { const char* v; const char* x; };
    auto c = GENERATE(values<C>({
        { "AlternatingData", "Marquee" }, { "BarsData", "Bars" }, { "BorderData", "Marquee" }, { "ButterflyData", "Butterfly" }, { "CandleData", "Candle" }, { "ChaseData", "Single Strand" },
        { "CirclesData", "Circles" }, { "ColorWashData", "Color Wash" }, { "CurtainData", "Curtain" }, { "Data", "On" }, { "DissolveData", "Off" }, { "FireData", "Fire" },
        { "FireworksData", "Fireworks" }, { "GarlandsData", "Garlands" }, { "GlediatorData", "Glediator" }, { "LifeData", "Life" }, { "LipSyncData", "Faces" }, { "LiquidData", "Liquid" },
        { "MeteorsData", "Meteors" }, { "MorphData", "Morph" }, { "PictureData", "Pictures" }, { "PinWheelData", "Pinwheel" }, { "PlasmaData", "Plasma" }, { "PulseData", "On" },
        { "SetLevelData", "On" }, { "ShapesData", "Shape" }, { "ShockwaveData", "Shockwave" }, { "SnowflakesData", "Snowflakes" }, { "SnowStormData", "Snowstorm" }, { "SpinData", "Pinwheel" },
        { "SpiralData", "Spirals" }, { "SpirographData", "Spirograph" }, { "StrobeData", "Strobe" }, { "TextData", "Text" }, { "TreeData", "Tree" }, { "TwinkleData", "Twinkle" },
        { "VideoData", "Video" }, { "VUMeterData", "VU Meter" }, { "WaveData", "Wave" }, { "WipeData", "Single Strand" }, { "", "Off" }, { "Bogus", "Off" }, { "bars", "Off" }, { "AudioData", "Off" },
    }));
    VixenEffect e(0, 1, "x");
    e.type = c.v;
    CAPTURE(c.v);
    CHECK(e.GetXLightsType() == c.x);
}

TEST_CASE("Vixen3 nutcracker effect type mapping", "[impexp2][vixen]") {
    struct C { const char* v; const char* x; };
    auto c = GENERATE(values<C>({ { "ColorWash", "Color Wash" }, { "Movie", "Video" }, { "Picture", "Pictures" }, { "PictureTile", "Pictures" }, { "Twinkles", "Twinkle" }, { "Bars", "Bars" }, { "Fire", "Fire" }, { "", "" }, { "Weird", "Weird" } }));
    auto e = Eff("NutcrackerModuleData", { { "CurrentEffect", c.v } });
    CAPTURE(c.v);
    CHECK(e.GetXLightsType() == c.x);
}

TEST_CASE("Vixen3 nutcracker effect type without CurrentEffect must not throw", "[impexp2][vixen]") {
    // settings.at("CurrentEffect") throws std::out_of_range
    VixenEffect e(0, 1, "x");
    e.type = "NutcrackerModuleData";
    CHECK_NOTHROW(e.GetXLightsType());
}

// ---------------------------------------------------------------- GetSettings table

TEST_CASE("Vixen3 bars direction mapping", "[impexp2][vixen]") {
    struct C { const char* v; const char* x; };
    auto c = GENERATE(values<C>({ { "Moves Down", "up" }, { "Moves Up", "down" }, { "Expands", "expand" }, { "Compresses", "compress" }, { "Moves Left", "Right" }, { "Moves Right", "Left" },
                                  { "Horizontal Expand", "H-expand" }, { "Horizontal Compress", "H-compress" }, { "Alternate Up", "Alternate Down" }, { "Alternate Down", "Alternate Up" },
                                  { "Alternate Left", "Alternate Right" }, { "Alternate Right", "Alternate Left" }, { "", "down" }, { "junk", "down" } }));
    auto e = Eff("BarsData", { { "Direction", c.v }, { "TargetPositioning", "Group" }, { "Show3D", "1" }, { "Highlight", "0" }, { "Repeat", "3" }, { "Speed", "2" } });
    CAPTURE(c.v);
    CHECK(e.GetSettings() == std::string("E_CHECKBOX_Bars_3D=1,E_CHECKBOX_Bars_Highlight=0,E_CHOICE_Bars_Direction=") + c.x + ",E_SLIDER_Bars_BarCount=3,E_TEXTCTRL_Bars_Cycles=2");
}

TEST_CASE("Vixen3 bars buffer style by orientation and bar count cap", "[impexp2][vixen]") {
    struct C { const char* orient; const char* rep; const char* prefix; const char* count; };
    auto c = GENERATE(values<C>({ { "Vertical", "1", "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,", "1" }, { "Horizontal", "9", "B_CHOICE_BufferStyle=Vertical Per Model/Strand,", "5" },
                                  { "Vertical", "abc", "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,", "0" }, { "Vertical", "-4", "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,", "-4" } }));
    auto e = Eff("BarsData", { { "Direction", "Expands" }, { "TargetPositioning", "Strings" }, { "Orientation", c.orient }, { "Show3D", "0" }, { "Highlight", "0" }, { "Repeat", c.rep }, { "Speed", "1" } });
    auto s = e.GetSettings();
    CAPTURE(c.orient, c.rep);
    CHECK(s.find(c.prefix) == 0);
    CHECK(s.find(std::string("E_SLIDER_Bars_BarCount=") + c.count + ",") != std::string::npos);
}

TEST_CASE("Vixen3 simple effect settings", "[impexp2][vixen]") {
    CHECK(Eff("AlternatingData", { { "GroupLevel", "4" } }).GetSettings() == "B_CHOICE_BufferStyle=Single Line,E_SLIDER_Marquee_Band_Size=4");
    CHECK(Eff("TwinkleData", { { "AveragePulseTime", "3" }, { "AverageCoverage", "7" } }).GetSettings() == "E_SLIDER_Twinkle_Steps=3,E_SLIDER_Twinkle_Count=7");
    CHECK(Eff("VUMeterData", {}).GetSettings() == "E_CHOICE_VUMeter_Type=On");
    for (auto t : { "AudioData", "ChaseData", "CirclesData", "Data", "FireData", "GarlandsData", "MeteorsData", "PictureData", "PinWheelData", "PlasmaData", "SetLevelData", "SpinData", "SpiralData", "SpirographData", "TextData", "Bogus", "" })
        CHECK(Eff(t, {}).GetSettings().empty());
}

TEST_CASE("Vixen3 butterfly style is the type text after the first e", "[impexp2][vixen]") {
    auto e = Eff("ButterflyData", { { "BackgroundChunks", "3" }, { "BackgroundSkips", "2" }, { "ButterflyType", "Butterfly2" }, { "Direction", "Forward" }, { "ColorScheme", "Gradient" } });
    CHECK(e.GetSettings() == "E_SLIDER_Butterfly_Chunks=3,E_SLIDER_Butterfly_Skip=2,E_SLIDER_Butterfly_Style=rfly2,E_CHOICE_Butterfly_Direction=0,E_CHOICE_Butterfly_Colors=Palette");
    auto f = Eff("ButterflyData", { { "BackgroundChunks", "3" }, { "BackgroundSkips", "2" }, { "ButterflyType", "Style1" }, { "Direction", "Back" }, { "ColorScheme", "Other" } });
    CHECK(f.GetSettings().find("E_SLIDER_Butterfly_Style=1,") != std::string::npos);
    CHECK(f.GetSettings().find("E_CHOICE_Butterfly_Direction=1,E_CHOICE_Butterfly_Colors=Rainbow") != std::string::npos);
}

TEST_CASE("Vixen3 wipe settings", "[impexp2][vixen]") {
    struct C { const char* dir; const char* rev; const char* move; const char* expect; };
    auto c = GENERATE(values<C>({
        { "Vertical", "true", "Time", "E_CHOICE_Chase_Type1=Left-Right,B_CHOICE_BufferTransform=Rotate CC 90" },
        { "Vertical", "false", "Time", "E_CHOICE_Chase_Type1=Left-Right,B_CHOICE_BufferTransform=Rotate CW 90" },
        { "Horizontal", "true", "Time", "E_CHOICE_Chase_Type1=Right-Left" },
        { "Horizontal", "false", "Time", "E_CHOICE_Chase_Type1=Left-Right" },
        { "Diagonal", "false", "Time", "E_CHOICE_Chase_Type1=From Middle" },
        { "Horizontal", "false", "Count", "E_CHOICE_Chase_Type1=Left-Right,E_TEXTCTRL_Chase_Rotations=4" },
    }));
    auto e = Eff("WipeData", { { "Direction", c.dir }, { "ReverseDirection", c.rev }, { "WipeMovement", c.move }, { "PassCount", "4" } });
    CAPTURE(c.dir, c.rev, c.move);
    CHECK(e.GetSettings() == c.expect);
}

namespace {
std::map<std::string, std::string> NC(const std::string& effect, std::map<std::string, std::string> extra, const std::string& orient = "Vertical", const std::string& speed = "10") {
    std::map<std::string, std::string> m = { { "CurrentEffect", effect }, { "FitToTime", "true" }, { "Speed", speed }, { "StringOrienation", orient } };
    for (auto& kv : extra) m[kv.first] = kv.second;
    return m;
}
}

TEST_CASE("Vixen3 nutcracker buffer style follows orientation", "[impexp2][vixen]") {
    CHECK(Eff("NutcrackerModuleData", NC("Unknown", {}, "Vertical")).GetSettings() == "B_CHOICE_BufferStyle=Horizontal Per Model/Strand");
    CHECK(Eff("NutcrackerModuleData", NC("Unknown", {}, "Horizontal")).GetSettings() == "B_CHOICE_BufferStyle=Vertical Per Model/Strand");
}

TEST_CASE("Vixen3 nutcracker bars directions", "[impexp2][vixen]") {
    const char* dirs[] = { "up", "down", "expand", "compress", "Left", "Right", "H-expand", "H-compress", "Alternate Up", "Alternate Down", "Alternate Left", "Alternate Right" };
    int i = GENERATE(Catch::Generators::range(0, 14));
    auto e = Eff("NutcrackerModuleData", NC("Bars", { { "Bars_Direction", std::to_string(i) }, { "Bars_3D", "true" }, { "Bars_Highlight", "false" }, { "Bars_PaletteRepeat", "2" } }));
    CAPTURE(i);
    std::string expected = i < 12 ? dirs[i] : "up";
    CHECK(e.GetSettings() == "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,E_CHECKBOX_Bars_3D=1,E_CHECKBOX_Bars_Highlight=0,E_CHOICE_Bars_Direction=" + expected + ",E_TEXTCTRL_Bars_BarCount=2");
}

TEST_CASE("Vixen3 nutcracker effects", "[impexp2][vixen]") {
    struct C { std::string eff; std::map<std::string, std::string> kv; std::string tail; };
    auto c = GENERATE(values<C>({
        { "Butterfly", { { "Butterfly_BkgrdChunks", "3" }, { "Butterfly_BkgrdSkip", "2" }, { "Butterfly_Style", "4" }, { "Butterfly_Direction", "1" }, { "Butterfly_Colors", "0" } },
          ",E_SLIDER_Butterfly_Chunks=3,E_SLIDER_Butterfly_Skip=2,E_SLIDER_Butterfly_Speed=50,E_SLIDER_Butterfly_Style=4,E_CHOICE_Butterfly_Direction=1,E_CHOICE_Butterfly_Colors=Rainbow" },
        { "Butterfly", { { "Butterfly_BkgrdChunks", "3" }, { "Butterfly_BkgrdSkip", "2" }, { "Butterfly_Style", "4" }, { "Butterfly_Direction", "1" }, { "Butterfly_Colors", "1" } },
          ",E_SLIDER_Butterfly_Chunks=3,E_SLIDER_Butterfly_Skip=2,E_SLIDER_Butterfly_Speed=50,E_SLIDER_Butterfly_Style=4,E_CHOICE_Butterfly_Direction=1,E_CHOICE_Butterfly_Colors=Palette" },
        { "ColorWash", { { "ColorWash_Count", "2" }, { "ColorWash_FadeHorizontal", "true" }, { "ColorWash_FadeVertical", "false" } }, ",E_TEXTCTRL_ColorWash_Cycles=2,E_CHECKBOX_ColorWash_HFade=1,E_CHECKBOX_ColorWash_VFade=0" },
        { "Fire", { { "Fire_Height", "40" }, { "Fire_Hue", "5" } }, ",E_SLIDER_Fire_Height=40,E_SLIDER_Fire_HueShift=5" },
        { "Garlands", { { "Garland_Spacing", "3" }, { "Garland_Type", "2" } }, ",E_SLIDER_Garlands_Spacing=3,E_SLIDER_Garlands_Type=2" },
        { "Glediator", { { "Glediator_Filename", "g.gled" } }, ",E_FILEPICKERCTRL_Glediator_Filename=g.gled" },
        { "Life", { { "Life_CellsToStart", "9" }, { "Life_Type", "1" } }, ",E_SLIDER_Life_Count=9,E_SLIDER_Life_Seed=1,E_SLIDER_Life_Speed=15" },
        { "Meteors", { { "Meteor_Colors", "1" }, { "Meteor_Count", "7" }, { "Meteor_TrailLength", "8" } }, ",E_CHOICE_Meteors_Type=Range,E_SLIDER_Meteors_Count=7,E_SLIDER_Meteors_Length=8,E_SLIDER_Meteors_Speed=25" },
        { "Meteors", { { "Meteor_Colors", "0" }, { "Meteor_Count", "7" }, { "Meteor_TrailLength", "8" } }, ",E_CHOICE_Meteors_Type=Rainbow,E_SLIDER_Meteors_Count=7,E_SLIDER_Meteors_Length=8,E_SLIDER_Meteors_Speed=25" },
        { "Meteors", { { "Meteor_Colors", "2" }, { "Meteor_Count", "7" }, { "Meteor_TrailLength", "8" } }, ",E_CHOICE_Meteors_Type=Palette,E_SLIDER_Meteors_Count=7,E_SLIDER_Meteors_Length=8,E_SLIDER_Meteors_Speed=25" },
        { "Movie", { { "Movie_DataPath", "m.mp4" } }, ",E_FILEPICKERCTRL_Video_Filename=m.mp4" },
        { "PictureTile", { { "PictureTile_FileName", "t.png" } }, ",E_TEXTCTRL_Pictures_Filename=t.png" },
        { "Snowflakes", { { "Snowflakes_Max", "4" }, { "Snowflakes_Type", "1" } }, ",E_SLIDER_Snowflakes_Count=20,E_SLIDER_Snowflakes_Speed=25,E_SLIDER_Snowflakes_Type=1" },
        { "Snowstorm", { { "Snowstorm_MaxFlakes", "9" }, { "Snowstorm_TrailLength", "3" } }, ",E_SLIDER_Snowstorm_Count=9,E_SLIDER_Snowstorm_Length=3,E_SLIDER_Snowstorm_Speed=25" },
        { "Spirals", { { "Spirals_3D", "true" }, { "Spirals_Blend", "false" }, { "Spirals_Thickness", "7" } }, ",E_CHECKBOX_Spirals_3D=1,E_CHECKBOX_Spirals_Blend=0,E_SLIDER_Spirals_Thickness=7" },
        { "Spirograph", { { "Spirograph_ROuter", "1" }, { "Spirograph_Distance", "2" }, { "Spirograph_RInner", "3" } }, ",E_SLIDER_Spirograph_R=1,E_SLIDER_Spirograph_d=2,E_SLIDER_Spirograph_r=3,E_TEXTCTRL_Spirograph_Speed=25" },
        { "Tree", { { "Tree_Branches", "5" } }, ",E_SLIDER_Tree_Branches=5,E_SLIDER_Tree_Speed=25" },
        { "Twinkles", { { "Twinkles_Strobe", "true" }, { "Twinkles_Count", "5" }, { "Twinkles_Steps", "6" } }, ",E_CHECKBOX_Twinkle_Strobe=1,E_CHOICE_Twinkle_Style=Old Render Method,E_SLIDER_Twinkle_Count=5,E_SLIDER_Twinkle_Steps=6" },
        { "Unknown", {}, "" },
    }));
    auto e = Eff("NutcrackerModuleData", NC(c.eff, c.kv));
    CAPTURE(c.eff);
    CHECK(e.GetSettings() == "B_CHOICE_BufferStyle=Horizontal Per Model/Strand" + c.tail);
}

TEST_CASE("Vixen3 nutcracker twinkle strobe flag is a clean boolean", "[impexp2][vixen]") {
    // "E_CHECKBOX_Twinkle_Strobe=1" + BoolToIntStr(...) yields 10 or 11 rather than 0 or 1
    auto e = Eff("NutcrackerModuleData", NC("Twinkles", { { "Twinkles_Strobe", "false" }, { "Twinkles_Count", "5" }, { "Twinkles_Steps", "6" } }));
    CHECK(e.GetSettings().find("E_CHECKBOX_Twinkle_Strobe=0,") != std::string::npos);
}

TEST_CASE("Vixen3 nutcracker fireworks velocity should not copy the fade value", "[impexp2][vixen]") {
    // E_SLIDER_Fireworks_Velocity is fed from Fireworks_Fade
    auto e = Eff("NutcrackerModuleData", NC("Fireworks", { { "Fireworks_Particles", "10" }, { "Fireworks_Explosions", "8" }, { "Fireworks_Fade", "30" }, { "Fireworks_Velocity", "70" } }));
    auto s = e.GetSettings();
    CHECK(s.find("E_SLIDER_Fireworks_Count=10,E_SLIDER_Fireworks_Explosions=4,E_SLIDER_Fireworks_Fade=30") != std::string::npos);
    CHECK(s.find("E_SLIDER_Fireworks_Velocity=70") != std::string::npos);
}

TEST_CASE("Vixen3 nutcracker butterfly setting names carry the slider prefix", "[impexp2][vixen]") {
    // chunks and skip are written as E_Butterfly_Chunks / E_Butterfly_Skip, not E_SLIDER_Butterfly_*
    auto e = Eff("NutcrackerModuleData", NC("Butterfly", { { "Butterfly_BkgrdChunks", "3" }, { "Butterfly_BkgrdSkip", "2" }, { "Butterfly_Style", "4" }, { "Butterfly_Direction", "1" }, { "Butterfly_Colors", "0" } }));
    CHECK(e.GetSettings().find("E_SLIDER_Butterfly_Chunks=3") != std::string::npos);
}

TEST_CASE("Vixen3 nutcracker picture and text directions", "[impexp2][vixen]") {
    const char* dirs[] = { "left", "right", "up", "down", "none", "up-left", "down-left", "up-right", "down-right", "peekaboo", "peekaboo 90", "peekaboo 180", "peekaboo 270", "wiggle" };
    int i = GENERATE(Catch::Generators::range(0, 16));
    auto e = Eff("NutcrackerModuleData", NC("Picture", { { "Picture_Direction", std::to_string(i) }, { "Picture_FileName", "p.png" }, { "Picture_GifSpeed", "2" } }));
    CAPTURE(i);
    std::string expected = i < 14 ? dirs[i] : "left";
    CHECK(e.GetSettings() == "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,E_CHOICE_Pictures_Direction=" + expected + ",E_TEXTCTRL_Pictures_Filename=p.png,E_TEXTCTRL_Pictures_Speed=2");
}

TEST_CASE("Vixen3 nutcracker text lines", "[impexp2][vixen]") {
    int i = GENERATE(Catch::Generators::range(0, 6));
    const char* dirs[] = { "left", "right", "up", "down", "none" };
    auto e = Eff("NutcrackerModuleData", NC("Text", { { "Picture_Direction", std::to_string(i) }, { "Text_CenterStop", "true" }, { "Text_Line1", "a" }, { "Text_Line2", "b" }, { "Text_Line3", "" }, { "Text_Line4", "d" } }));
    CAPTURE(i);
    std::string expected = i < 5 ? dirs[i] : "left";
    CHECK(e.GetSettings() == "B_CHOICE_BufferStyle=Horizontal Per Model/Strand,E_CHECKBOX_TextToCenter=1,E_CHOICE_Text_Dir=" + expected + ",E_TEXTCTRL_Text=a\nb\n\nd,E_TEXTCTRL_Text_Speed=25");
}

TEST_CASE("Vixen3 nutcracker curtain edges and effects", "[impexp2][vixen]") {
    const char* edges[] = { "left", "center", "right", "bottom", "middle", "top" };
    const char* effs[] = { "open", "close", "open then close", "close then open" };
    int i = GENERATE(Catch::Generators::range(0, 7));
    int j = GENERATE(Catch::Generators::range(0, 5));
    auto e = Eff("NutcrackerModuleData", NC("Curtain", { { "Curtain_Edge", std::to_string(i) }, { "Curtain_Effect", std::to_string(j) }, { "Curtain_Repeat", "true" }, { "Curtain_SwagWidth", "5" } }));
    CAPTURE(i, j);
    CHECK(e.GetSettings() == std::string("B_CHOICE_BufferStyle=Horizontal Per Model/Strand,E_CHECKBOX_Curtain_Repeat=1,E_CHOICE_Curtain_Edge=") + (i < 6 ? edges[i] : "left") + ",E_CHOICE_Curtain_Effect=" +
                                 (j < 4 ? effs[j] : "open") + ",E_SLIDER_Curtain_Swag=5");
}

TEST_CASE("Vixen3 nutcracker speed scaling", "[impexp2][vixen]") {
    struct C { const char* speed; const char* snow; };
    auto c = GENERATE(values<C>({ { "0", "0" }, { "1", "3" }, { "2", "5" }, { "20", "50" }, { "abc", "0" }, { "-4", "-10" }, { "", "0" } }));
    auto e = Eff("NutcrackerModuleData", NC("Snowstorm", { { "Snowstorm_MaxFlakes", "1" }, { "Snowstorm_TrailLength", "1" } }, "Vertical", c.speed));
    CAPTURE(c.speed);
    CHECK(e.GetSettings().find(std::string("E_SLIDER_Snowstorm_Speed=") + c.snow) != std::string::npos);
}

TEST_CASE("Vixen3 settings with a missing key must not throw", "[impexp2][vixen]") {
    // GetSettings uses settings.at() everywhere: a file lacking a key terminates the import with std::out_of_range
    for (auto t : { "AlternatingData", "BarsData", "ButterflyData", "TwinkleData", "WipeData", "NutcrackerModuleData" }) {
        VixenEffect e(0, 1, "x");
        e.type = t;
        CAPTURE(t);
        CHECK_NOTHROW(e.GetSettings());
    }
}

// ---------------------------------------------------------------- palette strings

TEST_CASE("Vixen3 palette strings", "[impexp2][vixen]") {
    VixenEffect e(0, 1, "x");
    CHECK(e.GetPalette().empty());
    e.palette.push_back({ VixenColor(xlColor(255, 0, 16), 0) });
    e.palette.push_back({ VixenColor(xlColor(0, 0, 0), 0.25f), VixenColor(xlColor(1, 2, 3), 1.0f) });
    CHECK(e.GetPalette() == "C_BUTTON_Palette1=#ff0010,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=Active=TRUE|Id=ID_BUTTON_Palette2|Values=x=0.250^c=#000000;x=1.000^c=#010203|,C_CHECKBOX_Palette2=1");
}

TEST_CASE("Vixen3 butterfly gradient is split into separate palette colours", "[impexp2][vixen]") {
    VixenEffect e(0, 1, "x");
    e.type = "ButterflyData";
    e.palette.push_back({ VixenColor(xlColor(1, 1, 1), 0), VixenColor(xlColor(2, 2, 2), 1) });
    CHECK(e.GetPalette() == "C_BUTTON_Palette1=#010101,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#020202,C_CHECKBOX_Palette2=1");
}

TEST_CASE("Vixen3 palette with an empty gradient entry", "[impexp2][vixen]") {
    VixenEffect e(0, 1, "x");
    e.palette.push_back({});
    CHECK(e.GetPalette() == "C_BUTTON_Palette1=Active=TRUE|Id=ID_BUTTON_Palette1|Values=|,C_CHECKBOX_Palette1=1");
}

TEST_CASE("Vixen3 pulse curve is only added with more than two points", "[impexp2][vixen]") {
    VixenEffect e(0, 1, "x");
    e.type = "PulseData";
    e.levelCurve = { VixenPoint(0, 0), VixenPoint(100, 100) };
    CHECK(e.GetPalette().empty());
    e.type = "SetLevelData";
    e.levelCurve = { VixenPoint(0, 0), VixenPoint(50, 100), VixenPoint(100, 100) };
    CHECK(e.GetPalette().empty());
}

TEST_CASE("Vixen3 known colour table spot checks", "[impexp2][vixen]") {
    CHECK(VixenEffect::KNOWN_COLOR[37] == 0xFF0000FFu);
    CHECK(VixenEffect::KNOWN_COLOR[141] == 0xFFFF0000u);
    CHECK(VixenEffect::KNOWN_COLOR[104] == 0xFF00FF00u);
    CHECK(VixenEffect::KNOWN_COLOR[0] == 0u);
    CHECK(sizeof(VixenEffect::KNOWN_COLOR) / sizeof(VixenEffect::KNOWN_COLOR[0]) == 175);
    CHECK(VixenEffect::KNOWN_COLOR[166] == 0xFFFFFF00u);
}

// ---------------------------------------------------------------- AddVixenMarksToLayer

namespace {
struct LayerFix {
    HeadlessRenderContext ctx;
    EffectLayer* layer = nullptr;
    LayerFix() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        layer = se.AddElement("T", "timing", true, false, true, false, false)->AddEffectLayer();
    }
};
}

TEST_CASE("AddVixenMarksToLayer snaps marks to the frame grid", "[impexp2][vixen]") {
    LayerFix f;
    std::list<VixenTiming> marks = { VixenTiming(0.0f, 1.0f, "a"), VixenTiming(1.0f, 2.03f, "b") };
    AddVixenMarksToLayer(marks, f.layer, 50);
    REQUIRE(f.layer->GetEffectCount() == 2);
    CHECK(f.layer->GetEffect(0)->GetStartTimeMS() == 0);
    CHECK(f.layer->GetEffect(0)->GetEndTimeMS() == 1000);
    CHECK(f.layer->GetEffect(1)->GetEndTimeMS() == 2050);
    CHECK(f.layer->GetEffect(1)->GetEffectName() == "b");
}

TEST_CASE("AddVixenMarksToLayer drops marks that do not fit", "[impexp2][vixen]") {
    LayerFix f;
    std::list<VixenTiming> marks = { VixenTiming(0.0f, 2.0f, "a"), VixenTiming(1.0f, 1.5f, "inside"), VixenTiming(1.0f, 3.0f, "overlap"), VixenTiming(5.0f, 4.0f, "reversed"), VixenTiming(4.0f, 4.0f, "zero") };
    AddVixenMarksToLayer(marks, f.layer, 25);
    REQUIRE(f.layer->GetEffectCount() == 2);
    CHECK(f.layer->GetEffect(1)->GetStartTimeMS() == 2000);
    CHECK(f.layer->GetEffect(1)->GetEndTimeMS() == 3000);
}

TEST_CASE("AddVixenMarksToLayer handles empty input", "[impexp2][vixen]") {
    LayerFix f;
    AddVixenMarksToLayer({}, f.layer, 50);
    CHECK(f.layer->GetEffectCount() == 0);
}

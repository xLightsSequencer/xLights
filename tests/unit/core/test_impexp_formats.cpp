#include <catch2/catch_test_macros.hpp>

#include "import_export/HLSFile.h"
#include "import_export/ImportDataRow.h"
#include "import_export/LORMusic.h"
#include "import_export/MapHintsIO.h"
#include "import_export/MusicCatalog.h"
#include "import_export/Vixen2File.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "utils/Base64.h"
#include "utils/Color.h"
#include "utils/ZipUtils.h"

#include <pugixml.hpp>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct LayerFixture {
    HeadlessRenderContext ctx;
    EffectLayer* layer = nullptr;
    LayerFixture() {
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* e = se.AddElement("Model", "model", true, false, true, false, false);
        layer = e->AddEffectLayer();
    }
    std::vector<std::tuple<std::string, int, int>> Effects() const {
        std::vector<std::tuple<std::string, int, int>> out;
        for (int i = 0; i < layer->GetEffectCount(); ++i) {
            Effect* ef = layer->GetEffect(i);
            out.emplace_back(ef->GetEffectName(), ef->GetStartTimeMS(), ef->GetEndTimeMS());
        }
        return out;
    }
};

pugi::xml_document Parse(const std::string& xml) {
    pugi::xml_document d;
    d.load_string(xml.c_str());
    return d;
}

std::string B64(const std::vector<uint8_t>& v) {
    return Base64::Encode(v.data(), v.size());
}

}

TEST_CASE("Map hints write and load round trip", "[impexp][maphints]") {
    TempDir d;
    std::vector<MapHintEntry> in{ { "^Star(s?)$", "Star", "B" }, { "Mega\\sTree", "Mega Tree", "M" }, { "^House$", "House", "G" } };
    REQUIRE(WriteMapHintsFile(d.File("a.xmaphint"), in));
    auto out = LoadMapHintsFile(d.File("a.xmaphint"));
    REQUIRE(out.size() == 3);
    for (size_t i = 0; i < in.size(); ++i) {
        CHECK(out[i].toRegex == in[i].toRegex);
        CHECK(out[i].fromModel == in[i].fromModel);
        CHECK(out[i].applyTo == in[i].applyTo);
    }
}

TEST_CASE("Map hints writer defaults an empty applyTo and ignores the sample comment", "[impexp][maphints]") {
    TempDir d;
    REQUIRE(WriteMapHintsFile(d.File("b.xmaphint"), { { "x", "y", "" } }));
    auto out = LoadMapHintsFile(d.File("b.xmaphint"));
    REQUIRE(out.size() == 1);
    CHECK(out[0].applyTo == "B");
}

TEST_CASE("Map hints writer fails for an unwritable path", "[impexp][maphints]") {
    TempDir d;
    CHECK_FALSE(WriteMapHintsFile(d.File("no/such/dir/x.xmaphint"), {}));
}

TEST_CASE("Map hints loader skips incomplete entries and is case insensitive", "[impexp][maphints]") {
    TempDir d;
    auto f = d.Write("c.xmaphint",
        "<MapHints><MAP ToRegex=\"a\" FromModel=\"A\"/><map ToRegex=\"\" FromModel=\"B\"/>"
        "<map ToRegex=\"c\" FromModel=\"\"/><other ToRegex=\"d\" FromModel=\"D\"/></MapHints>");
    auto out = LoadMapHintsFile(f);
    REQUIRE(out.size() == 1);
    CHECK(out[0].toRegex == "a");
    CHECK(out[0].applyTo == "B");
}

TEST_CASE("Map hints loader copes with missing, empty and corrupt files", "[impexp][maphints]") {
    TempDir d;
    CHECK(LoadMapHintsFile(d.File("none.xmaphint")).empty());
    CHECK(LoadMapHintsFile(d.Write("e.xmaphint", "")).empty());
    CHECK(LoadMapHintsFile(d.Write("j.xmaphint", "<<<>>> not xml")).empty());
    CHECK(LoadMapHintsFile(d.Write("n.xmaphint", std::string("\xff\xfe\x00\x01", 4))).empty());
}

TEST_CASE("Map hints are gathered from the maphints folder of a show", "[impexp][maphints]") {
    TempDir d;
    CHECK(LoadMapHintsFromShowDir("").empty());
    CHECK(LoadMapHintsFromShowDir(d.path.string()).empty());
    std::filesystem::create_directories(d.path / "maphints");
    REQUIRE(WriteMapHintsFile((d.path / "maphints" / "one.xmaphint").string(), { { "a", "A", "B" } }));
    REQUIRE(WriteMapHintsFile((d.path / "maphints" / "two.xmaphint").string(), { { "b", "B", "M" }, { "c", "C", "G" } }));
    d.Write("maphints/ignored.txt", "<MapHints><Map ToRegex=\"z\" FromModel=\"Z\"/></MapHints>");
    CHECK(LoadMapHintsFromShowDir(d.path.string()).size() == 3);
}

TEST_CASE("ImportDataRow Base64Decode decodes padded and unpadded input", "[impexp][dataRow]") {
    auto dec = [](const std::string& s) {
        std::vector<unsigned char> v;
        Base64Decode(s, v);
        return std::string(v.begin(), v.end());
    };
    CHECK(dec("") == "");
    CHECK(dec("Zg==") == "f");
    CHECK(dec("Zm8=") == "fo");
    CHECK(dec("Zm9v") == "foo");
    CHECK(dec("Zm9vYg==") == "foob");
    CHECK(dec("Zm9vYmE=") == "fooba");
    CHECK(dec("Zm9vYmFy") == "foobar");
}

TEST_CASE("ImportDataRow Base64Decode stops at invalid characters", "[impexp][dataRow]") {
    std::vector<unsigned char> v;
    Base64Decode("Zm9v!!!!Zm9v", v);
    CHECK(std::string(v.begin(), v.end()) == "foo");
    v.clear();
    Base64Decode("====", v);
    CHECK(v.empty());
    v.clear();
    std::string huge(100000, 'A');
    Base64Decode(huge, v);
    CHECK(v.size() == 75000);
    v.clear();
    Base64Decode(std::string("\xff\xfe\x80", 3), v);
    CHECK(v.empty());
}

TEST_CASE("ImportDataRow Base64Decode agrees with the shared Base64 helper", "[impexp][dataRow]") {
    std::vector<uint8_t> bytes;
    for (int i = 0; i < 300; ++i) {
        bytes.push_back(static_cast<uint8_t>((i * 37 + 11) & 0xFF));
    }
    for (size_t len : { 0u, 1u, 2u, 3u, 4u, 5u, 100u, 299u, 300u }) {
        std::vector<uint8_t> part(bytes.begin(), bytes.begin() + len);
        std::vector<unsigned char> out;
        Base64Decode(B64(part), out);
        CHECK(std::vector<uint8_t>(out.begin(), out.end()) == part);
    }
}

TEST_CASE("ConvertDataRowToEffects turns a run of one colour into an On effect", "[impexp][dataRow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 5; ++i) colors.push_back(xlRED);
    for (int i = 0; i < 5; ++i) colors.push_back(xlBLACK);
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(std::get<0>(e[0]) == "On");
    CHECK(std::get<1>(e[0]) == 0);
    CHECK(std::get<2>(e[0]) == 250);
}

TEST_CASE("ConvertDataRowToEffects emits one effect per colour change", "[impexp][dataRow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 2; ++i) colors.push_back(xlRED);
    for (int i = 0; i < 2; ++i) colors.push_back(xlGREEN);
    for (int i = 0; i < 2; ++i) colors.push_back(xlBLACK);
    for (int i = 0; i < 2; ++i) colors.push_back(xlBLUE);
    for (int i = 0; i < 2; ++i) colors.push_back(xlBLACK);
    ConvertDataRowToEffects(f.layer, colors, 100, false);
    auto e = f.Effects();
    REQUIRE(e.size() == 3);
    CHECK(e[0] == std::make_tuple(std::string("On"), 0, 200));
    CHECK(e[1] == std::make_tuple(std::string("On"), 200, 400));
    CHECK(e[2] == std::make_tuple(std::string("On"), 600, 800));
}

TEST_CASE("ConvertDataRowToEffects detects a ramp between two colours", "[impexp][dataRow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 10; ++i) {
        colors.push_back(xlColor(i * 25, 0, 255 - i * 25));
    }
    for (int i = 0; i < 4; ++i) colors.push_back(xlBLACK);
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    auto e = f.Effects();
    REQUIRE(e.size() >= 1);
    CHECK(std::get<0>(e[0]) == "Color Wash");
    CHECK(std::get<1>(e[0]) == 0);
    CHECK(std::get<2>(e[0]) == 500);
}

TEST_CASE("ConvertDataRowToEffects erases existing effects when asked", "[impexp][dataRow]") {
    LayerFixture f;
    f.layer->AddEffect(0, "Bars", "", "", 0, 1000, 0, false);
    xlColorVector colors(8, xlBLACK);
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    CHECK(f.layer->GetEffectCount() == 1);
    xlColorVector colors2(8, xlBLACK);
    ConvertDataRowToEffects(f.layer, colors2, 50, true);
    CHECK(f.layer->GetEffectCount() == 0);
}

TEST_CASE("ConvertDataRowToEffects does not overwrite effects already in the range", "[impexp][dataRow]") {
    LayerFixture f;
    f.layer->AddEffect(0, "Bars", "", "", 0, 250, 0, false);
    xlColorVector colors;
    for (int i = 0; i < 5; ++i) colors.push_back(xlRED);
    for (int i = 0; i < 5; ++i) colors.push_back(xlBLACK);
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    CHECK(f.layer->GetEffectCount() == 1);
}

TEST_CASE("ConvertDataRowToEffects with fewer than three colours must not run off the end", "[impexp][dataRow]") {
    LayerFixture f;
    xlColorVector colors;
    colors.push_back(xlRED);
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    colors.clear();
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    colors = { xlRED, xlBLUE };
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    SUCCEED();
}

namespace {
std::string HlsDoc(int frames, int frameMs, const std::string& channels) {
    return "<HLSFile><NumberOfTimeCells>" + std::to_string(frames) + "</NumberOfTimeCells><MilliSecPerTimeUnit>" + std::to_string(frameMs) +
           "</MilliSecPerTimeUnit><TotalUniverses><Universe><Channels>" + channels + "</Channels></Universe></TotalUniverses></HLSFile>";
}
std::string HlsChan(const std::string& info, const std::string& data) {
    return "<ChannelData><ChanInfo>" + info + "</ChanInfo><IlluminationData>" + data + "</IlluminationData></ChannelData>";
}
}

TEST_CASE("HLS file indexes channels and frames", "[impexp][hls]") {
    auto doc = Parse(HlsDoc(10, 25,
        HlsChan("Icicle 2, Normal", "<a>0-FF</a>") + HlsChan("Icicle 10, Normal", "<a>0-FF</a>") + HlsChan("Icicle 1, Normal", "<a>0-FF</a>") +
        HlsChan("Tree_0001, RGB-R", "<a>0-FF</a>") + HlsChan("Tree_0001, RGB-G", "<a>0-FF</a>") + HlsChan("Tree_0001, RGB-B", "<a>0-FF</a>")));
    HLSFile h(doc);
    CHECK(h.GetFrames() == 10);
    CHECK(h.GetFrameTime() == 25);
    auto names = h.GetChannelNames();
    REQUIRE(names.size() >= 6);
    CHECK(names[0] == "Icicle 1");
    CHECK(names[1] == "Icicle 2");
    CHECK(names[2] == "Icicle 10");
    CHECK(h.GetChannelColor("Icicle 1") == xlWHITE);
    CHECK(h.GetChannelColor("Tree_0001, RGB-R") == xlRED);
    CHECK(h.GetChannelColor("Tree_0001, RGB-G") == xlGREEN);
    CHECK(h.GetChannelColor("Tree_0001, RGB-B") == xlBLUE);
    CHECK(h.GetChannelColor("Tree_0001") == xlBLACK);
    CHECK(h.GetChannelColor("missing") == xlWHITE);
}

TEST_CASE("HLS file maps a normal channel to On effects", "[impexp][hls]") {
    auto doc = Parse(HlsDoc(10, 50, HlsChan("Lamp, Normal", "<a>0-FF,FF,FF,FF,FF,00,00,00,00,00</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "Lamp", xlWHITE, true));
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(e[0] == std::make_tuple(std::string("On"), 0, 250));
}

TEST_CASE("HLS file maps rgb legs together", "[impexp][hls]") {
    auto doc = Parse(HlsDoc(10, 50,
        HlsChan("Lamp, RGB-R", "<a>0-FF,FF,FF,FF,FF</a>") + HlsChan("Lamp, RGB-G", "<a>0-00</a>") + HlsChan("Lamp, RGB-B", "<a>0-00</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "Lamp", xlWHITE, false));
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(std::get<1>(e[0]) == 0);
    CHECK(std::get<2>(e[0]) == 250);
}

TEST_CASE("HLS file refuses unknown channels and null layers", "[impexp][hls]") {
    auto doc = Parse(HlsDoc(10, 50, HlsChan("Lamp, Normal", "<a>0-FF</a>")));
    HLSFile h(doc);
    LayerFixture f;
    CHECK_FALSE(h.MapChannelEffects(f.layer, "Other", xlWHITE, false));
    CHECK_FALSE(h.MapChannelEffects(f.layer, "", xlWHITE, false));
    CHECK_FALSE(h.MapChannelEffects(nullptr, "Lamp", xlWHITE, false));
}

TEST_CASE("HLS file ignores out of range data offsets", "[impexp][hls]") {
    auto doc = Parse(HlsDoc(5, 50, HlsChan("Lamp, Normal", "<a>-5-FF,FF</a><a>9999-FF</a><a>zzz</a><a></a><a>3-GG,,</a>")));
    HLSFile h(doc);
    LayerFixture f;
    h.MapChannelEffects(f.layer, "Lamp", xlWHITE, false);
    SUCCEED();
}

TEST_CASE("HLS file survives empty and malformed documents", "[impexp][hls]") {
    pugi::xml_document empty;
    HLSFile h(empty);
    CHECK(h.GetFrames() == 0);
    CHECK(h.GetChannelNames().empty());
    auto junk = Parse("<HLSFile><NumberOfTimeCells>abc</NumberOfTimeCells><TotalUniverses><Universe><Channels><ChannelData><ChanInfo>, Normal</ChanInfo></ChannelData></Channels></Universe></TotalUniverses></HLSFile>");
    HLSFile h2(junk);
    CHECK(h2.GetFrames() == 0);
}

TEST_CASE("HLS FindStrandName picks the first matching padding", "[impexp][hls]") {
    auto doc = Parse(HlsDoc(5, 50, HlsChan("Str P005, Normal", "<a>0-FF</a>")));
    HLSFile h(doc);
    auto r = h.FindStrandName("Str", 5);
    CHECK(r.find("5") != std::string::npos);
}

namespace {
std::string VixDoc(int totalMs, int period, const std::string& channels, const std::vector<uint8_t>& events, const std::string& extra = "") {
    return "<Program>" + extra + "<Time>" + std::to_string(totalMs) + "</Time><EventPeriodInMilliseconds>" + std::to_string(period) +
           "</EventPeriodInMilliseconds><Channels>" + channels + "</Channels><EventValues>" + B64(events) + "</EventValues></Program>";
}
}

TEST_CASE("Vixen 2 file reads channels, frames and event data", "[impexp][vixen2]") {
    std::vector<uint8_t> ev(20, 0);
    for (int i = 0; i < 5; ++i) ev[i] = 255;
    auto doc = Parse(VixDoc(500, 50, "<Channel name=\"Lamp\" color=\"16711680\"/><Channel name=\"Other\" color=\"255\"/>", ev));
    Vixen2File v(doc, "", "");
    CHECK(v.IsValid());
    CHECK(v.GetFrames() == 10);
    CHECK(v.GetFrameTime() == 50);
    CHECK(v.GetChannelNames() == std::vector<std::string>{ "Lamp", "Other" });
    CHECK(v.GetChannelColor("Lamp") == xlColor(0xFF0000, false));
    CHECK(v.GetChannelColor("unknown") == xlWHITE);

    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "Lamp", xlWHITE, true));
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(e[0] == std::make_tuple(std::string("On"), 0, 250));
}

TEST_CASE("Vixen 2 file collapses rgb legs into one channel", "[impexp][vixen2]") {
    std::vector<uint8_t> ev(30, 0);
    for (int i = 0; i < 4; ++i) ev[i] = 255;
    auto doc = Parse(VixDoc(500, 50, "<Channel name=\"Tree-R\"/><Channel name=\"Tree-G\"/><Channel name=\"Tree-B\"/><Channel name=\"Solo-R\"/>", ev));
    Vixen2File v(doc, "", "");
    REQUIRE(v.IsValid());
    auto names = v.GetChannelNames();
    CHECK(std::find(names.begin(), names.end(), "Tree") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Tree-R") == names.end());
    CHECK(std::find(names.begin(), names.end(), "Solo-R") != names.end());
    CHECK(v.GetChannelColor("Tree") == xlBLACK);

    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "Tree", xlWHITE, false));
    CHECK(f.layer->GetEffectCount() == 1);
    CHECK_FALSE(v.MapChannelEffects(f.layer, "Nope", xlWHITE, false));
}

TEST_CASE("Vixen 2 file defaults a bad frame period to 50 ms", "[impexp][vixen2]") {
    std::vector<uint8_t> ev(10, 0);
    auto doc = Parse(VixDoc(500, 0, "<Channel name=\"A\"/>", ev));
    Vixen2File v(doc, "", "");
    CHECK(v.GetFrameTime() == 50);
    CHECK(v.GetFrames() == 10);
}

TEST_CASE("Vixen 2 file reads channel names from a profile", "[impexp][vixen2]") {
    TempDir d;
    d.Write("My.pro", "<Profile><ChannelObjects><Channel name=\"P1\" color=\"255\"/><Channel>P2</Channel></ChannelObjects></Profile>");
    std::vector<uint8_t> ev(20, 0);
    auto doc = Parse("<Program><Profile>My</Profile><Time>500</Time><EventPeriodInMilliseconds>50</EventPeriodInMilliseconds><EventValues>" + B64(ev) + "</EventValues></Program>");
    Vixen2File v(doc, d.path.string(), d.path.string());
    CHECK(v.IsValid());
    CHECK(v.GetChannelNames() == std::vector<std::string>{ "P1", "P2" });
}

TEST_CASE("Vixen 2 file with a missing profile is invalid", "[impexp][vixen2]") {
    TempDir d;
    auto doc = Parse("<Program><Profile>Gone</Profile><Time>500</Time><EventValues>AAAA</EventValues></Program>");
    Vixen2File v(doc, d.path.string(), d.path.string());
    CHECK_FALSE(v.IsValid());
}

TEST_CASE("Vixen 2 file with missing sections is invalid and does not crash", "[impexp][vixen2]") {
    pugi::xml_document empty;
    Vixen2File a(empty, "", "");
    CHECK_FALSE(a.IsValid());
    auto d1 = Parse("<Program/>");
    Vixen2File b(d1, "", "");
    CHECK_FALSE(b.IsValid());
    auto d2 = Parse(VixDoc(0, 50, "<Channel name=\"A\"/>", { 1, 2, 3 }));
    Vixen2File c(d2, "", "");
    CHECK_FALSE(c.IsValid());
    LayerFixture f;
    CHECK_FALSE(c.MapChannelEffects(f.layer, "A", xlWHITE, false));
    auto d3 = Parse(VixDoc(500, 50, "<Channel name=\"A\"/>", {}));
    Vixen2File e(d3, "", "");
    CHECK_FALSE(e.IsValid());
}

TEST_CASE("Vixen 2 file with short event data reads missing frames as zero", "[impexp][vixen2]") {
    std::vector<uint8_t> ev(3, 255);
    auto doc = Parse(VixDoc(500, 50, "<Channel name=\"A\"/><Channel name=\"B\"/>", ev));
    Vixen2File v(doc, "", "");
    REQUIRE(v.IsValid());
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "B", xlWHITE, false));
    CHECK(f.layer->GetEffectCount() == 0);
}

namespace {
std::string LorDoc() {
    return "<sequence><channels>"
           "<channel name=\"Lamp\" color=\"255\" unit=\"1\" circuit=\"1\" savedIndex=\"0\">"
           "<effect type=\"intensity\" startCentisecond=\"0\" endCentisecond=\"100\" intensity=\"100\"/>"
           "<effect type=\"intensity\" startCentisecond=\"100\" endCentisecond=\"200\" startIntensity=\"0\" endIntensity=\"50\"/>"
           "<effect type=\"shimmer\" startCentisecond=\"200\" endCentisecond=\"300\" intensity=\"80\"/>"
           "</channel>"
           "<channel name=\"Lamp\" color=\"16711680\" unit=\"2\" circuit=\"3\" savedIndex=\"1\"/>"
           "<channel name=\"Dmx\" color=\"0\" unit=\"3\" circuit=\"1\" savedIndex=\"2\">"
           "<effect type=\"DMX intensity\" startCentisecond=\"0\" endCentisecond=\"50\" intensity=\"255\"/></channel>"
           "<rgbChannel name=\"Roof-P02\" savedIndex=\"3\"/><rgbChannel name=\"Roof-P01\" savedIndex=\"4\"/><rgbChannel name=\"Roof-P10\" savedIndex=\"5\"/>"
           "</channels>"
           "<timingGrids>"
           "<timingGrid name=\"Fixed\" type=\"fixed\"><timing centisecond=\"0\"/></timingGrid>"
           "<timingGrid name=\"Free\" type=\"freeform\"><timing centisecond=\"50\"/><timing centisecond=\"125\"/><timing centisecond=\"125\"/><timing centisecond=\"300\"/></timingGrid>"
           "<timingGrid type=\"freeform\"><timing centisecond=\"10\"/></timingGrid>"
           "</timingGrids></sequence>";
}
}

TEST_CASE("LOR music file indexes channels, duplicates and ccr names", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    auto names = l.GetChannelNames();
    CHECK(std::find(names.begin(), names.end(), "Lamp") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Lamp_Unit_2_Circuit_3") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Dmx") != names.end());
    CHECK(l.GetCCRNames() == std::vector<std::string>{ "Roof" });
    CHECK(std::find(names.begin(), names.end(), "Roof-P01") == names.end());
    CHECK(l.IsCCRName("Roof-P01"));
    CHECK_FALSE(l.IsCCRName("Roof"));
    CHECK_FALSE(l.IsCCRName("Lamp"));
    CHECK(l.GetChannelColor("Lamp") == xlColor(255, 0, 0));
    CHECK(l.GetChannelColor("Lamp_Unit_2_Circuit_3") == xlColor(0, 0, 255));
    CHECK(l.GetChannelColor("nothing") == xlBLACK);
}

TEST_CASE("LOR music file resolves ccr node names", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    CHECK(l.ResolveCCRNodeName("Roof", 0) == "Roof-P01");
    CHECK(l.ResolveCCRNodeName("Other", 3) == "Other-P04");
}

TEST_CASE("LOR music file lists freeform timing tracks and rounds marks", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    CHECK(l.GetTimingTracks() == std::vector<std::string>{ "Free" });
    auto t = l.GetTimings("Free");
    REQUIRE(t.size() == 3);
    CHECK(t[0] == std::make_pair(0u, 500u));
    CHECK(t[1] == std::make_pair(500u, 1250u));
    CHECK(t[2] == std::make_pair(1250u, 3000u));
    auto off = l.GetTimings("Free", 1000);
    REQUIRE(off.size() == 3);
    CHECK(off[0].first == 1000u);
    CHECK(l.GetTimings("nope").empty());
}

TEST_CASE("LOR music file maps intensity effects to On effects", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    LayerFixture f;
    REQUIRE(l.MapChannelEffects(f.layer, "Lamp", xlWHITE, 1, true));
    REQUIRE(f.layer->GetEffectCount() == 3);
    Effect* a = f.layer->GetEffect(0);
    CHECK(a->GetStartTimeMS() == 0);
    CHECK(a->GetEndTimeMS() == 1000);
    CHECK(a->GetEffectName() == "On");
    Effect* b = f.layer->GetEffect(1);
    CHECK(b->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "0");
    CHECK(b->GetSettings().Get("E_TEXTCTRL_Eff_On_End", "") == "50");
    Effect* c = f.layer->GetEffect(2);
    CHECK(c->GetSettings().Get("E_CHECKBOX_On_Shimmer", "") == "1");
    CHECK(c->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "80");
}

TEST_CASE("LOR music file scales DMX intensity from 255 to 100", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    LayerFixture f;
    REQUIRE(l.MapChannelEffects(f.layer, "Dmx", xlWHITE, 1, false));
    REQUIRE(f.layer->GetEffectCount() == 1);
    CHECK(f.layer->GetEffect(0)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "x") == "x");
}

TEST_CASE("LOR music file uses the node colour for multi channel nodes", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    LayerFixture f;
    REQUIRE(l.MapChannelEffects(f.layer, "Lamp", xlGREEN, 3, false));
    REQUIRE(f.layer->GetEffectCount() == 3);
    INFO(f.layer->GetEffect(0)->GetPaletteAsString());
    CHECK(f.layer->GetEffect(0)->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#00ff00");
}

TEST_CASE("LOR music file refuses unknown channels and keeps effects when not erasing", "[impexp][lormusic]") {
    auto doc = Parse(LorDoc());
    LORMusic l(doc, 20);
    LayerFixture f;
    f.layer->AddEffect(0, "Bars", "", "", 5000, 6000, 0, false);
    CHECK_FALSE(l.MapChannelEffects(f.layer, "Unknown", xlWHITE, 1, false));
    CHECK_FALSE(l.MapChannelEffects(f.layer, "", xlWHITE, 1, false));
    CHECK_FALSE(l.MapChannelEffects(nullptr, "Lamp", xlWHITE, 1, false));
    CHECK(f.layer->GetEffectCount() == 1);
    CHECK_FALSE(l.MapChannelEffects(f.layer, "Unknown", xlWHITE, 1, true));
    CHECK(f.layer->GetEffectCount() == 0);
}

TEST_CASE("LOR music file maps an rgb channel from its three colour channels", "[impexp][lormusic]") {
    auto doc = Parse(
        "<sequence><channels>"
        "<rgbChannel name=\"Tree\" savedIndex=\"9\"><channels><channel savedIndex=\"0\"/><channel savedIndex=\"1\"/><channel savedIndex=\"2\"/></channels></rgbChannel>"
        "<channel name=\"r\" savedIndex=\"0\"><effect type=\"intensity\" startCentisecond=\"0\" endCentisecond=\"100\" intensity=\"100\"/></channel>"
        "<channel name=\"g\" savedIndex=\"1\"/>"
        "<channel name=\"b\" savedIndex=\"2\"/>"
        "</channels></sequence>");
    LORMusic l(doc, 20);
    LayerFixture f;
    REQUIRE(l.MapChannelEffects(f.layer, "Tree", xlWHITE, 3, false));
    REQUIRE(f.layer->GetEffectCount() == 1);
    Effect* e = f.layer->GetEffect(0);
    CHECK(e->GetStartTimeMS() == 0);
    CHECK(e->GetEndTimeMS() == 1000);
    INFO(e->GetPaletteAsString());
    CHECK(e->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#ff0000");
}

TEST_CASE("LOR music file copes with empty and bogus documents", "[impexp][lormusic]") {
    pugi::xml_document empty;
    LORMusic a(empty, 20);
    CHECK(a.GetChannelNames().empty());
    CHECK(a.GetTimingTracks().empty());
    auto bogus = Parse("<sequence><channels><channel/><rgbChannel/><rgbChannel name=\"X P\"/><rgbChannel name=\"Y Pabc\"/></channels><timingGrids><timingGrid name=\"t\"/></timingGrids></sequence>");
    LORMusic b(bogus, 20);
    CHECK(b.GetTimings("t").empty());
}

namespace {
std::string MusicInventory() {
    return "<musicinventory><vendor><name>Acme</name><contact>Bob</contact><email>b@x</email><notes>line1\\nline2</notes></vendor>"
           "<categories><category><id>1</id><name>Pop</name><categories><category><id>2</id><name>Rock</name></category></categories></category></categories>"
           "<music>"
           "<song><title>Zebra</title><artist>Z</artist><hash>h1</hash><categoryid>2</categoryid><lyric/><download>http://x/y/zebra.xtiming?v=1</download></song>"
           "<song><title>apple</title><sequence/><categoryid>1</categoryid><categoryid>2</categoryid><download>http://x/apple.zip</download></song>"
           "<song><title>Mystery</title><download>http://x/file.dat</download><unexpected/></song>"
           "<song><title>Nodl</title><lyric/></song>"
           "</music></musicinventory>";
}
}

TEST_CASE("Music catalog parses vendor, categories and sorted songs", "[impexp][catalog]") {
    auto doc = Parse(MusicInventory());
    music_catalog::Vendor v(doc, -1);
    CHECK(v._name == "Acme");
    CHECK(v._contact == "Bob");
    REQUIRE(v._categories.size() == 1);
    auto* pop = v._categories.front();
    CHECK(pop->GetPath() == "Pop");
    REQUIRE(pop->_categories.size() == 1);
    CHECK(pop->_categories.front()->GetPath() == "Pop/Rock");
    REQUIRE(v._items.size() == 4);
    std::vector<std::string> titles;
    for (auto* i : v._items) titles.push_back(i->_title);
    CHECK(titles == std::vector<std::string>{ "apple", "Mystery", "Nodl", "Zebra" });
}

TEST_CASE("Music catalog items know their type, extension and file name", "[impexp][catalog]") {
    auto doc = Parse(MusicInventory());
    music_catalog::Vendor v(doc, -1);
    std::map<std::string, music_catalog::Item*> by;
    for (auto* i : v._items) by[i->_title] = i;
    CHECK(by["apple"]->GetTypeName() == "Sequence");
    CHECK(by["apple"]->GetExt() == "zip");
    CHECK(by["apple"]->GetDownloadFileName() == "apple.zip");
    CHECK(by["Zebra"]->GetTypeName() == "Lyrics");
    CHECK(by["Zebra"]->GetExt() == "xtiming");
    CHECK(by["Zebra"]->GetDownloadFileName() == "zebra.xtiming");
    CHECK(by["Mystery"]->GetTypeName() == "Unknown");
    CHECK(by["Mystery"]->GetExt() == "dat");
    CHECK(by["Nodl"]->GetDownloadFileName() == "Nodl.xtiming");
    CHECK(by["apple"]->InCategory("1"));
    CHECK(by["apple"]->InCategory("2"));
    CHECK_FALSE(by["Zebra"]->InCategory("1"));
}

TEST_CASE("Music catalog descriptions expand literal newlines", "[impexp][catalog]") {
    auto doc = Parse(MusicInventory());
    music_catalog::Vendor v(doc, -1);
    auto d = v.GetDescription();
    CHECK(d.find("Acme") != std::string::npos);
    CHECK(d.find("line1\nline2") != std::string::npos);
    CHECK(d.find("\\n") == std::string::npos);
    CHECK(v._items.front()->GetDescription().find("Type:") != std::string::npos);
}

TEST_CASE("Music catalog honours the max item count", "[impexp][catalog]") {
    auto doc = Parse(MusicInventory());
    music_catalog::Vendor v(doc, 2);
    CHECK(v._items.size() == 2);
}

TEST_CASE("Music catalog ignores documents with the wrong root", "[impexp][catalog]") {
    auto doc = Parse("<somethingelse><vendor><name>X</name></vendor></somethingelse>");
    music_catalog::Vendor v(doc, -1);
    CHECK(v._name.empty());
    CHECK(v._items.empty());
    pugi::xml_document empty;
    music_catalog::Vendor e(empty, -1);
    CHECK(e._items.empty());
}

TEST_CASE("Music catalog download refuses empty arguments", "[impexp][catalog]") {
    CHECK(music_catalog::Catalog::DownloadTo("", "x", "y").empty());
    CHECK(music_catalog::Catalog::DownloadTo("http://x", "", "y").empty());
    CHECK(music_catalog::Catalog::DownloadTo("http://x", "x", "").empty());
}

TEST_CASE("Music catalog download returns an existing file without fetching", "[impexp][catalog]") {
    TempDir d;
    d.Write("have.zip", "x");
    auto got = music_catalog::Catalog::DownloadTo("http://invalid.invalid/have.zip", d.path.string(), "have.zip");
    CHECK_FALSE(got.empty());
}

// Synthetic Vixen 2 (.vix/.pro), HLS (.hlsIdata), data-row conversion and map hint files.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/HLSFile.h"
#include "import_export/ImportDataRow.h"
#include "import_export/MapHintsIO.h"
#include "import_export/Vixen2File.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "utils/Base64.h"
#include "utils/Color.h"

#include <algorithm>
#include <cstdint>
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
        layer = se.AddElement("Model", "model", true, false, true, false, false)->AddEffectLayer();
    }
    struct E {
        std::string name;
        int start;
        int end;
        std::string settings;
        std::string palette;
    };
    std::vector<E> Effects() const {
        std::vector<E> out;
        for (int i = 0; i < layer->GetEffectCount(); ++i) {
            Effect* e = layer->GetEffect(i);
            out.push_back({ e->GetEffectName(), e->GetStartTimeMS(), e->GetEndTimeMS(), e->GetSettingsAsString(), e->GetPaletteAsString() });
        }
        return out;
    }
};

pugi::xml_document Parse(const std::string& xml) {
    pugi::xml_document d;
    d.load_string(xml.c_str());
    return d;
}
std::string B64(const std::vector<uint8_t>& v) { return Base64::Encode(v.data(), v.size()); }

std::string Vix(const std::string& time, const std::string& period, const std::string& channels, const std::string& events, const std::string& extra = "") {
    return "<Program>" + extra + "<Time>" + time + "</Time><EventPeriodInMilliseconds>" + period + "</EventPeriodInMilliseconds><Channels>" + channels + "</Channels><EventValues>" + events + "</EventValues></Program>";
}
std::string Ch(const std::string& name, const std::string& color = "") { return "<Channel name=\"" + name + "\"" + (color.empty() ? "" : " color=\"" + color + "\"") + "/>"; }

std::string HlsDoc(const std::string& frames, const std::string& ms, const std::string& channels) {
    return "<HLS_OutputSequence><NumberOfTimeCells>" + frames + "</NumberOfTimeCells><MilliSecPerTimeUnit>" + ms +
           "</MilliSecPerTimeUnit><TotalUniverses><Universe><Channels>" + channels + "</Channels></Universe></TotalUniverses></HLS_OutputSequence>";
}
std::string HlsChan(const std::string& info, const std::string& data) { return "<ChannelData><ChanInfo>" + info + "</ChanInfo><IlluminationData>" + data + "</IlluminationData></ChannelData>"; }

} // namespace

// ---------------------------------------------------------------- Vixen 2

TEST_CASE("Vixen2 frame count rounds up", "[impexp2][vixen2]") {
    struct C { const char* time; const char* period; int frames; int ft; };
    auto c = GENERATE(values<C>({ { "500", "50", 10, 50 }, { "501", "50", 11, 50 }, { "1", "50", 1, 50 }, { "49", "50", 1, 50 }, { "100", "33", 4, 33 }, { "1000", "1", 1000, 1 }, { "500", "-50", 10, 50 },
                                  { "500", "abc", 10, 50 }, { "500", "", 10, 50 }, { "abc", "50", 0, 50 }, { "-500", "50", -10, 50 }, { "0", "50", 0, 50 } }));
    auto doc = Parse(Vix(c.time, c.period, Ch("A"), "AAAA"));
    Vixen2File v(doc, "", "");
    CAPTURE(c.time, c.period);
    CHECK(v.GetFrames() == c.frames);
    CHECK(v.GetFrameTime() == c.ft);
    CHECK(v.IsValid() == (c.frames > 0));
}

TEST_CASE("Vixen2 channel names come from name attribute or text", "[impexp2][vixen2]") {
    auto doc = Parse(Vix("500", "50", "<Channel name=\"Named\">ignored</Channel><Channel>Texted</Channel><Channel name=\"\">FromText</Channel><Channel/><Channel name=\"\"/><Other>x</Other>", "AAAA"));
    Vixen2File v(doc, "", "");
    CHECK(v.GetChannelNames() == std::vector<std::string>{ "FromText", "Named", "Texted" });
}

TEST_CASE("Vixen2 duplicate channel names are listed once and the first wins", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev(20, 0);
    for (int i = 0; i < 10; ++i) ev[i] = 255;
    auto doc = Parse(Vix("500", "50", Ch("Dup") + Ch("Dup"), B64(ev)));
    Vixen2File v(doc, "", "");
    CHECK(v.GetChannelNames().size() == 1);
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "Dup", xlWHITE, false));
    CHECK(f.Effects().size() == 1);
}

TEST_CASE("Vixen2 channel colours", "[impexp2][vixen2]") {
    struct C { const char* col; unsigned int rgb; };
    auto c = GENERATE(values<C>({ { "16777215", 0xFFFFFF }, { "0", 0 }, { "255", 0xFF }, { "16711680", 0xFF0000 }, { "4278190335", 0x0000FF }, { "abc", 0 }, { "-1", 0xFFFFFF } }));
    auto doc = Parse(Vix("500", "50", Ch("A", c.col), "AAAA"));
    Vixen2File v(doc, "", "");
    CAPTURE(c.col);
    CHECK(v.GetChannelColor("A") == xlColor(c.rgb, false));
}

TEST_CASE("Vixen2 channel names sort with embedded numbers", "[impexp2][vixen2]") {
    auto doc = Parse(Vix("500", "50", Ch("Ch 10") + Ch("Ch 2") + Ch("Ch 1") + Ch("ch 3") , "AAAA"));
    Vixen2File v(doc, "", "");
    auto n = v.GetChannelNames();
    REQUIRE(n.size() == 4);
    CHECK(n[0] == "Ch 1");
    CHECK(n[1] == "Ch 2");
}

TEST_CASE("Vixen2 rgb collapsing variants", "[impexp2][vixen2]") {
    struct C { std::vector<std::string> chans; std::vector<std::string> expect; };
    auto c = GENERATE(values<C>({
        { { "TreeRed", "TreeGreen", "TreeBlue" }, { "Tree" } },
        { { "Tree-R", "Tree-G", "Tree-B" }, { "Tree" } },
        { { "Tree-R", "TreeGreen", "TreeBlue" }, { "Tree", "TreeGreen", "TreeBlue" } }, // mixed suffix styles: the green/blue legs also stay listed
        { { "TreeRed", "Tree-G", "Tree-B" }, { "Tree", "Tree-G", "Tree-B" } },
        { { "TreeRed", "TreeGreen" }, { "TreeGreen", "TreeRed" } },
        { { "Tree-R", "Tree-G" }, { "Tree-G", "Tree-R" } },
        { { "Red", "Green", "Blue" }, { "Blue", "Green", "Red" } },
        { { "A-R", "A-G", "A-B", "B-R", "B-G" }, { "A", "B-G", "B-R" } },
        { { "X-R", "X-G", "X-B", "X-R" }, { "X" } },
    }));
    std::string chans;
    for (auto& n : c.chans) chans += Ch(n);
    auto doc = Parse(Vix("500", "50", chans, "AAAA"));
    Vixen2File v(doc, "", "");
    auto names = v.GetChannelNames();
    std::sort(names.begin(), names.end());
    auto ex = c.expect;
    std::sort(ex.begin(), ex.end());
    CAPTURE(chans);
    CHECK(names == ex);
}

TEST_CASE("Vixen2 rgb channel maps its three event streams", "[impexp2][vixen2]") {
    // 4 frames, channels R,G,B; red on frames 0-1, green on 2-3
    std::vector<uint8_t> ev = { 255, 255, 0, 0, /*G*/ 0, 0, 255, 255, /*B*/ 0, 0, 0, 0 };
    auto doc = Parse(Vix("200", "50", Ch("L-R") + Ch("L-G") + Ch("L-B"), B64(ev)));
    Vixen2File v(doc, "", "");
    REQUIRE(v.IsValid());
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "L", xlWHITE, false));
    auto e = f.Effects();
    REQUIRE(e.size() == 2);
    CHECK(e[0].start == 0);
    CHECK(e[0].end == 100);
    CHECK(e[0].palette == "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1");
    CHECK(e[1].start == 100);
    CHECK(e[1].end == 200);
    CHECK(e[1].palette == "C_BUTTON_Palette1=#00ff00,C_CHECKBOX_Palette1=1");
}

TEST_CASE("Vixen2 rgb legs can be mapped by their collapsed name only", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev(12, 255);
    auto doc = Parse(Vix("200", "50", Ch("L-R") + Ch("L-G") + Ch("L-B"), B64(ev)));
    Vixen2File v(doc, "", "");
    LayerFixture f;
    CHECK(v.MapChannelEffects(f.layer, "L", xlWHITE, false));
    CHECK_FALSE(v.MapChannelEffects(f.layer, "L-", xlWHITE, false));
}

TEST_CASE("Vixen2 single channel intensity scales the supplied colour", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev = { 255, 255, 255, 255, 0, 0, 0, 0 };
    auto doc = Parse(Vix("200", "50", Ch("A") + Ch("B"), B64(ev)));
    Vixen2File v(doc, "", "");
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "A", xlColor(0, 255, 0), false));
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(e[0].palette == "C_BUTTON_Palette1=#00ff00,C_CHECKBOX_Palette1=1");
    CHECK(e[0].end == 200);
    LayerFixture g;
    REQUIRE(v.MapChannelEffects(g.layer, "B", xlWHITE, false));
    CHECK(g.Effects().empty());
}

TEST_CASE("Vixen2 ramp becomes an On effect with start and end intensity", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev(10);
    for (int i = 0; i < 10; ++i) ev[i] = (uint8_t)(i * 28);
    auto doc = Parse(Vix("500", "50", Ch("A"), B64(ev)));
    Vixen2File v(doc, "", "");
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "A", xlWHITE, true));
    auto e = f.Effects();
    REQUIRE(!e.empty());
    CHECK(e[0].name == "On");
    CHECK(e[0].start == 0);
    CHECK(e[0].settings.find("E_TEXTCTRL_Eff_On_Start=0") != std::string::npos);
}

TEST_CASE("Vixen2 map erases or keeps existing effects", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev(10, 255);
    auto doc = Parse(Vix("500", "50", Ch("A"), B64(ev)));
    Vixen2File v(doc, "", "");
    LayerFixture f;
    f.layer->AddEffect(0, "Off", "", "", 1000, 1100, false, false);
    f.layer->AddEffect(0, "Off", "", "", 2000, 2100, false, false);
    REQUIRE(v.MapChannelEffects(f.layer, "A", xlWHITE, false));
    CHECK(f.layer->GetEffectCount() == 3);
    REQUIRE(v.MapChannelEffects(f.layer, "A", xlWHITE, true));
    CHECK(f.layer->GetEffectCount() == 1);
}

TEST_CASE("Vixen2 map refuses bad arguments", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev(10, 255);
    auto doc = Parse(Vix("500", "50", Ch("A"), B64(ev)));
    Vixen2File v(doc, "", "");
    LayerFixture f;
    CHECK_FALSE(v.MapChannelEffects(nullptr, "A", xlWHITE, false));
    CHECK_FALSE(v.MapChannelEffects(f.layer, "", xlWHITE, false));
    CHECK_FALSE(v.MapChannelEffects(f.layer, "Missing", xlWHITE, false));
    CHECK_FALSE(v.MapChannelEffects(f.layer, "ARed", xlWHITE, false));
}

TEST_CASE("Vixen2 event data with line breaks is decoded as far as the first break", "[impexp2][vixen2]") {
    std::vector<uint8_t> ev(100, 255);
    std::string b = B64(ev);
    std::string broken = b.substr(0, 16) + "\r\n" + b.substr(16);
    auto doc = Parse(Vix("2500", "50", Ch("A"), broken));
    Vixen2File v(doc, "", "");
    REQUIRE(v.IsValid());
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "A", xlWHITE, false));
    // 12 bytes decoded; frames beyond that read as zero
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(e[0].end == 600);
}

TEST_CASE("Vixen2 event data with junk is not valid when nothing decodes", "[impexp2][vixen2]") {
    auto doc = Parse(Vix("500", "50", Ch("A"), "!!!!"));
    Vixen2File v(doc, "", "");
    CHECK_FALSE(v.IsValid());
}

TEST_CASE("Vixen2 profile lookup prefers the profile folder", "[impexp2][vixen2]") {
    TempDir d;
    std::filesystem::create_directories(d.path / "prof");
    std::filesystem::create_directories(d.path / "vix");
    d.Write("prof/P.pro", "<Profile><ChannelObjects><Channel name=\"FromProfileDir\"/></ChannelObjects></Profile>");
    d.Write("vix/P.pro", "<Profile><ChannelObjects><Channel name=\"FromVixDir\"/></ChannelObjects></Profile>");
    auto doc = Parse("<Program><Profile>P</Profile><Time>500</Time><EventValues>AAAA</EventValues></Program>");
    Vixen2File v(doc, (d.path / "vix").string(), (d.path / "prof").string());
    CHECK(v.GetChannelNames() == std::vector<std::string>{ "FromProfileDir" });
    Vixen2File w(doc, (d.path / "vix").string(), (d.path / "none").string());
    CHECK(w.GetChannelNames() == std::vector<std::string>{ "FromVixDir" });
}

TEST_CASE("Vixen2 profile channels precede inline channels in the event stream", "[impexp2][vixen2]") {
    TempDir d;
    d.Write("P.pro", "<Profile><ChannelObjects><Channel name=\"P1\"/></ChannelObjects></Profile>");
    std::vector<uint8_t> ev(20, 0);
    for (int i = 0; i < 10; ++i) ev[i] = 255; // first channel only
    auto doc = Parse("<Program><Profile>P</Profile><Time>500</Time><EventPeriodInMilliseconds>50</EventPeriodInMilliseconds><Channels>" + Ch("Inline") + "</Channels><EventValues>" + B64(ev) + "</EventValues></Program>");
    Vixen2File v(doc, d.path.string(), d.path.string());
    REQUIRE(v.IsValid());
    CHECK(v.GetChannelNames() == std::vector<std::string>{ "Inline", "P1" });
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "P1", xlWHITE, false));
    CHECK(f.Effects().size() == 1);
    LayerFixture g;
    REQUIRE(v.MapChannelEffects(g.layer, "Inline", xlWHITE, false));
    CHECK(g.Effects().empty());
}

TEST_CASE("Vixen2 profile oddities", "[impexp2][vixen2]") {
    TempDir d;
    d.Write("Bad.pro", "<<<not xml");
    d.Write("Empty.pro", "");
    d.Write("NoObjects.pro", "<Profile/>");
    d.Write("Mixed.pro", "<Profile><ChannelObjects><Other name=\"no\"/><Channel color=\"255\">TextName</Channel><Channel name=\"N\" color=\"zz\"/><Channel/></ChannelObjects><ChannelObjects><Channel name=\"Second\"/></ChannelObjects></Profile>");
    for (std::string p : { "Bad", "Empty", "NoObjects" }) {
        auto doc = Parse("<Program><Profile>" + p + "</Profile><Time>500</Time><EventValues>AAAA</EventValues></Program>");
        Vixen2File v(doc, d.path.string(), d.path.string());
        CAPTURE(p);
        CHECK(v.GetChannelNames().empty());
        CHECK_FALSE(v.IsValid());
    }
    auto doc = Parse("<Program><Profile>Mixed</Profile><Time>500</Time><EventValues>AAAA</EventValues></Program>");
    Vixen2File v(doc, d.path.string(), d.path.string());
    auto n = v.GetChannelNames();
    CHECK(std::find(n.begin(), n.end(), "TextName") != n.end());
    CHECK(std::find(n.begin(), n.end(), "Second") != n.end());
    CHECK(v.GetChannelColor("TextName") == xlColor(255, false));
}

TEST_CASE("Vixen2 profile name with path separators does not escape", "[impexp2][vixen2]") {
    TempDir d;
    auto doc = Parse("<Program><Profile>../../../etc/passwd</Profile><Time>500</Time><EventValues>AAAA</EventValues></Program>");
    Vixen2File v(doc, d.path.string(), d.path.string());
    CHECK_FALSE(v.IsValid());
}

TEST_CASE("Vixen2 wrong root names still parse", "[impexp2][vixen2]") {
    auto doc = Parse("<Whatever><Time>100</Time><EventPeriodInMilliseconds>50</EventPeriodInMilliseconds><Channels><Channel name=\"A\"/></Channels><EventValues>AAAAAAAA</EventValues></Whatever>");
    Vixen2File v(doc, "", "");
    CHECK(v.IsValid());
}

TEST_CASE("Vixen2 large frame count maps quickly", "[impexp2][vixen2]") {
    const int frames = 30000;
    std::vector<uint8_t> ev(frames * 2, 0);
    for (int i = 0; i < frames; ++i) ev[i] = (i / 10) % 2 ? 255 : 0;
    auto doc = Parse(Vix(std::to_string(frames * 50), "50", Ch("A") + Ch("B"), B64(ev)));
    Vixen2File v(doc, "", "");
    REQUIRE(v.IsValid());
    CHECK(v.GetFrames() == frames);
    LayerFixture f;
    REQUIRE(v.MapChannelEffects(f.layer, "A", xlWHITE, false));
    CHECK(f.layer->GetEffectCount() == frames / 20);
}

// ---------------------------------------------------------------- HLS

TEST_CASE("HLS indexes channel kinds", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("10", "25", HlsChan("Lamp, Normal", "<a>0-FF</a>") + HlsChan("Rgb, RGB-R", "") + HlsChan("Rgb, RGB-G", "") + HlsChan("Rgb, RGB-B", "") + HlsChan("Odd, RGB-Z", "") +
                                       HlsChan("No suffix", "") + HlsChan(", Normal", "") + HlsChan("X, Normal, Normal", "")));
    HLSFile h(doc);
    auto n = h.GetChannelNames();
    CHECK(std::find(n.begin(), n.end(), "Lamp") != n.end());
    CHECK(std::find(n.begin(), n.end(), "Rgb") != n.end());
    CHECK(std::find(n.begin(), n.end(), "Rgb, RGB-G") != n.end());
    CHECK(std::find(n.begin(), n.end(), "Odd, RGB-Z") != n.end());
    CHECK(std::find(n.begin(), n.end(), "No suffix") == n.end());
    CHECK(std::find(n.begin(), n.end(), "X") != n.end());
    CHECK(std::find(n.begin(), n.end(), "") != n.end());
    CHECK(h.GetChannelColor("Odd, RGB-Z") == xlBLUE);
    CHECK(h.GetChannelColor("Odd") == xlWHITE);
}

TEST_CASE("HLS frame fields", "[impexp2][hls]") {
    struct C { const char* f; const char* ms; int frames; int ft; };
    auto c = GENERATE(values<C>({ { "10", "50", 10, 50 }, { "", "", 0, 0 }, { "abc", "xyz", 0, 0 }, { "-3", "-4", -3, -4 }, { " 7 ", " 20 ", 7, 20 }, { "5.9", "25.5", 5, 25 } }));
    auto doc = Parse(HlsDoc(c.f, c.ms, ""));
    HLSFile h(doc);
    CAPTURE(c.f, c.ms);
    CHECK(h.GetFrames() == c.frames);
    CHECK(h.GetFrameTime() == c.ft);
}

TEST_CASE("HLS ignores misnested structure", "[impexp2][hls]") {
    auto doc = Parse("<HLS><TotalUniverses><Channels><ChannelData><ChanInfo>A, Normal</ChanInfo></ChannelData></Channels></TotalUniverses><Universe><Channels><ChannelData><ChanInfo>B, Normal</ChanInfo></ChannelData></Channels></Universe></HLS>");
    HLSFile h(doc);
    CHECK(h.GetChannelNames().empty());
    LayerFixture f;
    CHECK_FALSE(h.MapChannelEffects(f.layer, "A", xlWHITE, false));
}

TEST_CASE("HLS multiple universes are all indexed", "[impexp2][hls]") {
    auto doc = Parse("<HLS><NumberOfTimeCells>4</NumberOfTimeCells><MilliSecPerTimeUnit>50</MilliSecPerTimeUnit><TotalUniverses><Universe><Channels>" + HlsChan("A, Normal", "<a>0-FF,FF,FF,FF</a>") +
                     "</Channels></Universe><Universe><Channels>" + HlsChan("B, Normal", "<a>0-FF,FF</a>") + "</Channels><Channels>" + HlsChan("C, Normal", "") + "</Channels></Universe></TotalUniverses></HLS>");
    HLSFile h(doc);
    CHECK(h.GetChannelNames().size() == 3);
    LayerFixture f;
    CHECK(h.MapChannelEffects(f.layer, "B", xlWHITE, false));
    CHECK(f.Effects().size() == 1);
}

TEST_CASE("HLS illumination data blocks", "[impexp2][hls]") {
    // offset-hex,hex,...  later blocks overwrite earlier ones
    struct C { const char* blocks; int effects; int end; };
    auto c = GENERATE(values<C>({
        { "<a>0-FF,FF,FF,FF</a>", 1, 200 },
        { "<a>0-ff,ff,ff,ff</a>", 1, 200 },
        { "<a>2-FF,FF</a>", 1, 200 },
        { "<a>0-FF,FF,FF,FF</a><a>1-00,00</a>", 2, 200 },
        { "<a>0-FF</a><a>1-FF</a><a>2-FF</a><a>3-FF</a>", 1, 200 },
        { "<a>4-FF</a><a>100-FF</a>", 0, 0 },
        { "<a>0-1FF,FF,FF,FF</a>", 1, 200 },
        { "<a>0</a><a></a><a>-</a><a>x-y</a>", 0, 0 },
        { "<a>0-,,,</a>", 0, 0 },
        { "<a> 0-FF,FF,FF,FF</a>", 1, 200 },
    }));
    auto doc = Parse(HlsDoc("4", "50", HlsChan("L, Normal", c.blocks)));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
    CAPTURE(c.blocks);
    auto e = f.Effects();
    CHECK((int)e.size() == c.effects);
    if (c.effects > 0 && c.end > 0) CHECK(e.back().end <= c.end);
}

TEST_CASE("HLS single channel palette uses the supplied colour", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("6", "50", HlsChan("L, Normal", "<a>0-FF,FF,FF,00,00,00</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "L", xlColor(0, 0, 255), false));
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(e[0].start == 0);
    CHECK(e[0].end == 150);
    CHECK(e[0].palette == "C_BUTTON_Palette1=#0000ff,C_CHECKBOX_Palette1=1");
}

TEST_CASE("HLS rgb with a missing leg", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("4", "50", HlsChan("L, RGB-R", "<a>0-FF,FF,FF,FF</a>") + HlsChan("L, RGB-G", "<a>0-FF,FF,FF,FF</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
    auto e = f.Effects();
    REQUIRE(e.size() == 1);
    CHECK(e[0].palette == "C_BUTTON_Palette1=#ffff00,C_CHECKBOX_Palette1=1");
}

TEST_CASE("HLS blue only is not mappable", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("4", "50", HlsChan("L, RGB-B", "<a>0-FF,FF,FF,FF</a>")));
    HLSFile h(doc);
    LayerFixture f;
    CHECK_FALSE(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
}

TEST_CASE("HLS ramp end points of 1-3 percent are cleared so ramps are detected", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("12", "50", HlsChan("L, Normal", "<a>0-00,01,40,80,C0,FF,C0,80,40,01,00,00</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
    CHECK(f.Effects().size() >= 1);
}

TEST_CASE("HLS with zero frames maps to nothing", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("0", "50", HlsChan("L, Normal", "<a>0-FF,FF</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
    CHECK(f.Effects().empty());
}

TEST_CASE("HLS with fewer than four frames does not read past the data", "[impexp2][hls]") {
    for (int n = 1; n <= 4; ++n) {
        auto doc = Parse(HlsDoc(std::to_string(n), "50", HlsChan("L, Normal", "<a>0-01,02,03,04</a>")));
        HLSFile h(doc);
        LayerFixture f;
        CAPTURE(n);
        CHECK(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
    }
}

TEST_CASE("HLS negative frame count must not throw from mapping", "[impexp2][hls]") {
    // std::vector<unsigned char> redData(_frames) with _frames < 0 throws std::length_error
    auto doc = Parse(HlsDoc("-5", "50", HlsChan("L, Normal", "<a>0-FF</a>")));
    HLSFile h(doc);
    LayerFixture f;
    CHECK_NOTHROW(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
}

TEST_CASE("HLS map refuses empty names and null layers", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("4", "50", HlsChan("L, Normal", "<a>0-FF</a>")));
    HLSFile h(doc);
    LayerFixture f;
    CHECK_FALSE(h.MapChannelEffects(f.layer, "", xlWHITE, false));
    CHECK_FALSE(h.MapChannelEffects(nullptr, "L", xlWHITE, false));
    CHECK_FALSE(h.MapChannelEffects(f.layer, "L, Normal", xlWHITE, false));
}

TEST_CASE("HLS duplicate channel names use the last one", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("4", "50", HlsChan("L, Normal", "<a>0-FF,FF,FF,FF</a>") + HlsChan("L, Normal", "<a>0-00,00,00,00</a>")));
    HLSFile h(doc);
    LayerFixture f;
    REQUIRE(h.MapChannelEffects(f.layer, "L", xlWHITE, false));
    CHECK(f.Effects().empty());
}

TEST_CASE("HLS CCR strand names", "[impexp2][hls]") {
    struct C { const char* name; std::vector<std::string> ccr; };
    auto c = GENERATE(values<C>({
        { "Plain", {} },
        { "Tree _0001", { "Tree" } },
        { "Tree_0001", { "Tre" } },
        { "Tree _0002", { "Tree" } },
        { "Tree _abc", {} },
        { "Tree _", { "Tree" } },
        { "_0001", { "" } },
        { "Roof _S001P001", { "Roof _S00" } },
        { "Roof _P001", {} },
        { "Roof _S001", {} },
        { "A_1_2", {} },
    }));
    auto doc = Parse(HlsDoc("4", "50", HlsChan(std::string(c.name) + ", Normal", "")));
    HLSFile h(doc);
    CAPTURE(c.name);
    auto got = h.GetCCRNames();
    if (std::string(c.name) == "_0001") {
        // name.find("_") - 1 underflows: the whole name is used
        CHECK(got.size() <= 1);
    } else {
        CHECK(got == c.ccr);
    }
    for (auto& g : got) CHECK(h.IsCCRName(g));
}

TEST_CASE("HLS CCR names are deduplicated and sorted", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("4", "50", HlsChan("Zed _0001, Normal", "") + HlsChan("Zed _0002, Normal", "") + HlsChan("Alpha _0001, Normal", "") + HlsChan("Tree 10 _0001, Normal", "") + HlsChan("Tree 2 _0001, Normal", "")));
    HLSFile h(doc);
    CHECK(h.GetCCRNames() == std::vector<std::string>{ "Alpha", "Tree 2", "Tree 10", "Zed" });
    CHECK_FALSE(h.IsCCRName("Beta"));
}

TEST_CASE("HLS FindStrandName padding order", "[impexp2][hls]") {
    struct C { const char* chan; const char* expect; };
    auto c = GENERATE(values<C>({ { "SP005", "SP005" }, { "SP0005", "SP0005" }, { "SP05", "SP05" }, { "S_0005", "S_0005" }, { "S_005", "S_005" }, { "nothing", "S_005" } }));
    auto doc = Parse(HlsDoc("4", "50", HlsChan(std::string(c.chan) + ", Normal", "")));
    HLSFile h(doc);
    CAPTURE(c.chan);
    CHECK(h.FindStrandName("S", 5) == c.expect);
}

TEST_CASE("HLS FindStrandName with large and negative node numbers", "[impexp2][hls]") {
    auto doc = Parse(HlsDoc("4", "50", ""));
    HLSFile h(doc);
    CHECK(h.FindStrandName("S", 12345) == "S_12345");
    CHECK(h.FindStrandName("S", 0) == "S_000");
    CHECK(h.FindStrandName("", 1) == "_001");
    CHECK(h.FindStrandName("S", -4).find("-") != std::string::npos);
}

TEST_CASE("HLS strand lookup finds the channels that MapToStrandName derived it from", "[impexp2][hls][!mayfail]") {
    // for names shaped "Name _0001" the derived strand ("Name") plus "_0001" is never an existing channel ("Name _0001"),
    // so FindStrandName cannot resolve the node channels. The same logic exists on desktop; real HLS names are unverified.
    auto doc = Parse(HlsDoc("4", "50", HlsChan("Mega _0001, Normal", "") + HlsChan("Mega _0002, Normal", "")));
    HLSFile h(doc);
    REQUIRE(h.GetCCRNames().size() == 1);
    auto n = h.GetChannelNames();
    auto r = h.FindStrandName(h.GetCCRNames()[0], 1);
    CHECK(std::find(n.begin(), n.end(), r) != n.end());
}

// ---------------------------------------------------------------- ConvertDataRowToEffects

namespace {
xlColor Gray(int v) { return xlColor(v, v, v); }
}

TEST_CASE("DataRow all black produces nothing and appends a sentinel", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector c(20, xlBLACK);
    ConvertDataRowToEffects(f.layer, c, 50, false);
    CHECK(f.layer->GetEffectCount() == 0);
    CHECK(c.size() == 21);
}

TEST_CASE("DataRow runs of one colour", "[impexp2][datarow]") {
    struct C { std::vector<int> v; std::vector<std::pair<int, int>> spans; };
    auto c = GENERATE(values<C>({
        { { 0, 0, 255, 255, 255, 255, 0, 0 }, { { 100, 300 } } },
        { { 255, 255, 255, 255, 255, 255 }, { { 0, 300 } } },
        { { 0, 0, 0, 0, 0, 0, 255, 255 }, { { 300, 400 } } },
        { { 255, 255, 255, 0, 0, 0, 255, 255, 255, 0 }, { { 0, 150 }, { 300, 450 } } },
        { { 255, 0, 0, 0, 0 }, { { 0, 50 } } },
        { { 0, 0, 0, 0, 255 }, { { 200, 250 } } },
        { { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }, { { 0, 600 } } },
    }));
    LayerFixture f;
    xlColorVector colors;
    for (int v : c.v) colors.push_back(Gray(v));
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    auto e = f.Effects();
    REQUIRE(e.size() == c.spans.size());
    for (size_t i = 0; i < e.size(); ++i) {
        CHECK(e[i].name == "On");
        CHECK(e[i].start == c.spans[i].first);
        CHECK(e[i].end == c.spans[i].second);
    }
}

TEST_CASE("DataRow every frame a different colour", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector colors = { xlRED, xlGREEN, xlBLUE, xlRED, xlGREEN, xlBLUE };
    ConvertDataRowToEffects(f.layer, colors, 100, false);
    auto e = f.Effects();
    REQUIRE(e.size() == 6);
    CHECK(e[0].palette == "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1");
    CHECK(e[1].start == 100);
    CHECK(e[5].end == 600);
}

TEST_CASE("DataRow rising black to white ramp", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 10; ++i) colors.push_back(Gray(i * 25));
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    auto e = f.Effects();
    REQUIRE(e.size() >= 1);
    CHECK(e[0].name == "On");
    CHECK(e[0].start == 0);
    CHECK(e[0].settings.find("E_TEXTCTRL_Eff_On_Start=0") != std::string::npos);
}

TEST_CASE("DataRow ramp between two hues is a Color Wash", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 10; ++i) colors.push_back(xlColor(255 - i * 25, i * 25, 0));
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    auto e = f.Effects();
    REQUIRE(e.size() >= 1);
    CHECK(e[0].name == "Color Wash");
    CHECK(e[0].palette.find("C_BUTTON_Palette1=#ff0000") == 0);
}

TEST_CASE("DataRow descending ramp keeps both intensities", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 10; ++i) colors.push_back(Gray(250 - i * 25));
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    auto e = f.Effects();
    REQUIRE(e.size() >= 1);
    CHECK(e[0].name == "On");
    CHECK(e[0].settings.find("E_TEXTCTRL_Eff_On_Start=98") != std::string::npos);
}

TEST_CASE("DataRow short ramps fall back to plain On effects", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector colors = { xlBLACK, Gray(100), Gray(200), xlBLACK, xlBLACK };
    ConvertDataRowToEffects(f.layer, colors, 50, false);
    for (auto& e : f.Effects()) CHECK(e.name == "On");
}

TEST_CASE("DataRow frame time of zero and negative values", "[impexp2][datarow]") {
    for (int ft : { 0, -50, 1, 1000000 }) {
        LayerFixture f;
        xlColorVector colors = { xlBLACK, xlWHITE, xlWHITE, xlBLACK, xlRED, xlBLACK };
        ConvertDataRowToEffects(f.layer, colors, ft, false);
        CAPTURE(ft);
        SUCCEED();
    }
}

TEST_CASE("DataRow with empty and tiny inputs", "[impexp2][datarow]") {
    for (size_t n = 0; n < 6; ++n) {
        LayerFixture f;
        xlColorVector colors(n, xlWHITE);
        ConvertDataRowToEffects(f.layer, colors, 50, true);
        CAPTURE(n);
        if (n > 0) {
            auto e = f.Effects();
            REQUIRE(e.size() == 1);
            CHECK(e[0].end == (int)n * 50);
        } else {
            CHECK(f.layer->GetEffectCount() == 0);
        }
    }
}

TEST_CASE("DataRow honours erase and existing effect protection", "[impexp2][datarow]") {
    LayerFixture f;
    f.layer->AddEffect(0, "Off", "", "", 100, 200, false, false);
    xlColorVector a(6, xlWHITE);
    ConvertDataRowToEffects(f.layer, a, 50, false);
    CHECK(f.layer->GetEffectCount() == 1);
    xlColorVector b(6, xlWHITE);
    ConvertDataRowToEffects(f.layer, b, 50, true);
    CHECK(f.layer->GetEffectCount() == 1);
    CHECK(f.Effects()[0].name == "On");
}

TEST_CASE("DataRow handles a very long row", "[impexp2][datarow]") {
    LayerFixture f;
    xlColorVector colors;
    for (int i = 0; i < 30000; ++i) colors.push_back((i / 7) % 2 ? xlWHITE : xlBLACK);
    ConvertDataRowToEffects(f.layer, colors, 25, false);
    CHECK(f.layer->GetEffectCount() == 30000 / 14 + (30000 % 14 >= 7 ? 1 : 0));
}

// ---------------------------------------------------------------- Base64Decode

TEST_CASE("Base64Decode round trips every length", "[impexp2][datarow]") {
    for (size_t n = 0; n < 70; ++n) {
        std::vector<uint8_t> in(n);
        for (size_t i = 0; i < n; ++i) in[i] = (uint8_t)(i * 37 + n);
        std::vector<unsigned char> out;
        Base64Decode(B64(in), out);
        CAPTURE(n);
        CHECK(out == std::vector<unsigned char>(in.begin(), in.end()));
    }
}

TEST_CASE("Base64Decode stops at the first non base64 character", "[impexp2][datarow]") {
    struct C { const char* s; size_t n; };
    auto c = GENERATE(values<C>({ { "QUJD", 3 }, { "QUJD\nREVG", 3 }, { "QUJD REVG", 3 }, { "QUJD-REVG", 3 }, { "QUJD_REVG", 3 }, { "QUJDREVG", 6 }, { "=", 0 }, { "====", 0 }, { "", 0 }, { "QQ==", 1 },
                                  { "QUI=", 2 }, { "QUJDRA==", 4 }, { "QQ", 0 }, { "QUJDQQ", 3 }, { "\xff\xfe", 0 }, { "QU\0JD", 0 } }));
    std::vector<unsigned char> out;
    Base64Decode(std::string(c.s), out);
    CAPTURE(c.s);
    CHECK(out.size() == c.n);
}

TEST_CASE("Base64Decode appends to existing data", "[impexp2][datarow]") {
    std::vector<unsigned char> out = { 9, 9 };
    Base64Decode("QUJD", out);
    CHECK(out == std::vector<unsigned char>{ 9, 9, 'A', 'B', 'C' });
}

TEST_CASE("Base64Decode large input", "[impexp2][datarow]") {
    std::vector<uint8_t> in(3 * 1000 * 1000 + 1);
    for (size_t i = 0; i < in.size(); ++i) in[i] = (uint8_t)(i * 31 + (i >> 8));
    std::vector<unsigned char> out;
    Base64Decode(B64(in), out);
    CHECK(out == std::vector<unsigned char>(in.begin(), in.end()));
}

// ---------------------------------------------------------------- MapHints

TEST_CASE("Map hints special characters round trip", "[impexp2][maphints]") {
    TempDir d;
    std::vector<MapHintEntry> in = { { "a>b", "x>y", "B" }, { "it's", "back\\slash (x|y)+$", "M" }, { "^Star(s?)(\\sGroup)?$", "Star", "G" } };
    REQUIRE(WriteMapHintsFile(d.File("s.xmaphint"), in));
    auto out = LoadMapHintsFile(d.File("s.xmaphint"));
    REQUIRE(out.size() == in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        CAPTURE(i);
        CHECK(out[i].toRegex == in[i].toRegex);
        CHECK(out[i].fromModel == in[i].fromModel);
    }
}

TEST_CASE("Map hints writer must escape xml special characters", "[impexp2][maphints]") {
    // WriteMapHintsFile writes attribute values raw, so a literal &amp; is read back as & (and a \" makes the file unparseable)
    TempDir d;
    REQUIRE(WriteMapHintsFile(d.File("s.xmaphint"), { { "a&amp;b", "x&lt;y", "B" } }));
    auto out = LoadMapHintsFile(d.File("s.xmaphint"));
    REQUIRE(out.size() == 1);
    CHECK(out[0].toRegex == "a&amp;b");
    CHECK(out[0].fromModel == "x&lt;y");
}

TEST_CASE("Map hints writer with a quote must not lose the other entries", "[impexp2][maphints]") {
    TempDir d;
    REQUIRE(WriteMapHintsFile(d.File("q.xmaphint"), { { "ok", "ok", "B" }, { "has\"quote", "x", "B" }, { "after", "after", "B" } }));
    CHECK(LoadMapHintsFile(d.File("q.xmaphint")).size() == 3);
}

TEST_CASE("Map hints unicode text round trips", "[impexp2][maphints]") {
    TempDir d;
    std::string uni = "\xC3\xA9toile \xE2\x98\x83 \xF0\x9F\x8E\x84";
    REQUIRE(WriteMapHintsFile(d.File("u.xmaphint"), { { uni, uni, "B" } }));
    auto out = LoadMapHintsFile(d.File("u.xmaphint"));
    REQUIRE(out.size() == 1);
    CHECK(out[0].toRegex == uni);
}

TEST_CASE("Map hints writer output is wrapped and carries the samples comment", "[impexp2][maphints]") {
    TempDir d;
    REQUIRE(WriteMapHintsFile(d.File("w.xmaphint"), {}));
    std::ifstream in(d.File("w.xmaphint"), std::ios::binary);
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(s.find("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") == 0);
    CHECK(s.find("<MapHints>") != std::string::npos);
    CHECK(s.find("<!-- Samples") != std::string::npos);
    CHECK(s.rfind("</MapHints>") == s.size() - 11);
    CHECK(LoadMapHintsFile(d.File("w.xmaphint")).empty());
}

TEST_CASE("Map hints apply to defaults and odd values", "[impexp2][maphints]") {
    TempDir d;
    auto f = d.Write("a.xmaphint", "<MapHints><Map ToRegex=\"a\" FromModel=\"A\"/><Map ToRegex=\"b\" FromModel=\"B\" ApplyTo=\"\"/><Map ToRegex=\"c\" FromModel=\"C\" ApplyTo=\"zzz\"/><Map ToRegex=\"d\" FromModel=\"D\" ApplyTo=\"m\"/></MapHints>");
    auto out = LoadMapHintsFile(f);
    REQUIRE(out.size() == 4);
    CHECK(out[0].applyTo == "B");
    CHECK(out[1].applyTo == "");
    CHECK(out[2].applyTo == "zzz");
    CHECK(out[3].applyTo == "m");
}

TEST_CASE("Map hints loader only looks at direct children of the root", "[impexp2][maphints]") {
    TempDir d;
    auto f = d.Write("n.xmaphint", "<MapHints><Group><Map ToRegex=\"deep\" FromModel=\"D\"/></Group><Map ToRegex=\"top\" FromModel=\"T\"><Map ToRegex=\"child\" FromModel=\"C\"/></Map></MapHints>");
    auto out = LoadMapHintsFile(f);
    REQUIRE(out.size() == 1);
    CHECK(out[0].toRegex == "top");
}

TEST_CASE("Map hints loader handles BOM, comments and odd roots", "[impexp2][maphints]") {
    TempDir d;
    CHECK(LoadMapHintsFile(d.Write("b.xmaphint", "\xEF\xBB\xBF<MapHints><Map ToRegex=\"a\" FromModel=\"A\"/></MapHints>")).size() == 1);
    CHECK(LoadMapHintsFile(d.Write("c.xmaphint", "<!-- c --><Anything><!-- x --><Map ToRegex=\"a\" FromModel=\"A\"/><!-- y --></Anything>")).size() == 1);
    CHECK(LoadMapHintsFile(d.Write("e.xmaphint", "<MapHints/>")).empty());
    CHECK(LoadMapHintsFile(d.Write("f.xmaphint", "<Map ToRegex=\"a\" FromModel=\"A\"/>")).empty());
    CHECK(LoadMapHintsFile(d.Write("g.xmaphint", "<MapHints><Map ToRegex=\"a\" FromModel=\"A\"")).size() <= 1);
}

TEST_CASE("Map hints keep duplicates and order", "[impexp2][maphints]") {
    TempDir d;
    std::vector<MapHintEntry> in;
    for (int i = 0; i < 2000; ++i) in.push_back({ "r" + std::to_string(i % 10), "m" + std::to_string(i), i % 2 ? "M" : "G" });
    REQUIRE(WriteMapHintsFile(d.File("big.xmaphint"), in));
    auto out = LoadMapHintsFile(d.File("big.xmaphint"));
    REQUIRE(out.size() == in.size());
    CHECK(out[1999].fromModel == "m1999");
    CHECK(out[0].toRegex == "r0");
}

TEST_CASE("Map hints very long values", "[impexp2][maphints]") {
    TempDir d;
    std::string big(200000, 'x');
    REQUIRE(WriteMapHintsFile(d.File("l.xmaphint"), { { big, big, "B" } }));
    auto out = LoadMapHintsFile(d.File("l.xmaphint"));
    REQUIRE(out.size() == 1);
    CHECK(out[0].toRegex.size() == big.size());
}

TEST_CASE("Map hints show dir scan ignores look alikes", "[impexp2][maphints]") {
    TempDir d;
    std::filesystem::create_directories(d.path / "maphints" / "sub.xmaphint");
    std::filesystem::create_directories(d.path / "maphints" / "nested");
    REQUIRE(WriteMapHintsFile((d.path / "maphints" / "good.xmaphint").string(), { { "a", "A", "B" } }));
    REQUIRE(WriteMapHintsFile((d.path / "maphints" / "nested" / "deep.xmaphint").string(), { { "b", "B", "B" } }));
    d.Write("maphints/UPPER.XMAPHINT", "<MapHints><Map ToRegex=\"u\" FromModel=\"U\"/></MapHints>");
    d.Write("maphints/noext", "<MapHints><Map ToRegex=\"u\" FromModel=\"U\"/></MapHints>");
    d.Write("maphints/bad.xmaphint", "garbage");
    auto out = LoadMapHintsFromShowDir(d.path.string());
    CHECK(out.size() == 1);
}

TEST_CASE("Map hints show dir scan with missing or file-in-place folders", "[impexp2][maphints]") {
    TempDir d;
    CHECK(LoadMapHintsFromShowDir((d.path / "nope").string()).empty());
    d.Write("maphints", "i am a file");
    CHECK(LoadMapHintsFromShowDir(d.path.string()).empty());
}

TEST_CASE("Map hints writer to a directory path fails", "[impexp2][maphints]") {
    TempDir d;
    CHECK_FALSE(WriteMapHintsFile(d.path.string(), { { "a", "b", "B" } }));
    CHECK_FALSE(WriteMapHintsFile("", {}));
}

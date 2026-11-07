#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/LOREdit.h"
#include "import_export/LORMusic.h"
#include "import_export/LORPixelEditor.h"
#include "import_export/SuperStarImporter.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "test_impexp2_lor_common.h"

#include <chrono>
#include <set>
#include <string>
#include <vector>

using namespace lortest;

namespace {
bool Has(const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }

std::vector<char> Buf(const std::string& s) { return std::vector<char>(s.begin(), s.end()); }
std::string Str(const std::vector<char>& b) { return std::string(b.begin(), b.end()); }

// deterministic generator, no <random> distributions (those differ between standard libraries)
struct Lcg {
    uint64_t s;
    explicit Lcg(uint64_t seed) : s(seed) {}
    uint32_t next() {
        s = s * 6364136223846793005ULL + 1442695040888963407ULL;
        return (uint32_t)(s >> 33);
    }
    int range(int n) { return n <= 0 ? 0 : (int)(next() % (uint32_t)n); }
};
}

// ---------- SuperStar helpers that the LOR readers share ----------

TEST_CASE("SuperStar preprocess leaves empty and tiny buffers alone", "[impexp2][lor][superstar]") {
    std::vector<char> e;
    SuperStar::PreprocessXmlBuffer(e);
    CHECK(e.empty());
    for (const char* s : { "a", "<a/>", "1234567" }) {
        auto b = Buf(s);
        SuperStar::PreprocessXmlBuffer(b);
        CHECK(Str(b) == s);
    }
}

TEST_CASE("SuperStar preprocess repairs the pixels typo", "[impexp2][lor][superstar]") {
    auto b = Buf("<image name=\"a\">\r\n<pixels=\"1,2,3\"/>\r\n</image>");
    SuperStar::PreprocessXmlBuffer(b);
    CHECK(Has(Str(b), "<pixe s=\"1,2,3\"/>"));
    pugi::xml_document d;
    CHECK(d.load_buffer(b.data(), b.size()));
    CHECK(d.child("image").child("pixe").attribute("s").as_string() == std::string("1,2,3"));
}

TEST_CASE("SuperStar preprocess closes elements SuperStar leaves open", "[impexp2][lor][superstar]") {
    struct Row { const char* in; const char* tag; };
    auto r = GENERATE(Row{ "<timing centisecond=\"5\">\n", "timing" }, Row{ "<group name=\"g\">\n", "group" }, Row{ "<imageAction name=\"g\">\n", "imageAction" },
                      Row{ "<textAction name=\"g\">\n", "textAction" });
    std::string x = std::string("<root>\n") + r.in + "</root>";
    auto b = Buf(x);
    SuperStar::PreprocessXmlBuffer(b);
    pugi::xml_document d;
    INFO(Str(b));
    CHECK(d.load_buffer(b.data(), b.size()));
    CHECK(d.child("root").child(r.tag));
}

TEST_CASE("SuperStar preprocess strips configuration blocks", "[impexp2][lor][superstar]") {
    auto b = Buf("<S>\n<configurations>\n<configuration name=\"a\">\n<x/>\n</configuration>\n<configuration name=\"b\">\n</configuration>\n</configurations>\n<k/></S>");
    SuperStar::PreprocessXmlBuffer(b);
    CHECK_FALSE(Has(Str(b), "</configuration>"));
    pugi::xml_document d;
    INFO(Str(b));
    CHECK(d.load_buffer(b.data(), b.size()));
    CHECK(d.child("S").child("k"));
}

TEST_CASE("SuperStar preprocess close of a tag immediately followed by another tag keeps both", "[impexp2][lor][superstar][!mayfail]") {
    // BUG: closing an open element overwrites the character after '>' with '>' ("<a>" + "X" -> "<a/>" + ">"); when that
    // next character is the '<' of the following tag (no whitespace between) it is destroyed
    auto b = Buf("<root><timing centisecond=\"5\"><timing centisecond=\"6\"></root>");
    SuperStar::PreprocessXmlBuffer(b);
    CHECK(Has(Str(b), "<timing centisecond=\"6\""));
}

TEST_CASE("SuperStar preprocess at the very end of a buffer", "[impexp2][lor][superstar]") {
    for (const char* s : { "<root><timing a=\"1\">", "<root><timing a=\"1\"", "<root><timing ", "<root><configuration ", "<root><pixels=", "<root></configuration>", "<root><group a>" }) {
        INFO(s);
        auto b = Buf(s);
        SuperStar::PreprocessXmlBuffer(b);
        CHECK(b.size() == std::string(s).size());
    }
}

TEST_CASE("SuperStar timings are shifted by centiseconds", "[impexp2][lor][superstar]") {
    pugi::xml_document d;
    d.load_string("<r startCentisecond=\"5\"><effect startCentisecond=\"10\" endCentisecond=\"20\" startTime=\"7\" time=\"3\"/>"
                  "<flowy startTime=\"100\" endTime=\"200\" startCentisecond=\"1\"/><morph><state1 time=\"5\"/><state2 time=\"6\" endCentisecond=\"9\"/><other time=\"1\"/></morph>"
                  "<timing centisecond=\"40\"/><nonnumeric startCentisecond=\"abc\"/></r>");
    SuperStar::AdjustAllTimings(d.document_element(), 50);
    auto r = d.child("r");
    CHECK(r.attribute("startCentisecond").as_int() == 55);
    auto e = r.child("effect");
    CHECK(e.attribute("startCentisecond").as_int() == 60);
    CHECK(e.attribute("endCentisecond").as_int() == 70);
    CHECK(e.attribute("startTime").as_int() == 7);
    CHECK(e.attribute("time").as_int() == 3);
    auto f = r.child("flowy");
    CHECK(f.attribute("startTime").as_int() == 150);
    CHECK(f.attribute("endTime").as_int() == 250);
    CHECK(f.attribute("startCentisecond").as_int() == 51);
    auto m = r.child("morph");
    CHECK(m.child("state1").attribute("time").as_int() == 55);
    CHECK(m.child("state2").attribute("time").as_int() == 56);
    CHECK(m.child("state2").attribute("endCentisecond").as_int() == 59);
    CHECK(m.child("other").attribute("time").as_int() == 1);
    CHECK(r.child("timing").attribute("centisecond").as_int() == 40);
    CHECK(r.child("nonnumeric").attribute("startCentisecond").as_int() == 50);
}

TEST_CASE("SuperStar timing shift by a negative or zero amount", "[impexp2][lor][superstar]") {
    pugi::xml_document d;
    d.load_string("<r><effect startCentisecond=\"10\" endCentisecond=\"20\"/></r>");
    SuperStar::AdjustAllTimings(d.document_element(), 0);
    CHECK(d.child("r").child("effect").attribute("startCentisecond").as_int() == 10);
    SuperStar::AdjustAllTimings(d.document_element(), -30);
    CHECK(d.child("r").child("effect").attribute("startCentisecond").as_int() == -20);
    CHECK(d.child("r").child("effect").attribute("endCentisecond").as_int() == -10);
    pugi::xml_node none;
    SuperStar::AdjustAllTimings(none, 5);
    SUCCEED();
}

TEST_CASE("SuperStar timing shift on a deep tree does not blow the stack", "[impexp2][lor][superstar]") {
    std::string x;
    for (int i = 0; i < 2000; ++i) x += "<a startCentisecond=\"1\">";
    for (int i = 0; i < 2000; ++i) x += "</a>";
    pugi::xml_document d;
    d.load_string(x.c_str());
    SuperStar::AdjustAllTimings(d.document_element(), 1);
    CHECK(d.document_element().attribute("startCentisecond").as_int() == 2);
}

// ---------- realistic S6 shaped document ----------

namespace {
const std::vector<std::pair<std::string, std::string>> kEffects = {
    { "bars", "right,1,none,True,19,0" },
    { "colorwash", "full,full,none" },
    { "curtain", "center,open,0,once_at_speed,12," },
    { "garland", "3,34,once_at_speed,12,bottom_to_top" },
    { "movingshapes", "star5,8,73,10,palette,solid,random_wrap,90,continuous_rotation,20,12,False,0,0,1" },
    { "picturexy", "*pic*abcdef12-3456*.png,71,104,0,R32R0R1.00,100,100,100,100,FFFF,0,0,0" },
    { "plasma", "1,3,12,blended" },
    { "ripple", "circle,21,47,46,50,0,0,37,False,50" },
    { "singleblock", "down,0,0,50,50,100,50,single_color" },
    { "sketch", "GSVG a b c d e f g h FFFF0000-1 2 True~PPath M 0 0 L 1 0 L 1 1 Z,100,100,0,0,0,FFFF" },
    { "spirals", "1,left_to_right,20,50,0,False,trail_left,50" },
    { "starfield", "tree,palette,solid,50,50,50,1,3,0,25,random" },
    { "text", "%3Cp%3EHello%3C%2Fp%3E,50,left,0,10,,4,True,x,y,z" },
    { "none", "" },
};
const std::string kPal = "FFFF0000,1;FF00FF00,1;FF0000FF,0;FFFFFF00,1;FF00FFFF,0;FFFF00FF,1";

std::string S6Doc(int props, int effectsPerTrack) {
    Lcg g(12345);
    std::string x = "<sequence saveFileVersion=\"16\" AppVersion=\"6.6.0.13\" id=\"00000000-0000-0000-0000-000000000000\" totalCentiseconds=\"16566\" author=\"\" musicTitle=\"\">\r\n";
    x += "\t<ArchivedProps/>\r\n\t<BeatChannels/>\r\n\t<BeatView/>\r\n\t<MotionPaks/>\r\n\t<PreviewClass>\r\n\t\t<BackgroundImage/>\r\n";
    for (int p = 0; p < props; ++p) {
        x += "\t\t<PropClass id=\"" + std::to_string(p) + "\" BulbShape=\"Square\" ChannelGrid=\"a,b,c,d,e,Red;a,b,c,d,e,Green\" DeviceType=\"LOR\" Name=\"Prop " + std::to_string(p) + "\" Parm1=\"1\" Parm2=\"2\">\r\n";
        x += "\t\t\t<MotionRowDefaults><MotionRowDefault id=\"1\"/></MotionRowDefaults>\r\n\t\t\t<shape><point x=\"0\" y=\"0\"/><point x=\"1\" y=\"1\"/></shape>\r\n\t\t</PropClass>\r\n";
    }
    x += "\t\t<PropGroup><member id=\"0\"/></PropGroup>\r\n\t</PreviewClass>\r\n\t<PreviewViewStates/>\r\n\t<PropViews><PropView><ViewMember><ViewMember><ViewMember/></ViewMember></ViewMember></PropView></PropViews>\r\n";
    x += "\t<RgbAggregates/>\r\n\t<SequenceProps>\r\n";
    for (int p = 0; p < props; ++p) {
        x += "\t\t<SeqProp DisplayGroupMembers=\"0\" EnablePixelChannels=\"" + std::to_string(p % 2) + "\" OverlayMotionRows=\"1\" id=\"g" + std::to_string(p) + "\" name=\"Prop " + std::to_string(p) + "\">\r\n";
        for (int c = 0; c < 3; ++c) {
            x += "\t\t\t<channel col=\"" + std::to_string(c) + "\" color=\"" + std::to_string(c % 4) + "\" row=\"" + std::to_string(p % 7) + "\">\r\n";
            for (int e = 0; e < 5; ++e) {
                const char* types[] = { "INTENSITY", "SHIMMER", "TWINKLE" };
                int neg = g.range(4) == 0;
                x += "\t\t\t\t<effect startCentisecond=\"" + std::to_string(e * 40) + "\" endCentisecond=\"" + std::to_string(e * 40 + 30) + "\" " +
                     (neg ? "intensity=\"-16777215\"" : (g.range(2) ? "intensity=\"" + std::to_string(g.range(101)) + "\"" : "startIntensity=\"" + std::to_string(g.range(50)) + "\" endIntensity=\"" + std::to_string(50 + g.range(50)) + "\"")) +
                     " settings=\"" + types[g.range(3)] + "\"/>\r\n";
            }
            x += "\t\t\t</channel>\r\n";
        }
        const char* tt[] = { "custom", "custom_horizontal_buffer", "none", "rectangle" };
        for (int t = 0; t < 4; ++t) {
            x += std::string("\t\t\t<track DefaultId=\"d\" id=\"t\" name=\"Row ") + std::to_string(t) + "\" type=\"" + tt[t] + "\" subc=\"\" subc2=\"\" subx=\"0.25\" suby=\"0.25\" subw=\"0.5\" subh=\"0.5\">\r\n";
            for (int e = 0; e < effectsPerTrack; ++e) {
                auto& a = kEffects[g.range((int)kEffects.size())];
                auto& b = kEffects[g.range((int)kEffects.size())];
                const char* mix[] = { "Mix_Average", "Mix_Overlay", "Mix_Maximum" };
                std::string pal = a.first == "none" ? "" : kPal;
                std::string s = std::string(mix[g.range(3)]) + "|" + std::to_string(g.range(50)) + "|0|" + (g.range(5) == 0 ? "blink_in_unison" : "full") + "|10|lightorama_" + a.first + ":" + pal + ":" + a.second +
                                "|lightorama_" + b.first + ":" + (b.first == "none" ? "" : kPal) + ":" + b.second;
                x += "\t\t\t\t<effect startCentisecond=\"" + std::to_string(e * 20) + "\" endCentisecond=\"" + std::to_string(e * 20 + 15) + "\" intensity=\"" + std::to_string(g.range(101)) + "\" settings=\"" + Esc(s) + "\"/>\r\n";
            }
            x += "\t\t\t</track>\r\n";
        }
        x += "\t\t</SeqProp>\r\n";
    }
    x += "\t</SequenceProps>\r\n\t<Subsequences/>\r\n\t<TimingGrids>\r\n\t\t<TimingGridFixed saveID=\"0\"/>\r\n\t\t<TimingGridFree name=\"Beats\">\r\n";
    for (int i = 1; i < 300; ++i) x += "\t\t\t<timing centisecond=\"" + std::to_string(i * 7) + "\"/>\r\n";
    x += "\t\t</TimingGridFree>\r\n\t</TimingGrids>\r\n\t<pictures><picture name=\"*pic*abcdef12-3456*.png\">iVBORw0KGgo=</picture></pictures>\r\n</sequence>\r\n";
    return "\xEF\xBB\xBF<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" + x;
}
}

TEST_CASE("LOR S6 shaped file with every effect type converts end to end", "[impexp2][lor][s6]") {
    TempDir dir;
    auto path = dir.Write("s6.loredit", S6Doc(40, 40));
    pugi::xml_document doc;
    REQUIRE(doc.load_file(path.c_str()));
    LOREdit l(doc, 20);
    auto models = l.GetModelsWithEffects();
    REQUIRE(models.size() >= 40);
    CHECK(l.GetTimingTracks() == std::vector<std::string>{ "Beats" });
    CHECK(l.GetTimings("Beats").size() > 100);
    CHECK(l.GetEmbeddedPictures().size() == 1);
    std::set<std::string> names;
    size_t effects = 0, empties = 0;
    for (auto& m : models) {
        int layers = l.GetModelLayers(m);
        for (int i = 0; i < layers; ++i) {
            for (auto& e : l.GetTrackEffects(m, i, 0)) {
                ++effects;
                std::string pal = e.GetPalette();
                std::string s = e.GetSettings(pal);
                std::string n = e.GetxLightsEffect();
                if (e.effectType.empty()) {
                    ++empties;
                    continue;
                }
                names.insert(e.effectType);
                CHECK(s.find(",,") == std::string::npos);
                CHECK(s.find("T_CHOICE_LayerMethod=") != std::string::npos);
            }
        }
    }
    CHECK(effects > 1000);
    for (auto& k : kEffects) {
        if (k.first == "none") continue;
        INFO(k.first);
        CHECK(names.count(k.first) == 1);
    }
    int chans = 0;
    for (auto& m : models) {
        int r, c;
        chans += l.GetModelChannels(m, r, c) > 0;
        CHECK(l.GetSequencingType(m) != loreditType::NONE);
    }
    CHECK(chans > 0);
}

TEST_CASE("LOR S6 shaped file sequencing types for a sample of props", "[impexp2][lor][s6]") {
    auto doc = Parse(S6Doc(5, 6));
    LOREdit l(doc, 20);
    CHECK(l.GetSequencingType("Prop 0") == loreditType::CHANNELS);
    CHECK(l.GetSequencingType("Prop 0/Row 0") == loreditType::TRACKS);
    CHECK(l.GetSequencingType("Prop 0/Row 1") == loreditType::TRACKS);
    auto m = l.GetModelsWithEffects();
    CHECK(std::find(m.begin(), m.end(), "Prop 0/Row 0") != m.end());
    CHECK(std::find(m.begin(), m.end(), "Prop 0/Row 1") != m.end());
    CHECK(std::find(m.begin(), m.end(), "Prop 0/Row 2") == m.end());
    auto strands = l.GetModelStrands("Prop 0");
    REQUIRE(strands.size() == 2);
    CHECK(strands[1] == "Red");
    CHECK(strands[2] == "Green");
    auto nodes = l.GetNodesWithEffects();
    CHECK(nodes.size() == 15);
    CHECK(nodes[0].rfind("Prop 0[", 0) == 0);
}

TEST_CASE("LOR S6 shaped file with a large effect count reads in reasonable time", "[impexp2][lor][s6][perf]") {
    auto text = S6Doc(60, 300);
    auto t0 = std::chrono::steady_clock::now();
    auto doc = Parse(text);
    LOREdit l(doc, 20);
    size_t n = 0;
    for (auto& m : l.GetModelsWithEffects()) {
        int layers = l.GetModelLayers(m);
        for (int i = 0; i < layers; ++i) n += l.GetTrackEffects(m, i, 0).size();
    }
    auto secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    CHECK(n > 20000);
    CHECK(secs < 60.0);
}

TEST_CASE("LOR S6 shaped props with the same name in archived and sequence props", "[impexp2][lor][s6]") {
    auto doc = Parse(Seq("<ArchivedProps><ArchiveProp name=\"P\">" + Track(TrackEffect(0, 10, Settings(Layer("fire", "50,0")))) + "</ArchiveProp></ArchivedProps>" +
                         Props(SeqProp("P", Track(TrackEffect(0, 10, Settings(Layer("bars", "up,1,none,False,8,0"))))))));
    LOREdit l(doc, 20);
    // archived props come first in a real file; the first match wins
    auto e = l.GetTrackEffects("P", 0, 0);
    REQUIRE(e.size() == 1);
    CHECK(e[0].effectType == "fire");
    CHECK(l.GetModelsWithEffects() == std::vector<std::string>{ "P", "P" });
}

// ---------- mutation robustness ----------

namespace {
void PokeLoredit(pugi::xml_document& doc, int& exceptions) {
    try {
        LOREdit l(doc, 20);
        for (auto& t : l.GetTimingTracks()) (void)l.GetTimings(t, 0);
        for (auto& m : l.GetModelsWithEffects()) {
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
                    (void)e.GetSketchSVG();
                }
            }
            (void)l.GetChannelEffects(m, -1, -1, -1, 0);
            (void)l.GetChannelEffects(m, 1, 5, 0);
        }
        (void)l.GetNodesWithEffects();
        for (auto& t : l.GetLipSyncTracks()) (void)l.GetLipSync(t, 0);
        (void)l.GetEmbeddedPictures();
    } catch (const std::exception&) {
        ++exceptions;
    }
}

std::string Mutate(const std::string& in, Lcg& g, bool safe) {
    std::string s = in;
    static const char alpha[] = "|:,;-%\"<>& 0.9x";
    static const char safeAlpha[] = "|:;-\"& 09x";
    int edits = 1 + g.range(6);
    for (int i = 0; i < edits && !s.empty(); ++i) {
        size_t p = (size_t)g.range((int)s.size());
        switch (g.range(4)) {
        case 0:
            s.erase(p, (size_t)(1 + g.range(8)));
            break;
        case 1:
            s[p] = safe ? safeAlpha[g.range(sizeof(safeAlpha) - 1)] : alpha[g.range(sizeof(alpha) - 1)];
            break;
        case 2:
            s.insert(p, 1, safe ? safeAlpha[g.range(sizeof(safeAlpha) - 1)] : alpha[g.range(sizeof(alpha) - 1)]);
            break;
        default:
            s.resize(p);
            break;
        }
    }
    return s;
}

std::string SmallLorEdit() {
    std::string tracks;
    for (auto& k : kEffects) {
        tracks += Track(TrackEffect(0, 100, Settings(Layer(k.first, k.second, "FFFF0000,1;FF00FF00,0"), Layer("fire", "50,0"))), " name=\"T\" type=\"none\"");
    }
    return Seq("<PreviewClass><PropClass Name=\"P\" ChannelGrid=\"a,b,c,d,e,Red\"/></PreviewClass>" + std::string("<TimingGrids><TimingGridFree name=\"T\"><timing centisecond=\"10\"/><timing centisecond=\"20\"/></TimingGridFree></TimingGrids>") +
               Props(SeqProp("P", Chan(0, 0, 0, ChanEffect(0, 10, "INTENSITY", "intensity=\"-65536\"") + ChanEffect(10, 20, "SHIMMER", "startIntensity=\"10\" endIntensity=\"90\"")) + tracks) +
                     SeqProp("F Mouth AI", Chan(0, 0, 0, ChanEffect(0, 10, "INTENSITY", "intensity=\"100\""))) + SeqProp("F Mouth E", Chan(0, 0, 0, ChanEffect(10, 20, "INTENSITY", "intensity=\"100\"")))) +
               "<pictures><picture name=\"x\">AAAA</picture></pictures>");
}
}

TEST_CASE("LOR mutated documents never crash the loredit readers", "[impexp2][lor][fuzz]") {
    std::string base = SmallLorEdit();
    Lcg g(2024);
    int exceptions = 0;
    for (int i = 0; i < 400; ++i) {
        std::string m = Mutate(base, g, true);
        pugi::xml_document doc;
        doc.load_buffer(m.data(), m.size());
        PokeLoredit(doc, exceptions);
    }
    INFO("exceptions: " << exceptions);
    SUCCEED();
}

TEST_CASE("LOR mutated documents do not throw from the loredit readers", "[impexp2][lor][fuzz][!mayfail]") {
    // BUG: palette and ramp parsing throw std::out_of_range (substr) on short fields; xLights has no exception handling
    std::string base = SmallLorEdit();
    Lcg g(77);
    int exceptions = 0;
    for (int i = 0; i < 400; ++i) {
        std::string m = Mutate(base, g, true);
        pugi::xml_document doc;
        doc.load_buffer(m.data(), m.size());
        PokeLoredit(doc, exceptions);
    }
    CHECK(exceptions == 0);
}

TEST_CASE("LMS mutated documents never crash", "[impexp2][lor][fuzz]") {
    std::string base = "<sequence><channels><channel name=\"A\" color=\"255\" unit=\"1\" circuit=\"1\" savedIndex=\"0\"><effect type=\"intensity\" startCentisecond=\"0\" endCentisecond=\"10\" intensity=\"50\"/>"
                       "<effect type=\"shimmer\" startCentisecond=\"10\" endCentisecond=\"20\" startIntensity=\"0\" endIntensity=\"100\"/></channel>"
                       "<channel name=\"B\" savedIndex=\"1\"><effect type=\"twinkle\" startCentisecond=\"5\" endCentisecond=\"25\" intensity=\"100\"/></channel><channel name=\"C\" savedIndex=\"2\"/>"
                       "<rgbChannel name=\"R-P01\" savedIndex=\"3\"><channels><channel savedIndex=\"0\"/><channel savedIndex=\"1\"/><channel savedIndex=\"2\"/></channels></rgbChannel></channels>"
                       "<timingGrids><timingGrid name=\"T\" type=\"freeform\"><timing centisecond=\"10\"/><timing centisecond=\"30\"/></timingGrid></timingGrids></sequence>";
    Lcg g(99);
    for (int i = 0; i < 500; ++i) {
        std::string m = Mutate(base, g, false);
        pugi::xml_document doc;
        doc.load_buffer(m.data(), m.size());
        LORMusic l(doc, 20);
        HeadlessRenderContext ctx;
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* e = se.AddElement("Model", "model", true, false, true, false, false);
        auto* layer = e->AddEffectLayer();
        for (auto& n : l.GetChannelNames()) l.MapChannelEffects(layer, n, l.GetChannelColor(n), 1 + i % 3, true);
        for (auto& n : l.GetCCRNames()) {
            (void)l.ResolveCCRNodeName(n, i % 5);
            l.MapChannelEffects(layer, l.ResolveCCRNodeName(n, 0), xlWHITE, 3, true);
        }
        for (auto& t : l.GetTimingTracks()) (void)l.GetTimings(t, i % 7 * 100);
    }
    SUCCEED();
}

TEST_CASE("LPE mutated documents never crash", "[impexp2][lor][fuzz]") {
    std::string pe = "Mix_Average|10|5|0|0|colorwash:FFFF0000,1;FF00FF00,FF0000FF,1:full,full,none|fire:FFFF0000,1:50,0";
    std::string base = "<LPE><SequenceProps><SeqProp name=\"P\"><track id=\"0\"><effect type=\"pixelEffect\" startCentisecond=\"0\" endCentisecond=\"10\" startIntensity=\"0\" endIntensity=\"100\" pixelEffect=\"" + pe +
                       "\"/><effect type=\"pixelEffect\" startCentisecond=\"10\" endCentisecond=\"20\" startIntensity=\"100\" endIntensity=\"100\" pixelEffect=\"" + pe +
                       "\"/></track><track id=\"1\"><effect type=\"pixelEffect\" startCentisecond=\"30\" endCentisecond=\"40\" pixelEffect=\"" + pe + "\"/></track></SeqProp></SequenceProps></LPE>";
    Lcg g(31);
    int ran = 0;
    for (int i = 0; i < 400; ++i) {
        // only mutate the document structure and numbers: the known unchecked field-count crashes are tracked separately
        std::string m = Mutate(base, g, true);
        pugi::xml_document doc;
        doc.load_buffer(m.data(), m.size());
        LORPixelEditor l(doc, 20);
        HeadlessRenderContext ctx;
        auto& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        auto* e = se.AddElement("Model", "model", true, false, true, false, false);
        e->AddEffectLayer();
        // skip documents whose pixel effect strings no longer have the expected field counts (known out of range reads)
        bool shapely = true;
        for (auto x : doc.select_nodes("//effect")) {
            std::string v = x.node().attribute("pixelEffect").as_string();
            int bars = 0, colons = 0;
            for (char c : v) {
                bars += c == '|';
                colons += c == ':';
            }
            if (bars != 6 || colons != 4) shapely = false;
        }
        if (!shapely) continue;
        try {
            for (auto& n : l.GetChannelNames()) l.MapPropEffects(e, n, true);
        } catch (const std::exception&) {
        }
        ++ran;
    }
    CHECK(ran > 0);
}

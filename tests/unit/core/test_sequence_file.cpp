#include <catch2/catch_test_macros.hpp>

#include "render/DataLayer.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "support/TempDir.h"

#include <memory>
#include <string>
#include <vector>

namespace {
struct Loaded {
    HeadlessRenderContext ctx;
    std::unique_ptr<SequenceFile> file;
    bool ok = false;

    Loaded(const std::string& path, const std::string& showDir) {
        ctx.showDirectory = showDir;
        file = std::make_unique<SequenceFile>(path);
        auto doc = file->Open(showDir, true, path);
        if (doc) {
            ok = ctx.LoadSequenceElements(*file, *doc);
        }
    }
    SequenceElements& se() { return ctx.GetSequenceElements(); }
};

struct Source {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    Source() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
};

std::string FixedPointDoc(const std::string& body, const std::string& head = "<sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>10.000</sequenceDuration>") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
           "<head><version>2024.01</version>" + head + "</head>" + body + "</xsequence>";
}
}

TEST_CASE("SequenceFile defaults", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    CHECK(sf.GetSequenceType() == "Animation");
    CHECK(sf.GetSequenceTiming() == "50 ms");
    CHECK(sf.GetFrameMS() == 50);
    CHECK(sf.GetFrequency() == 20);
    CHECK(sf.GetSequenceDurationMS() == 30000);
    CHECK(sf.GetRenderMode() == SequenceFile::ERASE_MODE);
    CHECK_FALSE(sf.IsOpen());
    CHECK(sf.supportsModelBlending());
    CHECK(sf.GetMedia() == nullptr);
}

TEST_CASE("SequenceFile frame time constructor argument", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq", 25);
    CHECK(sf.GetFrameMS() == 25);
    CHECK(sf.GetFrequency() == 40);
}

TEST_CASE("SequenceFile frame time math", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    sf.SetSequenceTiming("33 ms");
    CHECK(sf.GetFrameMS() == 33);
    CHECK(sf.GetFrequency() == 30);
    sf.SetSequenceTiming("100 ms");
    CHECK(sf.GetFrequency() == 10);
    sf.SetSequenceTiming("garbage");
    CHECK(sf.GetFrameMS() == 0);
    CHECK(sf.GetFrequency() == 20);
}

TEST_CASE("SequenceFile duration setters and formatting", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    sf.SetSequenceDurationMS(12345);
    CHECK(sf.GetSequenceDurationMS() == 12345);
    CHECK(sf.GetSequenceDurationString() == "12.345");
    sf.SetSequenceDuration("1.5");
    CHECK(sf.GetSequenceDurationMS() == 1500);
    sf.SetSequenceDuration(2.25);
    CHECK(sf.GetSequenceDurationString() == "2.250");
}

TEST_CASE("SequenceFile path accessors", "[sequence][xsq]") {
    TempDir td;
    std::string p = td.File("show.xsq");
    SequenceFile sf(p);
    CHECK(sf.GetName() == "show");
    CHECK(sf.GetFullName() == "show.xsq");
    CHECK(sf.GetExt() == "xsq");
    CHECK(sf.GetPath() == td.path.string());
    sf.SetExt("xml");
    CHECK(sf.GetExt() == "xml");
    CHECK(sf.GetFullName() == "show.xml");
    CHECK_FALSE(sf.FileExists());
}

TEST_CASE("SequenceFile header info set and get", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, "Me");
    sf.SetHeaderInfo(HEADER_INFO_TYPES::SONG, "Tune");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR) == "Me");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::SONG) == "Tune");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::ALBUM).empty());
}

TEST_CASE("SequenceFile RecordImportedFrom keeps a bounded most recent first list", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    sf.RecordImportedFrom("");
    CHECK(sf.GetImportedFrom().empty());
    sf.RecordImportedFrom("a.xsq");
    sf.RecordImportedFrom("b.xsq");
    sf.RecordImportedFrom("a.xsq");
    REQUIRE(sf.GetImportedFrom().size() == 2);
    CHECK(sf.GetImportedFrom()[0] == "a.xsq");
    CHECK(sf.GetImportedFrom()[1] == "b.xsq");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::IMPORTED_FROM) == "a.xsq");
    for (int i = 0; i < 30; ++i) {
        sf.RecordImportedFrom("f" + std::to_string(i));
    }
    CHECK(sf.GetImportedFrom().size() == SequenceFile::MAX_IMPORTED_FROM);
    CHECK(sf.GetImportedFrom().front() == "f29");
}

TEST_CASE("SequenceFile SetImportedFrom replaces the list and keeps the header in sync", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    sf.RecordImportedFrom("old.xsq");

    sf.SetImportedFrom({ "", "a.xsq", "b.xsq", "a.xsq", "" });
    REQUIRE(sf.GetImportedFrom().size() == 2);
    CHECK(sf.GetImportedFrom()[0] == "a.xsq");
    CHECK(sf.GetImportedFrom()[1] == "b.xsq");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::IMPORTED_FROM) == "a.xsq");

    std::vector<std::string> many;
    for (int i = 0; i < 30; ++i) {
        many.push_back("f" + std::to_string(i));
    }
    sf.SetImportedFrom(many);
    CHECK(sf.GetImportedFrom().size() == SequenceFile::MAX_IMPORTED_FROM);
    CHECK(sf.GetImportedFrom().front() == "f0");
    CHECK(sf.GetImportedFrom().back() == "f" + std::to_string(SequenceFile::MAX_IMPORTED_FROM - 1));

    sf.SetImportedFrom({});
    CHECK(sf.GetImportedFrom().empty());
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::IMPORTED_FROM).empty());
}

TEST_CASE("SequenceFile timing list helpers", "[sequence][xsq]") {
    Source s;
    s.se.AddElement("Beats", "timing", true, false, true, false, false);
    s.se.AddElement("Tree", "model", true, false, true, false, false);
    s.se.AddElement("Bars", "timing", true, false, true, false, false);
    SequenceFile sf("dummy.xsq");
    auto list = sf.GetTimingList(s.se);
    REQUIRE(list.size() == 2);
    CHECK(list[0] == "Beats");
    CHECK(list[1] == "Bars");
    sf.SetTimingSectionName("Beats", "Renamed");
    CHECK(sf.GetTimingList()[0] == "Renamed");
    sf.DeleteTimingSection("Bars");
    CHECK(sf.GetTimingList().size() == 1);
}

TEST_CASE("SequenceFile render mode is held in the Nutcracker data layer", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    sf.SetRenderMode(SequenceFile::CANVAS_MODE);
    CHECK(sf.GetRenderMode() == SequenceFile::CANVAS_MODE);
    CHECK(sf.GetDataLayers().GetNumLayers() == 1);
    CHECK(sf.GetDataLayers().GetDataLayer(0)->GetName() == "Nutcracker");
}

TEST_CASE("SequenceFile alt track display names", "[sequence][xsq]") {
    SequenceFile sf("dummy.xsq");
    CHECK(sf.GetAltTrackCount() == 0);
    CHECK(sf.GetAltTrackDisplayName(0).empty());
    CHECK(sf.GetAltTrackMedia(3) == nullptr);
}

TEST_CASE("SequenceFile GetFSEQForXSQ looks beside the xsq then in the fseq folder", "[sequence][xsq]") {
    TempDir td;
    auto xsq = td.Write("a.xsq", "x");
    CHECK(SequenceFile::GetFSEQForXSQ(xsq, td.File("fseq")).empty());
    std::filesystem::create_directories(td.path / "fseq");
    td.Write("fseq/a.fseq", "x");
    CHECK(SequenceFile::GetFSEQForXSQ(xsq, td.File("fseq")) == (td.path / "fseq" / "a.fseq").string());
    td.Write("a.fseq", "x");
    CHECK(SequenceFile::GetFSEQForXSQ(xsq, td.File("fseq")) == td.File("a.fseq"));
    CHECK(SequenceFile::GetFSEQForXSQ(td.File("missing.xsq"), td.File("fseq")).empty());
}

TEST_CASE("SequenceFile IsXmlSequence detects the xsequence root", "[sequence][xsq]") {
    TempDir td;
    CHECK(SequenceFile::IsXmlSequence(td.Write("a.xsq", "<?xml version=\"1.0\"?><xsequence/>")));
    CHECK_FALSE(SequenceFile::IsXmlSequence(td.Write("b.xsq", "<other/>")));
    CHECK_FALSE(SequenceFile::IsXmlSequence(td.File("missing.xsq")));
}

TEST_CASE("SequenceFile Open fails cleanly for missing and non xml files", "[sequence][xsq]") {
    TempDir td;
    SequenceFile missing(td.File("nope.xsq"));
    CHECK_FALSE(missing.Open(td.path.string(), true, td.File("nope.xsq")).has_value());

    auto junk = td.Write("junk.xsq", "this is not xml at all");
    SequenceFile sf(junk);
    CHECK_FALSE(sf.Open(td.path.string(), true, junk).has_value());
    CHECK_FALSE(sf.IsOpen());
}

TEST_CASE("SequenceFile Open fails cleanly for truncated xml", "[sequence][xsq]") {
    TempDir td;
    auto full = FixedPointDoc("<ElementEffects/>");
    auto p = td.Write("t.xsq", full.substr(0, full.size() / 2));
    SequenceFile sf(p);
    auto doc = sf.Open(td.path.string(), true, p);
    CHECK_FALSE(doc.has_value());
}

TEST_CASE("SequenceFile Open reads the header", "[sequence][xsq]") {
    TempDir td;
    auto p = td.Write("h.xsq", FixedPointDoc("<ElementEffects/>",
        "<version>2024.05</version><author>A &amp; B</author><author-email>a@b.c</author-email><song>S</song><artist>Art</artist>"
        "<album>Alb</album><MusicURL>http://x</MusicURL><comment>hi</comment><sequenceTiming>25 ms</sequenceTiming>"
        "<sequenceType>Animation</sequenceType><sequenceDuration>42.500</sequenceDuration><imageDir></imageDir>"));
    SequenceFile sf(p);
    auto doc = sf.Open(td.path.string(), true, p);
    REQUIRE(doc.has_value());
    CHECK(sf.IsOpen());
    CHECK(sf.GetFrameMS() == 25);
    CHECK(sf.GetSequenceDurationMS() == 42500);
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::SONG) == "S");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::ARTIST) == "Art");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::ALBUM) == "Alb");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::URL) == "http://x");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::COMMENT) == "hi");
    CHECK(sf.GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL) == "a@b.c");
    CHECK(sf.GetVersion() == "2024.05");
}

TEST_CASE("SequenceFile Open without media for an Animation sequence", "[sequence][xsq]") {
    TempDir td;
    auto p = td.Write("m.xsq", FixedPointDoc("<ElementEffects/>",
        "<sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><mediaFile>/no/such/song.mp3</mediaFile><sequenceDuration>5.000</sequenceDuration>"));
    SequenceFile sf(p);
    auto doc = sf.Open(td.path.string(), false, p);
    REQUIRE(doc.has_value());
    CHECK_FALSE(sf.HasAudioMedia());
}

TEST_CASE("xsq round trip of model effects", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("rt.xsq");
    {
        Source s;
        auto* m = s.se.AddElement("Tree", "model", true, false, true, false, false);
        auto* l0 = m->AddEffectLayer();
        l0->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=80,X_Foo=bar", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 1000, 0, false);
        l0->AddEffect(0, "Off", "", "", 1000, 2000, 0, false);
        auto* l1 = m->AddEffectLayer();
        l1->SetLayerName("Second");
        l1->AddEffect(0, "Bars", "", "", 500, 1500, 0, true);
        SequenceFile sf(path);
        sf.SetSequenceDurationMS(10000);
        sf.SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, "Tester");
        sf.SetHeaderInfo(HEADER_INFO_TYPES::COMMENT, "a <b> & c");
        REQUIRE(sf.Save(s.se));
    }
    REQUIRE(std::filesystem::exists(path));

    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.file->GetSequenceDurationMS() == 10000);
    CHECK(in.file->GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR) == "Tester");
    CHECK(in.file->GetHeaderInfo(HEADER_INFO_TYPES::COMMENT) == "a <b> & c");

    Element* m = in.se().GetElement("Tree");
    REQUIRE(m != nullptr);
    CHECK(m->GetType() == ElementType::ELEMENT_TYPE_MODEL);
    REQUIRE(m->GetEffectLayerCount() == 2);
    EffectLayer* l0 = m->GetEffectLayer(0);
    REQUIRE(l0->GetEffectCount() == 2);
    CHECK(l0->GetEffect(0)->GetEffectName() == "On");
    CHECK(l0->GetEffect(0)->GetStartTimeMS() == 0);
    CHECK(l0->GetEffect(0)->GetEndTimeMS() == 1000);
    CHECK(l0->GetEffect(0)->GetSetting("X_Foo") == "bar");
    CHECK(l0->GetEffect(0)->GetSetting("E_TEXTCTRL_Eff_On_Start") == "80");
    REQUIRE(l0->GetEffect(0)->GetPaletteSize() == 1);
    CHECK(l0->GetEffect(0)->GetPalette()[0].Red() == 255);
    CHECK(l0->GetEffect(1)->GetEffectName() == "Off");
    CHECK(l0->GetEffect(1)->GetStartTimeMS() == 1000);
    EffectLayer* l1 = m->GetEffectLayer(1);
    CHECK(l1->GetLayerName() == "Second");
    REQUIRE(l1->GetEffectCount() == 1);
    CHECK(l1->GetEffect(0)->GetEffectName() == "Bars");
    CHECK(l1->GetEffect(0)->GetProtected());
}

TEST_CASE("xsq round trip of timing tracks", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("timing.xsq");
    {
        Source s;
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("Beats", "timing", true, false, true, false, false));
        t->SetSubType("Beats");
        auto* l = t->AddEffectLayer();
        l->AddEffect(0, "one", "", "", 0, 500, 0, false);
        l->AddEffect(0, "two", "", "", 500, 1000, 0, false);
        auto* l2 = t->AddEffectLayer();
        l2->AddEffect(0, "word", "", "", 0, 250, 0, false);
        SequenceFile sf(path);
        REQUIRE(sf.Save(s.se));
    }
    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    auto* t = in.se().GetTimingElement("Beats");
    REQUIRE(t != nullptr);
    CHECK(t->GetSubType() == "Beats");
    CHECK(t->GetActive());
    REQUIRE(t->GetEffectLayerCount() == 2);
    REQUIRE(t->GetEffectLayer(0)->GetEffectCount() == 2);
    CHECK(t->GetEffectLayer(0)->GetEffect(1)->GetEffectName() == "two");
    CHECK(t->GetEffectLayer(0)->GetEffect(1)->GetStartTimeMS() == 500);
    CHECK(t->GetEffectLayer(1)->GetEffect(0)->GetEffectName() == "word");
}

TEST_CASE("xsq round trip of a fixed interval timing track", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("fixed.xsq");
    {
        Source s;
        auto* t = dynamic_cast<TimingElement*>(s.se.AddElement("Fixed", "timing", true, false, true, false, false));
        t->SetFixedTiming(500);
        t->AddEffectLayer();
        SequenceFile sf(path);
        sf.SetSequenceDurationMS(2000);
        REQUIRE(sf.Save(s.se));
    }
    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    auto* t = in.se().GetTimingElement("Fixed");
    REQUIRE(t != nullptr);
    CHECK(t->IsFixedTiming());
    CHECK(t->GetFixedTiming() == 500);
    REQUIRE(t->GetEffectLayerCount() == 1);
    CHECK(t->GetEffectLayer(0)->GetEffectCount() == 4);
    CHECK(t->GetEffectLayer(0)->GetEffect(3)->GetEndTimeMS() == 2000);
}

TEST_CASE("xsq round trip keeps element flags and order", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("flags.xsq");
    {
        Source s;
        auto* a = s.se.AddElement("A", "model", true, true, true, false, false);
        auto* b = s.se.AddElement("B", "model", false, false, true, false, true);
        s.se.AddElement("C", "model", true, false, true, false, false);
        a->AddEffectLayer();
        b->AddEffectLayer();
        SequenceFile sf(path);
        REQUIRE(sf.Save(s.se));
    }
    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    REQUIRE(in.se().GetElementCount() == 3);
    CHECK(in.se().GetElement((size_t)0)->GetName() == "A");
    CHECK(in.se().GetElement((size_t)1)->GetName() == "B");
    CHECK(in.se().GetElement((size_t)2)->GetName() == "C");
    CHECK(in.se().GetElement("A")->GetCollapsed());
    CHECK_FALSE(in.se().GetElement("B")->GetVisible());
    CHECK(in.se().GetElement("B")->IsRenderDisabled());
    CHECK_FALSE(in.se().GetElement("C")->GetCollapsed());
}

TEST_CASE("xsq round trip keeps timeline tags and sequence settings", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("tags.xsq");
    {
        Source s;
        s.se.AddElement("A", "model", true, false, true, false, false)->AddEffectLayer();
        s.se.SetTagPosition(0, 1234);
        s.se.SetTagPosition(9, 99);
        s.se.SetSupportsModelBlending(false);
        SequenceFile sf(path, 25);
        sf.SetSequenceDurationMS(5000);
        sf.setSupportsModelBlending(false);
        REQUIRE(sf.Save(s.se));
    }
    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.se().GetTagPosition(0) == 1234);
    CHECK(in.se().GetTagPosition(9) == 99);
    CHECK(in.se().GetTagPosition(1) == -1);
    CHECK(in.file->GetFrameMS() == 25);
    CHECK(in.se().GetFrequency() == 40);
    CHECK_FALSE(in.se().SupportsModelBlending());
}

TEST_CASE("xsq written file has the expected top level structure", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("struct.xsq");
    Source s;
    s.se.AddElement("Tree", "model", true, false, true, false, false)->AddEffectLayer()->AddEffect(0, "On", "X_A=1", "", 0, 100, 0, false);
    SequenceFile sf(path);
    REQUIRE(sf.Save(s.se));

    pugi::xml_document doc;
    REQUIRE(doc.load_file(path.c_str()));
    auto root = doc.child("xsequence");
    REQUIRE(root);
    CHECK(std::string(root.attribute("FixedPointTiming").as_string()) == "1");
    CHECK(root.child("head"));
    CHECK(root.child("ColorPalettes"));
    CHECK(root.child("EffectDB"));
    CHECK(root.child("DataLayers"));
    CHECK(root.child("DisplayElements"));
    CHECK(root.child("ElementEffects"));
    CHECK(root.child("TimingTags").children("Tag").begin() != root.child("TimingTags").children("Tag").end());
    auto eff = root.child("ElementEffects").child("Element").child("EffectLayer").child("Effect");
    REQUIRE(eff);
    CHECK(std::string(eff.attribute("name").as_string()) == "On");
    CHECK(eff.attribute("startTime").as_int() == 0);
    CHECK(eff.attribute("endTime").as_int() == 100);
    CHECK(std::string(root.child("EffectDB").child("Effect").text().as_string()) == "X_A=1");
}

TEST_CASE("xsq identical effect settings share one EffectDB entry", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("db.xsq");
    Source s;
    auto* l = s.se.AddElement("Tree", "model", true, false, true, false, false)->AddEffectLayer();
    l->AddEffect(0, "On", "X_A=1", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    l->AddEffect(0, "On", "X_A=1", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 100, 200, 0, false);
    l->AddEffect(0, "On", "X_A=2", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 200, 300, 0, false);
    SequenceFile sf(path);
    REQUIRE(sf.Save(s.se));
    pugi::xml_document doc;
    REQUIRE(doc.load_file(path.c_str()));
    auto root = doc.child("xsequence");
    CHECK(std::distance(root.child("EffectDB").children().begin(), root.child("EffectDB").children().end()) == 2);
    CHECK(std::distance(root.child("ColorPalettes").children().begin(), root.child("ColorPalettes").children().end()) == 1);
}

TEST_CASE("xsq round trip is stable across a second save", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string p1 = td.File("one.xsq");
    std::string p2 = td.File("two.xsq");
    {
        Source s;
        auto* m = s.se.AddElement("Tree", "model", true, false, true, false, false);
        auto* l = m->AddEffectLayer();
        l->AddEffect(0, "On", "X_A=1", "C_BUTTON_Palette1=#00ff00,C_CHECKBOX_Palette1=1", 0, 1000, 0, false);
        l->AddEffect(0, "Off", "", "", 1000, 2000, 0, false);
        auto* t = s.se.AddElement("Beats", "timing", true, false, true, false, false);
        t->AddEffectLayer()->AddEffect(0, "x", "", "", 0, 500, 0, false);
        SequenceFile sf(p1);
        sf.SetSequenceDurationMS(3000);
        REQUIRE(sf.Save(s.se));
    }
    {
        Loaded in(p1, td.path.string());
        REQUIRE(in.ok);
        in.file->SetFullPath(p2);
        REQUIRE(in.file->Save(in.se()));
    }
    auto read = [](const std::string& p) {
        std::ifstream f(p, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    };
    CHECK(read(p1) == read(p2));
}

TEST_CASE("xsq with non fixed point timing is converted from seconds", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB><Effect>X_A=1</Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer><Effect ref=\"0\" name=\"On\" startTime=\"1.5\" endTime=\"2.5\"/></EffectLayer></Element></ElementEffects>";
    std::string doc =
        "<?xml version=\"1.0\"?><xsequence BaseChannel=\"0\" ModelBlending=\"true\"><head><version>2016.1</version>"
        "<sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>10.000</sequenceDuration></head>" + body + "</xsequence>";
    auto p = td.Write("old.xsq", doc);
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    auto* l = in.se().GetElement("Tree")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 1500);
    CHECK(l->GetEffect(0)->GetEndTimeMS() == 2500);
}

TEST_CASE("xsq load rounds effect times to the frame period", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB><Effect></Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer><Effect ref=\"0\" name=\"On\" startTime=\"1010\" endTime=\"2040\"/></EffectLayer></Element></ElementEffects>";
    auto p = td.Write("round.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    auto* l = in.se().GetElement("Tree")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(l->GetEffect(0)->GetStartTimeMS() % 50 == 0);
    CHECK(l->GetEffect(0)->GetEndTimeMS() % 50 == 0);
}

TEST_CASE("xsq load drops effects whose end is not after start", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB><Effect></Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer>"
        "<Effect ref=\"0\" name=\"On\" startTime=\"1000\" endTime=\"1000\"/>"
        "<Effect ref=\"0\" name=\"On\" startTime=\"2000\" endTime=\"1000\"/>"
        "<Effect ref=\"0\" name=\"On\" startTime=\"0\" endTime=\"500\"/>"
        "</EffectLayer></Element></ElementEffects>";
    auto p = td.Write("bad.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.se().GetElement("Tree")->GetEffectLayer(0)->GetEffectCount() == 1);
}

TEST_CASE("xsq load skips effects with bad ref or palette indices", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB><Effect>X_A=1</Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer>"
        "<Effect ref=\"9\" name=\"On\" startTime=\"0\" endTime=\"500\" palette=\"7\"/>"
        "<Effect ref=\"-1\" name=\"Off\" startTime=\"500\" endTime=\"1000\"/>"
        "</EffectLayer></Element></ElementEffects>";
    auto p = td.Write("refs.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    auto* l = in.se().GetElement("Tree")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(0)->GetSettingsAsString().empty());
    CHECK(l->GetEffect(0)->GetPaletteSize() == 0);
}

TEST_CASE("xsq load ignores unknown effect types", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB><Effect></Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer>"
        "<Effect ref=\"0\" name=\"NotARealEffect\" startTime=\"0\" endTime=\"500\"/>"
        "<Effect ref=\"0\" name=\"On\" startTime=\"500\" endTime=\"1000\"/>"
        "</EffectLayer></Element></ElementEffects>";
    auto p = td.Write("unk.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    auto* l = in.se().GetElement("Tree")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(l->GetEffect(0)->GetEffectName() == "On");
}

TEST_CASE("xsq load does not load the legacy Random effect", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB><Effect></Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer>"
        "<Effect ref=\"0\" name=\"Random\" startTime=\"0\" endTime=\"500\"/>"
        "</EffectLayer></Element></ElementEffects>";
    auto p = td.Write("rnd.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.se().GetElement("Tree")->GetEffectLayer(0)->GetEffectCount() == 0);
}

TEST_CASE("xsq load rejects duplicate element names", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/>"
        "<Element collapsed=\"0\" type=\"model\" name=\"Tree\" visible=\"1\"/></DisplayElements>"
        "<EffectDB/><ElementEffects/>";
    auto p = td.Write("dup.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.se().GetElementCount() == 1);
}

TEST_CASE("xsq load trims whitespace in element names and gives every element a layer", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"0\" type=\"model\" name=\"  Tree \" visible=\"1\"/></DisplayElements>"
        "<EffectDB/><ElementEffects/>";
    auto p = td.Write("ws.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    REQUIRE(in.se().GetElement("Tree") != nullptr);
    CHECK(in.se().GetElement("Tree")->GetEffectLayerCount() == 1);
}

TEST_CASE("xsq load ignores timing tags outside the supported range", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<TimingTags><Tag number=\"0\" position=\"10\"/><Tag number=\"10\" position=\"20\"/><Tag number=\"-3\" position=\"30\"/><Tag number=\"99999\" position=\"5\"/></TimingTags>"
        "<DisplayElements/><EffectDB/><ElementEffects/>";
    auto p = td.Write("tags.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.se().GetTagPosition(0) == 10);
    for (int i = 1; i < SequenceElements::TagCount; ++i) {
        CHECK(in.se().GetTagPosition(i) == -1);
    }
    CHECK(in.se().GetTagPosition(-1) == -1);
    CHECK(in.se().GetTagPosition(SequenceElements::TagCount) == -1);
}

TEST_CASE("xsq load tolerates garbage attribute values", "[sequence][xsq]") {
    TempDir td;
    std::string body =
        "<DisplayElements><Element collapsed=\"maybe\" type=\"model\" name=\"Tree\" visible=\"banana\"/></DisplayElements>"
        "<EffectDB><Effect></Effect></EffectDB>"
        "<ElementEffects><Element type=\"model\" name=\"Tree\"><EffectLayer>"
        "<Effect ref=\"zzz\" name=\"On\" startTime=\"abc\" endTime=\"def\"/>"
        "<Effect ref=\"0\" name=\"On\" startTime=\"0\" endTime=\"500\"/></EffectLayer></Element></ElementEffects>";
    auto p = td.Write("garbage.xsq", FixedPointDoc(body));
    Loaded in(p, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.se().GetElement("Tree")->GetEffectLayer(0)->GetEffectCount() == 1);
}

TEST_CASE("xsq data layer settings round trip", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("dl.xsq");
    {
        Source s;
        s.se.AddElement("A", "model", true, false, true, false, false)->AddEffectLayer();
        SequenceFile sf(path);
        sf.SetRenderMode(SequenceFile::CANVAS_MODE);
        REQUIRE(sf.Save(s.se));
    }
    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    CHECK(in.file->GetRenderMode() == SequenceFile::CANVAS_MODE);
    CHECK(in.file->GetDataLayers().GetNumLayers() == 1);
}

TEST_CASE("xsq imported from list round trips", "[sequence][xsq][roundtrip]") {
    TempDir td;
    std::string path = td.File("imp.xsq");
    {
        Source s;
        s.se.AddElement("A", "model", true, false, true, false, false)->AddEffectLayer();
        SequenceFile sf(path);
        sf.RecordImportedFrom("one.xsq");
        sf.RecordImportedFrom("two.xsq");
        REQUIRE(sf.Save(s.se));
    }
    Loaded in(path, td.path.string());
    REQUIRE(in.ok);
    REQUIRE(in.file->GetImportedFrom().size() == 2);
    CHECK(in.file->GetImportedFrom()[0] == "two.xsq");
    CHECK(in.file->GetHeaderInfo(HEADER_INFO_TYPES::IMPORTED_FROM) == "two.xsq");
}

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "../support/TempDir.h"
#include "outputs/OutputManager.h"
#include "outputs/TestPreset.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "settings/XLightsSettings.h"
#include "utils/UtilFunctions.h"

namespace {

std::string ReadAll(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

struct Scratch {
    std::string name;
    explicit Scratch(const char* n) : name(n) { XLightsSettings::Get().DeleteAll(name); }
    ~Scratch() {
        XLightsSettings::Get().DeleteAll(name);
        XLightsSettings::Get().SetFilePath({});
    }
};

} // namespace

// ---- XmlSafe / UnXmlSafe -----------------------------------------------------------------------------------------

TEST_CASE("Roundtrip XmlSafe of ASCII text is reversible", "[roundtrip][xmlsafe]") {
    for (const char* s : { "plain", "a & b", "<tag>", "\"q\" 'a'", "&amp;", "&#65;", "a\tb", "a\nb", "a\r\nb", "", " ", "100%", "a,b;c=d|e", "C:\\dir\\file", "tab\t\tnl\n\n" }) {
        INFO(s);
        CHECK(UnXmlSafe(XmlSafe(s)) == s);
    }
}
TEST_CASE("Roundtrip XmlSafe output has no raw markup characters", "[roundtrip][xmlsafe]") {
    for (const char* s : { "a & b", "<tag>", "\"q\" 'a'", "x>y" }) {
        INFO(s);
        std::string e = XmlSafe(s);
        CHECK(e.find('<') == std::string::npos);
        CHECK(e.find('>') == std::string::npos);
        CHECK(e.find('"') == std::string::npos);
        CHECK(e.find('\'') == std::string::npos);
    }
}
TEST_CASE("Roundtrip XmlSafe is not idempotent but UnXmlSafe undoes each level", "[roundtrip][xmlsafe]") {
    std::string s = "a & <b>";
    std::string once = XmlSafe(s);
    std::string twice = XmlSafe(once);
    CHECK(once != twice);
    CHECK(UnXmlSafe(twice) == once);
    CHECK(UnXmlSafe(UnXmlSafe(twice)) == s);
}
TEST_CASE("Roundtrip XmlSafe control characters other than tab and newline are dropped", "[roundtrip][xmlsafe]") {
    CHECK(XmlSafe(std::string("a\x01" "b\x1F" "c")) == "abc");
    CHECK(XmlSafe(std::string("a\0b", 3)) == "ab");
}
TEST_CASE("Roundtrip XmlSafe keeps UTF-8 text", "[roundtrip][xmlsafe]") {
    for (const char* s : { "caf\xC3\xA9", "\xE6\x97\xA5\xE6\x9C\xAC", "\xE2\x9D\x84", "\xF0\x9F\x8E\x84", "na\xC3\xAFve \xC3\xBC\xC3\xB1" }) {
        INFO(s);
        CHECK(UnXmlSafe(XmlSafe(s)) == s);
    }
}
TEST_CASE("Roundtrip UnXmlSafe leaves unrelated ampersands alone", "[roundtrip][xmlsafe]") {
    CHECK(UnXmlSafe("a & b") == "a & b");
    CHECK(UnXmlSafe("&unknown;") == "&unknown;");
    CHECK(UnXmlSafe("&") == "&");
    CHECK(UnXmlSafe("&#") == "&#");
    CHECK(UnXmlSafe("&#x41;") == "&#x41;");
}
TEST_CASE("Roundtrip UnXmlSafe of a numeric control reference", "[roundtrip][xmlsafe]") {
    CHECK(UnXmlSafe("a&#10;b") == "a\nb");
    CHECK(UnXmlSafe("a&#9;b") == "a\tb");
    CHECK(UnXmlSafe("a&#13;b") == "a\rb");
}
TEST_CASE("Roundtrip IsXmlSafe agrees with XmlSafe on ASCII", "[roundtrip][xmlsafe]") {
    CHECK(IsXmlSafe("plain text"));
    CHECK_FALSE(IsXmlSafe("a,b"));
    CHECK_FALSE(IsXmlSafe("it's"));
    CHECK_FALSE(IsXmlSafe("caf\xC3\xA9"));
}

// ---- settings store ----------------------------------------------------------------------------------------------------

TEST_CASE("Roundtrip settings doubles survive flush and load", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_doubles");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    const std::vector<double> values = { 0.0, -0.0, 0.1, 1.0 / 3.0, -1.0 / 3.0, 1e-300, 1e300, 123456789.123456789, std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), std::numeric_limits<double>::denorm_min(), 2.5, -7.75, 1e15, 1e16, 0.30000000000000004 };
    for (size_t i = 0; i < values.size(); ++i) c.WriteDouble("d" + std::to_string(i), values[i], s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    for (size_t i = 0; i < values.size(); ++i) {
        INFO(i << " " << values[i]);
        double got = c.ReadDouble("d" + std::to_string(i), -999.0, s.name);
        CHECK(got == values[i]);
    }
}
TEST_CASE("Roundtrip settings longs survive flush and load", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_longs");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    std::vector<long> values = { 0, 1, -1, 255, 65535, 2147483647L, -2147483647L - 1, std::numeric_limits<long>::max(), std::numeric_limits<long>::min() };
    if constexpr (sizeof(long) == 8) { // long is 32-bit on Windows
        values.push_back(static_cast<long>(4294967296LL));
        values.push_back(static_cast<long>(9007199254740993LL));
    }
    for (size_t i = 0; i < values.size(); ++i) c.WriteLong("l" + std::to_string(i), values[i], s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    for (size_t i = 0; i < values.size(); ++i) {
        INFO(i << " " << values[i]);
        CHECK(c.ReadLong("l" + std::to_string(i), -999, s.name) == values[i]);
    }
}
TEST_CASE("Roundtrip settings bools and types survive flush and load", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_types");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    c.WriteBool("t", true, s.name);
    c.WriteBool("f", false, s.name);
    c.WriteLong("l", 5, s.name);
    c.WriteDouble("d", 5.5, s.name);
    c.WriteString("s", "5", s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    CHECK(c.GetValueType("t", s.name) == XLightsSettings::ValueType::Boolean);
    CHECK(c.GetValueType("f", s.name) == XLightsSettings::ValueType::Boolean);
    CHECK(c.GetValueType("l", s.name) == XLightsSettings::ValueType::Integer);
    CHECK(c.GetValueType("d", s.name) == XLightsSettings::ValueType::Float);
    CHECK(c.GetValueType("s", s.name) == XLightsSettings::ValueType::String);
    CHECK(c.ReadBool("t", false, s.name));
    CHECK_FALSE(c.ReadBool("f", true, s.name));
}
TEST_CASE("Roundtrip settings a whole number double stays a double", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_wholedouble");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    c.WriteDouble("d", 3.0, s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    CHECK(c.GetValueType("d", s.name) == XLightsSettings::ValueType::Float);
    CHECK(c.ReadDouble("d", 0, s.name) == 3.0);
}
TEST_CASE("Roundtrip settings file is byte stable across load and flush", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_bytes");
    auto& c = XLightsSettings::Get();
    const std::string path = t.File("s.json");
    c.SetFilePath(path);
    c.WriteString("str", "line1\nline2 \"q\" \xC3\xA9", s.name);
    c.WriteLong("long", -42, s.name);
    c.WriteDouble("dbl", 0.125, s.name);
    c.WriteBool("bool", true, s.name);
    c.Flush();
    // The first load adds the default sections, so measure stability from the second save on.
    c.Load();
    c.WriteString("touch", "x", s.name);
    c.DeleteEntry("touch", s.name);
    c.Flush();
    std::string first = ReadAll(path);
    for (int i = 0; i < 3; ++i) {
        c.Load();
        c.WriteString("touch", "x", s.name);
        c.DeleteEntry("touch", s.name);
        c.Flush();
    }
    c.Load();
    c.Flush();
    CHECK(ReadAll(path) == first);
}
TEST_CASE("Roundtrip settings keys keep their values after many cycles", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_cycles");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    for (int i = 0; i < 50; ++i) {
        c.WriteString("key" + std::to_string(i), "value " + std::to_string(i), s.name);
        c.WriteDouble("dbl" + std::to_string(i), i * 0.1, s.name);
    }
    for (int cycle = 0; cycle < 5; ++cycle) {
        c.Flush();
        c.DeleteAll(s.name);
        c.Load();
    }
    for (int i = 0; i < 50; ++i) {
        CHECK(c.ReadString("key" + std::to_string(i), "", s.name) == "value " + std::to_string(i));
        CHECK(c.ReadDouble("dbl" + std::to_string(i), -1, s.name) == Catch::Approx(i * 0.1).epsilon(1e-12));
    }
    CHECK(c.GetAllKeys(s.name).size() == 100);
}
TEST_CASE("Roundtrip settings strings with special content", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_strings");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    const std::vector<std::string> values = { "", " ", "plain", "a\"b", "a\\b", "a/b", "tab\there", "nl\nhere", "cr\rhere", "\xC3\xA9\xE2\x9D\x84\xF0\x9F\x8E\x84", "{\"json\":[1,2,3]}", "<xml a=\"1\"/>", "&amp;", std::string(70000, 'q'), "path/with spaces/and\\backslashes", "%s %d {} {0}" };
    for (size_t i = 0; i < values.size(); ++i) c.WriteString("s" + std::to_string(i), values[i], s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    for (size_t i = 0; i < values.size(); ++i) {
        INFO(i);
        CHECK(c.HasEntry("s" + std::to_string(i), s.name));
        CHECK(c.ReadString("s" + std::to_string(i), "DEFAULT", s.name) == values[i]);
    }
}
TEST_CASE("Roundtrip settings keys with special content", "[roundtrip][settings]") {
    TempDir t;
    Scratch s("rt_keys");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    const std::vector<std::string> keys = { "a b", "a/b", "a\\b", "a.b", "a\"b", "caf\xC3\xA9", "\xE2\x9D\x84", "123", "a\tb", "{}", "[0]" };
    for (size_t i = 0; i < keys.size(); ++i) c.WriteLong(keys[i], (long)i + 100, s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    for (size_t i = 0; i < keys.size(); ++i) {
        INFO(keys[i]);
        CHECK(c.ReadLong(keys[i], -1, s.name) == (long)i + 100);
    }
    CHECK(c.GetAllKeys(s.name).size() == keys.size());
}
TEST_CASE("Roundtrip settings sections with special names", "[roundtrip][settings]") {
    TempDir t;
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    const std::vector<std::string> sections = { "rt sec space", "rt/sec/slash", "rt.sec.dot", "rt_caf\xC3\xA9" };
    for (auto& sec : sections) {
        c.DeleteAll(sec);
        c.WriteString("k", sec, sec);
    }
    c.Flush();
    for (auto& sec : sections) c.DeleteAll(sec);
    c.Load();
    for (auto& sec : sections) {
        INFO(sec);
        CHECK(c.ReadString("k", "", sec) == sec);
        c.DeleteAll(sec);
    }
    c.SetFilePath({});
}
TEST_CASE("Roundtrip settings default section values", "[roundtrip][settings]") {
    TempDir t;
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("s.json"));
    c.WriteString("rt_main_key", "v");
    c.Flush();
    c.DeleteEntry("rt_main_key");
    c.Load();
    CHECK(c.ReadString("rt_main_key", "") == "v");
    c.DeleteEntry("rt_main_key");
    c.SetFilePath({});
}

// ---- test presets --------------------------------------------------------------------------------------------------------

TEST_CASE("Roundtrip test preset channels are compressed and restored", "[roundtrip][outputs][testpreset]") {
    OutputManager om;
    auto* p = om.CreateTestPreset("p");
    p->AddChannelRange(1, 5);
    p->AddChannel(9);
    p->AddChannelRange(20, 22);
    pugi::xml_document d;
    auto n = d.append_child("root");
    p->Save(n);
    std::ostringstream os;
    n.print(os, "", pugi::format_raw);
    pugi::xml_document d2;
    REQUIRE(d2.load_string(os.str().c_str()));
    TestPreset q(d2.child("root").first_child());
    pugi::xml_document d3;
    auto n3 = d3.append_child("root");
    q.Save(n3);
    std::ostringstream os3;
    n3.print(os3, "", pugi::format_raw);
    CHECK(os3.str() == os.str());
}
TEST_CASE("Roundtrip test preset names with specials", "[roundtrip][outputs][testpreset]") {
    for (const char* name : { "plain", "a & b", "<x>", "quo\"te", "with space", "a,b", "100%" }) {
        INFO(name);
        OutputManager om;
        om.CreateTestPreset(name)->AddChannel(3);
        pugi::xml_document d;
        om.SaveToXML(d);
        std::ostringstream os;
        d.print(os, "", pugi::format_raw);
        pugi::xml_document d2;
        REQUIRE(d2.load_string(os.str().c_str()));
        auto tp = d2.child("Networks").child("testpreset");
        REQUIRE(tp);
        TestPreset q(tp);
        CHECK(q.GetName() == name);
    }
}
TEST_CASE("Roundtrip test preset name with non-ASCII text", "[roundtrip][outputs][testpreset]") {
    OutputManager om;
    om.CreateTestPreset("caf\xC3\xA9 \xE2\x9D\x84")->AddChannel(3);
    pugi::xml_document d;
    om.SaveToXML(d);
    auto tp = d.child("Networks").child("testpreset");
    REQUIRE(tp);
    TestPreset q(tp);
    CHECK(q.GetName() == "caf\xC3\xA9 \xE2\x9D\x84");
}

// ---- timing track export -> xtiming import --------------------------------------------------------------------------------

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
};

using Mark = std::tuple<int, int, std::string>;

std::vector<Mark> Marks(Element* e, int layer) {
    std::vector<Mark> out;
    if (e == nullptr || layer >= (int)e->GetEffectLayerCount()) return out;
    auto* l = e->GetEffectLayer(layer);
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        out.emplace_back(l->GetEffect(i)->GetStartTimeMS(), l->GetEffect(i)->GetEndTimeMS(), l->GetEffect(i)->GetEffectName());
    }
    return out;
}

// Exports `labels` through TimingElement::GetExport and imports the result again; returns the marks of every layer.
std::vector<std::vector<Mark>> ExportImport(const std::vector<std::vector<Mark>>& layers, size_t* importedLayers = nullptr) {
    TempDir td;
    TimingCtx src;
    auto* t = dynamic_cast<TimingElement*>(src.GetSequenceElements().AddElement("Track", "timing", true, false, true, false, false));
    for (auto& layer : layers) {
        auto* l = t->AddEffectLayer();
        for (auto& m : layer) l->AddEffect(0, std::get<2>(m), "", "", std::get<0>(m), std::get<1>(m), 0, false);
    }
    std::string doc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<timing name=\"Track\" SourceVersion=\"2024.01\">\n" + t->GetExport() + "</timing>\n";
    std::string path = td.Write("t.xtiming", doc);
    TimingCtx dst;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    sf.ProcessXTiming({ path }, &dst);
    std::vector<std::vector<Mark>> out;
    Element* e = dst.GetSequenceElements().GetElement("Track");
    if (e == nullptr) return out;
    if (importedLayers) *importedLayers = e->GetEffectLayerCount();
    for (int i = 0; i < (int)e->GetEffectLayerCount(); ++i) out.push_back(Marks(e, i));
    return out;
}

} // namespace

TEST_CASE("Roundtrip timing export and import of one layer", "[roundtrip][timing]") {
    std::vector<std::vector<Mark>> in = { { { 0, 500, "one" }, { 500, 1000, "two" }, { 1000, 1500, "three" } } };
    CHECK(ExportImport(in) == in);
}
TEST_CASE("Roundtrip timing export and import of three layers", "[roundtrip][timing]") {
    std::vector<std::vector<Mark>> in = {
        { { 0, 1000, "phrase" } },
        { { 0, 500, "w1" }, { 500, 1000, "w2" } },
        { { 0, 250, "p1" }, { 250, 500, "p2" }, { 500, 750, "p3" }, { 750, 1000, "p4" } },
    };
    CHECK(ExportImport(in) == in);
}
TEST_CASE("Roundtrip timing export and import of text with ASCII specials", "[roundtrip][timing]") {
    for (const char* label : { "a & b", "<x>", "he said \"hi\"", "it's", "a,b;c", "100%", "tab\there", "x=y", "a|b" }) {
        INFO(label);
        std::vector<std::vector<Mark>> in = { { { 0, 500, label } } };
        CHECK(ExportImport(in) == in);
    }
}
TEST_CASE("Roundtrip timing export and import of many marks", "[roundtrip][timing]") {
    std::vector<Mark> layer;
    for (int i = 0; i < 300; ++i) layer.emplace_back(i * 100, i * 100 + 100, "m" + std::to_string(i));
    std::vector<std::vector<Mark>> in = { layer };
    CHECK(ExportImport(in) == in);
}
TEST_CASE("Roundtrip timing export and import with an empty label", "[roundtrip][timing]") {
    std::vector<std::vector<Mark>> in = { { { 0, 500, "" }, { 500, 1000, "x" } } };
    CHECK(ExportImport(in) == in);
}
TEST_CASE("Roundtrip timing export and import of non-ASCII labels", "[roundtrip][timing]") {
    std::vector<std::vector<Mark>> in = { { { 0, 500, "caf\xC3\xA9" }, { 500, 1000, "\xE6\x97\xA5\xE6\x9C\xAC" } } };
    CHECK(ExportImport(in) == in);
}
TEST_CASE("Roundtrip timing export and import of entity lookalike labels", "[roundtrip][timing]") {
    std::vector<std::vector<Mark>> in = { { { 0, 500, "AT&amp;T" }, { 500, 1000, "&lt;3" } } };
    CHECK(ExportImport(in) == in);
}
TEST_CASE("Roundtrip timing export of an empty track imports as an empty layer", "[roundtrip][timing]") {
    size_t layers = 99;
    auto out = ExportImport({ {} }, &layers);
    CHECK(layers <= 1);
}

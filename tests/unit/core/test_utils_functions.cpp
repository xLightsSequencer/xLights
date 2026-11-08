#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <pugixml.hpp>

#include <set>
#include <string>
#include <vector>

#include "utils/UtilFunctions.h"
#include "utils/UtilClasses.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("FormatTime formats minutes seconds and millis", "[utils]") {
    CHECK(FormatTime(0) == "0:00.000");
    CHECK(FormatTime(999) == "0:00.999");
    CHECK(FormatTime(61001) == "1:01.001");
    CHECK(FormatTime(600000) == "10:00.000");
}

TEST_CASE("FormatTimeHMS formats hours minutes seconds and millis", "[utils]") {
    CHECK(FormatTimeHMS(0) == "00:00:00.000");
    CHECK(FormatTimeHMS(3723004) == "01:02:03.004");
    CHECK(FormatTimeHMS(36000000) == "10:00:00.000");
}

TEST_CASE("fnv1a matches published FNV-1a vectors", "[utils]") {
    CHECK(fnv1a("") == 2166136261u);
    CHECK(fnv1a("a") == 0xe40c292cu);
    CHECK(fnv1a("foobar") == 0xbf9cf968u);
    CHECK(fnv1a("abc") != fnv1a("abd"));
}

TEST_CASE("MakeControllerTimestampKey sanitises separators", "[utils]") {
    const std::string k = MakeControllerTimestampKey("pre", "My Ctrl.1", "");
    CHECK(k == "pre_default_My_Ctrl_1");

    const std::string k2 = MakeControllerTimestampKey("pre", "c", "/shows/xmas");
    CHECK(k2.find('/') == std::string::npos);
    CHECK(k2.find('.') == std::string::npos);
    CHECK(k2.find(' ') == std::string::npos);
    CHECK(k2.rfind("pre_xmas_", 0) == 0);
    CHECK(k2 != MakeControllerTimestampKey("pre", "c", "/shows/other"));
    CHECK(k2 == MakeControllerTimestampKey("pre", "c", "/shows/xmas"));
}

TEST_CASE("Ordinal handles teens and suffixes", "[utils]") {
    CHECK(Ordinal(1) == "1st");
    CHECK(Ordinal(2) == "2nd");
    CHECK(Ordinal(3) == "3rd");
    CHECK(Ordinal(4) == "4th");
    CHECK(Ordinal(11) == "11th");
    CHECK(Ordinal(12) == "12th");
    CHECK(Ordinal(13) == "13th");
    CHECK(Ordinal(21) == "21st");
    CHECK(Ordinal(22) == "22nd");
    CHECK(Ordinal(23) == "23rd");
    CHECK(Ordinal(101) == "101st");
    CHECK(Ordinal(111) == "111th");
    CHECK(Ordinal(0) == "0th");
}

TEST_CASE("DecodeMidi names notes", "[utils]") {
    CHECK(DecodeMidi(60) == "C4");
    CHECK(DecodeMidi(69) == "A4");
    CHECK(DecodeMidi(70) == "A#4");
    CHECK(DecodeMidi(61) == "C#4");
    CHECK(DecodeMidi(71) == "B4");
    CHECK(DecodeMidi(72) == "C5");
    CHECK(DecodeMidi(127) == "G9");
}

TEST_CASE("DecodeMidi never reports a negative octave", "[utils]") {
    CHECK(DecodeMidi(0) == "C0");
    CHECK(DecodeMidi(11) == "B0");
    CHECK(DecodeMidi(12) == "C0");
    CHECK(DecodeMidi(24) == "C1");
}

TEST_CASE("IsEmailValid accepts and rejects", "[utils]") {
    CHECK(IsEmailValid("a@b.com"));
    CHECK(IsEmailValid("first.last+tag@example.co.uk"));
    CHECK(IsEmailValid("  a@b.com  "));
    CHECK_FALSE(IsEmailValid(""));
    CHECK_FALSE(IsEmailValid("   "));
    CHECK_FALSE(IsEmailValid("a@b"));
    CHECK_FALSE(IsEmailValid("a b@c.com"));
    CHECK_FALSE(IsEmailValid("no-at-sign.com"));
}

TEST_CASE("IsVersionOlder compares dotted versions", "[utils]") {
    // IsVersionOlder(compare, version): true when `version` is older than `compare`
    CHECK(IsVersionOlder("2024.10", "2024.9"));
    CHECK_FALSE(IsVersionOlder("2024.9", "2024.10"));
    CHECK_FALSE(IsVersionOlder("2024.10", "2024.10"));
    CHECK(IsVersionOlder("2025.1", "2024.12"));
    CHECK_FALSE(IsVersionOlder("2024.12", "2025.1"));
}

TEST_CASE("IsVersionOlder handles a third component", "[utils]") {
    CHECK(IsVersionOlder("2024.10.2", "2024.10.1"));
    CHECK_FALSE(IsVersionOlder("2024.10.1", "2024.10.2"));
    CHECK_FALSE(IsVersionOlder("2024.10.1", "2024.10.1"));
    CHECK(IsVersionOlder("2024.10.1", "2024.10"));
    CHECK_FALSE(IsVersionOlder("2024.10", "2024.10.1"));
}

TEST_CASE("IsVersionOlder tolerates short versions", "[utils]") {
    CHECK(IsVersionOlder("2025", "2024"));
    CHECK_FALSE(IsVersionOlder("2024", "2025"));
    CHECK_FALSE(IsVersionOlder("2024", "2024"));
    CHECK(IsVersionOlder("2024.1", "2024"));
    CHECK_FALSE(IsVersionOlder("2024", "2024.1"));
    CHECK_FALSE(IsVersionOlder("2024", "2024.0"));
}

TEST_CASE("JSONSafe escapes backslash and quote", "[utils]") {
    CHECK(JSONSafe("plain") == "plain");
    CHECK(JSONSafe("a\"b") == "a\\\"b");
    CHECK(JSONSafe("c:\\dir") == "c:\\\\dir");
    CHECK(JSONSafe("") == "");
}

TEST_CASE("XmlSafe escapes markup and keeps whitespace as entities", "[utils]") {
    CHECK(XmlSafe("<a&b>") == "&lt;a&amp;b&gt;");
    CHECK(XmlSafe("it's \"q\"") == "it&apos;s &quot;q&quot;");
    CHECK(XmlSafe("a\tb") == "a&#9;b");
    CHECK(XmlSafe("a\nb") == "a&#10;b");
    CHECK(XmlSafe("a\rb") == "a&#13;b");
    CHECK(XmlSafe(std::string("a\x01" "b")) == "ab");
}

TEST_CASE("UnXmlSafe decodes the named entities", "[utils]") {
    CHECK(UnXmlSafe("&lt;a&amp;b&gt;") == "<a&b>");
    CHECK(UnXmlSafe("it&apos;s &quot;q&quot;") == "it's \"q\"");
    CHECK(UnXmlSafe("no entities") == "no entities");
    CHECK(UnXmlSafe("&amp;lt;") == "&lt;");
}

TEST_CASE("XmlSafe then UnXmlSafe round trips printable text", "[utils]") {
    const std::string s = "<tag attr=\"v\" other='x'> & more </tag>";
    CHECK(UnXmlSafe(XmlSafe(s)) == s);
}

TEST_CASE("UnXmlSafe round trips whitespace entities written by XmlSafe", "[utils]") {
    CHECK(UnXmlSafe(XmlSafe("a\tb")) == "a\tb");
    CHECK(UnXmlSafe(XmlSafe("a\nb")) == "a\nb");
    CHECK(UnXmlSafe(XmlSafe("a\r\nb1")) == "a\r\nb1");
    CHECK(UnXmlSafe("x&#10;1") == "x\n1");
    CHECK(UnXmlSafe("&#65;") == "&#65;");
}

TEST_CASE("IsXmlSafe rejects control, non ascii, comma and apostrophe", "[utils]") {
    CHECK(IsXmlSafe("Hello World 123"));
    CHECK(IsXmlSafe(""));
    CHECK_FALSE(IsXmlSafe("a,b"));
    CHECK_FALSE(IsXmlSafe("it's"));
    CHECK_FALSE(IsXmlSafe("tab\there"));
    CHECK_FALSE(IsXmlSafe("caf\xC3\xA9"));
}

TEST_CASE("RemoveUnsafeXmlChars strips markup control and high bytes", "[utils]") {
    CHECK(RemoveUnsafeXmlChars("a=b<c>d&e\"f'g") == "abcdefg");
    CHECK(RemoveUnsafeXmlChars("plain text 123") == "plain text 123");
    CHECK(RemoveUnsafeXmlChars("a\tb\nc") == "abc");
    CHECK(RemoveUnsafeXmlChars("caf\xC3\xA9") == "caf");
}

TEST_CASE("EscapeCSV doubles quotes only", "[utils]") {
    CHECK(EscapeCSV("plain") == "plain");
    CHECK(EscapeCSV("say \"hi\"") == "say \"\"hi\"\"");
    CHECK(EscapeCSV("a,b") == "a,b");
    CHECK(EscapeCSV("") == "");
}

TEST_CASE("EscapeRegex escapes metacharacters", "[utils]") {
    CHECK(EscapeRegex("abc") == "abc");
    CHECK(EscapeRegex("a+b") == "a\\+b");
    CHECK(EscapeRegex("(x)") == "\\(x\\)");
    CHECK(EscapeRegex("a*b?") == "a\\*b\\?");
    CHECK(EscapeRegex("^$") == "\\^\\$");
    CHECK(EscapeRegex("a\\b") == "a\\\\b");
    CHECK(EscapeRegex("[x{1}") == "\\[x\\{1}");
}

TEST_CASE("EscapeRegex escapes the dot", "[utils]") {
    CHECK(EscapeRegex("a.b") == "a\\.b");
}

TEST_CASE("isOdd and rounding helpers", "[utils]") {
    CHECK(isOdd(1));
    CHECK(isOdd(-3));
    CHECK_FALSE(isOdd(0));
    CHECK_FALSE(isOdd(10));
    CHECK(roundTo4(0) == 0);
    CHECK(roundTo4(1) == 4);
    CHECK(roundTo4(4) == 4);
    CHECK(roundTo4(5) == 8);
    CHECK(roundTo16(0) == 0);
    CHECK(roundTo16(1) == 16);
    CHECK(roundTo16(16) == 16);
    CHECK(roundTo16(17) == 32);
    CHECK(INTROUNDUPDIV(10, 3) == 4);
    CHECK(INTROUNDUPDIV(9, 3) == 3);
}

TEST_CASE("angle conversions", "[utils]") {
    CHECK_THAT(toRadians(180.0f), WithinAbs(PI, 1e-6));
    CHECK_THAT(toRadians(90.0f), WithinAbs(PI / 2, 1e-6));
    CHECK_THAT(toDegrees(PI), WithinAbs(180.0, 1e-9));
    CHECK_THAT(toDegrees(toRadians(37.5f)), WithinAbs(37.5, 1e-4));
}

TEST_CASE("fp_equal is relative", "[utils]") {
    CHECK(fp_equal(1.0, 1.0));
    CHECK(fp_equal(1.0f, 1.0f));
    CHECK_FALSE(fp_equal(1.0, 1.001));
    CHECK(fp_equal(1000.0, 1000.0 + 1e-9, 1e-6));
}

TEST_CASE("intRand stays within inclusive bounds", "[utils]") {
    std::set<int> seen;
    for (int i = 0; i < 500; ++i) {
        int v = intRand(3, 7);
        REQUIRE(v >= 3);
        REQUIRE(v <= 7);
        seen.insert(v);
    }
    CHECK(seen.size() == 5);
    CHECK(intRand(4, 4) == 4);
}

TEST_CASE("rand01 is in the unit interval", "[utils]") {
    for (int i = 0; i < 200; ++i) {
        double r = rand01();
        REQUIRE(r >= 0.0);
        REQUIRE(r <= 1.0);
    }
}

TEST_CASE("ExtractInt strips a leading number", "[utils]") {
    std::string s = "123abc";
    CHECK(ExtractInt(s) == 123);
    CHECK(s == "abc");

    std::string t = "abc";
    CHECK(ExtractInt(t) == -1);
    CHECK(t == "abc");

    std::string u = "42";
    CHECK(ExtractInt(u) == 42);
    CHECK(u.empty());
}

TEST_CASE("ExtractTrailingInt reads a number after leading text", "[utils]") {
    CHECK(ExtractTrailingInt("Model12") == 12);
    CHECK(ExtractTrailingInt("a-b-007") == 7);
    CHECK(ExtractTrailingInt("") == -1);
}

TEST_CASE("NumberAwareStringCompare orders numbers numerically", "[utils]") {
    CHECK(NumberAwareStringCompare("a2", "a10") == -1);
    CHECK(NumberAwareStringCompare("a10", "a2") == 1);
    CHECK(NumberAwareStringCompare("a10", "a10") == 0);
    CHECK(NumberAwareStringCompare("", "") == 0);
    CHECK(NumberAwareStringCompare("abc", "abd") == -1);
    CHECK(NumberAwareStringCompare("Tree 1 Left", "Tree 1 Right") == -1);
    CHECK(NumberAwareStringCompare("Tree 2", "Tree 11") == -1);
    CHECK(NumberAwareStringCompare("a", "a1") == -1);
    CHECK(NumberAwareStringCompare("x007", "x7") == 0);
}

TEST_CASE("NumberAwareStringCompare helpers sort a list", "[utils]") {
    std::vector<std::string> v = { "m10", "m2", "m1", "m20", "m3" };
    std::sort(v.begin(), v.end(), stdlistNumberAwareStringCompare);
    CHECK(v == std::vector<std::string>{ "m1", "m2", "m3", "m10", "m20" });
    CHECK(NumberAwareStringCompareRev("m2", "m10") == 1);

    std::list<std::string> l = { "b2", "b10", "b1" };
    l.sort(stdlistNumberAwareStringCompare);
    CHECK(l.front() == "b1");
    CHECK(l.back() == "b10");
}

TEST_CASE("GetPathSeparator is a single native separator", "[utils]") {
    CHECK(GetPathSeparator().size() == 1);
#ifdef _WIN32
    CHECK(GetPathSeparator() == "\\");
#else
    CHECK(GetPathSeparator() == "/");
#endif
}

TEST_CASE("GetxFadePort maps index to port", "[utils]") {
    CHECK(GetxFadePort(0) == 0);
    CHECK(GetxFadePort(1) == 49913);
    CHECK(GetxFadePort(10) == 49922);
}

TEST_CASE("ReverseCSV reverses element order", "[utils]") {
    CHECK(ReverseCSV("a,b,c") == "c,b,a");
    CHECK(ReverseCSV("single") == "single");
    CHECK(ReverseCSV("") == "");
}

TEST_CASE("IsFloat accepts plain decimal numbers only", "[utils]") {
    CHECK(IsFloat("1"));
    CHECK(IsFloat("1.5"));
    CHECK(IsFloat("-1.5"));
    CHECK(IsFloat(".5"));
    CHECK_FALSE(IsFloat(""));
    CHECK_FALSE(IsFloat("1.2.3"));
    CHECK_FALSE(IsFloat("--1"));
    CHECK_FALSE(IsFloat("1-"));
    CHECK_FALSE(IsFloat("abc"));
    CHECK_FALSE(IsFloat("1e5"));
    CHECK_FALSE(IsFloat("+1"));
}

TEST_CASE("FindXmlNode finds by tag and attribute and can create", "[utils]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<r><m name="a"/><m name="b"/><n/></r>)"));
    pugi::xml_node r = doc.child("r");

    CHECK(FindXmlNode(r, "m", "name", "b").attribute("name").as_string() == std::string("b"));
    CHECK(FindXmlNode(r, "m", "name", "zzz").empty());
    CHECK(FindXmlNode(r, "n", "", "").name() == std::string("n"));

    pugi::xml_node created = FindXmlNode(r, "m", "name", "c", true);
    REQUIRE_FALSE(created.empty());
    CHECK(created.attribute("name").as_string() == std::string("c"));
    CHECK(FindXmlNode(r, "m", "name", "c").internal_object() == created.internal_object());
}

TEST_CASE("xml path helpers navigate nested nodes", "[utils]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<root><a><b attr="v1">text</b><b attr="v2">second</b><e/></a><list><i>1</i><i>2</i><x>no</x><i>3</i></list></root>)"));
    pugi::xml_node root = doc.child("root");

    CHECK(DoesXmlNodeExist(root, "a"));
    CHECK(DoesXmlNodeExist(root, "a/b"));
    CHECK_FALSE(DoesXmlNodeExist(root, "a/zzz"));
    CHECK_FALSE(DoesXmlNodeExist(root, "q/b"));

    CHECK(GetXmlNodeContent(root, "a/b") == "text");
    CHECK(GetXmlNodeContent(root, "a/e", "dflt") == "dflt");
    CHECK(GetXmlNodeContent(root, "a/missing", "dflt") == "dflt");
    CHECK(GetXmlNodeContent(root, "a/missing") == "");

    CHECK(GetXmlNodeAttribute(root, "a/b", "attr") == "v1");
    CHECK(GetXmlNodeAttribute(root, "a/b", "nope", "d") == "d");
    CHECK(GetXmlNodeAttribute(root, "a/q", "attr", "d") == "d");

    CHECK(GetXmlNodeListContent(root, "list", "i") == std::vector<std::string>{ "1", "2", "3" });
    CHECK(GetXmlNodeListContent(root, "nothere", "i").empty());
}

TEST_CASE("SetXmlNodeAttribute adds and replaces", "[utils]") {
    pugi::xml_document doc;
    pugi::xml_node n = doc.append_child("n");
    SetXmlNodeAttribute(n, "k", "1");
    CHECK(n.attribute("k").as_string() == std::string("1"));
    SetXmlNodeAttribute(n, "k", "2");
    CHECK(n.attribute("k").as_string() == std::string("2"));
    int count = 0;
    for (auto a : n.attributes()) {
        (void)a;
        ++count;
    }
    CHECK(count == 1);
}

TEST_CASE("RangeAccumulator merges overlapping ranges", "[utils]") {
    RangeAccumulator ra;
    ra.Add(0, 10);
    ra.Add(5, 15);
    ra.Add(30, 40);
    ra.Consolidate();
    REQUIRE(ra.size() == 2);
    CHECK(ra.front() == std::pair<int, int>{ 0, 15 });
    CHECK(ra.back() == std::pair<int, int>{ 30, 40 });
}

TEST_CASE("RangeAccumulator respects minimum separation", "[utils]") {
    RangeAccumulator ra;
    ra.Add(0, 10);
    ra.Add(12, 20);
    ra.Consolidate(0);
    CHECK(ra.size() == 2);
    ra.Consolidate(5);
    CHECK(ra.size() == 1);
    CHECK(ra.front() == std::pair<int, int>{ 0, 20 });
    ra.clear();
    CHECK(ra.size() == 0);
}

TEST_CASE("LogarithmicScale sums are monotonic and capped", "[utils]") {
    CHECK(LogarithmicScale::GetLogSum(0) == 0);
    CHECK(LogarithmicScale::GetLogSum(1) == 18);
    CHECK(LogarithmicScale::GetLogSum(2) == 28);
    int prev = 0;
    for (int i = 1; i <= 127; ++i) {
        int s = LogarithmicScale::GetLogSum(i);
        CHECK(s >= prev);
        prev = s;
    }
    CHECK(LogarithmicScale::GetLogSum(500) == LogarithmicScale::GetLogSum(127));
}

TEST_CASE("SettingValue converts lazily and invalidates on assignment", "[utils]") {
    SettingValue v("12");
    CHECK(v.getInt(-1) == 12);
    v = "34";
    CHECK(v.getInt(-1) == 34);
    CHECK(SettingValue("abc").getInt(7) == 7);
    CHECK(SettingValue("99999999999").getInt(7) == 7);
    CHECK_THAT(SettingValue("2.5").getFloat(0.0f), WithinAbs(2.5, 1e-6));
    CHECK_THAT(SettingValue("x").getFloat(1.5f), WithinAbs(1.5, 1e-6));
    CHECK_THAT(SettingValue("-0.125").getDouble(0.0), WithinAbs(-0.125, 1e-12));

    SettingValue b("1");
    CHECK(b.getBool());
    b = "0";
    CHECK_FALSE(b.getBool());
    CHECK(SettingValue("True").getBool());
    CHECK_FALSE(SettingValue("False").getBool());
    CHECK_FALSE(SettingValue("").getBool());
}

TEST_CASE("SettingsMap typed getters use defaults", "[utils]") {
    SettingsMap m;
    m["i"] = "5";
    m["f"] = "1.5";
    m["b"] = "1";
    m["empty"] = "";
    CHECK(m.GetInt("i") == 5);
    CHECK(m.GetInt("missing", 9) == 9);
    CHECK(m.GetInt("empty", 9) == 9);
    CHECK_THAT(m.GetFloat("f"), WithinAbs(1.5, 1e-6));
    CHECK_THAT(m.GetDouble("missing", 2.5), WithinAbs(2.5, 1e-12));
    CHECK(m.GetBool("b"));
    CHECK(m.GetBool("missing", true));
    CHECK(m.Contains("i"));
    CHECK_FALSE(m.Contains("zz"));
    CHECK(m.Get("i", "x") == "5");
    CHECK(m.Get("zz", "x") == "x");
    CHECK(m.size() == 4);
    CHECK(m.erase("i") == 1);
    CHECK_FALSE(m.Contains("i"));
    m.clear();
    CHECK(m.empty());
}

TEST_CASE("SettingsMap Parse and AsString round trip with escapes", "[utils]") {
    SettingsMap m;
    m["E_TEXTCTRL_Text"] = "hello, world & more";
    m["E_SLIDER_Speed"] = "10";
    const std::string s = m.AsString();
    CHECK(s.find("&comma;") != std::string::npos);
    CHECK(s.find("&amp;") != std::string::npos);

    SettingsMap p;
    p.Parse(nullptr, s, "Text");
    CHECK(p.size() == 2);
    CHECK(p.Get("E_TEXTCTRL_Text", "") == "hello, world & more");
    CHECK(p.GetInt("E_SLIDER_Speed") == 10);
}

TEST_CASE("SettingsMap Parse keeps first duplicate and skips empty names", "[utils]") {
    SettingsMap p;
    p.Parse(nullptr, "A=1,A=2,=3,B=4", "Text");
    CHECK(p.GetInt("A") == 1);
    CHECK(p.GetInt("B") == 4);
    CHECK(p.size() == 2);
}

TEST_CASE("SettingsMap AsJSON and ParseJson round trip", "[utils]") {
    SettingsMap m;
    m["A"] = "1";
    m["B"] = "x, y";
    const std::string j = m.AsJSON();
    CHECK(j.front() == '{');
    CHECK(j.back() == '}');
    SettingsMap p;
    p.ParseJson(nullptr, j, "Text");
    CHECK(p.Get("A", "") == "1");
    CHECK(p.Get("B", "") == "x, y");
}

TEST_CASE("SettingsMap copies are independent", "[utils]") {
    SettingsMap a;
    a["k"] = "1";
    SettingsMap b = a;
    b["k"] = "2";
    CHECK(a.GetInt("k") == 1);
    CHECK(b.GetInt("k") == 2);
    SettingsMap c(std::move(b));
    CHECK(c.GetInt("k") == 2);
}

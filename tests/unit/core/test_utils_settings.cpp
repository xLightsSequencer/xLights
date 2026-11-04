#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "settings/XLightsSettings.h"
#include "support/TempDir.h"

using Catch::Matchers::WithinAbs;

namespace {

// XLightsSettings is a process wide singleton, so every test works in its own section and
// clears it afterwards.
struct Section {
    std::string name;
    explicit Section(const char* n) : name(n) { XLightsSettings::Get().DeleteAll(name); }
    ~Section() { XLightsSettings::Get().DeleteAll(name); }
};

bool Contains(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}

} // namespace

TEST_CASE("XLightsSettings is a singleton", "[utils]") {
    CHECK(&XLightsSettings::Get() == &XLightsSettings::Get());
}

TEST_CASE("XLightsSettings string round trip and defaults", "[utils]") {
    Section s("ut_strings");
    auto& cfg = XLightsSettings::Get();
    CHECK(cfg.ReadString("k", "dflt", s.name) == "dflt");
    CHECK_FALSE(cfg.HasEntry("k", s.name));
    cfg.WriteString("k", "hello world", s.name);
    CHECK(cfg.HasEntry("k", s.name));
    CHECK(cfg.ReadString("k", "dflt", s.name) == "hello world");
    cfg.WriteString("k", "", s.name);
    CHECK(cfg.ReadString("k", "dflt", s.name).empty());
    CHECK(cfg.GetValueType("k", s.name) == XLightsSettings::ValueType::String);
}

TEST_CASE("XLightsSettings typed values", "[utils]") {
    Section s("ut_typed");
    auto& cfg = XLightsSettings::Get();
    cfg.WriteBool("b", true, s.name);
    cfg.WriteLong("l", -42, s.name);
    cfg.WriteDouble("d", 2.5, s.name);
    CHECK(cfg.ReadBool("b", false, s.name));
    CHECK(cfg.ReadLong("l", 0, s.name) == -42);
    CHECK_THAT(cfg.ReadDouble("d", 0.0, s.name), WithinAbs(2.5, 1e-12));
    CHECK(cfg.GetValueType("b", s.name) == XLightsSettings::ValueType::Boolean);
    CHECK(cfg.GetValueType("l", s.name) == XLightsSettings::ValueType::Integer);
    CHECK(cfg.GetValueType("d", s.name) == XLightsSettings::ValueType::Float);
    CHECK(cfg.GetValueType("nope", s.name) == XLightsSettings::ValueType::Unknown);
    CHECK(cfg.GetValueType("b", "ut_no_such_section") == XLightsSettings::ValueType::Unknown);
}

TEST_CASE("XLightsSettings missing keys return the supplied defaults", "[utils]") {
    Section s("ut_defaults");
    auto& cfg = XLightsSettings::Get();
    CHECK(cfg.ReadBool("x", true, s.name));
    CHECK_FALSE(cfg.ReadBool("x", false, s.name));
    CHECK(cfg.ReadLong("x", 77, s.name) == 77);
    CHECK_THAT(cfg.ReadDouble("x", 1.25, s.name), WithinAbs(1.25, 1e-12));
    CHECK(cfg.ReadString("x", "d", "ut_no_such_section") == "d");
    CHECK(cfg.ReadLong("x", 5, "ut_no_such_section") == 5);
}

TEST_CASE("XLightsSettings converts between stored types on read", "[utils]") {
    Section s("ut_convert");
    auto& cfg = XLightsSettings::Get();
    cfg.WriteBool("flag", true, s.name);
    CHECK(cfg.ReadString("flag", "", s.name) == "1");
    CHECK(cfg.ReadLong("flag", 9, s.name) == 1);
    cfg.WriteLong("n", 0, s.name);
    CHECK_FALSE(cfg.ReadBool("n", true, s.name));
    cfg.WriteLong("n", 5, s.name);
    CHECK(cfg.ReadBool("n", false, s.name));
    CHECK(cfg.ReadString("n", "", s.name) == "5");
    CHECK_THAT(cfg.ReadDouble("n", 0.0, s.name), WithinAbs(5.0, 1e-12));
    cfg.WriteString("str", "12abc", s.name);
    CHECK(cfg.ReadLong("str", 0, s.name) == 12);
    cfg.WriteString("str", "abc", s.name);
    CHECK(cfg.ReadLong("str", 3, s.name) == 3);
    CHECK_THAT(cfg.ReadDouble("str", 4.5, s.name), WithinAbs(4.5, 1e-12));
    cfg.WriteString("str", "3.25", s.name);
    CHECK_THAT(cfg.ReadDouble("str", 0.0, s.name), WithinAbs(3.25, 1e-12));
}

TEST_CASE("XLightsSettings string to bool accepts the documented spellings", "[utils]") {
    Section s("ut_boolstr");
    auto& cfg = XLightsSettings::Get();
    for (const char* t : { "1", "true", "True", "yes" }) {
        cfg.WriteString("b", t, s.name);
        INFO(t);
        CHECK(cfg.ReadBool("b", false, s.name));
    }
    for (const char* f : { "0", "false", "no", "", "TRUE" }) {
        cfg.WriteString("b", f, s.name);
        INFO(f);
        CHECK_FALSE(cfg.ReadBool("b", true, s.name));
    }
}

TEST_CASE("XLightsSettings sections are independent", "[utils]") {
    Section a("ut_sec_a");
    Section b("ut_sec_b");
    auto& cfg = XLightsSettings::Get();
    cfg.WriteString("k", "A", a.name);
    cfg.WriteString("k", "B", b.name);
    CHECK(cfg.ReadString("k", "", a.name) == "A");
    CHECK(cfg.ReadString("k", "", b.name) == "B");
    cfg.DeleteEntry("k", a.name);
    CHECK_FALSE(cfg.HasEntry("k", a.name));
    CHECK(cfg.HasEntry("k", b.name));
    cfg.DeleteEntry("never existed", a.name);
}

TEST_CASE("XLightsSettings keys and DeleteAll", "[utils]") {
    Section s("ut_keys");
    auto& cfg = XLightsSettings::Get();
    CHECK(cfg.GetAllKeys(s.name).empty());
    cfg.WriteString("one", "1", s.name);
    cfg.WriteLong("two", 2, s.name);
    cfg.WriteBool("three", true, s.name);
    auto keys = cfg.GetAllKeys(s.name);
    CHECK(keys.size() == 3);
    CHECK(Contains(keys, "one"));
    CHECK(Contains(keys, "two"));
    CHECK(Contains(keys, "three"));
    cfg.DeleteAll(s.name);
    CHECK(cfg.GetAllKeys(s.name).empty());
    CHECK(cfg.GetAllKeys("ut_no_such_section").empty());
}

TEST_CASE("XLightsSettings default sections exist after Load", "[utils]") {
    TempDir dir;
    auto& cfg = XLightsSettings::Get();
    cfg.SetFilePath(dir.File("settings.json"));
    cfg.Load();
    CHECK(cfg.ReadString("anything", "d", "tod") == "d");
    CHECK(cfg.ReadString("anything", "d", "bookmarks") == "d");
    cfg.SetFilePath({});
}

TEST_CASE("XLightsSettings Flush and Load persist values", "[utils]") {
    TempDir dir;
    Section s("ut_persist");
    auto& cfg = XLightsSettings::Get();
    const std::string path = dir.File("settings.json");
    cfg.SetFilePath(path);
    cfg.WriteString("name", "value", s.name);
    cfg.WriteLong("count", 12, s.name);
    cfg.WriteBool("flag", true, s.name);
    cfg.WriteDouble("ratio", 0.75, s.name);
    cfg.Flush();
    REQUIRE(std::filesystem::exists(path));
    CHECK_FALSE(std::filesystem::exists(path + ".tmp"));

    cfg.DeleteAll(s.name);
    CHECK_FALSE(cfg.HasEntry("name", s.name));
    cfg.Load();
    CHECK(cfg.ReadString("name", "", s.name) == "value");
    CHECK(cfg.ReadLong("count", 0, s.name) == 12);
    CHECK(cfg.ReadBool("flag", false, s.name));
    CHECK_THAT(cfg.ReadDouble("ratio", 0.0, s.name), WithinAbs(0.75, 1e-12));
    cfg.SetFilePath({});
}

TEST_CASE("XLightsSettings Flush writes nothing when clean or without a path", "[utils]") {
    TempDir dir;
    auto& cfg = XLightsSettings::Get();
    cfg.SetFilePath({});
    cfg.WriteString("k", "v", "ut_nopath");
    cfg.Flush();
    cfg.DeleteAll("ut_nopath");

    cfg.SetFilePath(dir.File("settle.json"));
    cfg.Flush();
    const std::string path = dir.File("clean.json");
    cfg.SetFilePath(path);
    cfg.Flush();
    CHECK_FALSE(std::filesystem::exists(path));
    cfg.SetFilePath({});
}

TEST_CASE("XLightsSettings Load tolerates a corrupt file", "[utils]") {
    TempDir dir;
    auto& cfg = XLightsSettings::Get();
    cfg.SetFilePath(dir.Write("bad.json", "{ this is not json"));
    cfg.Load();
    CHECK(cfg.ReadString("k", "d", "main") == "d");
    cfg.WriteString("k", "after", "ut_corrupt");
    CHECK(cfg.ReadString("k", "", "ut_corrupt") == "after");
    cfg.DeleteAll("ut_corrupt");
    cfg.SetFilePath({});
}

TEST_CASE("XLightsSettings Load reads an existing json document", "[utils]") {
    TempDir dir;
    auto& cfg = XLightsSettings::Get();
    cfg.SetFilePath(dir.Write("s.json", R"({"main":{"a":"x","b":3,"c":true,"d":1.5},"tod":{"t":"1"}})"));
    cfg.Load();
    CHECK(cfg.ReadString("a", "", "main") == "x");
    CHECK(cfg.ReadLong("b", 0, "main") == 3);
    CHECK(cfg.ReadBool("c", false, "main"));
    CHECK_THAT(cfg.ReadDouble("d", 0.0, "main"), WithinAbs(1.5, 1e-12));
    CHECK(cfg.ReadString("t", "", "tod") == "1");
    CHECK(cfg.GetValueType("d", "main") == XLightsSettings::ValueType::Float);
    cfg.SetFilePath({});
}

TEST_CASE("XLightsSettings migration marker", "[utils]") {
    TempDir dir;
    auto& cfg = XLightsSettings::Get();
    cfg.SetFilePath(dir.Write("m.json", "{}"));
    cfg.Load();
    CHECK_FALSE(cfg.WasMigrated());
    cfg.SetMigrated();
    CHECK(cfg.WasMigrated());
    cfg.Flush();
    cfg.Load();
    CHECK(cfg.WasMigrated());
    cfg.SetFilePath({});
}

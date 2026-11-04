#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "settings/XLightsSettings.h"
#include "support/TempDir.h"

using Catch::Matchers::WithinAbs;

namespace {

struct Scratch {
    std::string name;
    explicit Scratch(const char* n) : name(n) { XLightsSettings::Get().DeleteAll(name); }
    ~Scratch() {
        XLightsSettings::Get().DeleteAll(name);
        XLightsSettings::Get().SetFilePath({});
    }
};

std::string ReadAll(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

} // namespace

TEST_CASE("XLightsSettings DeleteEntry removes one key only", "[misc][settings]") {
    Scratch s("ms_delentry");
    auto& c = XLightsSettings::Get();
    c.WriteString("a", "1", s.name);
    c.WriteString("b", "2", s.name);
    c.DeleteEntry("a", s.name);
    CHECK_FALSE(c.HasEntry("a", s.name));
    CHECK(c.HasEntry("b", s.name));
    c.DeleteEntry("missing", s.name);
    c.DeleteEntry("a", "ms_no_such_section");
    CHECK(c.GetAllKeys(s.name).size() == 1);
}

TEST_CASE("XLightsSettings unknown sections read as defaults without being created", "[misc][settings]") {
    auto& c = XLightsSettings::Get();
    CHECK(c.ReadString("k", "d", "ms_never_made") == "d");
    CHECK(c.ReadLong("k", 7, "ms_never_made") == 7);
    CHECK(c.ReadBool("k", true, "ms_never_made"));
    CHECK_THAT(c.ReadDouble("k", 2.5, "ms_never_made"), WithinAbs(2.5, 1e-12));
    CHECK_FALSE(c.HasEntry("k", "ms_never_made"));
    CHECK(c.GetAllKeys("ms_never_made").empty());
    CHECK(c.GetValueType("k", "ms_never_made") == XLightsSettings::ValueType::Unknown);
}

TEST_CASE("XLightsSettings stores extreme numbers", "[misc][settings]") {
    Scratch s("ms_extreme");
    auto& c = XLightsSettings::Get();
    c.WriteLong("max", std::numeric_limits<long>::max(), s.name);
    c.WriteLong("min", std::numeric_limits<long>::min(), s.name);
    c.WriteDouble("big", 1e300, s.name);
    c.WriteDouble("tiny", 1e-300, s.name);
    CHECK(c.ReadLong("max", 0, s.name) == std::numeric_limits<long>::max());
    CHECK(c.ReadLong("min", 0, s.name) == std::numeric_limits<long>::min());
    CHECK(c.ReadDouble("big", 0, s.name) == 1e300);
    CHECK(c.ReadDouble("tiny", 0, s.name) == 1e-300);
}

TEST_CASE("XLightsSettings string values with unusual content survive a save", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_strings");
    auto& c = XLightsSettings::Get();
    const std::string path = t.File("s.json");
    c.SetFilePath(path);
    const std::string weird = "line1\nline2\t\"quoted\" \\ back / slash \xC3\xA9 {}[]";
    c.WriteString("weird", weird, s.name);
    c.WriteString("empty", "", s.name);
    c.WriteString("long", std::string(100000, 'z'), s.name);
    c.Flush();
    c.DeleteAll(s.name);
    c.Load();
    CHECK(c.ReadString("weird", "x", s.name) == weird);
    CHECK(c.HasEntry("empty", s.name));
    CHECK(c.ReadString("empty", "x", s.name).empty());
    CHECK(c.ReadString("long", "", s.name).size() == 100000);
}

TEST_CASE("XLightsSettings keys with odd characters are independent", "[misc][settings]") {
    Scratch s("ms_keys");
    auto& c = XLightsSettings::Get();
    c.WriteString("a/b", "1", s.name);
    c.WriteString("a.b", "2", s.name);
    c.WriteString("", "3", s.name);
    c.WriteString("with space", "4", s.name);
    CHECK(c.ReadString("a/b", "", s.name) == "1");
    CHECK(c.ReadString("a.b", "", s.name) == "2");
    CHECK(c.ReadString("", "", s.name) == "3");
    CHECK(c.ReadString("with space", "", s.name) == "4");
    CHECK(c.GetAllKeys(s.name).size() == 4);
}

TEST_CASE("XLightsSettings reads values written as other json types", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_types");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.Write("t.json", R"({"ms_types":{"n":null,"arr":[1,2],"obj":{"x":1},"neg":-5,"f":2.5,"zero":0,"s":"abc","t":"true","y":"yes","no":"no"}})"));
    c.Load();
    CHECK(c.ReadString("n", "d", s.name) == "d");
    CHECK(c.ReadString("arr", "d", s.name) == "d");
    CHECK(c.ReadString("obj", "d", s.name) == "d");
    CHECK(c.ReadLong("n", 9, s.name) == 9);
    CHECK(c.ReadLong("arr", 9, s.name) == 9);
    CHECK(c.ReadLong("neg", 0, s.name) == -5);
    CHECK(c.ReadLong("f", 9, s.name) == 9);
    CHECK(c.ReadLong("s", 9, s.name) == 9);
    CHECK_THAT(c.ReadDouble("s", 3.5, s.name), WithinAbs(3.5, 1e-12));
    CHECK_THAT(c.ReadDouble("neg", 0, s.name), WithinAbs(-5.0, 1e-12));
    CHECK_FALSE(c.ReadBool("zero", true, s.name));
    CHECK(c.ReadBool("neg", false, s.name));
    CHECK(c.ReadBool("t", false, s.name));
    CHECK(c.ReadBool("y", false, s.name));
    CHECK_FALSE(c.ReadBool("no", true, s.name));
    CHECK(c.ReadBool("f", true, s.name));
    CHECK(c.ReadBool("arr", true, s.name));
    CHECK(c.GetValueType("n", s.name) == XLightsSettings::ValueType::Unknown);
    CHECK(c.GetValueType("arr", s.name) == XLightsSettings::ValueType::Unknown);
    CHECK(c.GetValueType("neg", s.name) == XLightsSettings::ValueType::Integer);
    CHECK(c.GetValueType("s", s.name) == XLightsSettings::ValueType::String);
}

TEST_CASE("XLightsSettings Load of a non object document starts fresh", "[misc][settings]") {
    TempDir t;
    auto& c = XLightsSettings::Get();
    for (const char* body : { "[]", "42", "\"str\"", "null", "", "   ", "{\"main\":", "{\"main\":5}" }) {
        c.SetFilePath(t.Write("x.json", body));
        c.Load();
        INFO(body);
        CHECK(c.ReadString("k", "d", "ms_nonobj") == "d");
        c.WriteString("k", "v", "ms_nonobj");
        CHECK(c.ReadString("k", "", "ms_nonobj") == "v");
        c.DeleteAll("ms_nonobj");
    }
    c.SetFilePath({});
}

TEST_CASE("XLightsSettings Load of a missing file keeps the in-memory values", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_missing");
    auto& c = XLightsSettings::Get();
    c.WriteString("k", "kept", s.name);
    c.SetFilePath(t.File("absent.json"));
    c.Load();
    CHECK(c.ReadString("k", "", s.name) == "kept");
}

TEST_CASE("XLightsSettings Load with no path is harmless", "[misc][settings]") {
    auto& c = XLightsSettings::Get();
    c.SetFilePath({});
    c.Load();
    c.Flush();
    CHECK(c.ReadString("nothing", "d", "main") == "d");
}

TEST_CASE("XLightsSettings Flush produces valid json and leaves no temp file", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_flush");
    auto& c = XLightsSettings::Get();
    const std::string path = t.File("f.json");
    c.SetFilePath(path);
    c.Load();
    c.WriteLong("n", 5, s.name);
    c.Flush();
    REQUIRE(std::filesystem::exists(path));
    CHECK_FALSE(std::filesystem::exists(path + ".tmp"));
    auto j = nlohmann::json::parse(ReadAll(path), nullptr, false);
    REQUIRE_FALSE(j.is_discarded());
    CHECK(j["ms_flush"]["n"] == 5);
    CHECK(j.contains("main"));
    CHECK(j.contains("tod"));
    CHECK(j.contains("bookmarks"));
}

TEST_CASE("XLightsSettings second Flush with no changes does not rewrite the file", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_flush2");
    auto& c = XLightsSettings::Get();
    const std::string path = t.File("f.json");
    c.SetFilePath(path);
    c.WriteBool("b", true, s.name);
    c.Flush();
    std::filesystem::remove(path);
    c.Flush();
    CHECK_FALSE(std::filesystem::exists(path));
    c.WriteBool("b", false, s.name);
    c.Flush();
    CHECK(std::filesystem::exists(path));
}

TEST_CASE("XLightsSettings Flush into a missing folder fails quietly", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_flush3");
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.File("no_dir/f.json"));
    c.WriteBool("b", true, s.name);
    c.Flush();
    CHECK_FALSE(std::filesystem::exists(t.File("no_dir/f.json")));
    CHECK(c.ReadBool("b", false, s.name));
}

TEST_CASE("XLightsSettings Load resets the dirty flag", "[misc][settings]") {
    TempDir t;
    Scratch s("ms_dirty");
    auto& c = XLightsSettings::Get();
    const std::string path = t.Write("d.json", "{}");
    c.SetFilePath(path);
    c.WriteBool("b", true, s.name);
    c.Load();
    std::filesystem::remove(path);
    c.Flush();
    CHECK_FALSE(std::filesystem::exists(path));
}

TEST_CASE("XLightsSettings migration marker is stored outside the user sections", "[misc][settings]") {
    TempDir t;
    auto& c = XLightsSettings::Get();
    const std::string path = t.Write("m.json", "{}");
    c.SetFilePath(path);
    c.Load();
    c.SetMigrated();
    c.Flush();
    auto j = nlohmann::json::parse(ReadAll(path));
    CHECK(j["_meta"]["migrated"] == true);
    c.SetFilePath(t.Write("m2.json", R"({"_meta":{"migrated":"yes"}})"));
    c.Load();
    CHECK_FALSE(c.WasMigrated());
    c.SetFilePath(t.Write("m3.json", R"({"_meta":5})"));
    c.Load();
    CHECK_FALSE(c.WasMigrated());
    c.SetFilePath({});
}

TEST_CASE("XLightsSettings tolerates concurrent readers and a writer", "[misc][settings]") {
    Scratch s("ms_threads");
    auto& c = XLightsSettings::Get();
    c.WriteLong("n", 0, s.name);
    std::vector<std::thread> th;
    for (int i = 0; i < 4; ++i) {
        th.emplace_back([&] {
            for (int k = 0; k < 500; ++k) {
                long v = c.ReadLong("n", -1, s.name);
                if (v < 0) {
                    FAIL_CHECK("lost value");
                    return;
                }
            }
        });
    }
    for (int k = 1; k <= 500; ++k) {
        c.WriteLong("n", k, s.name);
    }
    for (auto& x : th) {
        x.join();
    }
    CHECK(c.ReadLong("n", 0, s.name) == 500);
}

TEST_CASE("XLightsSettings string forms of floats and bools", "[misc][settings]") {
    Scratch s("ms_strform");
    auto& c = XLightsSettings::Get();
    c.WriteBool("b", true, s.name);
    c.WriteBool("f", false, s.name);
    c.WriteLong("l", -12, s.name);
    CHECK(c.ReadString("b", "", s.name) == "1");
    CHECK(c.ReadString("f", "", s.name) == "0");
    CHECK(c.ReadString("l", "", s.name) == "-12");
    c.WriteDouble("d", 0.5, s.name);
    CHECK(c.ReadString("d", "", s.name).rfind("0.5", 0) == 0);
    CHECK(c.ReadLong("b", 9, s.name) == 1);
    CHECK(c.ReadLong("f", 9, s.name) == 0);
}

TEST_CASE("XLightsSettings overwriting changes the stored type", "[misc][settings]") {
    Scratch s("ms_retype");
    auto& c = XLightsSettings::Get();
    c.WriteString("k", "text", s.name);
    CHECK(c.GetValueType("k", s.name) == XLightsSettings::ValueType::String);
    c.WriteLong("k", 3, s.name);
    CHECK(c.GetValueType("k", s.name) == XLightsSettings::ValueType::Integer);
    c.WriteDouble("k", 3.5, s.name);
    CHECK(c.GetValueType("k", s.name) == XLightsSettings::ValueType::Float);
    c.WriteBool("k", true, s.name);
    CHECK(c.GetValueType("k", s.name) == XLightsSettings::ValueType::Boolean);
    CHECK(c.GetAllKeys(s.name).size() == 1);
}

TEST_CASE("XLightsSettings ReadLong parses numeric prefixes of strings", "[misc][settings]") {
    Scratch s("ms_prefix");
    auto& c = XLightsSettings::Get();
    c.WriteString("a", "42abc", s.name);
    c.WriteString("b", "  17", s.name);
    c.WriteString("c", "abc", s.name);
    c.WriteString("d", "", s.name);
    c.WriteString("e", "99999999999999999999999", s.name);
    CHECK(c.ReadLong("a", 0, s.name) == 42);
    CHECK(c.ReadLong("b", 0, s.name) == 17);
    CHECK(c.ReadLong("c", 5, s.name) == 5);
    CHECK(c.ReadLong("d", 5, s.name) == 5);
    CHECK(c.ReadLong("e", 5, s.name) == std::numeric_limits<long>::max());
    CHECK_THAT(c.ReadDouble("a", 0, s.name), WithinAbs(42.0, 1e-12));
}

// A hand-edited or damaged file with a non-object section makes the write path throw json type_error.
TEST_CASE("XLightsSettings writing into a damaged non object section should not throw", "[misc][settings][!mayfail]") {
    TempDir t;
    auto& c = XLightsSettings::Get();
    c.SetFilePath(t.Write("dmg.json", R"({"main":5})"));
    c.Load();
    CHECK_NOTHROW(c.WriteString("k", "v", "main"));
    CHECK_NOTHROW(c.ReadString("k", "d", "main"));
    c.SetFilePath(t.Write("ok.json", "{}"));
    c.Load();
}

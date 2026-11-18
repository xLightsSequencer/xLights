#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/ip_utils.h"

namespace fs = std::filesystem;

namespace {
void MakeFile(const fs::path& p, const std::string& body = "x") {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << body;
}

// Restores the global FixFile configuration so tests stay independent.
struct FixFileScope {
    explicit FixFileScope(const std::string& show, const std::list<std::string>& dirs = {}) {
        FileUtils::SetFixFileShowDir(show);
        FileUtils::SetFixFileDirectories(dirs);
    }
    ~FixFileScope() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
    }
};

bool SamePath(const std::string& a, const fs::path& b) {
    std::error_code ec;
    return fs::equivalent(fs::path(a), b, ec);
}
} // namespace

TEST_CASE("GetFilenameFromPath splits on both separators", "[utils]") {
    CHECK(FileUtils::GetFilenameFromPath("a/b/c.txt") == "c.txt");
    CHECK(FileUtils::GetFilenameFromPath("a\\b\\c.txt") == "c.txt");
    CHECK(FileUtils::GetFilenameFromPath("a/b\\c.txt") == "c.txt");
    CHECK(FileUtils::GetFilenameFromPath("c.txt") == "c.txt");
    CHECK(FileUtils::GetFilenameFromPath("dir/") == "");
    CHECK(FileUtils::GetFilenameFromPath("") == "");
}

TEST_CASE("IsAbsoluteOrRootedPath understands both platforms' forms", "[utils]") {
    CHECK(FileUtils::IsAbsoluteOrRootedPath("/usr/share/x"));
    CHECK(FileUtils::IsAbsoluteOrRootedPath("\\rooted\\x"));
    CHECK(FileUtils::IsAbsoluteOrRootedPath("C:\\Show\\a.png"));
    CHECK(FileUtils::IsAbsoluteOrRootedPath("c:/Show/a.png"));
    CHECK(FileUtils::IsAbsoluteOrRootedPath("D:rel.png"));
    CHECK(FileUtils::IsAbsoluteOrRootedPath("\\\\server\\share\\a.png"));
    CHECK_FALSE(FileUtils::IsAbsoluteOrRootedPath(""));
    CHECK_FALSE(FileUtils::IsAbsoluteOrRootedPath("pics/a.png"));
    CHECK_FALSE(FileUtils::IsAbsoluteOrRootedPath("a.png"));
    CHECK_FALSE(FileUtils::IsAbsoluteOrRootedPath("..\\pics\\a.png"));
}

TEST_CASE("IsValidUTF8 accepts valid and rejects malformed sequences", "[utils]") {
    CHECK(FileUtils::IsValidUTF8(""));
    CHECK(FileUtils::IsValidUTF8("plain ascii"));
    CHECK(FileUtils::IsValidUTF8("caf\xC3\xA9"));
    CHECK(FileUtils::IsValidUTF8("\xE2\x82\xAC"));
    CHECK(FileUtils::IsValidUTF8("\xF0\x9F\x98\x80"));
    CHECK_FALSE(FileUtils::IsValidUTF8("caf\xE9"));
    CHECK_FALSE(FileUtils::IsValidUTF8("\xC0\x80"));
    CHECK_FALSE(FileUtils::IsValidUTF8("\xED\xA0\x80"));
    CHECK_FALSE(FileUtils::IsValidUTF8("\xF4\x90\x80\x80"));
    CHECK_FALSE(FileUtils::IsValidUTF8("\xE2\x82"));
    CHECK_FALSE(FileUtils::IsValidUTF8("\x80"));
}

TEST_CASE("ToValidUTF8 decodes legacy code pages", "[utils]") {
    CHECK(FileUtils::ToValidUTF8("caf\xC3\xA9") == "caf\xC3\xA9");
    CHECK(FileUtils::ToValidUTF8("caf\xE9") == "caf\xC3\xA9");
    CHECK(FileUtils::ToValidUTF8("\x93q\x94") == "\xE2\x80\x9Cq\xE2\x80\x9D");
    CHECK(FileUtils::ToValidUTF8("\x80") == "\xE2\x82\xAC");
    CHECK(FileUtils::ToValidUTF8("\x82", FileUtils::LegacyEncoding::CP437) == "\xC3\xA9");
    CHECK(FileUtils::ToValidUTF8("\xE9", FileUtils::LegacyEncoding::CP437) == "\xCE\x98");
    CHECK(FileUtils::IsValidUTF8(FileUtils::ToValidUTF8("bad\xFF\xFE" "bytes")));
}

TEST_CASE("ToValidUTF8 keeps valid sequences mixed with legacy bytes", "[utils]") {
    const std::string mixed = "a\xC3\xA9" "b\xE9";
    CHECK(FileUtils::ToValidUTF8(mixed) == "a\xC3\xA9" "b\xC3\xA9");
}

TEST_CASE("FixFile returns existing absolute paths untouched", "[utils]") {
    TempDir d;
    const std::string f = d.Write("a.png", "x");
    FixFileScope scope(d.File(""));
    CHECK(FileUtils::FixFile(d.path.string(), f) == f);
    CHECK(FileUtils::FixFile(d.path.string(), "") == "");
}

TEST_CASE("FixFile resolves show relative paths", "[utils]") {
    TempDir d;
    MakeFile(d.path / "pics" / "a.png");
    FixFileScope scope(d.path.string());
    const std::string r = FileUtils::FixFile(d.path.string(), "pics/a.png");
    CHECK(SamePath(r, d.path / "pics" / "a.png"));
    const std::string r2 = FileUtils::FixFile(d.path.string(), "pics\\a.png");
    CHECK(SamePath(r2, d.path / "pics" / "a.png"));
}

TEST_CASE("FixFile finds a moved file by name in the show folder", "[utils]") {
    TempDir d;
    MakeFile(d.path / "a.png");
    FixFileScope scope(d.path.string());
    const std::string r = FileUtils::FixFile(d.path.string(), "C:\\Old\\Place\\a.png");
    CHECK(SamePath(r, d.path / "a.png"));
}

TEST_CASE("FixFile searches immediate subfolders but not Backup", "[utils]") {
    TempDir d;
    MakeFile(d.path / "Images" / "found.png");
    MakeFile(d.path / "Backup" / "hidden.png");
    FixFileScope scope(d.path.string());
    CHECK(SamePath(FileUtils::FixFile(d.path.string(), "/gone/found.png"), d.path / "Images" / "found.png"));
    CHECK(FileUtils::FixFile(d.path.string(), "/gone/hidden.png") == "/gone/hidden.png");
}

TEST_CASE("FixFile uses additional media directories", "[utils]") {
    TempDir show;
    TempDir media;
    MakeFile(media.path / "song.png");
    FixFileScope scope(show.path.string(), { media.path.string() });
    const std::string r = FileUtils::FixFile(show.path.string(), "song.png");
    CHECK(SamePath(r, media.path / "song.png"));
}

TEST_CASE("FixFile returns the input when nothing matches", "[utils]") {
    TempDir d;
    FixFileScope scope(d.path.string());
    CHECK(FileUtils::FixFile(d.path.string(), "nothing/here.png") == "nothing/here.png");
    CHECK(FileUtils::FixFile(d.path.string(), "/definitely/not/here.png") == "/definitely/not/here.png");
}

TEST_CASE("FixFile tolerates invalid UTF-8 names", "[utils]") {
    TempDir d;
    FixFileScope scope(d.path.string());
    const std::string r = FileUtils::FixFile(d.path.string(), "missing\xE9.png");
    CHECK(FileUtils::IsValidUTF8(r));
}

TEST_CASE("MakeRelativeFile strips the show directory", "[utils]") {
    TempDir d;
    FixFileScope scope(d.path.string());
    CHECK(FileUtils::MakeRelativeFile((d.path / "sub" / "a.png").string()) == "sub/a.png");
    CHECK(FileUtils::MakeRelativeFile((d.path / "a.png").string()) == "a.png");
}

TEST_CASE("MakeRelativeFile handles media directories and outsiders", "[utils]") {
    TempDir show;
    TempDir media;
    TempDir other;
    FixFileScope scope(show.path.string(), { media.path.string() });
    CHECK(FileUtils::MakeRelativeFile((media.path / "m" / "b.png").string()) == "m/b.png");
    CHECK(FileUtils::MakeRelativeFile((other.path / "c.png").string()).empty());
    const std::string outside = (other.path / "c.png").string();
    CHECK(FileUtils::MakeRelativeFileOrOriginal(outside) == outside);
    CHECK(FileUtils::MakeRelativeFileOrOriginal((show.path / "c.png").string()) == "c.png");
}

TEST_CASE("MakeRelativeFile ignores relative and empty input", "[utils]") {
    TempDir d;
    FixFileScope scope(d.path.string());
    CHECK(FileUtils::MakeRelativeFile("").empty());
    CHECK(FileUtils::MakeRelativeFile("already/relative.png").empty());
    CHECK(FileUtils::MakeRelativeFileOrOriginal("already/relative.png") == "already/relative.png");
}

TEST_CASE("MakeRelativeFile does not match a sibling with the same prefix", "[utils]") {
    TempDir d;
    const fs::path show = d.path / "show";
    const fs::path sibling = d.path / "show2";
    FixFileScope scope(show.string());
    CHECK(FileUtils::MakeRelativeFile((sibling / "a.png").string()).empty());
}

TEST_CASE("CachedFileExists caches until cleared", "[utils]") {
    TempDir d;
    FixFileScope scope(d.path.string());
    const std::string f = d.File("late.txt");
    CHECK_FALSE(FileUtils::CachedFileExists(f));
    MakeFile(f);
    CHECK_FALSE(FileUtils::CachedFileExists(f));
    FileUtils::ClearNonExistentFiles();
    CHECK(FileUtils::CachedFileExists(f));
    CHECK_FALSE(FileUtils::CachedFileExists(""));
}

TEST_CASE("IsFileInShowDir distinguishes show files from others", "[utils]") {
    TempDir show;
    TempDir other;
    MakeFile(show.path / "in.png");
    MakeFile(other.path / "out.png");
    FixFileScope scope(show.path.string());
    CHECK(FileUtils::IsFileInShowDir(show.path.string(), (show.path / "in.png").string()));
    CHECK_FALSE(FileUtils::IsFileInShowDir(show.path.string(), (other.path / "out.png").string()));
    CHECK(FileUtils::IsFileInShowDir("", (show.path / "in.png").string()));
}

TEST_CASE("IsFileInShowDir counts media directories", "[utils]") {
    TempDir show;
    TempDir media;
    MakeFile(media.path / "m.png");
    FixFileScope scope(show.path.string(), { media.path.string() });
    CHECK(FileUtils::IsFileInShowDir(show.path.string(), (media.path / "m.png").string()));
}

TEST_CASE("FixEffectFileParameter rewrites only the named parameter", "[utils]") {
    TempDir d;
    MakeFile(d.path / "pics" / "a.png");
    FixFileScope scope(d.path.string());
    const std::string in = "E_X=1,E_FILEPICKER_File=C:\\gone\\a.png,E_Y=2";
    const std::string out = FileUtils::FixEffectFileParameter("E_FILEPICKER_File", in, d.path.string());
    CHECK(out.rfind("E_X=1,E_FILEPICKER_File=", 0) == 0);
    CHECK(out.find((fs::path("pics") / "a.png").string()) != std::string::npos);
    CHECK(out.substr(out.size() - 6) == ",E_Y=2");

    CHECK(FileUtils::FixEffectFileParameter("E_NOPE", in, d.path.string()) == in);
    CHECK(FileUtils::FixEffectFileParameter("E_X", "E_X", d.path.string()) == "E_X");
}

TEST_CASE("DeleteDirectory removes a tree and reports a missing one", "[utils]") {
    TempDir d;
    MakeFile(d.path / "victim" / "deep" / "f.txt");
    const std::string victim = (d.path / "victim").string();
    CHECK(FileUtils::DeleteDirectory(victim));
    CHECK_FALSE(fs::exists(d.path / "victim"));
    CHECK_FALSE(FileUtils::DeleteDirectory(victim));
}

TEST_CASE("GetFileModTimeTicks reports recent mtime or nothing", "[utils]") {
    TempDir d;
    const std::string f = d.Write("t.txt", "x");
    auto t = FileUtils::GetFileModTimeTicks(f);
    REQUIRE(t.has_value());
    const long long now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    CHECK(*t > now - 120);
    CHECK(*t < now + 120);
    CHECK_FALSE(FileUtils::GetFileModTimeTicks(d.File("missing.txt")).has_value());
}

TEST_CASE("NeedsBaseFileUpdate compares mtime against the sync checkpoint", "[utils]") {
    TempDir d;
    const std::string f = d.Write("base.txt", "x");
    CHECK(FileUtils::NeedsBaseFileUpdate(d.File("missing.txt"), "123", "merge"));
    CHECK(FileUtils::NeedsBaseFileUpdate(f, "", "merge"));
    CHECK(FileUtils::NeedsBaseFileUpdate(f, "notanumber", "merge"));
    CHECK(FileUtils::NeedsBaseFileUpdate(f, "12abc", "merge"));
    CHECK(FileUtils::NeedsBaseFileUpdate(f, "0", "merge"));
    CHECK_FALSE(FileUtils::NeedsBaseFileUpdate(f, "99999999999", "merge"));
}

TEST_CASE("Resources dir round trips", "[utils]") {
    const std::string prev = FileUtils::GetResourcesDir();
    FileUtils::SetResourcesDir("some/dir");
    CHECK(FileUtils::GetResourcesDir() == "some/dir");
    FileUtils::SetResourcesDir(prev);
    CHECK(FileUtils::GetResourcesDir() == prev);
}

TEST_CASE("IsIPValid accepts dotted quads only", "[utils]") {
    CHECK(ip_utils::IsIPValid("192.168.1.1"));
    CHECK(ip_utils::IsIPValid("0.0.0.0"));
    CHECK(ip_utils::IsIPValid("255.255.255.255"));
    CHECK(ip_utils::IsIPValid(" 10.0.0.1 "));
    CHECK_FALSE(ip_utils::IsIPValid(""));
    CHECK_FALSE(ip_utils::IsIPValid("   "));
    CHECK_FALSE(ip_utils::IsIPValid("256.1.1.1"));
    CHECK_FALSE(ip_utils::IsIPValid("1.2.3"));
    CHECK_FALSE(ip_utils::IsIPValid("1.2.3.4.5"));
    CHECK_FALSE(ip_utils::IsIPValid("host"));
    CHECK_FALSE(ip_utils::IsIPValid("::1"));
    CHECK_FALSE(ip_utils::IsIPValid("1.2.3.-4"));
}

TEST_CASE("IsIPValidOrHostname honours the iponly flag", "[utils]") {
    CHECK(ip_utils::IsIPValidOrHostname("10.1.1.1"));
    CHECK(ip_utils::IsIPValidOrHostname("falcon"));
    CHECK(ip_utils::IsIPValidOrHostname("falcon.local"));
    CHECK(ip_utils::IsIPValidOrHostname("my-ctrl"));
    CHECK_FALSE(ip_utils::IsIPValidOrHostname("my host"));
    CHECK_FALSE(ip_utils::IsIPValidOrHostname(""));
    CHECK_FALSE(ip_utils::IsIPValidOrHostname("bad_name!"));
    CHECK(ip_utils::IsIPValidOrHostname("10.1.1.1", true));
    CHECK_FALSE(ip_utils::IsIPValidOrHostname("falcon", true));
}

TEST_CASE("IsValidHostname", "[utils]") {
    CHECK(ip_utils::IsValidHostname("controller1"));
    CHECK(ip_utils::IsValidHostname(" controller1 "));
    CHECK(ip_utils::IsValidHostname("a-b.cd"));
    CHECK_FALSE(ip_utils::IsValidHostname(""));
    CHECK_FALSE(ip_utils::IsValidHostname("a"));
    CHECK_FALSE(ip_utils::IsValidHostname("has space"));
    CHECK_FALSE(ip_utils::IsValidHostname("under_score"));
}

TEST_CASE("IsValidHostname accepts multi label names", "[utils]") {
    CHECK(ip_utils::IsValidHostname("www.example.com"));
    CHECK(ip_utils::IsIPValidOrHostname("falcon.home.lan"));
    CHECK_FALSE(ip_utils::IsIPValidOrHostname("falcon.home.lan", true));
    CHECK_FALSE(ip_utils::IsValidHostname("-bad.example.com"));
    CHECK_FALSE(ip_utils::IsValidHostname("bad-.example.com"));
    CHECK_FALSE(ip_utils::IsValidHostname("example..com"));
    CHECK_FALSE(ip_utils::IsValidHostname(".example.com"));
    CHECK_FALSE(ip_utils::IsValidHostname("example.com."));
    CHECK_FALSE(ip_utils::IsValidHostname("300.1.1.1"));
}

TEST_CASE("IsIPv6", "[utils]") {
    CHECK(ip_utils::IsIPv6("::1"));
    CHECK(ip_utils::IsIPv6("fe80::1"));
    CHECK(ip_utils::IsIPv6("2001:db8:0:0:0:0:0:1"));
    CHECK_FALSE(ip_utils::IsIPv6("1.2.3.4"));
    CHECK_FALSE(ip_utils::IsIPv6(""));
    CHECK_FALSE(ip_utils::IsIPv6("gggg::1"));
    CHECK_FALSE(ip_utils::IsIPv6("host:name"));
}

TEST_CASE("CleanupIP strips leading zeros from dotted quads only", "[utils]") {
    CHECK(ip_utils::CleanupIP("192.168.001.010") == "192.168.1.10");
    CHECK(ip_utils::CleanupIP("010.000.000.001") == "10.0.0.1");
    CHECK(ip_utils::CleanupIP("10.0.0.1") == "10.0.0.1");
    CHECK(ip_utils::CleanupIP("falcon") == "falcon");
    CHECK(ip_utils::CleanupIP("falcon.local") == "falcon.local");
    CHECK(ip_utils::CleanupIP("my-host") == "my-host");
    CHECK(ip_utils::CleanupIP("10.0.0") == "10.0.0");
    CHECK(ip_utils::CleanupIP("300.1.1.1") == "300.1.1.1");
    CHECK(ip_utils::CleanupIP("1234") == "1234");
    CHECK(ip_utils::CleanupIP("") == "");
}

TEST_CASE("ResolveIP leaves literals and partial entries alone", "[utils]") {
    CHECK(ip_utils::ResolveIP("10.1.2.3") == "10.1.2.3");
    CHECK(ip_utils::ResolveIP("MULTICAST") == "MULTICAST");
    CHECK(ip_utils::ResolveIP("") == "");
    CHECK(ip_utils::ResolveIP(".5") == ".5");
    CHECK(ip_utils::ResolveIP("192.168") == "192.168");
}

TEST_CASE("ResolveIP with callback answers synchronously for literals", "[utils]") {
    std::string got;
    int calls = 0;
    ip_utils::ResolveIP("10.9.8.7", [&](const std::string& s) {
        got = s;
        ++calls;
    });
    CHECK(calls == 1);
    CHECK(got == "10.9.8.7");
    ip_utils::ResolveIP("MULTICAST", [&](const std::string& s) {
        got = s;
        ++calls;
    });
    CHECK(calls == 2);
    CHECK(got == "MULTICAST");
}

TEST_CASE("IsInSameSubnet applies the mask", "[utils]") {
    CHECK(ip_utils::IsInSameSubnet("192.168.1.5", "192.168.1.200"));
    CHECK_FALSE(ip_utils::IsInSameSubnet("192.168.1.5", "192.168.2.5"));
    CHECK(ip_utils::IsInSameSubnet("192.168.1.5", "192.168.2.5", "255.255.0.0"));
    CHECK_FALSE(ip_utils::IsInSameSubnet("10.0.0.1", "11.0.0.1", "255.0.0.0"));
    CHECK(ip_utils::IsInSameSubnet("10.0.0.1", "10.255.255.255", "255.0.0.0"));
    CHECK(ip_utils::IsInSameSubnet("1.2.3.4", "9.9.9.9", "0.0.0.0"));
    CHECK_FALSE(ip_utils::IsInSameSubnet("", "1.2.3.4"));
    CHECK_FALSE(ip_utils::IsInSameSubnet("1.2.3.4", ""));
    CHECK_FALSE(ip_utils::IsInSameSubnet("1.2.3.4", "1.2.3.4", ""));
    CHECK_FALSE(ip_utils::IsInSameSubnet("bad", "1.2.3.4"));
    CHECK_FALSE(ip_utils::IsInSameSubnet("1.2.3.4", "1.2.3.4", "bad"));
}

TEST_CASE("IsValidLocalIP rejects junk and local IPs are never 0.0.0.0", "[utils]") {
    CHECK_FALSE(ip_utils::IsValidLocalIP("not an ip"));
    CHECK_FALSE(ip_utils::IsValidLocalIP(""));
    for (const auto& ip : ip_utils::GetLocalIPs()) {
        CHECK(ip != "0.0.0.0");
        CHECK(ip_utils::IsIPValid(ip));
    }
}

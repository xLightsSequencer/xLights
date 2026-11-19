#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "effects/EffectPresetManager.h"
#include "outputs/OutputManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "support/TempDir.h"
#include "utils/ShowFileGuard.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <fcntl.h>
#include <sys/xattr.h>
#endif

namespace {
std::string ReadAll(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

int CountEntries(const std::filesystem::path& dir) {
    int n = 0;
    for (const auto& e : std::filesystem::directory_iterator(dir)) {
        (void)e;
        ++n;
    }
    return n;
}
} // namespace

TEST_CASE("ShowFileGuard reports no conflict for an unchanged file", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string path = dir.Write("xlights_rgbeffects.xml", "<xrgb><models/></xrgb>");
    guard.RecordFromDisk(path);
    CHECK(guard.IsTracked(path));
    CHECK_FALSE(guard.HasChangedOnDisk(path));
    CHECK_FALSE(guard.HasChangedOnDisk(path));
}

TEST_CASE("ShowFileGuard flags an external rewrite even when the size is unchanged", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string path = dir.Write("xlights_networks.xml", "<Networks a=\"1\"/>");
    guard.RecordFromDisk(path);
    dir.Write("xlights_networks.xml", "<Networks a=\"2\"/>");
    CHECK(guard.HasChangedOnDisk(path));

    dir.Write("xlights_networks.xml", "<Networks a=\"22\"/>");
    CHECK(guard.HasChangedOnDisk(path));
}

TEST_CASE("ShowFileGuard ignores files it never recorded", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string path = dir.Write("a.xsq", "x");
    CHECK_FALSE(guard.IsTracked(path));
    CHECK_FALSE(guard.HasChangedOnDisk(path));
    CHECK_FALSE(guard.HasChangedOnDisk(dir.File("never.xsq")));
}

TEST_CASE("ShowFileGuard treats delete and create as changes only against what was recorded", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;

    const std::string loaded = dir.Write("loaded.xml", "abc");
    guard.RecordFromDisk(loaded);
    std::filesystem::remove(loaded);
    CHECK(guard.HasChangedOnDisk(loaded));

    const std::string absent = dir.File("absent.xml");
    guard.RecordFromDisk(absent);
    CHECK(guard.IsTracked(absent));
    CHECK_FALSE(guard.HasChangedOnDisk(absent));
    dir.Write("absent.xml", "someone else created it");
    CHECK(guard.HasChangedOnDisk(absent));

    // An empty file is not the same as no file.
    const std::string empty = dir.File("empty.xml");
    guard.RecordFromDisk(empty);
    dir.Write("empty.xml", "");
    CHECK(guard.HasChangedOnDisk(empty));
}

TEST_CASE("ShowFileGuard SaveFile records our own write", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string path = dir.Write("presets.json", "{}");
    guard.RecordFromDisk(path);

    REQUIRE(guard.SaveFile(path, "{\"version\":\"0009\"}"));
    CHECK(ReadAll(path) == "{\"version\":\"0009\"}");
    CHECK_FALSE(guard.HasChangedOnDisk(path));

    // A save over a file someone else changed replaces it and is then current.
    dir.Write("presets.json", "{\"theirs\":1}");
    CHECK(guard.HasChangedOnDisk(path));
    REQUIRE(guard.SaveFile(path, "{\"mine\":1}"));
    CHECK_FALSE(guard.HasChangedOnDisk(path));

    // Streamed writer form.
    REQUIRE(guard.SaveFile(path, ShowFileGuard::Writer([](const std::string& p) {
        std::ofstream(p, std::ios::binary) << "streamed";
        return true;
    })));
    CHECK(ReadAll(path) == "streamed");
    CHECK_FALSE(guard.HasChangedOnDisk(path));
}

TEST_CASE("ShowFileGuard streamed save records what it wrote and not a file that replaced it right after", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string path = dir.Write("xlights_networks.xml", "<Networks/>");
    guard.RecordFromDisk(path);
    // Another machine's save lands between ours and the guard recording it.
    guard.SetAfterSaveHook([&](const std::string&) { dir.Write("xlights_networks.xml", "<Networks theirs=\"1\"/>"); });
    REQUIRE(guard.SaveFile(path, ShowFileGuard::Writer([](const std::string& p) {
        std::ofstream(p, std::ios::binary) << "<Networks mine=\"1\"/>";
        return true;
    })));
    guard.SetAfterSaveHook(nullptr);
    auto base = guard.RecordedText(path);
    REQUIRE(base.has_value());
    CHECK(*base == "<Networks mine=\"1\"/>");
    // Their write is seen as a change, so the next save merges or asks instead of overwriting it.
    CHECK(guard.HasChangedOnDisk(path));
}

TEST_CASE("ShowFileGuard SavePatchedFile keeps an outstanding external change flagged", "[utils][showfileguard]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string path = dir.Write("xlights_rgbeffects.xml", "<xrgb/>");
    guard.RecordFromDisk(path);

    REQUIRE(guard.SavePatchedFile(path, "<xrgb><Viewpoints/></xrgb>"));
    CHECK_FALSE(guard.HasChangedOnDisk(path));

    dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"theirs\"/></models></xrgb>");
    REQUIRE(guard.SavePatchedFile(path, "<xrgb><models><model name=\"theirs\"/></models><Viewpoints/></xrgb>"));
    CHECK(guard.HasChangedOnDisk(path));
}

TEST_CASE("ShowFileGuard atomic write leaves the right content and no temp file", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.Write("xlights_rgbeffects.xml", std::string(100000, 'o'));
    REQUIRE(ShowFileGuard::WriteFileAtomically(path, "new contents"));
    CHECK(ReadAll(path) == "new contents");
    CHECK(CountEntries(dir.path) == 1);

    // A file that does not exist yet is created.
    const std::string fresh = dir.File("fresh.xml");
    REQUIRE(ShowFileGuard::WriteFileAtomically(fresh, "made"));
    CHECK(ReadAll(fresh) == "made");
    CHECK(CountEntries(dir.path) == 2);
}

TEST_CASE("ShowFileGuard failed write leaves the existing file and no temp file", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.Write("seq.xsq", "original");
    CHECK_FALSE(ShowFileGuard::WriteFileAtomically(path, ShowFileGuard::Writer([](const std::string& p) {
        std::ofstream(p, std::ios::binary) << "half";
        return false;
    })));
    CHECK(ReadAll(path) == "original");
    CHECK(CountEntries(dir.path) == 1);

    CHECK_FALSE(ShowFileGuard::WriteFileAtomically(dir.File("no/such/dir/x.xml"), "x"));
}

TEST_CASE("ShowFileGuard stages the new copy outside the show folder or under a sync-ignored name", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.Write("xlights_networks.xml", "old");
    std::string staged;
    REQUIRE(ShowFileGuard::WriteFileAtomically(path, ShowFileGuard::Writer([&](const std::string& p) {
        staged = p;
        std::ofstream(p, std::ios::binary) << "new";
        return true;
    })));
    CHECK(ReadAll(path) == "new");
    CHECK(CountEntries(dir.path) == 1);
    const std::filesystem::path sp(staged);
#ifdef __APPLE__
    // Apple: a replacement directory on the same volume, removed afterwards.
    CHECK(sp.parent_path() != dir.path);
    CHECK_FALSE(std::filesystem::exists(sp.parent_path()));
#else
    CHECK(sp.parent_path() == dir.path);
    CHECK(ShowFileGuard::IsTempFileName(sp.filename().string()));
#endif
}

TEST_CASE("ShowFileGuard temp names are recognised and sync-ignored", "[utils][showfileguard]") {
    TempDir dir;
    const std::filesystem::path tmp(ShowFileGuard::TempPathFor(dir.File("xlights_rgbeffects.xml")));
    CHECK(tmp.parent_path() == dir.path);
    const std::string name = tmp.filename().string();
    CHECK(name.rfind(".~xlights_rgbeffects.xml.xl", 0) == 0);
    CHECK(ShowFileGuard::IsTempFileName(name));
    CHECK(ShowFileGuard::TempPathFor(dir.File("a.xsq")) != ShowFileGuard::TempPathFor(dir.File("a.xsq")));

    CHECK(ShowFileGuard::IsTempFileName(".~song.xsq.xl0123456789abcdef.tmp"));
    CHECK(ShowFileGuard::IsTempFileName(".~song.xsq.xl0123456789abcdef.nosync"));
    CHECK_FALSE(ShowFileGuard::IsTempFileName("song.xsq"));
    CHECK_FALSE(ShowFileGuard::IsTempFileName(".~lock.song.xsq#"));
    CHECK_FALSE(ShowFileGuard::IsTempFileName(".~song.xsq.tmp"));
    CHECK_FALSE(ShowFileGuard::IsTempFileName(".~song.xsq.xl0123456789abcdeg.tmp"));
    CHECK_FALSE(ShowFileGuard::IsTempFileName("~song.xsq.xl0123456789abcdef.tmp"));
}

TEST_CASE("ShowFileGuard sweeps stale temp files and nothing else", "[utils][showfileguard]") {
    TempDir dir;
    const auto old = std::filesystem::file_time_type::clock::now() - std::chrono::hours(3);
    const std::string stale1 = ShowFileGuard::TempPathFor(dir.File("xlights_rgbeffects.xml"));
    const std::string stale2 = dir.File(".~song.xsq.xl0123456789abcdef.nosync");
    const std::string fresh = ShowFileGuard::TempPathFor(dir.File("xlights_networks.xml"));
    const std::string unrelatedOld = dir.Write("song.xsq.tmp", "keep");
    const std::string lookalike = dir.Write(".~lock.song.xsq#", "keep");
    std::ofstream(stale1) << "x";
    std::ofstream(stale2) << "x";
    std::ofstream(fresh) << "x";
    std::filesystem::create_directories(dir.path / "sub");
    const std::string nested = ShowFileGuard::TempPathFor((dir.path / "sub" / "a.xsq").string());
    std::ofstream(nested) << "x";
    for (const auto& p : { stale1, stale2, unrelatedOld, lookalike, nested }) {
        std::filesystem::last_write_time(p, old);
    }

    CHECK(ShowFileGuard::RemoveStaleTempFiles(dir.path.string()) == 2);
    CHECK_FALSE(std::filesystem::exists(stale1));
    CHECK_FALSE(std::filesystem::exists(stale2));
    CHECK(std::filesystem::exists(fresh));
    CHECK(std::filesystem::exists(unrelatedOld));
    CHECK(std::filesystem::exists(lookalike));
    CHECK(std::filesystem::exists(nested));
    CHECK(ShowFileGuard::RemoveStaleTempFiles(dir.File("missing")) == 0);
}

TEST_CASE("ShowFileGuard opening a sequence sweeps stale temps beside it", "[utils][showfileguard]") {
    TempDir dir;
    std::filesystem::create_directories(dir.path / "sub");
    const std::string path = (dir.path / "sub" / "song.xsq").string();
    std::ofstream(path, std::ios::binary) << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\"><head><version>2024.01</version><sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>10.000</sequenceDuration></head></xsequence>";
    const std::string stale = ShowFileGuard::TempPathFor(path);
    std::ofstream(stale) << "x";
    std::filesystem::last_write_time(stale, std::filesystem::file_time_type::clock::now() - std::chrono::hours(2));
    SequenceFile file(path);
    REQUIRE(file.Open(dir.path.string(), true, path).has_value());
    CHECK_FALSE(std::filesystem::exists(stale));
}

TEST_CASE("ShowFileGuard keeps a hard-linked file linked", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.Write("xlights_rgbeffects.xml", "old");
    const std::string other = dir.File("linked.xml");
    std::filesystem::create_hard_link(path, other);
    REQUIRE(ShowFileGuard::WriteFileAtomically(path, "new"));
    CHECK(ReadAll(path) == "new");
    CHECK(ReadAll(other) == "new");
    CHECK(std::filesystem::hard_link_count(path) == 2);
    CHECK(CountEntries(dir.path) == 2);
}

#ifndef _WIN32
TEST_CASE("ShowFileGuard keeps the file's permissions", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.Write("xlights_rgbeffects.xml", "old");
    REQUIRE(::chmod(path.c_str(), 0640) == 0);
    struct stat before;
    REQUIRE(::stat(path.c_str(), &before) == 0);
    REQUIRE(ShowFileGuard::WriteFileAtomically(path, "new"));
    struct stat after;
    REQUIRE(::stat(path.c_str(), &after) == 0);
    CHECK((after.st_mode & 07777) == 0640);
    CHECK(after.st_uid == before.st_uid);
    CHECK(after.st_gid == before.st_gid);
    CHECK(ReadAll(path) == "new");
}
#endif

#ifdef __APPLE__
TEST_CASE("ShowFileGuard keeps extended attributes, tags and creation date on Apple", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.Write("song.xsq", "old");
    REQUIRE(::setxattr(path.c_str(), "org.xlights.unit-test", "kept", 4, 0, 0) == 0);
    // A Finder "Red" tag, as Finder writes it.
    static const unsigned char tagPlist[] = { 0x62, 0x70, 0x6c, 0x69, 0x73, 0x74, 0x30, 0x30, 0xa1, 0x01, 0x55, 0x52, 0x65, 0x64, 0x0a, 0x36,
                                              0x08, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10 };
    REQUIRE(::setxattr(path.c_str(), "com.apple.metadata:_kMDItemUserTags", tagPlist, sizeof(tagPlist), 0, 0) == 0);
    struct timespec times[2] = {};
    times[0].tv_nsec = UTIME_OMIT;
    times[1].tv_sec = 1000000000;
    REQUIRE(::utimensat(AT_FDCWD, path.c_str(), times, 0) == 0); // moves the birth time back too
    struct stat before;
    REQUIRE(::stat(path.c_str(), &before) == 0);
    REQUIRE(before.st_birthtimespec.tv_sec == 1000000000);

    REQUIRE(ShowFileGuard::WriteFileAtomically(path, std::string("new contents")));
    CHECK(ReadAll(path) == "new contents");

    char buf[16] = {};
    CHECK(::getxattr(path.c_str(), "org.xlights.unit-test", buf, sizeof(buf), 0, 0) == 4);
    CHECK(std::string(buf) == "kept");
    CHECK(::getxattr(path.c_str(), "com.apple.metadata:_kMDItemUserTags", nullptr, 0, 0, 0) == (ssize_t)sizeof(tagPlist));
    struct stat after;
    REQUIRE(::stat(path.c_str(), &after) == 0);
    CHECK(after.st_birthtimespec.tv_sec == 1000000000);
    CHECK(CountEntries(dir.path) == 1);
}
#endif

#ifndef _WIN32
TEST_CASE("ShowFileGuard atomic write goes through a symlink", "[utils][showfileguard]") {
    TempDir dir;
    const std::string real = dir.Write("real.xml", "old");
    const std::string link = dir.File("link.xml");
    std::filesystem::create_symlink(real, link);
    REQUIRE(ShowFileGuard::WriteFileAtomically(link, "new"));
    CHECK(std::filesystem::is_symlink(link));
    CHECK(ReadAll(real) == "new");
}
#endif

TEST_CASE("ShowFileGuard copy path is timestamped and unique", "[utils][showfileguard]") {
    TempDir dir;
    const std::string path = dir.File("xlights_rgbeffects.xml");
    std::tm tmv{};
    tmv.tm_year = 2026 - 1900;
    tmv.tm_mon = 10;
    tmv.tm_mday = 18;
    tmv.tm_hour = 14;
    tmv.tm_min = 25;
    tmv.tm_sec = 30;
    tmv.tm_isdst = -1;
    const std::time_t when = std::mktime(&tmv);

    const std::string first = ShowFileGuard::MakeCopyPath(path, when);
    CHECK(std::filesystem::path(first).filename().string() == "xlights_rgbeffects_mine_20261118_142530.xml");
    CHECK(std::filesystem::path(first).parent_path() == dir.path);

    dir.Write("xlights_rgbeffects_mine_20261118_142530.xml", "x");
    const std::string second = ShowFileGuard::MakeCopyPath(path, when);
    CHECK(std::filesystem::path(second).filename().string() == "xlights_rgbeffects_mine_20261118_142530_2.xml");
}

TEST_CASE("ShowFileGuard fingerprint does not depend on how the bytes are chunked", "[utils][showfileguard]") {
    TempDir dir;
    std::string body;
    for (int i = 0; i < 3000001; ++i) {
        body.push_back((char)(i * 31 + (i >> 7)));
    }
    const std::string path = dir.Write("big.xsq", body);
    auto fromDisk = ShowFileGuard::FingerprintFile(path);
    REQUIRE(fromDisk.has_value());
    CHECK(*fromDisk == ShowFileGuard::FingerprintOf(body));
    CHECK(fromDisk->size == body.size());

    std::string other = body;
    other[1500000] ^= 1;
    CHECK_FALSE(ShowFileGuard::FingerprintOf(other) == ShowFileGuard::FingerprintOf(body));
}

TEST_CASE("Show files loaded and saved through the core are fingerprinted", "[utils][showfileguard]") {
    TempDir dir;
    auto& guard = ShowFileGuard::Instance();

    SECTION("controllers") {
        {
            OutputManager om;
            om.SetShowDir(dir.path.string());
            REQUIRE(om.Save());
        }
        OutputManager om;
        REQUIRE(om.Load(dir.path.string()));
        const std::string path = om.GetNetworksFilePath();
        CHECK_FALSE(guard.HasChangedOnDisk(path));
        dir.Write("xlights_networks.xml", "<Networks/>");
        CHECK(guard.HasChangedOnDisk(path));
        REQUIRE(om.Save());
        CHECK_FALSE(guard.HasChangedOnDisk(path));

        const std::string copy = dir.File("copy.xml");
        dir.Write("xlights_networks.xml", "<Networks theirs=\"1\"/>");
        REQUIRE(om.SaveCopyTo(copy));
        CHECK(guard.HasChangedOnDisk(path));
        CHECK(ReadAll(path) == "<Networks theirs=\"1\"/>");
        CHECK_FALSE(ReadAll(copy).empty());
    }

    SECTION("presets") {
        const std::string path = dir.File("xlights_effectpresets.json");
        EffectPresetManager m;
        m.SetVersion("0009");
        CHECK_FALSE(m.LoadJsonFile(path));
        CHECK(guard.IsTracked(path));
        dir.Write("xlights_effectpresets.json", "{\"version\":\"0009\",\"children\":[]}");
        CHECK(guard.HasChangedOnDisk(path));
        REQUIRE(m.LoadJsonFile(path));
        CHECK_FALSE(guard.HasChangedOnDisk(path));
        m.AddPreset(nullptr, "P", "On\ta\tb\t0\t100", "0009", "2026.01");
        REQUIRE(m.SaveJsonFile(path));
        CHECK_FALSE(guard.HasChangedOnDisk(path));
    }

    SECTION("sequence") {
        const std::string path = dir.Write("song.xsq",
                                           "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xsequence BaseChannel=\"0\" ChanCtrlBasic=\"0\" ChanCtrlColor=\"0\" FixedPointTiming=\"1\" ModelBlending=\"true\">"
                                           "<head><version>2024.01</version><sequenceTiming>50 ms</sequenceTiming><sequenceType>Animation</sequenceType><sequenceDuration>10.000</sequenceDuration></head></xsequence>");
        HeadlessRenderContext ctx;
        ctx.showDirectory = dir.path.string();
        auto file = std::make_unique<SequenceFile>(path);
        auto doc = file->Open(dir.path.string(), true, path);
        REQUIRE(doc.has_value());
        REQUIRE(ctx.LoadSequenceElements(*file, *doc));
        CHECK_FALSE(guard.HasChangedOnDisk(path));

        std::ofstream(path, std::ios::binary | std::ios::app) << "<!-- edited elsewhere -->";
        CHECK(guard.HasChangedOnDisk(path));

        REQUIRE(file->Save(ctx.GetSequenceElements()));
        CHECK_FALSE(guard.HasChangedOnDisk(path));
        CHECK(CountEntries(dir.path) == 1);
    }
}

#include <catch2/catch_test_macros.hpp>

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

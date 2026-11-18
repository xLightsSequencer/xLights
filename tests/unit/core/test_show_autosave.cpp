#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "support/TempDir.h"
#include "utils/ShowAutosave.h"
#include "utils/ShowFileGuard.h"

namespace {
const std::string kMine = "11111111-2222-4333-8444-555555555555";
const std::string kOther = "aaaaaaaa-bbbb-4ccc-8ddd-eeeeeeeeeeee";

std::string ReadAll(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

bool Exists(const std::string& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

// Moves the file's modification time relative to now.
void SetAge(const std::string& path, int secondsAgo) {
    auto t = std::filesystem::file_time_type::clock::now() - std::chrono::seconds(secondsAgo);
    std::filesystem::last_write_time(path, t);
}

ShowAutosaveInfo Identity(const std::string& id) {
    ShowAutosaveInfo i;
    i.installId = id;
    i.deviceName = "test device";
    i.appVersion = "1.2.3";
    return i;
}

// A show file loaded by this process, edited in memory, and autosaved.
struct Scenario {
    TempDir dir;
    ShowFileGuard guard;
    std::string real;
    std::string backup;

    explicit Scenario(const std::string& writer = kMine) {
        real = dir.Write("xlights_rgbeffects.xml", "<xrgb>original</xrgb>");
        guard.RecordFromDisk(real);
        std::filesystem::create_directories(dir.path / ".xlights" / "autosave" / writer);
        backup = ShowAutosave::LayoutAutosavePath(dir.path.string(), writer, "xlights_rgbeffects.xml");
        std::ofstream(backup, std::ios::binary) << "<xrgb>my unsaved edits</xrgb>";
        REQUIRE(ShowAutosave::WriteSidecar(backup, ShowAutosave::BaseFor(real, guard), Identity(writer)));
    }
};
} // namespace

TEST_CASE("Layout autosaves go to a folder per install", "[utils][autosave]") {
    const std::string show = "/shows/xmas";
    const std::string a = ShowAutosave::LayoutAutosavePath(show, kMine, "xlights_rgbeffects.xml");
    const std::string b = ShowAutosave::LayoutAutosavePath(show, kOther, "xlights_rgbeffects.xml");
    CHECK(a == "/shows/xmas/.xlights/autosave/" + kMine + "/xlights_rgbeffects.xbkp");
    CHECK(b == "/shows/xmas/.xlights/autosave/" + kOther + "/xlights_rgbeffects.xbkp");
    CHECK(a != b);
    CHECK(ShowAutosave::LayoutAutosavePath(show, kMine, "xlights_effectpresets.json") ==
          "/shows/xmas/.xlights/autosave/" + kMine + "/xlights_effectpresets.jbkp");
    CHECK(ShowAutosave::LegacyLayoutAutosavePath(show, "xlights_rgbeffects.xml") == "/shows/xmas/xlights_rgbeffects.xbkp");
    CHECK(ShowAutosave::SidecarPath(a) == a + ".json");
    CHECK(ShowAutosave::IsShowStateDirName(".xlights"));
    CHECK_FALSE(ShowAutosave::IsShowStateDirName("Backup"));
}

TEST_CASE("PrepareLayoutAutosavePath creates this install's folder", "[utils][autosave]") {
    TempDir dir;
    ShowAutosave::SetIdentity(kMine, "test device", "1.2.3");
    const std::string p = ShowAutosave::PrepareLayoutAutosavePath(dir.path.string(), "xlights_rgbeffects.xml");
    CHECK(p == ShowAutosave::LayoutAutosavePath(dir.path.string(), kMine, "xlights_rgbeffects.xml"));
    CHECK(std::filesystem::is_directory(dir.path / ".xlights" / "autosave" / kMine));

    ShowAutosave::SetIdentity("", "", "");
    CHECK(ShowAutosave::PrepareLayoutAutosavePath(dir.path.string(), "xlights_rgbeffects.xml").empty());
}

TEST_CASE("Install ids are UUIDs and a bad stored one is replaced", "[utils][autosave]") {
    CHECK(ShowAutosave::IsValidInstallId(kMine));
    CHECK_FALSE(ShowAutosave::IsValidInstallId(""));
    CHECK_FALSE(ShowAutosave::IsValidInstallId("../../etc"));
    CHECK_FALSE(ShowAutosave::IsValidInstallId("11111111-2222-4333-8444-55555555555/"));
    CHECK(ShowAutosave::ValidInstallIdOr(kMine) == kMine);
    const std::string fresh = ShowAutosave::ValidInstallIdOr("garbage");
    CHECK(ShowAutosave::IsValidInstallId(fresh));
    CHECK(fresh != ShowAutosave::NewInstallId());
}

TEST_CASE("Autosave sidecar round-trips", "[utils][autosave]") {
    ShowAutosaveInfo info = Identity(kMine);
    info.savedAt = 1790000000;
    info.base = ShowFileGuard::FingerprintOf(std::string("base"));
    info.backup = ShowFileGuard::FingerprintOf(std::string("backup bytes"));
    info.base.hash = 0xFEDCBA9876543210ULL;  // survives as hex, not as a double
    info.declined = true;

    auto back = ShowAutosave::FromJson(ShowAutosave::ToJson(info));
    REQUIRE(back);
    CHECK(back->installId == kMine);
    CHECK(back->deviceName == "test device");
    CHECK(back->appVersion == "1.2.3");
    CHECK(back->savedAt == 1790000000);
    CHECK(back->base == info.base);
    CHECK(back->backup == info.backup);
    CHECK(back->declined);

    CHECK_FALSE(ShowAutosave::FromJson("not json"));
    CHECK_FALSE(ShowAutosave::FromJson("{\"installId\":\"x\"}"));

    Scenario s;
    auto onDisk = ShowAutosave::ReadSidecar(s.backup);
    REQUIRE(onDisk);
    CHECK(onDisk->installId == kMine);
    CHECK(onDisk->base == *ShowFileGuard::FingerprintFile(s.real));
    CHECK(onDisk->backup == *ShowFileGuard::FingerprintFile(s.backup));
    CHECK_FALSE(onDisk->declined);
    CHECK(onDisk->savedAt > 0);
}

TEST_CASE("BaseFor is what the guard recorded, not the file as it is now", "[utils][autosave]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string real = dir.Write("seq.xsq", "loaded");
    guard.RecordFromDisk(real);
    dir.Write("seq.xsq", "someone else saved");
    CHECK(ShowAutosave::BaseFor(real, guard) == ShowFileGuard::FingerprintOf(std::string("loaded")));

    ShowFileGuard untracked;
    CHECK(ShowAutosave::BaseFor(real, untracked) == ShowFileGuard::FingerprintOf(std::string("someone else saved")));
}

TEST_CASE("This install's autosave is offered when it differs from an unchanged file", "[utils][autosave]") {
    Scenario s;
    auto c = ShowAutosave::Check(s.real, s.backup, kMine);
    CHECK(c.offer == AutosaveOffer::Offer);
    CHECK(c.hasInfo);
    CHECK(c.info.deviceName == "test device");
}

TEST_CASE("Another install's autosave is never offered", "[utils][autosave]") {
    Scenario s(kOther);
    CHECK(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::None);
    // Not even when the shared file changed since, or matches it.
    TempDir& d = s.dir;
    d.Write("xlights_rgbeffects.xml", "<xrgb>changed</xrgb>");
    CHECK(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::None);
    // It is still theirs to recover.
    CHECK(ShowAutosave::Check(s.real, s.backup, kOther).offer == AutosaveOffer::OfferConflict);
}

TEST_CASE("This install's autosave that matches the real file is stale", "[utils][autosave]") {
    Scenario s;
    s.dir.Write("xlights_rgbeffects.xml", "<xrgb>my unsaved edits</xrgb>");
    CHECK(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::Stale);
}

TEST_CASE("A newer mtime alone does not make an autosave worth offering", "[utils][autosave]") {
    TempDir dir;
    const std::string real = dir.Write("seq.xsq", "same");
    const std::string backup = dir.Write("seq.xbkp", "same");
    SetAge(real, 600);
    CHECK(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::None);
}

TEST_CASE("This install's autosave is a conflict when someone else saved the file since", "[utils][autosave]") {
    Scenario s;
    s.dir.Write("xlights_rgbeffects.xml", "<xrgb>their saved work</xrgb>");
    auto c = ShowAutosave::Check(s.real, s.backup, kMine);
    CHECK(c.offer == AutosaveOffer::OfferConflict);

    // Deleted since counts too.
    std::filesystem::remove(s.real);
    CHECK(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::OfferConflict);
}

TEST_CASE("An autosave with no sidecar is offered as legacy only when it is newer and differs", "[utils][autosave]") {
    TempDir dir;
    const std::string real = dir.Write("xlights_rgbeffects.xml", "<xrgb>saved</xrgb>");
    const std::string backup = dir.Write("xlights_rgbeffects.xbkp", "<xrgb>autosaved</xrgb>");
    SetAge(real, 600);
    SetAge(backup, 60);
    auto c = ShowAutosave::Check(real, backup, kMine);
    CHECK(c.offer == AutosaveOffer::OfferLegacy);
    CHECK_FALSE(c.hasInfo);

    SetAge(backup, 1200);
    CHECK(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::None);

    SetAge(backup, 60);
    dir.Write("xlights_rgbeffects.xbkp", "<xrgb>saved</xrgb>");
    SetAge(real, 600);
    CHECK(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::None);

    // No real file at all: anything autosaved is worth offering.
    dir.Write("xlights_rgbeffects.xbkp", "<xrgb>autosaved</xrgb>");
    std::filesystem::remove(real);
    CHECK(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::OfferLegacy);
}

TEST_CASE("A sidecar an older version left behind does not make its backup ours", "[utils][autosave]") {
    Scenario s;
    SetAge(s.real, 600);
    // An older version rewrites the backup but not the sidecar.
    std::ofstream(s.backup, std::ios::binary) << "<xrgb>someone else's autosave</xrgb>";
    auto c = ShowAutosave::Check(s.real, s.backup, kMine);
    CHECK(c.offer == AutosaveOffer::OfferLegacy);
    CHECK_FALSE(c.hasInfo);
    CHECK_FALSE(ShowAutosave::RemoveIfMine(s.backup));
    CHECK(Exists(s.backup));
}

TEST_CASE("A declined autosave is not offered again until a new autosave replaces it", "[utils][autosave]") {
    Scenario s;
    REQUIRE(ShowAutosave::Decline(s.backup));
    CHECK(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::None);
    CHECK(Exists(s.backup));

    std::ofstream(s.backup, std::ios::binary) << "<xrgb>more edits</xrgb>";
    REQUIRE(ShowAutosave::WriteSidecar(s.backup, ShowAutosave::BaseFor(s.real, s.guard), Identity(kMine)));
    CHECK(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::Offer);
}

TEST_CASE("Declining a legacy autosave stops the offer until an older version rewrites it", "[utils][autosave]") {
    TempDir dir;
    const std::string real = dir.Write("seq.xsq", "saved");
    const std::string backup = dir.Write("seq.xbkp", "autosaved");
    SetAge(real, 600);
    REQUIRE(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::OfferLegacy);
    REQUIRE(ShowAutosave::Decline(backup));
    CHECK(Exists(ShowAutosave::SidecarPath(backup)));
    CHECK(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::None);

    dir.Write("seq.xbkp", "a newer autosave from an older version");
    CHECK(ShowAutosave::Check(real, backup, kMine).offer == AutosaveOffer::OfferLegacy);
}

TEST_CASE("Restore anyway keeps the replaced file as a copy and records the restored one", "[utils][autosave]") {
    Scenario s;
    s.dir.Write("xlights_rgbeffects.xml", "<xrgb>their saved work</xrgb>");
    REQUIRE(ShowAutosave::Check(s.real, s.backup, kMine).offer == AutosaveOffer::OfferConflict);

    std::string kept;
    REQUIRE(ShowAutosave::Restore(s.real, s.backup, /*keepCurrentAsCopy*/ true, s.guard, &kept));
    CHECK(ReadAll(s.real) == "<xrgb>my unsaved edits</xrgb>");
    REQUIRE_FALSE(kept.empty());
    CHECK(kept != s.real);
    CHECK(kept.find("xlights_rgbeffects_before_restore_") != std::string::npos);
    CHECK(ReadAll(kept) == "<xrgb>their saved work</xrgb>");
    CHECK_FALSE(Exists(s.backup));
    CHECK_FALSE(Exists(ShowAutosave::SidecarPath(s.backup)));
    // The next save compares against the restored file.
    CHECK_FALSE(s.guard.HasChangedOnDisk(s.real));
    CHECK(s.guard.RecordedFingerprint(s.real) == ShowFileGuard::FingerprintOf(std::string("<xrgb>my unsaved edits</xrgb>")));
}

TEST_CASE("Restoring without a conflict replaces the file and leaves no copy", "[utils][autosave]") {
    Scenario s;
    std::string kept;
    REQUIRE(ShowAutosave::Restore(s.real, s.backup, /*keepCurrentAsCopy*/ false, s.guard, &kept));
    CHECK(kept.empty());
    CHECK(ReadAll(s.real) == "<xrgb>my unsaved edits</xrgb>");
    int files = 0;
    for (const auto& e : std::filesystem::directory_iterator(s.dir.path)) {
        if (e.is_regular_file()) {
            ++files;
        }
    }
    CHECK(files == 1);
}

TEST_CASE("Saving the autosave as a copy leaves the other person's file alone", "[utils][autosave]") {
    Scenario s;
    s.dir.Write("xlights_rgbeffects.xml", "<xrgb>their saved work</xrgb>");
    const std::string copy = ShowAutosave::SaveAsCopy(s.real, s.backup);
    REQUIRE_FALSE(copy.empty());
    CHECK(copy.find("xlights_rgbeffects_mine_") != std::string::npos);
    CHECK(ReadAll(copy) == "<xrgb>my unsaved edits</xrgb>");
    CHECK(ReadAll(s.real) == "<xrgb>their saved work</xrgb>");
    CHECK_FALSE(Exists(s.backup));
    CHECK_FALSE(Exists(ShowAutosave::SidecarPath(s.backup)));
}

TEST_CASE("RemoveIfMine removes only this install's autosave", "[utils][autosave]") {
    ShowAutosave::SetIdentity(kMine, "test device", "1.2.3");
    Scenario mine(kMine);
    CHECK(ShowAutosave::RemoveIfMine(mine.backup));
    CHECK_FALSE(Exists(mine.backup));
    CHECK_FALSE(Exists(ShowAutosave::SidecarPath(mine.backup)));

    Scenario theirs(kOther);
    CHECK_FALSE(ShowAutosave::RemoveIfMine(theirs.backup));
    CHECK(Exists(theirs.backup));

    TempDir dir;
    const std::string legacy = dir.Write("seq.xbkp", "older version");
    CHECK_FALSE(ShowAutosave::RemoveIfMine(legacy));
    CHECK(Exists(legacy));
    ShowAutosave::SetIdentity("", "", "");
}

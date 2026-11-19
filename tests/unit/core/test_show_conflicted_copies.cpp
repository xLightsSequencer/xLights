#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "support/TempDir.h"
#include "utils/ShowAutosave.h"
#include "utils/ShowConflictedCopies.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowSaveMerge.h"

// Conflicted copies a sync client leaves of the show-wide files: finding
// them, choosing the version to merge them from, merging, and moving them out
// of the way afterwards.
using namespace ShowConflictedCopies;
using ShowFileMerge::FileKind;
using ShowFileMerge::Resolution;

namespace {

const std::string kInstall = "0f8fad5b-d9cb-469f-a165-70867728950e";

std::string Slurp(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// The code under test joins with '/', the tests' paths come from std::filesystem (which joins with '\\' on
// Windows): compare as paths, not strings.
bool SamePath(const std::string& a, const std::string& b) {
    return std::filesystem::path(a).lexically_normal() == std::filesystem::path(b).lexically_normal();
}

bool Exists(const std::string& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

void SetAge(const std::string& path, int secondsAgo) {
    auto t = std::filesystem::file_time_type::clock::now() - std::chrono::seconds(secondsAgo);
    std::filesystem::last_write_time(path, t);
}

std::string Layout(const std::vector<std::pair<std::string, int>>& models) {
    std::string s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n";
    for (const auto& [name, nodes] : models) {
        s += "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) + "\"/>\n";
    }
    return s + "</models>\n</xrgb>\n";
}

pugi::xml_node ModelIn(const pugi::xml_document& d, const std::string& name) {
    return d.document_element().child("models").find_child_by_attribute("model", "name", name.c_str());
}

std::vector<std::string> Copies(const std::vector<Found>& found) {
    std::vector<std::string> out;
    for (const auto& f : found) {
        out.push_back(std::filesystem::path(f.copyPath).filename().string());
    }
    return out;
}

} // namespace

TEST_CASE("Conflicted copy names are recognised conservatively", "[conflictedcopies]") {
    struct Case {
        const char* name;
        FileKind kind;
        const char* client;
    };
    const Case positive[] = {
        { "xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml", FileKind::RgbEffects, "Dropbox" },
        { "xlights_rgbeffects (conflicted copy 2026-11-18 104512).xml", FileKind::RgbEffects, "Dropbox" },
        { "xlights_networks (Alex's Conflicted Copy 2026-11-18).xml", FileKind::Networks, "Dropbox" },
        { "xlights_rgbeffects 2.xml", FileKind::RgbEffects, "iCloud" },
        { "xlights_effectpresets 3.json", FileKind::Presets, "iCloud" },
        { "xlights_rgbeffects 12.xml", FileKind::RgbEffects, "iCloud" },
        { "xlights_rgbeffects-LAPTOP.xml", FileKind::RgbEffects, "OneDrive" },
        { "xlights_networks-DESKTOP-4KQ2M7.xml", FileKind::Networks, "OneDrive" },
        { "xlights_rgbeffects(1).xml", FileKind::RgbEffects, "Google Drive" },
        { "xlights_rgbeffects (2).xml", FileKind::RgbEffects, "Google Drive" },
        { "xlights_effectpresets.sync-conflict-20261118-104512-ABCDEFG.json", FileKind::Presets, "Syncthing" },
        { "xlights_rgbeffects 2.XML", FileKind::RgbEffects, "iCloud" },
    };
    for (const auto& c : positive) {
        INFO(c.name);
        auto m = MatchFileName(c.name);
        REQUIRE(m.has_value());
        CHECK(m->kind == c.kind);
        CHECK(m->client == c.client);
    }
    CHECK(MatchFileName("xlights_networks 2.xml")->realName == "xlights_networks.xml");

    const char* negative[] = {
        "xlights_rgbeffects.xml",
        "xlights_networks.xml",
        "xlights_effectpresets.json",
        "xlights_rgbeffects 1.xml",                       // iCloud starts at 2
        "xlights_rgbeffects 0.xml",
        "xlights_rgbeffects (copy).xml",                  // not a conflict marker
        "xlights_rgbeffects_mine_20261118_101010.xml",    // our own "save a copy"
        "xlights_rgbeffects_before_restore_20261118_101010.xml",
        "xlights_rgbeffects.xml.iPad-bkp",
        "xlights_rgbeffects.xbkp",
        "xlights_rgbeffects 2.json",                      // wrong extension for the file
        "xlights_effectpresets 2.xml",
        "xlights_rgbeffectsX.xml",
        "xlights_rgbeffects-.xml",
        "xlights_rgbeffects-ThisNameIsTooLongForAComputer.xml",
        "xlights_rgbeffects-two words.xml",
        "Xlights_rgbeffects 2.xml",
        "my show (conflicted copy).xml",
        "xlights_rgbeffects.sync-conflict-2026-ABC.xml",
        "xlights_keybindings 2.xml",
        "",
    };
    for (const char* n : negative) {
        INFO(n);
        CHECK_FALSE(MatchFileName(n).has_value());
    }
}

TEST_CASE("Conflicted copies are found by name and contents", "[conflictedcopies]") {
    TempDir dir;
    const std::string show = dir.path.string();
    dir.Write("xlights_rgbeffects.xml", Layout({ { "A", 10 } }));
    dir.Write("xlights_networks.xml", "<?xml version=\"1.0\"?>\n<Networks/>\n");
    dir.Write("xlights_rgbeffects 2.xml", Layout({ { "A", 20 } }));
    dir.Write("xlights_networks (Sam's conflicted copy 2026-11-18).xml", "<?xml version=\"1.0\"?>\n<Networks computer=\"x\"/>\n");
    dir.Write("xlights_rgbeffects-LAPTOP.xml", "<?xml version=\"1.0\"?>\n<Networks/>\n"); // a controllers file under a layout name
    dir.Write("xlights_rgbeffects(1).xml", "not xml");
    dir.Write("xlights_effectpresets 2.json", "{}"); // no presets file to merge into
    dir.Write(".xlights_rgbeffects 3.xml", Layout({ { "A", 30 } }));
    std::filesystem::create_directories(dir.path / "sub");
    dir.Write("sub/xlights_rgbeffects 4.xml", Layout({ { "A", 40 } }));

    auto found = Find(show, kInstall);
    REQUIRE(Copies(found) == std::vector<std::string>{ "xlights_networks (Sam's conflicted copy 2026-11-18).xml", "xlights_rgbeffects 2.xml" });
    CHECK(found[0].kind == FileKind::Networks);
    CHECK(found[0].client == "Dropbox");
    CHECK(SamePath(found[1].realPath, dir.File("xlights_rgbeffects.xml")));
    CHECK_FALSE(found[1].sameAsCurrent);
    CHECK(Describe(found[1]) == "xlights_rgbeffects 2.xml (iCloud)");

    SECTION("a copy that holds what the file holds is flagged") {
        dir.Write("xlights_rgbeffects 2.xml", Layout({ { "A", 10 } }));
        CHECK(Find(show, kInstall)[1].sameAsCurrent);
    }
    SECTION("Ignore remembers the copy, per install and by contents") {
        REQUIRE(Ignore(show, kInstall, found[1]));
        CHECK(Copies(Find(show, kInstall)) == std::vector<std::string>{ "xlights_networks (Sam's conflicted copy 2026-11-18).xml" });
        // Another install still sees it.
        CHECK(Find(show, "3b241101-e2bb-4255-8caf-4136c566a962").size() == 2);
        // A new copy under the same name is offered again.
        dir.Write("xlights_rgbeffects 2.xml", Layout({ { "A", 25 } }));
        CHECK(Find(show, kInstall).size() == 2);
        // Control: the ignore list really is what hid it.
        dir.Write("xlights_rgbeffects 2.xml", Layout({ { "A", 20 } }));
        CHECK(Find(show, kInstall).size() == 1);
        std::filesystem::remove(IgnoreListPath(show, kInstall));
        CHECK(Find(show, kInstall).size() == 2);
    }
    SECTION("without an install id nothing can be ignored") {
        CHECK_FALSE(Ignore(show, "", found[1]));
        CHECK(Find(show, "").size() == 2);
    }
}

TEST_CASE("The base for merging a conflicted copy", "[conflictedcopies]") {
    TempDir dir;
    const std::string show = dir.path.string();
    const std::string real = dir.Write("xlights_rgbeffects.xml", Layout({ { "A", 10 }, { "B", 10 } }));
    dir.Write("xlights_rgbeffects 2.xml", Layout({ { "A", 10 }, { "B", 10 }, { "C", 5 } }));
    SetAge(real, 600);
    SetAge(dir.File("xlights_rgbeffects 2.xml"), 300);
    auto found = Find(show, kInstall);
    REQUIRE(found.size() == 1);

    SECTION("no backups and no autosave: none, merged two-way") {
        CHECK(ChooseBase(found[0], show, kInstall, show).source == BaseSource::None);
    }
    SECTION("the newest backup older than both versions") {
        auto backup = [&](const std::string& folder, const std::string& text, int age) {
            std::filesystem::create_directories(dir.path / "Backup" / folder);
            std::string p = dir.Write("Backup/" + folder + "/xlights_rgbeffects.xml", text);
            SetAge(p, age);
            return p;
        };
        backup("2026-11-01-100000", Layout({ { "A", 1 } }), 5000);
        const std::string want = backup("2026-11-10-100000", Layout({ { "A", 2 } }), 1000);
        backup("2026-11-18-100000", Layout({ { "A", 3 } }), 100); // newer than the file: not a common ancestor
        backup("2026-11-12-100000", "garbage", 900);              // newest older one, but not a layout
        std::filesystem::create_directories(dir.path / "Backup" / "ConflictedCopies");
        SetAge(dir.Write("Backup/ConflictedCopies/xlights_rgbeffects.xml", Layout({ { "A", 4 } })), 800);
        auto choice = ChooseBase(found[0], show, kInstall, show);
        CHECK(choice.source == BaseSource::Backup);
        CHECK(SamePath(choice.path, want));
        CHECK(choice.text == Layout({ { "A", 2 } }));
    }
    SECTION("this install's autosave base copy comes first") {
        std::filesystem::create_directories(dir.path / "Backup" / "2026-11-10-100000");
        SetAge(dir.Write("Backup/2026-11-10-100000/xlights_rgbeffects.xml", Layout({ { "A", 2 } })), 1000);
        const std::string backupFile = ShowAutosave::LayoutAutosavePath(show, kInstall, "xlights_rgbeffects.xml");
        std::filesystem::create_directories(std::filesystem::path(backupFile).parent_path());
        std::ofstream(backupFile, std::ios::binary) << Layout({ { "A", 99 } });
        const std::string baseText = Layout({ { "A", 10 } });
        ShowAutosaveInfo who;
        who.installId = kInstall;
        REQUIRE(ShowAutosave::WriteSidecar(backupFile, ShowFileGuard::FingerprintOf(baseText), who));
        REQUIRE(ShowAutosave::WriteBaseCopy(backupFile, baseText));
        auto choice = ChooseBase(found[0], show, kInstall, show);
        CHECK(choice.source == BaseSource::Autosave);
        CHECK(choice.text == baseText);
        // Another install's autosave is not this one's base.
        CHECK(ChooseBase(found[0], show, "3b241101-e2bb-4255-8caf-4136c566a962", show).source == BaseSource::Backup);
        // A base copy that no longer matches its sidecar is stale.
        std::ofstream(ShowAutosave::BaseCopyPath(backupFile), std::ios::binary) << Layout({ { "A", 11 } });
        CHECK(ChooseBase(found[0], show, kInstall, show).source == BaseSource::Backup);
    }
}

TEST_CASE("Merging a conflicted copy", "[conflictedcopies]") {
    TempDir dir;
    const std::string show = dir.path.string();
    const std::string base = Layout({ { "A", 10 }, { "B", 10 } });
    // The file: B changed. The copy: C added.
    const std::string real = dir.Write("xlights_rgbeffects.xml", Layout({ { "A", 10 }, { "B", 20 } }));
    const std::string copyPath = dir.Write("xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml", Layout({ { "A", 10 }, { "B", 10 }, { "C", 5 } }));
    auto found = Find(show, kInstall);
    REQUIRE(found.size() == 1);
    ShowFileGuard guard;

    SECTION("with a base, both changes merge without asking") {
        BaseChoice b;
        b.source = BaseSource::Backup;
        b.text = base;
        auto plan = PrepareMerge(found[0], b);
        REQUIRE(plan.has_value());
        REQUIRE(plan->CanMerge());
        CHECK_FALSE(plan->HasConflicts());
        auto r = Finish(*plan, found[0], show, guard);
        REQUIRE(r.written);
        pugi::xml_document d;
        REQUIRE(d.load_string(Slurp(real).c_str()));
        CHECK(ModelIn(d, "B").attribute("NodesPerString").as_int() == 20);
        CHECK(ModelIn(d, "C"));
        // Moved, never deleted; the file before the merge is kept too.
        CHECK_FALSE(Exists(copyPath));
        CHECK(SamePath(r.movedCopy, (dir.path / "Backup" / "ConflictedCopies" / "xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml").string()));
        CHECK(Slurp(r.movedCopy) == Layout({ { "A", 10 }, { "B", 10 }, { "C", 5 } }));
        CHECK(Slurp(r.keptCurrent) == Layout({ { "A", 10 }, { "B", 20 } }));
        CHECK(guard.RecordedFingerprint(real) == ShowFileGuard::FingerprintFile(real));
        CHECK(Find(show, kInstall).empty());
    }
    SECTION("without a base every difference is a conflict") {
        auto plan = PrepareMerge(found[0], BaseChoice{});
        REQUIRE(plan.has_value());
        REQUIRE(plan->CanMerge());
        std::vector<ShowFileMerge::ConflictKind> kinds;
        for (auto ref : plan->Conflicts()) {
            kinds.push_back(plan->GetConflict(ref).kind);
        }
        // B differs (both have it), C only in the copy.
        REQUIRE(kinds.size() == 2);
        CHECK(std::count(kinds.begin(), kinds.end(), ShowFileMerge::ConflictKind::BothAdded) == 1);
        CHECK(std::count(kinds.begin(), kinds.end(), ShowFileMerge::ConflictKind::DeletedByTheirs) == 1);
        // Nothing is written until every one is decided.
        CHECK_FALSE(Finish(*plan, found[0], show, guard).written);
        CHECK(Exists(copyPath));
        plan->ResolveAll(Resolution::Mine); // the copy's version of each
        auto r = Finish(*plan, found[0], show, guard);
        REQUIRE(r.written);
        pugi::xml_document d;
        REQUIRE(d.load_string(Slurp(real).c_str()));
        CHECK(ModelIn(d, "B").attribute("NodesPerString").as_int() == 10);
        CHECK(ModelIn(d, "C"));
    }
    SECTION("a record only the file has is a conflict too, and Theirs keeps it") {
        dir.Write("xlights_rgbeffects.xml", Layout({ { "A", 10 }, { "D", 7 } }));
        dir.Write("xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml", Layout({ { "A", 10 } }));
        found = Find(show, kInstall);
        auto plan = PrepareMerge(found[0], BaseChoice{});
        REQUIRE(plan.has_value());
        REQUIRE(plan->Conflicts().size() == 1);
        CHECK(plan->GetConflict(plan->Conflicts()[0]).kind == ShowFileMerge::ConflictKind::DeletedByMine);
        CHECK(plan->GetConflict(plan->Conflicts()[0]).mineName.empty());
        plan->ResolveAll(Resolution::Theirs);
        REQUIRE(Finish(*plan, found[0], show, guard).written);
        pugi::xml_document d;
        REQUIRE(d.load_string(Slurp(real).c_str()));
        CHECK(ModelIn(d, "D"));
    }
    SECTION("control: the same files with their real base are a clean merge") {
        // Proves the conflicts above come from the missing base, not the inputs.
        dir.Write("xlights_rgbeffects.xml", Layout({ { "A", 10 }, { "D", 7 } }));
        dir.Write("xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml", Layout({ { "A", 10 } }));
        found = Find(show, kInstall);
        BaseChoice b;
        b.source = BaseSource::Backup;
        b.text = Layout({ { "A", 10 } });
        auto plan = PrepareMerge(found[0], b);
        REQUIRE(plan.has_value());
        CHECK_FALSE(plan->HasConflicts());
    }
    SECTION("the file changing again before the write leaves everything in place") {
        BaseChoice b;
        b.source = BaseSource::Backup;
        b.text = base;
        auto plan = PrepareMerge(found[0], b);
        REQUIRE(plan.has_value());
        dir.Write("xlights_rgbeffects.xml", Layout({ { "A", 11 }, { "B", 20 } }));
        auto r = Finish(*plan, found[0], show, guard);
        CHECK_FALSE(r.written);
        CHECK(r.writeResult == ShowSaveMerge::WriteResult::ChangedAgain);
        CHECK(Exists(copyPath));
        CHECK(Slurp(real) == Layout({ { "A", 11 }, { "B", 20 } }));
    }
    SECTION("a name already in the backup folder is not overwritten") {
        std::filesystem::create_directories(dir.path / "Backup" / "ConflictedCopies");
        const std::string taken = dir.Write("Backup/ConflictedCopies/xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml", "older");
        const std::string moved = MoveToBackup(copyPath, show);
        REQUIRE(!moved.empty());
        CHECK(moved != taken);
        CHECK(Slurp(taken) == "older");
        CHECK(Exists(moved));
        CHECK_FALSE(Exists(copyPath));
    }
}

TEST_CASE("The backup folder comes from the layout's setting", "[conflictedcopies]") {
    TempDir show;
    TempDir elsewhere;
    show.Write("xlights_rgbeffects.xml", "<xrgb><settings><backupDir value=\"" + elsewhere.path.string() + "\"/></settings></xrgb>");
    CHECK(BackupRootFor(show.path.string()) == elsewhere.path.string());
    show.Write("xlights_rgbeffects.xml", "<xrgb><settings><backupDir value=\"/no/such/folder\"/></settings></xrgb>");
    CHECK(BackupRootFor(show.path.string()) == show.path.string());
    CHECK(BackupFolder("/x") == "/x/Backup/ConflictedCopies");
}

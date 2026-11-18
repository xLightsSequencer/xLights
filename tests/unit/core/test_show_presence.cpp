#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "support/TempDir.h"
#include "utils/ShowPresence.h"

namespace {
const std::string kMine = "11111111-2222-4333-8444-555555555555";
const std::string kOther = "aaaaaaaa-bbbb-4ccc-8ddd-eeeeeeeeeeee";
const std::string kThird = "99999999-8888-4777-8666-555555555555";
const int64_t kNow = 1800000000;

bool Exists(const std::string& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

void SetFileAge(const std::string& path, int64_t secondsAgo) {
    auto t = std::filesystem::file_time_type::clock::now() - std::chrono::seconds(secondsAgo);
    std::filesystem::last_write_time(path, t);
}

ShowPresenceEntry Entry(const std::string& id, int64_t heartbeat, const std::string& seq = "") {
    ShowPresenceEntry e;
    e.installId = id;
    e.userName = "Sam";
    e.deviceName = "Office PC";
    e.platform = "Windows";
    e.appVersion = "2026.21";
    e.sequence = seq;
    e.heartbeat = heartbeat;
    return e;
}

// Writes an entry the way another install would have.
std::string Plant(const TempDir& dir, const ShowPresenceEntry& e) {
    std::filesystem::create_directories(ShowPresence::PresenceDir(dir.path.string()));
    const std::string path = ShowPresence::PresencePath(dir.path.string(), e.installId);
    std::ofstream(path, std::ios::binary) << ShowPresence::ToJson(e);
    return path;
}

void Setup(ShowPresencePublisher& p) {
    p.SetIdentity(kMine, "Me", "My Mac", "macOS", "2026.21");
    p.Enable(true);
}
} // namespace

TEST_CASE("ShowPresence: JSON round trip", "[presence]") {
    auto e = Entry(kOther, kNow, "Songs/Jingle.xsq");
    e.editingLayout = true;
    e.editingControllers = true;
    auto back = ShowPresence::FromJson(ShowPresence::ToJson(e));
    REQUIRE(back);
    CHECK(back->installId == kOther);
    CHECK(back->userName == "Sam");
    CHECK(back->deviceName == "Office PC");
    CHECK(back->platform == "Windows");
    CHECK(back->appVersion == "2026.21");
    CHECK(back->sequence == "Songs/Jingle.xsq");
    CHECK(back->editingLayout);
    CHECK(back->editingControllers);
    CHECK(back->heartbeat == kNow);

    CHECK_FALSE(ShowPresence::FromJson("not json"));
    CHECK_FALSE(ShowPresence::FromJson("{\"installId\":\"x\"}"));
}

TEST_CASE("ShowPresence: publisher writes a file the reader sees", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    ShowPresencePublisher pub;
    Setup(pub);
    ShowPresenceState s;
    s.sequencePath = (dir.path / "Songs" / "Jingle.xsq").string();
    s.editingLayout = true;
    REQUIRE(pub.Update(show, s, kNow));

    auto all = ShowPresence::ReadEntries(show);
    REQUIRE(all.size() == 1);
    CHECK(all[0].installId == kMine);
    CHECK(all[0].deviceName == "My Mac");
    CHECK(all[0].sequence == "Songs/Jingle.xsq");
    CHECK(all[0].editingLayout);
    CHECK_FALSE(all[0].editingControllers);
    CHECK(all[0].heartbeat == kNow);

    // Another install reading the folder sees it; this one does not see itself.
    CHECK(ShowPresence::FreshOthers(show, kOther, kNow + 10).size() == 1);
    CHECK(ShowPresence::FreshOthers(show, kMine, kNow + 10).empty());
}

TEST_CASE("ShowPresence: own entry is excluded", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    Plant(dir, Entry(kMine, kNow, "A.xsq"));
    Plant(dir, Entry(kOther, kNow, "A.xsq"));

    auto others = ShowPresence::FreshOthers(show, kMine, kNow);
    REQUIRE(others.size() == 1);
    CHECK(others[0].installId == kOther);

    // Control: as a third install, both are visible, so the exclusion above is
    // what removed this install's own entry.
    CHECK(ShowPresence::FreshOthers(show, kThird, kNow).size() == 2);
    // Case of the id does not matter.
    std::string upper = kMine;
    for (auto& c : upper) c = (char)std::toupper((unsigned char)c);
    CHECK(ShowPresence::FreshOthers(show, upper, kNow).size() == 1);
}

TEST_CASE("ShowPresence: stale entries are ignored", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    Plant(dir, Entry(kOther, kNow - ShowPresence::kStaleSeconds - 1, "A.xsq"));
    Plant(dir, Entry(kThird, kNow - ShowPresence::kStaleSeconds + 1, "A.xsq"));

    auto others = ShowPresence::FreshOthers(show, kMine, kNow);
    REQUIRE(others.size() == 1);
    CHECK(others[0].installId == kThird);

    // Control: with a longer timeout the stale one counts, so it was the age
    // check that dropped it.
    auto all = ShowPresence::ReadEntries(show);
    REQUIRE(all.size() == 2);
    for (const auto& e : all) {
        CHECK(ShowPresence::IsFresh(e, kNow, ShowPresence::kStaleSeconds * 2));
    }
}

TEST_CASE("ShowPresence: heartbeats in the future are capped", "[presence]") {
    // Slightly ahead (clock skew): fresh, age 0.
    auto nearFuture = Entry(kOther, kNow + 120);
    CHECK(ShowPresence::IsFresh(nearFuture, kNow));
    CHECK(ShowPresence::AgeSeconds(nearFuture, kNow) == 0);
    // Far ahead: not trusted, so it cannot stay fresh for days.
    auto farFuture = Entry(kOther, kNow + ShowPresence::kMaxFutureSeconds + 1);
    CHECK_FALSE(ShowPresence::IsFresh(farFuture, kNow));
    // At the limit it is still fresh, and goes stale once the reader's clock
    // passes it by the timeout.
    auto atLimit = Entry(kOther, kNow + ShowPresence::kMaxFutureSeconds);
    CHECK(ShowPresence::IsFresh(atLimit, kNow));
    CHECK_FALSE(ShowPresence::IsFresh(atLimit, kNow + ShowPresence::kMaxFutureSeconds + ShowPresence::kStaleSeconds + 1));
}

TEST_CASE("ShowPresence: who has a sequence open", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    Plant(dir, Entry(kOther, kNow, "Songs/Jingle Bells.xsq"));
    Plant(dir, Entry(kThird, kNow, "Songs/Other.xsq"));

    const std::string mine = (dir.path / "Songs" / "Jingle Bells.xsq").string();
    auto holders = ShowPresence::WhoHasSequenceOpen(show, mine, kMine, kNow);
    REQUIRE(holders.size() == 1);
    CHECK(holders[0].installId == kOther);

    // Normalisation: "." / ".." segments, case and a trailing separator on the show folder.
    CHECK(ShowPresence::WhoHasSequenceOpen(show + "/", show + "/Songs/./x/../JINGLE BELLS.xsq", kMine, kNow).size() == 1);
    CHECK(ShowPresence::SameSequence("Songs\\Jingle Bells.xsq", "songs/jingle bells.xsq"));
    CHECK_FALSE(ShowPresence::SameSequence("", ""));

    CHECK(ShowPresence::RelativeSequencePath(show, mine) == "Songs/Jingle Bells.xsq");
    CHECK(ShowPresence::RelativeSequencePath(show + "/", mine) == "Songs/Jingle Bells.xsq");
    CHECK(ShowPresence::RelativeSequencePath(show, "") == "");
    // Opened from its autosave: still the same sequence.
    CHECK(ShowPresence::WhoHasSequenceOpen(show, show + "/Songs/Jingle Bells.xbkp", kMine, kNow).size() == 1);
    // Outside the show folder the path stays absolute.
    const std::string outside = (dir.path.parent_path() / "elsewhere" / "Seq.xsq").lexically_normal().generic_string();
    CHECK(ShowPresence::RelativeSequencePath(show, outside) == outside);

    // Not open by anyone, or open only by a stale entry.
    CHECK(ShowPresence::WhoHasSequenceOpen(show, show + "/Songs/Nobody.xsq", kMine, kNow).empty());
    CHECK(ShowPresence::WhoHasSequenceOpen(show, mine, kMine, kNow + ShowPresence::kStaleSeconds + 1).empty());
}

TEST_CASE("ShowPresence: files that are not presence entries are skipped", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    Plant(dir, Entry(kOther, kNow));
    const std::string pdir = ShowPresence::PresenceDir(show);
    std::ofstream(pdir + "/notes.json") << ShowPresence::ToJson(Entry(kThird, kNow));
    std::ofstream(pdir + "/" + kThird + ".json") << "garbage";
    // Content naming a different id than the file name.
    std::ofstream(pdir + "/" + kMine + ".json") << ShowPresence::ToJson(Entry(kThird, kNow));
    auto all = ShowPresence::ReadEntries(show);
    REQUIRE(all.size() == 1);
    CHECK(all[0].installId == kOther);
    CHECK(ShowPresence::ReadEntries((dir.path / "missing").string()).empty());
}

TEST_CASE("ShowPresence: very stale cleanup removes only old foreign entries", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    const int64_t now = ShowPresence::Now();
    const int64_t old = ShowPresence::kCleanupSeconds + 3600;
    const std::string skewId = "12345678-1234-4234-8234-123456789012";

    const std::string stale = Plant(dir, Entry(kOther, now - old));
    SetFileAge(stale, old);
    const std::string fresh = Plant(dir, Entry(kThird, now));
    SetFileAge(fresh, old);  // old file time, but the heartbeat is recent
    const std::string mine = Plant(dir, Entry(kMine, now - old));
    SetFileAge(mine, old);
    const std::string notes = ShowPresence::PresenceDir(show) + "/readme.txt";
    std::ofstream(notes) << "keep me";
    SetFileAge(notes, old);
    // Old heartbeat but recently written (the writer's clock is behind).
    const std::string skewed = Plant(dir, Entry(skewId, now - old));

    CHECK(ShowPresence::RemoveVeryStale(show, kMine, now) == 1);
    CHECK_FALSE(Exists(stale));
    CHECK(Exists(fresh));
    CHECK(Exists(mine));
    CHECK(Exists(notes));
    CHECK(Exists(skewed));

    // Control: with a longer limit nothing is old enough.
    const std::string stale2 = Plant(dir, Entry(kOther, now - old));
    SetFileAge(stale2, old);
    CHECK(ShowPresence::RemoveVeryStale(show, kMine, now, old * 2) == 0);
    CHECK(Exists(stale2));
}

TEST_CASE("ShowPresence: publisher throttles heartbeat-only rewrites", "[presence]") {
    TempDir dir;
    const std::string show = dir.path.string();
    ShowPresencePublisher pub;
    Setup(pub);
    ShowPresenceState s;
    REQUIRE(pub.Update(show, s, kNow));
    CHECK(pub.WriteCount() == 1);

    // Nothing but time changed, inside the heartbeat window: no write.
    CHECK_FALSE(pub.Update(show, s, kNow + 1));
    CHECK_FALSE(pub.Update(show, s, kNow + ShowPresence::kHeartbeatSeconds - 1));
    CHECK(pub.WriteCount() == 1);
    CHECK(ShowPresence::ReadEntries(show)[0].heartbeat == kNow);

    // A state change writes at once.
    s.editingControllers = true;
    CHECK(pub.Update(show, s, kNow + 5));
    CHECK(pub.WriteCount() == 2);
    CHECK(ShowPresence::ReadEntries(show)[0].editingControllers);

    // Heartbeat interval passed: write again.
    CHECK(pub.Update(show, s, kNow + 5 + ShowPresence::kHeartbeatSeconds));
    CHECK(pub.WriteCount() == 3);
    CHECK(ShowPresence::ReadEntries(show)[0].heartbeat == kNow + 5 + ShowPresence::kHeartbeatSeconds);

    // Clock went backwards: rewrite rather than wait.
    CHECK(pub.Update(show, s, kNow - 1000));
}

TEST_CASE("ShowPresence: publisher moves with the show folder and clears", "[presence]") {
    TempDir a;
    TempDir b;
    ShowPresencePublisher pub;
    Setup(pub);
    ShowPresenceState s;
    REQUIRE(pub.Update(a.path.string(), s, kNow));
    const std::string fileA = ShowPresence::PresencePath(a.path.string(), kMine);
    CHECK(Exists(fileA));

    REQUIRE(pub.Update(b.path.string(), s, kNow + 1));
    CHECK_FALSE(Exists(fileA));
    const std::string fileB = ShowPresence::PresencePath(b.path.string(), kMine);
    CHECK(Exists(fileB));

    pub.Clear();
    CHECK_FALSE(Exists(fileB));

    // Clear does not touch other installs' files.
    Plant(b, Entry(kOther, kNow));
    REQUIRE(pub.Update(b.path.string(), s, kNow + 2));
    pub.Clear();
    CHECK(ShowPresence::ReadEntries(b.path.string()).size() == 1);
}

TEST_CASE("ShowPresence: a disabled publisher writes nothing", "[presence]") {
    TempDir dir;
    ShowPresencePublisher pub;
    pub.SetIdentity(kMine, "Me", "My Mac", "macOS", "2026.21");
    CHECK_FALSE(pub.Update(dir.path.string(), ShowPresenceState{}, kNow));
    CHECK_FALSE(Exists(ShowPresence::PresenceDir(dir.path.string())));

    // Without a valid install id there is nothing to name the file after.
    ShowPresencePublisher noId;
    noId.Enable(true);
    CHECK_FALSE(noId.Update(dir.path.string(), ShowPresenceState{}, kNow));

    // Control: enabled with an id, it writes.
    pub.Enable(true);
    CHECK(pub.Update(dir.path.string(), ShowPresenceState{}, kNow));
}

TEST_CASE("ShowPresence: messages", "[presence]") {
    auto sam = Entry(kOther, kNow - 180, "A.xsq");
    CHECK(ShowPresence::SequenceOpenWarning({}, kNow).empty());
    CHECK(ShowPresence::SequenceOpenWarning({ sam }, kNow) ==
          "Sam has this sequence open on Office PC (last seen 3 min ago). Opening it too can lose one of your changes.");
    auto ipad = Entry(kThird, kNow, "A.xsq");
    ipad.userName.clear();
    ipad.deviceName = "Studio iPad";
    CHECK(ShowPresence::SequenceOpenWarning({ ipad, sam }, kNow) ==
          "Studio iPad has this sequence open (last seen just now). Sam on Office PC has it open too. Opening it too can lose one of your changes.");

    CHECK(ShowPresence::EditingHint({ sam }).empty());
    sam.editingLayout = true;
    CHECK(ShowPresence::EditingHint({ sam }) == "Sam on Office PC is editing the layout");
    ipad.editingControllers = true;
    CHECK(ShowPresence::EditingHint({ sam, ipad }) == "Sam on Office PC is editing the layout; Studio iPad is editing the controllers");
    sam.editingControllers = true;
    CHECK(ShowPresence::EditingHint({ sam }) == "Sam on Office PC is editing the layout and controllers");
}

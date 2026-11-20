#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "collab/CollabManifest.h"
#include "render/SequenceMediaCommands.h"

#include <atomic>
#include <chrono>
#include <thread>

using namespace Collab;
using namespace CollabTest;
namespace fs = std::filesystem;

namespace {

// Paths built with "/" and with fs::path differ only in separators on Windows.
std::string Norm(const std::string& p) {
    return fs::path(p).lexically_normal().string();
}

LoopbackOptions Lan(uint32_t seed = 1) {
    LoopbackOptions o;
    o.seed = seed;
    o.minDelayMs = 10;
    o.maxDelayMs = 10;
    return o;
}

SessionOptions Opts() {
    SessionOptions o;
    o.verifyHashes = true;
    return o;
}

SessionOptions Fetching(const std::string& folder) {
    SessionOptions o = Opts();
    o.fetchFiles = true;
    o.sessionFolder = folder;
    return o;
}

const std::vector<std::string> kAllFiles{ kLayoutFile, kNetworksFile, "song.wav", "Images/a.png" };

// Another device's show folder: a copy of the host's files.
struct Device {
    TempDir dir;
    std::string path;
    Device(MediaShow& ms, const std::vector<std::string>& files = kAllFiles) : path(dir.path.string()) {
        CopyShowFiles(ms.show.path, dir.path, files);
    }
};

// Every active client holds the host's document, ids included. (.xsq bytes
// differ: the audio path is saved as each device resolved it.)
void RequireSameDocument(Sim& sim) {
    REQUIRE(sim.Quiesce());
    Node& h = sim.host();
    std::string desc = DescribeSequence(h.Seq());
    std::string problem;
    for (size_t i = 1; i < sim.nodes.size(); ++i) {
        Node& c = *sim.nodes[i];
        if (!c.Client().IsActive()) {
            continue;
        }
        INFO("client " << i);
        CHECK(DescribeSequence(c.Seq()) == desc);
        CHECK(DocHasher::FullHash(c.Seq()) == DocHasher::FullHash(h.Seq()));
        CHECK(c.Seq().ValidateUids(&problem));
    }
}

bool Contains(const std::vector<std::string>& v, const std::string& s) {
    return std::find(v.begin(), v.end(), s) != v.end();
}

std::unique_ptr<SequenceCommand> AddPicture(Node& n, const std::string& file, int start) {
    EffectState s;
    s.name = "Pictures";
    s.settings = "E_TEXTCTRL_Pictures_Filename=" + file;
    s.startTimeMS = start;
    s.endTimeMS = start + 400;
    Element* el = n.Seq().GetElement("Line");
    REQUIRE(el != nullptr);
    return std::make_unique<AddEffectCommand>(LocatorOf(el->GetEffectLayer(0)), s);
}

} // namespace

TEST_CASE("A joiner with its own copy of the show opens the session against it", "[collab][collab_join]") {
    MediaShow ms;
    Device dev(ms);
    TempDir session;
    Sim sim(ms.show, Lan(), Opts());
    sim.AddHost("", std::nullopt, ms.xsq);
    auto before = FolderState(dev.dir.path);
    Node& a = sim.AddClientOn(dev.path);
    // Fetching changes nothing when every file matches.
    Node& b = sim.AddClientOn(dev.path, Fetching((session.path / "b").string()));
    RequireSameDocument(sim);
    for (Node* n : { &a, &b }) {
        const JoinReport& r = n->Client().GetJoinReport();
        CHECK(n->Client().IsActive());
        CHECK(r.showFolder == dev.path);
        CHECK_FALSE(r.sessionCopy);
        CHECK(r.fetched.empty());
        CHECK(r.missingMedia.empty());
        CHECK(r.differentMedia.empty());
        CHECK(r.layoutDifference.empty());
        CHECK(n->doc->ShowFolder() == dev.path);
        CHECK(n->Client().FileChecks().size() == 5);
    }
    // The document came as a blob, one per joiner.
    CHECK(sim.net.BlobsDelivered() == 2);
    CHECK(FolderState(dev.dir.path) == before);
}

TEST_CASE("A joiner whose layout differs is refused with what differs", "[collab][collab_join]") {
    MediaShow ms;
    Device dev(ms);
    WriteFile(dev.dir.path / kLayoutFile, ChangedLayout(ReadFile(ms.show.dir.File(kLayoutFile))));
    auto before = FolderState(dev.dir.path);
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(dev.path);
    sim.RunFor(500);
    CHECK(a.Client().GetState() == CollabClient::State::Ended);
    CHECK(a.Client().EndReason() == Reason::LayoutDiffers);
    const std::string& why = a.Client().EndText();
    INFO(why);
    CHECK(why.find("layout differs") != std::string::npos);
    CHECK(why.find("1 model changed (Arch)") != std::string::npos);
    CHECK(why.find("1 model only on this device (Extra)") != std::string::npos);
    CHECK(why.find("1 model only on the host (Pole)") != std::string::npos);
    CHECK(a.Client().GetJoinReport().refusal == why);
    CHECK(a.Client().GetJoinReport().layoutDifference.find("Arch") != std::string::npos);
    // No document was sent, and the host let it go.
    CHECK(sim.net.BlobsDelivered() == 1); // the host's layout, to describe the difference
    CHECK(h.Host().ClientCount() == 0);
    CHECK(FolderState(dev.dir.path) == before);
}

TEST_CASE("Layout differences the shared-folder merge ignores don't refuse a join", "[collab][collab_join]") {
    MediaShow ms;
    Device dev(ms);
    WriteFile(dev.dir.path / kLayoutFile, ReformattedLayout(ReadFile(ms.show.dir.File(kLayoutFile))));
    Sim sim(ms.show, Lan(), Opts());
    sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(dev.path);
    RequireSameDocument(sim);
    CHECK(a.Client().IsActive());
    CHECK(a.Client().GetJoinReport().layoutDifference.empty());
    CHECK(a.doc->ShowFolder() == dev.path);
}

TEST_CASE("A joiner whose audio differs is refused; missing media is only reported", "[collab][collab_join]") {
    MediaShow ms;
    Device wrongSong(ms);
    WriteFile(wrongSong.dir.path / "song.wav", SilentWav(2));
    Device noPicture(ms, { kLayoutFile, kNetworksFile, "song.wav" });
    Device otherPicture(ms);
    WriteFile(otherPicture.dir.path / "Images" / "a.png", "another picture");
    Sim sim(ms.show, Lan(), Opts());
    sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(wrongSong.path);
    Node& b = sim.AddClientOn(noPicture.path);
    Node& c = sim.AddClientOn(otherPicture.path);
    RequireSameDocument(sim);
    CHECK(a.Client().EndReason() == Reason::AudioDiffers);
    CHECK(a.Client().EndText().find("song.wav (different)") != std::string::npos);
    REQUIRE(b.Client().IsActive());
    CHECK(b.Client().GetJoinReport().missingMedia == std::vector<std::string>{ "Images/a.png" });
    CHECK(b.Client().GetJoinReport().differentMedia.empty());
    REQUIRE(c.Client().IsActive());
    CHECK(c.Client().GetJoinReport().differentMedia == std::vector<std::string>{ "Images/a.png" });
    // Media isn't fetched without fetchFiles.
    CHECK(sim.net.BlobsDelivered() == 2);
}

TEST_CASE("With fetchFiles a joiner copies what differs and opens against a session copy", "[collab][collab_join]") {
    MediaShow ms;
    Device dev(ms, { kNetworksFile, "song.wav" });
    WriteFile(dev.dir.path / kLayoutFile, ChangedLayout(ReadFile(ms.show.dir.File(kLayoutFile))));
    WriteFile(dev.dir.path / "song.wav", SilentWav(2));
    auto before = FolderState(dev.dir.path);
    TempDir session;
    std::string folder = (session.path / "a").string();
    Sim sim(ms.show, Lan(), Opts());
    sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(dev.path, Fetching(folder));
    RequireSameDocument(sim);
    REQUIRE(a.Client().IsActive());
    const JoinReport& r = a.Client().GetJoinReport();
    std::string copy = a.session->SessionShowFolder();
    CHECK(r.sessionCopy);
    CHECK(r.showFolder == copy);
    CHECK(a.doc->ShowFolder() == copy);
    // Inside a folder of this session's own, inside the one it was given.
    CHECK(fs::path(copy).parent_path().parent_path() == fs::path(folder));
    CHECK(fs::path(copy).parent_path() == fs::path(a.session->SessionFolder()));
    CHECK(r.fetched == std::vector<std::string>{ "Images/a.png", "song.wav", kLayoutFile });
    CHECK(r.missingMedia.empty());
    CHECK(r.differentMedia.empty());
    CHECK(r.layoutDifference.find("1 model changed (Arch)") != std::string::npos);
    // The copy holds the host's files: fetched ones and the networks file
    // that matched, copied from the device's own show folder.
    for (const char* f : { kLayoutFile, kNetworksFile, "song.wav", "Images/a.png" }) {
        INFO(f);
        CHECK(ReadFile((fs::path(copy) / f).string()) == ReadFile(ms.show.dir.File(f)));
    }
    // The device's own show folder was never written to.
    CHECK(FolderState(dev.dir.path) == before);
    // The session copy goes with the session...
    {
        Sim::Scope s(a);
        a.session.reset();
    }
    CHECK_FALSE(fs::exists(copy));
    CHECK(fs::exists(folder)); // the folder is the app's; only what the session made goes
}

TEST_CASE("A session copy is kept when asked to", "[collab][collab_join]") {
    MediaShow ms;
    Device dev(ms, { kLayoutFile, kNetworksFile, "song.wav" });
    TempDir session;
    Sim sim(ms.show, Lan(), Opts());
    sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(dev.path, Fetching(session.path.string()));
    RequireSameDocument(sim);
    REQUIRE(a.Client().GetJoinReport().sessionCopy);
    std::string copy = a.session->SessionShowFolder();
    a.session->KeepSessionFiles(true);
    {
        Sim::Scope s(a);
        a.session.reset();
    }
    CHECK(ReadFile((fs::path(copy) / "Images" / "a.png").string()) == "picture-a");
    CHECK(fs::exists(fs::path(copy) / kLayoutFile));
    // Blobs on their way are never kept.
    CHECK_FALSE(fs::exists(session.path / "incoming"));
}

TEST_CASE("A joiner that can't open a session copy, or can't get a layout file, is refused", "[collab][collab_join]") {
    MediaShow ms;
    Device dev(ms, { kNetworksFile, "song.wav", "Images/a.png" });
    TempDir session;
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(dev.path, Fetching((session.path / "a").string()), false);
    a.doc->allowShowFolderChange = false;
    sim.Join(a);
    sim.RunFor(500);
    CHECK(a.Client().EndReason() == Reason::FilesUnavailable);
    // The host's layout went away after it listed it.
    Node& b = sim.AddClientOn(dev.path, Fetching((session.path / "b").string()), false);
    std::string saved = ReadFile(ms.show.dir.File(kLayoutFile));
    sim.net.tamper = [&](PeerId from, PeerId, std::string& frame) {
        if (from == h.transport->LocalPeer() && frame.find("\"t\":\"Manifest\"") != std::string::npos) {
            fs::remove(ms.show.dir.File(kLayoutFile));
        }
    };
    sim.Join(b);
    sim.RunFor(500);
    sim.net.tamper = nullptr;
    WriteFile(ms.show.dir.File(kLayoutFile), saved);
    CHECK(b.Client().EndReason() == Reason::FilesUnavailable);
    CHECK(b.Client().EndText().find(kLayoutFile) != std::string::npos);
    // ...or changed after it listed it: what comes isn't what the manifest said.
    Node& c = sim.AddClientOn(dev.path, Fetching((session.path / "c").string()), false);
    sim.net.tamper = [&](PeerId from, PeerId, std::string& frame) {
        if (from == h.transport->LocalPeer() && frame.find("\"t\":\"Manifest\"") != std::string::npos) {
            WriteFile(ms.show.dir.File(kLayoutFile), ChangedLayout(saved));
        }
    };
    sim.Join(c);
    sim.RunFor(500);
    sim.net.tamper = nullptr;
    WriteFile(ms.show.dir.File(kLayoutFile), saved);
    CHECK(c.Client().EndReason() == Reason::FilesUnavailable);
    CHECK(c.Client().EndText().find(kLayoutFile) != std::string::npos);
    CHECK(h.Host().ClientCount() == 0);
}

TEST_CASE("The host only sends files its manifest lists", "[collab][collab_join]") {
    MediaShow ms;
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Node& raw = sim.AddClient(false);
    std::vector<Message> got;
    std::vector<std::string> blobs;
    raw.transport->onMessage = [&](PeerId, const Message& m) { got.push_back(m); };
    raw.transport->onBlob = [&](PeerId, BlobId, const std::string& p) { blobs.push_back(ReadFile(p)); };
    PeerId host = h.transport->LocalPeer();
    Message hello;
    hello.type = MsgType::Hello;
    raw.transport->Send(host, hello);
    sim.RunFor(100);
    // Asking before joining gets nothing.
    Message early;
    early.type = MsgType::NeedFiles;
    early.paths = { "Images/a.png" };
    Node& stranger = sim.AddClient(false);
    stranger.transport->onBlob = [&](PeerId, BlobId, const std::string&) { blobs.push_back("stranger"); };
    stranger.transport->Send(host, early);

    Message need;
    need.type = MsgType::NeedFiles;
    need.paths = { kLayoutFile, "../" + std::string(kLayoutFile), "/etc/hosts", ms.show.dir.File("media.xsq"), "Images/a.png", "Images/a.png" };
    raw.transport->Send(host, need);
    sim.RunFor(100);
    std::map<std::string, std::string> answers; // path -> reason ("" = sent)
    size_t files = 0;
    for (const auto& m : got) {
        if (m.type == MsgType::File) {
            ++files;
            answers[m.files[0].path] = m.reason;
        }
    }
    CHECK(files == 5); // the repeated path is answered once
    CHECK(answers[kLayoutFile].empty());
    CHECK(answers["Images/a.png"].empty());
    CHECK(answers["../" + std::string(kLayoutFile)] == Reason::NotInManifest);
    CHECK(answers["/etc/hosts"] == Reason::NotInManifest);
    CHECK(answers[ms.show.dir.File("media.xsq")] == Reason::NotInManifest);
    REQUIRE(blobs.size() == 2);
    CHECK(std::count(blobs.begin(), blobs.end(), std::string("picture-a")) == 1);
}

TEST_CASE("Joining works whatever order blobs and messages arrive in", "[collab][collab_join]") {
    MediaShow ms;
    Device same(ms);
    Device differs(ms, { kNetworksFile });
    for (uint32_t seed = 1; seed <= 6; ++seed) {
        INFO("seed " << seed);
        LoopbackOptions o;
        o.seed = seed;
        o.minDelayMs = 0;
        o.maxDelayMs = 40;
        o.fifoPerLink = false;
        o.dropRate = seed % 2 ? 0.05 : 0.0;
        TempDir session;
        SessionOptions opts = Fetching(session.path.string());
        opts.fileRetryMs = 500;
        Sim sim(ms.show, o, Opts());
        Node& h = sim.AddHost("", std::nullopt, ms.xsq);
        Node& a = sim.AddClientOn(same.path);
        Node& b = sim.AddClientOn(differs.path, opts);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(sim.As(h, [&] { return h.session->Submit(AddPicture(h, "Images/a.png", 5000 + i * 500)); }).ok);
            sim.RunFor(15);
        }
        RequireSameDocument(sim);
        CHECK(a.Client().IsActive());
        CHECK(b.Client().IsActive());
        CHECK(b.Client().GetJoinReport().sessionCopy);
        CHECK(a.Client().ResyncCount() == 0);
        CHECK(b.Client().ResyncCount() == 0);
        if (o.dropRate == 0) {
            // Nothing was asked for twice: a snapshot each, and each file b fetched.
            CHECK(sim.net.BlobsDelivered() == 2 + b.Client().GetJoinReport().fetched.size());
        }
    }
}

TEST_CASE("The host's private session folder goes with the session", "[collab][collab_join]") {
    MediaShow ms;
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    sim.AddClient();
    RequireSameDocument(sim);
    std::string folder = h.session->SessionFolder();
    CHECK(fs::exists(folder));
    // Snapshots on their way are deleted once handed to the transport.
    bool pending = fs::exists(fs::path(folder) / "outgoing") && !fs::is_empty(fs::path(folder) / "outgoing");
    CHECK_FALSE(pending);
    {
        Sim::Scope s(h);
        h.session.reset();
    }
    CHECK_FALSE(fs::exists(folder));
}

TEST_CASE("Media added by file reference reaches the host and every other participant", "[collab][collab_join]") {
    MediaShow ms;
    Device da(ms);
    Device db(ms);
    Device dc(ms);
    WriteFile(da.dir.path / "Images" / "new.png", "new picture");
    WriteFile(ms.show.dir.path / "Images" / "hosts.png", "host picture");
    auto hostBefore = FolderState(ms.show.dir.path / "Images");
    auto bBefore = FolderState(db.dir.path);
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(da.path);
    Node& b = sim.AddClientOn(db.path);
    REQUIRE(sim.Quiesce());
    auto aBefore = FolderState(da.dir.path);

    // The author adds the media entry and an effect that uses it.
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<PutMediaCommand>(MediaEntryState::FileReference("Images/new.png", MediaType::Image))); }).ok);
    REQUIRE(sim.As(a, [&] { return a.session->Submit(AddPicture(a, "Images/new.png", 6000)); }).ok);
    RequireSameDocument(sim);
    for (Node* n : { &h, &b }) {
        INFO((n == &h ? "host" : "b"));
        auto& got = n->session->ReceivedFiles();
        REQUIRE(got.count("Images/new.png") == 1);
        CHECK(ReadFile(got.at("Images/new.png")) == "new picture");
        CHECK(Norm(got.at("Images/new.png")) == Norm((fs::path(n->session->SessionShowFolder()) / "Images" / "new.png").string()));
        // The document finds it through the session's media folder.
        CHECK(Norm(sim.As(*n, [&] { return ResolvePath(n->doc->ShowFolder(), "Images/new.png"); })) == Norm(got.at("Images/new.png")));
        CHECK(n->doc->MediaFolders() == std::list<std::string>{ n->session->SessionShowFolder() });
    }
    CHECK(a.session->ReceivedFiles().empty());

    // The host's own file reference reaches the clients the same way.
    REQUIRE(sim.As(h, [&] { return h.session->Submit(std::make_unique<PutMediaCommand>(MediaEntryState::FileReference("Images/hosts.png", MediaType::Image))); }).ok);
    RequireSameDocument(sim);
    for (Node* n : { &a, &b }) {
        REQUIRE(n->session->ReceivedFiles().count("Images/hosts.png") == 1);
        CHECK(ReadFile(n->session->ReceivedFiles().at("Images/hosts.png")) == "host picture");
    }

    // A late joiner fetches it like any other file the sequence uses.
    TempDir session;
    Node& c = sim.AddClientOn(dc.path, Fetching(session.path.string()));
    RequireSameDocument(sim);
    REQUIRE(c.Client().IsActive());
    CHECK(Contains(c.Client().GetJoinReport().fetched, "Images/new.png"));
    CHECK(Contains(c.Client().GetJoinReport().fetched, "Images/hosts.png"));

    // No show folder was written to.
    CHECK(FolderState(ms.show.dir.path / "Images") == hostBefore);
    CHECK(FolderState(da.dir.path) == aBefore);
    CHECK(FolderState(db.dir.path) == bBefore);
}

TEST_CASE("Damaged uploads are dropped and uploads stay inside the session folder", "[collab][collab_join]") {
    MediaShow ms;
    Device da(ms);
    Device db(ms);
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(da.path);
    Node& b = sim.AddClientOn(db.path);
    REQUIRE(sim.Quiesce());
    PeerId host = h.transport->LocalPeer();
    TempDir scratch;
    std::string file = scratch.Write("x.png", "payload");
    // A name no earlier run can have left behind (were the check broken).
    const std::string outside = "outside-" + scratch.path.filename().string() + ".png";
    const std::string climbing = "../../../" + outside;
    auto send = [&](BlobId id, const std::string& path, uint64_t hash, FileRole role = FileRole::Media) {
        Message m;
        m.type = MsgType::File;
        m.blob = id;
        m.files = { { path, role, 7, hash } };
        a.transport->SendBlob(host, id, file);
        a.transport->Send(host, m);
    };
    // Each is media a added, so what drops them is the check under test.
    for (const std::string& key : { std::string("Images/evil.png"), climbing, std::string(kLayoutFile), std::string("Images/o.png") }) {
        REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<PutMediaCommand>(MediaEntryState::FileReference(key, MediaType::Image))); }).ok);
    }
    REQUIRE(sim.Quiesce());
    send(900, "Images/evil.png", HashBytes("something else"));
    send(901, climbing, HashBytes("payload"));
    send(902, kLayoutFile, HashBytes("payload"), FileRole::Layout);
    // A blob nobody names, and a message whose blob never comes.
    a.transport->SendBlob(host, 903, file);
    Message orphan;
    orphan.type = MsgType::File;
    orphan.blob = 904;
    orphan.files = { { "Images/o.png", FileRole::Media, 7, HashBytes("payload") } };
    a.transport->Send(host, orphan);
    sim.RunFor(200);
    auto& got = h.session->ReceivedFiles();
    CHECK(got.count("Images/evil.png") == 0);
    CHECK(got.count(kLayoutFile) == 0);
    CHECK(got.count("Images/o.png") == 0);
    REQUIRE(got.count(climbing) == 1);
    CHECK(Norm(got.at(climbing)) == Norm((fs::path(h.session->SessionShowFolder()) / outside).string()));
    CHECK_FALSE(fs::exists(fs::path(h.session->SessionShowFolder()) / climbing));
    // Only the good one is passed on.
    CHECK(b.session->ReceivedFiles().size() == 1);
    CHECK(b.session->ReceivedFiles().count(climbing) == 1);
    RequireSameDocument(sim);
}

TEST_CASE("The host takes files only from editors, with media they added", "[collab][collab_join][collab_roles]") {
    MediaShow ms;
    Device da(ms);
    Device db(ms);
    WriteFile(da.dir.path / "Images" / "new.png", "new picture");
    WriteFile(da.dir.path / "Images" / "other.png", "not added");
    WriteFile(db.dir.path / "Images" / "new.png", "someone else's");
    Sim sim(ms.show, Lan(), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Node& a = sim.AddClientOn(da.path);
    Node& b = sim.AddClientOn(db.path);
    REQUIRE(sim.Quiesce());
    auto logged = [&](const std::string& text) {
        return std::any_of(h.log.begin(), h.log.end(), [&](const std::string& l) { return l.find(text) != std::string::npos; });
    };
    // a adds a media entry: its file goes everywhere (the normal flow).
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<PutMediaCommand>(MediaEntryState::FileReference("Images/new.png", MediaType::Image))); }).ok);
    RequireSameDocument(sim);
    REQUIRE(h.session->ReceivedFiles().count("Images/new.png") == 1);
    CHECK(ReadFile(h.session->ReceivedFiles().at("Images/new.png")) == "new picture");

    // A file with no media entry of its sender behind it is refused...
    REQUIRE(sim.As(a, [&] { return a.session->ShareFile("Images/other.png"); }));
    // ...and so is one for media someone else added.
    REQUIRE(sim.As(b, [&] { return b.session->ShareFile("Images/new.png"); }));
    sim.RunFor(500);
    CHECK(h.session->ReceivedFiles().count("Images/other.png") == 0);
    CHECK(b.session->ReceivedFiles().count("Images/other.png") == 0);
    CHECK(ReadFile(h.session->ReceivedFiles().at("Images/new.png")) == "new picture");
    CHECK(logged("refused 'Images/other.png' from participant " + std::to_string(a.session->Participant()) + ": not media it added"));
    CHECK(logged("refused 'Images/new.png' from participant " + std::to_string(b.session->Participant()) + ": not media it added"));

    // A viewer's file is refused, even for media it added as an editor.
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(a.session->Participant(), ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    WriteFile(da.dir.path / "Images" / "new.png", "changed by a viewer");
    REQUIRE(sim.As(a, [&] { return a.session->ShareFile("Images/new.png"); }));
    sim.RunFor(500);
    CHECK(ReadFile(h.session->ReceivedFiles().at("Images/new.png")) == "new picture");
    CHECK(logged("refused 'Images/new.png' from participant " + std::to_string(a.session->Participant()) + ": a viewer"));
}

namespace {
// Makes every file read for hashing slow, as hashing a large video is.
struct SlowHashing {
    std::atomic<int> reads{ 0 };
    explicit SlowHashing(int ms) {
        SetHashFileHook([this, ms](const std::string&) {
            ++reads;
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        });
    }
    ~SlowHashing() { SetHashFileHook(nullptr); }
};

double MsSince(std::chrono::steady_clock::time_point t) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t).count();
}
// Hashing happens on worker threads in real time: runs the session in small
// steps with real pauses until everyone has caught up.
bool QuiesceReal(Sim& sim, double* longest = nullptr) {
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (std::chrono::steady_clock::now() < end) {
        auto t = std::chrono::steady_clock::now();
        sim.RunFor(5);
        if (longest != nullptr) {
            *longest = std::max(*longest, MsSince(t));
        }
        if (sim.Quiet()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}
} // namespace

TEST_CASE("Hashing show files doesn't hold up the host or the joiner", "[collab][collab_join]") {
    MediaShow ms;
    Device da(ms);
    SessionOptions async = Opts();
    async.syncHashBytes = 0; // every file counts as large
    Sim sim(ms.show, Lan(), async);
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    SlowHashing slow(300);
    Node& a = sim.AddClientOn(da.path, async);
    // The Hello reaches the host, which lists its files; the manifest
    // reaches the joiner, which checks its own. Neither may wait 300 ms a file.
    double longest = 0;
    REQUIRE(QuiesceReal(sim, &longest));
    REQUIRE(a.Client().IsActive());
    CHECK(slow.reads > 0);
    INFO("longest step " << longest << " ms");
    CHECK(longest < 150);
    CHECK(h.session->HashedFiles() > 0);
    CHECK(a.session->HashedFiles() > 0);
}

TEST_CASE("Unchanged show files are hashed once; a changed one again", "[collab][collab_join]") {
    MediaShow ms;
    Device da(ms);
    Device db(ms);
    Device dc(ms);
    SessionOptions async = Opts();
    async.syncHashBytes = 0;
    Sim sim(ms.show, Lan(), async);
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    sim.AddClientOn(da.path, async);
    REQUIRE(QuiesceReal(sim));
    size_t first = h.session->HashedFiles();
    REQUIRE(first > 0);
    // A second joiner: the same files, nothing read again.
    sim.AddClientOn(db.path, async);
    REQUIRE(QuiesceReal(sim));
    CHECK(h.session->HashedFiles() == first);
    // One file changes (size and time): only it is read again.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    WriteFile(ms.show.dir.path / "Images" / "a.png", "picture-a, edited");
    sim.AddClientOn(dc.path, async);
    REQUIRE(QuiesceReal(sim));
    CHECK(h.session->HashedFiles() == first + 1);
}

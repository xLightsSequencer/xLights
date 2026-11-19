// Large transfers in a live session: files and snapshots on a slow link are
// asked for and sent once, the layout reaches a joiner however large it is,
// and a message too large to send is an error its sender sees.
#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "render/SequenceElementCommands.h"

using namespace Collab;
using namespace CollabTest;
namespace fs = std::filesystem;

namespace {

// A link that moves `bytesPerMs` of blob data per ms.
LoopbackOptions Slow(uint64_t bytesPerMs) {
    LoopbackOptions o;
    o.seed = 1;
    o.minDelayMs = 10;
    o.maxDelayMs = 10;
    o.blobBytesPerMs = bytesPerMs;
    return o;
}

SessionOptions Opts() {
    SessionOptions o;
    o.verifyHashes = true;
    return o;
}

// Counts every message by sender and type as it is sent.
using Counts = std::map<std::pair<PeerId, MsgType>, int>;
void CountMessages(LoopbackNetwork& net, Counts& counts) {
    net.tamper = [&counts](PeerId from, PeerId, std::string& frame) {
        if (auto m = DecodeMessage(frame)) {
            ++counts[{ from, m->type }];
        }
    };
}

PeerId PeerOf(Node& n) {
    return n.transport->LocalPeer();
}

void SendRaw(Node& from, Node& to, const Message& m) {
    from.transport->Send(PeerOf(to), m);
}

Effect* EffectAt(Node& n, const std::string& element, int startMS) {
    Element* el = n.Seq().GetElement(element);
    REQUIRE(el != nullptr);
    for (Effect* e : el->GetEffectLayer(0)->GetEffects()) {
        if (e->GetStartTimeMS() == startMS) {
            return e;
        }
    }
    return nullptr;
}

std::unique_ptr<SequenceCommand> AddOn(Node& n, const std::string& element, int start, int end, const std::string& settings = "") {
    EffectState s;
    s.name = "On";
    s.settings = settings;
    s.startTimeMS = start;
    s.endTimeMS = end;
    Element* el = n.Seq().GetElement(element);
    REQUIRE(el != nullptr);
    return std::make_unique<AddEffectCommand>(LocatorOf(el->GetEffectLayer(0)), s);
}

} // namespace

TEST_CASE("A snapshot on its way is neither asked for nor sent again", "[collab][collab_transfer]") {
    Show show;
    Sim sim(show, Slow(1), Opts());
    Node& h = sim.AddHost();
    size_t bytes = h.doc->Save().size();
    REQUIRE(bytes > 2000); // seconds on this link: more than one snapshot retry period
    Counts counts;
    CountMessages(sim.net, counts);
    Node& a = sim.AddClient();
    // Halfway, the client asks again (a duplicate on the network, or a
    // request made just before the snapshot started).
    sim.RunFor(bytes / 2);
    REQUIRE(a.Client().GetState() == CollabClient::State::AwaitingSnapshot);
    Message again;
    again.type = MsgType::NeedSnapshot;
    again.reason = "retry";
    SendRaw(a, h, again);
    REQUIRE(sim.Quiesce());
    REQUIRE(a.Client().IsActive());
    // The client didn't ask again while it was arriving...
    CHECK(counts[{ PeerOf(a), MsgType::NeedSnapshot }] == 2); // its own request and the forced one
    // ...and the host didn't start another copy for the repeated request.
    CHECK(sim.net.BlobsSent(PeerOf(h)) == 1);
    CHECK(counts[{ PeerOf(h), MsgType::Snapshot }] == 1);
}

TEST_CASE("Files on their way are neither asked for nor sent again", "[collab][collab_transfer]") {
    MediaShow ms;
    TempDir devDir;
    CopyShowFiles(ms.show.path, devDir.path, { kLayoutFile, kNetworksFile });
    TempDir session;
    SessionOptions fetching = Opts();
    fetching.fetchFiles = true;
    fetching.sessionFolder = (session.path / "a").string();
    fetching.fileRetryMs = 2000;
    Sim sim(ms.show, Slow(1), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    Counts counts;
    CountMessages(sim.net, counts);
    Node& a = sim.AddClientOn(devDir.path.string(), fetching);
    // song.wav alone takes 16 s on this link, many file retry periods.
    sim.RunFor(4000);
    REQUIRE(a.Client().GetState() == CollabClient::State::Checking);
    Message again;
    again.type = MsgType::NeedFiles;
    again.paths = { "song.wav" };
    SendRaw(a, h, again);
    REQUIRE(sim.Quiesce());
    REQUIRE(a.Client().IsActive());
    CHECK(a.Client().GetJoinReport().fetched == std::vector<std::string>{ "Images/a.png", "song.wav" });
    CHECK(counts[{ PeerOf(a), MsgType::NeedFiles }] == 2); // its own request and the forced one
    // The two files and the snapshot, each once.
    CHECK(sim.net.BlobsSent(PeerOf(h)) == 3);
    CHECK(counts[{ PeerOf(h), MsgType::File }] == 2);
}

TEST_CASE("A layout larger than a message reaches a joiner", "[collab][collab_transfer][collab_layout]") {
    Show show;
    // A layout far larger than any message may be on this network.
    std::string layout = ReadFile(show.dir.File(kLayoutFile));
    layout += "<!-- " + std::string(200000, 'x') + " -->\n";
    show.dir.Write(kLayoutFile, layout);
    LoopbackOptions o = Slow(0);
    o.maxMessageBytes = 64 * 1024;
    SessionOptions live = Opts();
    live.liveLayout = true;
    Sim sim(show, o, live);
    Node& h = sim.AddHost();
    LayoutFiles files;
    files.rgbEffects = layout;
    files.networks = ReadFile(show.dir.File(kNetworksFile));
    h.Host().EnableLiveLayout(files);
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());
    REQUIRE(a.Client().IsActive());
    CHECK(a.Client().LiveLayout());
    CHECK(a.Client().Layout() == files);
    CHECK(a.Client().LayoutHash() == h.Host().LayoutHash());
    // A re-sync brings the layout the same way.
    a.Client().RequestResync("test");
    REQUIRE(sim.Quiesce());
    CHECK(a.Client().ResyncCount() == 1);
    CHECK(a.Client().Layout() == files);
}

TEST_CASE("A proposal too large to send is rejected at once", "[collab][collab_transfer]") {
    Show show;
    LoopbackOptions o = Slow(0);
    o.maxMessageBytes = 64 * 1024;
    Sim sim(show, o, Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());
    std::string huge = "E_TEXTCTRL_Text=" + std::string(100000, 'x');
    SubmitResult r = sim.As(a, [&] { return a.session->Submit(AddOn(a, "Grid", 3000, 3100, huge)); });
    REQUIRE(r.ok);
    REQUIRE(sim.Quiesce());
    CHECK(a.rejects[Reason::TooLarge] == 1);
    CHECK(a.Client().PendingCount() == 0);
    CHECK(EffectAt(a, "Grid", 3000) == nullptr);
    CHECK(EffectAt(h, "Grid", 3000) == nullptr);
    // The session carries on.
    REQUIRE(sim.As(a, [&] { return a.session->Submit(AddOn(a, "Grid", 3000, 3100)); }).ok);
    // Its number was taken by an empty stand-in, so later proposals aren't
    // held waiting for it.
    REQUIRE(sim.Quiesce());
    CHECK(EffectAt(h, "Grid", 3000) != nullptr);
}

TEST_CASE("A commit too large to send reaches the clients as a snapshot", "[collab][collab_transfer]") {
    Show show;
    LoopbackOptions o = Slow(0);
    o.maxMessageBytes = 64 * 1024;
    Sim sim(show, o, Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    std::string huge = "E_TEXTCTRL_Text=" + std::string(100000, 'x');
    REQUIRE(sim.As(h, [&] { return h.session->Submit(AddOn(h, "Grid", 3000, 3100, huge)); }).ok);
    REQUIRE(sim.Quiesce());
    for (Node* c : { &a, &b }) {
        INFO("client " << PeerOf(*c));
        CHECK(c->Client().ResyncCount() == 1);
        REQUIRE(EffectAt(*c, "Grid", 3000) != nullptr);
        CHECK(DescribeSequence(c->Seq()) == DescribeSequence(h.Seq()));
        CHECK(DocHasher::FullHash(c->Seq()) == DocHasher::FullHash(h.Seq()));
    }
    // One snapshot each, not one per resend request.
    CHECK(sim.net.BlobsSent(PeerOf(h)) == 4); // two joins, two re-syncs
}

TEST_CASE("A joiner the session is too large to reach is refused with a reason", "[collab][collab_transfer]") {
    Show show;
    LoopbackOptions o = Slow(0);
    // So small that even the manifest can't go.
    o.maxMessageBytes = 200;
    Sim sim(show, o, Opts());
    sim.AddHost();
    Node& a = sim.AddClient();
    sim.RunFor(3000);
    CHECK(a.Client().GetState() == CollabClient::State::Ended);
    CHECK(a.Client().EndReason() == Reason::TooLarge);
    CHECK(a.Client().EndText() == "This session is too large to send to this device.");
}

TEST_CASE("A snapshot the transport loses without a word is sent again when asked", "[collab][collab_transfer]") {
    Show show;
    Sim sim(show, Slow(0), Opts());
    Node& h = sim.AddHost();
    int lost = 0;
    // The first blob to the joiner (its snapshot) vanishes: no delivery, no
    // report.
    sim.net.loseBlob = [&](PeerId from, PeerId, BlobId) { return from == PeerOf(h) && lost++ == 0; };
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce(30000));
    REQUIRE(a.Client().IsActive());
    CHECK(lost >= 1);
    CHECK(sim.net.BlobsSent(PeerOf(h)) == 2);
}

TEST_CASE("A file the transport loses without a word is sent again when asked", "[collab][collab_transfer]") {
    MediaShow ms;
    TempDir devDir;
    CopyShowFiles(ms.show.path, devDir.path, { kLayoutFile, kNetworksFile });
    TempDir session;
    SessionOptions fetching = Opts();
    fetching.fetchFiles = true;
    fetching.sessionFolder = (session.path / "a").string();
    fetching.fileRetryMs = 2000;
    Sim sim(ms.show, Slow(0), Opts());
    Node& h = sim.AddHost("", std::nullopt, ms.xsq);
    int lost = 0;
    sim.net.loseBlob = [&](PeerId from, PeerId, BlobId) { return from == PeerOf(h) && lost++ == 0; };
    Node& a = sim.AddClientOn(devDir.path.string(), fetching);
    REQUIRE(sim.Quiesce(60000));
    REQUIRE(a.Client().IsActive());
    CHECK(a.Client().GetJoinReport().fetched == std::vector<std::string>{ "Images/a.png", "song.wav" });
}

TEST_CASE("A layout the transport loses without a word reaches the joiner on its next hello", "[collab][collab_transfer][collab_layout]") {
    Show show;
    SessionOptions live = Opts();
    live.liveLayout = true;
    Sim sim(show, Slow(0), live);
    Node& h = sim.AddHost();
    LayoutFiles files;
    files.rgbEffects = ReadFile(show.dir.File(kLayoutFile));
    files.networks = ReadFile(show.dir.File(kNetworksFile));
    h.Host().EnableLiveLayout(files);
    int lost = 0;
    sim.net.loseBlob = [&](PeerId from, PeerId, BlobId) { return from == PeerOf(h) && lost++ == 0; };
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce(30000));
    REQUIRE(a.Client().IsActive());
    CHECK(a.Client().Layout() == files);
}

TEST_CASE("Loopback: a blob to a peer that goes is reported as not sent", "[collab][collab_transfer]") {
    LoopbackOptions o;
    o.minDelayMs = o.maxDelayMs = 50;
    LoopbackNetwork net(o);
    LoopbackTransport& from = net.AddPeer();
    LoopbackTransport& to = net.AddPeer();
    std::vector<std::pair<BlobId, bool>> done;
    from.onBlobDone = [&](PeerId, BlobId id, bool sent) { done.emplace_back(id, sent); };
    TempDir dir;
    from.SendBlob(to.LocalPeer(), 7, dir.Write("b", "data"));
    net.Disconnect(to.LocalPeer());
    net.RunUntilIdle();
    REQUIRE(done.size() == 1);
    CHECK(done[0] == std::make_pair(BlobId(7), false));
}

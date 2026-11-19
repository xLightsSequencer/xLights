#include <catch2/catch_test_macros.hpp>

#include "test_collab_common.h"

#include "collab/CollabCommandInfo.h"
#include "collab/CollabDocHash.h"
#include "collab/CollabUidTable.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceMediaCommands.h"
#include "utils/xlImage.h"

using namespace Collab;
using namespace CollabTest;

namespace {

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

EffectLayer* LayerOf(Node& n, const std::string& element, int layer = 0) {
    Element* el = n.Seq().GetElement(element);
    REQUIRE(el != nullptr);
    REQUIRE((int)el->GetEffectLayerCount() > layer);
    return el->GetEffectLayer(layer);
}

Effect* EffectAt(Node& n, const std::string& element, int startMS, int layer = 0) {
    for (Effect* e : LayerOf(n, element, layer)->GetEffects()) {
        if (e->GetStartTimeMS() == startMS) {
            return e;
        }
    }
    return nullptr;
}

std::string Setting(Node& n, const std::string& element, int startMS, const std::string& key) {
    Effect* e = EffectAt(n, element, startMS);
    REQUIRE(e != nullptr);
    return e->GetSettings().Get(key, std::string("<none>"));
}

std::unique_ptr<SequenceCommand> SetKey(Node& n, const std::string& element, int startMS, const std::string& key, const std::string& value) {
    Effect* e = EffectAt(n, element, startMS);
    REQUIRE(e != nullptr);
    return std::make_unique<ModifyEffectCommand>(LocatorOf(e), std::nullopt, SettingsChange{ false, { { key, value } } }, SettingsChange{});
}

std::unique_ptr<SequenceCommand> AddOn(Node& n, const std::string& element, int start, int end, int layer = 0) {
    EffectState s;
    s.name = "On";
    s.startTimeMS = start;
    s.endTimeMS = end;
    return std::make_unique<AddEffectCommand>(LocatorOf(LayerOf(n, element, layer)), s);
}

// Every joined client holds exactly the host's document, ids included.
// Collapsed / hidden rows are saved in the .xsq; a test that changes them
// compares files only after putting them back.
void RequireConverged(Sim& sim, bool compareFiles = true) {
    REQUIRE(sim.Quiesce());
    Node& h = sim.host();
    std::string hostDesc = DescribeSequence(h.Seq());
    std::string hostXsq = h.doc->Save();
    std::string problem;
    REQUIRE(h.Seq().ValidateUids(&problem));
    for (size_t i = 1; i < sim.nodes.size(); ++i) {
        Node& c = *sim.nodes[i];
        if (!c.Client().IsActive()) {
            continue;
        }
        INFO("client " << i);
        CHECK(DescribeSequence(c.Seq()) == hostDesc);
        if (compareFiles) {
            CHECK(c.doc->Save() == hostXsq);
        }
        CHECK(c.Seq().ValidateUids(&problem));
        CHECK(DocHasher::FullHash(c.Seq()) == DocHasher::FullHash(h.Seq()));
    }
}

} // namespace

TEST_CASE("Clients join with the host's document and ids", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    CHECK(a.Client().GetState() == CollabClient::State::Active);
    CHECK(b.Client().GetState() == CollabClient::State::Active);
    CHECK(a.session->Participant() != b.session->Participant());
    CHECK(a.session->Participant() > kHostParticipant);
    CHECK(h.Host().ClientCount() == 2);
    RequireConverged(sim);
}

TEST_CASE("A client with another protocol version is refused with a reason", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient(false);
    std::string ended;
    // Send a Hello from a future version by hand.
    Message hello;
    hello.type = MsgType::Hello;
    hello.protocol = kProtocolVersion + 1;
    std::optional<Message> reply;
    a.transport->onMessage = [&](PeerId, const Message& m) { reply = m; };
    a.transport->Send(h.transport->LocalPeer(), hello);
    sim.RunFor(50);
    REQUIRE(reply.has_value());
    CHECK(reply->type == MsgType::Refuse);
    CHECK(reply->reason == Reason::Protocol);
    CHECK(reply->text.find("Update xLights") != std::string::npos);
    CHECK(h.Host().ClientCount() == 0);
}

TEST_CASE("Structural edits wait for the commit; settings edits apply at once", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    sim.AddHost();
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());

    SubmitResult r = sim.As(a, [&] { return a.session->Submit(AddOn(a, "Grid", 5000, 5500)); });
    REQUIRE(r.ok);
    CHECK_FALSE(r.optimistic);
    CHECK(EffectAt(a, "Grid", 5000) == nullptr);
    r = sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_X", "7")); });
    REQUIRE(r.ok);
    CHECK(r.optimistic);
    CHECK(Setting(a, "Grid", 1000, "E_SLIDER_X") == "7");
    CHECK(Setting(sim.host(), "Grid", 1000, "E_SLIDER_X") == "<none>");
    RequireConverged(sim);
    CHECK(EffectAt(a, "Grid", 5000) != nullptr);
    CHECK(Setting(sim.host(), "Grid", 1000, "E_SLIDER_X") == "7");
    // The new effect carries the client's id prefix.
    CHECK(SeqIds::PeerOf(EffectAt(sim.host(), "Grid", 5000)->GetUid()) == a.session->Participant());
}

TEST_CASE("Concurrent adds to the same spot: the first commits, the second is rejected as an overlap", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    REQUIRE(sim.As(a, [&] { return a.session->Submit(AddOn(a, "Line", 1000, 2000)); }).ok);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(AddOn(b, "Line", 1500, 2500)); }).ok);
    RequireConverged(sim);
    CHECK(a.rejects.size() + b.rejects.size() == 1);
    CHECK(a.rejects[Reason::Overlap] + b.rejects[Reason::Overlap] == 1);
    CHECK(LayerOf(sim.host(), "Line")->GetEffectCount() == 2);
    CHECK(FindOverlap(sim.host().Seq()).empty());
}

TEST_CASE("Edits to an effect someone deleted are rejected; a second delete is a no-op", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    Effect* ea = EffectAt(a, "Grid", 2000);
    Effect* eb = EffectAt(b, "Grid", 2000);
    REQUIRE(ea->GetUid() == eb->GetUid());
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<DeleteEffectCommand>(LocatorOf(ea))); }).ok);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(std::make_unique<DeleteEffectCommand>(LocatorOf(eb))); }).ok);
    RequireConverged(sim);
    CHECK(a.rejects.empty());
    CHECK(b.rejects.empty());
    CHECK(h.Host().RejectCounts().empty());
    CHECK(EffectAt(h, "Grid", 2000) == nullptr);

    // Now a move and an optimistic settings write race a delete.
    Effect* ma = EffectAt(a, "Grid", 1000);
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<DeleteEffectCommand>(LocatorOf(ma))); }).ok);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(std::make_unique<MoveEffectCommand>(LocatorOf(EffectAt(b, "Grid", 1000)), 3000, 3200)); }).ok);
    // Moves apply at once on the client too.
    REQUIRE(EffectAt(b, "Grid", 3000) != nullptr);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(SetKey(b, "Grid", 3000, "E_SLIDER_X", "1")); }).ok);
    CHECK(Setting(b, "Grid", 3000, "E_SLIDER_X") == "1");
    RequireConverged(sim);
    CHECK(b.rejects[Reason::TargetGone] == 2);
    CHECK(EffectAt(b, "Grid", 1000) == nullptr);
    CHECK(EffectAt(b, "Grid", 3000) == nullptr);
    CHECK(b.Client().OverlayCount() == 0);
}

TEST_CASE("Settings: last writer per key wins and a pending local write never flickers", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    // a writes first, b second: both commit, b's (later rev) wins.
    REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_X", "a")); }).ok);
    sim.RunFor(1);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(SetKey(b, "Grid", 1000, "E_SLIDER_X", "b")); }).ok);
    // Different key on the same effect.
    REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_Y", "ya")); }).ok);
    // While a's commit reaches b first, b keeps showing its own pending value.
    bool flickered = false;
    while (!sim.Quiet()) {
        sim.RunFor(1, 1);
        if (b.Client().PendingCount() != 0 && Setting(b, "Grid", 1000, "E_SLIDER_X") != "b") {
            flickered = true;
        }
    }
    CHECK_FALSE(flickered);
    RequireConverged(sim);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "b");
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_Y") == "ya");
    CHECK(h.Host().RejectCounts().empty());
}

TEST_CASE("Changing an effect's type rejects a settings write made before it", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    Effect* e = EffectAt(a, "Grid", 1000);
    REQUIRE(sim.As(a, [&] {
                return a.session->Submit(std::make_unique<ModifyEffectCommand>(LocatorOf(e), std::string("Bars"), SettingsChange::ReplaceWith("E_SLIDER_Bars_BarCount=3"),
                                                                               SettingsChange{}));
            }).ok);
    sim.RunFor(1);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(SetKey(b, "Grid", 1000, "E_SLIDER_X", "stale")); }).ok);
    RequireConverged(sim);
    CHECK(b.rejects[Reason::StaleType] == 1);
    CHECK(EffectAt(h, "Grid", 1000)->GetEffectName() == "Bars");
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "<none>");
    // Made after seeing the type change, the same write is fine.
    REQUIRE(sim.As(b, [&] { return b.session->Submit(SetKey(b, "Grid", 1000, "E_SLIDER_X", "fresh")); }).ok);
    RequireConverged(sim);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "fresh");
}

TEST_CASE("A batch with a failing member is rejected whole", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());
    std::string before = DescribeSequence(h.Seq());
    auto batch = std::make_unique<BatchCommand>("Paste");
    batch->Add(AddOn(a, "Line", 3000, 3100));
    batch->Add(AddOn(a, "Grid", 1200, 1300)); // overlaps the On at 1000-1500
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::move(batch)); }).ok);
    RequireConverged(sim);
    CHECK(a.rejects[Reason::Overlap] == 1);
    CHECK(DescribeSequence(h.Seq()) == before);
}

TEST_CASE("Removing a layer first makes a concurrent add into it target_gone", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    Element* grid = a.Seq().GetElement("Grid");
    REQUIRE(sim.As(a, [&] {
                return a.session->Submit(std::make_unique<AddLayerCommand>(grid->GetUid(), kNoSeqObjectId, grid->GetEffectLayer(0)->GetUid()));
            }).ok);
    RequireConverged(sim);
    REQUIRE(h.Seq().GetElement("Grid")->GetEffectLayerCount() == 2);
    Element* ga = a.Seq().GetElement("Grid");
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<RemoveLayerCommand>(ga->GetUid(), ga->GetEffectLayer(1)->GetUid())); }).ok);
    sim.RunFor(1);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(AddOn(b, "Grid", 100, 200, 1)); }).ok);
    RequireConverged(sim);
    CHECK(b.rejects[Reason::TargetGone] == 1);
    CHECK(h.Seq().GetElement("Grid")->GetEffectLayerCount() == 1);
}

TEST_CASE("Undo is per user and refuses to overwrite someone else's later change", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    std::string start = DescribeSequence(h.Seq());

    REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_X", "a1")); }).ok);
    REQUIRE(sim.As(a, [&] { return a.session->Submit(AddOn(a, "Line", 3000, 3500)); }).ok);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(AddOn(b, "Line", 5000, 5500)); }).ok);
    RequireConverged(sim);
    REQUIRE(a.session->CanUndo());
    // a's undo removes a's effect, not b's.
    REQUIRE(sim.As(a, [&] { return a.session->Undo(); }).ok);
    RequireConverged(sim);
    CHECK(EffectAt(h, "Line", 3000) == nullptr);
    CHECK(EffectAt(h, "Line", 5000) != nullptr);
    // Redo puts it back with the same id.
    REQUIRE(a.session->CanRedo());
    REQUIRE(sim.As(a, [&] { return a.session->Redo(); }).ok);
    RequireConverged(sim);
    REQUIRE(EffectAt(h, "Line", 3000) != nullptr);
    CHECK(SeqIds::PeerOf(EffectAt(h, "Line", 3000)->GetUid()) == a.session->Participant());
    REQUIRE(sim.As(a, [&] { return a.session->Undo(); }).ok);
    RequireConverged(sim);

    // b changes the key a set; a's undo of that key is refused, naming b.
    REQUIRE(sim.As(b, [&] { return b.session->Submit(SetKey(b, "Grid", 1000, "E_SLIDER_X", "b1")); }).ok);
    RequireConverged(sim);
    REQUIRE(sim.As(a, [&] { return a.session->Undo(); }).ok);
    RequireConverged(sim);
    CHECK(a.rejects[Reason::ChangedSince] == 1);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "b1");
    CHECK_FALSE(a.session->CanUndo());

    // A change to a different key of the same effect doesn't block undo.
    REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_Z", "z")); }).ok);
    RequireConverged(sim);
    REQUIRE(sim.As(b, [&] { return b.session->Submit(SetKey(b, "Grid", 1000, "E_SLIDER_W", "w")); }).ok);
    RequireConverged(sim);
    REQUIRE(sim.As(a, [&] { return a.session->Undo(); }).ok);
    RequireConverged(sim);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_Z") == "<none>");
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_W") == "w");

    // The host undoes its own edits the same way.
    REQUIRE(sim.As(h, [&] { return h.session->Submit(AddOn(h, "Line", 7000, 7100)); }).ok);
    REQUIRE(sim.As(h, [&] { return h.session->Undo(); }).ok);
    RequireConverged(sim);
    CHECK(EffectAt(h, "Line", 7000) == nullptr);
}

TEST_CASE("Objects created on different devices get different ids", "[collab][collab_session]") {
    Show show;
    for (bool broken : { false, true }) {
        INFO((broken ? "control: shared id prefix" : "assigned prefixes"));
        Sim sim(show, Lan(), Opts());
        Node& h = sim.AddHost();
        Node& a = sim.AddClient();
        Node& b = sim.AddClient();
        a.session->faults.clientSharesHostIdPrefix = broken;
        b.session->faults.clientSharesHostIdPrefix = broken;
        REQUIRE(sim.Quiesce());
        // Every device's counter starts at the same value (Sim), as separate
        // processes' would.
        for (int i = 0; i < 4; ++i) {
            REQUIRE(sim.As(a, [&] { return a.session->Submit(AddOn(a, "Line", 1000 + i * 1000, 1100 + i * 1000)); }).ok);
            REQUIRE(sim.As(b, [&] { return b.session->Submit(AddOn(b, "Grid", 5000 + i * 1000, 5100 + i * 1000)); }).ok);
            REQUIRE(sim.As(h, [&] { return h.session->Submit(AddOn(h, "Beats", 9000 + i * 200, 9100 + i * 200)); }).ok);
        }
        REQUIRE(sim.Quiesce());
        size_t invalid = h.Host().RejectCounts().count(Reason::Invalid) ? h.Host().RejectCounts().at(Reason::Invalid) : 0;
        if (!broken) {
            CHECK(invalid == 0);
            CHECK(LayerOf(h, "Line")->GetEffectCount() == 5);
            CHECK(LayerOf(h, "Grid")->GetEffectCount() == 6);
            RequireConverged(sim);
        } else {
            // The host refuses colliding ids (work is lost) and its document
            // keeps every id unique.
            CHECK(invalid > 0);
            std::string problem;
            CHECK(h.Seq().ValidateUids(&problem));
        }
    }
}

TEST_CASE("A client that drifts re-syncs from a snapshot", "[collab][collab_session]") {
    Show show;
    for (bool detect : { true, false }) {
        INFO((detect ? "drift detection on" : "control: drift detection off"));
        Sim sim(show, Lan(), Opts());
        Node& h = sim.AddHost();
        Node& a = sim.AddClient();
        a.session->faults.clientIgnoresDrift = !detect;
        REQUIRE(sim.Quiesce());
        // Corrupt a's copy behind the session's back.
        {
            SeqMutationGuard::Scope s;
            EffectAt(a, "Grid", 1000)->SetSettings("E_SLIDER_Corrupt=1", false);
        }
        REQUIRE(sim.As(h, [&] { return h.session->Submit(SetKey(h, "Grid", 1000, "E_SLIDER_X", "1")); }).ok);
        REQUIRE(sim.Quiesce());
        bool same = DescribeSequence(a.Seq()) == DescribeSequence(h.Seq());
        if (detect) {
            CHECK(a.Client().ResyncCount() == 1);
            CHECK(same);
            RequireConverged(sim);
        } else {
            CHECK(a.Client().ResyncCount() == 0);
            CHECK_FALSE(same);
        }
    }
}

TEST_CASE("When the host goes away the session ends and clients keep their copy", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_X", "mine")); }).ok);
    sim.net.Disconnect(h.transport->LocalPeer());
    sim.RunFor(100);
    CHECK(a.Client().GetState() == CollabClient::State::Ended);
    CHECK(a.session->EndReason() == Reason::HostLost);
    CHECK(Setting(a, "Grid", 1000, "E_SLIDER_X") == "mine");
    CHECK_FALSE(a.doc->Save().empty());
    CHECK_FALSE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_X", "x")); }).ok);
    CHECK(b.session->EndReason() == Reason::HostLost);
}

TEST_CASE("A silent host times out; an ended session says so", "[collab][collab_session]") {
    Show show;
    {
        Sim sim(show, Lan(), Opts());
        Node& h = sim.AddHost();
        Node& a = sim.AddClient();
        REQUIRE(sim.Quiesce());
        // The host stops responding but the link stays up.
        sim.net.tamper = [&](PeerId from, PeerId, std::string& f) {
            if (from == h.transport->LocalPeer()) {
                f.clear();
            }
        };
        SessionOptions o;
        sim.RunFor(o.peerTimeoutMs + 2000, 50);
        CHECK(a.session->EndReason() == Reason::HostLost);
    }
    {
        Sim sim(show, Lan(), Opts());
        Node& h = sim.AddHost();
        Node& a = sim.AddClient();
        REQUIRE(sim.Quiesce());
        sim.As(h, [&] { h.Host().End(); });
        sim.RunFor(100);
        CHECK(a.session->EndReason() == Reason::Ended);
        Node& late = sim.AddClient();
        sim.RunFor(100);
        CHECK(late.session->EndReason() == Reason::Ended);
    }
}

TEST_CASE("Per-user state never crosses the wire", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    size_t commits = 0;
    bool clean = true;
    sim.net.tamper = [&](PeerId, PeerId, std::string& f) {
        auto m = DecodeMessage(f);
        if (m && (m->type == MsgType::Commit || m->type == MsgType::Propose)) {
            ++commits;
            clean = clean && HasNoLocalState(m->command);
        }
    };
    // b selects an effect and collapses / hides rows.
    EffectAt(b, "Grid", 1000)->SetSelected(1);
    b.Seq().GetElement("Grid")->SetCollapsed(true);
    b.Seq().GetElement("Line")->SetVisible(false);
    // a selects, then adds, moves and edits.
    EffectAt(a, "Grid", 2000)->SetSelected(1);
    auto add = AddOn(a, "Grid", 4000, 4100);
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::move(add)); }).ok);
    REQUIRE(sim.As(a, [&] { return a.session->Submit(std::make_unique<MoveEffectCommand>(LocatorOf(EffectAt(a, "Grid", 1000)), 1100, 1600)); }).ok);
    REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 2000, "E_SLIDER_X", "1")); }).ok);
    RequireConverged(sim, false);
    CHECK(commits >= 6);
    CHECK(clean);
    RequireConverged(sim, false);
    CHECK(EffectAt(b, "Grid", 1100)->GetSelected() == 1);
    CHECK(b.Seq().GetElement("Grid")->GetCollapsed());
    CHECK_FALSE(b.Seq().GetElement("Line")->GetVisible());
    CHECK(EffectAt(h, "Grid", 2000)->GetSelected() == 0);
    CHECK(EffectAt(h, "Grid", 4000)->GetSelected() == 0);
    CHECK(EffectAt(a, "Grid", 2000)->GetSelected() == 1);
    // Collapsed / visible rows are saved in the .xsq, so b's file matches the
    // host's once b's own view state is put back.
    b.Seq().GetElement("Grid")->SetCollapsed(false);
    b.Seq().GetElement("Line")->SetVisible(true);
    RequireConverged(sim, true);
}

TEST_CASE("Presence is relayed to everyone else and never touches the document", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    PresenceInfo p;
    p.startMS = 1000;
    p.endMS = 9000;
    p.selectedEffects = { EffectAt(a, "Grid", 1000)->GetUid() };
    p.softLock = p.selectedEffects[0];
    std::string before = DescribeSequence(h.Seq());
    std::optional<Message> atB;
    sim.net.tamper = [&](PeerId, PeerId to, std::string& f) {
        auto m = DecodeMessage(f);
        if (m && m->type == MsgType::Presence && to == b.transport->LocalPeer()) {
            atB = m;
        }
    };
    sim.As(a, [&] { a.session->SetPresence(p); });
    sim.RunFor(100);
    REQUIRE(atB.has_value());
    CHECK(atB->participant == a.session->Participant());
    CHECK(atB->presence == p);
    CHECK(DescribeSequence(h.Seq()) == before);
    CHECK(h.Host().HeadRev() == 0);
}

TEST_CASE("A late joiner and a lossy, reordering network still converge", "[collab][collab_session]") {
    Show show;
    LoopbackOptions o;
    o.seed = 77;
    o.minDelayMs = 1;
    o.maxDelayMs = 40;
    o.fifoPerLink = false;
    o.dropRate = 0.05;
    o.duplicateRate = 0.05;
    Sim sim(show, o, Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());
    for (int i = 0; i < 10; ++i) {
        REQUIRE(sim.As(a, [&] { return a.session->Submit(AddOn(a, "Line", 1000 + i * 300, 1200 + i * 300)); }).ok);
        REQUIRE(sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_SLIDER_X", std::to_string(i))); }).ok);
        REQUIRE(sim.As(h, [&] { return h.session->Submit(SetKey(h, "Grid", 2000, "E_SLIDER_Y", std::to_string(i))); }).ok);
        sim.RunFor(7);
    }
    Node& late = sim.AddClient();
    for (int i = 0; i < 5; ++i) {
        REQUIRE(sim.As(h, [&] { return h.session->Submit(AddOn(h, "Beats", 5000 + i * 100, 5050 + i * 100)); }).ok);
        sim.RunFor(3);
    }
    RequireConverged(sim);
    CHECK(late.Client().GetState() == CollabClient::State::Active);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "9");
    CHECK(sim.net.Dropped() > 0);
    CHECK(a.Client().ResyncCount() == 0);
}

TEST_CASE("The host survives malformed and hostile messages", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());
    Node& evil = sim.AddClient(false);
    std::mt19937 rng(42);

    // Valid commands to mutate.
    std::vector<nlohmann::json> seeds;
    Effect* e = EffectAt(h, "Grid", 1000);
    seeds.push_back(nlohmann::json::parse(AddOn(h, "Line", 3000, 3100)->ToJson()));
    seeds.push_back(nlohmann::json::parse(DeleteEffectCommand(LocatorOf(e)).ToJson()));
    seeds.push_back(nlohmann::json::parse(MoveEffectCommand(LocatorOf(e), 1000, 1700).ToJson()));
    seeds.push_back(nlohmann::json::parse(SetKey(h, "Grid", 1000, "E_X", "1")->ToJson()));
    seeds.push_back(nlohmann::json::parse(RemoveLayerCommand(h.Seq().GetElement("Grid")->GetUid(), 77).ToJson()));
    seeds.push_back(nlohmann::json::parse(SetTagCommand(3, 100).ToJson()));
    seeds.push_back(nlohmann::json::parse(RemoveMediaCommand("x.png").ToJson()));

    // Join the hostile peer properly so its proposals are processed.
    sim.Join(evil);
    REQUIRE(sim.Quiesce());
    PeerId hostPeer = h.transport->LocalPeer();
    uint64_t seq = 1;
    std::function<void(nlohmann::json&)> mutate = [&](nlohmann::json& j) {
        if (j.is_object() && !j.empty()) {
            auto it = j.begin();
            std::advance(it, rng() % j.size());
            switch (rng() % 6) {
            case 0: it.value() = (int64_t)(rng() % 3 == 0 ? -1 : (int64_t)rng()); break;
            case 1: it.value() = "garbage"; break;
            case 2: it.value() = nullptr; break;
            case 3: j.erase(it); break;
            case 4: it.value() = (uint64_t)rng() << 20; break;
            default: mutate(it.value()); break;
            }
        } else if (j.is_array() && !j.empty()) {
            mutate(j[rng() % j.size()]);
        }
    };
    for (int i = 0; i < 3000; ++i) {
        Message m;
        switch (rng() % 8) {
        case 0:
            m.type = MsgType::Resend;
            m.rev = rng() % 5 == 0 ? 0 : rng();
            break;
        case 1:
            m.type = MsgType::Propose;
            m.clientSeq = rng() % 3 == 0 ? rng() : seq;
            m.kind = (EditKind)(rng() % 3);
            m.undoOf = rng() % 10;
            m.baseRev = rng();
            m.command = nlohmann::json::object();
            break;
        default: {
            m.type = MsgType::Propose;
            m.clientSeq = seq++;
            m.baseRev = rng() % 4 == 0 ? (uint64_t)-1 : 0;
            m.kind = rng() % 5 == 0 ? EditKind::Undo : EditKind::Edit;
            m.undoOf = rng() % 20;
            m.command = seeds[rng() % seeds.size()];
            mutate(m.command);
            break;
        }
        }
        std::string frame = EncodeMessage(m);
        if (rng() % 4 == 0 && !frame.empty()) {
            frame[rng() % frame.size()] = (char)(rng() % 256);
        }
        // Straight onto the wire as a raw frame.
        sim.net.tamper = nullptr;
        if (auto dm = DecodeMessage(frame)) {
            evil.transport->Send(hostPeer, *dm);
        }
        if (i % 50 == 0) {
            sim.RunFor(20);
        }
    }
    REQUIRE(sim.Quiesce());
    std::string problem;
    CHECK(h.Seq().ValidateUids(&problem));
    CHECK(FindOverlap(h.Seq()).empty());
    // The honest client is still in step.
    CHECK(DescribeSequence(a.Seq()) == DescribeSequence(h.Seq()));
    CHECK(a.Client().ResyncCount() == 0);
    CHECK(h.session->HashVerifyFailures() == 0);
}

TEST_CASE("Media registered locally doesn't block a rename on the host or a client", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    // An effect names a file the sequence holds no entry for.
    REQUIRE(sim.As(h, [&] {
                EffectState s;
                s.name = "Pictures";
                s.settings = "E_TEXTCTRL_Pictures_Filename=Images/x.png";
                s.startTimeMS = 3000;
                s.endTimeMS = 3500;
                return h.session->Submit(std::make_unique<AddEffectCommand>(LocatorOf(LayerOf(h, "Line")), s));
            }).ok);
    auto img = MediaEntryState::EmbeddedImage("Images/a.png", xlImage(2, 2));
    REQUIRE(img.has_value());
    REQUIRE(sim.As(h, [&] { return h.session->Submit(std::make_unique<PutMediaCommand>(*img)); }).ok);
    REQUIRE_FALSE(h.Seq().GetSequenceMedia().HasMedia("Images/x.png"));
    // Loading the snapshot registers the file the effect names on the client;
    // a render would do the same on the host.
    Node& a = sim.AddClient(false);
    sim.Join(a);
    REQUIRE(sim.Quiesce());
    REQUIRE(a.Seq().GetSequenceMedia().HasMedia("Images/x.png"));
    (void)h.Seq().GetSequenceMedia().GetImage("Images/x.png");
    REQUIRE(h.Seq().GetSequenceMedia().HasMedia("Images/x.png"));
    auto rename = BuildRenameMedia(h.Seq(), "Images/a.png", "Images/x.png");
    REQUIRE(rename != nullptr);
    REQUIRE_FALSE(rename->IsEmpty());
    REQUIRE(sim.As(h, [&] { return h.session->Submit(std::move(rename)); }).ok);
    REQUIRE(sim.Quiesce());
    CHECK(a.Client().ResyncCount() == 0);
    RequireConverged(sim);
    CHECK(h.Seq().GetSequenceMedia().GetEntryState("Images/x.png")->embedded);
}

TEST_CASE("A rejected batch leaves the host's effect order as it was", "[collab][collab_session]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    REQUIRE(sim.Quiesce());
    // Moves don't re-sort a layer: Grid's layer is now out of time order.
    REQUIRE(sim.As(h, [&] { return h.session->Submit(std::make_unique<MoveEffectCommand>(LocatorOf(EffectAt(h, "Grid", 1000)), 3000, 3200)); }).ok);
    RequireConverged(sim);
    auto order = [](EffectLayer* l) {
        std::vector<std::pair<SeqObjectId, int>> o;
        for (Effect* e : l->GetEffects()) {
            o.push_back({ e->GetUid(), e->GetID() });
        }
        return o;
    };
    auto before = order(LayerOf(h, "Grid"));
    auto failing = [&](Node& n) {
        auto b = std::make_unique<BatchCommand>("Paste");
        b->Add(AddOn(n, "Grid", 5000, 5100));           // re-sorts the layer
        b->Add(AddOn(n, "Grid", 2100, 2200));           // overlaps Bars: fails
        return b;
    };
    // The hazard: rolling a failed strict batch back re-sorts the layer.
    {
        Doc scratch(show.path, show.dir.File("scratch"));
        std::string xsq;
        REQUIRE(h.doc->SaveXsq(xsq));
        REQUIRE(scratch.LoadXsq(xsq));
        REQUIRE(ApplyUids(scratch.Sequence(), CaptureUids(h.Seq())));
        EffectLayer* sl = scratch.Sequence().GetElement("Grid")->GetEffectLayer(0);
        REQUIRE(order(sl) == before);
        auto b = std::make_unique<BatchCommand>("Paste");
        EffectState s;
        s.name = "On";
        s.startTimeMS = 5000;
        s.endTimeMS = 5100;
        b->Add(std::make_unique<AddEffectCommand>(LocatorOf(sl), s));
        s.startTimeMS = 2100;
        s.endTimeMS = 2200;
        b->Add(std::make_unique<AddEffectCommand>(LocatorOf(sl), s));
        REQUIRE(b->Apply(scratch.Sequence(), ApplyMode::Strict) == nullptr);
        CHECK(order(sl) != before);
    }
    REQUIRE(sim.As(a, [&] { return a.session->Submit(failing(a)); }).ok);
    REQUIRE(sim.As(h, [&] { return h.session->Submit(failing(h)); }).ok == false);
    RequireConverged(sim);
    CHECK(a.rejects[Reason::Overlap] == 1);
    CHECK(order(LayerOf(h, "Grid")) == before);
}

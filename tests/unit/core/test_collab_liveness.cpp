// Keeping a session alive: a side that was blocked doesn't drop its peers,
// a dropped client is told, and a host that was replaced stands down.
#include <catch2/catch_test_macros.hpp>

#include "test_collab_common.h"

#include "render/SequenceElementCommands.h"

using namespace Collab;
using namespace CollabTest;

namespace {

LoopbackOptions Lan() {
    LoopbackOptions o;
    o.seed = 1;
    o.minDelayMs = 10;
    o.maxDelayMs = 10;
    return o;
}

SessionOptions Opts(uint64_t peerTimeoutMs = 15000) {
    SessionOptions o;
    o.verifyHashes = true;
    o.peerTimeoutMs = peerTimeoutMs;
    return o;
}

std::unique_ptr<SequenceCommand> AddOn(Node& n, const std::string& element, int start, int end) {
    EffectState s;
    s.name = "On";
    s.startTimeMS = start;
    s.endTimeMS = end;
    Element* el = n.Seq().GetElement(element);
    REQUIRE(el != nullptr);
    return std::make_unique<AddEffectCommand>(LocatorOf(el->GetEffectLayer(0)), s);
}

Effect* EffectAt(Node& n, const std::string& element, int startMS) {
    for (Effect* e : n.Seq().GetElement(element)->GetEffectLayer(0)->GetEffects()) {
        if (e->GetStartTimeMS() == startMS) {
            return e;
        }
    }
    return nullptr;
}

// Runs the network and ticks every node but `blocked` for `ms`: what reaches
// `blocked` is handled (with its clock where it stopped), as messages queued
// while an app's UI thread was busy are once it frees up.
void RunWithout(Sim& sim, Node& blocked, uint64_t ms) {
    uint64_t end = sim.net.Now() + ms;
    while (sim.net.Now() < end) {
        sim.net.RunUntil(std::min(end, sim.net.Now() + 5));
        for (auto& n : sim.nodes) {
            if (n.get() != &blocked) {
                sim.As(*n, [&] { n->session->Tick(sim.net.Now()); });
            }
        }
    }
}

bool LogHas(Node& n, const std::string& text) {
    return std::any_of(n.log.begin(), n.log.end(), [&](const std::string& l) { return l.find(text) != std::string::npos; });
}

} // namespace

TEST_CASE("A host blocked for 20 s drops nobody", "[collab][collab_liveness]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    // The client waits longer for the host than the host's stall lasts.
    Node& a = sim.AddClientOn("", Opts(60000));
    REQUIRE(sim.Quiesce());
    // While the host is blocked, the client edits; the proposal reaches the
    // host and is handled with the host's clock from before the stall.
    REQUIRE(sim.As(a, [&] { return a.session->Submit(AddOn(a, "Grid", 3000, 3100)); }).ok);
    RunWithout(sim, h, 20000);
    // The host's next tick is 20 s after its last.
    sim.RunFor(2000);
    CHECK(h.Host().ClientCount() == 1);
    CHECK(a.Client().IsActive());
    REQUIRE(sim.Quiesce());
    CHECK(EffectAt(h, "Grid", 3000) != nullptr);
    CHECK(LogHas(h, "ticks resumed after"));
}

TEST_CASE("A client blocked for 20 s doesn't give up on its host", "[collab][collab_liveness]") {
    Show show;
    Sim sim(show, Lan(), Opts(60000));
    sim.AddHost();
    Node& a = sim.AddClientOn("", Opts());
    REQUIRE(sim.Quiesce());
    // The host keeps pinging; the client handles the pings late.
    RunWithout(sim, a, 20000);
    sim.RunFor(2000);
    CHECK(a.Client().GetState() == CollabClient::State::Active);
    CHECK(LogHas(a, "ticks resumed after"));
    REQUIRE(sim.Quiesce());
}

TEST_CASE("A client the host stops hearing is dropped and told so", "[collab][collab_liveness]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    // It waits for the host far longer, so only the host's drop can end it.
    Node& a = sim.AddClientOn("", Opts(600000));
    REQUIRE(sim.Quiesce());
    sim.RunFor(1500); // it has answered pings: settled in, not loading
    PeerId ap = a.transport->LocalPeer();
    // A one-way link: nothing from the client gets through.
    sim.net.tamper = [&](PeerId from, PeerId, std::string& frame) {
        if (from == ap) {
            frame.clear();
        }
    };
    sim.RunFor(14000);
    CHECK(h.Host().ClientCount() == 1); // a real silence, not yet long enough
    sim.RunFor(3000);
    CHECK(h.Host().ClientCount() == 0);
    // Told, it ends with the reason rather than carrying on unheard.
    CHECK(a.Client().GetState() == CollabClient::State::Ended);
    CHECK(a.Client().EndReason() == Reason::TimedOut);
    CHECK(a.Client().EndText() == "The host stopped hearing from this device.");
}

TEST_CASE("Messages from a connection that isn't in the session are answered with a Bye", "[collab][collab_liveness]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& x = sim.AddClient(false);
    REQUIRE(sim.Quiesce());
    PeerId hp = h.transport->LocalPeer();
    PeerId xp = x.transport->LocalPeer();
    std::vector<Message> toX;
    sim.net.tamper = [&](PeerId from, PeerId to, std::string& frame) {
        if (from == hp && to == xp) {
            if (auto m = DecodeMessage(frame)) {
                toX.push_back(*m);
            }
        }
    };
    Message p;
    p.type = MsgType::Pong;
    for (int i = 0; i < 3; ++i) {
        x.transport->Send(hp, p);
    }
    sim.RunFor(200);
    REQUIRE(toX.size() == 1); // told once, not once per message
    CHECK(toX[0].type == MsgType::Bye);
    CHECK(toX[0].reason == Reason::NotInSession);
}

TEST_CASE("A client still loading its snapshot isn't dropped", "[collab][collab_liveness]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClientOn("", Opts(600000));
    PeerId ap = a.transport->LocalPeer();
    bool loading = false;
    // Once the snapshot is on its way, the client says nothing for 25 s (as
    // while it opens a large sequence).
    sim.net.tamper = [&](PeerId from, PeerId, std::string& frame) {
        if (from == ap && loading) {
            frame.clear();
        }
    };
    for (int i = 0; i < 2000 && !(a.Client().GetState() == CollabClient::State::Active); ++i) {
        sim.RunFor(5);
    }
    REQUIRE(a.Client().IsActive());
    loading = true;
    sim.RunFor(25000);
    loading = false;
    CHECK(h.Host().ClientCount() == 1);
    REQUIRE(sim.Quiesce());
    CHECK(a.Client().IsActive());
}

TEST_CASE("A host replaced while it was blocked stands down", "[collab][collab_liveness][migrate]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = sim.AddHost();
    Node& a = sim.AddClient();
    Node& b = sim.AddClient();
    REQUIRE(sim.Quiesce());
    sim.RunFor(1500); // a ping round: everyone knows a takes over
    a.migrates = b.migrates = true;
    // The host is blocked; its clients give up on it and move to a.
    RunWithout(sim, h, 16000);
    REQUIRE(a.Client().GetState() == CollabClient::State::Migrating);
    REQUIRE(b.Client().GetState() == CollabClient::State::Migrating);
    sim.As(a, [&] { a.Client().BeginSuccession(); });
    sim.TakeOver(a);
    PeerId ap = a.transport->LocalPeer();
    sim.As(b, [&] { b.Client().Rejoin(ap); });
    RunWithout(sim, h, 4000);
    REQUIRE(b.Client().GetState() == CollabClient::State::Active);
    // The old host resumes: it learns its clients moved on and ends rather
    // than run a second session.
    sim.RunFor(500);
    CHECK_FALSE(h.Host().IsActive());
    CHECK(h.Host().EndReason() == Reason::HostChanged);
    CHECK(h.Host().EndText() == "Another device took over this session while this one was not responding.");
    // Someone joining the old host now is turned away.
    Node& c = sim.AddClient();
    sim.RunFor(500);
    CHECK(c.Client().GetState() == CollabClient::State::Ended);
    // The new session carries on.
    REQUIRE(sim.As(b, [&] { return b.session->Submit(AddOn(b, "Grid", 3000, 3100)); }).ok);
    sim.RunFor(500);
    CHECK(EffectAt(a, "Grid", 3000) != nullptr);
    CHECK(EffectAt(b, "Grid", 3000) != nullptr);
}

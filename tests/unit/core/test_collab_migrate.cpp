// Host migration: when the host of a live session is lost, a participant
// takes over and the others carry on with it. Each simulated device has its
// own document, id allocator and CollabController, over a LoopbackNetwork
// with a transport that can move a session to a successor the way the LAN
// transport does (a new listener every participant can derive the address
// of).

#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "collab/CollabController.h"
#include "collab/CollabNet.h"
#include "collab/CollabNetWire.h"
#include "render/SequenceCommands.h"
#include "render/UndoManager.h"
#include "utils/LayoutLock.h"

using namespace Collab;
using namespace CollabTest;

namespace {

class MigrateApp : public ICollabApp {
public:
    MigrateApp(const std::string& show, const std::string& file) : _show(show), _file(file) {}

    void Open(const std::string& xsq) { REQUIRE(Load(xsq)); }

    SequenceElements& Sequence() override { return _ctx->GetSequenceElements(); }
    bool HasSequence() override { return _ctx != nullptr; }
    bool SaveXsq(std::string& out) override {
        out = SaveAs(_file + ".snap.xsq");
        return !out.empty();
    }
    bool LoadXsq(const std::string& xsq) override {
        std::string p = _file + ".load.xsq";
        std::ofstream(p, std::ios::binary) << xsq;
        ++loads;
        return Load(p);
    }
    std::string ShowFolder() override { return _show; }
    bool UseShowFolder(const std::string& folder) override {
        _show = folder;
        InstallDirs();
        return true;
    }
    void AddMediaFolder(const std::string& folder) override {
        _media.push_back(folder);
        InstallDirs();
    }
    void InstallDirs() const {
        FileUtils::SetFixFileShowDir(_show);
        FileUtils::SetFixFileDirectories(_media);
    }
    std::string SaveAs(const std::string& path) {
        _seqFile->SetFullPath(path);
        if (!_seqFile->Save(Sequence())) {
            return "";
        }
        return ReadFile(path);
    }
    std::string Save() { return SaveAs(_file + ".final.xsq"); }

    void SetCollaborationLock(bool l, const std::string& reason) override {
        locked = l;
        lockReason = reason;
        if (_ctx) {
            _ctx->SetCollaborationLock(l, reason);
        }
    }
    void MarkSequenceModified() override { ++modified; }
    void OnRejected(const RejectNotice& n) override { rejects.push_back(n); }
    void OnHostChanged(const HostChange& c) override { hostChanges.push_back(c); }
    void OnPresenceChanged(ParticipantId who) override { presenceEvents.push_back(who); }
    void OnSessionNotice(const std::string& text) override { notices.push_back(text); }

    bool locked = false;
    std::string lockReason;
    std::vector<std::string> notices;
    int modified = 0;
    int loads = 0;
    std::vector<RejectNotice> rejects;
    std::vector<HostChange> hostChanges;
    std::vector<ParticipantId> presenceEvents;

private:
    bool Load(const std::string& xsq) {
        _seqFile.reset();
        _ctx.reset();
        _ctx = std::make_unique<HeadlessRenderContext>();
        if (!_ctx->LoadShowFolder(_show, _media)) {
            return false;
        }
        _seqFile = std::make_unique<SequenceFile>(xsq);
        auto doc = _seqFile->Open(_show, false, xsq);
        if (!doc || !_ctx->LoadSequenceElements(*_seqFile, *doc)) {
            return false;
        }
        if (locked) {
            _ctx->SetCollaborationLock(true, "");
        }
        return true;
    }

    std::string _show;
    std::string _file;
    std::list<std::string> _media;
    std::unique_ptr<HeadlessRenderContext> _ctx;
    std::unique_ptr<SequenceFile> _seqFile;
};

// Sessions listening on the simulated network, by id.
struct Directory {
    struct Entry {
        PeerId peer = kNoPeer;
        std::string code;
        SessionAdvert advert;
    };
    std::map<std::string, Entry> sessions;
};

// An INetTransport over one LoopbackNetwork peer. A successor listens under
// "<session id>/<participant>", which every participant can work out.
class LoopNet : public INetTransport {
public:
    LoopNet(LoopbackTransport& t, LoopbackNetwork& net, Directory& dir, Dispatcher d) : _t(t), _net(net), _dir(dir), _dispatch(std::move(d)) {
        _t.onMessage = [this](PeerId from, const Message& m) {
            if (onMessage && !_closed) {
                onMessage(from, m);
            }
        };
        _t.onPeer = [this](PeerId p, PeerEvent e) {
            if (onPeer && !_closed) {
                onPeer(p, e);
            }
        };
        _t.onBlob = [this](PeerId from, BlobId id, const std::string& path) {
            if (onBlob && !_closed) {
                onBlob(from, id, path);
            }
        };
        _t.onBlobProgress = [this](PeerId from, BlobId id, uint64_t received, uint64_t size) {
            if (onBlobProgress && !_closed) {
                onBlobProgress(from, id, received, size);
            }
        };
        _t.onBlobDone = [this](PeerId to, BlobId id, bool sent) {
            if (onBlobDone && !_closed) {
                onBlobDone(to, id, sent);
            }
        };
        _t.onSendFailed = [this](PeerId to, const Message& m, const std::string& reason) {
            if (onSendFailed && !_closed) {
                onSendFailed(to, m, reason);
            }
        };
    }
    ~LoopNet() override {
        *_alive = false;
        _t.onMessage = nullptr;
        _t.onPeer = nullptr;
        _t.onBlob = nullptr;
        _t.onBlobProgress = nullptr;
        _t.onBlobDone = nullptr;
        _t.onSendFailed = nullptr;
        Unlist();
    }
    PeerId LocalPeer() const override { return _t.LocalPeer(); }
    void Send(PeerId to, const Message& m) override {
        if (!_closed) {
            _t.Send(to, m);
        }
    }
    void Broadcast(const Message& m) override {
        if (!_closed) {
            _t.Broadcast(m);
        }
    }
    void SendBlob(PeerId to, BlobId id, const std::string& path) override {
        if (!_closed) {
            _t.SendBlob(to, id, path);
        }
    }
    bool Listen(const std::string& code, const SessionAdvert& advert, std::string& /*error*/) override {
        List("session-" + std::to_string(_t.LocalPeer()), code, advert);
        return true;
    }
    void Connect(const std::string& id, const std::string& code) override {
        auto it = _dir.sessions.find(id);
        if (it == _dir.sessions.end() || it->second.code != code) {
            _state = NetState::Failed;
            if (onState) {
                onState(NetState::Failed, "The pairing code is wrong.");
            }
            return;
        }
        _state = NetState::Connected;
        if (onPeer) {
            onPeer(it->second.peer, PeerEvent::Joined);
        }
    }
    void Close() override {
        _closed = true;
        _state = NetState::Closed;
        Unlist();
    }
    NetState State() const override { return _state; }

    bool SupportsMigration() const override { return migratable; }
    std::string SuccessorSessionId(const Succession& s) const override {
        return s.sessionId.empty() ? std::string() : s.sessionId + "/" + std::to_string(s.successor);
    }
    bool ListenAsSuccessor(const Succession& s, const SessionAdvert& advert, std::string& error) override {
        ++listens;
        std::string id = SuccessorSessionId(s);
        if (id.empty() || failListen) {
            error = "Could not listen.";
            return false;
        }
        List(id, s.pairingCode, advert);
        _state = NetState::Listening;
        return true;
    }
    // Asynchronous, like a real connection: it is there or it isn't.
    void ConnectToSuccessor(const Succession& s) override {
        ++connects;
        std::string id = SuccessorSessionId(s);
        std::string code = s.pairingCode;
        _state = NetState::Connecting;
        auto alive = _alive;
        _dispatch([this, alive, id, code] {
            if (!*alive || _closed) {
                return;
            }
            auto it = _dir.sessions.find(id);
            if (it == _dir.sessions.end() || it->second.code != code || !_net.IsConnected(it->second.peer) || !_net.IsConnected(_t.LocalPeer())) {
                _state = NetState::Failed;
                if (onState) {
                    onState(NetState::Failed, "The session is no longer on the network.");
                }
                return;
            }
            _state = NetState::Connected;
            if (onPeer) {
                onPeer(it->second.peer, PeerEvent::Joined);
            }
        });
    }

    bool migratable = true;
    bool failListen = false;
    int listens = 0;
    int connects = 0;
    std::string id;

private:
    void List(const std::string& newId, const std::string& code, const SessionAdvert& advert) {
        Unlist();
        id = newId;
        Directory::Entry e{ _t.LocalPeer(), code, advert };
        e.advert.id = id;
        _dir.sessions[id] = e;
    }
    void Unlist() {
        auto it = _dir.sessions.find(id);
        if (!id.empty() && it != _dir.sessions.end() && it->second.peer == _t.LocalPeer()) {
            _dir.sessions.erase(it);
        }
    }

    LoopbackTransport& _t;
    LoopbackNetwork& _net;
    Directory& _dir;
    Dispatcher _dispatch;
    std::shared_ptr<bool> _alive = std::make_shared<bool>(true);
    bool _closed = false;
    NetState _state = NetState::Idle;
};

class LoopBrowser : public ISessionBrowser {
public:
    explicit LoopBrowser(Directory& dir) : _dir(dir) {}
    void Start() override {
        std::vector<SessionAdvert> list;
        for (const auto& [id, e] : _dir.sessions) {
            list.push_back(e.advert);
        }
        if (onChanged) {
            onChanged(list);
        }
    }
    void Stop() override {}

private:
    Directory& _dir;
};

struct Peer {
    std::string name;
    IdState ids;
    std::unique_ptr<MigrateApp> app;
    LoopbackTransport* lt = nullptr;
    LoopNet* net = nullptr;
    std::deque<std::function<void()>> posted;
    std::unique_ptr<CollabController> ctl;
    bool failListen = false; // for every transport this device makes

    SequenceElements& Seq() { return app->Sequence(); }
    CommandExecutor& Exec() { return Seq().GetCommandExecutor(); }
    UndoManager& Undo() { return Seq().get_undo_mgr(); }
    CollabClient& Client() {
        // Checked: after a host migration a device's session changes kind.
        auto* s = dynamic_cast<CollabClient*>(ctl->Session());
        REQUIRE(s != nullptr);
        return *s;
    }
    CollabHost& Host() {
        // Checked: after a host migration a device's session changes kind.
        auto* s = dynamic_cast<CollabHost*>(ctl->Session());
        REQUIRE(s != nullptr);
        return *s;
    }
    bool IsHost() const { return ctl->GetRole() == CollabController::Role::Host; }
};

class World {
public:
    explicit World(Show& show, LoopbackOptions o = {}) : net(o), _show(show) {
        _baseCounter = SeqIds::Counter().load() + 1000;
        net.deliveryWrapper = [this](PeerId to, const std::function<void()>& deliver) {
            Scope s(*peers[to - 1]);
            deliver();
        };
    }
    ~World() {
        for (auto& p : peers) {
            Scope s(*p);
            p->ctl.reset();
            p->app.reset();
        }
    }

    struct Scope {
        ActAs ids;
        explicit Scope(Peer& p) : ids(p.ids) {
            if (p.app) {
                p.app->InstallDirs();
            }
        }
    };

    Peer& Add(const std::string& name, const std::string& device = "mac") {
        auto p = std::make_unique<Peer>();
        Peer* pp = p.get();
        pp->name = name;
        pp->ids.counter = _baseCounter;
        pp->lt = &net.AddPeer();
        pp->app = std::make_unique<MigrateApp>(_show.path, _show.dir.File("dev" + std::to_string(peers.size())));
        peers.push_back(std::move(p));
        Scope s(*pp);
        pp->app->Open(_show.start);
        Dispatcher d = [pp](std::function<void()> fn) { pp->posted.push_back(std::move(fn)); };
        CollabNetFactory f;
        f.transport = [this, pp, d] {
            auto t = std::make_unique<LoopNet>(*pp->lt, net, dir, d);
            t->failListen = pp->failListen;
            pp->net = t.get();
            return t;
        };
        f.browser = [this] { return std::make_unique<LoopBrowser>(dir); };
        f.available = [] { return true; };
        f.pairingCode = [] { return std::string("424242"); };
        ControllerOptions opts;
        opts.appVersion = "test";
        opts.deviceKind = device;
        opts.session.verifyHashes = true;
        if (configure) {
            configure(opts);
        }
        pp->ctl = std::make_unique<CollabController>(*pp->app, std::move(f), d, opts);
        return *pp;
    }

    void HostOn(Peer& p) {
        Scope s(p);
        std::string err;
        REQUIRE(p.ctl->Host(p.name, "start.xsq", code, err));
        p.ctl->Tick(net.Now());
    }

    void JoinTo(Peer& p, const std::string& sessionId) {
        Scope s(p);
        std::string err;
        p.ctl->Tick(net.Now());
        REQUIRE(p.ctl->Join(p.name, sessionId, code, false, err));
    }

    bool Gone(const Peer& p) const { return !net.IsConnected(p.lt->LocalPeer()); }

    // The peer hosting now (still on the network), if any.
    Peer* CurrentHost() {
        for (auto& p : peers) {
            if (!Gone(*p) && p->IsHost() && p->ctl->GetState() == CollabController::State::Active) {
                return p.get();
            }
        }
        return nullptr;
    }

    void Pump() {
        for (bool any = true; any;) {
            any = false;
            for (auto& p : peers) {
                Scope s(*p);
                while (p->ctl && !p->posted.empty()) {
                    auto fn = std::move(p->posted.front());
                    p->posted.pop_front();
                    fn();
                    any = true;
                }
            }
        }
    }

    void Tick() {
        for (auto& p : peers) {
            Scope s(*p);
            if (p->ctl && !Gone(*p)) {
                p->ctl->Tick(net.Now());
            }
        }
    }

    void RunFor(uint64_t ms, uint64_t step = 5) {
        uint64_t end = net.Now() + ms;
        while (net.Now() < end) {
            Pump();
            net.RunUntil(std::min(end, net.Now() + step));
            Pump();
            Tick();
        }
        Pump();
    }

    // Everyone still on the network is in the session with one host and
    // has caught up with it.
    bool Quiet() {
        Pump();
        if (net.InFlight() != 0) {
            return false;
        }
        Peer* h = CurrentHost();
        if (h == nullptr) {
            return false;
        }
        Rev head = h->Host().HeadRev();
        for (auto& pp : peers) {
            Peer& p = *pp;
            if (&p == h || Gone(p)) {
                continue;
            }
            auto st = p.ctl->GetState();
            if (st == CollabController::State::Ended || st == CollabController::State::Idle) {
                continue;
            }
            if (st != CollabController::State::Active || p.IsHost() || p.Client().PendingCount() != 0 || p.Client().AppliedRev() != head ||
                p.ctl->HeldDeliveries() != 0) {
                return false;
            }
        }
        return true;
    }

    bool Quiesce(uint64_t limitMs = 60000) {
        uint64_t end = net.Now() + limitMs;
        while (net.Now() < end) {
            if (Quiet()) {
                return true;
            }
            net.RunUntil(net.Now() + 5);
            Pump();
            Tick();
        }
        return Quiet();
    }

    // The host goes away without a word (a crash, a dropped network).
    void Lose(Peer& p) { net.Disconnect(p.lt->LocalPeer()); }

    template<class F>
    auto As(Peer& p, F&& fn) {
        Scope s(p);
        return fn();
    }

    LoopbackNetwork net;
    Directory dir;
    std::vector<std::unique_ptr<Peer>> peers;
    std::string code;
    // Adjusts the options of every device added from now on.
    std::function<void(ControllerOptions&)> configure;

private:
    Show& _show;
    uint64_t _baseCounter;
};

EffectLayer* LayerOf(Peer& p, const std::string& element, int layer = 0) {
    Element* el = p.Seq().GetElement(element);
    REQUIRE(el != nullptr);
    REQUIRE((int)el->GetEffectLayerCount() > layer);
    return el->GetEffectLayer(layer);
}

Effect* EffectAt(Peer& p, const std::string& element, int startMS) {
    Element* el = p.Seq().GetElement(element);
    if (el == nullptr) {
        return nullptr;
    }
    for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
        for (Effect* e : el->GetEffectLayer((int)l)->GetEffects()) {
            if (e->GetStartTimeMS() == startMS) {
                return e;
            }
        }
    }
    return nullptr;
}

Effect* AddOn(World& w, Peer& p, const std::string& element, int start, int end) {
    return w.As(p, [&] {
        p.Undo().CreateUndoStep();
        return p.Exec().AddEffect(LayerOf(p, element), "On", "", "", start, end, 0, false);
    });
}

void SetKey(World& w, Peer& p, const std::string& element, int start, const std::string& key, const std::string& value) {
    w.As(p, [&] {
        Effect* e = EffectAt(p, element, start);
        REQUIRE(e != nullptr);
        p.Undo().CreateUndoStep();
        REQUIRE(p.Exec().SetEffectSetting(e, key, value));
    });
}

std::string Setting(Peer& p, const std::string& element, int start, const std::string& key) {
    Effect* e = EffectAt(p, element, start);
    REQUIRE(e != nullptr);
    return e->GetSettings().Get(key, std::string("<none>"));
}

// Every peer still in the session has the host's document.
void RequireConverged(World& w) {
    REQUIRE(w.Quiesce());
    Peer& h = *w.CurrentHost();
    std::string desc = DescribeSequence(h.Seq());
    std::string xsq = w.As(h, [&] { return h.app->Save(); });
    std::string problem;
    REQUIRE(h.Seq().ValidateUids(&problem));
    for (auto& pp : w.peers) {
        Peer& c = *pp;
        if (&c == &h || w.Gone(c) || c.ctl->GetState() != CollabController::State::Active) {
            continue;
        }
        INFO("peer " << c.name);
        CHECK(DescribeSequence(c.Seq()) == desc);
        CHECK(w.As(c, [&] { return c.app->Save(); }) == xsq);
        CHECK(DocHasher::FullHash(c.Seq()) == DocHasher::FullHash(h.Seq()));
        CHECK(c.Seq().ValidateUids(&problem));
        CHECK(c.Client().HashVerifyFailures() == 0);
    }
    CHECK(h.Host().HashVerifyFailures() == 0);
}

// A host and `clients` clients (Ari = 2, Bea = 3, Cy = 4), joined, caught up
// and ranked (a ping round has told everyone who takes over).
void StartSession(World& w, int clients) {
    Peer& h = w.Add("Hana");
    w.HostOn(h);
    const char* names[] = { "Ari", "Bea", "Cy" };
    for (int i = 0; i < clients; ++i) {
        Peer& c = w.Add(names[i], i % 2 ? "mac" : "ipad");
        w.JoinTo(c, "session-" + std::to_string(h.lt->LocalPeer()));
    }
    REQUIRE(w.Quiesce());
    w.RunFor(1500);
    REQUIRE(w.Quiesce());
    for (auto& p : w.peers) {
        REQUIRE(p->ctl->InSession());
    }
}

// Runs until the session has a host again and everyone caught up.
void RequireMigrated(World& w, Peer& expectedHost) {
    REQUIRE(w.Quiesce(30000));
    REQUIRE(w.CurrentHost() == &expectedHost);
}

} // namespace

TEST_CASE("Host migration: the host is lost with three clients and the best-ranked client takes over", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];

    // Edits from everyone, including undoable ones of Bea's.
    REQUIRE(AddOn(w, h, "Line", 1000, 1100) != nullptr);
    REQUIRE(AddOn(w, ari, "Line", 2000, 2100) != nullptr);
    REQUIRE(AddOn(w, bea, "Line", 3000, 3100) != nullptr);
    REQUIRE(AddOn(w, cy, "Line", 4000, 4100) != nullptr);
    RequireConverged(w);
    SetKey(w, bea, "Line", 3000, "E_SLIDER_Test", "7");
    RequireConverged(w);
    w.RunFor(1200);
    Rev before = h.Host().HeadRev();
    std::vector<ParticipantId> ranked{ 2, 3, 4 };
    CHECK(h.Host().RankSuccessors() == ranked);
    for (Peer* c : { &ari, &bea, &cy }) {
        INFO(c->name);
        CHECK(c->Client().Succession() == ranked);
        CHECK(c->ctl->HostParticipant() == 1);
    }
    CHECK(bea.ctl->StatusText() == "In Hana's session with 2 others");

    w.Lose(h);
    w.RunFor(1);
    CHECK(bea.ctl->GetState() == CollabController::State::Migrating);
    CHECK(bea.ctl->StatusText() == "Hana left - reconnecting...");
    RequireMigrated(w, ari);

    // Roles, names and who hosts, on every device.
    CHECK(ari.ctl->GetRole() == CollabController::Role::Host);
    CHECK(ari.ctl->LocalParticipant() == 2);
    CHECK_FALSE(ari.ctl->SaveAsOnly());
    for (Peer* c : { &ari, &bea, &cy }) {
        INFO(c->name);
        CHECK(c->ctl->InSession());
        CHECK(c->ctl->HostParticipant() == 2);
        CHECK(c->ctl->HostName() == "Ari");
        auto people = c->ctl->Participants();
        REQUIRE(people.size() == 3);
        CHECK(people[0].id == 2);
        CHECK(people[0].isHost);
        REQUIRE(c->app->hostChanges.size() == 1);
        CHECK(c->app->hostChanges[0].oldHost == 1);
        CHECK(c->app->hostChanges[0].newHost == 2);
        CHECK(c->ctl->Session()->Epoch() == 1);
    }
    CHECK(ari.app->hostChanges[0].becameHost);
    CHECK(ari.app->hostChanges[0].text == "Hana left; you are now hosting.");
    CHECK(bea.app->hostChanges[0].text == "Hana left; Ari is now hosting.");
    CHECK_FALSE(bea.app->hostChanges[0].becameHost);
    CHECK(bea.ctl->StatusText() == "In Ari's session with 1 other");
    CHECK(ari.ctl->StatusText() == "Hosting with 2 others");
    // The new host has unsaved changes; it never re-synced anyone.
    CHECK(ari.app->modified > 0);
    CHECK(ari.ctl->CanDriveControllers());
    CHECK_FALSE(bea.ctl->CanDriveControllers());
    // Revs carry on from where the old host was.
    CHECK(ari.Host().HeadRev() == before);
    CHECK(bea.Client().ResyncCount() == 0);
    CHECK(cy.Client().ResyncCount() == 0);
    CHECK(bea.app->loads == 1);

    // Editing carries on, and undo stacks survived.
    REQUIRE(AddOn(w, bea, "Grid", 5000, 5100) != nullptr);
    REQUIRE(AddOn(w, cy, "Grid", 6000, 6100) != nullptr);
    REQUIRE(AddOn(w, ari, "Grid", 7000, 7100) != nullptr);
    RequireConverged(w);
    CHECK(ari.Host().HeadRev() > before);
    CHECK(EffectAt(cy, "Grid", 5000) != nullptr);
    // Bea's undo: her new add, then the key she set before the host left.
    REQUIRE(w.As(bea, [&] { return bea.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(ari, "Grid", 5000) == nullptr);
    CHECK(w.As(bea, [&] { return bea.ctl->UndoLabel(); }) != "Undo");
    REQUIRE(w.As(bea, [&] { return bea.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(Setting(ari, "Line", 3000, "E_SLIDER_Test") == "<none>");
    CHECK(Setting(cy, "Line", 3000, "E_SLIDER_Test") == "<none>");
    // And the new host's own undo of an edit made while it was a client.
    REQUIRE(w.As(ari, [&] { return ari.Undo().UndoLastStep(); })); // Grid 7000
    REQUIRE(w.As(ari, [&] { return ari.Undo().UndoLastStep(); })); // Line 2000, from before
    RequireConverged(w);
    CHECK(EffectAt(bea, "Line", 2000) == nullptr);
    for (auto& p : w.peers) {
        CHECK(p->app->rejects.empty());
    }
}

TEST_CASE("Host migration: proposals the old host never answered go to the new one", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];
    REQUIRE(AddOn(w, h, "Line", 1000, 1100) != nullptr);
    RequireConverged(w);

    // Proposals on their way when the host goes: a structural add (pending
    // ghost), a key change (applied at once) and the successor's own edit.
    REQUIRE(AddOn(w, bea, "Line", 3000, 3100) != nullptr);
    SetKey(w, cy, "Line", 1000, "E_SLIDER_Cy", "5");
    REQUIRE(AddOn(w, ari, "Grid", 3000, 3100) != nullptr);
    w.Pump();
    CHECK(bea.Client().PendingCount() == 1);
    CHECK(EffectAt(bea, "Line", 3000) == nullptr); // waits for the commit
    CHECK(Setting(cy, "Line", 1000, "E_SLIDER_Cy") == "5");
    w.Lose(h); // everything in flight to it is gone
    w.RunFor(1);
    // While the host changes, edits keep being taken.
    REQUIRE(AddOn(w, cy, "Grid", 4000, 4100) != nullptr);
    w.Pump();
    CHECK(cy.ctl->GetState() == CollabController::State::Migrating);
    RequireMigrated(w, ari);
    RequireConverged(w);
    for (Peer* p : { &ari, &bea, &cy }) {
        INFO(p->name);
        CHECK(EffectAt(*p, "Line", 3000) != nullptr);
        CHECK(EffectAt(*p, "Grid", 3000) != nullptr);
        CHECK(EffectAt(*p, "Grid", 4000) != nullptr);
        CHECK(Setting(*p, "Line", 1000, "E_SLIDER_Cy") == "5");
        for (const auto& r : p->app->rejects) {
            UNSCOPED_INFO("reject " << r.id << " " << r.reason << " " << r.text);
        }
        CHECK(p->app->rejects.empty());
    }
    // Each was committed once (the start sequence has one on Line), and
    // undoes as its author's.
    int lines = w.As(ari, [&] { return (int)LayerOf(ari, "Line")->GetEffectCount(); });
    CHECK(lines == 3);
    REQUIRE(w.As(bea, [&] { return bea.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(cy, "Line", 3000) == nullptr);
}

TEST_CASE("Host migration: a proposal the old host rejected but whose answer was lost is rejected cleanly", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    // Bea's first add overlaps one Hana makes at the same time (the host
    // rejects Bea's), her second is fine. Every reject to her is lost.
    PeerId beaPeer = bea.lt->LocalPeer();
    w.net.tamper = [&](PeerId /*from*/, PeerId to, std::string& frame) {
        auto m = DecodeMessage(frame);
        if (to == beaPeer && m && m->type == MsgType::Reject) {
            frame.clear();
        }
    };
    REQUIRE(AddOn(w, h, "Line", 1000, 2000) != nullptr);
    REQUIRE(AddOn(w, bea, "Line", 1500, 1600) != nullptr);
    w.Pump();
    REQUIRE(AddOn(w, bea, "Line", 3000, 3100) != nullptr);
    w.RunFor(200);
    CHECK(EffectAt(bea, "Line", 3000) != nullptr); // committed and back
    CHECK(bea.Client().PendingCount() == 1);       // the overlap: no answer
    CHECK(bea.app->rejects.empty());
    w.net.tamper = nullptr;
    w.Lose(h);
    RequireMigrated(w, ari);
    RequireConverged(w);
    REQUIRE(bea.app->rejects.size() == 1);
    CHECK(bea.app->rejects[0].reason == Reason::HostChanged);
    CHECK(bea.Client().PendingCount() == 0);
    CHECK(w.As(ari, [&] { return (int)LayerOf(ari, "Line")->GetEffectCount(); }) == 3);
}

TEST_CASE("Host migration: commits only some clients received are not lost", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];
    Rev head = h.Host().HeadRev();
    // Hana's last two edits reach only Bea before she is lost: Ari (the
    // successor) and Cy are behind.
    PeerId from = h.lt->LocalPeer();
    PeerId toAri = ari.lt->LocalPeer();
    PeerId toCy = cy.lt->LocalPeer();
    w.net.tamper = [&](PeerId f, PeerId to, std::string& frame) {
        if (f == from && (to == toAri || to == toCy)) {
            frame.clear();
        }
    };
    REQUIRE(AddOn(w, h, "Line", 1000, 1100) != nullptr);
    w.Pump();
    REQUIRE(AddOn(w, h, "Grid", 3000, 3100) != nullptr);
    w.RunFor(50);
    CHECK(bea.Client().AppliedRev() == head + 2);
    CHECK(ari.Client().AppliedRev() == head);
    CHECK(cy.Client().AppliedRev() == head);
    w.Lose(h);
    w.net.tamper = nullptr;
    RequireMigrated(w, ari);
    RequireConverged(w);
    CHECK(ari.Host().HeadRev() == head + 2);
    for (Peer* p : { &ari, &bea, &cy }) {
        INFO(p->name);
        CHECK(EffectAt(*p, "Line", 1000) != nullptr);
        CHECK(EffectAt(*p, "Grid", 3000) != nullptr);
        // Ari is the host now; only the clients have a re-sync count.
        if (!p->IsHost()) {
            CHECK(p->Client().ResyncCount() == 0);
        }
    }
    // Cy caught up from the new host's log, not a snapshot.
    CHECK(cy.app->loads == 1);
}

TEST_CASE("Host migration: a client behind the new host's log catches up from one snapshot", "[collab][migrate]") {
    Show show;
    World w(show);
    // Every client keeps only the last two commits for a successor.
    w.configure = [](ControllerOptions& o) { o.session.clientLogLimit = 2; };
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Rev head = h.Host().HeadRev();
    // Bea misses Hana's next five commits; Ari gets them all.
    PeerId from = h.lt->LocalPeer();
    PeerId toBea = bea.lt->LocalPeer();
    w.net.tamper = [&](PeerId f, PeerId to, std::string& frame) {
        if (f == from && to == toBea) {
            auto m = DecodeMessage(frame);
            if (m && m->type == MsgType::Commit) {
                frame.clear();
            }
        }
    };
    for (int i = 0; i < 5; ++i) {
        REQUIRE(AddOn(w, h, "Line", 1000 + 200 * i, 1100 + 200 * i) != nullptr);
        w.RunFor(20);
    }
    w.RunFor(1200); // a ping round ranks Ari first: she kept up
    REQUIRE(ari.Client().AppliedRev() == head + 5);
    REQUIRE(bea.Client().AppliedRev() == head);
    REQUIRE(ari.Client().LoggedCommits() == 2);
    w.Lose(h);
    w.net.tamper = nullptr;
    // Ari's log starts after what Bea has: Bea gets a snapshot, once.
    size_t blobsBefore = w.net.BlobsSent(ari.lt->LocalPeer());
    RequireMigrated(w, ari);
    RequireConverged(w);
    w.RunFor(3000);
    RequireConverged(w);
    CHECK(bea.Client().AppliedRev() == head + 5);
    CHECK(bea.Client().ResyncCount() == 1);
    CHECK(w.net.BlobsSent(ari.lt->LocalPeer()) - blobsBefore == 1);
    for (int i = 0; i < 5; ++i) {
        CHECK(EffectAt(bea, "Line", 1000 + 200 * i) != nullptr);
    }
}

TEST_CASE("Host migration: clients that disagree about the successor still converge", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];
    // The last word Bea had about the succession was older and named Cy
    // first (as if the newer pings were lost on the way to her).
    PeerId toBea = bea.lt->LocalPeer();
    w.net.tamper = [&](PeerId /*f*/, PeerId to, std::string& frame) {
        auto m = DecodeMessage(frame);
        if (to == toBea && m && (m->type == MsgType::Ping || m->type == MsgType::Roster) && !m->succession.empty()) {
            m->succession = { 4, 2, 3 };
            frame = EncodeMessage(*m);
        }
    };
    w.RunFor(1500);
    REQUIRE(bea.Client().Succession() == std::vector<ParticipantId>{ 4, 2, 3 });
    REQUIRE(cy.Client().Succession() == std::vector<ParticipantId>{ 2, 3, 4 });
    REQUIRE(AddOn(w, bea, "Line", 3000, 3100) != nullptr);
    w.Pump();
    w.Lose(h);
    w.net.tamper = nullptr;
    // Bea looks for Cy, who isn't hosting; Ari carries on without her and
    // she reaches him once Cy's turn is over.
    w.RunFor(3000);
    CHECK(ari.ctl->GetRole() == CollabController::Role::Host);
    CHECK(ari.Host().ClientCount() == 1);
    CHECK(bea.ctl->GetState() == CollabController::State::Migrating);
    CHECK(bea.net->connects >= 2);
    RequireMigrated(w, ari);
    RequireConverged(w);
    CHECK(bea.ctl->HostParticipant() == 2);
    CHECK(EffectAt(cy, "Line", 3000) != nullptr);
    CHECK(cy.net->listens == 0);
    CHECK(bea.net->listens == 0);
    CHECK(ari.Host().ClientCount() == 2);
}

TEST_CASE("Host migration: when the successor can't take over the next one does", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    ari.net->failListen = true;
    REQUIRE(AddOn(w, ari, "Line", 2000, 2100) != nullptr);
    RequireConverged(w);
    w.Lose(h);
    RequireMigrated(w, bea);
    RequireConverged(w);
    CHECK(ari.net->listens == 1);
    CHECK(ari.ctl->HostParticipant() == 3);
    CHECK(ari.app->hostChanges.size() == 1);
    CHECK(ari.app->hostChanges[0].text == "Hana left; Bea is now hosting.");
    // Ari's undo stack still works against the new host.
    REQUIRE(w.As(ari, [&] { return ari.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(bea, "Line", 2000) == nullptr);
}

TEST_CASE("Host migration: with no one to take over the session ends as host lost", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    ari.net->failListen = true;
    bea.net->failListen = true;
    w.Lose(h);
    w.RunFor(30000);
    for (Peer* p : { &ari, &bea }) {
        INFO(p->name);
        CHECK(p->ctl->GetState() == CollabController::State::Ended);
        CHECK(p->ctl->EndReason() == Reason::HostLost);
        CHECK(p->ctl->EndText() == "The host left and no one could take over the session.");
        CHECK_FALSE(p->app->locked);
        CHECK(p->app->hostChanges.empty());
    }
}

TEST_CASE("Host migration: a transport that can't move a session ends it as before", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    ari.net->migratable = false;
    bea.net->migratable = false;
    w.Lose(h);
    w.RunFor(200);
    for (Peer* p : { &ari, &bea }) {
        INFO(p->name);
        CHECK(p->ctl->GetState() == CollabController::State::Ended);
        CHECK(p->ctl->EndReason() == Reason::HostLost);
    }
}

TEST_CASE("Host migration: the new host can end the session and a returning host joins as a client", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    REQUIRE(AddOn(w, bea, "Line", 3000, 3100) != nullptr);
    RequireConverged(w);
    w.Lose(h);
    RequireMigrated(w, ari);
    CHECK(w.dir.sessions.count("session-1/2") == 1);
    CHECK(ari.ctl->Session()->LoggedCommits() > 0);

    // Hana's device comes back (a new connection) and joins Ari's session
    // the usual way: a new participant id, never one used before.
    Peer& back = w.Add("Hana");
    w.JoinTo(back, "session-1/2");
    REQUIRE(w.Quiesce());
    RequireConverged(w);
    CHECK(back.ctl->GetRole() == CollabController::Role::Client);
    CHECK(back.ctl->LocalParticipant() == 4);
    CHECK(back.ctl->HostName() == "Ari");
    CHECK(EffectAt(back, "Line", 3000) != nullptr);
    REQUIRE(AddOn(w, back, "Grid", 3000, 3100) != nullptr);
    RequireConverged(w);
    CHECK(EffectAt(bea, "Grid", 3000) != nullptr);

    // Ari ends it: it ends for everyone; nobody takes over.
    w.As(ari, [&] { ari.ctl->End(); });
    w.RunFor(3000);
    for (Peer* p : { &ari, &bea, &back }) {
        INFO(p->name);
        CHECK(p->ctl->GetState() == CollabController::State::Ended);
        CHECK(p->ctl->EndReason() == Reason::Ended);
    }
    CHECK(bea.ctl->EndText() == "The host ended the session.");
    CHECK(bea.app->hostChanges.size() == 1);
}

TEST_CASE("Host migration: a session of two carries on with the one left", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    REQUIRE(AddOn(w, h, "Line", 1000, 1100) != nullptr);
    RequireConverged(w);
    w.Lose(h);
    w.RunFor(3000);
    REQUIRE(w.CurrentHost() == &ari);
    CHECK(ari.ctl->StatusText() == "Hosting - no one has joined yet");
    CHECK(EffectAt(ari, "Line", 1000) != nullptr);
    REQUIRE(AddOn(w, ari, "Grid", 3000, 3100) != nullptr);
    w.RunFor(100);
    CHECK(EffectAt(ari, "Grid", 3000) != nullptr);
    CHECK(ari.Host().HeadRev() >= 2);
}

TEST_CASE("Host migration: messages carry the rejoin, ranking and roster fields", "[collab][migrate]") {
    Message h;
    h.type = MsgType::Hello;
    h.rejoin = true;
    h.participant = 7;
    h.rev = 42;
    h.clientSeq = 9;
    auto hd = DecodeMessage(EncodeMessage(h));
    REQUIRE(hd);
    CHECK(hd->rejoin);
    CHECK(hd->participant == 7);
    CHECK(hd->rev == 42);
    CHECK(hd->clientSeq == 9);

    Message plain;
    plain.type = MsgType::Hello;
    auto pd = DecodeMessage(EncodeMessage(plain));
    REQUIRE(pd);
    CHECK_FALSE(pd->rejoin);
    // A rejoin must say who it was.
    std::string bad = EncodeMessage(h);
    auto at = bad.find("\"participant\":7");
    REQUIRE(at != std::string::npos);
    bad.replace(at, 15, "\"participant\":0");
    CHECK_FALSE(DecodeMessage(bad));

    Message w;
    w.type = MsgType::Welcome;
    w.participant = 7;
    w.rejoin = true;
    w.clientSeq = 5;
    auto wd = DecodeMessage(EncodeMessage(w));
    REQUIRE(wd);
    CHECK(wd->rejoin);
    CHECK(wd->clientSeq == 5);

    Message p;
    p.type = MsgType::Ping;
    p.succession = { 3, 2, 9 };
    auto pg = DecodeMessage(EncodeMessage(p));
    REQUIRE(pg);
    CHECK(pg->succession == p.succession);
    Message pong;
    pong.type = MsgType::Pong;
    pong.active = true;
    CHECK(DecodeMessage(EncodeMessage(pong))->active);

    Message r;
    r.type = MsgType::Roster;
    r.roster = { { 2, "Ari", "mac" } };
    r.epoch = 3;
    r.nextParticipant = 12;
    r.succession = { 2 };
    auto rd = DecodeMessage(EncodeMessage(r));
    REQUIRE(rd);
    CHECK(rd->epoch == 3);
    CHECK(rd->nextParticipant == 12);
    CHECK(rd->succession == r.succession);
    // A succession entry that is no participant is malformed.
    std::string s = EncodeMessage(r);
    at = s.find("\"succ\":[2]");
    REQUIRE(at != std::string::npos);
    s.replace(at, 10, "\"succ\":[0]");
    CHECK_FALSE(DecodeMessage(s));
}

TEST_CASE("Host migration: every participant derives the same successor service name", "[collab][migrate]") {
    std::string salt = Wire::NewSalt();
    std::string id = Wire::ServiceNameFor(salt);
    std::string a, b, c, renamed;
    REQUIRE(Wire::SuccessorSalt(id, 2, a));
    REQUIRE(Wire::SuccessorSalt(id, 2, b));
    REQUIRE(Wire::SuccessorSalt(id, 3, c));
    REQUIRE(Wire::SuccessorSalt(id + " (2)", 2, renamed));
    CHECK(a.size() == Wire::kSaltBytes);
    CHECK(a == b);
    CHECK(a != c);
    CHECK(a == renamed);
    CHECK(a != salt);
    // A successor's successor is another name again.
    std::string next;
    REQUIRE(Wire::SuccessorSalt(Wire::ServiceNameFor(a), 2, next));
    CHECK(next != a);
    std::string none;
    CHECK_FALSE(Wire::SuccessorSalt("tcp:127.0.0.1:5000:0011223344556677", 2, none));
}

TEST_CASE("Host migration: a client cut off from a host that is still there doesn't carry on alone", "[collab][migrate]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];
    // Only Bea's link to the host drops; everyone else carries on with Hana.
    PeerId hostPeer = h.lt->LocalPeer();
    PeerId beaPeer = bea.lt->LocalPeer();
    w.net.tamper = [&](PeerId f, PeerId to, std::string& frame) {
        if ((f == hostPeer && to == beaPeer) || (f == beaPeer && to == hostPeer)) {
            frame.clear();
        }
    };
    w.As(bea, [&] { bea.net->onPeer(hostPeer, PeerEvent::Lost); });
    w.Pump();
    CHECK(bea.ctl->GetState() == CollabController::State::Migrating);
    REQUIRE(AddOn(w, ari, "Grid", 3000, 3100) != nullptr);
    w.RunFor(40000);
    CHECK(bea.ctl->GetState() == CollabController::State::Ended);
    CHECK(bea.ctl->EndReason() == Reason::HostLost);
    CHECK(bea.ctl->GetRole() != CollabController::Role::Host);
    CHECK(bea.net->listens == 1); // its turn came, nobody reconnected to it
    REQUIRE(w.Quiesce());
    CHECK(w.CurrentHost() == &h);
    for (Peer* p : { &ari, &cy }) {
        INFO(p->name);
        CHECK(p->ctl->InSession());
        CHECK(p->ctl->HostParticipant() == 1);
        CHECK(p->app->hostChanges.empty());
        CHECK(EffectAt(*p, "Grid", 3000) != nullptr);
    }
    CHECK(h.Host().ClientCount() == 2);
}

// ---------------------------------------------------------------------------
// Roles and row assignments across a host change
// ---------------------------------------------------------------------------

TEST_CASE("Host migration: roles, the join role and row assignments carry over and are still enforced", "[collab][migrate][collab_roles]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];
    SeqObjectId line = h.Seq().GetElement("Line")->GetUid();
    SeqObjectId grid = h.Seq().GetElement("Grid")->GetUid();
    std::string err;
    w.As(h, [&] { h.ctl->SetJoinRole(ParticipantRole::Viewer); });
    REQUIRE(w.As(h, [&] { return h.ctl->SetRole(3, ParticipantRole::Viewer, err); }));
    REQUIRE(w.As(h, [&] { return h.ctl->Assign(line, 2, err); }));
    REQUIRE(w.As(h, [&] { return h.ctl->Assign(grid, 1, err); })); // the host's own claim
    REQUIRE(w.Quiesce());
    w.RunFor(1500);
    REQUIRE(w.Quiesce());
    // Viewers rank after every editor.
    std::vector<ParticipantId> ranked{ 2, 4, 3 };
    CHECK(h.Host().RankSuccessors() == ranked);
    CHECK(cy.Client().Succession() == ranked);
    CHECK(bea.ctl->LocalRole() == ParticipantRole::Viewer);

    w.Lose(h);
    RequireMigrated(w, ari);
    for (Peer* p : { &ari, &bea, &cy }) {
        INFO(p->name);
        CHECK(p->ctl->RoleOf(3) == ParticipantRole::Viewer);
        CHECK(p->ctl->RoleOf(4) == ParticipantRole::Editor);
        CHECK(p->ctl->RoleOf(2) == ParticipantRole::Editor);
        auto held = p->ctl->AssignmentOf(line);
        REQUIRE(held.has_value());
        CHECK(held->owner == 2);
        // The departed host's claim went with it.
        CHECK_FALSE(p->ctl->AssignmentOf(grid).has_value());
    }
    CHECK(ari.ctl->JoinRole() == ParticipantRole::Viewer);
    // The viewer's gate, undo block and layout lock still hold after it rejoined.
    CHECK_FALSE(bea.ctl->CanEditSequence());
    CHECK_FALSE(bea.ctl->CanUndo());
    CHECK(bea.app->lockReason == "You're a viewer in this session, so the layout is read-only");
    w.As(bea, [&] {
        bea.Undo().CreateUndoStep();
        CHECK_FALSE(bea.Exec().MoveEffect(EffectAt(bea, "Grid", 1000), 1100, 1600));
    });
    // And the new host enforces both when a proposal gets to it anyway.
    std::string before = DescribeSequence(ari.Seq());
    w.As(bea, [&] { bea.Client().Submit(std::make_unique<MoveEffectCommand>(LocatorOf(EffectAt(bea, "Grid", 1000)), 1100, 1600)); });
    w.As(cy, [&] { cy.Client().Submit(std::make_unique<MoveEffectCommand>(LocatorOf(EffectAt(cy, "Line", 0)), 100, 300)); });
    RequireConverged(w);
    CHECK(DescribeSequence(ari.Seq()) == before);
    CHECK(ari.Host().RejectCounts().count(Reason::Viewer) == 1);
    CHECK(ari.Host().RejectCounts().count(Reason::Assigned) == 1);
    // Cy, an editor, edits the unclaimed Grid; Ari still owns Line.
    std::string why;
    CHECK_FALSE(cy.ctl->CanEdit(line, why));
    CHECK(why == "Assigned to Ari");
    REQUIRE(AddOn(w, cy, "Grid", 3000, 3100) != nullptr);
    RequireConverged(w);
    CHECK(EffectAt(ari, "Grid", 3000) != nullptr);
    // Cy claims Grid; the new host may release a claim that isn't its own.
    REQUIRE(w.As(cy, [&] { return cy.ctl->Assign(grid, 4, err); }));
    REQUIRE(w.Quiesce());
    REQUIRE(ari.ctl->AssignmentOf(grid).has_value());
    REQUIRE(w.As(ari, [&] { return ari.ctl->Release(grid, err); }));
    REQUIRE(w.Quiesce());
    CHECK_FALSE(cy.ctl->AssignmentOf(grid).has_value());
    // Someone joining the new host gets the join role the old host had set.
    Peer& dee = w.Add("Dee");
    w.JoinTo(dee, "session-1/2");
    REQUIRE(w.Quiesce());
    CHECK(dee.ctl->LocalRole() == ParticipantRole::Viewer);
    CHECK(dee.ctl->LocalParticipant() == 5);
}

TEST_CASE("Host migration: a viewer that takes over becomes an editor and its layout lock lifts", "[collab][migrate][collab_roles]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    std::string err;
    REQUIRE(w.As(h, [&] { return h.ctl->SetRole(2, ParticipantRole::Viewer, err); }));
    REQUIRE(w.Quiesce());
    w.RunFor(1500);
    REQUIRE(ari.ctl->LocalRole() == ParticipantRole::Viewer);
    REQUIRE(ari.app->lockReason == "You're a viewer in this session, so the layout is read-only");
    w.Lose(h);
    w.RunFor(3000);
    REQUIRE(w.CurrentHost() == &ari);
    CHECK(ari.ctl->LocalRole() == ParticipantRole::Editor);
    CHECK(ari.ctl->CanEditSequence());
    CHECK(ari.app->lockReason == LayoutLock::DEFAULT_REASON);
    REQUIRE(AddOn(w, ari, "Grid", 3000, 3100) != nullptr);
    w.RunFor(100);
    CHECK(EffectAt(ari, "Grid", 3000) != nullptr);
    CHECK(ari.ctl->CanUndo());
}

TEST_CASE("Host migration: claims of someone who doesn't come back are released", "[collab][migrate][collab_roles]") {
    Show show;
    World w(show);
    StartSession(w, 3);
    Peer& h = *w.peers[0];
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Peer& cy = *w.peers[3];
    SeqObjectId line = h.Seq().GetElement("Line")->GetUid();
    SeqObjectId grid = h.Seq().GetElement("Grid")->GetUid();
    std::string err;
    REQUIRE(w.As(h, [&] { return h.ctl->Assign(line, 4, err); }));
    REQUIRE(w.As(h, [&] { return h.ctl->Assign(grid, 3, err); }));
    REQUIRE(w.Quiesce());
    w.RunFor(1500);
    // The host and Cy go together.
    w.Lose(h);
    w.Lose(cy);
    w.RunFor(3000);
    REQUIRE(w.CurrentHost() == &ari);
    // Cy might still come back for a while: its claim stays.
    CHECK(ari.ctl->AssignmentOf(line).has_value());
    w.RunFor(15000);
    for (Peer* p : { &ari, &bea }) {
        INFO(p->name);
        CHECK_FALSE(p->ctl->AssignmentOf(line).has_value());
        REQUIRE(p->ctl->AssignmentOf(grid).has_value());
        CHECK(p->ctl->AssignmentOf(grid)->owner == 3);
    }
    std::string why;
    CHECK(ari.ctl->CanEdit(line, why));
}

TEST_CASE("Host migration: the roster carries the join role", "[collab][migrate][collab_roles]") {
    Message r;
    r.type = MsgType::Roster;
    r.roster = { { 2, "Ari", "mac", ParticipantRole::Editor } };
    r.joinRole = ParticipantRole::Viewer;
    r.epoch = 1;
    auto d = DecodeMessage(EncodeMessage(r));
    REQUIRE(d);
    CHECK(d->joinRole == ParticipantRole::Viewer);
    CHECK(d->epoch == 1);
    r.joinRole = ParticipantRole::Editor;
    std::string s = EncodeMessage(r);
    CHECK(s.find("\"join\"") == std::string::npos);
    CHECK(DecodeMessage(s)->joinRole == ParticipantRole::Editor);
    s.insert(s.size() - 1, ",\"join\":\"boss\"");
    CHECK_FALSE(DecodeMessage(s));
}

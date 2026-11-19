// CollabController: the app-facing session controller. Each simulated device
// has its own document (SequenceElements + CommandExecutor + UndoManager),
// its own id allocator and its own controller; edits are made only through
// the executor's public API and undo only through the UndoManager, exactly
// as the apps do.

#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "collab/CollabController.h"
#include "collab/CollabNet.h"
#include "render/EffectEditCoalescer.h"
#include "render/SequenceEditCapture.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "render/UndoManager.h"
#include "utils/LayoutLock.h"
#include "utils/xlImage.h"

using namespace Collab;
using namespace CollabTest;

namespace {

// A render context that records whether the document was inside an edit
// whenever someone stopped the render.
class ProbeContext : public HeadlessRenderContext {
public:
    explicit ProbeContext(std::pair<int, int>& counts) : _counts(counts) {}
    bool AbortRender(int maxTimeMs = 60000) override {
        ++_counts.first;
        if (GetSequenceElements().IsEditInProgress()) {
            ++_counts.second;
        }
        return HeadlessRenderContext::AbortRender(maxTimeMs);
    }

private:
    std::pair<int, int>& _counts;
};

// One device's app: its open sequence plus a record of every notification.
class TestApp : public ICollabApp {
public:
    TestApp(const std::string& show, const std::string& file) : _show(show), _file(file) {}

    void Open(const std::string& xsq) { REQUIRE(Load(xsq)); }

    SequenceElements& Sequence() override { return _ctx->GetSequenceElements(); }
    bool HasSequence() override { return _ctx != nullptr; }
    bool SaveXsq(std::string& out) override {
        out = SaveAs(_file + ".snap.xsq");
        if (duringSave) {
            duringSave();
        }
        return !out.empty();
    }
    bool LoadXsq(const std::string& xsq) override {
        int call = ++loadCalls;
        if (duringLoad) {
            duringLoad();
        }
        std::string p = _file + ".load.xsq";
        std::ofstream(p, std::ios::binary) << (failLoadCall == call ? std::string("not a sequence") : xsq);
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
        lockCalls.push_back(l);
        lockReason = reason;
        if (_ctx) {
            _ctx->SetCollaborationLock(l, reason);
        }
    }
    void OnSessionStateChanged() override { ++stateChanges; }
    void OnParticipantsChanged() override { ++participantChanges; }
    void OnPresenceChanged(ParticipantId who) override { presenceEvents.push_back(who); }
    void OnSequenceChanged(const SequenceChange& c) override {
        // Notifications never arrive while the session is applying.
        if (_ctx) {
            CHECK_FALSE(Sequence().IsEditInProgress());
        }
        changes.push_back(c);
        if (duringChange) {
            duringChange();
        }
    }
    void MarkSequenceModified() override { ++modified; }
    void OnRejected(const RejectNotice& n) override { rejects.push_back(n); }
    void OnUndoStateChanged() override { ++undoChanges; }
    void OnJoinReport(const JoinReport& r) override { report = r; }
    void OnSessionsFound(const std::vector<SessionAdvert>& s) override { found = s; }
    void OnSessionNotice(const std::string& text) override { notices.push_back(text); }

    // Run inside the hooks, e.g. a nested event loop.
    std::function<void()> duringSave;
    std::function<void()> duringLoad;
    std::function<void()> duringChange;
    bool locked = false;
    std::vector<bool> lockCalls;
    std::string lockReason;
    int stateChanges = 0;
    int participantChanges = 0;
    std::vector<ParticipantId> presenceEvents;
    std::vector<SequenceChange> changes;
    int modified = 0;
    std::vector<RejectNotice> rejects;
    std::vector<std::string> notices;
    int undoChanges = 0;
    std::optional<JoinReport> report;
    std::vector<SessionAdvert> found;
    int loads = 0;
    int loadCalls = 0;
    int failLoadCall = 0; // this LoadXsq call (1-based) fails
    std::pair<int, int> aborts{ 0, 0 }; // render stops, of them inside an edit

private:
    bool Load(const std::string& xsq) {
        _seqFile.reset();
        _ctx.reset();
        _ctx = std::make_unique<ProbeContext>(aborts);
        if (!_ctx->LoadShowFolder(_show, _media)) {
            return false;
        }
        _seqFile = std::make_unique<SequenceFile>(xsq);
        auto doc = _seqFile->Open(_show, false, xsq);
        if (!doc || !_ctx->LoadSequenceElements(*_seqFile, *doc)) {
            return false;
        }
        if (locked) {
            _ctx->SetCollaborationLock(true, lockReason);
        }
        return true;
    }

    std::string _show;
    std::string _file;
    std::list<std::string> _media;
    std::unique_ptr<ProbeContext> _ctx;
    std::unique_ptr<SequenceFile> _seqFile;
};

// Sessions advertised on the simulated network.
struct Directory {
    struct Entry {
        PeerId peer = kNoPeer;
        std::string code;
        SessionAdvert advert;
    };
    std::map<std::string, Entry> sessions;
};

// An INetTransport over one LoopbackNetwork peer.
class LoopNet : public INetTransport {
public:
    LoopNet(LoopbackTransport& t, Directory& dir) : _t(t), _dir(dir) {
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
        _t.onMessage = nullptr;
        _t.onPeer = nullptr;
        _t.onBlob = nullptr;
        _t.onBlobProgress = nullptr;
        _t.onBlobDone = nullptr;
        _t.onSendFailed = nullptr;
        _dir.sessions.erase(_id);
    }
    PeerId LocalPeer() const override { return _t.LocalPeer(); }
    void Send(PeerId to, const Message& m) override {
        if (!_closed) {
            ++sent;
            _t.Send(to, m);
        }
    }
    void Broadcast(const Message& m) override {
        if (!_closed) {
            ++sent;
            _t.Broadcast(m);
        }
    }
    void SendBlob(PeerId to, BlobId id, const std::string& path) override {
        if (!_closed) {
            ++sent;
            _t.SendBlob(to, id, path);
        }
    }
    size_t sent = 0;
    bool Listen(const std::string& code, const SessionAdvert& advert, std::string& /*error*/) override {
        _id = "session-" + std::to_string(_t.LocalPeer());
        Directory::Entry e{ _t.LocalPeer(), code, advert };
        e.advert.id = _id;
        _dir.sessions[_id] = e;
        _state = NetState::Listening;
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
        _dir.sessions.erase(_id);
    }
    NetState State() const override { return _state; }

private:
    LoopbackTransport& _t;
    Directory& _dir;
    std::string _id;
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
    std::unique_ptr<TestApp> app;
    LoopbackTransport* lt = nullptr;
    LoopNet* net = nullptr; // the current session's transport
    std::deque<std::function<void()>> posted; // the Dispatcher's queue
    std::unique_ptr<CollabController> ctl;

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

    Peer& Add(const std::string& name, const std::string& device, bool open = true, const std::string& showPath = "",
              const std::string& sessionFolder = "") {
        auto p = std::make_unique<Peer>();
        Peer* pp = p.get();
        pp->name = name;
        pp->ids.counter = _baseCounter;
        pp->lt = &net.AddPeer();
        pp->app = std::make_unique<TestApp>(showPath.empty() ? _show.path : showPath, _show.dir.File("dev" + std::to_string(peers.size())));
        peers.push_back(std::move(p));
        Scope s(*pp);
        if (open) {
            pp->app->Open(_show.start);
        }
        CollabNetFactory f;
        f.transport = [this, pp] {
            auto t = std::make_unique<LoopNet>(*pp->lt, dir);
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
        opts.sessionFolder = sessionFolder;
        pp->ctl = std::make_unique<CollabController>(*pp->app, std::move(f), [pp](std::function<void()> fn) { pp->posted.push_back(std::move(fn)); },
                                                    opts);
        return *pp;
    }

    Peer& host() { return *peers[0]; }

    void HostOn(Peer& p) {
        Scope s(p);
        std::string err;
        REQUIRE(p.ctl->Host(p.name, "start.xsq", code, err));
        p.ctl->Tick(net.Now());
    }

    void JoinTo(Peer& p, bool fetch = false) {
        Scope s(p);
        std::string err;
        p.ctl->Tick(net.Now());
        REQUIRE(p.ctl->Join(p.name, "session-" + std::to_string(host().lt->LocalPeer()), code, fetch, err));
    }

    // Runs everything the Dispatchers were asked to run.
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
            if (p->ctl) {
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

    // An event loop run from inside a hook: the network delivers, the
    // Dispatchers run and the timers fire for `ms` of virtual time.
    void NestedLoop(uint64_t ms) {
        uint64_t end = net.Now() + ms;
        while (net.Now() < end) {
            net.RunUntil(std::min(end, net.Now() + 5));
            Pump();
            Tick();
        }
    }

    bool Quiet() {
        Pump();
        if (net.InFlight() != 0) {
            return false;
        }
        if (host().ctl->GetState() != CollabController::State::Active) {
            return true;
        }
        Rev head = host().Host().HeadRev();
        for (size_t i = 1; i < peers.size(); ++i) {
            Peer& p = *peers[i];
            auto st = p.ctl->GetState();
            if (st == CollabController::State::Ended || st == CollabController::State::Idle) {
                continue;
            }
            if (st != CollabController::State::Active || p.Client().PendingCount() != 0 || p.Client().AppliedRev() != head ||
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

    template<class F>
    auto As(Peer& p, F&& fn) {
        Scope s(p);
        return fn();
    }

    LoopbackNetwork net;
    Directory dir;
    std::vector<std::unique_ptr<Peer>> peers;
    std::string code;

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

int CountAt(Peer& p, const std::string& element, int startMS) {
    int n = 0;
    Element* el = p.Seq().GetElement(element);
    if (el == nullptr) {
        return 0;
    }
    for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
        for (Effect* e : el->GetEffectLayer((int)l)->GetEffects()) {
            n += e->GetStartTimeMS() == startMS ? 1 : 0;
        }
    }
    return n;
}

Effect* EffectAt(Peer& p, const std::string& element, int startMS, int layer = 0) {
    Element* el = p.Seq().GetElement(element);
    if (el == nullptr || (int)el->GetEffectLayerCount() <= layer) {
        return nullptr;
    }
    for (Effect* e : el->GetEffectLayer(layer)->GetEffects()) {
        if (e->GetStartTimeMS() == startMS) {
            return e;
        }
    }
    return nullptr;
}

std::string Setting(Peer& p, const std::string& element, int startMS, const std::string& key) {
    Effect* e = EffectAt(p, element, startMS);
    REQUIRE(e != nullptr);
    return e->GetSettings().Get(key, std::string("<none>"));
}

// An edit site starts its undo step and then edits, as the apps do.
void Step(Peer& p) {
    p.Undo().CreateUndoStep();
}

Effect* AddOn(Peer& p, const std::string& element, int start, int end, int selected = 0, int layer = 0) {
    Step(p);
    return p.Exec().AddEffect(LayerOf(p, element, layer), "On", "", "", start, end, selected, false);
}

void RequireConverged(World& w) {
    REQUIRE(w.Quiesce());
    Peer& h = w.host();
    std::string desc = DescribeSequence(h.Seq());
    std::string xsq = h.app->Save();
    std::string problem;
    REQUIRE(h.Seq().ValidateUids(&problem));
    for (size_t i = 1; i < w.peers.size(); ++i) {
        Peer& c = *w.peers[i];
        if (c.ctl->GetState() != CollabController::State::Active) {
            continue;
        }
        INFO("peer " << i);
        CHECK(DescribeSequence(c.Seq()) == desc);
        CHECK(c.app->Save() == xsq);
        CHECK(DocHasher::FullHash(c.Seq()) == DocHasher::FullHash(h.Seq()));
        CHECK(c.Seq().ValidateUids(&problem));
        CHECK(c.Client().ResyncCount() == 0);
        CHECK(c.Client().HashVerifyFailures() == 0);
    }
    CHECK(h.Host().HashVerifyFailures() == 0);
}

// Host plus `clients` clients, all joined and caught up.
void StartSession(World& w, int clients) {
    Peer& h = w.Add("Hana", "mac");
    w.HostOn(h);
    const char* names[] = { "Ari", "Bea", "Cy" };
    for (int i = 0; i < clients; ++i) {
        Peer& c = w.Add(names[i], i % 2 ? "mac" : "ipad");
        w.JoinTo(c);
    }
    REQUIRE(w.Quiesce());
    for (auto& p : w.peers) {
        REQUIRE(p->ctl->InSession());
    }
}

} // namespace

TEST_CASE("Hosting and joining lock the layout, list everyone and end cleanly", "[collab][controller]") {
    Show show;
    World w(show);
    Peer& h = w.Add("Hana", "mac");
    REQUIRE(h.ctl->Available());
    // Edits from before the session aren't undoable inside it.
    REQUIRE(w.As(h, [&] { return AddOn(h, "Line", 3000, 3100); }) != nullptr);
    REQUIRE(h.Undo().CanUndo());
    w.HostOn(h);
    CHECK(h.app->locked);
    CHECK(h.app->lockReason == LayoutLock::DEFAULT_REASON);
    CHECK_FALSE(h.Undo().CanUndo());
    CHECK(h.ctl->GetRole() == CollabController::Role::Host);
    CHECK(h.ctl->StatusText() == "Hosting - no one has joined yet");

    Peer& a = w.Add("Ari", "ipad");
    Peer& b = w.Add("Bea", "mac");
    w.As(a, [&] { a.ctl->StartBrowsing(); });
    REQUIRE(a.app->found.size() == 1);
    CHECK(a.app->found[0].hostName == "Hana");
    CHECK(a.app->found[0].sequenceName == "start.xsq");
    w.JoinTo(a);
    CHECK(a.app->locked);
    CHECK(a.ctl->GetState() == CollabController::State::Joining);
    w.JoinTo(b);
    RequireConverged(w);
    CHECK(a.app->report.has_value());
    CHECK(a.app->report->refusal.empty());
    CHECK(a.app->loads == 1);
    CHECK(a.ctl->StatusText() == "In Hana's session with 1 other");
    CHECK(h.ctl->StatusText() == "Hosting with 2 others");

    // Everyone sees the same people in the same colours.
    for (Peer* p : { &h, &a, &b }) {
        auto ps = p->ctl->Participants();
        REQUIRE(ps.size() == 3);
        CHECK(ps[0].name == "Hana");
        CHECK(ps[0].isHost);
        CHECK(ps[0].deviceKind == "mac");
        CHECK(ps[1].name == "Ari");
        CHECK(ps[1].deviceKind == "ipad");
        CHECK(ps[2].name == "Bea");
        for (const auto& q : ps) {
            CHECK(q.colour == ParticipantColour(q.id));
            CHECK(q.isLocal == (q.id == p->ctl->LocalParticipant()));
        }
    }
    CHECK(ParticipantColour(1) != ParticipantColour(2));
    CHECK(ParticipantColour(2) != ParticipantColour(3));

    // A client leaves: its lock goes, the others see it gone, its copy stays open.
    w.As(a, [&] { a.ctl->Leave(); });
    w.RunFor(50);
    CHECK_FALSE(a.app->locked);
    CHECK(a.ctl->GetState() == CollabController::State::Ended);
    CHECK(a.ctl->EndText() == "You left the session.");
    CHECK(a.Undo().GetRedirect() == nullptr);
    CHECK(h.ctl->Participants().size() == 2);
    CHECK(b.ctl->Participants().size() == 2);
    CHECK(EffectAt(a, "Line", 3000) != nullptr);

    // The host ends it for everyone.
    w.As(h, [&] { h.ctl->End(); });
    w.RunFor(50);
    CHECK_FALSE(h.app->locked);
    CHECK_FALSE(b.app->locked);
    CHECK(b.ctl->GetState() == CollabController::State::Ended);
    CHECK(b.ctl->EndText() == "The host ended the session.");
    CHECK(h.ctl->EndText() == "You ended the session.");
    CHECK(h.Undo().GetRedirect() == nullptr);
    CHECK(b.Undo().GetRedirect() == nullptr);
    CHECK(h.app->lockCalls == std::vector<bool>{ true, false });
    CHECK(b.app->lockCalls == std::vector<bool>{ true, false });
    // Edits after the end are local again, with local undo.
    REQUIRE(w.As(b, [&] { return AddOn(b, "Line", 9000, 9100); }) != nullptr);
    CHECK(b.Undo().CanUndo());
    w.RunFor(50);
    CHECK(EffectAt(h, "Line", 9000) == nullptr);
    // After the close delay the transports go.
    w.RunFor(3000, 100);
    CHECK(w.dir.sessions.empty());
}

TEST_CASE("A wrong pairing code or a lost host releases the layout lock", "[collab][controller]") {
    Show show;
    World w(show);
    Peer& h = w.Add("Hana", "mac");
    w.HostOn(h);
    Peer& a = w.Add("Ari", "ipad");
    {
        World::Scope s(a);
        std::string err;
        REQUIRE(a.ctl->Join("Ari", "session-1", "000000", false, err));
    }
    CHECK(a.ctl->GetState() == CollabController::State::Ended);
    CHECK(a.ctl->EndText() == "The pairing code is wrong.");
    CHECK(a.app->lockCalls == std::vector<bool>{ true, false });
    CHECK_FALSE(a.app->locked);

    // A second try with the right code works.
    w.JoinTo(a);
    REQUIRE(w.Quiesce());
    REQUIRE(a.ctl->InSession());
    CHECK(a.app->locked);
    // The host vanishes: the client ends, keeps its document, drops the lock.
    w.net.Disconnect(h.lt->LocalPeer());
    w.RunFor(100);
    CHECK(a.ctl->GetState() == CollabController::State::Ended);
    CHECK(a.ctl->EndReason() == Reason::HostLost);
    CHECK_FALSE(a.app->locked);
    CHECK(a.Undo().GetRedirect() == nullptr);
    CHECK(a.Seq().GetElement("Grid") != nullptr);
    // A client never marks its copy modified.
    CHECK(a.app->modified == 0);
}

TEST_CASE("Every kind of executor edit reaches the session once and everyone converges", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];

    // Single commands on all three, interleaved before anyone flushes.
    REQUIRE(w.As(h, [&] { return AddOn(h, "Line", 1000, 1100); }) != nullptr);
    REQUIRE(w.As(a, [&] { return AddOn(a, "Line", 1200, 1300); }) != nullptr);
    REQUIRE(w.As(b, [&] { return AddOn(b, "Line", 1400, 1500); }) != nullptr);
    RequireConverged(w);
    for (Peer* p : { &h, &a, &b }) {
        CHECK(CountAt(*p, "Line", 1000) == 1);
        CHECK(CountAt(*p, "Line", 1200) == 1);
        CHECK(CountAt(*p, "Line", 1400) == 1);
    }
    Rev head = h.Host().HeadRev();
    CHECK(head == 3);

    // A labelled batch on a client: one commit.
    w.As(a, [&] {
        ScopedCommandBatch batch(a.Exec(), "Paste");
        AddOn(a, "Grid", 3000, 3100);
        AddOn(a, "Grid", 3200, 3300);
    });
    RequireConverged(w);
    CHECK(h.Host().HeadRev() == head + 1);
    CHECK(EffectAt(h, "Grid", 3200) != nullptr);

    // A live-preview drag on the host and on a client.
    w.As(h, [&] {
        h.Exec().BeginPreview();
        Effect* e = EffectAt(h, "Line", 1000);
        for (int t = 1; t <= 5; ++t) {
            h.Exec().MoveEffect(e, 1000 + 100 * t, 1100 + 100 * t);
        }
    });
    // Nothing is sent while the drag is open.
    w.RunFor(20);
    CHECK(EffectAt(a, "Line", 1000) != nullptr);
    w.As(h, [&] { h.Exec().CommitPreview(""); });
    w.As(b, [&] {
        b.Exec().BeginPreview();
        b.Exec().MoveEffect(EffectAt(b, "Line", 1400), 1700, 1800);
        b.Exec().CommitPreview("");
    });
    RequireConverged(w);
    CHECK(EffectAt(a, "Line", 1500) != nullptr);
    CHECK(EffectAt(a, "Line", 1700) != nullptr);

    // EditEffect (settings changed in place) and a whole-map replacement.
    w.As(b, [&] {
        Step(b);
        b.Exec().EditEffect(EffectAt(b, "Grid", 1000), [](Effect& e) { e.GetSettings()["E_SLIDER_Speed"] = "7"; });
        b.Exec().SetEffectSettings(EffectAt(b, "Grid", 2000), "E_SLIDER_Bars_Cycles=3", true);
    });
    RequireConverged(w);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_Speed") == "7");
    CHECK(Setting(h, "Grid", 2000, "E_SLIDER_Bars_Cycles") == "3");

    // Layers: insert on one client, rename and remove on another.
    w.As(a, [&] {
        Step(a);
        REQUIRE(a.Exec().InsertLayer(a.Seq().GetElement("Line"), 1) != nullptr);
    });
    RequireConverged(w);
    w.As(b, [&] {
        Step(b);
        REQUIRE(b.Exec().SetLayerName(LayerOf(b, "Line", 1), "Second"));
    });
    RequireConverged(w);
    CHECK(LayerOf(h, "Line", 1)->GetLayerName() == "Second");
    REQUIRE(w.As(h, [&] { return AddOn(h, "Line", 5000, 5100, 0, 1); }) != nullptr);
    RequireConverged(w);
    w.As(a, [&] {
        Step(a);
        REQUIRE(a.Exec().RemoveLayer(a.Seq().GetElement("Line"), 1));
    });
    RequireConverged(w);
    CHECK(h.Seq().GetElement("Line")->GetEffectLayerCount() == 1);

    // Timing marks are effects on a timing row.
    w.As(b, [&] {
        ScopedCommandBatch batch(b.Exec(), "Add Timing Marks");
        b.Exec().AddEffect(LayerOf(b, "Beats"), "x", "", "", 4000, 4250, 0, false);
        b.Exec().AddEffect(LayerOf(b, "Beats"), "y", "", "", 4250, 4500, 0, false);
    });
    RequireConverged(w);
    CHECK(EffectAt(a, "Beats", 4250) != nullptr);

    // Rows: a client adds a model row; the host deletes it.
    w.As(a, [&] {
        ElementSnapshot snap;
        snap.name = "Star";
        REQUIRE(a.Exec().Execute(std::make_unique<AddElementCommand>(snap, a.Seq().GetElement("Line")->GetUid())).Ok());
    });
    RequireConverged(w);
    REQUIRE(h.Seq().GetElement("Star") != nullptr);
    REQUIRE(w.As(h, [&] { return AddOn(h, "Star", 100, 200); }) != nullptr);
    RequireConverged(w);
    CHECK(EffectAt(b, "Star", 100) != nullptr);

    // Media added by an edit.
    auto img = MediaEntryState::EmbeddedImage("Images/c.png", xlImage(2, 2));
    REQUIRE(img.has_value());
    w.As(b, [&] { REQUIRE(b.Exec().Execute(std::make_unique<PutMediaCommand>(*img)).Ok()); });
    RequireConverged(w);
    CHECK(h.Seq().GetSequenceMedia().HasMedia("Images/c.png"));

    // An import capture: direct changes recorded as one edit.
    w.As(a, [&] {
        SequenceEditCapture capture(a.Seq(), "Import Effects");
        SeqMutationGuard::Scope direct;
        Element* pole = a.Seq().AddElement("Pole", "model", true, false, false, false, false);
        if (pole->GetEffectLayerCount() == 0) {
            pole->AddEffectLayer();
        }
        pole->GetEffectLayer(0)->AddEffect(0, "On", "", "", 0, 50, 0, false);
        LayerOf(a, "Grid")->AddEffect(0, "Bars", "", "", 6000, 6100, 0, false);
    });
    RequireConverged(w);
    CHECK(EffectAt(h, "Pole", 0) != nullptr);
    CHECK(EffectAt(h, "Grid", 6000) != nullptr);

    // Deleting a row (a model with effects) from a client.
    w.As(b, [&] {
        Element* star = b.Seq().GetElement("Star");
        REQUIRE(b.Exec().Execute(std::make_unique<DeleteElementCommand>(star->GetUid(), false)).Ok());
    });
    RequireConverged(w);
    CHECK(h.Seq().GetElement("Star") == nullptr);
    // Nothing ever re-synced, and the host's own document is the reference.
    CHECK(a.Client().ResyncCount() == 0);
    CHECK(b.Client().ResyncCount() == 0);
}

TEST_CASE("A client's structural edit waits for the commit and keeps its selection", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    a.app->changes.clear();

    Effect* added = w.As(a, [&] { return AddOn(a, "Line", 3000, 3500, 1); });
    REQUIRE(added != nullptr);
    SeqObjectId uid = added->GetUid();
    // Until the next safe point the edit is simply local.
    CHECK(EffectAt(a, "Line", 3000) != nullptr);
    w.Pump();
    // Taken out and proposed: a ghost until the commit comes back.
    CHECK(EffectAt(a, "Line", 3000) == nullptr);
    auto ghosts = a.ctl->PendingGhosts();
    REQUIRE(ghosts.size() == 1);
    CHECK(ghosts[0].kind == PendingGhost::Kind::Add);
    CHECK(ghosts[0].effect.effect == uid);
    CHECK(ghosts[0].startMS == 3000);
    REQUIRE(!a.app->changes.empty());
    CHECK(a.app->changes.back().cause == SequenceChange::Cause::OwnEditPending);
    CHECK(a.app->changes.back().effects == std::vector<SeqObjectId>{ uid });

    RequireConverged(w);
    Effect* back = EffectAt(a, "Line", 3000);
    REQUIRE(back != nullptr);
    CHECK(back->GetUid() == uid);
    CHECK(back->GetSelected() == 1);
    CHECK(a.ctl->PendingGhosts().empty());
    CHECK(a.app->changes.back().cause == SequenceChange::Cause::OwnEdit);
    // The host's own edits are never taken out.
    Effect* hostAdded = w.As(h, [&] { return AddOn(h, "Line", 4000, 4500, 1); });
    w.Pump();
    CHECK(EffectAt(h, "Line", 4000) == hostAdded);
    CHECK(hostAdded->GetSelected() == 1);
    RequireConverged(w);
    // A pending delete is a ghost too. Mixed with a move, the move waits as well.
    w.As(a, [&] {
        Step(a);
        a.Exec().MoveEffect(EffectAt(a, "Line", 4000), 4600, 4700);
        a.Exec().DeleteEffect(EffectAt(a, "Line", 3000));
    });
    w.Pump();
    ghosts = a.ctl->PendingGhosts();
    REQUIRE(ghosts.size() == 2);
    CHECK(ghosts[0].kind == PendingGhost::Kind::Move);
    CHECK(ghosts[0].startMS == 4600);
    CHECK(ghosts[1].kind == PendingGhost::Kind::Delete);
    CHECK(ghosts[1].startMS == 3000);
    CHECK(EffectAt(a, "Line", 3000) != nullptr);
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 4600) != nullptr);
    CHECK(EffectAt(h, "Line", 3000) == nullptr);
}

TEST_CASE("Settings edits stay applied on a client while their commits travel", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    w.As(a, [&] {
        Step(a);
        a.Exec().SetEffectSetting(EffectAt(a, "Grid", 1000), "E_SLIDER_X", std::string("5"));
    });
    w.Pump();
    CHECK(Setting(a, "Grid", 1000, "E_SLIDER_X") == "5");
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "<none>");
    // A batch of key changes on two effects stays applied too.
    w.As(a, [&] {
        ScopedCommandBatch batch(a.Exec(), "Color");
        a.Exec().SetEffectSetting(EffectAt(a, "Grid", 1000), "E_SLIDER_Y", std::string("1"));
        a.Exec().SetEffectSetting(EffectAt(a, "Grid", 2000), "E_SLIDER_Y", std::string("2"));
    });
    w.Pump();
    CHECK(Setting(a, "Grid", 1000, "E_SLIDER_Y") == "1");
    CHECK(Setting(a, "Grid", 2000, "E_SLIDER_Y") == "2");
    CHECK(a.ctl->PendingGhosts().empty());
    RequireConverged(w);
    CHECK(Setting(h, "Grid", 2000, "E_SLIDER_Y") == "2");
    // ...and undo as one step.
    REQUIRE(w.As(a, [&] { return a.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_Y") == "<none>");
    CHECK(Setting(h, "Grid", 2000, "E_SLIDER_Y") == "<none>");
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "5");
}

TEST_CASE("Remote edits apply inside an edit scope and wait for an open local edit", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];

    // A row add stops the render first, and the document reports an edit
    // in progress meanwhile (so the desktop doesn't yield inside it), on
    // the host processing the proposal and on the client applying the commit.
    h.app->aborts = { 0, 0 };
    a.app->aborts = { 0, 0 };
    w.As(a, [&] {
        ElementSnapshot snap;
        snap.name = "Arch";
        REQUIRE(a.Exec().Execute(std::make_unique<AddElementCommand>(snap, kNoSeqObjectId)).Ok());
    });
    RequireConverged(w);
    CHECK(h.app->aborts.first > 0);
    CHECK(h.app->aborts.second == h.app->aborts.first);
    CHECK(a.app->aborts.first > 0);
    CHECK(a.app->aborts.second == a.app->aborts.first);
    // Not inside an apply now.
    CHECK_FALSE(h.Seq().IsEditInProgress());
    CHECK_FALSE(a.Seq().IsEditInProgress());

    // The client is dragging when the host's edit arrives: it waits.
    w.As(a, [&] {
        a.Exec().BeginPreview();
        a.Exec().MoveEffect(EffectAt(a, "Line", 0), 50, 250);
    });
    REQUIRE(w.As(h, [&] { return AddOn(h, "Grid", 3000, 3100); }) != nullptr);
    w.RunFor(50);
    CHECK(a.ctl->HeldDeliveries() > 0);
    CHECK(EffectAt(a, "Grid", 3000) == nullptr);
    w.As(a, [&] { a.Exec().CommitPreview(""); });
    RequireConverged(w);
    CHECK(EffectAt(a, "Grid", 3000) != nullptr);
    CHECK(EffectAt(h, "Line", 50) != nullptr);

    // The same for the host, with a batch open while a proposal arrives.
    REQUIRE(w.As(a, [&] { return AddOn(a, "Grid", 4000, 4100); }) != nullptr);
    w.Pump();
    w.As(h, [&] { h.Exec().BeginBatch("Long edit"); });
    w.As(h, [&] { AddOn(h, "Grid", 5000, 5100); });
    w.RunFor(50);
    CHECK(h.ctl->HeldDeliveries() > 0);
    CHECK(EffectAt(h, "Grid", 4000) == nullptr);
    w.As(h, [&] { h.Exec().EndBatch(); });
    RequireConverged(w);
    CHECK(EffectAt(a, "Grid", 5000) != nullptr);
    CHECK(EffectAt(h, "Grid", 4000) != nullptr);
}

TEST_CASE("Remote commits tell the app what changed and mark the host modified", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    a.app->changes.clear();
    h.app->changes.clear();
    int modified = h.app->modified;

    Effect* e = w.As(h, [&] { return AddOn(h, "Line", 3000, 3500); });
    SeqObjectId uid = e->GetUid();
    RequireConverged(w);
    REQUIRE(a.app->changes.size() == 1);
    const SequenceChange& c = a.app->changes[0];
    CHECK(c.cause == SequenceChange::Cause::RemoteEdit);
    CHECK(c.author == kHostParticipant);
    CHECK(c.rowNames == std::vector<std::string>{ "Line" });
    CHECK(c.startMS == 3000);
    CHECK(c.endMS == 3500);
    CHECK(c.effects == std::vector<SeqObjectId>{ uid });
    CHECK(c.label == "Effect(s) Added");
    CHECK_FALSE(c.rowsAdded);
    // The host's own edit isn't reported back to it.
    CHECK(h.app->changes.empty());

    // A client's move across time reports both ends; the host is modified.
    w.As(a, [&] {
        Step(a);
        a.Exec().MoveEffect(EffectAt(a, "Line", 3000), 6000, 6500);
    });
    RequireConverged(w);
    REQUIRE(!h.app->changes.empty());
    CHECK(h.app->changes.back().cause == SequenceChange::Cause::RemoteEdit);
    CHECK(h.app->changes.back().startMS == 3000);
    CHECK(h.app->changes.back().endMS == 6500);
    CHECK(h.app->modified > modified);
    // A deleted submodel row's effect reports its top-level row.
    w.As(a, [&] {
        Element* grid = a.Seq().GetElement("Grid");
        auto* me = dynamic_cast<ModelElement*>(grid);
        REQUIRE(me != nullptr);
        REQUIRE(me->GetSubModelAndStrandCount() > 0);
        Element* subRow = me->GetSubModel(0);
        Step(a);
        EffectLayer* sub = subRow->GetEffectLayerCount() > 0 ? subRow->GetEffectLayer(0) : a.Exec().InsertLayer(subRow, 0);
        REQUIRE(sub != nullptr);
        REQUIRE(a.Exec().AddEffect(sub, "On", "", "", 700, 800, 0, false) != nullptr);
    });
    RequireConverged(w);
    CHECK(h.app->changes.back().rowNames == std::vector<std::string>{ "Grid" });
    // Rows added.
    w.As(h, [&] {
        ElementSnapshot snap;
        snap.name = "Arch";
        REQUIRE(h.Exec().Execute(std::make_unique<AddElementCommand>(snap, kNoSeqObjectId)).Ok());
    });
    RequireConverged(w);
    CHECK(a.app->changes.back().rowsAdded);
    CHECK(a.app->changes.back().rowNames == std::vector<std::string>{ "Arch" });
    CHECK(a.app->changes.back().startMS == -1);
    CHECK(a.app->modified == 0);
}

TEST_CASE("Undo goes through the UndoManager to this user's own committed edits", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];

    REQUIRE(w.As(a, [&] { return AddOn(a, "Line", 3000, 3500); }) != nullptr);
    REQUIRE(w.As(b, [&] { return AddOn(b, "Line", 5000, 5500); }) != nullptr);
    RequireConverged(w);
    CHECK(a.Undo().CanUndo());
    CHECK(a.Undo().GetUndoString() == "Undo: Effect(s) Added");
    CHECK_FALSE(a.Undo().CanRedo());
    CHECK(a.Undo().GetRedoString() == "Redo");
    // a's undo removes a's effect, not b's.
    REQUIRE(w.As(a, [&] { return a.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 3000) == nullptr);
    CHECK(EffectAt(h, "Line", 5000) != nullptr);
    CHECK(a.Undo().CanRedo());
    CHECK(a.Undo().GetRedoString() == "Redo: Effect(s) Added");
    REQUIRE(w.As(a, [&] { return a.Undo().RedoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 3000) != nullptr);
    // Nothing went into the local history meanwhile.
    CHECK(a.Undo().GetRedirect() != nullptr);

    // The host's undo applies at once, and the app hears about it.
    h.app->changes.clear();
    w.As(h, [&] {
        Step(h);
        h.Exec().SetLayerName(LayerOf(h, "Grid"), "Main");
    });
    w.Pump();
    CHECK(h.Undo().GetUndoString() == "Undo: Layer Renamed");
    REQUIRE(w.As(h, [&] { return h.Undo().UndoLastStep(); }));
    CHECK(LayerOf(h, "Grid")->GetLayerName().empty());
    REQUIRE(!h.app->changes.empty());
    CHECK(h.app->changes.back().cause == SequenceChange::Cause::OwnUndo);
    RequireConverged(w);

    // b changes a key a set; a's undo is refused, naming b.
    w.As(a, [&] {
        Step(a);
        a.Exec().SetEffectSetting(EffectAt(a, "Grid", 1000), "E_SLIDER_X", std::string("a1"));
    });
    RequireConverged(w);
    w.As(b, [&] {
        Step(b);
        b.Exec().SetEffectSetting(EffectAt(b, "Grid", 1000), "E_SLIDER_X", std::string("b1"));
    });
    RequireConverged(w);
    a.app->rejects.clear();
    REQUIRE(w.As(a, [&] { return a.Undo().UndoLastStep(); }));
    RequireConverged(w);
    REQUIRE(a.app->rejects.size() == 1);
    CHECK(a.app->rejects[0].reason == Reason::ChangedSince);
    CHECK(a.app->rejects[0].kind == EditKind::Undo);
    CHECK(a.app->rejects[0].text == "Can't undo \xE2\x80\x94 that effect was changed by Bea");
    CHECK(Setting(h, "Grid", 1000, "E_SLIDER_X") == "b1");
}

TEST_CASE("A slider drag sent as many commits undoes as one step", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    for (Peer* p : { &a, &h }) {
        INFO(p->name);
        EffectEditCoalescer slider;
        int64_t t = 1000;
        for (const char* v : { "1", "2", "3", "4" }) {
            w.As(*p, [&] {
                SettingsChange ch;
                ch.keys["E_SLIDER_Speed"] = std::string(v);
                REQUIRE(slider.Apply(p->Seq(), EffectAt(*p, "Grid", 2000), std::nullopt, ch, SettingsChange{}, t));
            });
            t += 50;
            // Each tick is its own turn: it goes out on its own.
            RequireConverged(w);
            CHECK(Setting(h, "Grid", 2000, "E_SLIDER_Speed") == v);
        }
        REQUIRE(w.As(*p, [&] { return p->Undo().UndoLastStep(); }));
        RequireConverged(w);
        CHECK(Setting(h, "Grid", 2000, "E_SLIDER_Speed") == "<none>");
        CHECK_FALSE(p->Undo().CanUndo());
    }
}

TEST_CASE("Rejects come with readable text and put-on-new-layer works", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];

    // Both clients drop an effect on the same spot; b's arrives second.
    REQUIRE(w.As(a, [&] { return AddOn(a, "Line", 3000, 3500); }) != nullptr);
    w.Pump();
    REQUIRE(w.As(b, [&] { return AddOn(b, "Line", 3200, 3700); }) != nullptr);
    RequireConverged(w);
    REQUIRE(b.app->rejects.size() == 1);
    const RejectNotice n = b.app->rejects[0];
    CHECK(n.reason == Reason::Overlap);
    CHECK(n.text == "Someone placed an effect there first");
    CHECK(n.action == RejectAction::PutOnNewLayer);
    CHECK(b.ctl->PendingGhosts().empty());
    CHECK(EffectAt(b, "Line", 3200) == nullptr);
    REQUIRE(w.As(b, [&] { return b.ctl->PutOnNewLayer(n.id); }));
    RequireConverged(w);
    REQUIRE(h.Seq().GetElement("Line")->GetEffectLayerCount() == 2);
    CHECK(EffectAt(h, "Line", 3200, 1) != nullptr);
    CHECK(EffectAt(h, "Line", 3000, 0) != nullptr);
    CHECK_FALSE(w.As(b, [&] { return b.ctl->PutOnNewLayer(n.id); }));

    // An edit to an effect someone else deleted.
    w.As(a, [&] {
        Step(a);
        a.Exec().DeleteEffect(EffectAt(a, "Grid", 2000));
    });
    w.Pump();
    w.As(b, [&] {
        Step(b);
        b.Exec().MoveEffect(EffectAt(b, "Grid", 2000), 2100, 2600);
    });
    RequireConverged(w);
    REQUIRE(b.app->rejects.size() == 2);
    CHECK(b.app->rejects[1].reason == Reason::TargetGone);
    CHECK(b.app->rejects[1].action == RejectAction::None);

    // The shared texts.
    CHECK(DescribeReject(Reason::Overlap, EditKind::Edit, "").action == RejectAction::PutOnNewLayer);
    CHECK(DescribeReject(Reason::Overlap, EditKind::Undo, "").action == RejectAction::None);
    CHECK(DescribeReject(Reason::ChangedSince, EditKind::Redo, "Alex").text == "Can't redo \xE2\x80\x94 that effect was changed by Alex");
    CHECK(DescribeReject(Reason::StaleType, EditKind::Edit, "").text == "someone else changed that effect's type first, so the change wasn't made");
    CHECK(DescribeReject(Reason::Locked, EditKind::Edit, "").text == "That effect is locked");
    CHECK(DescribeReject("odd", EditKind::Edit, "").text == "That change couldn't be made (odd)");
}

TEST_CASE("Presence is throttled, soft locks drop on deselect idle and leave", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];
    SeqObjectId eff = EffectAt(h, "Grid", 1000)->GetUid();
    SeqObjectId row = h.Seq().GetElement("Grid")->GetUid();
    ParticipantId aid = a.ctl->LocalParticipant();

    size_t before = h.app->presenceEvents.size();
    // Many changes inside one interval: one goes out now, the newest after.
    for (int i = 0; i < 10; ++i) {
        PresenceInfo p;
        p.startMS = i * 100;
        p.endMS = i * 100 + 5000;
        p.focusedElements = { row };
        p.selectedEffects = { eff };
        p.softLock = eff;
        w.As(a, [&] { a.ctl->SetLocalPresence(p); });
    }
    w.net.RunUntil(w.net.Now() + 5);
    CHECK(h.app->presenceEvents.size() - before == 1);
    w.RunFor(400);
    CHECK(h.app->presenceEvents.size() - before == 2);
    REQUIRE(h.ctl->PresenceOf(aid) != nullptr);
    CHECK(h.ctl->PresenceOf(aid)->startMS == 900);
    CHECK(b.ctl->PresenceOf(aid)->startMS == 900);
    CHECK(h.ctl->SoftLockHolder(eff) == aid);
    CHECK(b.ctl->SoftLockHolder(eff) == aid);
    CHECK(b.ctl->SelectedBy(eff) == std::vector<ParticipantId>{ aid });
    CHECK(a.ctl->SoftLockHolder(eff) == kNoParticipant); // not to itself
    // Nothing changes: nothing is sent.
    size_t settled = h.app->presenceEvents.size();
    w.RunFor(1000, 50);
    CHECK(h.app->presenceEvents.size() == settled);

    // Deselecting drops the lock.
    PresenceInfo p = *h.ctl->PresenceOf(aid);
    p.selectedEffects.clear();
    w.As(a, [&] { a.ctl->SetLocalPresence(p); });
    w.RunFor(400);
    CHECK(h.ctl->SoftLockHolder(eff) == kNoParticipant);
    // Selected again, then idle for a minute: it drops.
    p.selectedEffects = { eff };
    w.As(a, [&] { a.ctl->SetLocalPresence(p); });
    w.RunFor(400);
    CHECK(h.ctl->SoftLockHolder(eff) == aid);
    w.RunFor(61000, 500);
    CHECK(h.ctl->SoftLockHolder(eff) == kNoParticipant);
    CHECK(h.ctl->SelectedBy(eff) == std::vector<ParticipantId>{ aid });
    // Editing counts as activity: the lock comes back.
    w.As(a, [&] {
        Step(a);
        a.Exec().SetEffectSetting(EffectAt(a, "Grid", 1000), "E_SLIDER_X", std::string("1"));
    });
    w.RunFor(400);
    CHECK(h.ctl->SoftLockHolder(eff) == aid);
    // Leaving drops everything that participant had.
    w.As(a, [&] { a.ctl->Leave(); });
    w.RunFor(200);
    CHECK(h.ctl->SoftLockHolder(eff) == kNoParticipant);
    CHECK(h.ctl->PresenceOf(aid) == nullptr);
    CHECK(b.ctl->PresenceOf(aid) == nullptr);
}

TEST_CASE("Unflushed local edits count for undo and an ended session stops routing", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    // Undo right after an edit, before the safe point, undoes that edit.
    REQUIRE(w.As(h, [&] { return AddOn(h, "Line", 3000, 3100); }) != nullptr);
    CHECK(h.Undo().CanUndo());
    CHECK(h.Undo().GetUndoString() == "Undo: Effect(s) Added");
    REQUIRE(w.As(h, [&] { return h.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 3000) == nullptr);
    CHECK(EffectAt(a, "Line", 3000) == nullptr);
    // A client loses its unsent edit's undo once the host is gone, and edits
    // become plain local ones.
    w.net.Disconnect(h.lt->LocalPeer());
    w.RunFor(50);
    REQUIRE(a.ctl->GetState() == CollabController::State::Ended);
    REQUIRE(w.As(a, [&] { return AddOn(a, "Line", 7000, 7100); }) != nullptr);
    w.Pump();
    CHECK(EffectAt(a, "Line", 7000) != nullptr);
    REQUIRE(a.Undo().CanUndo());
    REQUIRE(w.As(a, [&] { return a.Undo().UndoLastStep(); }));
    CHECK(EffectAt(a, "Line", 7000) == nullptr);
}

TEST_CASE("A client that joined against a session copy can keep it after the session", "[collab][controller]") {
    Show show;
    TempDir dev;
    TempDir sessions;
    CopyShowFiles(show.path, dev.path, { "xlights_networks.xml", "xlights_rgbeffects.xml" });
    WriteFile(dev.path / "xlights_rgbeffects.xml", ChangedLayout(ReadFile(show.dir.File("xlights_rgbeffects.xml"))));
    World w(show);
    Peer& h = w.Add("Hana", "mac");
    w.HostOn(h);
    // No sequence open on the joining device until the session loads one.
    Peer& a = w.Add("Ari", "ipad", false, dev.path.string(), (sessions.path / "a").string());
    w.JoinTo(a, true);
    RequireConverged(w);
    REQUIRE(a.app->report.has_value());
    CHECK(a.app->report->sessionCopy);
    CHECK(a.app->report->layoutDifference.empty() == false);
    std::string folder;
    CHECK_FALSE(h.ctl->KeepSessionCopy(folder));
    w.As(a, [&] { a.ctl->Leave(); });
    REQUIRE(a.ctl->KeepSessionCopy(folder));
    w.As(a, [&] { a.ctl.reset(); });
    CHECK(std::filesystem::exists(std::filesystem::path(folder) / "xlights_rgbeffects.xml"));
    CHECK_FALSE(a.app->locked);
}

TEST_CASE("Moves on a client stay applied and a rejected one falls back", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];
    // A drag of two neighbours to the right: moved one at a time in the
    // order they were touched, the first would land on the second.
    w.As(a, [&] {
        a.Exec().BeginPreview();
        Effect* on = EffectAt(a, "Grid", 1000);
        Effect* bars = EffectAt(a, "Grid", 2000);
        a.Exec().MoveEffect(on, 1700, 2200);
        a.Exec().MoveEffect(bars, 2700, 3200);
        a.Exec().CommitPreview("");
    });
    w.Pump();
    // Still where the user dropped them, no ghosts.
    CHECK(EffectAt(a, "Grid", 1700) != nullptr);
    CHECK(EffectAt(a, "Grid", 2700) != nullptr);
    CHECK(a.ctl->PendingGhosts().empty());
    RequireConverged(w);
    CHECK(a.app->rejects.empty());
    CHECK(EffectAt(h, "Grid", 1700) != nullptr);
    CHECK(EffectAt(h, "Grid", 2700) != nullptr);
    // One undo puts both back.
    REQUIRE(w.As(a, [&] { return a.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(h, "Grid", 1000) != nullptr);
    CHECK(EffectAt(h, "Grid", 2000) != nullptr);

    // b takes a spot first; a's move there is rejected and snaps back.
    REQUIRE(w.As(b, [&] { return AddOn(b, "Line", 3000, 3500); }) != nullptr);
    w.Pump();
    w.As(a, [&] {
        Step(a);
        a.Exec().MoveEffect(EffectAt(a, "Line", 0), 3100, 3300);
    });
    w.Pump();
    CHECK(EffectAt(a, "Line", 3100) != nullptr);
    RequireConverged(w);
    REQUIRE(a.app->rejects.size() == 1);
    CHECK(a.app->rejects[0].reason == Reason::Overlap);
    CHECK(EffectAt(a, "Line", 0) != nullptr);
    CHECK(EffectAt(a, "Line", 3100) == nullptr);

    // A remote move of the same effect lands first in host order; a's later
    // pending move still wins and never flickers.
    w.As(b, [&] {
        Step(b);
        b.Exec().MoveEffect(EffectAt(b, "Line", 0), 600, 800);
    });
    w.Pump();
    w.As(a, [&] {
        Step(a);
        a.Exec().MoveEffect(EffectAt(a, "Line", 0), 900, 1100);
    });
    w.Pump();
    w.RunFor(5);
    CHECK(EffectAt(a, "Line", 900) != nullptr);
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 900) != nullptr);
}

TEST_CASE("A host edit only lenient mode allows doesn't make clients re-sync", "[collab][controller]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    // The edit-site API applies leniently: an add over an existing effect
    // goes in (strict validation would refuse it as an overlap).
    REQUIRE(w.As(h, [&] { return AddOn(h, "Grid", 1200, 1300); }) != nullptr);
    RequireConverged(w);
    CHECK(a.Client().ResyncCount() == 0);
    CHECK(EffectAt(a, "Grid", 1200) != nullptr);
    // Undo of it goes through the session's strict path and still works.
    REQUIRE(w.As(h, [&] { return h.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(a, "Grid", 1200) == nullptr);
}

namespace {

// Hosts, joins and edits while the host's SaveXsq and the joiner's LoadXsq
// run a nested event loop; returns what each sent from inside its loop.
std::pair<size_t, size_t> RunNestedHooks(Show& show, bool guard) {
    World w(show);
    Peer& h = w.Add("Hana", "mac");
    w.HostOn(h);
    Peer& b = w.Add("Bea", "mac");
    w.JoinTo(b);
    REQUIRE(w.Quiesce());
    Peer& a = w.Add("Ari", "ipad");
    h.ctl->reentryGuard = guard;
    a.ctl->reentryGuard = guard;
    size_t hostSent = 0;
    size_t joinerSent = 0;
    int saves = 0;
    h.app->duringSave = [&] {
        if (saves++ != 0) {
            return;
        }
        size_t before = h.net->sent;
        // Someone else edits while the host is busy saving.
        w.As(b, [&] { AddOn(b, "Line", 6000, 6100); });
        w.NestedLoop(1500);
        hostSent += h.net->sent - before;
    };
    int loads = 0;
    a.app->duringLoad = [&] {
        if (loads++ != 0) {
            return;
        }
        size_t before = a.net->sent;
        w.NestedLoop(1500);
        joinerSent += a.net->sent - before;
    };
    w.JoinTo(a);
    if (guard) {
        RequireConverged(w);
        CHECK(EffectAt(a, "Line", 6000) != nullptr);
        CHECK(a.app->loads == 1);
        CHECK(h.ctl->DeferredCalls() == 0);
        CHECK(a.ctl->DeferredCalls() == 0);
    } else {
        w.Quiesce(5000);
    }
    return { hostSent, joinerSent };
}

} // namespace

TEST_CASE("Hooks that run a nested event loop never let the session re-enter", "[collab][controller]") {
    Show show;
    auto sent = RunNestedHooks(show, true);
    // Everything that arrived meanwhile waited for the hook to return.
    CHECK(sent.first == 0);
    CHECK(sent.second == 0);
}

TEST_CASE("Without the re-entry guard a nested event loop re-enters the session", "[collab][controller]") {
    // The control for the test above: the same run re-enters (answers pings,
    // asks again for the snapshot, commits the other edit) from inside the hooks.
    Show show;
    auto sent = RunNestedHooks(show, false);
    CHECK(sent.first > 0);
    CHECK(sent.second > 0);
}

TEST_CASE("A snapshot load that fails after a nested one succeeded ends the session", "[collab][controller]") {
    // Without the re-entry guard a second snapshot is loaded from inside the
    // first load and makes the session active; then the outer load fails.
    // The session must not carry on with nothing gating or sending edits.
    // The nested loop outlasts the snapshot stall time, after which the
    // client asks for the snapshot again.
    Show show;
    World w(show);
    Peer& h = w.Add("Hana", "mac");
    w.HostOn(h);
    Peer& a = w.Add("Ari", "ipad");
    a.ctl->reentryGuard = false;
    a.app->failLoadCall = 1;
    bool nestedActive = false;
    a.app->duringLoad = [&] {
        if (a.app->loadCalls != 1) {
            return;
        }
        w.NestedLoop(8000);
        nestedActive = a.ctl->InSession();
    };
    w.JoinTo(a);
    w.Quiesce(5000);
    REQUIRE(a.app->loadCalls == 2);
    REQUIRE(nestedActive);
    CHECK(a.ctl->GetState() == CollabController::State::Ended);
    CHECK(a.ctl->EndReason() == Reason::SnapshotFailed);
    CHECK(a.Exec().GetGate() == nullptr);
    CHECK(a.Undo().GetRedirect() == nullptr);
}

TEST_CASE("Edits and undo from inside a notification are recorded and sent later", "[collab][controller]") {
    for (bool guard : { true, false }) {
        INFO("guard " << guard);
        Show show;
        World w(show);
        StartSession(w, 1);
        Peer& h = *w.peers[0];
        Peer& a = *w.peers[1];
        a.ctl->reentryGuard = guard;
        bool fired = false;
        bool undone = true;
        size_t sentInside = 0;
        a.app->duringChange = [&] {
            if (fired) {
                return;
            }
            fired = true;
            size_t before = a.net->sent;
            REQUIRE(AddOn(a, "Line", 5000, 5100) != nullptr);
            undone = a.Undo().UndoLastStep();
            w.NestedLoop(20);
            sentInside = a.net->sent - before;
        };
        REQUIRE(w.As(h, [&] { return AddOn(h, "Grid", 3000, 3100); }) != nullptr);
        if (guard) {
            RequireConverged(w);
        } else {
            w.Quiesce(5000);
        }
        REQUIRE(fired);
        if (guard) {
            CHECK_FALSE(undone);
            CHECK(sentInside == 0);
            CHECK(EffectAt(h, "Line", 5000) != nullptr);
            CHECK(EffectAt(a, "Line", 5000) != nullptr);
        } else {
            // Without the guard the edit goes out (and is undone) from inside the notification.
            CHECK(sentInside > 0);
        }
    }
}

TEST_CASE("A viewer's edits are refused before they apply, its undo is blocked and its layout locked", "[collab][controller][collab_roles]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];
    ParticipantId aid = a.ctl->LocalParticipant();
    REQUIRE(w.As(a, [&] { return AddOn(a, "Line", 3000, 3100); }) != nullptr);
    RequireConverged(w);
    REQUIRE(a.ctl->CanUndo());
    CHECK(a.app->lockReason == LayoutLock::DEFAULT_REASON);

    std::string err;
    CHECK_FALSE(w.As(a, [&] { return a.ctl->SetRole(aid, ParticipantRole::Viewer, err); }));
    CHECK(err == "Only the host can change someone's role.");
    REQUIRE(w.As(h, [&] { return h.ctl->SetRole(aid, ParticipantRole::Viewer, err); }));
    REQUIRE(w.Quiesce());
    CHECK(a.ctl->LocalRole() == ParticipantRole::Viewer);
    CHECK_FALSE(a.ctl->CanEditSequence());
    CHECK(h.ctl->CanEditSequence());
    for (Peer* p : { &h, &a, &b }) {
        CHECK(p->ctl->RoleOf(aid) == ParticipantRole::Viewer);
        CHECK(p->ctl->Participants()[1].role == ParticipantRole::Viewer);
        CHECK(p->ctl->Participants()[2].role == ParticipantRole::Editor);
    }
    REQUIRE(!a.app->notices.empty());
    CHECK(a.app->notices.back() == "The host made you a viewer: you can watch but not change the sequence");
    CHECK(a.app->lockReason == "You're a viewer in this session, so the layout is read-only");
    std::string why;
    CHECK_FALSE(a.ctl->CanEdit(a.Seq().GetElement("Grid")->GetUid(), why));
    CHECK(why == "You're a viewer in this session");

    // Refused before anything applies, even for a moment.
    std::string before = DescribeSequence(a.Seq());
    size_t changes = a.app->changes.size();
    Effect* e = EffectAt(a, "Grid", 1000);
    REQUIRE(e != nullptr);
    w.As(a, [&] {
        Step(a);
        CHECK_FALSE(a.Exec().MoveEffect(e, 1100, 1600));
        CHECK_FALSE(a.Exec().SetEffectSetting(e, "E_TEXTCTRL_Eff_On_Start", "99"));
        CHECK_FALSE(a.Exec().EditEffect(e, [](Effect& x) { x.SetStartTimeMS(1200); }));
        CHECK_FALSE(a.Exec().DeleteEffect(e));
        CHECK_FALSE(a.Exec().SetLayerName(LayerOf(a, "Grid"), "Mine"));
        CommandResult r = a.Exec().Execute(std::make_unique<DeleteEffectCommand>(LocatorOf(e)));
        CHECK(r.status == CommandStatus::Refused);
        CHECK(r.message == "You're a viewer in this session, so you can't make changes");
        CHECK(DescribeSequence(a.Seq()) == before);
    });
    w.RunFor(50);
    CHECK(a.app->changes.size() == changes);
    REQUIRE(!a.app->rejects.empty());
    CHECK(a.app->rejects.back().reason == Reason::Viewer);
    CHECK(a.app->rejects.back().text == "You're a viewer in this session, so you can't make changes");
    size_t told = a.app->rejects.size();
    // An add applies (its caller gets the effect) and comes back out when
    // it would be sent.
    REQUIRE(w.As(a, [&] { return AddOn(a, "Grid", 4000, 4100); }) != nullptr);
    w.RunFor(50);
    CHECK(EffectAt(a, "Grid", 4000) == nullptr);
    CHECK(DescribeSequence(a.Seq()) == before);
    REQUIRE(a.app->changes.size() > changes);
    // Taken back out: what it touched renders again.
    CHECK(a.app->changes.back().cause == SequenceChange::Cause::OwnEditRejected);
    // One refusal is told once, not at every step of a drag.
    CHECK(a.app->rejects.size() == told);

    // Undo and redo are refused too.
    CHECK_FALSE(a.ctl->CanUndo());
    CHECK_FALSE(a.Undo().CanUndo());
    CHECK_FALSE(w.As(a, [&] { return a.Undo().UndoLastStep(); }));
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 3000) != nullptr);
    CHECK(EffectAt(h, "Grid", 4000) == nullptr);
    CHECK(h.Host().RejectCounts().count(Reason::Viewer) == 0); // none of it reached the host

    // An editor again.
    REQUIRE(w.As(h, [&] { return h.ctl->SetRole(aid, ParticipantRole::Editor, err); }));
    REQUIRE(w.Quiesce());
    CHECK(a.ctl->LocalRole() == ParticipantRole::Editor);
    CHECK(a.app->lockReason == LayoutLock::DEFAULT_REASON);
    CHECK(a.ctl->CanUndo());
    w.As(a, [&] {
        Step(a);
        CHECK(a.Exec().MoveEffect(EffectAt(a, "Grid", 1000), 1100, 1600));
    });
    RequireConverged(w);
    CHECK(EffectAt(h, "Grid", 1100) != nullptr);
}

TEST_CASE("The host can admit joiners as viewers through the controller", "[collab][controller][collab_roles]") {
    Show show;
    World w(show);
    StartSession(w, 1);
    Peer& h = *w.peers[0];
    CHECK(h.ctl->JoinRole() == ParticipantRole::Editor);
    w.As(h, [&] { h.ctl->SetJoinRole(ParticipantRole::Viewer); });
    CHECK(h.ctl->JoinRole() == ParticipantRole::Viewer);
    Peer& b = w.Add("Bea", "mac");
    w.JoinTo(b);
    REQUIRE(w.Quiesce());
    CHECK(b.ctl->LocalRole() == ParticipantRole::Viewer);
    CHECK(w.peers[1]->ctl->LocalRole() == ParticipantRole::Editor);
    CHECK(b.app->lockReason == "You're a viewer in this session, so the layout is read-only");
    w.As(b, [&] {
        Step(b);
        CHECK_FALSE(b.Exec().MoveEffect(EffectAt(b, "Grid", 1000), 1100, 1600));
    });
}

TEST_CASE("Rows assigned to someone else refuse local edits and show who holds them", "[collab][controller][collab_roles]") {
    Show show;
    World w(show);
    StartSession(w, 2);
    Peer& h = *w.peers[0];
    Peer& a = *w.peers[1];
    Peer& b = *w.peers[2];
    ParticipantId aid = a.ctl->LocalParticipant();
    ParticipantId bid = b.ctl->LocalParticipant();
    SeqObjectId line = h.Seq().GetElement("Line")->GetUid();
    SeqObjectId grid = h.Seq().GetElement("Grid")->GetUid();
    std::string err;
    REQUIRE(w.As(h, [&] { return h.ctl->Assign(line, aid, err); }));
    REQUIRE(w.Quiesce());
    for (Peer* p : { &h, &a, &b }) {
        auto as = p->ctl->AssignmentOf(line);
        REQUIRE(as.has_value());
        CHECK(as->owner == aid);
        CHECK(as->name == "Line");
        CHECK_FALSE(p->ctl->AssignmentOf(grid).has_value());
        auto ps = p->ctl->Participants();
        CHECK(ps[1].assignedRows == std::vector<std::string>{ "Line" });
        CHECK(ps[2].assignedRows.empty());
    }
    std::string why;
    CHECK_FALSE(b.ctl->CanEdit(line, why));
    CHECK(why == "Assigned to Ari");
    CHECK_FALSE(h.ctl->CanEdit(line, why));
    CHECK(a.ctl->CanEdit(line, why));
    CHECK(b.ctl->CanEdit(grid, why));

    // Others' edits there are refused before they apply; the host's too.
    std::string before = DescribeSequence(h.Seq());
    w.As(b, [&] {
        Step(b);
        CHECK_FALSE(b.Exec().MoveEffect(EffectAt(b, "Line", 0), 100, 300));
    });
    w.As(h, [&] {
        Step(h);
        CHECK_FALSE(h.Exec().SetEffectSetting(EffectAt(h, "Line", 0), "E_TEXTCTRL_X", "1"));
    });
    // An add (which the executor can't refuse in advance) is taken back out.
    REQUIRE(w.As(h, [&] { return AddOn(h, "Line", 4000, 4100); }) != nullptr);
    w.RunFor(50);
    CHECK(EffectAt(h, "Line", 4000) == nullptr);
    RequireConverged(w);
    CHECK(DescribeSequence(h.Seq()) == before);
    REQUIRE(!b.app->rejects.empty());
    CHECK(b.app->rejects.back().reason == Reason::Assigned);
    CHECK(b.app->rejects.back().by == aid);
    CHECK(b.app->rejects.back().text == "That row is assigned to Ari");
    REQUIRE(!h.app->rejects.empty());
    CHECK(h.app->rejects.back().text == "That row is assigned to Ari");
    // The owner edits freely, and everyone else elsewhere.
    REQUIRE(w.As(a, [&] { return AddOn(a, "Line", 4000, 4100); }) != nullptr);
    REQUIRE(w.As(b, [&] { return AddOn(b, "Grid", 4000, 4100); }) != nullptr);
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 4000) != nullptr);
    CHECK(EffectAt(h, "Grid", 4000) != nullptr);

    // Claims checked here first: someone else's row, someone else's release.
    CHECK_FALSE(w.As(b, [&] { return b.ctl->Assign(line, bid, err); }));
    CHECK(err == "Line is already assigned to Ari");
    CHECK_FALSE(w.As(b, [&] { return b.ctl->Release(line, err); }));
    CHECK(err == "Line is assigned to Ari; only they or the host can release it");
    // Two claims on one row at once: the host takes the first and tells the other.
    w.As(a, [&] { CHECK(a.ctl->Assign(grid, aid, err)); });
    w.As(b, [&] { CHECK(b.ctl->Assign(grid, bid, err)); });
    REQUIRE(w.Quiesce());
    w.RunFor(50);
    CHECK(h.ctl->AssignmentOf(grid)->owner == aid);
    REQUIRE(!b.app->notices.empty());
    CHECK(b.app->notices.back() == "Grid is already assigned to Ari");
    // The owner releases: b may edit again.
    REQUIRE(w.As(a, [&] { return a.ctl->Release(line, err); }));
    REQUIRE(w.Quiesce());
    w.RunFor(50);
    CHECK(b.ctl->CanEdit(line, why));
    w.As(b, [&] {
        Step(b);
        CHECK(b.Exec().MoveEffect(EffectAt(b, "Line", 0), 100, 300));
    });
    RequireConverged(w);
    CHECK(EffectAt(h, "Line", 100) != nullptr);
}

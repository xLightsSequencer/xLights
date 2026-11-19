// Live layout editing through CollabController: each simulated device has
// its own show context (models, controllers text, open sequence), its own id
// allocator and its own controller, and edits the layout the way an app's
// layout editor does: in memory, then the controller notices.

#include <catch2/catch_test_macros.hpp>

#include "test_collab_join_common.h"

#include "collab/CollabController.h"
#include "collab/CollabLayout.h"
#include "collab/CollabNet.h"
#include "models/Model.h"
#include "render/ShowMergeApply.h"
#include "utils/LayoutObjectId.h"
#include "utils/LayoutRecords.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <deque>
#include <list>
#include <random>
#include <regex>

using namespace Collab;
using namespace CollabTest;

namespace {

pugi::xml_document ParseXml(const std::string& text) {
    pugi::xml_document d;
    d.load_string(text.c_str());
    return d;
}

std::string SaveXml(const pugi::xml_document& d) {
    std::ostringstream os;
    d.save(os, "  ");
    return os.str();
}

// The layout file as an app writes it from memory.
std::string SerializeRgb(xLightsShowContext& ctx) {
    pugi::xml_document doc;
    doc.append_child(pugi::node_declaration).append_attribute("version") = "1.0";
    pugi::xml_node root = doc.append_child("xrgb");
    XmlSerializingVisitor visitor{ root };
    XmlSerializer::SerializeAllModels(ctx.AllModels, visitor);
    XmlSerializer::SerializeAllObjects(ctx.AllObjects, visitor);
    ctx._sequenceViewManager.Save(visitor);
    XmlSerializer::SerializeAllModelSets(ctx.AllModels.GetSetManager(), visitor);
    return SaveXml(doc);
}

class ProbeContext : public HeadlessRenderContext {
public:
    bool AbortRender(int maxTimeMs = 60000) override {
        ++aborts;
        return HeadlessRenderContext::AbortRender(maxTimeMs);
    }
    // As in the apps, deleting a model takes its row out of the open sequence.
    bool IsSequenceLoaded() const override { return sequenceOpen; }
    int aborts = 0;
    bool sequenceOpen = false;
};

// One device's app with live layout. Real mode: the layout is the show
// context's models (edited through the managers, applied with
// ShowMergeApply). Fake mode: the layout is just text in memory.
class LayoutApp : public ICollabApp {
public:
    LayoutApp(const std::string& show, const std::string& file) : _show(show), _file(file) {}

    void Open(const std::string& xsq) { REQUIRE(Load(xsq)); }

    // ---- ICollabDocument ----
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

    // ---- ICollabApp ----
    void SetCollaborationLock(bool l, const std::string& reason) override {
        locked = l;
        lockReason = reason;
        if (_ctx) {
            _ctx->SetCollaborationLock(l, reason);
        }
    }
    void OnRejected(const RejectNotice& n) override { rejects.push_back(n); }
    void OnJoinReport(const JoinReport& r) override { report = r; }
    void OnSequenceChanged(const SequenceChange& c) override { changes.push_back(c); }

    // ---- ICollabLayout ----
    bool SupportsLiveLayout() override { return live; }
    uint64_t LayoutGeneration() override { return gen; }
    bool LayoutEditInProgress() override { return editing; }
    bool SerializeLayout(LayoutFiles& out) override {
        if (fake) {
            out = fakeMemory;
            return true;
        }
        if (!_ctx) {
            return false;
        }
        out.rgbEffects = SerializeRgb(*_ctx);
        out.networks = networks;
        return true;
    }
    bool ApplyLayout(const LayoutFiles& current, const LayoutFiles& target, LayoutApplied& out) override {
        if (failApply > 0) {
            --failApply;
            ++failedApplies;
            return false;
        }
        ++applies;
        if (_ctx && Sequence().GetCommandExecutor().InPreview()) {
            ++appliesDuringEdit;
        }
        ++gen;
        if (fake) {
            fakeMemory = target;
            out.layoutChanged = true;
            return true;
        }
        if (!_ctx->AbortRender()) {
            return false;
        }
        pugi::xml_document c = ParseXml(current.rgbEffects);
        pugi::xml_document t = ParseXml(target.rgbEffects);
        auto ch = ShowMergeApply::ApplyLayout(*_ctx, c, t);
        out.layoutChanged = ch.Any();
        out.modelsChanged = ch.modelsChanged;
        out.modelsRemoved = ch.modelsRemoved;
        out.modelsRenamed = ch.modelsRenamed;
        out.controllersChanged = current.networks != target.networks;
        networks = target.networks;
        return true;
    }
    void OnLayoutPresenceChanged() override { ++layoutPresenceChanges; }
    void OnLayoutRejected(const std::string& text) override { layoutNotices.push_back(text); }
    void SetSequenceSettingsLock(bool l, const std::string& reason) override {
        settingsLocked = l;
        if (_ctx) {
            _ctx->SetSequenceSettingsLock(l, reason);
        }
    }

    // ---- what a layout editor does ----
    Model* M(const std::string& name) { return _ctx->AllModels.GetModel(name); }
    void Touch() { ++gen; }
    // Edits the layout file's text and brings memory to it, as an editor's
    // change would (deletes and renames reach the open sequence).
    void EditRgb(const std::function<void(pugi::xml_node root)>& edit) {
        std::string before = SerializeRgb(*_ctx);
        pugi::xml_document d = ParseXml(before);
        edit(d.document_element());
        pugi::xml_document b = ParseXml(before);
        ShowMergeApply::ApplyLayout(*_ctx, b, d);
        Touch();
    }
    LayoutFiles Memory() {
        LayoutFiles f;
        REQUIRE(SerializeLayout(f));
        return f;
    }
    xLightsShowContext& Ctx() { return *_ctx; }

    bool live = true;
    bool fake = false;
    LayoutFiles fakeMemory;
    uint64_t gen = 1;
    bool editing = false;
    int failApply = 0;
    int appliesDuringEdit = 0; // ApplyLayout while a local edit (preview) was open
    int failedApplies = 0;
    int applies = 0;
    std::string networks;
    bool locked = false;
    bool settingsLocked = false;
    std::string lockReason;
    int layoutPresenceChanges = 0;
    std::vector<std::string> layoutNotices;
    std::vector<RejectNotice> rejects;
    std::vector<SequenceChange> changes;
    std::optional<JoinReport> report;
    int loads = 0;

private:
    bool Load(const std::string& xsq) {
        _seqFile.reset();
        _ctx.reset();
        _ctx = std::make_unique<ProbeContext>();
        if (!_ctx->LoadShowFolder(_show, _media)) {
            return false;
        }
        networks = ReadFile((std::filesystem::path(_show) / "xlights_networks.xml").string());
        _seqFile = std::make_unique<SequenceFile>(xsq);
        auto doc = _seqFile->Open(_show, false, xsq);
        if (!doc || !_ctx->LoadSequenceElements(*_seqFile, *doc)) {
            return false;
        }
        _ctx->sequenceOpen = true;
        if (locked) {
            _ctx->SetCollaborationLock(true, lockReason);
        }
        if (settingsLocked) {
            _ctx->SetSequenceSettingsLock(true);
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
    bool Listen(const std::string& code, const SessionAdvert& advert, std::string&) override {
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

struct Peer {
    std::string name;
    IdState ids;
    std::unique_ptr<LayoutApp> app;
    LoopbackTransport* lt = nullptr;
    std::deque<std::function<void()>> posted;
    std::unique_ptr<CollabController> ctl;

    SequenceElements& Seq() { return app->Sequence(); }
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
    ParticipantId Id() const { return ctl->LocalParticipant(); }
};

// The collaboration test show with layout ids, a group and a controller,
// written the way memory serializes it so every device's own folder holds
// the session's layout.
// Controllers log to "curl"; a bare test process has no such logger.
struct CurlLogger {
    CurlLogger() {
        if (!spdlog::get("curl")) {
            spdlog::register_logger(std::make_shared<spdlog::logger>("curl", std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
};

struct LayoutShow {
    CurlLogger curl;
    Show show;

    LayoutShow() {
        show.dir.Write("xlights_networks.xml",
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">\n"
                       "  <Controller Type=\"Null\" Id=\"64001\" Name=\"Bulk\" xlId=\"c-bulk\"><network NetworkType=\"NULL\" MaxChannels=\"500000\"/></Controller>\n"
                       "</Networks>\n");
        show.dir.Write("xlights_rgbeffects.xml",
                       "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n"
                       "<model name=\"Grid\" xlId=\"m-grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
                       "<subModel name=\"Top\" xlId=\"m-grid-top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>\n"
                       "<model name=\"Line\" xlId=\"m-line\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"4\"/>\n"
                       "<model name=\"Arch\" xlId=\"m-arch\" DisplayAs=\"Arches\" StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"8\"/>\n"
                       "<model name=\"Star\" xlId=\"m-star\" DisplayAs=\"Star\" StartChannel=\"300\" NumStrings=\"1\" NodesPerString=\"10\"/>\n"
                       "<model name=\"Pole\" xlId=\"m-pole\" DisplayAs=\"Single Line\" StartChannel=\"400\" NumStrings=\"2\" NodesPerString=\"3\"/>\n"
                       "</models>\n<modelGroups>\n"
                       "<modelGroup name=\"Yard\" xlId=\"g-yard\" models=\"Arch,Star\" layout=\"minimalGrid\" GridSize=\"400\"/>\n"
                       "</modelGroups>\n</xrgb>\n");
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(show.path));
        show.dir.Write("xlights_rgbeffects.xml", SerializeRgb(ctx));
    }
};

class World {
public:
    explicit World(LayoutShow& show, LoopbackOptions o = {}) : net(o), _show(show.show) {
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

    Peer& Add(const std::string& name, bool live = true, const std::string& showPath = "") {
        auto p = std::make_unique<Peer>();
        Peer* pp = p.get();
        pp->name = name;
        pp->ids.counter = _baseCounter;
        pp->lt = &net.AddPeer();
        pp->app = std::make_unique<LayoutApp>(showPath.empty() ? _show.path : showPath, _show.dir.File("dev" + std::to_string(peers.size())));
        pp->app->live = live;
        peers.push_back(std::move(p));
        Scope s(*pp);
        pp->app->Open(_show.start);
        CollabNetFactory f;
        f.transport = [this, pp] { return std::make_unique<LoopNet>(*pp->lt, dir); };
        f.available = [] { return true; };
        f.pairingCode = [] { return std::string("424242"); };
        ControllerOptions opts;
        opts.appVersion = "test";
        opts.deviceKind = "mac";
        opts.session.verifyHashes = true;
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

    bool Quiet() {
        Pump();
        if (net.InFlight() != 0) {
            return false;
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

    // Lets layout edits rest, be proposed and committed everywhere.
    void Settle() {
        RunFor(600);
        REQUIRE(Quiesce());
        RunFor(600);
        REQUIRE(Quiesce());
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

void StartSession(World& w, int clients) {
    Peer& h = w.Add("Hana");
    w.HostOn(h);
    const char* names[] = { "Ari", "Bea", "Cy" };
    for (int i = 0; i < clients; ++i) {
        Peer& c = w.Add(names[i]);
        w.JoinTo(c);
    }
    REQUIRE(w.Quiesce());
    for (auto& p : w.peers) {
        REQUIRE(p->ctl->InSession());
    }
}

// Everyone holds the same session layout, memory matches it, and the
// sequences are one document.
void RequireConverged(World& w) {
    REQUIRE(w.Quiesce());
    Peer& h = w.host();
    uint64_t layout = h.ctl->Session()->LayoutHash();
    uint64_t memory = w.As(h, [&] { return LayoutDelta::Hash(h.app->Memory()); });
    uint64_t doc = DocHasher::FullHash(h.Seq());
    CHECK(layout == memory);
    for (size_t i = 1; i < w.peers.size(); ++i) {
        Peer& c = *w.peers[i];
        if (c.ctl->GetState() != CollabController::State::Active) {
            continue;
        }
        INFO("peer " << c.name);
        CHECK(c.ctl->Session()->LayoutHash() == layout);
        CHECK(w.As(c, [&] { return LayoutDelta::Hash(c.app->Memory()); }) == memory);
        CHECK(DocHasher::FullHash(c.Seq()) == doc);
        CHECK(c.Client().ResyncCount() == 0);
        CHECK(c.Client().HashVerifyFailures() == 0);
    }
    CHECK(h.Host().HashVerifyFailures() == 0);
}

std::string Desc(Peer& p, const std::string& model) {
    Model* m = p.app->M(model);
    return m == nullptr ? std::string("<none>") : m->GetDescription();
}

void SetDesc(World& w, Peer& p, const std::string& model, const std::string& d) {
    w.As(p, [&] {
        Model* m = p.app->M(model);
        REQUIRE(m != nullptr);
        m->SetDescription(d);
        p.app->Touch();
    });
}

bool HasNotice(Peer& p, const std::string& part) {
    for (const auto& n : p.app->layoutNotices) {
        if (n.find(part) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void FlushLayout(World& w, Peer& p) {
    w.As(p, [&] { p.ctl->FlushLayout(); });
}

void RemoveModel(pugi::xml_node root, const std::string& name) {
    for (pugi::xml_node m : root.child("models").children("model")) {
        if (name == m.attribute("name").value()) {
            root.child("models").remove_child(m);
            break;
        }
    }
    for (pugi::xml_node g : root.child("modelGroups").children("modelGroup")) {
        auto members = LayoutRecords::SplitMembers(g.attribute("models").value());
        members.erase(std::remove(members.begin(), members.end(), name), members.end());
        g.attribute("models").set_value(LayoutRecords::JoinMembers(members).c_str());
    }
}

} // namespace

TEST_CASE("Live layout: the layout is open and sequence settings are frozen", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 1);
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(p->ctl->LiveLayout());
        CHECK_FALSE(p->app->locked);
        CHECK(p->app->settingsLocked);
        CHECK_FALSE(p->app->Ctx().IsLayoutLocked());
        CHECK(p->app->Ctx().RefusesSequenceSettingsWrite("test"));
    }
    CHECK(w.host().ctl->CanDriveControllers());
    CHECK_FALSE(w.peers[1]->ctl->CanDriveControllers());
    w.As(*w.peers[1], [&] { w.peers[1]->ctl->Leave(); });
    w.RunFor(50);
    CHECK_FALSE(w.peers[1]->app->settingsLocked);
    CHECK_FALSE(w.peers[1]->app->Ctx().RefusesSequenceSettingsWrite("test"));
    CHECK(w.peers[1]->ctl->CanDriveControllers());
    w.As(w.host(), [&] { w.host().ctl->End(); });
    w.RunFor(50);
    CHECK_FALSE(w.host().app->settingsLocked);
    CHECK_FALSE(w.host().app->locked);
}

TEST_CASE("Live layout: a client's edit is proposed once memory rests and reaches everyone", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    Rev head = h.Host().HeadRev();
    SetDesc(w, ari, "Arch", "from ari");
    w.RunFor(100);
    // still resting
    CHECK(h.Host().HeadRev() == head);
    w.Settle();
    CHECK(h.Host().HeadRev() == head + 1);
    CHECK(Desc(h, "Arch") == "from ari");
    CHECK(Desc(bea, "Arch") == "from ari");
    RequireConverged(w);

    // the host's own edit
    SetDesc(w, h, "Star", "from hana");
    w.Settle();
    CHECK(Desc(ari, "Star") == "from hana");
    CHECK(Desc(bea, "Star") == "from hana");
    RequireConverged(w);
    // a controller edit travels too
    w.As(h, [&] {
        h.app->networks = std::regex_replace(h.app->networks, std::regex("MaxChannels=\"500000\""), "MaxChannels=\"400000\"");
        h.app->Touch();
    });
    w.Settle();
    CHECK(bea.app->networks.find("MaxChannels=\"400000\"") != std::string::npos);
    RequireConverged(w);
}

TEST_CASE("Live layout: concurrent edits to different attributes of one model both survive", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    w.As(ari, [&] {
        ari.app->M("Arch")->SetPixelSize(5);
        ari.app->Touch();
        ari.ctl->FlushLayout();
    });
    w.As(bea, [&] {
        bea.app->M("Arch")->SetTransparency(30);
        bea.app->Touch();
        bea.ctl->FlushLayout();
    });
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(p->app->M("Arch")->GetPixelSize() == 5);
        CHECK(p->app->M("Arch")->GetTransparency() == 30);
    }
    RequireConverged(w);
}

TEST_CASE("Live layout: the later edit of one attribute wins", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    SetDesc(w, ari, "Arch", "ari");
    SetDesc(w, bea, "Arch", "bea");
    FlushLayout(w, ari);
    FlushLayout(w, bea);
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(Desc(*p, "Arch") == "bea");
    }
    CHECK(ari.app->layoutNotices.empty());
    CHECK(bea.app->layoutNotices.empty());
    RequireConverged(w);
}

TEST_CASE("Live layout: an edit to a model someone deleted is dropped with a notice", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    w.As(ari, [&] { ari.app->EditRgb([](pugi::xml_node root) { RemoveModel(root, "Star"); }); });
    SetDesc(w, bea, "Star", "bea");
    FlushLayout(w, ari);
    FlushLayout(w, bea);
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(p->app->M("Star") == nullptr);
    }
    CHECK(HasNotice(bea, "'Star' was deleted by Ari"));
    CHECK(ari.app->layoutNotices.empty());
    RequireConverged(w);
}

TEST_CASE("Live layout: a model both add under one name refuses the later proposal", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    auto add = [&](Peer& p, const std::string& displayAs) {
        w.As(p, [&] {
            pugi::xml_document d = ParseXml("<model name=\"Tree\" DisplayAs=\"" + displayAs +
                                            "\" StartChannel=\"600\" NumStrings=\"1\" NodesPerString=\"5\"/>");
            REQUIRE(p.app->Ctx().AllModels.createAndAddModel(d.document_element(), 100, 100) != nullptr);
            p.app->Touch();
            p.ctl->FlushLayout();
        });
    };
    add(ari, "Single Line");
    add(bea, "Arches");
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        REQUIRE(p->app->M("Tree") != nullptr);
        CHECK(p->app->M("Tree")->GetDisplayAs() == DisplayAsType::SingleLine);
    }
    CHECK(HasNotice(bea, "'Tree' was added by someone else"));
    RequireConverged(w);
}

TEST_CASE("Live layout: a commit is merged under a pending local edit", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    bea.app->editing = true;
    SetDesc(w, bea, "Arch", "bea");
    SetDesc(w, ari, "Star", "ari");
    w.Settle();
    // Bea's edit is still in progress: kept in memory, not proposed
    CHECK(Desc(bea, "Arch") == "bea");
    CHECK(Desc(bea, "Star") == "ari");
    CHECK(Desc(h, "Arch").empty());
    CHECK(Desc(h, "Star") == "ari");
    bea.app->editing = false;
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(Desc(*p, "Arch") == "bea");
        CHECK(Desc(*p, "Star") == "ari");
    }
    RequireConverged(w);
}

TEST_CASE("Live layout: an edit in progress holds the proposal", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 1);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    ari.app->editing = true;
    SetDesc(w, ari, "Pole", "dragging");
    FlushLayout(w, ari);
    w.RunFor(2000);
    CHECK(Desc(h, "Pole").empty());
    ari.app->editing = false;
    w.Settle();
    CHECK(Desc(h, "Pole") == "dragging");
    RequireConverged(w);
}

TEST_CASE("Live layout: a commit memory can't take yet is held and retried", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    SECTION("on a client") {
        bea.app->failApply = 2;
        SetDesc(w, ari, "Arch", "ari");
        w.Settle();
        CHECK(bea.app->failedApplies == 2);
        CHECK(Desc(bea, "Arch") == "ari");
    }
    SECTION("on the host, for a client's proposal") {
        h.app->failApply = 3;
        SetDesc(w, ari, "Arch", "ari");
        w.Settle();
        CHECK(h.app->failedApplies == 3);
        CHECK(Desc(h, "Arch") == "ari");
        CHECK(Desc(bea, "Arch") == "ari");
    }
    RequireConverged(w);
}

TEST_CASE("Live layout: a rename and a delete of models with effects leave every peer one document", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    REQUIRE(h.Seq().GetElement("Grid") != nullptr);
    int gridEffects = h.Seq().GetElement("Grid")->GetEffectCount();
    REQUIRE(gridEffects > 0);
    REQUIRE(h.Seq().GetElement("Line")->GetEffectCount() > 0);
    // As the layout editors do it: the model, then the sequence row.
    w.As(ari, [&] {
        ari.app->Ctx().AllModels.Rename("Grid", "Grid 2");
        REQUIRE(ari.app->M("Grid 2") != nullptr);
        ari.Seq().RenameModelInViews("Grid", "Grid 2");
        ari.app->Touch();
    });
    w.As(bea, [&] {
        REQUIRE(bea.app->Ctx().AllModels.Delete("Line", false));
        bea.app->Touch();
    });
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(p->app->M("Grid") == nullptr);
        CHECK(p->app->M("Grid 2") != nullptr);
        CHECK(p->app->M("Line") == nullptr);
        CHECK(p->Seq().GetElement("Line") == nullptr);
        REQUIRE(p->Seq().GetElement("Grid 2") != nullptr);
        CHECK(p->Seq().GetElement("Grid 2")->GetEffectCount() == gridEffects);
    }
    RequireConverged(w);

    // The host does the same to another model.
    w.As(h, [&] {
        h.app->Ctx().AllModels.Rename("Grid 2", "Matrix");
        REQUIRE(h.app->M("Matrix") != nullptr);
        h.Seq().RenameModelInViews("Grid 2", "Matrix");
        h.app->Touch();
    });
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        REQUIRE(p->Seq().GetElement("Matrix") != nullptr);
        CHECK(p->Seq().GetElement("Matrix")->GetEffectCount() == gridEffects);
    }
    RequireConverged(w);
}

TEST_CASE("Live layout: an effect added to a row whose model is being deleted converges", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    SECTION("the add first") {
        w.As(ari, [&] {
            ari.Seq().get_undo_mgr().CreateUndoStep();
            ari.Seq().GetCommandExecutor().AddEffect(ari.Seq().GetElement("Line")->GetEffectLayer(0), "On", "", "", 3000, 3100, 0, false);
        });
        w.As(bea, [&] {
            REQUIRE(bea.app->Ctx().AllModels.Delete("Line", false));
            bea.app->Touch();
        });
    }
    SECTION("the delete first") {
        w.As(bea, [&] {
            REQUIRE(bea.app->Ctx().AllModels.Delete("Line", false));
            bea.app->Touch();
        });
        w.Pump();
        w.As(ari, [&] {
            ari.Seq().get_undo_mgr().CreateUndoStep();
            ari.Seq().GetCommandExecutor().AddEffect(ari.Seq().GetElement("Line")->GetEffectLayer(0), "On", "", "", 3000, 3100, 0, false);
        });
    }
    w.Settle();
    for (auto& p : w.peers) {
        INFO(p->name);
        CHECK(p->app->M("Line") == nullptr);
        CHECK(p->Seq().GetElement("Line") == nullptr);
    }
    RequireConverged(w);
}

TEST_CASE("Live layout: a joiner with a different layout opens against a session copy", "[collab][layout][controller][join]") {
    LayoutShow show;
    World w(show);
    Peer& h = w.Add("Hana");
    w.HostOn(h);
    SetDesc(w, h, "Arch", "hosted");
    w.RunFor(600);
    auto before = FolderState(show.show.dir.path);
    Peer& ari = w.Add("Ari");
    w.JoinTo(ari);
    REQUIRE(w.Quiesce());
    REQUIRE(ari.ctl->InSession());
    REQUIRE(ari.app->report);
    CHECK(ari.app->report->sessionCopy);
    CHECK(ari.app->report->refusal.empty());
    CHECK(ari.app->report->layoutDifference.find("Arch") != std::string::npos);
    CHECK(ari.app->ShowFolder() != show.show.path);
    CHECK(Desc(ari, "Arch") == "hosted");
    // the joiner's own show folder is never touched
    auto after = FolderState(show.show.dir.path);
    for (const auto& [f, state] : before) {
        if (f.rfind("xlights_", 0) == 0) {
            CHECK(after[f] == state);
        }
    }
    RequireConverged(w);
    SetDesc(w, ari, "Star", "copy");
    w.Settle();
    CHECK(Desc(h, "Star") == "copy");
    RequireConverged(w);
}

TEST_CASE("Live layout: a joiner with the same layout uses its own folder", "[collab][layout][controller][join]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 1);
    Peer& ari = *w.peers[1];
    REQUIRE(ari.app->report);
    CHECK_FALSE(ari.app->report->sessionCopy);
    CHECK(ari.app->ShowFolder() == show.show.path);
    CHECK(ari.app->applies == 0);
    RequireConverged(w);
}

TEST_CASE("Live layout: the first to take a layout lock holds it", "[collab][layout][controller][presence]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    std::string heldBy;
    CHECK(w.As(ari, [&] { return ari.ctl->BeginLayoutLock({ "m-arch", "Arch", "submodels" }, heldBy); }));
    w.As(ari, [&] { ari.ctl->SetLayoutSelection({ "m-arch", "m-star" }); });
    w.RunFor(500);
    CHECK_FALSE(w.As(bea, [&] { return bea.ctl->BeginLayoutLock({ "m-arch", "Arch", "faces" }, heldBy); }));
    CHECK(heldBy == "Ari");
    std::string why;
    CHECK(bea.ctl->LayoutObjectReadOnly("m-arch", why));
    CHECK(why == "Ari is editing Arch's submodels");
    CHECK(h.ctl->LayoutLockHolder("m-arch") == ari.Id());
    CHECK(ari.ctl->LayoutLockHolder("m-arch") == kNoParticipant);
    CHECK_FALSE(ari.ctl->LayoutObjectReadOnly("m-arch", why));
    CHECK(bea.ctl->LayoutSelectedBy("m-star") == std::vector<ParticipantId>{ ari.Id() });
    CHECK(bea.app->layoutPresenceChanges > 0);

    // Released: someone else may take it.
    w.As(ari, [&] { ari.ctl->EndLayoutLock(); });
    w.RunFor(500);
    CHECK(bea.ctl->LayoutLockHolder("m-arch") == kNoParticipant);

    // Both at once: the host saw Ari's first; Bea is told.
    CHECK(w.As(ari, [&] { return ari.ctl->BeginLayoutLock({ "m-star", "Star", "states" }, heldBy); }));
    CHECK(w.As(bea, [&] { return bea.ctl->BeginLayoutLock({ "m-star", "Star", "faces" }, heldBy); }));
    w.RunFor(500);
    CHECK(HasNotice(bea, "Ari started editing Star first"));
    CHECK(ari.app->layoutNotices.empty());
    CHECK(bea.ctl->LayoutLockHolder("m-star") == ari.Id());
    CHECK(h.ctl->LayoutLockHolder("m-star") == ari.Id());
    CHECK(ari.ctl->LayoutLockHolder("m-star") == kNoParticipant);

    // Leaving releases it.
    w.As(ari, [&] { ari.ctl->Leave(); });
    w.RunFor(500);
    CHECK(bea.ctl->LayoutLockHolder("m-star") == kNoParticipant);
    CHECK(h.ctl->LayoutLockHolder("m-star") == bea.Id());
}

TEST_CASE("Live layout: a layout hash mismatch re-syncs the client", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    bea.Client().faults.clientSkipsLayoutUpdate = true;
    SetDesc(w, ari, "Arch", "ari");
    w.Settle();
    bea.Client().faults.clientSkipsLayoutUpdate = false;
    w.RunFor(3000);
    REQUIRE(w.Quiesce());
    CHECK(bea.Client().ResyncCount() >= 1);
    CHECK(bea.ctl->Session()->LayoutHash() == w.host().ctl->Session()->LayoutHash());
    CHECK(Desc(bea, "Arch") == "ari");
    CHECK(w.As(bea, [&] { return LayoutDelta::Hash(bea.app->Memory()); }) == w.host().ctl->Session()->LayoutHash());
    CHECK(DocHasher::FullHash(bea.Seq()) == DocHasher::FullHash(w.host().Seq()));
    // and it goes on editing
    SetDesc(w, bea, "Star", "bea");
    w.Settle();
    CHECK(Desc(ari, "Star") == "bea");
}

TEST_CASE("Live layout: without live layout support the layout stays locked", "[collab][layout][controller]") {
    LayoutShow show;
    SECTION("nobody supports it") {
        World w(show);
        Peer& h = w.Add("Hana", false);
        w.HostOn(h);
        Peer& ari = w.Add("Ari", false);
        w.JoinTo(ari);
        REQUIRE(w.Quiesce());
        for (auto& p : w.peers) {
            INFO(p->name);
            CHECK_FALSE(p->ctl->LiveLayout());
            CHECK(p->app->locked);
            CHECK_FALSE(p->app->settingsLocked);
            CHECK(p->app->Ctx().IsLayoutLocked());
            CHECK(p->app->Ctx().RefusesSequenceSettingsWrite("test"));
        }
        Rev head = h.Host().HeadRev();
        ari.app->Touch();
        w.RunFor(1000);
        CHECK(h.Host().HeadRev() == head);
    }
    SECTION("the host does, this client doesn't") {
        World w(show);
        Peer& h = w.Add("Hana");
        w.HostOn(h);
        Peer& ari = w.Add("Ari", false);
        w.JoinTo(ari);
        REQUIRE(w.Quiesce());
        CHECK(h.ctl->LiveLayout());
        CHECK_FALSE(ari.ctl->LiveLayout());
        CHECK(ari.app->locked);
        CHECK_FALSE(ari.ctl->Session()->LiveLayout());
    }
    SECTION("this client does, the host doesn't") {
        World w(show);
        Peer& h = w.Add("Hana", false);
        w.HostOn(h);
        Peer& ari = w.Add("Ari");
        w.JoinTo(ari);
        REQUIRE(w.Quiesce());
        CHECK_FALSE(ari.ctl->LiveLayout());
        CHECK(ari.app->locked);
        CHECK_FALSE(ari.app->settingsLocked);
        // and a differing layout still refuses the join without fetching
    }
}

// ---------------------------------------------------------------------------
// Fuzz: several peers edit a layout held as text, and the sequence, at once.
// ---------------------------------------------------------------------------

namespace {

// The show's layout plus models, a group and controllers only the fuzz edits.
LayoutFiles FuzzLayout(const std::string& rgb, const std::string& net) {
    pugi::xml_document r = ParseXml(rgb);
    pugi::xml_node models = r.document_element().child("models");
    std::string members;
    for (int i = 1; i <= 6; ++i) {
        pugi::xml_node m = models.append_child("model");
        std::string name = "M" + std::to_string(i);
        m.append_attribute("name") = name.c_str();
        m.append_attribute("xlId") = ("fz-" + name).c_str();
        m.append_attribute("DisplayAs") = "Single Line";
        m.append_attribute("Description") = "";
        m.append_attribute("PixelSize") = "2";
        members += (members.empty() ? "" : ",") + name;
    }
    pugi::xml_node groups = r.document_element().child("modelGroups");
    if (!groups) {
        groups = r.document_element().append_child("modelGroups");
    }
    pugi::xml_node g = groups.append_child("modelGroup");
    g.append_attribute("name") = "FuzzGroup";
    g.append_attribute("xlId") = "fz-group";
    g.append_attribute("models") = members.c_str();
    pugi::xml_document n = ParseXml(net);
    for (int i = 1; i <= 3; ++i) {
        pugi::xml_node c = n.document_element().append_child("Controller");
        std::string name = "K" + std::to_string(i);
        c.append_attribute("Name") = name.c_str();
        c.append_attribute("xlId") = ("fz-" + name).c_str();
        c.append_attribute("Type") = "Null";
        c.append_attribute("Id") = std::to_string(100 + i).c_str();
        c.append_child("network").append_attribute("NetworkType") = "NULL";
    }
    return { SaveXml(r), SaveXml(n) };
}

std::vector<pugi::xml_node> FuzzModels(pugi::xml_node root) {
    std::vector<pugi::xml_node> out;
    for (pugi::xml_node m : root.child("models").children("model")) {
        std::string name = m.attribute("name").value();
        if (name[0] == 'M' || name[0] == 'N') {
            out.push_back(m);
        }
    }
    return out;
}

void RenameInGroups(pugi::xml_node root, const std::string& from, const std::string& to) {
    for (pugi::xml_node g : root.child("modelGroups").children("modelGroup")) {
        auto members = LayoutRecords::SplitMembers(g.attribute("models").value());
        for (auto& m : members) {
            if (m == from) {
                m = to;
            }
        }
        g.attribute("models").set_value(LayoutRecords::JoinMembers(members).c_str());
    }
}

// One random layout edit of `files`, with its cascades.
void FuzzEdit(std::mt19937& rng, LayoutFiles& files, const std::string& who, int& counter) {
    auto pick = [&](int n) { return (int)(rng() % (uint32_t)std::max(1, n)); };
    pugi::xml_document r = ParseXml(files.rgbEffects);
    pugi::xml_document n = ParseXml(files.networks);
    pugi::xml_node root = r.document_element();
    auto models = FuzzModels(root);
    switch (pick(8)) {
    case 0:
    case 1: { // attribute
        if (models.empty()) {
            break;
        }
        pugi::xml_node m = models[pick((int)models.size())];
        const char* attr = pick(2) ? "Description" : "PixelSize";
        if (!m.attribute(attr)) {
            m.append_attribute(attr);
        }
        m.attribute(attr).set_value(attr[0] == 'D' ? (who + std::to_string(++counter)).c_str() : std::to_string(1 + pick(9)).c_str());
        break;
    }
    case 2: { // add
        pugi::xml_node m = root.child("models").append_child("model");
        std::string name = "N" + who + std::to_string(++counter);
        m.append_attribute("name") = name.c_str();
        m.append_attribute("xlId") = LayoutObjectId::Generate().c_str();
        m.append_attribute("DisplayAs") = "Single Line";
        break;
    }
    case 3: { // remove
        if (models.size() < 2) {
            break;
        }
        pugi::xml_node m = models[pick((int)models.size())];
        std::string name = m.attribute("name").value();
        RemoveModel(root, name);
        break;
    }
    case 4: { // rename
        if (models.empty()) {
            break;
        }
        pugi::xml_node m = models[pick((int)models.size())];
        std::string from = m.attribute("name").value();
        std::string to = from.substr(0, 1) + who + std::to_string(++counter);
        m.attribute("name").set_value(to.c_str());
        RenameInGroups(root, from, to);
        break;
    }
    case 5: { // group members
        pugi::xml_node g = root.child("modelGroups").find_child_by_attribute("modelGroup", "name", "FuzzGroup");
        if (!g) {
            break;
        }
        std::vector<std::string> members;
        for (pugi::xml_node m : models) {
            if (pick(2)) {
                members.push_back(m.attribute("name").value());
            }
        }
        std::shuffle(members.begin(), members.end(), rng);
        g.attribute("models").set_value(LayoutRecords::JoinMembers(members).c_str());
        break;
    }
    case 6: { // controller add or rename
        pugi::xml_node nr = n.document_element();
        std::vector<pugi::xml_node> ctls;
        for (pugi::xml_node c : nr.children("Controller")) {
            if (c.attribute("Name").value()[0] == 'K') {
                ctls.push_back(c);
            }
        }
        if (pick(2) || ctls.empty()) {
            pugi::xml_node c = nr.append_child("Controller");
            c.append_attribute("Name") = ("K" + who + std::to_string(++counter)).c_str();
            c.append_attribute("xlId") = LayoutObjectId::Generate().c_str();
            c.append_attribute("Type") = "Null";
        } else {
            ctls[pick((int)ctls.size())].attribute("Name").set_value(("K" + who + std::to_string(++counter)).c_str());
        }
        break;
    }
    case 7: { // controller order
        pugi::xml_node nr = n.document_element();
        std::vector<pugi::xml_node> ctls;
        for (pugi::xml_node c : nr.children("Controller")) {
            ctls.push_back(c);
        }
        if (ctls.size() >= 2) {
            pugi::xml_node a = ctls[pick((int)ctls.size())];
            nr.prepend_move(a);
        }
        break;
    }
    }
    files = { SaveXml(r), SaveXml(n) };
}

} // namespace

TEST_CASE("Live layout fuzz: peers editing the layout and the sequence at once converge", "[collab][layout][fuzz]") {
    LayoutShow show;
    // Every device's own folder holds the starting layout, with the models only the fuzz edits.
    LayoutFiles start = FuzzLayout(ReadFile(show.show.dir.File("xlights_rgbeffects.xml")), ReadFile(show.show.dir.File("xlights_networks.xml")));
    show.show.dir.Write("xlights_rgbeffects.xml", start.rgbEffects);
    show.show.dir.Write("xlights_networks.xml", start.networks);

    LoopbackOptions o;
    o.seed = 20261119;
    o.minDelayMs = 1;
    o.maxDelayMs = 40;
    World w(show, o);
    auto addPeer = [&](const std::string& name) -> Peer& {
        Peer& p = w.Add(name);
        p.app->fake = true;
        p.app->fakeMemory = start;
        return p;
    };
    Peer& h = addPeer("Hana");
    w.HostOn(h);
    const char* names[] = { "Ari", "Bea", "Cy" };
    for (const char* n : names) {
        Peer& c = addPeer(n);
        w.JoinTo(c);
    }
    REQUIRE(w.Quiesce());
    for (auto& p : w.peers) {
        REQUIRE(p->ctl->LiveLayout());
    }

    std::mt19937 rng(7);
    int counter = 0;
    int layoutEdits = 0;
    int sequenceEdits = 0;
    for (int step = 0; step < 300; ++step) {
        Peer& p = *w.peers[rng() % w.peers.size()];
        w.As(p, [&] {
            int what = (int)(rng() % 10);
            if (what < 6) {
                FuzzEdit(rng, p.app->fakeMemory, p.name.substr(0, 1), counter);
                p.app->Touch();
                ++layoutEdits;
                if (rng() % 3 != 0) {
                    p.ctl->FlushLayout();
                }
            } else if (what < 9) {
                const char* row = rng() % 2 ? "Grid" : "Line";
                Element* el = p.Seq().GetElement(row);
                if (el != nullptr && el->GetEffectLayerCount() > 0) {
                    int s = (int)(rng() % 40) * 100;
                    p.Seq().get_undo_mgr().CreateUndoStep();
                    p.Seq().GetCommandExecutor().AddEffect(el->GetEffectLayer(0), "On", "", "", s, s + 50, 0, false);
                    ++sequenceEdits;
                }
            } else {
                p.app->editing = !p.app->editing;
            }
        });
        w.RunFor(rng() % 30);
    }
    for (auto& p : w.peers) {
        p->app->editing = false;
    }
    w.RunFor(3000);
    REQUIRE(w.Quiesce());
    w.RunFor(3000);
    REQUIRE(w.Quiesce());

    INFO(layoutEdits << " layout edits, " << sequenceEdits << " sequence edits, head rev " << h.Host().HeadRev());
    uint64_t layout = h.ctl->Session()->LayoutHash();
    uint64_t doc = DocHasher::FullHash(h.Seq());
    CHECK(LayoutDelta::Hash(h.app->fakeMemory) == layout);
    size_t resyncs = 0;
    for (size_t i = 1; i < w.peers.size(); ++i) {
        Peer& c = *w.peers[i];
        INFO(c.name);
        REQUIRE(c.ctl->InSession());
        CHECK(c.ctl->Session()->LayoutHash() == layout);
        CHECK(LayoutDelta::Hash(c.app->fakeMemory) == layout);
        CHECK(DocHasher::FullHash(c.Seq()) == doc);
        resyncs += c.Client().ResyncCount();
    }
    CHECK(resyncs == 0);
    CHECK(h.Host().LayoutVersionsKept() <= 2);
    // The run did what it is for: many layout commits, some refused.
    size_t refused = 0;
    for (const auto& [reason, n] : h.Host().RejectCounts()) {
        refused += n;
    }
    size_t layoutCommits = 0;
    for (auto& p : w.peers) {
        layoutCommits += p->app->applies;
    }
    INFO("layout edits " << layoutEdits << ", sequence edits " << sequenceEdits << ", head rev " << h.Host().HeadRev() << ", layout applies "
                         << layoutCommits << ", rejects " << refused);
    CHECK(layoutCommits > 100);
}

TEST_CASE("Live layout: a held commit waits for an open local edit before it applies", "[collab][layout][controller]") {
    LayoutShow show;
    World w(show);
    StartSession(w, 2);
    Peer& h = w.host();
    Peer& ari = *w.peers[1];
    Peer& bea = *w.peers[2];
    auto holdThenEdit = [&](Peer& p) {
        // Its first try fails: the change is held, retried on a tick.
        p.app->failApply = 1;
        SetDesc(w, ari, "Arch", "ari");
        // Everyone else ticks; this device doesn't yet.
        for (int i = 0; i < 4000 && p.app->failedApplies == 0; ++i) {
            w.Pump();
            w.net.RunUntil(w.net.Now() + 5);
            w.Pump();
            for (auto& q : w.peers) {
                if (q.get() != &p) {
                    w.As(*q, [&] { q->ctl->Tick(w.net.Now()); });
                }
            }
        }
        REQUIRE(p.app->failedApplies == 1);
        // A drag starts on that device before the next tick.
        w.As(p, [&] { p.Seq().GetCommandExecutor().BeginPreview(); });
        int applies = p.app->applies;
        w.RunFor(1500);
        CHECK(p.app->applies == applies);
        CHECK(Desc(p, "Arch") != "ari");
        w.As(p, [&] { p.Seq().GetCommandExecutor().CancelPreview(); });
        w.Settle();
        CHECK(Desc(p, "Arch") == "ari");
        CHECK(p.app->appliesDuringEdit == 0);
    };
    SECTION("a client's held commit") {
        holdThenEdit(bea);
        CHECK(Desc(h, "Arch") == "ari");
    }
    SECTION("the host's held proposal") {
        holdThenEdit(h);
        CHECK(Desc(bea, "Arch") == "ari");
    }
    RequireConverged(w);
}

#pragma once

// In-process live-session harness: a show folder, a host and clients, each
// with its own copy of the sequence and its own id allocator, connected by a
// LoopbackNetwork.

#include <catch2/catch_test_macros.hpp>

#include "support/TempDir.h"
#include "collab/CollabDocHash.h"
#include "collab/CollabSession.h"
#include "collab/CollabTransport.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "render/SequenceIds.h"
#include "render/SequenceMutationGuard.h"
#include "utils/FileUtils.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace CollabTest {

inline std::string ReadFile(const std::string& p) {
    std::ifstream f(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

// A show folder with a few models and a starting sequence.
struct Show {
    TempDir dir;
    FixFileReset reset;
    std::string path;
    std::string start;

    Show() {
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
        dir.Write("xlights_rgbeffects.xml",
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n"
                  "<model name=\"Grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
                  "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>\n"
                  "<model name=\"Line\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"4\"/>\n"
                  "<model name=\"Arch\" DisplayAs=\"Arches\" StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"8\"/>\n"
                  "<model name=\"Star\" DisplayAs=\"Star\" StartChannel=\"300\" NumStrings=\"1\" NodesPerString=\"10\"/>\n"
                  "<model name=\"Pole\" DisplayAs=\"Single Line\" StartChannel=\"400\" NumStrings=\"2\" NodesPerString=\"3\"/>\n"
                  "</models>\n<modelGroups/>\n</xrgb>\n");
        path = dir.path.string();
        start = dir.File("start.xsq");
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(path));
        SequenceElements& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        SequenceFile sf(start, 50);
        SeqMutationGuard::Scope building;
        Element* beats = se.AddElement("Beats", "timing", true, false, true, false, false);
        EffectLayer* bl = beats->AddEffectLayer();
        for (int i = 0; i < 8; ++i) {
            bl->AddEffect(0, "", "", "", i * 500, i * 500 + 500, 0, false);
        }
        Element* grid = se.AddElement("Grid", "model", true, false, false, false, false);
        if (grid->GetEffectLayerCount() == 0) {
            grid->AddEffectLayer();
        }
        grid->GetEffectLayer(0)->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=10", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", 1000, 1500, 0, false);
        grid->GetEffectLayer(0)->AddEffect(0, "Bars", "", "", 2000, 2500, 0, false);
        Element* line = se.AddElement("Line", "model", true, false, false, false, false);
        if (line->GetEffectLayerCount() == 0) {
            line->AddEffectLayer();
        }
        line->GetEffectLayer(0)->AddEffect(0, "On", "", "", 0, 200, 0, false);
        REQUIRE(sf.Save(se));
    }
};

// One participant's copy of the sequence.
class Doc : public Collab::ICollabDocument {
public:
    Doc(const std::string& show, const std::string& file) : _show(show), _file(file) {}

    // Opens `xsq` (the host's starting point).
    void Open(const std::string& xsq) {
        _ctx = std::make_unique<HeadlessRenderContext>();
        REQUIRE(_ctx->LoadShowFolder(_show, _mediaFolders));
        _seqFile = std::make_unique<SequenceFile>(xsq);
        auto doc = _seqFile->Open(_show, false, xsq);
        REQUIRE(doc);
        REQUIRE(_ctx->LoadSequenceElements(*_seqFile, *doc));
    }
    bool IsOpen() const { return _ctx != nullptr; }

    SequenceElements& Sequence() override { return _ctx->GetSequenceElements(); }
    bool SaveXsq(std::string& out) override {
        lastSnapshotParts = Collab::DocHasher::Components(Sequence());
        out = SaveAs(_file + ".snap.xsq");
        return !out.empty();
    }
    bool LoadXsq(const std::string& xsq) override {
        std::string p = _file + ".load.xsq";
        std::ofstream(p, std::ios::binary) << xsq;
        ++loads;
        _seqFile.reset();
        _ctx.reset();
        _ctx = std::make_unique<HeadlessRenderContext>();
        if (!_ctx->LoadShowFolder(_show, _mediaFolders)) {
            return false;
        }
        _seqFile = std::make_unique<SequenceFile>(p);
        auto doc = _seqFile->Open(_show, false, p);
        return doc && _ctx->LoadSequenceElements(*_seqFile, *doc);
    }
    std::string SaveAs(const std::string& path) {
        _seqFile->SetFullPath(path);
        if (!_seqFile->Save(Sequence())) {
            return "";
        }
        return ReadFile(path);
    }
    std::string Save() { return SaveAs(_file + ".final.xsq"); }

    std::string ShowFolder() override { return _show; }
    std::vector<std::string> AudioFiles() override {
        std::vector<std::string> out;
        if (_seqFile) {
            if (!_seqFile->GetMediaFile().empty()) {
                out.push_back(_seqFile->GetMediaFile());
            }
            for (int i = 0; i < _seqFile->GetAltTrackCount(); ++i) {
                out.push_back(_seqFile->GetAltTrack(i).path);
            }
        }
        return out;
    }
    bool UseShowFolder(const std::string& folder) override {
        if (!allowShowFolderChange) {
            return false;
        }
        _show = folder;
        InstallDirs();
        return true;
    }
    void AddMediaFolder(const std::string& folder) override {
        _mediaFolders.push_back(folder);
        if (_ctx) {
            _ctx->mediaDirectories = _mediaFolders;
        }
        InstallDirs();
    }
    // Each simulated device resolves files against its own folders; the
    // FixFile state is process-wide, so it is switched with the device.
    void InstallDirs() const {
        FileUtils::SetFixFileShowDir(_show);
        FileUtils::SetFixFileDirectories(_mediaFolders);
    }
    const std::list<std::string>& MediaFolders() const { return _mediaFolders; }
    SequenceFile* File() { return _seqFile.get(); }

    bool allowShowFolderChange = true;
    int loads = 0;
    std::string lastSnapshotParts; // DocHasher::Components when the last snapshot was taken

private:
    std::string _show;
    std::string _file;
    std::list<std::string> _mediaFolders;
    std::unique_ptr<HeadlessRenderContext> _ctx;
    std::unique_ptr<SequenceFile> _seqFile;
};

// Each simulated device has its own id counter and prefix, as separate
// processes would. ActAs installs them while acting as that device.
struct IdState {
    uint64_t counter = 0;
    uint16_t peer = 0;
};

class ActAs {
public:
    explicit ActAs(IdState& s) : _s(s) {
        _savedCounter = SeqIds::Counter().load();
        _savedPeer = SeqIds::GetPeer();
        SeqIds::Counter().store(_s.counter);
        SeqIds::SetPeer(_s.peer);
    }
    ~ActAs() {
        _s.counter = SeqIds::Counter().load();
        _s.peer = SeqIds::GetPeer();
        SeqIds::Counter().store(_savedCounter);
        SeqIds::SetPeer(_savedPeer);
    }

private:
    IdState& _s;
    uint64_t _savedCounter;
    uint16_t _savedPeer;
};

struct Node {
    std::unique_ptr<Doc> doc;
    Collab::LoopbackTransport* transport = nullptr;
    std::unique_ptr<Collab::CollabParticipant> session;
    IdState ids;
    std::vector<std::string> log;
    std::map<std::string, int> rejects; // reason -> count (this participant's)
    // onRolledBack: (what the rollback applied, its inverse), serialized
    std::vector<std::pair<nlohmann::json, nlohmann::json>> rolledBack;
    std::vector<std::pair<std::string, std::string>> notices; // (reason, text)
    bool migrates = false; // onHostLost: someone takes over

    Collab::CollabHost& Host() { return static_cast<Collab::CollabHost&>(*session); }
    Collab::CollabClient& Client() { return static_cast<Collab::CollabClient&>(*session); }
    SequenceElements& Seq() { return doc->Sequence(); }
};

// A host plus clients over one LoopbackNetwork, with a virtual clock.
class Sim {
public:
    Sim(Show& show, Collab::LoopbackOptions netOpts, Collab::SessionOptions opts = {}) :
        net(netOpts), _show(show), _opts(opts) {
        _baseCounter = SeqIds::Counter().load() + 1000;
        net.deliveryWrapper = [this](Collab::PeerId to, const std::function<void()>& deliver) {
            Scope a(*nodes[to - 1]);
            deliver();
        };
    }
    ~Sim() {
        // Sessions unhook from their transports before the network goes.
        for (auto& n : nodes) {
            Scope a(*n);
            n->session.reset();
            n->doc.reset();
        }
    }

    // Acting as one device: its id allocator and its file folders.
    struct Scope {
        ActAs ids;
        explicit Scope(Node& n) : ids(n.ids) {
            if (n.doc) {
                n.doc->InstallDirs();
            }
        }
    };

    // A host on `show` (the Sim's show when empty).
    Node& AddHost(const std::string& show = "", std::optional<Collab::SessionOptions> opts = std::nullopt, const std::string& xsq = "") {
        Node& n = NewNode(show);
        Scope a(n);
        n.doc->Open(xsq.empty() ? _show.start : xsq);
        n.session = std::make_unique<Collab::CollabHost>(*n.transport, *n.doc, opts ? *opts : _opts, Events(n));
        n.Host().Start();
        return n;
    }

    Node& AddClient(bool join = true) { return AddClientOn("", std::nullopt, join); }

    // A client whose own show folder is `show` (the Sim's show when empty).
    Node& AddClientOn(const std::string& show, std::optional<Collab::SessionOptions> opts = std::nullopt, bool join = true) {
        Node& n = NewNode(show);
        n.session = std::make_unique<Collab::CollabClient>(*n.transport, *n.doc, opts ? *opts : _opts, Events(n));
        if (join) {
            Join(n);
        }
        return n;
    }

    void Join(Node& n) {
        Scope a(n);
        n.session->Tick(net.Now());
        n.Client().Join(nodes[0]->transport->LocalPeer());
    }

    Node& host() { return *nodes[0]; }

    void Tick() {
        for (auto& n : nodes) {
            Scope a(*n);
            n->session->Tick(net.Now());
        }
    }

    // Runs the network and timers for `ms` of virtual time.
    void RunFor(uint64_t ms, uint64_t step = 5) {
        uint64_t end = net.Now() + ms;
        while (net.Now() < end) {
            net.RunUntil(std::min(end, net.Now() + step));
            Tick();
        }
    }

    bool Quiet() {
        if (net.InFlight() != 0) {
            return false;
        }
        Collab::Rev head = host().Host().HeadRev();
        for (size_t i = 1; i < nodes.size(); ++i) {
            auto& c = nodes[i]->Client();
            if (c.GetState() == Collab::CollabClient::State::Ended || c.GetState() == Collab::CollabClient::State::Idle) {
                continue;
            }
            if (c.GetState() != Collab::CollabClient::State::Active || c.PendingCount() != 0 || c.AppliedRev() != head) {
                return false;
            }
        }
        return true;
    }

    // Runs until everyone has caught up. False if that takes longer than
    // `limitMs` of virtual time.
    bool Quiesce(uint64_t limitMs = 120000) {
        uint64_t end = net.Now() + limitMs;
        while (net.Now() < end) {
            if (Quiet()) {
                return true;
            }
            uint64_t next = net.InFlight() ? std::min(net.NextDeliveryTime(), net.Now() + 5) : net.Now() + 5;
            net.RunUntil(std::max(next, net.Now() + 1));
            Tick();
        }
        return Quiet();
    }

    // Host migration: client `n` (Migrating, after BeginSuccession) takes
    // over as host and opens the session again.
    void TakeOver(Node& n) {
        Scope a(n);
        std::unique_ptr<Collab::CollabParticipant> old = std::move(n.session);
        n.session = std::make_unique<Collab::CollabHost>(*n.transport, *n.doc, _opts, Events(n), static_cast<Collab::CollabClient&>(*old));
        old.reset();
        n.Host().ResumeAsSuccessor();
    }

    // Does `fn` as participant `n` (its id allocator and folders installed).
    template<class F>
    auto As(Node& n, F&& fn) {
        Scope a(n);
        return fn();
    }

    Collab::LoopbackNetwork net;
    std::vector<std::unique_ptr<Node>> nodes;
    std::function<void(Node&, const std::string&)> onLog;

private:
    Node& NewNode(const std::string& show) {
        auto n = std::make_unique<Node>();
        n->transport = &net.AddPeer();
        n->doc = std::make_unique<Doc>(show.empty() ? _show.path : show, _show.dir.File("peer" + std::to_string(nodes.size())));
        // Every device starts counting from the same value, so ids only
        // stay apart through their prefixes.
        n->ids.counter = _baseCounter;
        n->ids.peer = 0;
        nodes.push_back(std::move(n));
        return *nodes.back();
    }

    Collab::SessionEvents Events(Node& n) {
        Collab::SessionEvents ev;
        Node* np = &n;
        ev.onLog = [this, np](const std::string& line) {
            np->log.push_back(line);
            if (onLog) {
                onLog(*np, line);
            }
        };
        ev.onReject = [np](uint64_t, Collab::EditKind, const std::string& reason, Collab::ParticipantId) { ++np->rejects[reason]; };
        ev.onRolledBack = [np](const SequenceCommand& applied, const SequenceCommand& inverse) {
            nlohmann::json a;
            nlohmann::json i;
            applied.Serialize(a);
            inverse.Serialize(i);
            np->rolledBack.emplace_back(a, i);
        };
        ev.onNotice = [np](const std::string& reason, const std::string& text) { np->notices.emplace_back(reason, text); };
        ev.onHostLost = [np] { return np->migrates; };
        return ev;
    }

    Show& _show;
    Collab::SessionOptions _opts;
    uint64_t _baseCounter;
};

// Layers in a sequence (every row, submodel, strand and node layer).
inline void ForEachLayer(SequenceElements& seq, const std::function<void(Element*, EffectLayer*)>& fn) {
    auto row = [&](Element* el) {
        for (size_t i = 0; i < el->GetEffectLayerCount(); ++i) {
            fn(el, el->GetEffectLayer((int)i));
        }
        if (auto* st = dynamic_cast<StrandElement*>(el)) {
            for (int n = 0; n < st->GetNodeLayerCount(); ++n) {
                fn(el, st->GetNodeLayer(n));
            }
        }
    };
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        Element* el = seq.GetElement(i, MASTER_VIEW);
        row(el);
        if (auto* me = dynamic_cast<ModelElement*>(el)) {
            for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
                row(me->GetSubModel(x));
            }
        }
    }
}

// First overlap found in any layer, or "".
inline std::string FindOverlap(SequenceElements& seq) {
    std::string out;
    ForEachLayer(seq, [&](Element* el, EffectLayer* l) {
        std::vector<std::pair<int, int>> t;
        for (Effect* e : l->GetEffects()) {
            t.push_back({ e->GetStartTimeMS(), e->GetEndTimeMS() });
        }
        std::sort(t.begin(), t.end());
        for (size_t i = 1; i < t.size(); ++i) {
            if (t[i].first < t[i - 1].second && out.empty()) {
                out = el->GetFullName() + " " + std::to_string(t[i - 1].first) + "-" + std::to_string(t[i - 1].second) + " / " +
                      std::to_string(t[i].first) + "-" + std::to_string(t[i].second);
            }
        }
    });
    return out;
}

} // namespace CollabTest

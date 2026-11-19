#include <catch2/catch_test_macros.hpp>

#include "test_collab_common.h"

#include "collab/CollabCommandInfo.h"
#include "collab/CollabDocHash.h"
#include "collab/CollabRoles.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "utils/xlImage.h"

#include <cstdio>
#include <cstdlib>
#include <random>
#include <sstream>

// Convergence fuzz: a host and several clients make random edits from the
// real command set at the same time over a loopback network with random
// latency (and, for some seeds, reordering, loss and duplication). Once
// everything has been delivered every participant must hold the host's
// document exactly (same .xsq bytes, same ids), every id must be unique, no
// layer may hold overlapping effects, every reject must be one a concurrent
// edit can cause, and nobody may have needed a re-sync. Then every
// participant undoes everything it can, and they must still agree. With
// roles on, the host also makes clients viewers and editors again and
// everyone claims and releases rows at random; no commit may come from a
// viewer or touch a row assigned to someone else.
//
// Reproduce one seed:  XL_COLLAB_FUZZ_SEED=<seed> xLights-unit-tests "[collab_fuzz]"
// Print each seed as it starts (to find one that crashes): XL_COLLAB_FUZZ_TRACE=1
// More seeds:          XL_COLLAB_FUZZ_SEEDS=<count> xLights-unit-tests "[collab_fuzz]"
// Soak (10k seeds):    xLights-unit-tests "[collab_fuzz_soak]"  (XL_COLLAB_FUZZ_FIRST / _SEEDS pick the range)

using namespace Collab;
using namespace CollabTest;

namespace {

struct FuzzConfig {
    uint32_t seed = 1;
    int steps = 160;
    SessionFaults faults;      // applied to every client (and the host's half)
    bool forceReorder = false; // reordering network for every seed
    bool roles = false;        // random viewer / editor switches and row claims
};

struct FuzzResult {
    bool ok = true;
    std::string why;
    size_t commits = 0;
    size_t rejects = 0;
    size_t clients = 0;
    size_t undone = 0;
    size_t roleRejects = 0; // viewer / assigned
    size_t claims = 0;
};

const std::vector<std::string> kModelEffects{ "On", "Bars", "Twinkle", "Color Wash", "Butterfly" };
const std::vector<std::string> kExtraModels{ "Arch", "Star", "Pole" };

class Fuzzer {
public:
    Fuzzer(Show& show, const FuzzConfig& cfg) :
        _cfg(cfg), _rng(cfg.seed), _sim(show, NetFor(cfg), Options()) {
        _sim.AddHost();
        _clientCount = 2 + (int)(_rng() % 3);
        for (int i = 0; i < _clientCount; ++i) {
            Node& n = _sim.AddClient(false);
            n.session->faults = cfg.faults;
            _sim.Join(n);
        }
        _sim.host().session->faults = cfg.faults;
        _sim.host().session->faults.clientSharesHostIdPrefix = false;
    }

    FuzzResult Run() {
        FuzzResult r;
        if (!_sim.Quiesce()) {
            return Fail("clients did not join");
        }
        HookCommitOrder();
        int lateJoinAt = _rng() % 3 == 0 ? (int)(_rng() % _cfg.steps) : -1;
        for (int step = 0; step < _cfg.steps; ++step) {
            if (step == lateJoinAt) {
                Node& n = _sim.AddClient(false);
                n.session->faults = _cfg.faults;
                _sim.Join(n);
                ++_clientCount;
            }
            Node& n = *_sim.nodes[_rng() % _sim.nodes.size()];
            if (n.session->IsActive()) {
                if (_cfg.roles && Chance(12)) {
                    _sim.As(n, [&] { RoleStep(n); });
                } else {
                    _sim.As(n, [&] { Edit(n); });
                }
            }
            _sim.RunFor(_rng() % 12, 1 + _rng() % 4);
        }
        if (!_sim.Quiesce()) {
            return Fail("did not quiesce");
        }
        std::string why = Check("after edits");
        if (!why.empty()) {
            return Fail(why);
        }
        // Everybody undoes everything they can, interleaved.
        size_t undone = 0;
        for (int round = 0; round < 400; ++round) {
            bool any = false;
            for (auto& n : _sim.nodes) {
                if (n->session->IsActive() && n->session->CanUndo()) {
                    any = true;
                    _sim.As(*n, [&] { n->session->Undo(); });
                    ++undone;
                    _sim.RunFor(_rng() % 6, 1 + _rng() % 3);
                }
            }
            if (!any) {
                break;
            }
        }
        if (!_sim.Quiesce()) {
            return Fail("did not quiesce after undo");
        }
        why = Check("after undo");
        if (!why.empty()) {
            return Fail(why);
        }
        r.commits = _sim.host().Host().HeadRev();
        for (const auto& [reason, count] : _sim.host().Host().RejectCounts()) {
            r.rejects += count;
        }
        r.clients = _sim.nodes.size() - 1;
        r.undone = undone;
        for (const char* reason : { Reason::Viewer, Reason::Assigned }) {
            auto it = _sim.host().Host().RejectCounts().find(reason);
            r.roleRejects += it == _sim.host().Host().RejectCounts().end() ? 0 : it->second;
        }
        r.claims = _claims;
        return r;
    }

private:
    static LoopbackOptions NetFor(const FuzzConfig& cfg) {
        std::mt19937 r(cfg.seed * 7919u + 1);
        LoopbackOptions o;
        o.seed = cfg.seed;
        o.minDelayMs = r() % 3;
        o.maxDelayMs = o.minDelayMs + 2 + r() % 40;
        o.fifoPerLink = cfg.forceReorder ? false : (r() % 4 != 0);
        if (r() % 4 == 0) {
            o.dropRate = (r() % 30) / 1000.0;
            o.duplicateRate = (r() % 30) / 1000.0;
        }
        return o;
    }

    static SessionOptions Options() {
        SessionOptions o;
        o.verifyHashes = true;
        o.retryMs = 60;
        return o;
    }

    FuzzResult Fail(const std::string& why) {
        FuzzResult r;
        r.ok = false;
        std::ostringstream s;
        s << why;
        if (!_orderOk) {
            s << "\n  the host committed an author's proposals out of order";
        }
        s << "\n  net: delay " << _sim.net.Options().minDelayMs << "-" << _sim.net.Options().maxDelayMs << "ms fifo=" << _sim.net.Options().fifoPerLink
          << " drop=" << _sim.net.Options().dropRate << " dup=" << _sim.net.Options().duplicateRate;
        for (size_t i = 0; i < _sim.nodes.size(); ++i) {
            auto& n = *_sim.nodes[i];
            size_t tail = std::min<size_t>(n.log.size(), 6);
            for (size_t k = n.log.size() - tail; k < n.log.size(); ++k) {
                s << "\n  [" << i << "] " << n.log[k].substr(0, 300);
            }
        }
        r.why = s.str();
        return r;
    }

    void HookCommitOrder() {
        _lastSeq.clear();
        _orderOk = true;
        _sim.net.tamper = [this](PeerId from, PeerId, std::string& f) {
            if (from != _sim.host().transport->LocalPeer()) {
                return;
            }
            auto m = DecodeMessage(f);
            if (!m || m->type != MsgType::Commit) {
                return;
            }
            // The same commit goes to every client and may be resent; look
            // at each rev once.
            if (m->rev <= _lastRev) {
                return;
            }
            _lastRev = m->rev;
            uint64_t& last = _lastSeq[m->participant];
            if (m->clientSeq <= last) {
                _orderOk = false;
            }
            last = m->clientSeq;
            if (_cfg.roles && m->layout.is_null()) {
                CheckRoles(*m);
            }
        };
    }

    // Sent as the host commits, before anything else changes: the author
    // may edit every row the command names.
    void CheckRoles(const Message& m) {
        Node& h = _sim.host();
        if (!_rolesWhy.empty()) {
            return;
        }
        if (h.session->RoleOf(m.participant) == ParticipantRole::Viewer) {
            _rolesWhy = "the host committed rev " + std::to_string(m.rev) + " from a viewer";
            return;
        }
        auto cmd = DeserializeCommand(m.command);
        if (!cmd) {
            return;
        }
        AssignmentIndex index;
        index.Build(h.Seq(), h.session->Assignments());
        for (SeqObjectId row : CommandRows(*cmd)) {
            ParticipantId owner = index.OwnerOf(h.Seq(), row);
            if (owner != kNoParticipant && owner != m.participant) {
                _rolesWhy = "the host committed rev " + std::to_string(m.rev) + " by " + std::to_string(m.participant) + " on a row assigned to " +
                            std::to_string(owner);
                return;
            }
        }
    }

    std::string Check(const char* when) {
        std::ostringstream s;
        Node& h = _sim.host();
        std::string hostDesc = DescribeSequence(h.Seq());
        std::string hostXsq = h.doc->Save();
        uint64_t hostHash = DocHasher::FullHash(h.Seq());
        std::string problem;
        if (!h.Seq().ValidateUids(&problem)) {
            s << when << ": host ids: " << problem << "\n";
        }
        std::string overlap = FindOverlap(h.Seq());
        if (!overlap.empty()) {
            s << when << ": overlap on the host: " << overlap << "\n";
        }
        if (!_orderOk) {
            s << when << ": the host committed an author's proposals out of order\n";
        }
        if (!_rolesWhy.empty()) {
            s << when << ": " << _rolesWhy << "\n";
        }
        for (const auto& [reason, text] : h.Host().RejectLog()) {
            if (!AllowedReject(reason, text)) {
                s << when << ": unexpected reject " << reason << ": " << text << "\n";
                break;
            }
        }
        for (size_t i = 0; i < _sim.nodes.size(); ++i) {
            Node& n = *_sim.nodes[i];
            if (n.session->HashVerifyFailures() != 0) {
                s << when << ": participant " << i << " had a stale incremental hash\n";
            }
            if (i == 0) {
                continue;
            }
            auto& c = n.Client();
            if (c.GetState() != CollabClient::State::Active) {
                s << when << ": client " << i << " is not active (" << c.EndReason() << ")\n";
                if (c.EndReason() == Reason::SnapshotFailed && n.doc->IsOpen()) {
                    s << "  host parts at the snapshot vs the client's:\n"
                      << FirstDifference(h.doc->lastSnapshotParts, DocHasher::Components(n.Seq())) << "\n";
                }
                continue;
            }
            if (c.ResyncCount() != 0) {
                s << when << ": client " << i << " needed " << c.ResyncCount() << " re-sync(s)\n";
            }
            if (c.OverlayCount() != 0) {
                s << when << ": client " << i << " still has optimistic state\n";
            }
            if (!n.Seq().ValidateUids(&problem)) {
                s << when << ": client " << i << " ids: " << problem << "\n";
            }
            std::string desc = DescribeSequence(n.Seq());
            if (desc != hostDesc) {
                s << when << ": client " << i << " differs from the host:\n" << FirstDifference(hostDesc, desc) << "\n";
                continue;
            }
            if (DocHasher::FullHash(n.Seq()) != hostHash) {
                s << when << ": client " << i << " hashes differently\n";
            }
            if (n.doc->Save() != hostXsq) {
                s << when << ": client " << i << " saves a different .xsq:\n" << FirstDifference(hostXsq, n.doc->Save()) << "\n";
            }
        }
        return s.str();
    }

    static std::string FirstDifference(const std::string& a, const std::string& b) {
        std::istringstream sa(a), sb(b);
        std::string la, lb;
        int line = 0;
        while (true) {
            bool ga = (bool)std::getline(sa, la);
            bool gb = (bool)std::getline(sb, lb);
            ++line;
            if (!ga && !gb) {
                return "  (no line differs)";
            }
            if (la != lb || ga != gb) {
                return "  line " + std::to_string(line) + "\n    host:   " + la.substr(0, 300) + "\n    client: " + lb.substr(0, 300);
            }
        }
    }

    // What a concurrent edit can legitimately make the host refuse.
    static bool AllowedReject(const std::string& reason, const std::string& text) {
        if (reason == Reason::Overlap || reason == Reason::TargetGone || reason == Reason::Locked || reason == Reason::StaleType ||
            reason == Reason::ChangedSince || reason == Reason::Viewer || reason == Reason::Assigned) {
            return true;
        }
        if (reason == Reason::Invalid) {
            // Name clashes between edits made at the same time.
            for (const char* ok : { "last layer", "is already in the sequence", "media key already in use", "already holds another type" }) {
                if (text.find(ok) != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    // ---- Random edits, built against the participant's own copy --------

    template<class T>
    const T& Pick(const std::vector<T>& v) { return v[_rng() % v.size()]; }
    int Chance(int pct) { return (int)(_rng() % 100) < pct; }

    std::vector<Element*> Rows(SequenceElements& seq, bool withSub) {
        std::vector<Element*> out;
        for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
            Element* el = seq.GetElement(i, MASTER_VIEW);
            out.push_back(el);
            if (auto* me = dynamic_cast<ModelElement*>(el); me != nullptr && withSub) {
                for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
                    out.push_back(me->GetSubModel(x));
                }
            }
        }
        return out;
    }

    EffectLayer* RandomLayer(SequenceElements& seq) {
        auto rows = Rows(seq, Chance(30));
        if (rows.empty()) {
            return nullptr;
        }
        Element* el = Pick(rows);
        if (el->GetEffectLayerCount() == 0) {
            return nullptr;
        }
        return el->GetEffectLayer((int)(_rng() % el->GetEffectLayerCount()));
    }

    Effect* RandomEffect(SequenceElements& seq) {
        for (int tries = 0; tries < 8; ++tries) {
            EffectLayer* l = RandomLayer(seq);
            if (l != nullptr && l->GetEffectCount() > 0) {
                return l->GetEffect((int)(_rng() % l->GetEffectCount()));
            }
        }
        return nullptr;
    }

    bool IsTiming(EffectLayer* l) { return l->GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_TIMING; }

    EffectState NewEffect(EffectLayer* l, int start, int end) {
        EffectState s;
        s.startTimeMS = start;
        s.endTimeMS = end;
        if (IsTiming(l)) {
            s.name = "w" + std::to_string(_rng() % 100);
        } else {
            s.name = Pick(kModelEffects);
            if (Chance(30)) {
                s.settings = "E_SLIDER_Fuzz_A=" + std::to_string(_rng() % 100);
            }
            if (Chance(30)) {
                s.palette = "C_BUTTON_Palette1=#" + std::to_string(100000 + _rng() % 800000) + ",C_CHECKBOX_Palette1=1";
            }
            if (Chance(10) && !_mediaKeys.empty()) {
                s.name = "Pictures";
                s.settings = "E_TEXTCTRL_Pictures_Filename=" + Pick(_mediaKeys);
            }
        }
        return s;
    }

    std::pair<int, int> RandomRange() {
        int start = (int)(_rng() % 400) * 50;
        int len = 50 * (1 + (int)(_rng() % 20));
        return { start, start + len };
    }

    std::unique_ptr<SequenceCommand> AddEffect(SequenceElements& seq) {
        EffectLayer* l = RandomLayer(seq);
        if (l == nullptr) {
            return nullptr;
        }
        auto [s, e] = RandomRange();
        if (!l->GetRangeIsClearMS(s, e)) {
            return nullptr;
        }
        return std::make_unique<AddEffectCommand>(LocatorOf(l), NewEffect(l, s, e));
    }

    std::unique_ptr<SequenceCommand> SetKeys(Effect* e) {
        SettingsChange st, pal;
        int n = 1 + (int)(_rng() % 3);
        for (int i = 0; i < n; ++i) {
            if (Chance(70)) {
                std::string key = std::string("E_SLIDER_Fuzz_") + (char)('A' + _rng() % 4);
                st.keys[key] = Chance(15) ? std::nullopt : std::optional<std::string>(std::to_string(_rng() % 100));
            } else {
                std::string key = std::string("C_BUTTON_Palette") + (char)('1' + _rng() % 3);
                pal.keys[key] = Chance(15) ? std::nullopt : std::optional<std::string>("#" + std::to_string(100000 + _rng() % 800000));
            }
        }
        return std::make_unique<ModifyEffectCommand>(LocatorOf(e), std::nullopt, std::move(st), std::move(pal));
    }

    // The host switches a client between viewer and editor; anyone claims a
    // random row (the host sometimes for someone else) or releases one.
    void RoleStep(Node& n) {
        Node& h = _sim.host();
        std::string reason;
        std::string text;
        int op = (int)(_rng() % 100);
        if (&n == &h && op < 35) {
            auto clients = h.Host().Clients();
            if (!clients.empty()) {
                ParticipantId who = Pick(clients);
                h.Host().SetRole(who, h.session->RoleOf(who) == ParticipantRole::Viewer ? ParticipantRole::Editor : ParticipantRole::Viewer);
            }
            return;
        }
        auto rows = Rows(n.Seq(), Chance(20));
        if (rows.empty()) {
            return;
        }
        Element* row = Pick(rows);
        if (op < 75) {
            ParticipantId owner = n.session->Participant();
            if (&n == &h && Chance(50) && !h.session->Roster().empty()) {
                owner = Pick(h.session->Roster()).participant;
            }
            if (n.session->Claim(row->GetUid(), owner, reason, text)) {
                ++_claims;
            }
        } else {
            n.session->Claim(row->GetUid(), kNoParticipant, reason, text);
        }
    }

    void Edit(Node& n) {
        SequenceElements& seq = n.Seq();
        auto& session = *n.session;
        std::unique_ptr<SequenceCommand> cmd;
        int op = (int)(_rng() % 100);
        if (op < 18) {
            cmd = AddEffect(seq);
        } else if (op < 24) {
            if (Effect* e = RandomEffect(seq)) {
                cmd = std::make_unique<DeleteEffectCommand>(LocatorOf(e));
            }
        } else if (op < 32) {
            if (Effect* e = RandomEffect(seq)) {
                int shift = 50 * ((int)(_rng() % 21) - 10);
                int s = std::max(0, e->GetStartTimeMS() + shift);
                int len = Chance(30) ? 50 * (1 + (int)(_rng() % 10)) : e->GetEndTimeMS() - e->GetStartTimeMS();
                cmd = std::make_unique<MoveEffectCommand>(LocatorOf(e), s, s + len);
            }
        } else if (op < 52) {
            if (Effect* e = RandomEffect(seq)) {
                cmd = SetKeys(e);
            }
        } else if (op < 55) {
            if (Effect* e = RandomEffect(seq)) {
                SettingsChange st{ false, { { "X_Effect_Locked", e->IsLocked() ? std::nullopt : std::optional<std::string>("True") } } };
                cmd = std::make_unique<ModifyEffectCommand>(LocatorOf(e), std::nullopt, std::move(st), SettingsChange{});
            }
        } else if (op < 59) {
            if (Effect* e = RandomEffect(seq); e != nullptr && !IsTiming(e->GetParentEffectLayer())) {
                cmd = std::make_unique<ModifyEffectCommand>(LocatorOf(e), Pick(kModelEffects), SettingsChange::ReplaceWith(Chance(50) ? "" : "E_SLIDER_Fuzz_T=1"),
                                                            SettingsChange{});
            }
        } else if (op < 62) {
            if (EffectLayer* l = RandomLayer(seq)) {
                Element* el = l->GetParentElement();
                if (dynamic_cast<StrandElement*>(el) == nullptr || !Chance(50)) {
                    std::vector<EffectState> contents;
                    if (Chance(30) && !IsTiming(l)) {
                        auto [s, e] = RandomRange();
                        contents.push_back(NewEffect(l, s, e));
                        contents.back().uid = SeqIds::Next();
                    }
                    cmd = std::make_unique<AddLayerCommand>(el->GetUid(), kNoSeqObjectId, Chance(20) ? kNoSeqObjectId : l->GetUid(), -1,
                                                            Chance(30) ? "L" + std::to_string(_rng() % 10) : "", std::move(contents));
                }
            }
        } else if (op < 65) {
            if (EffectLayer* l = RandomLayer(seq); l != nullptr && l->GetParentElement()->GetEffectLayerCount() > 1) {
                cmd = std::make_unique<RemoveLayerCommand>(l->GetParentElement()->GetUid(), l->GetUid());
            }
        } else if (op < 67) {
            if (EffectLayer* l = RandomLayer(seq)) {
                cmd = std::make_unique<SetLayerNameCommand>(LocatorOf(l), "N" + std::to_string(_rng() % 20));
            }
        } else if (op < 69) {
            if (EffectLayer* l = RandomLayer(seq)) {
                std::vector<EffectState> effects;
                int t = 0;
                for (int i = 0; i < (int)(_rng() % 4); ++i) {
                    t += 50 * (1 + (int)(_rng() % 20));
                    int end = t + 50 * (1 + (int)(_rng() % 10));
                    effects.push_back(NewEffect(l, t, end));
                    t = end;
                }
                for (auto& s : effects) {
                    s.uid = SeqIds::Next();
                }
                cmd = std::make_unique<ReplaceLayerContentsCommand>(LocatorOf(l), std::move(effects));
            }
        } else if (op < 75) {
            // A batch: a few adds on one layer plus maybe a delete and a key change.
            auto batch = std::make_unique<BatchCommand>("Paste");
            if (EffectLayer* l = RandomLayer(seq)) {
                int t = (int)(_rng() % 200) * 50;
                for (int i = 0; i < 2 + (int)(_rng() % 3); ++i) {
                    int end = t + 50 * (1 + (int)(_rng() % 6));
                    if (l->GetRangeIsClearMS(t, end)) {
                        batch->Add(std::make_unique<AddEffectCommand>(LocatorOf(l), NewEffect(l, t, end)));
                    }
                    t = end + 50 * (int)(_rng() % 4);
                }
            }
            if (Chance(40)) {
                if (Effect* e = RandomEffect(seq)) {
                    batch->Add(SetKeys(e));
                }
            }
            if (Chance(30)) {
                if (Effect* e = RandomEffect(seq)) {
                    batch->Add(std::make_unique<DeleteEffectCommand>(LocatorOf(e)));
                }
            }
            if (!batch->IsEmpty()) {
                cmd = std::move(batch);
            }
        } else if (op < 77) {
            ElementSnapshot s;
            bool model = Chance(50);
            if (model) {
                s.name = Pick(kExtraModels);
            } else {
                s.timing = true;
                s.name = "T" + std::to_string(n.session->Participant()) + "_" + std::to_string(_rng() % 1000);
            }
            if (seq.GetElement(s.name) == nullptr) {
                auto rows = Rows(seq, false);
                SeqObjectId after = rows.empty() || Chance(20) ? kNoSeqObjectId : Pick(rows)->GetUid();
                cmd = std::make_unique<AddElementCommand>(std::move(s), after);
            }
        } else if (op < 79) {
            auto rows = Rows(seq, false);
            if (rows.size() > 2) {
                cmd = std::make_unique<DeleteElementCommand>(Pick(rows)->GetUid());
            }
        } else if (op < 81) {
            auto rows = Rows(seq, false);
            if (rows.size() > 1) {
                Element* el = Pick(rows);
                Element* after = Pick(rows);
                cmd = std::make_unique<MoveElementCommand>(el->GetUid(), after == el ? kNoSeqObjectId : after->GetUid());
            }
        } else if (op < 83) {
            std::vector<Element*> timing;
            for (Element* el : Rows(seq, false)) {
                if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                    timing.push_back(el);
                }
            }
            if (!timing.empty()) {
                std::string name = "R" + std::to_string(n.session->Participant()) + "_" + std::to_string(_rng() % 1000);
                if (Chance(50)) {
                    cmd = std::make_unique<RenameTimingTrackCommand>(Pick(timing)->GetUid(), name);
                } else {
                    cmd = std::make_unique<SetTimingFixedCommand>(Pick(timing)->GetUid(), Chance(50) ? 0 : 50 * (1 + (int)(_rng() % 4)));
                }
            }
        } else if (op < 84) {
            auto rows = Rows(seq, true);
            if (!rows.empty()) {
                Element* el = Pick(rows);
                cmd = std::make_unique<SetElementRenderDisabledCommand>(el->GetUid(), !el->IsRenderDisabledFlag());
            }
        } else if (op < 86) {
            cmd = std::make_unique<SetTagCommand>((int)(_rng() % 10), Chance(20) ? -1 : (int)(_rng() % 400) * 50);
        } else if (op < 89) {
            int pick = (int)(_rng() % 3);
            auto embedded = seq.GetSequenceMedia().GetEmbeddedEntries();
            if (pick == 0 || embedded.empty()) {
                std::string key = "Images/f" + std::to_string(_rng() % 6) + ".png";
                xlImage img(2, 2);
                for (int y = 0; y < 2; ++y) {
                    for (int x = 0; x < 2; ++x) {
                        img.SetRGB(x, y, (uint8_t)_rng(), (uint8_t)_rng(), (uint8_t)_rng());
                    }
                }
                if (auto st = MediaEntryState::EmbeddedImage(key, img)) {
                    cmd = std::make_unique<PutMediaCommand>(std::move(*st));
                    _mediaKeys.push_back(key);
                }
            } else {
                auto it = embedded.begin();
                std::advance(it, _rng() % embedded.size());
                if (pick == 1) {
                    cmd = std::make_unique<RemoveMediaCommand>(it->first);
                } else {
                    std::string to = "Images/r" + std::to_string(_rng() % 6) + ".png";
                    auto b = BuildRenameMedia(seq, it->first, to, Chance(30));
                    if (b != nullptr && !b->IsEmpty()) {
                        cmd = std::move(b);
                        _mediaKeys.push_back(to);
                    }
                }
            }
        } else if (op < 96) {
            session.Undo();
            return;
        } else {
            session.Redo();
            return;
        }
        if (cmd != nullptr) {
            session.Submit(std::move(cmd));
        }
    }

    FuzzConfig _cfg;
    std::mt19937 _rng;
    Sim _sim;
    int _clientCount = 0;
    std::vector<std::string> _mediaKeys;
    std::map<ParticipantId, uint64_t> _lastSeq;
    Rev _lastRev = 0;
    bool _orderOk = true;
    std::string _rolesWhy;
    size_t _claims = 0;
};

uint32_t EnvInt(const char* name, uint32_t def) {
    const char* v = std::getenv(name);
    if (v == nullptr || *v == 0) {
        return def;
    }
    return (uint32_t)std::strtoul(v, nullptr, 10);
}

FuzzResult RunSeed(Show& show, FuzzConfig cfg) {
    Fuzzer f(show, cfg);
    return f.Run();
}

void RunSeeds(uint32_t first, uint32_t count, bool roles = false) {
    Show show;
    uint32_t only = EnvInt("XL_COLLAB_FUZZ_SEED", 0);
    size_t commits = 0, rejects = 0, undone = 0, roleRejects = 0, claims = 0;
    for (uint32_t i = 0; i < count; ++i) {
        FuzzConfig cfg;
        cfg.seed = only != 0 ? only : first + i;
        cfg.roles = roles;
        if (std::getenv("XL_COLLAB_FUZZ_TRACE") != nullptr) {
            std::fprintf(stderr, "collab fuzz seed %u\n", cfg.seed);
        }
        FuzzResult r = RunSeed(show, cfg);
        INFO("collab fuzz seed " << cfg.seed << " failed; reproduce with XL_COLLAB_FUZZ_SEED=" << cfg.seed
                                 << " xLights-unit-tests \"[collab_fuzz]\"\n" << r.why);
        REQUIRE(r.ok);
        commits += r.commits;
        rejects += r.rejects;
        undone += r.undone;
        roleRejects += r.roleRejects;
        claims += r.claims;
        if (only != 0) {
            break;
        }
    }
    if (roles) {
        CHECK(roleRejects > 0);
        CHECK(claims > 0);
    }
    // The fuzz must actually exercise concurrency.
    CHECK(commits > 100);
    CHECK(rejects > 0);
    CHECK(undone > 0);
}

// For the controls: true when any of the seeds fails (in a way `counts`
// accepts, when given).
bool AnySeedFails(const SessionFaults& faults, bool reorder, uint32_t seeds, std::string* why,
                  const std::function<bool(const std::string&)>& counts = nullptr, bool roles = false) {
    Show show;
    for (uint32_t s = 1; s <= seeds; ++s) {
        FuzzConfig cfg;
        cfg.seed = 1000 + s;
        cfg.faults = faults;
        cfg.forceReorder = reorder;
        cfg.roles = roles;
        cfg.steps = 120;
        FuzzResult r = RunSeed(show, cfg);
        if (!r.ok && (!counts || counts(r.why))) {
            *why = r.why;
            return true;
        }
    }
    return false;
}

} // namespace

TEST_CASE("Collab convergence fuzz", "[collab][collab_fuzz]") {
    RunSeeds(1, EnvInt("XL_COLLAB_FUZZ_SEEDS", 12));
}

TEST_CASE("Collab convergence fuzz with viewers and row assignments", "[collab][collab_fuzz]") {
    RunSeeds(5000, EnvInt("XL_COLLAB_FUZZ_SEEDS", 10), true);
}

TEST_CASE("Collab fuzz catches a host that ignores roles and assignments", "[collab][collab_fuzz_control]") {
    SessionFaults f;
    f.hostIgnoresRoles = true;
    std::string why;
    CHECK(AnySeedFails(f, false, 6, &why, nullptr, true));
    INFO(why);
    CHECK(why.find("the host committed rev") != std::string::npos);
}

TEST_CASE("Collab convergence fuzz soak", "[.collab_fuzz_soak]") {
    RunSeeds(EnvInt("XL_COLLAB_FUZZ_FIRST", 100000), EnvInt("XL_COLLAB_FUZZ_SEEDS", 10000));
}

TEST_CASE("Collab fuzz catches a host that ignores each author's order", "[collab][collab_fuzz_control]") {
    SessionFaults f;
    f.hostIgnoresClientOrder = true;
    std::string why;
    CHECK(AnySeedFails(f, true, 6, &why));
    INFO(why);
    CHECK(why.find("out of order") != std::string::npos);
}

TEST_CASE("Collab fuzz catches clients applying commits out of order", "[collab][collab_fuzz_control]") {
    SessionFaults f;
    f.clientAppliesOutOfOrder = true;
    std::string why;
    CHECK(AnySeedFails(f, true, 6, &why));
}

TEST_CASE("Collab fuzz catches clients that don't rebase their optimistic edits", "[collab][collab_fuzz_control]") {
    SessionFaults f;
    f.clientSkipsRebase = true;
    std::string why;
    CHECK(AnySeedFails(f, false, 6, &why));
    INFO(why);
    // Drift detection notices and re-syncs, so the documents still agree.
    CHECK(why.find("re-sync") != std::string::npos);
    CHECK(why.find("differs from the host:") == std::string::npos);
    CHECK(why.find("saves a different") == std::string::npos);
    // Without drift detection they end up apart.
    f.clientIgnoresDrift = true;
    why.clear();
    auto apart = [](const std::string& w) { return w.find("differs from the host:") != std::string::npos || w.find("saves a different") != std::string::npos; };
    CHECK(AnySeedFails(f, false, 12, &why, apart));
    INFO(why);
    CHECK((why.find("differs from the host:") != std::string::npos || why.find("saves a different") != std::string::npos));
}

TEST_CASE("Collab fuzz catches colliding id allocation", "[collab][collab_fuzz_control]") {
    SessionFaults f;
    f.clientSharesHostIdPrefix = true;
    std::string why;
    CHECK(AnySeedFails(f, false, 6, &why));
    INFO(why);
    CHECK(why.find("already in use") != std::string::npos);
}

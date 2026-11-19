/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CollabCommandInfo.h"

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMediaCommands.h"
#include "render/SequenceMutationGuard.h"

#include <unordered_map>

namespace Collab {

namespace {

std::string E(SeqObjectId u, const char* aspect) {
    return "e" + std::to_string(u) + "." + aspect;
}
std::string L(SeqObjectId u, const char* aspect) {
    return "l" + std::to_string(u) + "." + aspect;
}
std::string X(SeqObjectId u, const char* aspect) {
    return "x" + std::to_string(u) + "." + aspect;
}

void EffectTouch(std::set<std::string>& out, const SequenceElements& seq, const EffectLocator& loc) {
    out.insert(E(loc.effect, "*"));
    out.insert(L(loc.layer, "*"));
    out.insert(X(TopLevelOf(seq, loc.element), "*"));
}

void Exists(std::set<std::string>& out, SeqObjectId effect) {
    out.insert(E(effect, "x"));
    out.insert(E(effect, "*"));
}

void Layer(std::set<std::string>& out, const SequenceElements& seq, SeqObjectId element, SeqObjectId layer) {
    out.insert(L(layer, "*"));
    out.insert(X(TopLevelOf(seq, element), "*"));
}

void LayerContents(std::set<std::string>& out, const std::vector<EffectState>& effects) {
    for (const auto& s : effects) {
        Exists(out, s.uid);
    }
}

void Snapshot(std::set<std::string>& out, const ElementSnapshot& s) {
    out.insert(X(s.uid, "x"));
    out.insert(X(s.uid, "*"));
    auto layers = [&](const std::vector<LayerSnapshot>& ls) {
        for (const auto& l : ls) {
            out.insert(L(l.uid, "x"));
            out.insert(L(l.uid, "*"));
            LayerContents(out, l.effects);
        }
    };
    layers(s.layers);
    for (const auto& sub : s.subElements) {
        out.insert(X(sub.uid, "*"));
        layers(sub.layers);
        for (SeqObjectId nl : sub.nodeLayerUids) {
            out.insert(L(nl, "*"));
        }
        for (const auto& [n, effects] : sub.nodeEffects) {
            LayerContents(out, effects);
        }
    }
}

void KeyItems(std::set<std::string>& out, SeqObjectId u, const char* map, const SettingsChange& c) {
    std::string prefix = "e" + std::to_string(u) + "." + map;
    if (c.replaceAll) {
        out.insert(prefix + "*");
    }
    for (const auto& [k, v] : c.keys) {
        out.insert(prefix + "." + k);
    }
}

void Touched(std::set<std::string>& out, const SequenceElements& seq, const SequenceCommand& a, const SequenceCommand* inv) {
    switch (a.Type()) {
    case CommandType::AddEffect: {
        const auto& c = static_cast<const AddEffectCommand&>(a);
        EffectTouch(out, seq, c.Target());
        Exists(out, c.GetState().uid);
        break;
    }
    case CommandType::DeleteEffect: {
        const auto& c = static_cast<const DeleteEffectCommand&>(a);
        EffectTouch(out, seq, c.Target());
        Exists(out, c.Target().effect);
        break;
    }
    case CommandType::MoveEffect: {
        const auto& c = static_cast<const MoveEffectCommand&>(a);
        EffectTouch(out, seq, c.Target());
        out.insert(E(c.Target().effect, "t"));
        break;
    }
    case CommandType::ModifyEffect: {
        const auto& c = static_cast<const ModifyEffectCommand&>(a);
        SeqObjectId u = c.Target().effect;
        EffectTouch(out, seq, c.Target());
        KeyItems(out, u, "s", c.GetSettings());
        KeyItems(out, u, "p", c.GetPalette());
        if (inv != nullptr && inv->Type() == CommandType::ModifyEffect) {
            const auto& i = static_cast<const ModifyEffectCommand&>(*inv);
            KeyItems(out, u, "s", i.GetSettings());
            KeyItems(out, u, "p", i.GetPalette());
            // Only an actual type change counts (the inverse names the old type).
            if (c.GetEffectName().has_value() && i.GetEffectName().has_value()) {
                out.insert(TypeItem(u));
            }
        }
        break;
    }
    case CommandType::AddLayer: {
        const auto& c = static_cast<const AddLayerCommand&>(a);
        Layer(out, seq, c.GetElement(), c.GetLayer());
        out.insert(L(c.GetLayer(), "x"));
        LayerContents(out, c.GetEffects());
        break;
    }
    case CommandType::RemoveLayer: {
        const auto& c = static_cast<const RemoveLayerCommand&>(a);
        Layer(out, seq, c.GetElement(), c.GetLayer());
        out.insert(L(c.GetLayer(), "x"));
        if (inv != nullptr && inv->Type() == CommandType::AddLayer) {
            LayerContents(out, static_cast<const AddLayerCommand&>(*inv).GetEffects());
        }
        break;
    }
    case CommandType::SetLayerName: {
        const auto& c = static_cast<const SetLayerNameCommand&>(a);
        Layer(out, seq, c.GetLayer().element, c.GetLayer().layer);
        out.insert(L(c.GetLayer().layer, "n"));
        break;
    }
    case CommandType::ReplaceLayerContents: {
        const auto& c = static_cast<const ReplaceLayerContentsCommand&>(a);
        Layer(out, seq, c.GetLayer().element, c.GetLayer().layer);
        out.insert(L(c.GetLayer().layer, "c"));
        LayerContents(out, c.GetEffects());
        if (inv != nullptr && inv->Type() == CommandType::ReplaceLayerContents) {
            LayerContents(out, static_cast<const ReplaceLayerContentsCommand&>(*inv).GetEffects());
        }
        break;
    }
    case CommandType::Batch: {
        const auto& members = static_cast<const BatchCommand&>(a).GetCommands();
        const BatchCommand* ib = (inv != nullptr && inv->Type() == CommandType::Batch) ? static_cast<const BatchCommand*>(inv) : nullptr;
        bool paired = ib != nullptr && ib->GetCommands().size() == members.size();
        for (size_t i = 0; i < members.size(); ++i) {
            const SequenceCommand* mi = paired ? ib->GetCommands()[members.size() - 1 - i].get() : nullptr;
            Touched(out, seq, *members[i], mi);
        }
        break;
    }
    case CommandType::AddElement:
        Snapshot(out, static_cast<const AddElementCommand&>(a).GetState());
        break;
    case CommandType::DeleteElement: {
        SeqObjectId u = static_cast<const DeleteElementCommand&>(a).GetElement();
        out.insert(X(u, "x"));
        out.insert(X(u, "*"));
        if (inv != nullptr && inv->Type() == CommandType::AddElement) {
            Snapshot(out, static_cast<const AddElementCommand&>(*inv).GetState());
        }
        break;
    }
    case CommandType::MoveElement: {
        SeqObjectId u = static_cast<const MoveElementCommand&>(a).GetElement();
        out.insert(X(u, "o"));
        out.insert(X(u, "*"));
        break;
    }
    case CommandType::RenameTimingTrack: {
        SeqObjectId u = static_cast<const RenameTimingTrackCommand&>(a).GetElement();
        out.insert(X(u, "n"));
        out.insert(X(u, "*"));
        break;
    }
    case CommandType::RenameModelElement: {
        SeqObjectId u = static_cast<const RenameModelElementCommand&>(a).GetElement();
        out.insert(X(u, "n"));
        out.insert(X(u, "*"));
        break;
    }
    case CommandType::SetTimingFixed: {
        SeqObjectId u = static_cast<const SetTimingFixedCommand&>(a).GetElement();
        out.insert(X(u, "f"));
        out.insert(X(u, "*"));
        break;
    }
    case CommandType::SetElementRenderDisabled: {
        SeqObjectId u = static_cast<const SetElementRenderDisabledCommand&>(a).GetElement();
        out.insert(X(TopLevelOf(seq, u), "*"));
        out.insert(X(u, "r"));
        out.insert(X(u, "*"));
        break;
    }
    case CommandType::SetTag:
        out.insert("g.tag" + std::to_string(static_cast<const SetTagCommand&>(a).GetTag()));
        break;
    case CommandType::SetSequenceFace:
        out.insert("g.face." + static_cast<const SetSequenceFaceCommand&>(a).GetName());
        break;
    case CommandType::SetSongStructure:
        out.insert("g.song");
        break;
    case CommandType::SetEffectSymbol:
        out.insert("g.sym." + static_cast<const SetEffectSymbolCommand&>(a).GetId());
        break;
    case CommandType::SetEffectSymbolLink: {
        const auto& c = static_cast<const SetEffectSymbolLinkCommand&>(a);
        EffectTouch(out, seq, c.GetTarget());
        out.insert(E(c.GetTarget().effect, "y"));
        break;
    }
    case CommandType::PutMedia:
        out.insert("m." + static_cast<const PutMediaCommand&>(a).GetState().key);
        break;
    case CommandType::RemoveMedia:
        out.insert("m." + static_cast<const RemoveMediaCommand&>(a).GetKey());
        break;
    case CommandType::RenameMedia: {
        const auto& c = static_cast<const RenameMediaCommand&>(a);
        out.insert("m." + c.GetFrom());
        out.insert("m." + c.GetTo());
        break;
    }
    }
}

void Conflicts(std::set<std::string>& out, const SequenceElements& seq, const SequenceCommand& a) {
    auto keyChecks = [&](SeqObjectId u, const char* map, const SettingsChange& c) {
        std::string prefix = "e" + std::to_string(u) + "." + map;
        out.insert(prefix + "*");
        for (const auto& [k, v] : c.keys) {
            out.insert(prefix + "." + k);
        }
    };
    switch (a.Type()) {
    case CommandType::AddEffect: {
        const auto& c = static_cast<const AddEffectCommand&>(a);
        out.insert(E(c.GetState().uid, "*"));
        out.insert(L(c.GetLayer().layer, "x"));
        break;
    }
    case CommandType::DeleteEffect:
        out.insert(E(static_cast<const DeleteEffectCommand&>(a).Target().effect, "*"));
        break;
    case CommandType::MoveEffect: {
        SeqObjectId u = static_cast<const MoveEffectCommand&>(a).Target().effect;
        out.insert(E(u, "t"));
        out.insert(E(u, "x"));
        break;
    }
    case CommandType::ModifyEffect: {
        const auto& c = static_cast<const ModifyEffectCommand&>(a);
        SeqObjectId u = c.Target().effect;
        out.insert(E(u, "x"));
        out.insert(TypeItem(u));
        if (!c.GetSettings().IsEmpty()) {
            keyChecks(u, "s", c.GetSettings());
        }
        if (!c.GetPalette().IsEmpty()) {
            keyChecks(u, "p", c.GetPalette());
        }
        break;
    }
    case CommandType::AddLayer: {
        const auto& c = static_cast<const AddLayerCommand&>(a);
        out.insert(L(c.GetLayer(), "*"));
        out.insert(X(TopLevelOf(seq, c.GetElement()), "x"));
        break;
    }
    case CommandType::RemoveLayer:
        out.insert(L(static_cast<const RemoveLayerCommand&>(a).GetLayer(), "*"));
        break;
    case CommandType::SetLayerName: {
        SeqObjectId l = static_cast<const SetLayerNameCommand&>(a).GetLayer().layer;
        out.insert(L(l, "n"));
        out.insert(L(l, "x"));
        break;
    }
    case CommandType::ReplaceLayerContents:
        out.insert(L(static_cast<const ReplaceLayerContentsCommand&>(a).GetLayer().layer, "*"));
        break;
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(a).GetCommands()) {
            Conflicts(out, seq, *m);
        }
        break;
    case CommandType::AddElement:
        out.insert(X(static_cast<const AddElementCommand&>(a).GetState().uid, "*"));
        break;
    case CommandType::DeleteElement:
        out.insert(X(static_cast<const DeleteElementCommand&>(a).GetElement(), "*"));
        break;
    case CommandType::MoveElement: {
        SeqObjectId u = static_cast<const MoveElementCommand&>(a).GetElement();
        out.insert(X(u, "o"));
        out.insert(X(u, "x"));
        break;
    }
    case CommandType::RenameTimingTrack: {
        SeqObjectId u = static_cast<const RenameTimingTrackCommand&>(a).GetElement();
        out.insert(X(u, "n"));
        out.insert(X(u, "x"));
        break;
    }
    case CommandType::RenameModelElement: {
        SeqObjectId u = static_cast<const RenameModelElementCommand&>(a).GetElement();
        out.insert(X(u, "n"));
        out.insert(X(u, "x"));
        break;
    }
    case CommandType::SetTimingFixed: {
        SeqObjectId u = static_cast<const SetTimingFixedCommand&>(a).GetElement();
        out.insert(X(u, "f"));
        out.insert(X(u, "x"));
        break;
    }
    case CommandType::SetElementRenderDisabled: {
        SeqObjectId u = static_cast<const SetElementRenderDisabledCommand&>(a).GetElement();
        out.insert(X(u, "r"));
        out.insert(X(u, "x"));
        break;
    }
    case CommandType::SetEffectSymbolLink: {
        SeqObjectId u = static_cast<const SetEffectSymbolLinkCommand&>(a).GetTarget().effect;
        out.insert(E(u, "y"));
        out.insert(E(u, "x"));
        break;
    }
    case CommandType::SetTag:
    case CommandType::SetSequenceFace:
    case CommandType::SetSongStructure:
    case CommandType::SetEffectSymbol:
    case CommandType::PutMedia:
    case CommandType::RemoveMedia:
    case CommandType::RenameMedia:
        // Global items are touched and checked under the same name.
        Touched(out, seq, a, nullptr);
        break;
    }
}

void CollectCreated(const SequenceCommand& cmd, std::set<SeqObjectId>& out) {
    switch (cmd.Type()) {
    case CommandType::AddEffect:
        out.insert(static_cast<const AddEffectCommand&>(cmd).GetState().uid);
        break;
    case CommandType::AddLayer: {
        const auto& c = static_cast<const AddLayerCommand&>(cmd);
        out.insert(c.GetLayer());
        for (const auto& e : c.GetEffects()) {
            out.insert(e.uid);
        }
        break;
    }
    case CommandType::ReplaceLayerContents:
        for (const auto& e : static_cast<const ReplaceLayerContentsCommand&>(cmd).GetEffects()) {
            out.insert(e.uid);
        }
        break;
    case CommandType::AddElement: {
        const auto& s = static_cast<const AddElementCommand&>(cmd).GetState();
        out.insert(s.uid);
        for (const auto& l : s.layers) {
            out.insert(l.uid);
            for (const auto& e : l.effects) {
                out.insert(e.uid);
            }
        }
        for (const auto& sub : s.subElements) {
            out.insert(sub.uid);
            out.insert(sub.nodeLayerUids.begin(), sub.nodeLayerUids.end());
            for (const auto& l : sub.layers) {
                out.insert(l.uid);
                for (const auto& e : l.effects) {
                    out.insert(e.uid);
                }
            }
            for (const auto& [n, effects] : sub.nodeEffects) {
                for (const auto& e : effects) {
                    out.insert(e.uid);
                }
            }
        }
        break;
    }
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            CollectCreated(*m, out);
        }
        break;
    default:
        break;
    }
}

// True when `cmd` deletes something that is already gone (and that the
// command itself doesn't create).
bool RedundantDelete(const SequenceElements& seq, const SequenceCommand& cmd, const std::set<SeqObjectId>& created) {
    switch (cmd.Type()) {
    case CommandType::DeleteEffect: {
        const auto& t = static_cast<const DeleteEffectCommand&>(cmd).Target();
        return created.count(t.effect) == 0 && created.count(t.layer) == 0 && FindEffect(seq, t) == nullptr;
    }
    case CommandType::RemoveLayer: {
        const auto& c = static_cast<const RemoveLayerCommand&>(cmd);
        return created.count(c.GetLayer()) == 0 && FindLayer(seq, { c.GetElement(), c.GetLayer() }) == nullptr;
    }
    case CommandType::DeleteElement: {
        SeqObjectId u = static_cast<const DeleteElementCommand&>(cmd).GetElement();
        return created.count(u) == 0 && seq.GetElementFromUid(u) == nullptr;
    }
    default:
        return false;
    }
}

// nullptr when nothing changed; otherwise the pruned copy.
std::unique_ptr<SequenceCommand> Prune(const SequenceElements& seq, const SequenceCommand& cmd, const std::set<SeqObjectId>& created) {
    if (RedundantDelete(seq, cmd, created)) {
        return std::make_unique<BatchCommand>("");
    }
    if (cmd.Type() != CommandType::Batch) {
        return nullptr;
    }
    const auto& b = static_cast<const BatchCommand&>(cmd);
    bool changed = false;
    std::vector<std::unique_ptr<SequenceCommand>> kept;
    for (const auto& m : b.GetCommands()) {
        auto p = Prune(seq, *m, created);
        if (p == nullptr) {
            kept.push_back(m->Clone());
            continue;
        }
        changed = true;
        if (!IsNoOp(*p)) {
            kept.push_back(std::move(p));
        }
    }
    if (!changed) {
        return nullptr;
    }
    return std::make_unique<BatchCommand>(b.GetLabel(), std::move(kept));
}

std::optional<std::string> ValueOf(const SettingsMap& m, const std::string& key) {
    if (!m.Contains(key)) {
        return std::nullopt;
    }
    return m.Get(key, std::string());
}

// The key changes that turn `current` into the complete map `wanted`.
KeyChanges Diff(const SettingsMap& current, const KeyChanges& wanted) {
    KeyChanges out;
    for (const auto& [k, v] : wanted) {
        if (!v.has_value()) {
            if (current.Contains(k)) {
                out[k] = std::nullopt;
            }
        } else if (ValueOf(current, k) != v) {
            out[k] = v;
        }
    }
    for (const auto& [k, v] : current) {
        if (wanted.find(k) == wanted.end()) {
            out[k] = std::nullopt;
        }
    }
    return out;
}

} // namespace

SeqObjectId TopLevelOf(const SequenceElements& seq, SeqObjectId elementUid) {
    Element* el = seq.GetElementFromUid(elementUid);
    if (auto* sub = dynamic_cast<SubModelElement*>(el)) {
        if (sub->GetModelElement() != nullptr) {
            return sub->GetModelElement()->GetUid();
        }
    }
    return elementUid;
}

std::string TypeItem(SeqObjectId effect) {
    return E(effect, "n");
}

std::set<std::string> TouchedItems(const SequenceElements& seq, const SequenceCommand& applied, const SequenceCommand& inverse) {
    std::set<std::string> out;
    Touched(out, seq, applied, &inverse);
    return out;
}

std::set<std::string> ConflictItems(const SequenceElements& seq, const SequenceCommand& cmd) {
    std::set<std::string> out;
    Conflicts(out, seq, cmd);
    return out;
}

bool IsOptimistic(const SequenceCommand& cmd) {
    if (cmd.Type() != CommandType::ModifyEffect) {
        return false;
    }
    const auto& c = static_cast<const ModifyEffectCommand&>(cmd);
    return !c.GetEffectName().has_value() && !c.GetSettings().replaceAll && !c.GetPalette().replaceAll &&
           !(c.GetSettings().IsEmpty() && c.GetPalette().IsEmpty());
}

std::unique_ptr<SequenceCommand> AsKeyChanges(const SequenceElements& seq, const SequenceCommand& cmd) {
    if (cmd.Type() != CommandType::ModifyEffect) {
        return nullptr;
    }
    const auto& c = static_cast<const ModifyEffectCommand&>(cmd);
    if (c.GetEffectName().has_value() || (!c.GetSettings().replaceAll && !c.GetPalette().replaceAll)) {
        return nullptr;
    }
    ::Effect* e = FindEffect(seq, c.Target());
    if (e == nullptr) {
        return nullptr;
    }
    SettingsChange s{ false, c.GetSettings().replaceAll ? Diff(e->GetSettings(), c.GetSettings().keys) : c.GetSettings().keys };
    SettingsChange p{ false, c.GetPalette().replaceAll ? Diff(e->GetPaletteMap(), c.GetPalette().keys) : c.GetPalette().keys };
    if (s.IsEmpty() && p.IsEmpty()) {
        return nullptr;
    }
    return std::make_unique<ModifyEffectCommand>(c.Target(), std::nullopt, std::move(s), std::move(p));
}

std::vector<SeqObjectId> KeyWriteTargets(const SequenceCommand& cmd) {
    std::vector<SeqObjectId> out;
    if (cmd.Type() == CommandType::ModifyEffect) {
        const auto& c = static_cast<const ModifyEffectCommand&>(cmd);
        if (!c.GetEffectName().has_value() && !(c.GetSettings().IsEmpty() && c.GetPalette().IsEmpty())) {
            out.push_back(c.Target().effect);
        }
    } else if (cmd.Type() == CommandType::Batch) {
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            auto sub = KeyWriteTargets(*m);
            out.insert(out.end(), sub.begin(), sub.end());
        }
    }
    return out;
}

std::unique_ptr<SequenceCommand> WithoutRedundantDeletes(const SequenceElements& seq, const SequenceCommand& cmd) {
    std::set<SeqObjectId> created;
    CollectCreated(cmd, created);
    return Prune(seq, cmd, created);
}

bool IsNoOp(const SequenceCommand& cmd) {
    if (cmd.Type() != CommandType::Batch) {
        return false;
    }
    for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
        if (!IsNoOp(*m)) {
            return false;
        }
    }
    return true;
}

namespace {

void UidOps(const SequenceCommand& cmd, const SequenceCommand* inverse, std::vector<std::pair<bool, SeqObjectId>>& ops) {
    if (cmd.Type() == CommandType::Batch) {
        const auto& members = static_cast<const BatchCommand&>(cmd).GetCommands();
        const BatchCommand* ib = (inverse != nullptr && inverse->Type() == CommandType::Batch) ? static_cast<const BatchCommand*>(inverse) : nullptr;
        bool paired = ib != nullptr && ib->GetCommands().size() == members.size();
        for (size_t i = 0; i < members.size(); ++i) {
            UidOps(*members[i], paired ? ib->GetCommands()[members.size() - 1 - i].get() : nullptr, ops);
        }
        return;
    }
    std::set<SeqObjectId> removed, added;
    if (inverse != nullptr) {
        CollectCreated(*inverse, removed);
    }
    CollectCreated(cmd, added);
    for (SeqObjectId u : removed) {
        ops.push_back({ false, u });
    }
    for (SeqObjectId u : added) {
        ops.push_back({ true, u });
    }
}

void AddRow(std::unordered_set<SeqObjectId>& ids, const Element* el) {
    ids.insert(el->GetUid());
    for (const EffectLayer* l : el->GetEffectLayers()) {
        ids.insert(l->GetUid());
        for (const ::Effect* e : l->GetEffects()) {
            ids.insert(e->GetUid());
        }
    }
    if (auto* st = dynamic_cast<const StrandElement*>(el)) {
        for (int n = 0; n < st->GetNodeLayerCount(); ++n) {
            const EffectLayer* l = st->GetNodeLayer(n);
            ids.insert(l->GetUid());
            for (const ::Effect* e : l->GetEffects()) {
                ids.insert(e->GetUid());
            }
        }
    }
}

} // namespace

void UidRegistry::Reset(const SequenceElements& seq) {
    _ids = AllUids(seq);
}

std::unordered_set<SeqObjectId> UidRegistry::AllUids(const SequenceElements& seq) {
    std::unordered_set<SeqObjectId> ids;
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el == nullptr) {
            continue;
        }
        AddRow(ids, el);
        if (auto* me = dynamic_cast<const ModelElement*>(el)) {
            for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
                if (const SubModelElement* sme = me->GetSubModel(x)) {
                    AddRow(ids, sme);
                }
            }
        }
    }
    return ids;
}

bool UidRegistry::Commit(const SequenceCommand& applied, const SequenceCommand& inverse, SeqObjectId* duplicate) {
    std::vector<std::pair<bool, SeqObjectId>> ops;
    UidOps(applied, &inverse, ops);
    std::unordered_map<SeqObjectId, bool> delta;
    auto present = [&](SeqObjectId u) {
        auto it = delta.find(u);
        return it != delta.end() ? it->second : _ids.count(u) != 0;
    };
    for (const auto& [add, u] : ops) {
        if (add && present(u)) {
            if (duplicate != nullptr) {
                *duplicate = u;
            }
            return false;
        }
        delta[u] = add;
    }
    for (const auto& [u, add] : delta) {
        if (add) {
            _ids.insert(u);
        } else {
            _ids.erase(u);
        }
    }
    return true;
}

namespace {

void NamedLayers(const SequenceCommand& cmd, std::vector<LayerLocator>& out) {
    switch (cmd.Type()) {
    case CommandType::AddEffect: out.push_back(static_cast<const AddEffectCommand&>(cmd).GetLayer()); break;
    case CommandType::DeleteEffect: out.push_back(static_cast<const DeleteEffectCommand&>(cmd).Target().Layer()); break;
    case CommandType::MoveEffect: out.push_back(static_cast<const MoveEffectCommand&>(cmd).Target().Layer()); break;
    case CommandType::ModifyEffect: out.push_back(static_cast<const ModifyEffectCommand&>(cmd).Target().Layer()); break;
    case CommandType::ReplaceLayerContents: out.push_back(static_cast<const ReplaceLayerContentsCommand&>(cmd).GetLayer()); break;
    case CommandType::SetEffectSymbolLink: out.push_back(static_cast<const SetEffectSymbolLinkCommand&>(cmd).GetTarget().Layer()); break;
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            NamedLayers(*m, out);
        }
        break;
    default:
        break;
    }
}

} // namespace

LayerOrderSnapshot::LayerOrderSnapshot(const SequenceElements& seq, const SequenceCommand& cmd) {
    std::vector<LayerLocator> named;
    NamedLayers(cmd, named);
    std::set<SeqObjectId> seen;
    for (const auto& loc : named) {
        EffectLayer* l = FindLayer(seq, loc);
        if (l == nullptr || !seen.insert(l->GetUid()).second) {
            continue;
        }
        Saved s;
        s.layer = loc;
        for (const ::Effect* e : l->GetEffects()) {
            s.effects.push_back({ e->GetUid(), e->GetID() });
        }
        _layers.push_back(std::move(s));
    }
}

void LayerOrderSnapshot::Restore(const SequenceElements& seq) const {
    for (const auto& s : _layers) {
        EffectLayer* l = FindLayer(seq, s.layer);
        if (l == nullptr || (size_t)l->GetEffectCount() != s.effects.size()) {
            continue;
        }
        std::vector<::Effect*> order;
        for (const auto& [uid, pos] : s.effects) {
            ::Effect* e = l->GetEffectFromUid(uid);
            if (e == nullptr) {
                break;
            }
            order.push_back(e);
        }
        if (order.size() != s.effects.size()) {
            continue;
        }
        SeqMutationGuard::Scope scope;
        if (l->ReorderEffects(order)) {
            for (size_t i = 0; i < order.size(); ++i) {
                order[i]->SetID(s.effects[i].second);
            }
        }
    }
}

bool HasNoLocalState(const nlohmann::json& j) {
    if (j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            const std::string& k = it.key();
            if (k == "selected" || k == "collapsed" || k == "visible" || k == "views" || k == "tagged") {
                return false;
            }
            if (!HasNoLocalState(it.value())) {
                return false;
            }
        }
    } else if (j.is_array()) {
        for (const auto& v : j) {
            if (!HasNoLocalState(v)) {
                return false;
            }
        }
    }
    return true;
}

} // namespace Collab

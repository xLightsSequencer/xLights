/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceEditCapture.h"

#include <algorithm>
#include <functional>
#include <set>

#include "CommandExecutor.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceCommands.h"
#include "SequenceElementCommands.h"
#include "SequenceElements.h"
#include "SequenceFaces.h"
#include "SequenceMedia.h"
#include "SequenceMediaCommands.h"

#include <log.h>

namespace {

using Snap = SequenceEditCapture;
using Cmds = std::vector<std::unique_ptr<SequenceCommand>>;

void ForEachElement(const SequenceElements& seq, const std::function<void(Element*)>& fn) {
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el == nullptr) {
            continue;
        }
        fn(el);
        if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            const ModelElement* mel = dynamic_cast<const ModelElement*>(el);
            for (int x = 0; mel != nullptr && x < mel->GetSubModelAndStrandCount(); ++x) {
                if (SubModelElement* sme = mel->GetSubModel(x)) {
                    fn(sme);
                }
            }
        }
    }
}

Snap::LayerSnap TakeLayer(const EffectLayer* layer) {
    Snap::LayerSnap ls;
    ls.uid = layer->GetUid();
    ls.name = layer->GetLayerName();
    ls.effects.reserve(layer->GetEffects().size());
    for (const Effect* e : layer->GetEffects()) {
        Snap::EffectSnap es;
        es.uid = e->GetUid();
        es.name = e->GetEffectName();
        es.startTimeMS = e->GetStartTimeMS();
        es.endTimeMS = e->GetEndTimeMS();
        es.isProtected = e->GetProtected();
        es.settings = e->GetSettings();
        es.palette = e->GetPaletteMap();
        es.linkedSymbol = e->GetLinkedSymbolId();
        ls.effects.push_back(std::move(es));
    }
    return ls;
}

Snap::ElementSnap TakeElement(const Element* el) {
    Snap::ElementSnap s;
    s.uid = el->GetUid();
    s.renderDisabled = el->IsRenderDisabledFlag();
    if (const TimingElement* te = dynamic_cast<const TimingElement*>(el)) {
        s.timing = true;
        s.fixedTimingMS = te->GetFixedTiming();
    }
    for (const EffectLayer* l : el->GetEffectLayers()) {
        s.layers.push_back(TakeLayer(l));
    }
    if (el->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        const StrandElement* st = dynamic_cast<const StrandElement*>(el);
        for (int n = 0; st != nullptr && n < st->GetNodeLayerCount(); ++n) {
            if (const NodeLayer* nl = st->GetNodeLayer(n)) {
                s.nodeLayers.push_back(TakeLayer(nl));
            }
        }
    }
    return s;
}

// Same form as Effect::GetSettingsAsString().
EffectState StateOf(const Snap::EffectSnap& e) {
    EffectState s;
    s.uid = e.uid;
    s.name = e.name;
    s.settings = e.settings.AsString();
    if (!e.linkedSymbol.empty()) {
        if (!s.settings.empty()) {
            s.settings += ",";
        }
        s.settings += "X_LinkedSymbolId=" + e.linkedSymbol;
    }
    s.palette = e.palette.AsString();
    s.startTimeMS = e.startTimeMS;
    s.endTimeMS = e.endTimeMS;
    s.isProtected = e.isProtected;
    return s;
}

std::vector<EffectState> StatesOf(const std::vector<Snap::EffectSnap>& v) {
    std::vector<EffectState> out;
    out.reserve(v.size());
    for (const auto& e : v) {
        out.push_back(StateOf(e));
    }
    return out;
}

KeyChanges DiffMaps(const SettingsMap& from, const SettingsMap& to) {
    KeyChanges out;
    for (const auto& [k, v] : from) {
        if (!to.Contains(k)) {
            out[k] = std::nullopt;
        }
    }
    for (const auto& [k, v] : to) {
        const SettingValue* cur = from.FindValue(k);
        if (cur == nullptr || *cur != v) {
            out[k] = std::string(v);
        }
    }
    return out;
}

struct Out {
    Cmds& applied;
    Cmds& inverses;
    void Add(std::unique_ptr<SequenceCommand> a, std::unique_ptr<SequenceCommand> i) {
        applied.push_back(std::move(a));
        inverses.push_back(std::move(i));
    }
};

bool SameMap(const SettingsMap& a, const SettingsMap& b) {
    if (a.size() != b.size()) {
        return false;
    }
    auto ib = b.begin();
    for (auto ia = a.begin(); ia != a.end(); ++ia, ++ib) {
        if (ia->first != ib->first || ia->second != ib->second) {
            return false;
        }
    }
    return true;
}

std::vector<EffectState> StatesOf(const std::vector<Effect*>& v) {
    std::vector<EffectState> out;
    out.reserve(v.size());
    for (const Effect* e : v) {
        out.push_back(EffectState::Of(*e));
    }
    return out;
}

// `before` is the snapshot of a layer, `after` its effects now.
void DiffEffects(const LayerLocator& loc, const std::vector<Snap::EffectSnap>& before, const std::vector<Effect*>& after, Out& out) {
    std::unordered_map<SeqObjectId, const Snap::EffectSnap*> was;
    was.reserve(before.size());
    for (const auto& e : before) {
        was[e.uid] = &e;
    }
    std::unordered_map<SeqObjectId, const Effect*> now;
    now.reserve(after.size());
    for (const Effect* e : after) {
        now[e->GetUid()] = e;
    }
    size_t changed = 0;
    for (const Effect* e : after) {
        auto it = was.find(e->GetUid());
        if (it == was.end()) {
            ++changed;
            continue;
        }
        const Snap::EffectSnap& b = *it->second;
        if (b.startTimeMS != e->GetStartTimeMS() || b.endTimeMS != e->GetEndTimeMS() || b.isProtected != e->GetProtected() ||
            b.name != e->GetEffectName() || b.linkedSymbol != e->GetLinkedSymbolId() || !SameMap(b.settings, e->GetSettings()) ||
            !SameMap(b.palette, e->GetPaletteMap())) {
            ++changed;
        }
    }
    for (const auto& e : before) {
        changed += now.count(e.uid) ? 0 : 1;
    }
    if (changed == 0) {
        return;
    }
    if (changed > SequenceEditCapture::kReplaceLayerThreshold && changed * 2 >= (std::max)(before.size(), after.size())) {
        out.Add(std::make_unique<ReplaceLayerContentsCommand>(loc, StatesOf(after)),
                std::make_unique<ReplaceLayerContentsCommand>(loc, StatesOf(before)));
        return;
    }

    auto target = [&](SeqObjectId uid) { return EffectLocator{ loc.element, loc.layer, uid }; };
    auto replaced = [&](const Snap::EffectSnap& b, const Effect* e) { return b.isProtected != e->GetProtected(); };

    // Deletes first and adds last, so nothing transiently overlaps.
    for (const auto& b : before) {
        auto it = now.find(b.uid);
        if (it == now.end() || replaced(b, it->second)) {
            out.Add(std::make_unique<DeleteEffectCommand>(target(b.uid)), std::make_unique<AddEffectCommand>(loc, StateOf(b)));
        }
    }
    for (const Effect* e : after) {
        auto it = was.find(e->GetUid());
        if (it == was.end() || replaced(*it->second, e)) {
            continue;
        }
        const Snap::EffectSnap& b = *it->second;
        SeqObjectId uid = b.uid;
        if (b.startTimeMS != e->GetStartTimeMS() || b.endTimeMS != e->GetEndTimeMS()) {
            out.Add(std::make_unique<MoveEffectCommand>(target(uid), e->GetStartTimeMS(), e->GetEndTimeMS()),
                    std::make_unique<MoveEffectCommand>(target(uid), b.startTimeMS, b.endTimeMS));
        }
        // A link that goes away is dropped before the settings change and a
        // new one made after it, so neither change spreads to the symbol's
        // other effects when replayed.
        const std::string& link = e->GetLinkedSymbolId();
        bool relinked = b.linkedSymbol != link;
        if (relinked && !b.linkedSymbol.empty()) {
            out.Add(std::make_unique<SetEffectSymbolLinkCommand>(target(uid), ""),
                    std::make_unique<SetEffectSymbolLinkCommand>(target(uid), b.linkedSymbol));
        }
        const SettingsMap& es = e->GetSettings();
        const SettingsMap& ep = e->GetPaletteMap();
        bool renamed = b.name != e->GetEffectName();
        if (renamed || !SameMap(b.settings, es) || !SameMap(b.palette, ep)) {
            std::optional<std::string> name, priorName;
            if (renamed) {
                name = e->GetEffectName();
                priorName = b.name;
            }
            out.Add(std::make_unique<ModifyEffectCommand>(target(uid), std::move(name), SettingsChange{ false, DiffMaps(b.settings, es) },
                                                          SettingsChange{ false, DiffMaps(b.palette, ep) }),
                    std::make_unique<ModifyEffectCommand>(target(uid), std::move(priorName), SettingsChange{ false, DiffMaps(es, b.settings) },
                                                          SettingsChange{ false, DiffMaps(ep, b.palette) }));
        }
        if (relinked && !link.empty()) {
            out.Add(std::make_unique<SetEffectSymbolLinkCommand>(target(uid), link),
                    std::make_unique<SetEffectSymbolLinkCommand>(target(uid), ""));
        }
    }
    for (const Effect* e : after) {
        auto it = was.find(e->GetUid());
        if (it == was.end() || replaced(*it->second, e)) {
            out.Add(std::make_unique<AddEffectCommand>(loc, EffectState::Of(*e)), std::make_unique<DeleteEffectCommand>(target(e->GetUid())));
        }
    }
}

// Indices into `seq` of a longest strictly increasing subsequence.
std::vector<size_t> LongestIncreasing(const std::vector<int>& seq) {
    std::vector<size_t> len(seq.size(), 1);
    std::vector<int> prev(seq.size(), -1);
    size_t best = 0;
    for (size_t i = 0; i < seq.size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            if (seq[j] < seq[i] && len[j] + 1 > len[i]) {
                len[i] = len[j] + 1;
                prev[i] = (int)j;
            }
        }
        if (len[i] > len[best]) {
            best = i;
        }
    }
    std::vector<size_t> out;
    if (seq.empty()) {
        return out;
    }
    for (int i = (int)best; i >= 0; i = prev[i]) {
        out.push_back((size_t)i);
    }
    std::reverse(out.begin(), out.end());
    return out;
}

void DiffElement(const Snap::ElementSnap& before, const Element* element, Out& out) {
    const SeqObjectId el = element->GetUid();
    const std::vector<EffectLayer*>& layers = element->GetEffectLayers();

    if (element->IsRenderDisabledFlag() != before.renderDisabled) {
        out.Add(std::make_unique<SetElementRenderDisabledCommand>(el, element->IsRenderDisabledFlag()),
                std::make_unique<SetElementRenderDisabledCommand>(el, before.renderDisabled));
    }
    if (const TimingElement* te = dynamic_cast<const TimingElement*>(element); te != nullptr && before.timing &&
                                                                               te->GetFixedTiming() != before.fixedTimingMS) {
        out.Add(std::make_unique<SetTimingFixedCommand>(el, te->GetFixedTiming()),
                std::make_unique<SetTimingFixedCommand>(el, before.fixedTimingMS));
    }

    // Node layers are fixed by the model; only their effects change.
    if (element->GetType() == ElementType::ELEMENT_TYPE_STRAND && !before.nodeLayers.empty()) {
        std::unordered_map<SeqObjectId, const Snap::LayerSnap*> nodesBefore;
        for (const auto& l : before.nodeLayers) {
            nodesBefore[l.uid] = &l;
        }
        const StrandElement* st = dynamic_cast<const StrandElement*>(element);
        for (int n = 0; st != nullptr && n < st->GetNodeLayerCount(); ++n) {
            const NodeLayer* nl = st->GetNodeLayer(n);
            auto it = nl != nullptr ? nodesBefore.find(nl->GetUid()) : nodesBefore.end();
            if (it != nodesBefore.end()) {
                DiffEffects({ el, nl->GetUid() }, it->second->effects, nl->GetEffects(), out);
            }
        }
    }

    std::unordered_map<SeqObjectId, int> beforeIndex;
    for (size_t i = 0; i < before.layers.size(); ++i) {
        beforeIndex[before.layers[i].uid] = (int)i;
    }
    // Layers present before and after, in their new order; those keeping
    // their relative order stay put, the others are moved.
    std::vector<size_t> survivors;
    std::vector<int> survivorBefore;
    for (size_t i = 0; i < layers.size(); ++i) {
        auto it = beforeIndex.find(layers[i]->GetUid());
        if (it != beforeIndex.end()) {
            survivors.push_back(i);
            survivorBefore.push_back(it->second);
        }
    }
    std::vector<bool> stable(layers.size(), false);
    for (size_t k : LongestIncreasing(survivorBefore)) {
        stable[survivors[k]] = true;
    }

    for (size_t i = 0; i < layers.size(); ++i) {
        if (!stable[i]) {
            continue;
        }
        const EffectLayer* a = layers[i];
        const Snap::LayerSnap& b = before.layers[beforeIndex[a->GetUid()]];
        LayerLocator loc{ el, a->GetUid() };
        if (a->GetLayerName() != b.name) {
            out.Add(std::make_unique<SetLayerNameCommand>(loc, a->GetLayerName()), std::make_unique<SetLayerNameCommand>(loc, b.name));
        }
        DiffEffects(loc, b.effects, a->GetEffects(), out);
    }

    // Walk the layer order as the commands change it, so each inverse can
    // name the sibling its layer sat after when it was removed.
    std::vector<SeqObjectId> cur;
    for (const auto& l : before.layers) {
        cur.push_back(l.uid);
    }
    auto indexIn = [&](SeqObjectId uid) {
        auto it = std::find(cur.begin(), cur.end(), uid);
        return it == cur.end() ? -1 : (int)(it - cur.begin());
    };
    auto remove = [&](const Snap::LayerSnap& b) {
        int pos = indexIn(b.uid);
        SeqObjectId prior = pos > 0 ? cur[pos - 1] : kNoSeqObjectId;
        out.Add(std::make_unique<RemoveLayerCommand>(el, b.uid),
                std::make_unique<AddLayerCommand>(el, b.uid, prior, pos, b.name, StatesOf(b.effects)));
        cur.erase(cur.begin() + pos);
    };

    for (size_t i = 0; i < layers.size(); ++i) {
        if (stable[i]) {
            continue;
        }
        const EffectLayer* a = layers[i];
        auto bi = beforeIndex.find(a->GetUid());
        if (bi != beforeIndex.end()) {
            remove(before.layers[bi->second]);
        }
        SeqObjectId prior = i > 0 ? layers[i - 1]->GetUid() : kNoSeqObjectId;
        int pos = prior == kNoSeqObjectId ? 0 : indexIn(prior) + 1;
        out.Add(std::make_unique<AddLayerCommand>(el, a->GetUid(), prior, pos, a->GetLayerName(), StatesOf(a->GetEffects())),
                std::make_unique<RemoveLayerCommand>(el, a->GetUid()));
        cur.insert(cur.begin() + pos, a->GetUid());
    }

    std::unordered_map<SeqObjectId, bool> present;
    for (const EffectLayer* l : layers) {
        present[l->GetUid()] = true;
    }
    for (const auto& b : before.layers) {
        if (!present.count(b.uid)) {
            remove(b);
        }
    }
}

} // namespace

SequenceEditCapture::SequenceEditCapture(SequenceElements& seq, std::string label, bool recordUndo) :
    _seq(seq), _label(std::move(label)), _recordUndo(recordUndo), _scope(std::make_unique<SeqMutationGuard::CaptureScope>()) {
    CommandExecutor& exec = _seq.GetCommandExecutor();
    _active = exec._capture == nullptr;
    if (_active) {
        exec.BeginBatch(_label);
        _before = Take(_seq);
        for (size_t i = 0; i < _seq.GetElementCount(MASTER_VIEW); ++i) {
            _knownTop.insert(_seq.GetElement(i, MASTER_VIEW)->GetUid());
        }
        _media = _seq.GetSequenceMedia().GetEmbeddedEntries();
        _faces = _seq.GetSequenceFaces().GetFaces();
        exec._capture = this;
    }
}

SequenceEditCapture::~SequenceEditCapture() {
    End(true);
}

void SequenceEditCapture::Commit() {
    End(true);
}

void SequenceEditCapture::Discard() {
    End(false);
}

namespace {
// Elements `cmd` adds (AddElement commands, including inside batches).
void AddedBy(const SequenceCommand& cmd, std::unordered_set<SeqObjectId>& out) {
    if (cmd.Type() == CommandType::AddElement) {
        out.insert(static_cast<const AddElementCommand&>(cmd).GetUid());
    } else if (cmd.Type() == CommandType::Batch) {
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            AddedBy(*m, out);
        }
    }
}

// Media keys `cmd` changes.
void MediaKeysOf(const SequenceCommand& cmd, std::set<std::string>& out) {
    switch (cmd.Type()) {
    case CommandType::PutMedia: out.insert(static_cast<const PutMediaCommand&>(cmd).GetState().key); break;
    case CommandType::RemoveMedia: out.insert(static_cast<const RemoveMediaCommand&>(cmd).GetKey()); break;
    case CommandType::RenameMedia:
        out.insert(static_cast<const RenameMediaCommand&>(cmd).GetFrom());
        out.insert(static_cast<const RenameMediaCommand&>(cmd).GetTo());
        break;
    case CommandType::Batch:
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            MediaKeysOf(*m, out);
        }
        break;
    default: break;
    }
}

bool RenamesMedia(const SequenceCommand& cmd) {
    if (cmd.Type() == CommandType::RenameMedia) {
        return true;
    }
    if (cmd.Type() == CommandType::Batch) {
        for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
            if (RenamesMedia(*m)) {
                return true;
            }
        }
    }
    return false;
}
} // namespace

void SequenceEditCapture::NoteElement(Element* el) {
    _knownTop.insert(el->GetUid());
    _before[el->GetUid()] = TakeElement(el);
    if (const ModelElement* mel = dynamic_cast<const ModelElement*>(el)) {
        for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
            if (SubModelElement* sme = mel->GetSubModel(x)) {
                _before[sme->GetUid()] = TakeElement(sme);
            }
        }
    }
}

void SequenceEditCapture::TakeDirectAdds(const SequenceCommand* except, Cmds& applied, Cmds& inverses) {
    std::unordered_set<SeqObjectId> skip;
    if (except != nullptr) {
        AddedBy(*except, skip);
    }
    for (size_t i = 0; i < _seq.GetElementCount(MASTER_VIEW); ++i) {
        Element* el = _seq.GetElement(i, MASTER_VIEW);
        if (_knownTop.count(el->GetUid()) || skip.count(el->GetUid())) {
            continue;
        }
        SeqObjectId after = i > 0 ? _seq.GetElement(i - 1, MASTER_VIEW)->GetUid() : kNoSeqObjectId;
        applied.push_back(std::make_unique<AddElementCommand>(ElementSnapshot::Of(*el), after, (int)i));
        inverses.push_back(std::make_unique<DeleteElementCommand>(el->GetUid(), true));
        NoteElement(el);
    }
}

void SequenceEditCapture::BeforeRecording(const SequenceCommand* cmd) {
    if (_flushing) {
        return;
    }
    Cmds applied, inverses;
    TakeDirectAdds(cmd, applied, inverses);
    if (applied.empty()) {
        return;
    }
    _flushing = true;
    CommandExecutor& exec = _seq.GetCommandExecutor();
    for (size_t i = 0; i < applied.size(); ++i) {
        exec.RecordApplied(std::move(applied[i]), std::move(inverses[i]), _recordUndo);
    }
    _flushing = false;
}

void SequenceEditCapture::AfterRecording(const SequenceCommand& cmd) {
    std::unordered_set<SeqObjectId> added;
    AddedBy(cmd, added);
    for (SeqObjectId uid : added) {
        if (Element* el = _seq.GetElementFromUid(uid)) {
            NoteElement(el);
        }
    }
    std::set<std::string> keys;
    MediaKeysOf(cmd, keys);
    if (!keys.empty()) {
        auto now = _seq.GetSequenceMedia().GetEmbeddedEntries();
        for (const auto& key : keys) {
            auto it = now.find(key);
            if (it == now.end()) {
                _media.erase(key);
            } else {
                _media[key] = it->second;
            }
        }
    }
    if (RenamesMedia(cmd)) {
        // A media rename repoints face images; the command carries that.
        _faces = _seq.GetSequenceFaces().GetFaces();
    }
}

void SequenceEditCapture::NoteCommandAdds() {
    for (size_t i = 0; i < _seq.GetElementCount(MASTER_VIEW); ++i) {
        Element* el = _seq.GetElement(i, MASTER_VIEW);
        if (!_knownTop.count(el->GetUid())) {
            NoteElement(el);
        }
    }
}

void SequenceEditCapture::ElementDeleting(Element* el, bool keepInViews) {
    int idx = -1;
    for (size_t i = 0; i < _seq.GetElementCount(MASTER_VIEW); ++i) {
        if (_seq.GetElement(i, MASTER_VIEW) == el) {
            idx = (int)i;
            break;
        }
    }
    if (idx < 0) {
        return;
    }
    // Anything added directly before it goes first, so the delete's undo
    // finds its place.
    BeforeRecording(nullptr);
    ElementSnapshot snap = ElementSnapshot::Of(*el);
    // Put back the layers as they were when the capture started, where the
    // layer structure is unchanged.
    auto restore = [&](SeqObjectId uid, std::vector<LayerSnapshot>& layers) {
        auto it = _before.find(uid);
        if (it == _before.end() || it->second.layers.size() != layers.size()) {
            return;
        }
        for (size_t i = 0; i < layers.size(); ++i) {
            if (it->second.layers[i].uid != layers[i].uid) {
                return;
            }
        }
        for (size_t i = 0; i < layers.size(); ++i) {
            layers[i].name = it->second.layers[i].name;
            layers[i].effects = StatesOf(it->second.layers[i].effects);
        }
    };
    restore(snap.uid, snap.layers);
    for (auto& sub : snap.subElements) {
        restore(sub.uid, sub.layers);
    }
    if (auto it = _before.find(snap.uid); it != _before.end()) {
        snap.renderDisabled = it->second.renderDisabled;
        if (snap.timing) {
            snap.fixedTimingMS = it->second.fixedTimingMS;
        }
    }
    SeqObjectId after = idx > 0 ? _seq.GetElement(idx - 1, MASTER_VIEW)->GetUid() : kNoSeqObjectId;
    _flushing = true;
    _seq.GetCommandExecutor().RecordApplied(std::make_unique<DeleteElementCommand>(el->GetUid(), keepInViews),
                                            std::make_unique<AddElementCommand>(std::move(snap), after, idx), _recordUndo);
    _flushing = false;
    _knownTop.erase(el->GetUid());
    _before.erase(el->GetUid());
    if (const ModelElement* mel = dynamic_cast<const ModelElement*>(el)) {
        for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
            if (SubModelElement* sme = mel->GetSubModel(x)) {
                _before.erase(sme->GetUid());
            }
        }
    }
}

void SequenceEditCapture::DiffMedia(Cmds& applied, Cmds& inverses) {
    SequenceMedia& media = _seq.GetSequenceMedia();
    auto now = media.GetEmbeddedEntries();
    auto restore = [&](const std::string& key) -> std::unique_ptr<SequenceCommand> {
        auto it = _media.find(key);
        if (it != _media.end()) {
            return std::make_unique<PutMediaCommand>(SequenceMedia::StateOf(key, *it->second));
        }
        return std::make_unique<RemoveMediaCommand>(key);
    };
    for (const auto& [key, entry] : _media) {
        if (now.count(key)) {
            continue;
        }
        // No longer embedded: removed, or left as a file reference.
        if (auto cur = media.GetEntryState(key)) {
            applied.push_back(std::make_unique<PutMediaCommand>(std::move(*cur)));
        } else {
            applied.push_back(std::make_unique<RemoveMediaCommand>(key));
        }
        inverses.push_back(restore(key));
    }
    for (const auto& [key, entry] : now) {
        auto it = _media.find(key);
        if (it != _media.end() && it->second == entry) {
            continue;
        }
        applied.push_back(std::make_unique<PutMediaCommand>(SequenceMedia::StateOf(key, *entry)));
        inverses.push_back(restore(key));
    }
}

void SequenceEditCapture::DiffFaces(Cmds& applied, Cmds& inverses) {
    const auto& now = _seq.GetSequenceFaces().GetFaces();
    for (const auto& [name, def] : _faces) {
        if (now.find(name) == now.end()) {
            applied.push_back(std::make_unique<SetSequenceFaceCommand>(name, std::nullopt));
            inverses.push_back(std::make_unique<SetSequenceFaceCommand>(name, def));
        }
    }
    for (const auto& [name, def] : now) {
        auto it = _faces.find(name);
        if (it != _faces.end() && it->second == def) {
            continue;
        }
        std::optional<SetSequenceFaceCommand::FaceDefinition> prior;
        if (it != _faces.end()) {
            prior = it->second;
        }
        applied.push_back(std::make_unique<SetSequenceFaceCommand>(name, def));
        inverses.push_back(std::make_unique<SetSequenceFaceCommand>(name, std::move(prior)));
    }
}

void SequenceEditCapture::End(bool record) {
    if (!_open) {
        return;
    }
    _open = false;
    _scope.reset();
    if (!_active) {
        return;
    }
    CommandExecutor& exec = _seq.GetCommandExecutor();
    exec._capture = nullptr;
    if (record) {
        Cmds applied, inverses;
        // New elements first (they carry their contents), then media and
        // faces the effects may use, then everything else.
        TakeDirectAdds(nullptr, applied, inverses);
        DiffMedia(applied, inverses);
        DiffFaces(applied, inverses);
        Diff(_before, _seq, applied, inverses);
        exec.RecordAppliedEdits(_label, std::move(applied), std::move(inverses), _recordUndo);
    }
    _before.clear();
    _knownTop.clear();
    _media.clear();
    _faces.clear();
    _madeUndoStep = exec.EndBatch();
}

SequenceEditCapture::Snapshot SequenceEditCapture::Take(const SequenceElements& seq) {
    Snapshot snap;
    ForEachElement(seq, [&](Element* el) { snap.emplace(el->GetUid(), TakeElement(el)); });
    return snap;
}

void SequenceEditCapture::Diff(const Snapshot& before, const SequenceElements& seq, Cmds& applied, Cmds& inverses) {
    Out out{ applied, inverses };
    ForEachElement(seq, [&](Element* el) {
        auto it = before.find(el->GetUid());
        if (it != before.end()) {
            DiffElement(it->second, el, out);
        }
    });
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceElementCommands.h"

#include <algorithm>
#include <climits>
#include <set>
#include <utility>

#include <log.h>

#include "CommandExecutor.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "RenderContext.h"
#include "EffectSymbol.h"
#include "EffectSymbolManager.h"
#include "SequenceElements.h"
#include "SequenceFaces.h"
#include "SongStructureManager.h"
#include "UndoManager.h"
#include "../effects/EffectManager.h"
#include "../effects/RenderableEffect.h"
#include "../utils/UtilClasses.h"

using nlohmann::json;

namespace {

// ---- lookups --------------------------------------------------------------

std::vector<Element*> MasterElements(const SequenceElements& seq) {
    std::vector<Element*> out;
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        out.push_back(seq.GetElement(i, MASTER_VIEW));
    }
    return out;
}

int MasterIndexOf(const SequenceElements& seq, SeqObjectId uid) {
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        if (seq.GetElement(i, MASTER_VIEW)->GetUid() == uid) {
            return (int)i;
        }
    }
    return -1;
}

// A top-level (master view) element, never a submodel or strand.
Element* TopLevelElement(const SequenceElements& seq, SeqObjectId uid) {
    int idx = MasterIndexOf(seq, uid);
    return idx < 0 ? nullptr : seq.GetElement(idx, MASTER_VIEW);
}

bool NameInUse(const SequenceElements& seq, const std::string& name) {
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        if (seq.GetElement(i, MASTER_VIEW)->GetName() == name) {
            return true;
        }
    }
    return false;
}

void ForEachLayer(Element* el, const std::function<void(EffectLayer*)>& fn) {
    for (EffectLayer* l : el->GetEffectLayers()) {
        fn(l);
    }
    if (auto* me = dynamic_cast<ModelElement*>(el)) {
        for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
            SubModelElement* sme = me->GetSubModel(x);
            if (sme == nullptr) {
                continue;
            }
            for (EffectLayer* l : sme->GetEffectLayers()) {
                fn(l);
            }
            if (auto* st = dynamic_cast<StrandElement*>(sme)) {
                for (int n = 0; n < st->GetNodeLayerCount(); ++n) {
                    fn(st->GetNodeLayer(n));
                }
            }
        }
    }
}

bool HoldsLockedEffect(Element* el) {
    bool locked = false;
    ForEachLayer(el, [&](EffectLayer* l) {
        for (const Effect* e : l->GetEffects()) {
            locked = locked || e->IsLocked();
        }
    });
    return locked;
}

std::unique_ptr<SequenceCommand> Failed(CommandResult* out, CommandResult r) {
    if (out != nullptr) {
        *out = std::move(r);
    }
    return nullptr;
}

std::unique_ptr<SequenceCommand> Succeeded(CommandResult* out, std::unique_ptr<SequenceCommand> inverse) {
    if (out != nullptr) {
        *out = CommandResult::Success();
    }
    return inverse;
}

// ---- snapshots ------------------------------------------------------------

LayerSnapshot SnapshotLayer(const EffectLayer* l) {
    LayerSnapshot s;
    s.uid = l->GetUid();
    s.name = l->GetLayerName();
    for (const Effect* e : l->GetEffects()) {
        s.effects.push_back(EffectState::Of(*e));
    }
    return s;
}

std::vector<LayerSnapshot> SnapshotLayers(const Element* el) {
    std::vector<LayerSnapshot> out;
    for (const EffectLayer* l : el->GetEffectLayers()) {
        out.push_back(SnapshotLayer(l));
    }
    return out;
}

Effect* CreateEffect(EffectLayer* layer, const EffectState& s) {
    Effect* e = layer->AddEffect(0, s.name, s.settings, s.palette, s.startTimeMS, s.endTimeMS, s.selected, s.isProtected);
    if (e != nullptr && s.uid != kNoSeqObjectId && layer->GetEffectFromUid(s.uid) == nullptr) {
        e->SetUid(s.uid);
    }
    if (e != nullptr) {
        e->HandlePastedSymbolLink();
    }
    return e;
}

// Makes the element's own layers match the snapshot (at least one layer).
void RestoreLayers(Element* el, const std::vector<LayerSnapshot>& layers) {
    size_t want = std::max<size_t>(1, layers.size());
    while (el->GetEffectLayerCount() < want) {
        el->AddEffectLayer();
    }
    while (el->GetEffectLayerCount() > want) {
        el->RemoveEffectLayer((int)el->GetEffectLayerCount() - 1);
    }
    for (size_t i = 0; i < want; ++i) {
        EffectLayer* l = el->GetEffectLayer((int)i);
        l->DeleteAllEffects();
        if (i >= layers.size()) {
            continue;
        }
        if (layers[i].uid != kNoSeqObjectId) {
            l->SetUid(layers[i].uid);
        }
        if (l->GetLayerName() != layers[i].name) {
            l->SetLayerName(layers[i].name);
        }
        for (const auto& s : layers[i].effects) {
            CreateEffect(l, s);
        }
    }
}

SubModelElement* FindOrCreateSubModel(ModelElement* me, const std::string& name) {
    for (int i = 0; i < me->GetSubModelCount(); ++i) {
        SubModelElement* sme = me->GetSubModel(i);
        if (sme != nullptr && sme->GetName() == name) {
            return sme;
        }
    }
    auto* sme = new SubModelElement(me, name);
    me->AddSubModel(sme);
    return sme;
}

void RestoreSubElement(ModelElement* me, const SubElementSnapshot& s) {
    SubModelElement* sme = s.strand ? me->GetStrand(s.strandIndex, true) : FindOrCreateSubModel(me, s.name);
    if (sme == nullptr) {
        return;
    }
    if (s.uid != kNoSeqObjectId) {
        sme->SetUid(s.uid);
    }
    RestoreLayers(sme, s.layers);
    auto* st = dynamic_cast<StrandElement*>(sme);
    if (st == nullptr) {
        return;
    }
    for (size_t n = 0; n < s.nodeLayerUids.size(); ++n) {
        NodeLayer* nl = st->GetNodeLayer((int)n, true);
        if (nl != nullptr && s.nodeLayerUids[n] != kNoSeqObjectId) {
            nl->SetUid(s.nodeLayerUids[n]);
        }
    }
    for (const auto& [n, effects] : s.nodeEffects) {
        NodeLayer* nl = st->GetNodeLayer(n, true);
        if (nl == nullptr) {
            continue;
        }
        nl->DeleteAllEffects();
        for (const auto& e : effects) {
            CreateEffect(nl, e);
        }
    }
}

// Position in the master view, counted without the element itself, that
// `after` names; -1 when after is set but not found.
int PositionAfter(const std::vector<Element*>& order, SeqObjectId self, SeqObjectId after) {
    if (after == kNoSeqObjectId) {
        return 0;
    }
    int pos = 0;
    for (Element* e : order) {
        if (e->GetUid() == self) {
            continue;
        }
        ++pos;
        if (e->GetUid() == after) {
            return pos;
        }
    }
    return -1;
}

SeqObjectId PrecedingUid(const SequenceElements& seq, int index) {
    return index > 0 ? seq.GetElement(index - 1, MASTER_VIEW)->GetUid() : kNoSeqObjectId;
}

// ---- JSON ----------------------------------------------------------------

bool ReadU64(const json& j, const char* key, uint64_t& out) {
    auto it = j.find(key);
    if (it == j.end()) {
        return false;
    }
    if (it->is_number_unsigned()) {
        out = it->get<uint64_t>();
        return true;
    }
    if (it->is_number_integer() && it->get<int64_t>() >= 0) {
        out = (uint64_t)it->get<int64_t>();
        return true;
    }
    return false;
}

bool ReadInt(const json& j, const char* key, int& out) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_number_integer()) {
        return false;
    }
    int64_t v = it->get<int64_t>();
    if (v < INT32_MIN || v > INT32_MAX) {
        return false;
    }
    out = (int)v;
    return true;
}

bool ReadString(const json& j, const char* key, std::string& out) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_string()) {
        return false;
    }
    out = it->get<std::string>();
    return true;
}

bool ReadBool(const json& j, const char* key, bool& out) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_boolean()) {
        return false;
    }
    out = it->get<bool>();
    return true;
}

const json* ReadArray(const json& j, const char* key) {
    auto it = j.find(key);
    return (it == j.end() || !it->is_array()) ? nullptr : &*it;
}

json WriteEffects(const std::vector<EffectState>& v) {
    json a = json::array();
    for (const auto& s : v) {
        a.push_back(json{ { "uid", s.uid }, { "name", s.name }, { "settings", s.settings }, { "palette", s.palette },
                          { "start", s.startTimeMS }, { "end", s.endTimeMS }, { "protected", s.isProtected } });
    }
    return a;
}

bool ReadEffects(const json* a, std::vector<EffectState>& v) {
    if (a == nullptr) {
        return false;
    }
    for (const auto& j : *a) {
        EffectState s;
        if (!j.is_object() || !ReadU64(j, "uid", s.uid) || s.uid == kNoSeqObjectId || !ReadString(j, "name", s.name) ||
            !ReadString(j, "settings", s.settings) || !ReadString(j, "palette", s.palette) || !ReadInt(j, "start", s.startTimeMS) ||
            !ReadInt(j, "end", s.endTimeMS) || !ReadBool(j, "protected", s.isProtected)) {
            return false;
        }
        v.push_back(std::move(s));
    }
    return true;
}

json WriteLayers(const std::vector<LayerSnapshot>& layers) {
    json a = json::array();
    for (const auto& l : layers) {
        a.push_back(json{ { "uid", l.uid }, { "name", l.name }, { "effects", WriteEffects(l.effects) } });
    }
    return a;
}

bool ReadLayers(const json* a, std::vector<LayerSnapshot>& out) {
    if (a == nullptr) {
        return false;
    }
    for (const auto& j : *a) {
        LayerSnapshot l;
        if (!j.is_object() || !ReadU64(j, "uid", l.uid) || l.uid == kNoSeqObjectId || !ReadString(j, "name", l.name) ||
            !ReadEffects(ReadArray(j, "effects"), l.effects)) {
            return false;
        }
        out.push_back(std::move(l));
    }
    return true;
}

json WriteSnapshot(const ElementSnapshot& s) {
    json subs = json::array();
    for (const auto& sub : s.subElements) {
        json nodeUids = json::array();
        for (SeqObjectId id : sub.nodeLayerUids) {
            nodeUids.push_back(id);
        }
        json nodeEffects = json::array();
        for (const auto& [n, effects] : sub.nodeEffects) {
            nodeEffects.push_back(json{ { "node", n }, { "effects", WriteEffects(effects) } });
        }
        subs.push_back(json{ { "strand", sub.strand }, { "name", sub.name }, { "strandIndex", sub.strandIndex }, { "uid", sub.uid },
                             { "layers", WriteLayers(sub.layers) }, { "nodeLayers", nodeUids }, { "nodeEffects", nodeEffects } });
    }
    return json{ { "uid", s.uid },       { "timing", s.timing },          { "name", s.name },
                 { "renderDisabled", s.renderDisabled }, { "subType", s.subType }, { "fixed", s.fixedTimingMS },
                 { "layers", WriteLayers(s.layers) },    { "subElements", subs } };
}

bool ReadSnapshot(const json& j, ElementSnapshot& s) {
    if (!j.is_object() || !ReadU64(j, "uid", s.uid) || s.uid == kNoSeqObjectId || !ReadBool(j, "timing", s.timing) ||
        !ReadString(j, "name", s.name) || !ReadBool(j, "renderDisabled", s.renderDisabled) || !ReadString(j, "subType", s.subType) ||
        !ReadInt(j, "fixed", s.fixedTimingMS) || !ReadLayers(ReadArray(j, "layers"), s.layers)) {
        return false;
    }
    const json* subs = ReadArray(j, "subElements");
    if (subs == nullptr) {
        return false;
    }
    for (const auto& sj : *subs) {
        SubElementSnapshot sub;
        if (!sj.is_object() || !ReadBool(sj, "strand", sub.strand) || !ReadString(sj, "name", sub.name) ||
            !ReadInt(sj, "strandIndex", sub.strandIndex) || !ReadU64(sj, "uid", sub.uid) || sub.uid == kNoSeqObjectId ||
            !ReadLayers(ReadArray(sj, "layers"), sub.layers)) {
            return false;
        }
        if (sub.strand ? sub.strandIndex < 0 : sub.name.empty()) {
            return false;
        }
        const json* nodes = ReadArray(sj, "nodeLayers");
        const json* nodeEffects = ReadArray(sj, "nodeEffects");
        if (nodes == nullptr || nodeEffects == nullptr) {
            return false;
        }
        for (const auto& n : *nodes) {
            if (!n.is_number_unsigned() && !(n.is_number_integer() && n.get<int64_t>() >= 0)) {
                return false;
            }
            sub.nodeLayerUids.push_back(n.get<uint64_t>());
        }
        for (const auto& ne : *nodeEffects) {
            int node = -1;
            std::vector<EffectState> effects;
            if (!ne.is_object() || !ReadInt(ne, "node", node) || node < 0 || !ReadEffects(ReadArray(ne, "effects"), effects)) {
                return false;
            }
            sub.nodeEffects[node] = std::move(effects);
        }
        s.subElements.push_back(std::move(sub));
    }
    return true;
}

json WriteViews(const std::vector<SongStructureView>& views) {
    json a = json::array();
    for (const auto& v : views) {
        json regions = json::array();
        for (const auto& r : v.regions) {
            regions.push_back(json{ { "id", r.id }, { "start", r.startTimeMS }, { "end", r.endTimeMS }, { "name", r.name }, { "color", r.colorARGB } });
        }
        a.push_back(json{ { "name", v.name }, { "nextId", v.nextId }, { "regions", regions } });
    }
    return a;
}

bool ReadViews(const json* a, std::vector<SongStructureView>& out) {
    if (a == nullptr) {
        return false;
    }
    for (const auto& j : *a) {
        SongStructureView v;
        const json* regions = j.is_object() ? ReadArray(j, "regions") : nullptr;
        if (regions == nullptr || !ReadString(j, "name", v.name) || !ReadInt(j, "nextId", v.nextId)) {
            return false;
        }
        for (const auto& rj : *regions) {
            SongStructureRegion r;
            uint64_t color = 0;
            if (!rj.is_object() || !ReadInt(rj, "id", r.id) || !ReadInt(rj, "start", r.startTimeMS) || !ReadInt(rj, "end", r.endTimeMS) ||
                !ReadString(rj, "name", r.name) || !ReadU64(rj, "color", color) || color > UINT32_MAX) {
                return false;
            }
            r.colorARGB = (uint32_t)color;
            v.regions.push_back(std::move(r));
        }
        out.push_back(std::move(v));
    }
    return true;
}

bool SameViews(const std::vector<SongStructureView>& a, const std::vector<SongStructureView>& b) {
    return WriteViews(a) == WriteViews(b);
}

KeyChanges Diff(const SettingsMap& from, const SettingsMap& to) {
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

thread_local NewElementsRecorder* t_recorder = nullptr;

} // namespace

// ---------------------------------------------------------------------------
// ElementSnapshot
// ---------------------------------------------------------------------------

ElementSnapshot ElementSnapshot::Of(const Element& e) {
    ElementSnapshot s;
    s.uid = e.GetUid();
    s.timing = e.GetType() == ElementType::ELEMENT_TYPE_TIMING;
    s.name = e.GetName();
    s.renderDisabled = e.IsRenderDisabled();
    s.layers = SnapshotLayers(&e);
    if (const auto* te = dynamic_cast<const TimingElement*>(&e)) {
        s.subType = te->GetSubType();
        s.fixedTimingMS = te->GetFixedTiming();
        s.views = te->GetViews();
    }
    if (const auto* me = dynamic_cast<const ModelElement*>(&e)) {
        for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
            const SubModelElement* sme = me->GetSubModel(x);
            if (sme == nullptr) {
                continue;
            }
            SubElementSnapshot sub;
            sub.uid = sme->GetUid();
            sub.layers = SnapshotLayers(sme);
            if (const auto* st = dynamic_cast<const StrandElement*>(sme)) {
                sub.strand = true;
                sub.strandIndex = st->GetStrand();
                for (int n = 0; n < st->GetNodeLayerCount(); ++n) {
                    const NodeLayer* nl = st->GetNodeLayer(n);
                    sub.nodeLayerUids.push_back(nl->GetUid());
                    if (nl->GetEffectCount() > 0) {
                        sub.nodeEffects[n] = SnapshotLayer(nl).effects;
                    }
                }
            } else {
                sub.name = sme->GetName();
            }
            s.subElements.push_back(std::move(sub));
        }
    }
    return s;
}

bool ElementSnapshot::SameDocumentState(const ElementSnapshot& o) const {
    return WriteSnapshot(*this) == WriteSnapshot(o);
}

// ---------------------------------------------------------------------------
// AddElement
// ---------------------------------------------------------------------------

AddElementCommand::AddElementCommand(ElementSnapshot state, SeqObjectId after, int indexHint) :
    _state(std::move(state)), _after(after), _indexHint(indexHint) {
    if (_state.uid == kNoSeqObjectId) {
        _state.uid = SeqIds::Next();
    }
    if (_state.layers.empty()) {
        _state.layers.emplace_back();
    }
    for (auto& l : _state.layers) {
        if (l.uid == kNoSeqObjectId) {
            l.uid = SeqIds::Next();
        }
        for (auto& e : l.effects) {
            if (e.uid == kNoSeqObjectId) {
                e.uid = SeqIds::Next();
            }
        }
    }
}

CommandResult AddElementCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    if (_state.name.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "element has no name");
    }
    if (NameInUse(seq, _state.name)) {
        return CommandResult::Fail(CommandStatus::Invalid, "an element named " + _state.name + " is already in the sequence");
    }
    if (seq.GetElementFromUid(_state.uid) != nullptr) {
        return CommandResult::Fail(CommandStatus::Invalid, "element id already in use");
    }
    if (mode == ApplyMode::Strict && _after != kNoSeqObjectId && TopLevelElement(seq, _after) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "sibling element not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> AddElementCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    int count = (int)seq.GetElementCount(MASTER_VIEW);
    int pos = PositionAfter(MasterElements(seq), kNoSeqObjectId, _after);
    if (pos < 0) {
        pos = _indexHint < 0 ? count : std::min(_indexHint, count);
    }
    Element* el = seq.AddElement(pos, _state.name, _state.timing ? "timing" : "model", true, false, false, false, _state.renderDisabled);
    if (el == nullptr) {
        return Failed(result, CommandResult::Fail(CommandStatus::Invalid, "element could not be created"));
    }
    el->SetUid(_state.uid);
    if (auto* te = dynamic_cast<TimingElement*>(el)) {
        te->SetSubType(_state.subType);
        te->SetFixedTiming(_state.fixedTimingMS);
        if (!_state.views.empty()) {
            te->SetViews(_state.views);
        }
    }
    RestoreLayers(el, _state.layers);
    if (auto* me = dynamic_cast<ModelElement*>(el)) {
        for (const auto& sub : _state.subElements) {
            RestoreSubElement(me, sub);
        }
        // Rows the layout model added that the snapshot didn't name got
        // fresh ids here; record them so that applying this command again
        // elsewhere gives every row the same id.
        std::string views = std::move(_state.views);
        _state = ElementSnapshot::Of(*el);
        _state.views = std::move(views);
    }
    seq.PopulateRowInformation();
    return Succeeded(result, std::make_unique<DeleteElementCommand>(_state.uid));
}

void AddElementCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", WriteSnapshot(_state) }, { "after", _after }, { "indexHint", _indexHint } };
}

std::unique_ptr<SequenceCommand> AddElementCommand::Clone() const {
    return std::make_unique<AddElementCommand>(*this);
}

// ---------------------------------------------------------------------------
// DeleteElement
// ---------------------------------------------------------------------------

DeleteElementCommand::DeleteElementCommand(SeqObjectId element, bool keepInViews) :
    _element(element), _keepInViews(keepInViews) {
}

CommandResult DeleteElementCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Element* el = TopLevelElement(seq, _element);
    if (el == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "element not found");
    }
    if (mode == ApplyMode::Strict && HoldsLockedEffect(el)) {
        return CommandResult::Fail(CommandStatus::Locked, "element holds a locked effect");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> DeleteElementCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    int idx = MasterIndexOf(seq, _element);
    Element* el = seq.GetElement(idx, MASTER_VIEW);
    ElementSnapshot snapshot = ElementSnapshot::Of(*el);
    SeqObjectId after = PrecedingUid(seq, idx);
    seq.DeleteElement(el->GetName(), !_keepInViews);
    return Succeeded(result, std::make_unique<AddElementCommand>(std::move(snapshot), after, idx));
}

void DeleteElementCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "keepInViews", _keepInViews } };
}

std::unique_ptr<SequenceCommand> DeleteElementCommand::Clone() const {
    return std::make_unique<DeleteElementCommand>(*this);
}

// ---------------------------------------------------------------------------
// MoveElement
// ---------------------------------------------------------------------------

MoveElementCommand::MoveElementCommand(SeqObjectId element, SeqObjectId after, int indexHint) :
    _element(element), _after(after), _indexHint(indexHint) {
}

std::unique_ptr<MoveElementCommand> MoveElementCommand::FromIndices(const SequenceElements& seq, int index, int dest) {
    int count = (int)seq.GetElementCount(MASTER_VIEW);
    if (index < 0 || index >= count) {
        return nullptr;
    }
    // Final position with the element taken out, as MoveSequenceElement
    // computes it.
    int pos;
    if (dest < 0 || dest >= count) {
        pos = count - 1;
    } else {
        pos = index >= dest ? dest : dest - 1;
    }
    if (pos == index) {
        return nullptr;
    }
    std::vector<Element*> rest = MasterElements(seq);
    Element* el = rest[index];
    rest.erase(rest.begin() + index);
    SeqObjectId after = pos > 0 ? rest[pos - 1]->GetUid() : kNoSeqObjectId;
    return std::make_unique<MoveElementCommand>(el->GetUid(), after, pos);
}

CommandResult MoveElementCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    if (TopLevelElement(seq, _element) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "element not found");
    }
    if (_after == _element) {
        return CommandResult::Fail(CommandStatus::Invalid, "element cannot follow itself");
    }
    if (mode == ApplyMode::Strict && _after != kNoSeqObjectId && TopLevelElement(seq, _after) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "sibling element not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> MoveElementCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    int index = MasterIndexOf(seq, _element);
    int count = (int)seq.GetElementCount(MASTER_VIEW);
    auto inverse = std::make_unique<MoveElementCommand>(_element, PrecedingUid(seq, index), index);
    int pos = PositionAfter(MasterElements(seq), _element, _after);
    if (pos < 0) {
        pos = _indexHint < 0 ? count - 1 : std::min(_indexHint, count - 1);
    }
    if (pos != index) {
        // MoveSequenceElement's dest is the index the element lands before.
        seq.MoveSequenceElement(index, pos <= index ? pos : pos + 1, MASTER_VIEW);
        seq.PopulateRowInformation();
    }
    return Succeeded(result, std::move(inverse));
}

void MoveElementCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "after", _after }, { "indexHint", _indexHint } };
}

std::unique_ptr<SequenceCommand> MoveElementCommand::Clone() const {
    return std::make_unique<MoveElementCommand>(*this);
}

// ---------------------------------------------------------------------------
// RenameTimingTrack
// ---------------------------------------------------------------------------

RenameTimingTrackCommand::RenameTimingTrackCommand(SeqObjectId element, std::string name, bool updateReferences) :
    _element(element), _name(std::move(name)), _updateReferences(updateReferences) {
}

CommandResult RenameTimingTrackCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Element* el = TopLevelElement(seq, _element);
    if (el == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "timing track not found");
    }
    if (el->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
        return CommandResult::Fail(CommandStatus::Invalid, "not a timing track");
    }
    if (_name.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "empty name");
    }
    if (mode == ApplyMode::Strict && _name != el->GetName() && NameInUse(seq, _name)) {
        return CommandResult::Fail(CommandStatus::Invalid, "an element named " + _name + " is already in the sequence");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> RenameTimingTrackCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Element* el = TopLevelElement(seq, _element);
    std::string oldName = el->GetName();
    if (oldName == _name) {
        return Succeeded(result, std::make_unique<RenameTimingTrackCommand>(_element, oldName, false));
    }
    // Effects look the track up by name while rendering, so stop the render
    // before the name and the references to it change.
    if (RenderContext* rc = seq.GetRenderContext()) {
        rc->AbortRender();
    }
    el->SetName(_name);

    // Inverse: put the name back without touching references, then restore
    // exactly the settings this rename rewrote.
    std::vector<std::unique_ptr<SequenceCommand>> inverses;
    inverses.push_back(std::make_unique<RenameTimingTrackCommand>(_element, oldName, false));
    if (_updateReferences) {
        EffectManager& em = seq.GetEffectManager();
        for (Element* model : MasterElements(seq)) {
            if (model->GetType() != ElementType::ELEMENT_TYPE_MODEL) {
                continue;
            }
            ForEachLayer(model, [&](EffectLayer* layer) {
                for (Effect* eff : layer->GetEffects()) {
                    int idx = eff->GetEffectIndex();
                    if (idx < 0 || idx >= (int)em.size() || em[idx] == nullptr) {
                        continue;
                    }
                    SettingsMap before = eff->GetSettings();
                    em[idx]->RenameTimingTrack(oldName, _name, eff);
                    KeyChanges prior = Diff(eff->GetSettings(), before);
                    if (!prior.empty()) {
                        inverses.push_back(std::make_unique<ModifyEffectCommand>(LocatorOf(eff), std::nullopt, SettingsChange{ false, std::move(prior) },
                                                                                  SettingsChange{}));
                    }
                }
            });
        }
    }
    seq.PopulateRowInformation();
    if (inverses.size() == 1) {
        return Succeeded(result, std::move(inverses.front()));
    }
    // Settings first, then the name, mirroring the forward order backwards.
    std::reverse(inverses.begin(), inverses.end());
    return Succeeded(result, std::make_unique<BatchCommand>("Rename Timing Track", std::move(inverses)));
}

void RenameTimingTrackCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "name", _name }, { "updateReferences", _updateReferences } };
}

std::unique_ptr<SequenceCommand> RenameTimingTrackCommand::Clone() const {
    return std::make_unique<RenameTimingTrackCommand>(*this);
}

// ---------------------------------------------------------------------------
// SetTimingFixed
// ---------------------------------------------------------------------------

SetTimingFixedCommand::SetTimingFixedCommand(SeqObjectId element, int fixedMS) :
    _element(element), _fixed(fixedMS) {
}

CommandResult SetTimingFixedCommand::Validate(const SequenceElements& seq, ApplyMode) const {
    Element* el = TopLevelElement(seq, _element);
    if (el == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "timing track not found");
    }
    if (el->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
        return CommandResult::Fail(CommandStatus::Invalid, "not a timing track");
    }
    if (_fixed < 0) {
        return CommandResult::Fail(CommandStatus::Invalid, "negative interval");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetTimingFixedCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    auto* te = dynamic_cast<TimingElement*>(TopLevelElement(seq, _element));
    int prior = te->GetFixedTiming();
    te->SetFixedTiming(_fixed);
    return Succeeded(result, std::make_unique<SetTimingFixedCommand>(_element, prior));
}

void SetTimingFixedCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "fixed", _fixed } };
}

std::unique_ptr<SequenceCommand> SetTimingFixedCommand::Clone() const {
    return std::make_unique<SetTimingFixedCommand>(*this);
}

// ---------------------------------------------------------------------------
// SetTag
// ---------------------------------------------------------------------------

SetTagCommand::SetTagCommand(int tag, int positionMS) :
    _tag(tag), _position(positionMS) {
}

CommandResult SetTagCommand::Validate(const SequenceElements&, ApplyMode) const {
    if (_tag < 0 || _tag >= SequenceElements::TagCount) {
        return CommandResult::Fail(CommandStatus::Invalid, "no such tag");
    }
    if (_position < -1) {
        return CommandResult::Fail(CommandStatus::Invalid, "bad tag position");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetTagCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    int prior = seq.GetTagPosition(_tag);
    seq.SetTagPosition(_tag, _position);
    return Succeeded(result, std::make_unique<SetTagCommand>(_tag, prior));
}

void SetTagCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "tag", _tag }, { "position", _position } };
}

std::unique_ptr<SequenceCommand> SetTagCommand::Clone() const {
    return std::make_unique<SetTagCommand>(*this);
}

// ---------------------------------------------------------------------------
// SetSequenceFace
// ---------------------------------------------------------------------------

SetSequenceFaceCommand::SetSequenceFaceCommand(std::string name, std::optional<FaceDefinition> definition) :
    _name(std::move(name)), _definition(std::move(definition)) {
}

CommandResult SetSequenceFaceCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    if (_name.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "face has no name");
    }
    if (mode == ApplyMode::Strict && !_definition.has_value() && seq.GetSequenceFaces().GetFace(_name) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "face not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetSequenceFaceCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    SequenceFaces& faces = seq.GetSequenceFaces();
    std::optional<FaceDefinition> prior;
    if (const FaceDefinition* cur = faces.GetFace(_name)) {
        prior = *cur;
    }
    // Faces effects hold references into the face definitions while rendering.
    if (RenderContext* rc = seq.GetRenderContext()) {
        rc->AbortRender();
    }
    if (_definition.has_value()) {
        faces.SetFace(_name, *_definition);
    } else {
        faces.RemoveFace(_name);
    }
    return Succeeded(result, std::make_unique<SetSequenceFaceCommand>(_name, std::move(prior)));
}

void SetSequenceFaceCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "name", _name } };
    if (_definition.has_value()) {
        json def = json::object();
        for (const auto& [k, v] : *_definition) {
            def[k] = v;
        }
        out["definition"] = def;
    } else {
        out["definition"] = nullptr;
    }
}

std::unique_ptr<SequenceCommand> SetSequenceFaceCommand::Clone() const {
    return std::make_unique<SetSequenceFaceCommand>(*this);
}

// ---------------------------------------------------------------------------
// SetSongStructure
// ---------------------------------------------------------------------------

SetSongStructureCommand::SetSongStructureCommand(std::vector<SongStructureView> views) :
    _views(std::move(views)) {
}

CommandResult SetSongStructureCommand::Validate(const SequenceElements&, ApplyMode) const {
    for (const auto& v : _views) {
        for (const auto& r : v.regions) {
            if (r.endTimeMS < r.startTimeMS) {
                return CommandResult::Fail(CommandStatus::Invalid, "bad region times");
            }
        }
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetSongStructureCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    SongStructureManager& ssm = seq.GetSongStructureManager();
    auto inverse = std::make_unique<SetSongStructureCommand>(ssm.GetViews());
    ssm.ReplaceViews(_views);
    return Succeeded(result, std::move(inverse));
}

void SetSongStructureCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "views", WriteViews(_views) } };
}

std::unique_ptr<SequenceCommand> SetSongStructureCommand::Clone() const {
    return std::make_unique<SetSongStructureCommand>(*this);
}

// ---------------------------------------------------------------------------
// SetElementRenderDisabled
// ---------------------------------------------------------------------------

SetElementRenderDisabledCommand::SetElementRenderDisabledCommand(SeqObjectId element, bool disabled) :
    _element(element), _disabled(disabled) {
}

CommandResult SetElementRenderDisabledCommand::Validate(const SequenceElements& seq, ApplyMode) const {
    if (seq.GetElementFromUid(_element) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "element not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetElementRenderDisabledCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Element* el = seq.GetElementFromUid(_element);
    // IsRenderDisabled on a submodel also reports its model's flag; the
    // inverse restores this row's own flag.
    bool prior = el->IsRenderDisabledFlag();
    if (prior != _disabled) {
        el->SetRenderDisabled(_disabled);
        seq.IncrementChangeCount(el);
    }
    return Succeeded(result, std::make_unique<SetElementRenderDisabledCommand>(_element, prior));
}

void SetElementRenderDisabledCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "disabled", _disabled } };
}

std::unique_ptr<SequenceCommand> SetElementRenderDisabledCommand::Clone() const {
    return std::make_unique<SetElementRenderDisabledCommand>(*this);
}

// ---------------------------------------------------------------------------
// RenameModelElement
// ---------------------------------------------------------------------------

RenameModelElementCommand::RenameModelElementCommand(SeqObjectId element, std::string name) :
    _element(element), _name(std::move(name)) {
}

CommandResult RenameModelElementCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Element* el = TopLevelElement(seq, _element);
    if (el == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "model not found");
    }
    if (el->GetType() != ElementType::ELEMENT_TYPE_MODEL) {
        return CommandResult::Fail(CommandStatus::Invalid, "not a model");
    }
    if (_name.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "empty name");
    }
    if (mode == ApplyMode::Strict && _name != el->GetName() && NameInUse(seq, _name)) {
        return CommandResult::Fail(CommandStatus::Invalid, "an element named " + _name + " is already in the sequence");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> RenameModelElementCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Element* el = TopLevelElement(seq, _element);
    std::string prior = el->GetName();
    if (prior != _name) {
        // Render threads look rows up by name.
        if (RenderContext* rc = seq.GetRenderContext()) {
            rc->AbortRender();
        }
        // Views hold the same Element objects as the master view.
        el->SetName(_name);
        seq.IncrementChangeCount(el);
        seq.PopulateRowInformation();
    }
    return Succeeded(result, std::make_unique<RenameModelElementCommand>(_element, prior));
}

void RenameModelElementCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "name", _name } };
}

std::unique_ptr<SequenceCommand> RenameModelElementCommand::Clone() const {
    return std::make_unique<RenameModelElementCommand>(*this);
}

// ---------------------------------------------------------------------------
// Effect symbols
// ---------------------------------------------------------------------------

EffectSymbolDef EffectSymbolDef::Of(const EffectSymbol& s) {
    return { s.GetName(), s.GetEffectType(), s.GetSettingsAsString(), s.GetPalette() };
}

SetEffectSymbolCommand::SetEffectSymbolCommand(std::string id, std::optional<EffectSymbolDef> definition) :
    _id(std::move(id)), _definition(std::move(definition)) {
}

CommandResult SetEffectSymbolCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    if (_id.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "empty symbol id");
    }
    if (_definition.has_value()) {
        if (_definition->name.empty()) {
            return CommandResult::Fail(CommandStatus::Invalid, "empty symbol name");
        }
        const EffectSymbol* named = seq.GetEffectSymbolManager().GetSymbolByName(_definition->name);
        if (mode == ApplyMode::Strict && named != nullptr && named->GetId() != _id) {
            return CommandResult::Fail(CommandStatus::Invalid, "a symbol named " + _definition->name + " already exists");
        }
    } else if (mode == ApplyMode::Strict && !seq.GetEffectSymbolManager().SymbolExists(_id)) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "symbol not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetEffectSymbolCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    EffectSymbolManager& mgr = seq.GetEffectSymbolManager();
    std::optional<EffectSymbolDef> prior;
    if (const EffectSymbol* cur = mgr.GetSymbol(_id)) {
        prior = EffectSymbolDef::Of(*cur);
    }
    if (_definition.has_value()) {
        mgr.PutSymbol(_id, _definition->name, _definition->effectType, seq.GetEffectManager().GetEffectIndex(_definition->effectType),
                      _definition->settings, _definition->palette);
        seq.IncrementChangeCount(nullptr);
        return Succeeded(result, std::make_unique<SetEffectSymbolCommand>(_id, std::move(prior)));
    }
    if (!prior.has_value()) {
        return Succeeded(result, std::make_unique<SetEffectSymbolCommand>(_id, std::nullopt));
    }
    std::vector<std::unique_ptr<SequenceCommand>> inverses;
    inverses.push_back(std::make_unique<SetEffectSymbolCommand>(_id, prior));
    for (Effect* e : mgr.GetLinkedEffects(_id)) {
        EffectLocator loc = LocatorOf(e);
        if (loc.effect != kNoSeqObjectId) {
            inverses.push_back(std::make_unique<SetEffectSymbolLinkCommand>(loc, _id));
        }
    }
    mgr.DeleteSymbol(_id); // unlinks the effects
    seq.IncrementChangeCount(nullptr);
    if (inverses.size() == 1) {
        return Succeeded(result, std::move(inverses.front()));
    }
    return Succeeded(result, std::make_unique<BatchCommand>("Remove Symbol", std::move(inverses)));
}

void SetEffectSymbolCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "id", _id } };
    if (_definition.has_value()) {
        out["definition"] = json{ { "name", _definition->name },
                                  { "effectType", _definition->effectType },
                                  { "settings", _definition->settings },
                                  { "palette", _definition->palette } };
    } else {
        out["definition"] = nullptr;
    }
}

std::unique_ptr<SequenceCommand> SetEffectSymbolCommand::Clone() const {
    return std::make_unique<SetEffectSymbolCommand>(*this);
}

SetEffectSymbolLinkCommand::SetEffectSymbolLinkCommand(EffectLocator target, std::string symbolId) :
    _target(target), _symbolId(std::move(symbolId)) {
}

CommandResult SetEffectSymbolLinkCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    if (FindEffect(seq, _target) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "effect not found");
    }
    if (!_symbolId.empty() && !seq.GetEffectSymbolManager().SymbolExists(_symbolId)) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "symbol not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetEffectSymbolLinkCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Effect* e = FindEffect(seq, _target);
    std::string prior = e->GetLinkedSymbolId();
    if (prior != _symbolId) {
        e->SetLinkedSymbolId(_symbolId);
        seq.IncrementChangeCount(e->GetParentEffectLayer()->GetParentElement());
    }
    return Succeeded(result, std::make_unique<SetEffectSymbolLinkCommand>(_target, prior));
}

void SetEffectSymbolLinkCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) },
                { "target", json{ { "element", _target.element }, { "layer", _target.layer }, { "effect", _target.effect } } },
                { "symbol", _symbolId } };
}

std::unique_ptr<SequenceCommand> SetEffectSymbolLinkCommand::Clone() const {
    return std::make_unique<SetEffectSymbolLinkCommand>(*this);
}

// ---------------------------------------------------------------------------
// Names and deserialization
// ---------------------------------------------------------------------------

const char* ElementCommandTypeName(CommandType t) {
    switch (t) {
    case CommandType::AddElement: return "AddElement";
    case CommandType::DeleteElement: return "DeleteElement";
    case CommandType::MoveElement: return "MoveElement";
    case CommandType::RenameTimingTrack: return "RenameTimingTrack";
    case CommandType::SetTimingFixed: return "SetTimingFixed";
    case CommandType::SetTag: return "SetTag";
    case CommandType::SetSequenceFace: return "SetSequenceFace";
    case CommandType::SetSongStructure: return "SetSongStructure";
    case CommandType::SetElementRenderDisabled: return "SetElementRenderDisabled";
    case CommandType::RenameModelElement: return "RenameModelElement";
    case CommandType::SetEffectSymbol: return "SetEffectSymbol";
    case CommandType::SetEffectSymbolLink: return "SetEffectSymbolLink";
    default: return "";
    }
}

const char* ElementCommandUndoLabel(CommandType t) {
    switch (t) {
    case CommandType::AddElement: return "Row(s) Added";
    case CommandType::DeleteElement: return "Row(s) Deleted";
    case CommandType::MoveElement: return "Row(s) Moved";
    case CommandType::RenameTimingTrack: return "Timing Track Renamed";
    case CommandType::SetTimingFixed: return "Timing Track Changed";
    case CommandType::SetTag: return "Tag Changed";
    case CommandType::SetSequenceFace: return "Sequence Faces Changed";
    case CommandType::SetSongStructure: return "Song Structure Changed";
    case CommandType::SetElementRenderDisabled: return "Render Disabled Changed";
    case CommandType::RenameModelElement: return "Model Renamed";
    case CommandType::SetEffectSymbol: return "Symbol Changed";
    case CommandType::SetEffectSymbolLink: return "Symbol Link Changed";
    default: return "";
    }
}

std::unique_ptr<SequenceCommand> DeserializeElementCommand(const json& in) {
    std::string type;
    if (!in.is_object() || !ReadString(in, "type", type)) {
        return nullptr;
    }
    if (type == "AddElement") {
        ElementSnapshot s;
        uint64_t after = 0;
        int hint = -1;
        auto it = in.find("element");
        if (it == in.end() || !ReadSnapshot(*it, s) || !ReadU64(in, "after", after) || !ReadInt(in, "indexHint", hint)) {
            return nullptr;
        }
        return std::make_unique<AddElementCommand>(std::move(s), after, hint);
    }
    if (type == "DeleteElement") {
        uint64_t element = 0;
        bool keep = false;
        if (!ReadU64(in, "element", element) || !ReadBool(in, "keepInViews", keep)) {
            return nullptr;
        }
        return std::make_unique<DeleteElementCommand>(element, keep);
    }
    if (type == "MoveElement") {
        uint64_t element = 0, after = 0;
        int hint = -1;
        if (!ReadU64(in, "element", element) || !ReadU64(in, "after", after) || !ReadInt(in, "indexHint", hint)) {
            return nullptr;
        }
        return std::make_unique<MoveElementCommand>(element, after, hint);
    }
    if (type == "RenameTimingTrack") {
        uint64_t element = 0;
        std::string name;
        bool refs = true;
        if (!ReadU64(in, "element", element) || !ReadString(in, "name", name) || !ReadBool(in, "updateReferences", refs)) {
            return nullptr;
        }
        return std::make_unique<RenameTimingTrackCommand>(element, std::move(name), refs);
    }
    if (type == "SetTimingFixed") {
        uint64_t element = 0;
        int fixed = 0;
        if (!ReadU64(in, "element", element) || !ReadInt(in, "fixed", fixed)) {
            return nullptr;
        }
        return std::make_unique<SetTimingFixedCommand>(element, fixed);
    }
    if (type == "SetTag") {
        int tag = 0, pos = 0;
        if (!ReadInt(in, "tag", tag) || !ReadInt(in, "position", pos)) {
            return nullptr;
        }
        return std::make_unique<SetTagCommand>(tag, pos);
    }
    if (type == "SetSequenceFace") {
        std::string name;
        auto it = in.find("definition");
        if (!ReadString(in, "name", name) || it == in.end()) {
            return nullptr;
        }
        std::optional<SetSequenceFaceCommand::FaceDefinition> def;
        if (it->is_object()) {
            SetSequenceFaceCommand::FaceDefinition d;
            for (auto kv = it->begin(); kv != it->end(); ++kv) {
                if (!kv->is_string()) {
                    return nullptr;
                }
                d[kv.key()] = kv->get<std::string>();
            }
            def = std::move(d);
        } else if (!it->is_null()) {
            return nullptr;
        }
        return std::make_unique<SetSequenceFaceCommand>(std::move(name), std::move(def));
    }
    if (type == "SetElementRenderDisabled") {
        uint64_t element = 0;
        bool disabled = false;
        if (!ReadU64(in, "element", element) || !ReadBool(in, "disabled", disabled)) {
            return nullptr;
        }
        return std::make_unique<SetElementRenderDisabledCommand>(element, disabled);
    }
    if (type == "RenameModelElement") {
        uint64_t element = 0;
        std::string name;
        if (!ReadU64(in, "element", element) || !ReadString(in, "name", name)) {
            return nullptr;
        }
        return std::make_unique<RenameModelElementCommand>(element, std::move(name));
    }
    if (type == "SetEffectSymbol") {
        std::string id;
        auto it = in.find("definition");
        if (!ReadString(in, "id", id) || it == in.end()) {
            return nullptr;
        }
        std::optional<EffectSymbolDef> def;
        if (it->is_object()) {
            EffectSymbolDef d;
            if (!ReadString(*it, "name", d.name) || !ReadString(*it, "effectType", d.effectType) || !ReadString(*it, "settings", d.settings) ||
                !ReadString(*it, "palette", d.palette)) {
                return nullptr;
            }
            def = std::move(d);
        } else if (!it->is_null()) {
            return nullptr;
        }
        return std::make_unique<SetEffectSymbolCommand>(std::move(id), std::move(def));
    }
    if (type == "SetEffectSymbolLink") {
        auto it = in.find("target");
        EffectLocator t;
        std::string symbol;
        if (it == in.end() || !it->is_object() || !ReadU64(*it, "element", t.element) || !ReadU64(*it, "layer", t.layer) ||
            !ReadU64(*it, "effect", t.effect) || !ReadString(in, "symbol", symbol)) {
            return nullptr;
        }
        return std::make_unique<SetEffectSymbolLinkCommand>(t, std::move(symbol));
    }
    if (type == "SetSongStructure") {
        std::vector<SongStructureView> views;
        if (!ReadViews(ReadArray(in, "views"), views)) {
            return nullptr;
        }
        return std::make_unique<SetSongStructureCommand>(std::move(views));
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// NewElementsRecorder
// ---------------------------------------------------------------------------

namespace {
void ForEachElementAndRow(const SequenceElements& seq, const std::function<void(Element*)>& fn) {
    for (Element* el : MasterElements(seq)) {
        fn(el);
        if (auto* me = dynamic_cast<ModelElement*>(el)) {
            for (int x = 0; x < me->GetSubModelAndStrandCount(); ++x) {
                if (SubModelElement* sme = me->GetSubModel(x)) {
                    fn(sme);
                }
            }
        }
    }
}
} // namespace

#ifndef NDEBUG
namespace {
// The rows the commands recorded in a NewElementsRecorder's scope changed,
// so it can tell an existing row changed by a command (recorded) from one
// changed directly (not recorded). It listens as commands are recorded, not
// for the batch notification, which an enclosing batch or preview holds back.
class TouchedRows : public ICommandObserver {
public:
    explicit TouchedRows(const SequenceElements& seq) : _seq(seq) {}
    void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand& inverse) override {
        Note(applied);
        Note(inverse);
    }
    std::set<SeqObjectId> rows;

private:
    void Row(SeqObjectId uid) {
        rows.insert(uid);
        // A submodel or strand row's edits move its model row's change count.
        if (auto* sub = dynamic_cast<SubModelElement*>(_seq.GetElementFromUid(uid))) {
            rows.insert(sub->GetModelElement()->GetUid());
        }
    }
    void Note(const SequenceCommand& cmd) {
        switch (cmd.Type()) {
        case CommandType::Batch:
            for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
                Note(*m);
            }
            break;
        case CommandType::AddEffect: Row(static_cast<const AddEffectCommand&>(cmd).GetLayer().element); break;
        case CommandType::DeleteEffect: Row(static_cast<const DeleteEffectCommand&>(cmd).Target().element); break;
        case CommandType::MoveEffect: Row(static_cast<const MoveEffectCommand&>(cmd).Target().element); break;
        case CommandType::ModifyEffect: Row(static_cast<const ModifyEffectCommand&>(cmd).Target().element); break;
        case CommandType::AddLayer: Row(static_cast<const AddLayerCommand&>(cmd).GetElement()); break;
        case CommandType::RemoveLayer: Row(static_cast<const RemoveLayerCommand&>(cmd).GetElement()); break;
        case CommandType::SetLayerName: Row(static_cast<const SetLayerNameCommand&>(cmd).GetLayer().element); break;
        case CommandType::ReplaceLayerContents: Row(static_cast<const ReplaceLayerContentsCommand&>(cmd).GetLayer().element); break;
        case CommandType::SetEffectSymbolLink: Row(static_cast<const SetEffectSymbolLinkCommand&>(cmd).GetTarget().element); break;
        case CommandType::AddElement: Row(static_cast<const AddElementCommand&>(cmd).GetUid()); break;
        case CommandType::DeleteElement: Row(static_cast<const DeleteElementCommand&>(cmd).GetElement()); break;
        case CommandType::MoveElement: Row(static_cast<const MoveElementCommand&>(cmd).GetElement()); break;
        case CommandType::RenameTimingTrack: Row(static_cast<const RenameTimingTrackCommand&>(cmd).GetElement()); break;
        case CommandType::SetTimingFixed: Row(static_cast<const SetTimingFixedCommand&>(cmd).GetElement()); break;
        case CommandType::SetElementRenderDisabled: Row(static_cast<const SetElementRenderDisabledCommand&>(cmd).GetElement()); break;
        case CommandType::RenameModelElement: Row(static_cast<const RenameModelElementCommand&>(cmd).GetElement()); break;
        default: break;
        }
    }
    const SequenceElements& _seq;
};
} // namespace
#endif

NewElementsRecorder::NewElementsRecorder(SequenceElements& seq, std::string label, bool recordUndo) :
    _seq(seq), _label(std::move(label)), _recordUndo(recordUndo),
    // An open SequenceEditCapture records new elements itself.
    _passive(Active(seq) != nullptr || seq.GetCommandExecutor().IsCapturing()), _outer(t_recorder) {
    if (_passive) {
        return;
    }
    ForEachElementAndRow(_seq, [this](Element* el) { _before[el->GetUid()] = el->getChangeCount(); });
#ifndef NDEBUG
    _touched = std::make_unique<TouchedRows>(_seq);
    _seq.GetCommandExecutor().AddRecordListener(_touched.get());
#endif
    t_recorder = this;
    // Anything executed as a command inside the scope joins the same step.
    _seq.GetCommandExecutor().BeginBatch(_label);
    _scope.emplace();
}

NewElementsRecorder::~NewElementsRecorder() {
    if (_passive) {
        return;
    }
    _scope.reset();
    t_recorder = _outer;
    CommandExecutor& exec = _seq.GetCommandExecutor();
    for (Element* el : MasterElements(_seq)) {
        if (_before.count(el->GetUid())) {
            continue;
        }
        int idx = MasterIndexOf(_seq, el->GetUid());
        exec.RecordExternal(std::make_unique<AddElementCommand>(ElementSnapshot::Of(*el), PrecedingUid(_seq, idx), idx),
                            std::make_unique<DeleteElementCommand>(el->GetUid()), _recordUndo);
    }
    exec.EndBatch();
#ifndef NDEBUG
    exec.RemoveRecordListener(_touched.get());
    const std::set<SeqObjectId>& touched = static_cast<TouchedRows&>(*_touched).rows;
    // The scope above exempts everything, so a direct change to a row that
    // already existed (rather than one this recorder creates) would go
    // unrecorded and unnoticed: report it like any other stray mutation.
    ForEachElementAndRow(_seq, [&](Element* el) {
        auto it = _before.find(el->GetUid());
        if (it != _before.end() && it->second != el->getChangeCount() && !touched.count(el->GetUid())) {
            spdlog::warn("{}: existing row {} changed directly while recording new elements; that change is not recorded", _label, el->GetFullName());
            SeqMutationGuard::Check("NewElementsRecorder: existing row changed directly", nullptr);
        }
    });
#endif
}

NewElementsRecorder* NewElementsRecorder::Active(const SequenceElements& seq) {
    for (NewElementsRecorder* r = t_recorder; r != nullptr; r = r->_outer) {
        if (&r->_seq == &seq) {
            return r;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// ElementEdits
// ---------------------------------------------------------------------------

namespace ElementEdits {

namespace {
SeqObjectId AfterForIndex(const SequenceElements& seq, int index) {
    int count = (int)seq.GetElementCount(MASTER_VIEW);
    if (index < 0 || index > count) {
        index = count;
    }
    return PrecedingUid(seq, index);
}

Element* AddNew(SequenceElements& seq, int index, ElementSnapshot state, bool recordUndo) {
    int count = (int)seq.GetElementCount(MASTER_VIEW);
    if (index < 0 || index > count) {
        index = count;
    }
    if (NewElementsRecorder::Active(seq) != nullptr) {
        // The recorder records the finished element.
        Element* el = seq.AddElement(index, state.name, state.timing ? "timing" : "model", true, false, false, false, false);
        if (el != nullptr) {
            if (auto* te = dynamic_cast<TimingElement*>(el)) {
                te->SetSubType(state.subType);
            }
            el->AddEffectLayer();
        }
        return el;
    }
    auto cmd = std::make_unique<AddElementCommand>(std::move(state), AfterForIndex(seq, index), index);
    SeqObjectId uid = cmd->GetUid();
    if (!seq.GetCommandExecutor().ExecuteInStep(std::move(cmd), recordUndo).Ok()) {
        return nullptr;
    }
    return seq.GetElementFromUid(uid);
}
} // namespace

TimingElement* AddTimingTrack(SequenceElements& seq, int index, const std::string& name, const std::string& subType, bool recordUndo) {
    if (index < 0) {
        index = seq.GetNumberOfTimingElements();
    }
    ElementSnapshot s;
    s.timing = true;
    s.name = name;
    s.subType = subType;
    return dynamic_cast<TimingElement*>(AddNew(seq, index, std::move(s), recordUndo));
}

Element* AddModel(SequenceElements& seq, int index, const std::string& name, bool recordUndo) {
    ElementSnapshot s;
    s.name = name;
    return AddNew(seq, index, std::move(s), recordUndo);
}

bool DeleteElement(SequenceElements& seq, const std::string& name, bool recordUndo, bool keepInViews) {
    for (Element* el : MasterElements(seq)) {
        if (el->GetName() == name) {
            return seq.GetCommandExecutor().ExecuteInStep(std::make_unique<DeleteElementCommand>(el->GetUid(), keepInViews), recordUndo).Ok();
        }
    }
    return false;
}

bool MoveElement(SequenceElements& seq, int index, int dest, int view, bool recordUndo) {
    if (view != MASTER_VIEW) {
        SeqMutationGuard::Scope perUserView;
        seq.MoveSequenceElement(index, dest, view);
        return true;
    }
    auto cmd = MoveElementCommand::FromIndices(seq, index, dest);
    if (!cmd) {
        return false;
    }
    return seq.GetCommandExecutor().ExecuteInStep(std::move(cmd), recordUndo).Ok();
}

bool RenameTimingTrack(SequenceElements& seq, const std::string& oldName, const std::string& newName, bool recordUndo) {
    for (Element* el : MasterElements(seq)) {
        if (el->GetName() == oldName && el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            return seq.GetCommandExecutor().ExecuteInStep(std::make_unique<RenameTimingTrackCommand>(el->GetUid(), newName), recordUndo).Ok();
        }
    }
    return false;
}

bool SetTimingFixed(SequenceElements& seq, TimingElement* timing, int fixedMS, bool recordUndo) {
    if (timing == nullptr) {
        return false;
    }
    if (TopLevelElement(seq, timing->GetUid()) != timing) {
        SeqMutationGuard::Scope notInSequence;
        timing->SetFixedTiming(fixedMS);
        return true;
    }
    if (timing->GetFixedTiming() == fixedMS) {
        return true;
    }
    return seq.GetCommandExecutor().ExecuteInStep(std::make_unique<SetTimingFixedCommand>(timing->GetUid(), fixedMS), recordUndo).Ok();
}

bool SetTagPosition(SequenceElements& seq, int tag, int positionMS, bool recordUndo) {
    if (seq.GetTagPosition(tag) == positionMS) {
        return true;
    }
    return seq.GetCommandExecutor().ExecuteInStep(std::make_unique<SetTagCommand>(tag, positionMS), recordUndo).Ok();
}

bool EditSequenceFaces(SequenceElements& seq, const std::function<void(SequenceFaces&)>& edit, bool recordUndo) {
    SequenceFaces& faces = seq.GetSequenceFaces();
    SequenceFaces::FaceMap before = faces.GetFaces();
    {
        SeqMutationGuard::Scope recorded;
        edit(faces);
    }
    const SequenceFaces::FaceMap& after = faces.GetFaces();
    CommandExecutor& exec = seq.GetCommandExecutor();
    bool changed = false;
    exec.BeginBatch("Sequence Faces");
    for (const auto& [name, def] : before) {
        if (after.find(name) == after.end()) {
            exec.RecordExternal(std::make_unique<SetSequenceFaceCommand>(name, std::nullopt),
                                std::make_unique<SetSequenceFaceCommand>(name, def), recordUndo);
            changed = true;
        }
    }
    for (const auto& [name, def] : after) {
        auto it = before.find(name);
        if (it == before.end() || it->second != def) {
            std::optional<SetSequenceFaceCommand::FaceDefinition> prior;
            if (it != before.end()) {
                prior = it->second;
            }
            exec.RecordExternal(std::make_unique<SetSequenceFaceCommand>(name, def),
                                std::make_unique<SetSequenceFaceCommand>(name, std::move(prior)), recordUndo);
            changed = true;
        }
    }
    exec.EndBatch();
    return changed;
}

bool EditSongStructure(SequenceElements& seq, const std::function<void(SongStructureManager&)>& edit, bool recordUndo) {
    SongStructureManager& ssm = seq.GetSongStructureManager();
    std::vector<SongStructureView> before = ssm.GetViews();
    {
        SeqMutationGuard::Scope recorded;
        edit(ssm);
    }
    if (SameViews(before, ssm.GetViews())) {
        return false;
    }
    seq.GetCommandExecutor().RecordExternal(std::make_unique<SetSongStructureCommand>(ssm.GetViews()),
                                            std::make_unique<SetSongStructureCommand>(std::move(before)), recordUndo);
    return true;
}

bool SetRenderDisabled(SequenceElements& seq, Element* element, bool disabled, bool recordUndo) {
    if (element == nullptr) {
        return false;
    }
    if (seq.GetElementFromUid(element->GetUid()) != element) {
        SeqMutationGuard::Scope notInSequence;
        element->SetRenderDisabled(disabled);
        return true;
    }
    if (element->IsRenderDisabledFlag() == disabled) {
        return false;
    }
    return seq.GetCommandExecutor().ExecuteInStep(std::make_unique<SetElementRenderDisabledCommand>(element->GetUid(), disabled), recordUndo).Ok();
}

bool RenameModel(SequenceElements& seq, const std::string& oldName, const std::string& newName, bool recordUndo) {
    if (oldName == newName) {
        return false;
    }
    for (Element* el : MasterElements(seq)) {
        if (el->GetName() == oldName && el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            return seq.GetCommandExecutor().ExecuteInStep(std::make_unique<RenameModelElementCommand>(el->GetUid(), newName), recordUndo).Ok();
        }
    }
    return false;
}

} // namespace ElementEdits

// ---------------------------------------------------------------------------
// SongStructureLiveEdit
// ---------------------------------------------------------------------------

SongStructureLiveEdit::SongStructureLiveEdit(SequenceElements& seq) :
    _seq(seq), _before(seq.GetSongStructureManager().GetViews()) {
}

SongStructureLiveEdit::~SongStructureLiveEdit() {
    Cancel();
}

void SongStructureLiveEdit::Apply(const std::function<void(SongStructureManager&)>& edit) {
    if (!_open) {
        return;
    }
    SeqMutationGuard::Scope livePreview;
    edit(_seq.GetSongStructureManager());
}

bool SongStructureLiveEdit::Commit(const std::string& label, bool recordUndo) {
    if (!_open) {
        return false;
    }
    _open = false;
    SongStructureManager& ssm = _seq.GetSongStructureManager();
    std::vector<SongStructureView> after = ssm.GetViews();
    {
        SeqMutationGuard::Scope livePreview;
        ssm.ReplaceViews(std::move(_before));
    }
    ScopedCommandBatch step(_seq, label);
    return ElementEdits::EditSongStructure(_seq, [&](SongStructureManager& m) { m.ReplaceViews(std::move(after)); }, recordUndo);
}

void SongStructureLiveEdit::Cancel() {
    if (!_open) {
        return;
    }
    _open = false;
    SeqMutationGuard::Scope livePreview;
    _seq.GetSongStructureManager().ReplaceViews(std::move(_before));
}

// ---------------------------------------------------------------------------
// SymbolEdits
// ---------------------------------------------------------------------------

namespace SymbolEdits {

namespace {
bool InSeq(const SequenceElements& seq, const Effect* e) {
    return e != nullptr && FindEffect(seq, LocatorOf(e)) == e;
}

// Gives `e` the symbol's type, settings and palette, recorded as a
// ModifyEffect. Unlinked first so the change stays on this effect.
void TakeSymbolSettings(SequenceElements& seq, Effect* e, const EffectSymbol& sym, bool recordUndo) {
    CommandExecutor& exec = seq.GetCommandExecutor();
    exec.EditEffect(e, [&](Effect& ef) {
        Effect::ScopedSymbolPropagationSuppressor noFanOut;
        ef.SetEffectName(sym.GetEffectType());
        ef.SetSettings(sym.GetSettingsAsString(), true);
        ef.SetPalette(sym.GetPalette());
    }, recordUndo);
}
} // namespace

std::string Create(SequenceElements& seq, const std::string& name, const Effect* source, bool link, const std::string& label) {
    EffectSymbolManager& mgr = seq.GetEffectSymbolManager();
    if (name.empty() || source == nullptr || mgr.SymbolNameExists(name)) {
        return "";
    }
    EffectSymbol probe("", name);
    probe.CopyFromEffect(source);
    std::string id = mgr.NewSymbolId();
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, label);
    if (!exec.ExecuteInStep(std::make_unique<SetEffectSymbolCommand>(id, EffectSymbolDef::Of(probe))).Ok()) {
        return "";
    }
    if (link && InSeq(seq, source)) {
        exec.ExecuteInStep(std::make_unique<SetEffectSymbolLinkCommand>(LocatorOf(source), id));
    }
    return id;
}

int Link(SequenceElements& seq, const std::vector<Effect*>& effects, const std::string& symbolId, bool recordUndo, const std::string& label) {
    const EffectSymbol* sym = seq.GetEffectSymbolManager().GetSymbol(symbolId);
    if (sym == nullptr) {
        return 0;
    }
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, label);
    int n = 0;
    for (Effect* e : effects) {
        if (!InSeq(seq, e)) {
            continue;
        }
        EffectLocator loc = LocatorOf(e);
        if (e->IsLinkedToSymbol()) {
            exec.ExecuteInStep(std::make_unique<SetEffectSymbolLinkCommand>(loc, ""), recordUndo);
        }
        TakeSymbolSettings(seq, e, *sym, recordUndo);
        if (exec.ExecuteInStep(std::make_unique<SetEffectSymbolLinkCommand>(loc, symbolId), recordUndo).Ok()) {
            ++n;
        }
    }
    return n;
}

int Unlink(SequenceElements& seq, const std::vector<Effect*>& effects, bool recordUndo, const std::string& label) {
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, label);
    int n = 0;
    for (Effect* e : effects) {
        if (InSeq(seq, e) && e->IsLinkedToSymbol() &&
            exec.ExecuteInStep(std::make_unique<SetEffectSymbolLinkCommand>(LocatorOf(e), ""), recordUndo).Ok()) {
            ++n;
        }
    }
    return n;
}

bool Rename(SequenceElements& seq, const std::string& symbolId, const std::string& name) {
    const EffectSymbol* sym = seq.GetEffectSymbolManager().GetSymbol(symbolId);
    if (sym == nullptr || name.empty() || name == sym->GetName()) {
        return false;
    }
    EffectSymbolDef def = EffectSymbolDef::Of(*sym);
    def.name = name;
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, "Rename Symbol");
    return exec.Execute(std::make_unique<SetEffectSymbolCommand>(symbolId, std::move(def))).Ok();
}

bool Delete(SequenceElements& seq, const std::string& symbolId) {
    if (!seq.GetEffectSymbolManager().SymbolExists(symbolId)) {
        return false;
    }
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, "Delete Symbol");
    return exec.ExecuteInStep(std::make_unique<SetEffectSymbolCommand>(symbolId, std::nullopt)).Ok();
}

std::pair<int, int> ConvertAllToEffects(SequenceElements& seq) {
    EffectSymbolManager& mgr = seq.GetEffectSymbolManager();
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, "Convert Symbols to Effects");
    int effects = 0, symbols = 0;
    for (EffectSymbol* sym : mgr.GetAllSymbols()) {
        std::string id = sym->GetId();
        effects += (int)mgr.GetLinkedEffectCount(id);
        if (exec.ExecuteInStep(std::make_unique<SetEffectSymbolCommand>(id, std::nullopt)).Ok()) {
            ++symbols;
        }
    }
    return { effects, symbols };
}

bool UpdateFromEffect(SequenceElements& seq, const std::string& symbolId, const Effect* source, const std::string& label) {
    EffectSymbolManager& mgr = seq.GetEffectSymbolManager();
    const EffectSymbol* sym = mgr.GetSymbol(symbolId);
    if (sym == nullptr || source == nullptr) {
        return false;
    }
    EffectSymbol probe(symbolId, sym->GetName());
    probe.CopyFromEffect(source);
    CommandExecutor& exec = seq.GetCommandExecutor();
    ScopedCommandBatch batch(exec, label);
    if (!exec.ExecuteInStep(std::make_unique<SetEffectSymbolCommand>(symbolId, EffectSymbolDef::Of(probe))).Ok()) {
        return false;
    }
    for (Effect* e : mgr.GetLinkedEffects(symbolId)) {
        if (e != source && InSeq(seq, e)) {
            TakeSymbolSettings(seq, e, probe, true);
        }
    }
    return true;
}

} // namespace SymbolEdits


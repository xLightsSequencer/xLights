/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceCommands.h"
#include "SequenceElementCommands.h"
#include "SequenceMutationGuard.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <set>
#include <sstream>
#include <utility>

#if !defined(NDEBUG) && (defined(__APPLE__) || defined(__linux__))
#include <execinfo.h>
#define XL_HAVE_EXECINFO 1
#endif

#include <log.h>

#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "SequenceMediaCommands.h"
#include "../effects/EffectManager.h"
#include "../utils/UtilClasses.h"

using nlohmann::json;

// ---------------------------------------------------------------------------
// Mutation guard
// ---------------------------------------------------------------------------

#ifndef NDEBUG
namespace SeqMutationGuard {
namespace {
thread_local int t_depth = 0;
std::atomic<size_t> s_violations{ 0 };

Mode InitialMode() {
    if (const char* env = std::getenv("XL_COMMAND_GUARD")) {
        if (std::strcmp(env, "off") == 0) return Mode::Off;
        if (std::strcmp(env, "count") == 0) return Mode::Count;
        if (std::strcmp(env, "assert") == 0) return Mode::Assert;
        if (std::strcmp(env, "log") == 0) return Mode::Log;
    }
#if defined(DEBUG) || defined(_DEBUG)
    return Mode::Log;
#else
    return Mode::Off;
#endif
}

std::atomic<Mode>& CurrentMode() {
    static std::atomic<Mode> mode{ InitialMode() };
    return mode;
}
} // namespace

Scope::Scope() { ++t_depth; }
Scope::~Scope() { --t_depth; }

namespace {
thread_local int t_effectEditDepth = 0;

bool IsEffectOrLayerEdit(const char* what) {
    return std::strncmp(what, "Effect::", 8) == 0 || std::strncmp(what, "EffectLayer::", 13) == 0 ||
           std::strcmp(what, "Element::AddEffectLayer") == 0 || std::strcmp(what, "Element::InsertEffectLayer") == 0 ||
           std::strcmp(what, "Element::RemoveEffectLayer") == 0;
}
} // namespace

EffectEditScope::EffectEditScope() { ++t_effectEditDepth; }
EffectEditScope::~EffectEditScope() { --t_effectEditDepth; }

void SetMode(Mode m) { CurrentMode().store(m); }
Mode GetMode() { return CurrentMode().load(); }
size_t ViolationCount() { return s_violations.load(); }
void ResetViolationCount() { s_violations.store(0); }

void Check(const char* what, const void* caller) {
    if (t_depth > 0 || (t_effectEditDepth > 0 && IsEffectOrLayerEdit(what))) {
        return;
    }
    Mode mode = CurrentMode().load(std::memory_order_relaxed);
    if (mode == Mode::Off) {
        return;
    }
    ++s_violations;
    if (mode == Mode::Assert) {
        spdlog::critical("Sequence mutated outside a command: {}", what);
        assert(false && "sequence mutated outside a SequenceCommand");
        return;
    }
    if (mode != Mode::Log) {
        return;
    }
    static std::mutex lock;
    static std::set<std::pair<std::string, const void*>> seen;
    {
        std::lock_guard<std::mutex> l(lock);
        if (!seen.emplace(what, caller).second) {
            return;
        }
    }
    std::string where;
#ifdef XL_HAVE_EXECINFO
    void* frames[8];
    int n = backtrace(frames, 8);
    if (char** syms = backtrace_symbols(frames, n)) {
        for (int i = 1; i < n; ++i) {
            where += "\n    ";
            where += syms[i];
        }
        free(syms);
    }
#endif
    spdlog::debug("Sequence mutated outside a command: {}{}", what, where);
}
} // namespace SeqMutationGuard
#endif

const char* CommandTypeName(CommandType t) {
    switch (t) {
    case CommandType::AddEffect: return "AddEffect";
    case CommandType::DeleteEffect: return "DeleteEffect";
    case CommandType::MoveEffect: return "MoveEffect";
    case CommandType::ModifyEffect: return "ModifyEffect";
    case CommandType::AddLayer: return "AddLayer";
    case CommandType::RemoveLayer: return "RemoveLayer";
    case CommandType::SetLayerName: return "SetLayerName";
    case CommandType::ReplaceLayerContents: return "ReplaceLayerContents";
    case CommandType::Batch: return "Batch";
    case CommandType::PutMedia: return "PutMedia";
    case CommandType::RemoveMedia: return "RemoveMedia";
    case CommandType::RenameMedia: return "RenameMedia";
    case CommandType::AddElement:
    case CommandType::DeleteElement:
    case CommandType::MoveElement:
    case CommandType::RenameTimingTrack:
    case CommandType::SetTimingFixed:
    case CommandType::SetTag:
    case CommandType::SetSequenceFace:
    case CommandType::SetSongStructure: return ElementCommandTypeName(t);
    }
    return "";
}

// ---------------------------------------------------------------------------
// Lookups
// ---------------------------------------------------------------------------

EffectLayer* FindLayer(const SequenceElements& seq, const LayerLocator& loc) {
    Element* e = seq.GetElementFromUid(loc.element);
    return e == nullptr ? nullptr : e->GetEffectLayerFromUid(loc.layer);
}

Effect* FindEffect(const SequenceElements& seq, const EffectLocator& loc) {
    EffectLayer* l = FindLayer(seq, loc.Layer());
    return l == nullptr ? nullptr : l->GetEffectFromUid(loc.effect);
}

LayerLocator LocatorOf(const EffectLayer* layer) {
    if (layer == nullptr || layer->GetParentElement() == nullptr) {
        return {};
    }
    return { layer->GetParentElement()->GetUid(), layer->GetUid() };
}

EffectLocator LocatorOf(const Effect* effect) {
    if (effect == nullptr) {
        return {};
    }
    LayerLocator l = LocatorOf(effect->GetParentEffectLayer());
    if (l.layer == kNoSeqObjectId) {
        return {};
    }
    return { l.element, l.layer, effect->GetUid() };
}

namespace {

int IndexOfLayer(const Element* element, SeqObjectId layer) {
    const auto& layers = element->GetEffectLayers();
    for (size_t i = 0; i < layers.size(); ++i) {
        if (layers[i]->GetUid() == layer) {
            return (int)i;
        }
    }
    return -1;
}

bool RangeClear(const EffectLayer* layer, int start, int end, SeqObjectId ignore) {
    for (const Effect* e : layer->GetEffects()) {
        if (e->GetUid() != ignore && e->OverlapsWith(start, end)) {
            return false;
        }
    }
    return true;
}

bool HasLockedEffect(const EffectLayer* layer) {
    for (const Effect* e : layer->GetEffects()) {
        if (e->IsLocked()) {
            return true;
        }
    }
    return false;
}

// The same checks EffectLayer::AddEffect makes before creating an effect.
CommandResult CheckNewEffect(const SequenceElements& seq, const EffectLayer* layer, const EffectState& s) {
    if (s.uid == kNoSeqObjectId) {
        return CommandResult::Fail(CommandStatus::Invalid, "effect has no id");
    }
    if (s.startTimeMS > s.endTimeMS || (s.startTimeMS < 0 && s.endTimeMS <= 0)) {
        return CommandResult::Fail(CommandStatus::Invalid, "bad effect times");
    }
    const Element* el = layer->GetParentElement();
    if (el != nullptr && el->GetType() == ElementType::ELEMENT_TYPE_MODEL && !s.name.empty() && s.name != "Random") {
        // GetEffectManager() is a non-const getter; looking a name up doesn't mutate.
        if (const_cast<SequenceElements&>(seq).GetEffectManager().GetEffectIndex(s.name) == -1) {
            return CommandResult::Fail(CommandStatus::Invalid, "unknown effect " + s.name);
        }
    }
    return CommandResult::Success();
}

// Validates a list of effects meant to live on one layer together.
CommandResult CheckNewEffects(const SequenceElements& seq, const EffectLayer* layer, const std::vector<EffectState>& effects, ApplyMode mode) {
    std::set<SeqObjectId> ids;
    for (const auto& s : effects) {
        CommandResult r = CheckNewEffect(seq, layer, s);
        if (!r.Ok()) {
            return r;
        }
        if (!ids.insert(s.uid).second) {
            return CommandResult::Fail(CommandStatus::Invalid, "duplicate effect id");
        }
    }
    if (mode == ApplyMode::Strict) {
        std::vector<const EffectState*> sorted;
        for (const auto& s : effects) {
            sorted.push_back(&s);
        }
        std::sort(sorted.begin(), sorted.end(), [](const EffectState* a, const EffectState* b) { return a->startTimeMS < b->startTimeMS; });
        for (size_t i = 1; i < sorted.size(); ++i) {
            if (sorted[i]->startTimeMS < sorted[i - 1]->endTimeMS) {
                return CommandResult::Fail(CommandStatus::Overlap, "effects overlap");
            }
        }
    }
    return CommandResult::Success();
}

// Creates the effect and gives it the state's id. Caller has validated.
Effect* CreateEffect(EffectLayer* layer, const EffectState& s) {
    Effect* e = layer->AddEffect(0, s.name, s.settings, s.palette, s.startTimeMS, s.endTimeMS, s.selected, s.isProtected);
    if (e != nullptr && layer->GetEffectFromUid(s.uid) == nullptr) {
        e->SetUid(s.uid);
    }
    return e;
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

// Effective changes needed to move `current` to what `change` describes,
// skipping no-ops.
KeyChanges EffectiveChanges(const SettingsMap& current, const SettingsChange& change) {
    KeyChanges out;
    if (change.replaceAll) {
        for (const auto& [key, value] : current) {
            auto it = change.keys.find(key);
            if (it == change.keys.end() || !it->second.has_value()) {
                out[key] = std::nullopt;
            }
        }
    }
    for (const auto& [key, value] : change.keys) {
        const SettingValue* cur = current.FindValue(key);
        if (value.has_value()) {
            if (cur == nullptr || *cur != *value) {
                out[key] = value;
            }
        } else if (cur != nullptr) {
            out[key] = std::nullopt;
        }
    }
    return out;
}

KeyChanges PriorValues(const SettingsMap& current, const KeyChanges& changes) {
    KeyChanges prior;
    for (const auto& [key, value] : changes) {
        const SettingValue* cur = current.FindValue(key);
        prior[key] = cur == nullptr ? std::nullopt : std::optional<std::string>(*cur);
    }
    return prior;
}

bool ChangeUnlocks(const Effect* e, const SettingsChange& change) {
    KeyChanges eff = EffectiveChanges(e->GetSettings(), change);
    auto it = eff.find("X_Effect_Locked");
    return it != eff.end() && !it->second.has_value();
}

// ---- JSON helpers: never throw, report failure by return value ----------

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

const json* ReadObject(const json& j, const char* key) {
    auto it = j.find(key);
    return (it == j.end() || !it->is_object()) ? nullptr : &*it;
}

const json* ReadArray(const json& j, const char* key) {
    auto it = j.find(key);
    return (it == j.end() || !it->is_array()) ? nullptr : &*it;
}

json WriteLayer(const LayerLocator& l) {
    return json{ { "element", l.element }, { "layer", l.layer } };
}

bool ReadLayer(const json& j, LayerLocator& l) {
    return j.is_object() && ReadU64(j, "element", l.element) && ReadU64(j, "layer", l.layer);
}

json WriteTarget(const EffectLocator& t) {
    return json{ { "element", t.element }, { "layer", t.layer }, { "effect", t.effect } };
}

bool ReadTarget(const json& j, EffectLocator& t) {
    return j.is_object() && ReadU64(j, "element", t.element) && ReadU64(j, "layer", t.layer) && ReadU64(j, "effect", t.effect);
}

json WriteEffect(const EffectState& s) {
    return json{ { "uid", s.uid }, { "name", s.name }, { "settings", s.settings }, { "palette", s.palette },
                 { "start", s.startTimeMS }, { "end", s.endTimeMS }, { "protected", s.isProtected } };
}

bool ReadEffect(const json& j, EffectState& s) {
    return j.is_object() && ReadU64(j, "uid", s.uid) && ReadString(j, "name", s.name) && ReadString(j, "settings", s.settings) &&
           ReadString(j, "palette", s.palette) && ReadInt(j, "start", s.startTimeMS) && ReadInt(j, "end", s.endTimeMS) &&
           ReadBool(j, "protected", s.isProtected);
}

json WriteEffects(const std::vector<EffectState>& v) {
    json a = json::array();
    for (const auto& s : v) {
        a.push_back(WriteEffect(s));
    }
    return a;
}

bool ReadEffects(const json* a, std::vector<EffectState>& v) {
    if (a == nullptr) {
        return false;
    }
    for (const auto& e : *a) {
        EffectState s;
        if (!ReadEffect(e, s)) {
            return false;
        }
        v.push_back(std::move(s));
    }
    return true;
}

json WriteChange(const SettingsChange& c) {
    json keys = json::object();
    for (const auto& [k, v] : c.keys) {
        keys[k] = v.has_value() ? json(*v) : json(nullptr);
    }
    return json{ { "replaceAll", c.replaceAll }, { "keys", keys } };
}

bool ReadChange(const json* j, SettingsChange& c) {
    if (j == nullptr || !ReadBool(*j, "replaceAll", c.replaceAll)) {
        return false;
    }
    const json* keys = ReadObject(*j, "keys");
    if (keys == nullptr) {
        return false;
    }
    for (auto it = keys->begin(); it != keys->end(); ++it) {
        if (it->is_null()) {
            c.keys[it.key()] = std::nullopt;
        } else if (it->is_string()) {
            c.keys[it.key()] = it->get<std::string>();
        } else {
            return false;
        }
    }
    return true;
}

std::unique_ptr<SequenceCommand> Deserialize(const json& in, int depth);

} // namespace

// ---------------------------------------------------------------------------
// Value types
// ---------------------------------------------------------------------------

EffectState EffectState::Of(const Effect& e) {
    EffectState s;
    s.uid = e.GetUid();
    s.name = e.GetEffectName();
    s.settings = e.GetSettingsAsString();
    s.palette = e.GetPaletteAsString();
    s.startTimeMS = e.GetStartTimeMS();
    s.endTimeMS = e.GetEndTimeMS();
    s.isProtected = e.GetProtected();
    return s;
}

SettingsChange SettingsChange::ReplaceWith(const std::string& serialized) {
    SettingsMap m;
    m.Parse(nullptr, serialized, "");
    SettingsChange c;
    c.replaceAll = true;
    for (const auto& [key, value] : m) {
        c.keys[key] = std::string(value);
    }
    // GetSettingsAsString() appends the symbol link as a pseudo-setting; the
    // link itself is not part of the settings map.
    c.keys.erase("X_LinkedSymbolId");
    return c;
}

std::unique_ptr<SequenceCommand> SequenceCommand::Apply(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    SeqMutationGuard::Scope scope;
    return ApplyImpl(seq, mode, result);
}

std::string SequenceCommand::ToJson() const {
    json j;
    Serialize(j);
    return j.dump(-1, ' ', false, json::error_handler_t::replace);
}

// ---------------------------------------------------------------------------
// AddEffect
// ---------------------------------------------------------------------------

AddEffectCommand::AddEffectCommand(LayerLocator layer, EffectState state) :
    _layer(layer), _state(std::move(state)) {
    if (_state.uid == kNoSeqObjectId) {
        _state.uid = SeqIds::Next();
    }
}

CommandResult AddEffectCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    EffectLayer* layer = FindLayer(seq, _layer);
    if (layer == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "layer not found");
    }
    CommandResult r = CheckNewEffect(seq, layer, _state);
    if (!r.Ok()) {
        return r;
    }
    if (layer->GetEffectFromUid(_state.uid) != nullptr) {
        return CommandResult::Fail(CommandStatus::Invalid, "effect id already in use");
    }
    if (mode == ApplyMode::Strict && !RangeClear(layer, _state.startTimeMS, _state.endTimeMS, kNoSeqObjectId)) {
        return CommandResult::Fail(CommandStatus::Overlap, "overlaps an existing effect");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> AddEffectCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    EffectLayer* layer = FindLayer(seq, _layer);
    if (CreateEffect(layer, _state) == nullptr) {
        return Failed(result, CommandResult::Fail(CommandStatus::Invalid, "effect could not be created"));
    }
    return Succeeded(result, std::make_unique<DeleteEffectCommand>(Target()));
}

void AddEffectCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "layer", WriteLayer(_layer) }, { "effect", WriteEffect(_state) } };
}

std::unique_ptr<SequenceCommand> AddEffectCommand::Clone() const {
    return std::make_unique<AddEffectCommand>(*this);
}

// ---------------------------------------------------------------------------
// DeleteEffect
// ---------------------------------------------------------------------------

DeleteEffectCommand::DeleteEffectCommand(EffectLocator target) :
    _target(target) {
}

CommandResult DeleteEffectCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Effect* e = FindEffect(seq, _target);
    if (e == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "effect not found");
    }
    if (mode == ApplyMode::Strict && e->IsLocked()) {
        return CommandResult::Fail(CommandStatus::Locked, "effect is locked");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> DeleteEffectCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    EffectLayer* layer = FindLayer(seq, _target.Layer());
    EffectState state = EffectState::Of(*layer->GetEffectFromUid(_target.effect));
    layer->DeleteEffectByUid(_target.effect);
    return Succeeded(result, std::make_unique<AddEffectCommand>(_target.Layer(), std::move(state)));
}

void DeleteEffectCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "target", WriteTarget(_target) } };
}

std::unique_ptr<SequenceCommand> DeleteEffectCommand::Clone() const {
    return std::make_unique<DeleteEffectCommand>(*this);
}

// ---------------------------------------------------------------------------
// MoveEffect
// ---------------------------------------------------------------------------

MoveEffectCommand::MoveEffectCommand(EffectLocator target, int startTimeMS, int endTimeMS) :
    _target(target), _start(startTimeMS), _end(endTimeMS) {
}

CommandResult MoveEffectCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Effect* e = FindEffect(seq, _target);
    if (e == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "effect not found");
    }
    if (mode == ApplyMode::Strict) {
        if (_start < 0 || _end < _start) {
            return CommandResult::Fail(CommandStatus::Invalid, "bad effect times");
        }
        if (e->IsLocked()) {
            return CommandResult::Fail(CommandStatus::Locked, "effect is locked");
        }
        if (!RangeClear(e->GetParentEffectLayer(), _start, _end, e->GetUid())) {
            return CommandResult::Fail(CommandStatus::Overlap, "overlaps an existing effect");
        }
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> MoveEffectCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Effect* e = FindEffect(seq, _target);
    auto inverse = std::make_unique<MoveEffectCommand>(_target, e->GetStartTimeMS(), e->GetEndTimeMS());
    // The setters assert the effect is unlocked; lenient mode moves locked
    // effects too, so lift the lock around the move.
    bool locked = e->IsLocked();
    if (locked) {
        e->SetLocked(false);
    }
    e->SetStartTimeMS(_start);
    e->SetEndTimeMS(_end);
    if (locked) {
        e->SetLocked(true);
    }
    return Succeeded(result, std::move(inverse));
}

void MoveEffectCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "target", WriteTarget(_target) }, { "start", _start }, { "end", _end } };
}

std::unique_ptr<SequenceCommand> MoveEffectCommand::Clone() const {
    return std::make_unique<MoveEffectCommand>(*this);
}

// ---------------------------------------------------------------------------
// ModifyEffect
// ---------------------------------------------------------------------------

ModifyEffectCommand::ModifyEffectCommand(EffectLocator target, std::optional<std::string> effectName, SettingsChange settings, SettingsChange palette) :
    _target(target), _name(std::move(effectName)), _settings(std::move(settings)), _palette(std::move(palette)) {
}

CommandResult ModifyEffectCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Effect* e = FindEffect(seq, _target);
    if (e == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "effect not found");
    }
    if (_name.has_value()) {
        EffectState probe;
        probe.uid = e->GetUid();
        probe.name = *_name;
        probe.startTimeMS = e->GetStartTimeMS();
        probe.endTimeMS = e->GetEndTimeMS();
        CommandResult r = CheckNewEffect(seq, e->GetParentEffectLayer(), probe);
        if (!r.Ok()) {
            return r;
        }
    }
    if (mode == ApplyMode::Strict && e->IsLocked() && !ChangeUnlocks(e, _settings)) {
        return CommandResult::Fail(CommandStatus::Locked, "effect is locked");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> ModifyEffectCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Effect* e = FindEffect(seq, _target);

    std::optional<std::string> priorName;
    if (_name.has_value() && *_name != e->GetEffectName()) {
        priorName = e->GetEffectName();
        e->SetEffectName(*_name);
    }

    KeyChanges settings = EffectiveChanges(e->GetSettings(), _settings);
    SettingsChange priorSettings{ false, PriorValues(e->GetSettings(), settings) };
    e->ApplySettingChanges(settings);

    KeyChanges palette = EffectiveChanges(e->GetPaletteMap(), _palette);
    SettingsChange priorPalette{ false, PriorValues(e->GetPaletteMap(), palette) };
    e->ApplyPaletteChanges(palette);

    return Succeeded(result, std::make_unique<ModifyEffectCommand>(_target, std::move(priorName), std::move(priorSettings), std::move(priorPalette)));
}

void ModifyEffectCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "target", WriteTarget(_target) },
                { "settings", WriteChange(_settings) }, { "palette", WriteChange(_palette) } };
    if (_name.has_value()) {
        out["name"] = *_name;
    }
}

std::unique_ptr<SequenceCommand> ModifyEffectCommand::Clone() const {
    return std::make_unique<ModifyEffectCommand>(*this);
}

// ---------------------------------------------------------------------------
// AddLayer
// ---------------------------------------------------------------------------

AddLayerCommand::AddLayerCommand(SeqObjectId element, SeqObjectId layerUid, SeqObjectId afterLayer, int indexHint,
                                 std::string name, std::vector<EffectState> effects) :
    _element(element), _layer(layerUid == kNoSeqObjectId ? SeqIds::Next() : layerUid), _after(afterLayer),
    _indexHint(indexHint), _name(std::move(name)), _effects(std::move(effects)) {
}

CommandResult AddLayerCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Element* element = seq.GetElementFromUid(_element);
    if (element == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "element not found");
    }
    if (element->GetEffectLayerFromUid(_layer) != nullptr) {
        return CommandResult::Fail(CommandStatus::Invalid, "layer id already in use");
    }
    if (mode == ApplyMode::Strict && _after != kNoSeqObjectId && IndexOfLayer(element, _after) < 0) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "sibling layer not found");
    }
    if (!_effects.empty()) {
        // Validate against an existing layer of the element for the effect-name
        // check; the new layer itself is empty so nothing can overlap it.
        const EffectLayer* probe = element->GetEffectLayerCount() > 0 ? element->GetEffectLayer(0) : nullptr;
        if (probe != nullptr) {
            CommandResult r = CheckNewEffects(seq, probe, _effects, mode);
            if (!r.Ok()) {
                return r;
            }
        }
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> AddLayerCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Element* element = seq.GetElementFromUid(_element);
    int count = (int)element->GetEffectLayerCount();
    int pos;
    if (_after == kNoSeqObjectId) {
        pos = 0;
    } else {
        int idx = IndexOfLayer(element, _after);
        pos = idx >= 0 ? idx + 1 : (_indexHint < 0 ? count : (std::min)(_indexHint, count));
    }
    EffectLayer* layer = pos >= count ? element->AddEffectLayer() : element->InsertEffectLayer(pos);
    layer->SetUid(_layer);
    if (!_name.empty()) {
        layer->SetLayerName(_name);
    }
    for (const auto& s : _effects) {
        CreateEffect(layer, s);
    }
    seq.PopulateRowInformation();
    return Succeeded(result, std::make_unique<RemoveLayerCommand>(_element, _layer));
}

void AddLayerCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "layer", _layer }, { "after", _after },
                { "indexHint", _indexHint }, { "name", _name }, { "effects", WriteEffects(_effects) } };
}

std::unique_ptr<SequenceCommand> AddLayerCommand::Clone() const {
    return std::make_unique<AddLayerCommand>(*this);
}

// ---------------------------------------------------------------------------
// RemoveLayer
// ---------------------------------------------------------------------------

RemoveLayerCommand::RemoveLayerCommand(SeqObjectId element, SeqObjectId layer) :
    _element(element), _layer(layer) {
}

CommandResult RemoveLayerCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    Element* element = seq.GetElementFromUid(_element);
    if (element == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "element not found");
    }
    int idx = IndexOfLayer(element, _layer);
    if (idx < 0) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "layer not found");
    }
    if (element->GetEffectLayerCount() <= 1) {
        return CommandResult::Fail(CommandStatus::Invalid, "cannot remove an element's last layer");
    }
    if (mode == ApplyMode::Strict && HasLockedEffect(element->GetEffectLayer(idx))) {
        return CommandResult::Fail(CommandStatus::Locked, "layer holds a locked effect");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> RemoveLayerCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    Element* element = seq.GetElementFromUid(_element);
    int pos = IndexOfLayer(element, _layer);
    EffectLayer* layer = element->GetEffectLayer(pos);
    SeqObjectId after = pos > 0 ? element->GetEffectLayer(pos - 1)->GetUid() : kNoSeqObjectId;
    std::vector<EffectState> effects;
    for (const Effect* e : layer->GetEffects()) {
        effects.push_back(EffectState::Of(*e));
    }
    std::string name = layer->GetLayerName();
    element->RemoveEffectLayer(pos);
    seq.PopulateRowInformation();
    return Succeeded(result, std::make_unique<AddLayerCommand>(_element, _layer, after, pos, std::move(name), std::move(effects)));
}

void RemoveLayerCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "element", _element }, { "layer", _layer } };
}

std::unique_ptr<SequenceCommand> RemoveLayerCommand::Clone() const {
    return std::make_unique<RemoveLayerCommand>(*this);
}

// ---------------------------------------------------------------------------
// SetLayerName
// ---------------------------------------------------------------------------

SetLayerNameCommand::SetLayerNameCommand(LayerLocator layer, std::string name) :
    _layer(layer), _name(std::move(name)) {
}

CommandResult SetLayerNameCommand::Validate(const SequenceElements& seq, ApplyMode) const {
    if (FindLayer(seq, _layer) == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "layer not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> SetLayerNameCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    EffectLayer* layer = FindLayer(seq, _layer);
    std::string prior = layer->GetLayerName();
    layer->SetLayerName(_name);
    return Succeeded(result, std::make_unique<SetLayerNameCommand>(_layer, std::move(prior)));
}

void SetLayerNameCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "layer", WriteLayer(_layer) }, { "name", _name } };
}

std::unique_ptr<SequenceCommand> SetLayerNameCommand::Clone() const {
    return std::make_unique<SetLayerNameCommand>(*this);
}

// ---------------------------------------------------------------------------
// ReplaceLayerContents
// ---------------------------------------------------------------------------

ReplaceLayerContentsCommand::ReplaceLayerContentsCommand(LayerLocator layer, std::vector<EffectState> effects) :
    _layer(layer), _effects(std::move(effects)) {
    for (auto& s : _effects) {
        if (s.uid == kNoSeqObjectId) {
            s.uid = SeqIds::Next();
        }
    }
}

CommandResult ReplaceLayerContentsCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    EffectLayer* layer = FindLayer(seq, _layer);
    if (layer == nullptr) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "layer not found");
    }
    if (mode == ApplyMode::Strict && HasLockedEffect(layer)) {
        return CommandResult::Fail(CommandStatus::Locked, "layer holds a locked effect");
    }
    return CheckNewEffects(seq, layer, _effects, mode);
}

std::unique_ptr<SequenceCommand> ReplaceLayerContentsCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Failed(result, r);
    }
    EffectLayer* layer = FindLayer(seq, _layer);
    std::vector<EffectState> prior;
    std::vector<SeqObjectId> ids;
    for (const Effect* e : layer->GetEffects()) {
        prior.push_back(EffectState::Of(*e));
        ids.push_back(e->GetUid());
    }
    for (SeqObjectId id : ids) {
        layer->DeleteEffectByUid(id);
    }
    for (const auto& s : _effects) {
        CreateEffect(layer, s);
    }
    return Succeeded(result, std::make_unique<ReplaceLayerContentsCommand>(_layer, std::move(prior)));
}

void ReplaceLayerContentsCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "layer", WriteLayer(_layer) }, { "effects", WriteEffects(_effects) } };
}

std::unique_ptr<SequenceCommand> ReplaceLayerContentsCommand::Clone() const {
    return std::make_unique<ReplaceLayerContentsCommand>(*this);
}

// ---------------------------------------------------------------------------
// Batch
// ---------------------------------------------------------------------------

BatchCommand::BatchCommand(std::string label, std::vector<std::unique_ptr<SequenceCommand>> commands) :
    _label(std::move(label)), _commands(std::move(commands)) {
}

CommandResult BatchCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    for (const auto& c : _commands) {
        CommandResult r = c->Validate(seq, mode);
        if (!r.Ok()) {
            return r;
        }
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> BatchCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    std::vector<std::unique_ptr<SequenceCommand>> inverses;
    for (auto& c : _commands) {
        CommandResult r;
        auto inv = c->Apply(seq, mode, &r);
        if (inv) {
            inverses.push_back(std::move(inv));
        } else if (mode == ApplyMode::Strict) {
            for (auto it = inverses.rbegin(); it != inverses.rend(); ++it) {
                (*it)->Apply(seq, ApplyMode::Lenient);
            }
            return Failed(result, r);
        }
    }
    std::reverse(inverses.begin(), inverses.end());
    return Succeeded(result, std::make_unique<BatchCommand>(_label, std::move(inverses)));
}

void BatchCommand::Serialize(json& out) const {
    json cmds = json::array();
    for (const auto& c : _commands) {
        json j;
        c->Serialize(j);
        cmds.push_back(std::move(j));
    }
    out = json{ { "type", CommandTypeName(Type()) }, { "label", _label }, { "commands", std::move(cmds) } };
}

std::unique_ptr<SequenceCommand> BatchCommand::Clone() const {
    std::vector<std::unique_ptr<SequenceCommand>> copies;
    for (const auto& c : _commands) {
        copies.push_back(c->Clone());
    }
    return std::make_unique<BatchCommand>(_label, std::move(copies));
}

// ---------------------------------------------------------------------------
// Deserialization
// ---------------------------------------------------------------------------

namespace {

constexpr int kMaxBatchDepth = 32;

std::unique_ptr<SequenceCommand> Deserialize(const json& in, int depth) {
    if (!in.is_object() || depth > kMaxBatchDepth) {
        return nullptr;
    }
    std::string type;
    if (!ReadString(in, "type", type)) {
        return nullptr;
    }
    if (type == "AddEffect") {
        LayerLocator l;
        EffectState s;
        const json* lj = ReadObject(in, "layer");
        const json* ej = ReadObject(in, "effect");
        if (lj == nullptr || ej == nullptr || !ReadLayer(*lj, l) || !ReadEffect(*ej, s) || s.uid == kNoSeqObjectId) {
            return nullptr;
        }
        return std::make_unique<AddEffectCommand>(l, std::move(s));
    }
    if (type == "DeleteEffect") {
        EffectLocator t;
        const json* tj = ReadObject(in, "target");
        if (tj == nullptr || !ReadTarget(*tj, t)) {
            return nullptr;
        }
        return std::make_unique<DeleteEffectCommand>(t);
    }
    if (type == "MoveEffect") {
        EffectLocator t;
        int start = 0, end = 0;
        const json* tj = ReadObject(in, "target");
        if (tj == nullptr || !ReadTarget(*tj, t) || !ReadInt(in, "start", start) || !ReadInt(in, "end", end)) {
            return nullptr;
        }
        return std::make_unique<MoveEffectCommand>(t, start, end);
    }
    if (type == "ModifyEffect") {
        EffectLocator t;
        SettingsChange settings, palette;
        const json* tj = ReadObject(in, "target");
        if (tj == nullptr || !ReadTarget(*tj, t) || !ReadChange(ReadObject(in, "settings"), settings) ||
            !ReadChange(ReadObject(in, "palette"), palette)) {
            return nullptr;
        }
        std::optional<std::string> name;
        if (in.contains("name")) {
            std::string n;
            if (!ReadString(in, "name", n)) {
                return nullptr;
            }
            name = std::move(n);
        }
        return std::make_unique<ModifyEffectCommand>(t, std::move(name), std::move(settings), std::move(palette));
    }
    if (type == "AddLayer") {
        uint64_t element = 0, layer = 0, after = 0;
        int hint = -1;
        std::string name;
        std::vector<EffectState> effects;
        if (!ReadU64(in, "element", element) || !ReadU64(in, "layer", layer) || layer == kNoSeqObjectId || !ReadU64(in, "after", after) ||
            !ReadInt(in, "indexHint", hint) || !ReadString(in, "name", name) || !ReadEffects(ReadArray(in, "effects"), effects)) {
            return nullptr;
        }
        return std::make_unique<AddLayerCommand>(element, layer, after, hint, std::move(name), std::move(effects));
    }
    if (type == "RemoveLayer") {
        uint64_t element = 0, layer = 0;
        if (!ReadU64(in, "element", element) || !ReadU64(in, "layer", layer)) {
            return nullptr;
        }
        return std::make_unique<RemoveLayerCommand>(element, layer);
    }
    if (type == "SetLayerName") {
        LayerLocator l;
        std::string name;
        const json* lj = ReadObject(in, "layer");
        if (lj == nullptr || !ReadLayer(*lj, l) || !ReadString(in, "name", name)) {
            return nullptr;
        }
        return std::make_unique<SetLayerNameCommand>(l, std::move(name));
    }
    if (type == "ReplaceLayerContents") {
        LayerLocator l;
        std::vector<EffectState> effects;
        const json* lj = ReadObject(in, "layer");
        if (lj == nullptr || !ReadLayer(*lj, l) || !ReadEffects(ReadArray(in, "effects"), effects)) {
            return nullptr;
        }
        for (const auto& s : effects) {
            if (s.uid == kNoSeqObjectId) {
                return nullptr;
            }
        }
        return std::make_unique<ReplaceLayerContentsCommand>(l, std::move(effects));
    }
    if (type == "Batch") {
        std::string label;
        const json* cmds = ReadArray(in, "commands");
        if (!ReadString(in, "label", label) || cmds == nullptr) {
            return nullptr;
        }
        std::vector<std::unique_ptr<SequenceCommand>> members;
        for (const auto& c : *cmds) {
            auto m = Deserialize(c, depth + 1);
            if (!m) {
                return nullptr;
            }
            members.push_back(std::move(m));
        }
        return std::make_unique<BatchCommand>(std::move(label), std::move(members));
    }
    if (auto media = DeserializeMediaCommand(type, in)) {
        return media;
    }
    return DeserializeElementCommand(in);
}

} // namespace

std::unique_ptr<SequenceCommand> DeserializeCommand(const json& in) {
    return Deserialize(in, 0);
}

std::unique_ptr<SequenceCommand> CommandFromJson(const std::string& text) {
    json j = json::parse(text, nullptr, false);
    if (j.is_discarded()) {
        return nullptr;
    }
    return Deserialize(j, 0);
}

// ---------------------------------------------------------------------------
// DescribeSequence
// ---------------------------------------------------------------------------

namespace {

const char* ElementTypeTag(ElementType t) {
    switch (t) {
    case ElementType::ELEMENT_TYPE_MODEL: return "model";
    case ElementType::ELEMENT_TYPE_SUBMODEL: return "submodel";
    case ElementType::ELEMENT_TYPE_STRAND: return "strand";
    case ElementType::ELEMENT_TYPE_TIMING: return "timing";
    }
    return "?";
}

void DescribeLayer(std::ostringstream& out, const EffectLayer* layer, const std::string& tag, bool ids) {
    out << "  " << tag << " '" << layer->GetLayerName() << "'";
    if (ids) {
        out << " #" << layer->GetUid();
    }
    out << "\n";
    // Effect order within the vector isn't document state (moves don't
    // re-sort), so describe effects in time order.
    std::vector<const Effect*> effects(layer->GetEffects().begin(), layer->GetEffects().end());
    std::stable_sort(effects.begin(), effects.end(), [](const Effect* a, const Effect* b) {
        if (a->GetStartTimeMS() != b->GetStartTimeMS()) {
            return a->GetStartTimeMS() < b->GetStartTimeMS();
        }
        return a->GetEndTimeMS() < b->GetEndTimeMS();
    });
    for (const Effect* e : effects) {
        out << "    " << e->GetStartTimeMS() << "-" << e->GetEndTimeMS() << " '" << e->GetEffectName() << "'";
        if (ids) {
            out << " #" << e->GetUid();
        }
        out << (e->GetProtected() ? " protected" : "") << " s{" << e->GetSettingsAsString() << "} c{" << e->GetPaletteAsString() << "}\n";
    }
}

void DescribeElement(std::ostringstream& out, const Element* el, bool ids) {
    out << ElementTypeTag(el->GetType()) << " '" << el->GetFullName() << "'";
    if (ids) {
        out << " #" << el->GetUid();
    }
    out << "\n";
    const auto& layers = el->GetEffectLayers();
    for (size_t i = 0; i < layers.size(); ++i) {
        DescribeLayer(out, layers[i], "L" + std::to_string(i), ids);
    }
    if (el->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        const StrandElement* st = dynamic_cast<const StrandElement*>(el);
        for (int n = 0; st != nullptr && n < st->GetNodeLayerCount(); ++n) {
            DescribeLayer(out, st->GetNodeLayer(n), "N" + std::to_string(n), ids);
        }
    }
}

} // namespace

std::string DescribeSequence(const SequenceElements& seq, bool includeIds) {
    std::ostringstream out;
    for (size_t i = 0; i < seq.GetElementCount(MASTER_VIEW); ++i) {
        const Element* el = seq.GetElement(i, MASTER_VIEW);
        if (el == nullptr) {
            continue;
        }
        DescribeElement(out, el, includeIds);
        if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            const ModelElement* mel = dynamic_cast<const ModelElement*>(el);
            for (int x = 0; mel != nullptr && x < mel->GetSubModelAndStrandCount(); ++x) {
                const SubModelElement* sme = mel->GetSubModel(x);
                if (sme != nullptr) {
                    DescribeElement(out, sme, includeIds);
                }
            }
        }
    }
    return out.str();
}

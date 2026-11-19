/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CommandExecutor.h"

#include <algorithm>

#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceEditCapture.h"
#include "SequenceElements.h"
#include "SequenceMutationGuard.h"
#include "UndoManager.h"
#include "../utils/UtilClasses.h"

#include <log.h>

CommandExecutor::CommandExecutor(SequenceElements& seq, UndoManager& undo) :
    _seq(seq), _undo(undo) {
}

ScopedCommandBatch::ScopedCommandBatch(SequenceElements& seq, const std::string& label) :
    ScopedCommandBatch(seq.GetCommandExecutor(), label) {
}

bool CommandExecutor::GateAllows(const SequenceCommand& cmd, std::string* why) {
    if (_gate == nullptr) {
        return true;
    }
    std::string w;
    bool ok = _gate->AllowCommand(cmd, w);
    if (why != nullptr) {
        *why = std::move(w);
    }
    return ok;
}

CommandResult CommandExecutor::Execute(std::unique_ptr<SequenceCommand> cmd) {
    if (!cmd) {
        return CommandResult::Fail(CommandStatus::Invalid, "no command");
    }
    if (std::string why; !GateAllows(*cmd, &why)) {
        return CommandResult::Fail(CommandStatus::Refused, why);
    }
    if (_previewDepth > 0) {
        PreviewTouch(*cmd);
    }
    if (_capture != nullptr) {
        _capture->BeforeRecording(nullptr);
    }
    CommandResult result;
    std::unique_ptr<SequenceCommand> inverse;
    {
        ApplyingScope applying(*this);
        inverse = cmd->Apply(_seq, ApplyMode::Strict, &result);
    }
    if (_capture != nullptr) {
        _capture->NoteCommandAdds();
    }
    if (!inverse) {
        return result;
    }
    if (_previewDepth > 0 || _capture != nullptr) {
        RecordApplied(std::move(cmd), std::move(inverse), true);
        return result;
    }
    NotifyRecorded(*cmd, *inverse);
    if (_batchDepth > 0) {
        _batchUndo.push_back(inverse->Clone());
        _batch->Add(std::move(cmd));
        _batchInverse->Add(std::move(inverse));
        return result;
    }
    _undo.CreateUndoStep();
    _undo.RecordInverse(cmd->Type(), "", inverse->Clone());
    Notify(*cmd, *inverse);
    return result;
}

void CommandExecutor::BeginBatch(const std::string& label) {
    if (_batchDepth++ == 0) {
        _batch = std::make_unique<BatchCommand>(label);
        _batchInverse = std::make_unique<BatchCommand>(label);
    }
}

bool CommandExecutor::EndBatch() {
    if (_batchDepth == 0) {
        return false;
    }
    if (--_batchDepth > 0) {
        return false;
    }
    std::unique_ptr<BatchCommand> batch = std::move(_batch);
    std::unique_ptr<BatchCommand> inverses = std::move(_batchInverse);
    std::vector<std::unique_ptr<SequenceCommand>> undo = std::move(_batchUndo);
    _batchUndo.clear();
    if (batch->IsEmpty()) {
        return false;
    }
    // One undo step whose entries are the recorded members' inverses in the
    // order the members ran; undo walks them backwards.
    if (!undo.empty()) {
        _undo.CreateUndoStep();
        for (auto& inv : undo) {
            _undo.RecordInverse(CommandType::Batch, batch->GetLabel(), std::move(inv));
        }
    }
    // The inverse of the batch as a whole runs the member inverses backwards.
    std::vector<std::unique_ptr<SequenceCommand>> reversed;
    for (auto it = inverses->GetCommands().rbegin(); it != inverses->GetCommands().rend(); ++it) {
        reversed.push_back((*it)->Clone());
    }
    BatchCommand inverse(batch->GetLabel(), std::move(reversed));
    Notify(*batch, inverse);
    return !undo.empty();
}

void CommandExecutor::AddObserver(ICommandObserver* observer) {
    if (observer != nullptr && std::find(_observers.begin(), _observers.end(), observer) == _observers.end()) {
        _observers.push_back(observer);
    }
}

void CommandExecutor::RemoveObserver(ICommandObserver* observer) {
    _observers.erase(std::remove(_observers.begin(), _observers.end(), observer), _observers.end());
}

void CommandExecutor::AddRecordListener(ICommandObserver* listener) {
    if (listener != nullptr && std::find(_recordListeners.begin(), _recordListeners.end(), listener) == _recordListeners.end()) {
        _recordListeners.push_back(listener);
    }
}

void CommandExecutor::RemoveRecordListener(ICommandObserver* listener) {
    _recordListeners.erase(std::remove(_recordListeners.begin(), _recordListeners.end(), listener), _recordListeners.end());
}

void CommandExecutor::NotifyRecorded(const SequenceCommand& applied, const SequenceCommand& inverse) {
    std::vector<ICommandObserver*> listeners = _recordListeners;
    for (ICommandObserver* l : listeners) {
        l->OnCommandApplied(applied, inverse);
    }
}

void CommandExecutor::Notify(const SequenceCommand& applied, const SequenceCommand& inverse) {
    // Copy: an observer may remove itself while being notified.
    std::vector<ICommandObserver*> observers = _observers;
    for (ICommandObserver* o : observers) {
        o->OnCommandApplied(applied, inverse);
    }
}

// ---------------------------------------------------------------------------
// Edit-site API
// ---------------------------------------------------------------------------

bool CommandExecutor::CapturedKind(const SequenceCommand& cmd) {
    switch (cmd.Type()) {
    case CommandType::AddEffect:
    case CommandType::DeleteEffect:
    case CommandType::MoveEffect:
    case CommandType::ModifyEffect:
    case CommandType::AddLayer:
    case CommandType::RemoveLayer:
    case CommandType::SetLayerName:
    case CommandType::ReplaceLayerContents:
    case CommandType::SetEffectSymbolLink:
    case CommandType::SetElementRenderDisabled:
    case CommandType::SetTimingFixed:
    case CommandType::SetSequenceFace:
        return true;
    case CommandType::Batch: {
        const BatchCommand& b = static_cast<const BatchCommand&>(cmd);
        for (const auto& m : b.GetCommands()) {
            if (!CapturedKind(*m)) {
                return false;
            }
        }
        return true;
    }
    default:
        return false;
    }
}

void CommandExecutor::DirectElementDelete(Element* element, bool keepInViews) {
    if (_capture != nullptr && _applyDepth == 0 && element != nullptr) {
        _capture->ElementDeleting(element, keepInViews);
    }
}

void CommandExecutor::RecordApplied(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo) {
    _undo.DocumentEdited();
    NotifyRecorded(*applied, *inverse);
    if (_capture != nullptr) {
        if (CapturedKind(*applied)) {
            return;
        }
        SequenceEditCapture* capture = _capture;
        capture->BeforeRecording(applied.get());
        std::unique_ptr<SequenceCommand> recorded = applied->Clone();
        RecordNow(std::move(applied), std::move(inverse), recordUndo);
        capture->AfterRecording(*recorded);
        return;
    }
    RecordNow(std::move(applied), std::move(inverse), recordUndo);
}

void CommandExecutor::RecordNow(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo) {
    if (_previewDepth > 0) {
        // Moves and modifies are recorded as one net change at commit.
        if (!PreviewAbsorbs(*applied)) {
            _previewOther.push_back({ std::move(applied), std::move(inverse), recordUndo });
        }
        return;
    }
    if (_batchDepth > 0) {
        // Everything in a batch reaches observers together, in order; only
        // the undo recording depends on recordUndo.
        if (recordUndo) {
            _batchUndo.push_back(inverse->Clone());
        }
        _batch->Add(std::move(applied));
        _batchInverse->Add(std::move(inverse));
        return;
    }
    if (recordUndo) {
        _undo.RecordInverse(applied->Type(), "", inverse->Clone());
    }
    Notify(*applied, *inverse);
}

CommandResult CommandExecutor::ExecuteInStep(std::unique_ptr<SequenceCommand> cmd, bool recordUndo, bool gated) {
    if (!cmd) {
        return CommandResult::Fail(CommandStatus::Invalid, "no command");
    }
    if (std::string why; gated && !GateAllows(*cmd, &why)) {
        return CommandResult::Fail(CommandStatus::Refused, why);
    }
    CommandResult strict = cmd->Validate(_seq, ApplyMode::Strict);
    if (_previewDepth > 0) {
        PreviewTouch(*cmd);
    }
    if (_capture != nullptr) {
        _capture->BeforeRecording(nullptr);
    }
    CommandResult result;
    std::unique_ptr<SequenceCommand> inverse;
    {
        ApplyingScope applying(*this);
        inverse = cmd->Apply(_seq, ApplyMode::Lenient, &result);
    }
    if (_capture != nullptr) {
        _capture->NoteCommandAdds();
    }
    if (!inverse) {
        return result;
    }
    if (!strict.Ok()) {
        spdlog::debug("{} applied although strict validation refuses it: {}", CommandTypeName(cmd->Type()), strict.message);
    }
    RecordApplied(std::move(cmd), std::move(inverse), recordUndo);
    return result;
}

bool CommandExecutor::InSequence(const Effect* effect) const {
    return effect != nullptr && FindEffect(_seq, LocatorOf(effect)) == effect;
}

Effect* CommandExecutor::AddEffect(EffectLayer* layer, const std::string& name, const std::string& settings, const std::string& palette,
                                   int startTimeMS, int endTimeMS, int selected, bool isProtected, bool recordUndo) {
    if (layer == nullptr) {
        return nullptr;
    }
    LayerLocator loc = LocatorOf(layer);
    if (FindLayer(_seq, loc) != layer) {
        return layer->AddEffect(0, name, settings, palette, startTimeMS, endTimeMS, selected, isProtected);
    }
    EffectState state;
    state.name = name;
    state.settings = settings;
    state.palette = palette;
    state.startTimeMS = startTimeMS;
    state.endTimeMS = endTimeMS;
    state.isProtected = isProtected;
    state.selected = selected;
    auto cmd = std::make_unique<AddEffectCommand>(loc, std::move(state));
    EffectLocator target = cmd->Target();
    if (!ExecuteInStep(std::move(cmd), recordUndo, false).Ok()) {
        return nullptr;
    }
    return FindEffect(_seq, target);
}

bool CommandExecutor::DeleteEffect(Effect* effect, bool recordUndo) {
    if (effect == nullptr) {
        return false;
    }
    if (!InSequence(effect)) {
        EffectLayer* layer = effect->GetParentEffectLayer();
        return layer != nullptr && layer->DeleteEffectByUid(effect->GetUid());
    }
    return ExecuteInStep(std::make_unique<DeleteEffectCommand>(LocatorOf(effect)), recordUndo).Ok();
}

bool CommandExecutor::MoveEffect(Effect* effect, int startTimeMS, int endTimeMS, bool recordUndo) {
    if (effect == nullptr) {
        return false;
    }
    if (!InSequence(effect)) {
        effect->SetStartTimeMS(startTimeMS);
        effect->SetEndTimeMS(endTimeMS);
        return true;
    }
    return ExecuteInStep(std::make_unique<MoveEffectCommand>(LocatorOf(effect), startTimeMS, endTimeMS), recordUndo).Ok();
}

bool CommandExecutor::ModifyEffect(Effect* effect, std::optional<std::string> effectName, SettingsChange settings, SettingsChange palette,
                                   bool recordUndo) {
    if (effect == nullptr) {
        return false;
    }
    if (!InSequence(effect)) {
        if (effectName.has_value()) {
            effect->SetEffectName(*effectName);
        }
        KeyChanges s = settings.keys;
        if (settings.replaceAll) {
            for (const auto& [k, v] : effect->GetSettings()) {
                if (!s.count(k)) {
                    s[k] = std::nullopt;
                }
            }
        }
        KeyChanges p = palette.keys;
        if (palette.replaceAll) {
            for (const auto& [k, v] : effect->GetPaletteMap()) {
                if (!p.count(k)) {
                    p[k] = std::nullopt;
                }
            }
        }
        effect->ApplySettingChanges(s);
        effect->ApplyPaletteChanges(p);
        return true;
    }
    return ExecuteInStep(std::make_unique<ModifyEffectCommand>(LocatorOf(effect), std::move(effectName), std::move(settings), std::move(palette)),
                         recordUndo).Ok();
}

bool CommandExecutor::SetEffectSettings(Effect* effect, const std::string& settings, bool keepXSettings, bool recordUndo) {
    if (effect == nullptr) {
        return false;
    }
    SettingsChange change = SettingsChange::ReplaceWith(settings);
    if (keepXSettings) {
        for (const auto& [k, v] : effect->GetSettings()) {
            if (k.size() > 2 && k[0] == 'X' && k[1] == '_') {
                change.keys[k] = std::string(v);
            }
        }
    }
    return ModifyEffect(effect, std::nullopt, std::move(change), SettingsChange{}, recordUndo);
}

bool CommandExecutor::SetEffectPalette(Effect* effect, const std::string& palette, bool recordUndo) {
    return ModifyEffect(effect, std::nullopt, SettingsChange{}, SettingsChange::ReplaceWith(palette), recordUndo);
}

bool CommandExecutor::SetEffectSetting(Effect* effect, const std::string& key, std::optional<std::string> value, bool recordUndo) {
    SettingsChange change;
    change.keys[key] = std::move(value);
    return ModifyEffect(effect, std::nullopt, std::move(change), SettingsChange{}, recordUndo);
}

bool CommandExecutor::SetEffectLocked(Effect* effect, bool locked, bool recordUndo) {
    return SetEffectSetting(effect, "X_Effect_Locked", locked ? std::optional<std::string>("True") : std::nullopt, recordUndo);
}

bool CommandExecutor::SetEffectRenderDisabled(Effect* effect, bool disabled, bool recordUndo) {
    return SetEffectSetting(effect, "X_Effect_RenderDisabled", disabled ? std::optional<std::string>("True") : std::nullopt, recordUndo);
}

namespace {
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
} // namespace

bool CommandExecutor::EditEffect(Effect* effect, const std::function<void(Effect&)>& mutate, bool recordUndo) {
    if (effect == nullptr) {
        return false;
    }
    if (!InSequence(effect)) {
        mutate(*effect);
        return true;
    }
    EffectLocator loc = LocatorOf(effect);
    if (!GateAllows(ModifyEffectCommand(loc, std::nullopt, SettingsChange{}, SettingsChange{}))) {
        return false;
    }
    if (_previewDepth > 0) {
        PreviewTouchEffect(effect);
    }
    std::string beforeName = effect->GetEffectName();
    int beforeStart = effect->GetStartTimeMS();
    int beforeEnd = effect->GetEndTimeMS();
    SettingsMap beforeSettings = effect->GetSettings();
    SettingsMap beforePalette = effect->GetPaletteMap();
    {
        SeqMutationGuard::Scope recorded;
        mutate(*effect);
    }
    bool changed = false;
    if (effect->GetStartTimeMS() != beforeStart || effect->GetEndTimeMS() != beforeEnd) {
        RecordApplied(std::make_unique<MoveEffectCommand>(loc, effect->GetStartTimeMS(), effect->GetEndTimeMS()),
                      std::make_unique<MoveEffectCommand>(loc, beforeStart, beforeEnd), recordUndo);
        changed = true;
    }
    std::string afterName = effect->GetEffectName();
    KeyChanges settings = Diff(beforeSettings, effect->GetSettings());
    KeyChanges palette = Diff(beforePalette, effect->GetPaletteMap());
    if (afterName != beforeName || !settings.empty() || !palette.empty()) {
        std::optional<std::string> name, priorName;
        if (afterName != beforeName) {
            name = afterName;
            priorName = beforeName;
        }
        KeyChanges priorSettings = Diff(effect->GetSettings(), beforeSettings);
        KeyChanges priorPalette = Diff(effect->GetPaletteMap(), beforePalette);
        RecordApplied(std::make_unique<ModifyEffectCommand>(loc, std::move(name), SettingsChange{ false, std::move(settings) }, SettingsChange{ false, std::move(palette) }),
                      std::make_unique<ModifyEffectCommand>(loc, std::move(priorName), SettingsChange{ false, std::move(priorSettings) },
                                                            SettingsChange{ false, std::move(priorPalette) }),
                      recordUndo);
        changed = true;
    }
    return changed;
}

EffectLayer* CommandExecutor::InsertLayer(Element* element, int index, bool recordUndo) {
    if (element == nullptr) {
        return nullptr;
    }
    if (_seq.GetElementFromUid(element->GetUid()) != element) {
        return index >= (int)element->GetEffectLayerCount() ? element->AddEffectLayer() : element->InsertEffectLayer(index);
    }
    int count = (int)element->GetEffectLayerCount();
    SeqObjectId after = kNoSeqObjectId;
    if (index > 0 && count > 0) {
        after = element->GetEffectLayer((std::min)(index, count) - 1)->GetUid();
    }
    auto cmd = std::make_unique<AddLayerCommand>(element->GetUid(), kNoSeqObjectId, after, index);
    SeqObjectId uid = cmd->GetLayer();
    if (!ExecuteInStep(std::move(cmd), recordUndo, false).Ok()) {
        return nullptr;
    }
    return element->GetEffectLayerFromUid(uid);
}

bool CommandExecutor::RemoveLayer(Element* element, int index, bool recordUndo) {
    if (element == nullptr || index < 0 || index >= (int)element->GetEffectLayerCount()) {
        return false;
    }
    if (_seq.GetElementFromUid(element->GetUid()) != element) {
        element->RemoveEffectLayer(index);
        return true;
    }
    return ExecuteInStep(std::make_unique<RemoveLayerCommand>(element->GetUid(), element->GetEffectLayer(index)->GetUid()), recordUndo).Ok();
}

bool CommandExecutor::SetLayerName(EffectLayer* layer, const std::string& name, bool recordUndo) {
    if (layer == nullptr) {
        return false;
    }
    LayerLocator loc = LocatorOf(layer);
    if (FindLayer(_seq, loc) != layer) {
        layer->SetLayerName(name);
        return true;
    }
    return ExecuteInStep(std::make_unique<SetLayerNameCommand>(loc, name), recordUndo).Ok();
}

void CommandExecutor::RecordAppliedEdits(const std::string& label, std::vector<std::unique_ptr<SequenceCommand>> applied,
                                         std::vector<std::unique_ptr<SequenceCommand>> inverses, bool recordUndo) {
    if (applied.empty() || applied.size() != inverses.size()) {
        return;
    }
    BeginBatch(label);
    for (size_t i = 0; i < applied.size(); ++i) {
        RecordApplied(std::move(applied[i]), std::move(inverses[i]), recordUndo);
    }
    EndBatch();
}

void CommandLogRecorder::OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) {
    nlohmann::json j;
    applied.Serialize(j);
    _log.push_back(std::move(j));
}

// ---------------------------------------------------------------------------
// Live preview
// ---------------------------------------------------------------------------

void CommandExecutor::BeginPreview() {
    ++_previewDepth;
}

bool CommandExecutor::PreviewAbsorbs(const SequenceCommand& cmd) const {
    switch (cmd.Type()) {
    case CommandType::MoveEffect:
    case CommandType::ModifyEffect:
        return true;
    case CommandType::Batch: {
        const auto& members = static_cast<const BatchCommand&>(cmd).GetCommands();
        return std::all_of(members.begin(), members.end(), [this](const auto& m) { return PreviewAbsorbs(*m); });
    }
    default:
        return false;
    }
}

void CommandExecutor::PreviewTouch(const SequenceCommand& cmd) {
    switch (cmd.Type()) {
    case CommandType::MoveEffect:
        PreviewTouchEffect(FindEffect(_seq, static_cast<const MoveEffectCommand&>(cmd).Target()));
        break;
    case CommandType::ModifyEffect:
        PreviewTouchEffect(FindEffect(_seq, static_cast<const ModifyEffectCommand&>(cmd).Target()));
        break;
    case CommandType::Batch:
        if (PreviewAbsorbs(cmd)) {
            for (const auto& m : static_cast<const BatchCommand&>(cmd).GetCommands()) {
                PreviewTouch(*m);
            }
        }
        break;
    default:
        break;
    }
}

void CommandExecutor::PreviewTouchEffect(const Effect* effect) {
    if (effect == nullptr || !InSequence(effect) || _previewIndex.count(effect->GetUid())) {
        return;
    }
    PreviewSnapshot snap;
    snap.loc = LocatorOf(effect);
    snap.name = effect->GetEffectName();
    snap.startTimeMS = effect->GetStartTimeMS();
    snap.endTimeMS = effect->GetEndTimeMS();
    snap.settings = effect->GetSettings();
    snap.palette = effect->GetPaletteMap();
    _previewIndex[effect->GetUid()] = _previewSnapshots.size();
    _previewSnapshots.push_back(std::move(snap));
}

bool CommandExecutor::CommitPreview(const std::string& label) {
    if (_previewDepth == 0 || --_previewDepth > 0) {
        return false;
    }
    std::vector<PreviewSnapshot> snaps = std::move(_previewSnapshots);
    std::vector<PreviewApplied> members = std::move(_previewOther);
    _previewSnapshots.clear();
    _previewIndex.clear();
    _previewOther.clear();

    for (const PreviewSnapshot& snap : snaps) {
        const Effect* e = FindEffect(_seq, snap.loc);
        if (e == nullptr) {
            continue; // deleted during the preview; the delete is in `members`
        }
        if (e->GetStartTimeMS() != snap.startTimeMS || e->GetEndTimeMS() != snap.endTimeMS) {
            members.push_back({ std::make_unique<MoveEffectCommand>(snap.loc, e->GetStartTimeMS(), e->GetEndTimeMS()),
                                std::make_unique<MoveEffectCommand>(snap.loc, snap.startTimeMS, snap.endTimeMS), true });
        }
        KeyChanges settings = Diff(snap.settings, e->GetSettings());
        KeyChanges palette = Diff(snap.palette, e->GetPaletteMap());
        std::optional<std::string> name, priorName;
        if (e->GetEffectName() != snap.name) {
            name = e->GetEffectName();
            priorName = snap.name;
        }
        if (name.has_value() || !settings.empty() || !palette.empty()) {
            KeyChanges priorSettings = Diff(e->GetSettings(), snap.settings);
            KeyChanges priorPalette = Diff(e->GetPaletteMap(), snap.palette);
            members.push_back({ std::make_unique<ModifyEffectCommand>(snap.loc, std::move(name), SettingsChange{ false, std::move(settings) },
                                                                      SettingsChange{ false, std::move(palette) }),
                                std::make_unique<ModifyEffectCommand>(snap.loc, std::move(priorName), SettingsChange{ false, std::move(priorSettings) },
                                                                      SettingsChange{ false, std::move(priorPalette) }),
                                true });
        }
    }
    if (members.empty()) {
        return false;
    }
    std::string stepLabel = label;
    if (stepLabel.empty()) {
        bool allMoves = std::all_of(members.begin(), members.end(), [](const PreviewApplied& m) { return m.applied->Type() == CommandType::MoveEffect; });
        stepLabel = allMoves ? "Effect(s) Moved" : "Effect(s) Modified";
    }
    if (_batchDepth > 0) {
        for (auto& m : members) {
            RecordApplied(std::move(m.applied), std::move(m.inverse), m.recordUndo);
        }
        return true;
    }
    bool anyUndo = std::any_of(members.begin(), members.end(), [](const PreviewApplied& m) { return m.recordUndo; });
    if (anyUndo) {
        _undo.CreateUndoStep();
        for (const auto& m : members) {
            if (m.recordUndo) {
                _undo.RecordInverse(CommandType::Batch, stepLabel, m.inverse->Clone());
            }
        }
    }
    BatchCommand batch(stepLabel);
    std::vector<std::unique_ptr<SequenceCommand>> reversed;
    for (auto it = members.rbegin(); it != members.rend(); ++it) {
        reversed.push_back(std::move(it->inverse));
    }
    for (auto& m : members) {
        batch.Add(std::move(m.applied));
    }
    Notify(batch, BatchCommand(stepLabel, std::move(reversed)));
    return true;
}

void CommandExecutor::CancelPreview() {
    if (_previewDepth == 0 || --_previewDepth > 0) {
        return;
    }
    std::vector<PreviewSnapshot> snaps = std::move(_previewSnapshots);
    std::vector<PreviewApplied> others = std::move(_previewOther);
    _previewSnapshots.clear();
    _previewIndex.clear();
    _previewOther.clear();
    // Undo the other commands, newest first, then put the touched effects
    // back as they were.
    for (auto it = others.rbegin(); it != others.rend(); ++it) {
        it->inverse->Apply(_seq, ApplyMode::Lenient);
    }
    for (const PreviewSnapshot& snap : snaps) {
        const Effect* e = FindEffect(_seq, snap.loc);
        if (e == nullptr) {
            continue;
        }
        if (e->GetStartTimeMS() != snap.startTimeMS || e->GetEndTimeMS() != snap.endTimeMS) {
            MoveEffectCommand(snap.loc, snap.startTimeMS, snap.endTimeMS).Apply(_seq, ApplyMode::Lenient);
        }
        KeyChanges settings = Diff(e->GetSettings(), snap.settings);
        KeyChanges palette = Diff(e->GetPaletteMap(), snap.palette);
        std::optional<std::string> name;
        if (e->GetEffectName() != snap.name) {
            name = snap.name;
        }
        if (name.has_value() || !settings.empty() || !palette.empty()) {
            ModifyEffectCommand(snap.loc, std::move(name), SettingsChange{ false, std::move(settings) }, SettingsChange{ false, std::move(palette) })
                .Apply(_seq, ApplyMode::Lenient);
        }
    }
}

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
#include "SequenceElements.h"
#include "SequenceMutationGuard.h"
#include "UndoManager.h"
#include "../utils/UtilClasses.h"

#include <log.h>

CommandExecutor::CommandExecutor(SequenceElements& seq, UndoManager& undo) :
    _seq(seq), _undo(undo) {
}

CommandResult CommandExecutor::Execute(std::unique_ptr<SequenceCommand> cmd) {
    if (!cmd) {
        return CommandResult::Fail(CommandStatus::Invalid, "no command");
    }
    CommandResult result;
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(_seq, ApplyMode::Strict, &result);
    if (!inverse) {
        return result;
    }
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

void CommandExecutor::EndBatch() {
    if (_batchDepth == 0) {
        return;
    }
    if (--_batchDepth > 0) {
        return;
    }
    std::unique_ptr<BatchCommand> batch = std::move(_batch);
    std::unique_ptr<BatchCommand> inverses = std::move(_batchInverse);
    std::vector<std::unique_ptr<SequenceCommand>> undo = std::move(_batchUndo);
    _batchUndo.clear();
    if (batch->IsEmpty()) {
        return;
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
}

void CommandExecutor::AddObserver(ICommandObserver* observer) {
    if (observer != nullptr && std::find(_observers.begin(), _observers.end(), observer) == _observers.end()) {
        _observers.push_back(observer);
    }
}

void CommandExecutor::RemoveObserver(ICommandObserver* observer) {
    _observers.erase(std::remove(_observers.begin(), _observers.end(), observer), _observers.end());
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

void CommandExecutor::RecordApplied(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo) {
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

CommandResult CommandExecutor::ExecuteInStep(std::unique_ptr<SequenceCommand> cmd, bool recordUndo) {
    if (!cmd) {
        return CommandResult::Fail(CommandStatus::Invalid, "no command");
    }
    CommandResult strict = cmd->Validate(_seq, ApplyMode::Strict);
    CommandResult result;
    std::unique_ptr<SequenceCommand> inverse = cmd->Apply(_seq, ApplyMode::Lenient, &result);
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
    if (!ExecuteInStep(std::move(cmd), recordUndo).Ok()) {
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
    if (!ExecuteInStep(std::move(cmd), recordUndo).Ok()) {
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

void CommandLogRecorder::OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) {
    nlohmann::json j;
    applied.Serialize(j);
    _log.push_back(std::move(j));
}

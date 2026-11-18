#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "SequenceCommands.h"

class SequenceElements;
class UndoManager;
class Effect;
class EffectLayer;
class Element;

class ICommandObserver {
public:
    virtual ~ICommandObserver() = default;
    // `applied` is the command as executed (a BatchCommand for a batch),
    // `inverse` the command that undoes it.
    virtual void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand& inverse) = 0;
};

// The single entry point for edits to a sequence's effects and layers:
// validates (strict), applies, records the inverse for undo and tells the
// observers.
//
// Outside a batch every Execute is its own undo step. Between BeginBatch and
// EndBatch (which nest) commands still apply immediately, so the UI sees
// each change, but they form one undo step and reach observers as one
// BatchCommand when the outermost EndBatch runs. Inner batch labels are not
// kept: the outer BatchCommand lists the members flat, in order. A batch
// with no successful members records no undo step and notifies nobody. A
// command rejected inside a batch is simply not added; the batch goes on.
//
// Undo and redo (UndoManager) apply recorded inverses directly and do not
// notify observers.
class CommandExecutor {
public:
    CommandExecutor(SequenceElements& seq, UndoManager& undo);
    CommandExecutor(const CommandExecutor&) = delete;
    CommandExecutor& operator=(const CommandExecutor&) = delete;

    CommandResult Execute(std::unique_ptr<SequenceCommand> cmd);

    void BeginBatch(const std::string& label);
    void EndBatch();
    bool InBatch() const { return _batchDepth > 0; }

    void AddObserver(ICommandObserver* observer);
    void RemoveObserver(ICommandObserver* observer);

    // ---- Edit-site API ---------------------------------------------------
    // For existing edit code that groups its own undo steps
    // (UndoManager::CreateUndoStep) and used to call UndoManager::Capture*
    // next to a direct mutation. These apply the change as a command in
    // lenient mode, so it behaves exactly like the mutation it replaces,
    // record the inverse into the caller's *current* undo step (unless
    // recordUndo is false) and notify observers. A change strict validation
    // would have refused still applies and is logged at debug level.
    //
    // Objects that aren't part of this sequence are changed directly, as
    // before, with no undo or notification.

    CommandResult ExecuteInStep(std::unique_ptr<SequenceCommand> cmd, bool recordUndo = true);

    Effect* AddEffect(EffectLayer* layer, const std::string& name, const std::string& settings, const std::string& palette,
                      int startTimeMS, int endTimeMS, int selected, bool isProtected, bool recordUndo = true);
    bool DeleteEffect(Effect* effect, bool recordUndo = true);
    bool MoveEffect(Effect* effect, int startTimeMS, int endTimeMS, bool recordUndo = true);
    bool ModifyEffect(Effect* effect, std::optional<std::string> effectName, SettingsChange settings, SettingsChange palette,
                      bool recordUndo = true);
    // Effect::SetSettings / SetPalette equivalents (whole-map replacement).
    bool SetEffectSettings(Effect* effect, const std::string& settings, bool keepXSettings, bool recordUndo = true);
    bool SetEffectPalette(Effect* effect, const std::string& palette, bool recordUndo = true);
    // std::nullopt removes the key.
    bool SetEffectSetting(Effect* effect, const std::string& key, std::optional<std::string> value, bool recordUndo = true);
    bool SetEffectLocked(Effect* effect, bool locked, bool recordUndo = true);
    bool SetEffectRenderDisabled(Effect* effect, bool disabled, bool recordUndo = true);

    // Runs `mutate` on the effect directly (it may change the effect's type,
    // settings, palette and times), then records what changed as commands.
    // For edits whose logic lives in Effect / RenderableEffect (value curves,
    // button presses, colour replacement, ...). Returns true if anything
    // changed.
    bool EditEffect(Effect* effect, const std::function<void(Effect&)>& mutate, bool recordUndo = true);

    // `index` >= the layer count appends.
    EffectLayer* InsertLayer(Element* element, int index, bool recordUndo = true);
    bool RemoveLayer(Element* element, int index, bool recordUndo = true);
    bool SetLayerName(EffectLayer* layer, const std::string& name, bool recordUndo = true);

private:
    void Notify(const SequenceCommand& applied, const SequenceCommand& inverse);
    // Records an already-applied command: inverse into the current undo step
    // (or the open batch), then observers.
    void RecordApplied(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo);
    bool InSequence(const Effect* effect) const;

    SequenceElements& _seq;
    UndoManager& _undo;
    std::vector<ICommandObserver*> _observers;

    int _batchDepth = 0;
    std::unique_ptr<BatchCommand> _batch;        // applied members, in order
    std::unique_ptr<BatchCommand> _batchInverse; // their inverses, in order
    std::vector<std::unique_ptr<SequenceCommand>> _batchUndo; // inverses to record for undo, in order
};

// Records every applied command as JSON, e.g. to replay an edit session.
class CommandLogRecorder : public ICommandObserver {
public:
    void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand& inverse) override;
    const nlohmann::json& GetLog() const { return _log; }
    void Clear() { _log = nlohmann::json::array(); }

private:
    nlohmann::json _log = nlohmann::json::array();
};

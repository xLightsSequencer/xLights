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
#include <unordered_map>
#include <utility>
#include <vector>

#include "SequenceCommands.h"
#include "../utils/UtilClasses.h"

class SequenceElements;
class UndoManager;
class Effect;
class EffectLayer;
class Element;
class SequenceEditCapture;

class ICommandObserver {
public:
    virtual ~ICommandObserver() = default;
    // `applied` is the command as executed (a BatchCommand for a batch),
    // `inverse` the command that undoes it.
    virtual void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand& inverse) = 0;
};

// Decides whether an edit may be made at all (a live session refuses a
// viewer's edits and edits to rows assigned to someone else).
class ICommandGate {
public:
    virtual ~ICommandGate() = default;
    // False, with readable text in `why`, to refuse `cmd` before it applies.
    virtual bool AllowCommand(const SequenceCommand& cmd, std::string& why) = 0;
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
//
// With a gate set, Execute, ExecuteInStep and the edit-site helpers that
// return bool ask it first and change nothing when it refuses
// (CommandStatus::Refused). AddEffect and InsertLayer don't ask: their
// callers expect the object they make, so a refused add applies and an
// observer that enforces the gate takes it back out (the live session does,
// when it sends the edit). Nor can edits that are already made (EditEffect's
// mutation runs after the check; RecordExternal, RecordAppliedEdits and
// captures report changes already in the document).
class CommandExecutor {
public:
    CommandExecutor(SequenceElements& seq, UndoManager& undo);
    CommandExecutor(const CommandExecutor&) = delete;
    CommandExecutor& operator=(const CommandExecutor&) = delete;

    CommandResult Execute(std::unique_ptr<SequenceCommand> cmd);

    void BeginBatch(const std::string& label);
    // True if closing the outermost batch recorded an undo step.
    bool EndBatch();
    bool InBatch() const { return _batchDepth > 0; }
    // A command is being applied right now.
    bool IsApplying() const { return _applyDepth > 0; }

    // For code that applies commands to the sequence itself, bypassing the
    // executor (a live session applying other participants' edits): while
    // one is alive the executor counts as applying, so the sequence reports
    // an edit in progress (hosts don't yield to the event loop or start
    // another edit) and a row deleted meanwhile isn't taken for a direct
    // delete.
    class ExternalApplyScope {
    public:
        explicit ExternalApplyScope(CommandExecutor& e) : _exec(e) { ++_exec._applyDepth; }
        ~ExternalApplyScope() { --_exec._applyDepth; }
        ExternalApplyScope(const ExternalApplyScope&) = delete;
        ExternalApplyScope& operator=(const ExternalApplyScope&) = delete;

    private:
        CommandExecutor& _exec;
    };

    // ---- Live preview ----------------------------------------------------
    // For interactive edits (dragging an effect's edges, nudging a
    // selection) that change effects many times before the user lets go.
    // Between BeginPreview and CommitPreview every edit still applies at
    // once, so the grid shows it live, but nothing is recorded or sent to
    // observers. Instead the executor remembers each effect's state the
    // first time a move or modify touches it. CommitPreview then records
    // the net change of every touched effect (MoveEffect / ModifyEffect
    // against that remembered state) as one labelled step: one undo step
    // and one BatchCommand to observers. Other commands applied during a
    // preview (adds, deletes, layers) are kept in order and go first in
    // that batch. CancelPreview puts everything back, silently.
    //
    // Previews nest; only the outermost Commit/Cancel acts. Inside a batch,
    // the committed changes join the batch instead of making a step. An
    // empty label is "Effect(s) Moved" when every change is a move, else
    // "Effect(s) Modified".
    void BeginPreview();
    // Returns true if anything was recorded.
    bool CommitPreview(const std::string& label);
    void CancelPreview();
    bool InPreview() const { return _previewDepth > 0; }

    void AddObserver(ICommandObserver* observer);
    void RemoveObserver(ICommandObserver* observer);
    // Told of each command as it is recorded, at once, even inside a batch
    // or preview (where observers hear of it later, or only of the net
    // change). A command may be reported more than once.
    void AddRecordListener(ICommandObserver* listener);
    void RemoveRecordListener(ICommandObserver* listener);
    void SetGate(ICommandGate* gate) { _gate = gate; }
    ICommandGate* GetGate() const { return _gate; }
    // True when there is no gate or it lets `cmd` apply.
    bool GateAllows(const SequenceCommand& cmd, std::string* why = nullptr);

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

    CommandResult ExecuteInStep(std::unique_ptr<SequenceCommand> cmd, bool recordUndo = true) {
        return ExecuteInStep(std::move(cmd), recordUndo, true);
    }

    // Records a change the caller already made directly (`applied`, undone
    // by `inverse`) exactly as ExecuteInStep would have recorded it.
    void RecordExternal(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo = true) {
        RecordApplied(std::move(applied), std::move(inverse), recordUndo);
    }

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

    // Records edits that were already made to the document (see
    // SequenceEditCapture): applied[i] has been applied and inverses[i]
    // undoes it. They form one undo step named `label` (or join the open
    // batch) and reach observers as one batch.
    void RecordAppliedEdits(const std::string& label, std::vector<std::unique_ptr<SequenceCommand>> applied,
                            std::vector<std::unique_ptr<SequenceCommand>> inverses, bool recordUndo = true);

    // True while a SequenceEditCapture is open. Effect and layer commands
    // applied meanwhile are neither recorded nor reported; the capture
    // records the net change. Other commands (elements, media, ...) are
    // recorded as usual, into the capture's batch.
    bool IsCapturing() const { return _capture != nullptr; }

    // SequenceElements calls this before deleting an element directly (not
    // while applying a command), so an open capture can record it.
    void DirectElementDelete(Element* element, bool keepInViews);

private:
    friend class SequenceEditCapture;
    // A command (or a batch of only those) for state a capture compares and
    // records itself.
    static bool CapturedKind(const SequenceCommand& cmd);

    void Notify(const SequenceCommand& applied, const SequenceCommand& inverse);
    void NotifyRecorded(const SequenceCommand& applied, const SequenceCommand& inverse);
    CommandResult ExecuteInStep(std::unique_ptr<SequenceCommand> cmd, bool recordUndo, bool gated);
    // Records an already-applied command: inverse into the current undo step
    // (or the open batch), then observers. While capturing, commands the
    // capture records itself are dropped, and anything added directly is
    // recorded ahead of the others.
    void RecordApplied(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo);
    void RecordNow(std::unique_ptr<SequenceCommand> applied, std::unique_ptr<SequenceCommand> inverse, bool recordUndo);

    struct ApplyingScope {
        explicit ApplyingScope(CommandExecutor& e) : exec(e) { ++exec._applyDepth; }
        ~ApplyingScope() { --exec._applyDepth; }
        CommandExecutor& exec;
    };
    int _applyDepth = 0;
    bool InSequence(const Effect* effect) const;
    // Live preview bookkeeping.
    void PreviewTouch(const SequenceCommand& cmd);
    void PreviewTouchEffect(const Effect* effect);
    bool PreviewAbsorbs(const SequenceCommand& cmd) const;

    SequenceElements& _seq;
    UndoManager& _undo;
    std::vector<ICommandObserver*> _observers;
    std::vector<ICommandObserver*> _recordListeners;
    ICommandGate* _gate = nullptr;

    int _batchDepth = 0;
    SequenceEditCapture* _capture = nullptr; // the outermost open capture
    std::unique_ptr<BatchCommand> _batch;        // applied members, in order
    std::unique_ptr<BatchCommand> _batchInverse; // their inverses, in order
    std::vector<std::unique_ptr<SequenceCommand>> _batchUndo; // inverses to record for undo, in order

    struct PreviewSnapshot {
        EffectLocator loc;
        std::string name;
        int startTimeMS = 0;
        int endTimeMS = 0;
        SettingsMap settings;
        SettingsMap palette;
    };
    struct PreviewApplied {
        std::unique_ptr<SequenceCommand> applied;
        std::unique_ptr<SequenceCommand> inverse;
        bool recordUndo = true;
    };
    int _previewDepth = 0;
    std::vector<PreviewSnapshot> _previewSnapshots; // first-touch order
    std::unordered_map<SeqObjectId, size_t> _previewIndex; // effect uid -> snapshot
    std::vector<PreviewApplied> _previewOther;
};

// Groups the edits made during its lifetime into one CommandExecutor batch
// (one undo step, one notification; inside an open batch it joins that one).
class ScopedCommandBatch {
public:
    ScopedCommandBatch(CommandExecutor& exec, const std::string& label) :
        _exec(exec) {
        _exec.BeginBatch(label);
    }
    // The sequence's executor.
    ScopedCommandBatch(SequenceElements& seq, const std::string& label);
    ~ScopedCommandBatch() { _exec.EndBatch(); }
    ScopedCommandBatch(const ScopedCommandBatch&) = delete;
    ScopedCommandBatch& operator=(const ScopedCommandBatch&) = delete;

private:
    CommandExecutor& _exec;
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

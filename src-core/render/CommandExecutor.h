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

#include <memory>
#include <string>
#include <vector>

#include "SequenceCommands.h"

class SequenceElements;
class UndoManager;

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

private:
    void Notify(const SequenceCommand& applied, const SequenceCommand& inverse);

    SequenceElements& _seq;
    UndoManager& _undo;
    std::vector<ICommandObserver*> _observers;

    int _batchDepth = 0;
    std::unique_ptr<BatchCommand> _batch;        // applied members, in order
    std::unique_ptr<BatchCommand> _batchInverse; // their inverses, in order
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

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

#include "SequenceElements.h"
#include "UndoManager.h"

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
    if (batch->IsEmpty()) {
        return;
    }
    // One undo step whose entries are the members' inverses in the order the
    // members ran; undo walks them backwards.
    _undo.CreateUndoStep();
    for (const auto& inv : inverses->GetCommands()) {
        _undo.RecordInverse(CommandType::Batch, batch->GetLabel(), inv->Clone());
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

void CommandLogRecorder::OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) {
    nlohmann::json j;
    applied.Serialize(j);
    _log.push_back(std::move(j));
}

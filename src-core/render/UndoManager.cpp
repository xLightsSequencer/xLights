/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "UndoManager.h"
#include "../render/Effect.h"
#include "../render/EffectLayer.h"
#include "../render/Element.h"
#include "../render/SequenceCommands.h"
#include "../render/SequenceElementCommands.h"
#include "../render/SequenceElements.h"
#include <log.h>

namespace {

UNDO_ACTIONS KindFor(CommandType appliedType) {
    switch (appliedType) {
    case CommandType::AddEffect: return UNDO_EFFECT_ADDED;
    case CommandType::DeleteEffect: return UNDO_EFFECT_DELETED;
    case CommandType::MoveEffect: return UNDO_EFFECT_MOVED;
    case CommandType::ModifyEffect:
    case CommandType::SetLayerName:
    case CommandType::ReplaceLayerContents: return UNDO_EFFECT_MODIFIED;
    case CommandType::AddLayer: return UNDO_LAYER_ADDED;
    case CommandType::RemoveLayer: return UNDO_LAYER_REMOVED;
    case CommandType::PutMedia:
    case CommandType::RemoveMedia:
    case CommandType::RenameMedia: return UNDO_EFFECT_MODIFIED;
    case CommandType::Batch:
    case CommandType::AddElement:
    case CommandType::DeleteElement:
    case CommandType::MoveElement:
    case CommandType::RenameTimingTrack:
    case CommandType::SetTimingFixed:
    case CommandType::SetTag:
    case CommandType::SetSequenceFace:
    case CommandType::SetSongStructure:
    case CommandType::SetElementRenderDisabled:
    case CommandType::RenameModelElement:
    case CommandType::SetEffectSymbol:
    case CommandType::SetEffectSymbolLink: return UNDO_BATCH;
    }
    return UNDO_EFFECT_MODIFIED;
}

// Undoing an add leaves a delete to redo, and so on.
UNDO_ACTIONS Opposite(UNDO_ACTIONS kind) {
    switch (kind) {
    case UNDO_EFFECT_ADDED: return UNDO_EFFECT_DELETED;
    case UNDO_EFFECT_DELETED: return UNDO_EFFECT_ADDED;
    case UNDO_LAYER_ADDED: return UNDO_LAYER_REMOVED;
    case UNDO_LAYER_REMOVED: return UNDO_LAYER_ADDED;
    default: return kind;
    }
}

std::string Describe(const std::string& prefix, UNDO_ACTIONS kind, const std::string& label) {
    switch (kind) {
    case UNDO_EFFECT_DELETED: return prefix + ": Effect(s) Deleted";
    case UNDO_EFFECT_ADDED: return prefix + ": Effect(s) Added";
    case UNDO_EFFECT_MOVED: return prefix + ": Effect(s) Moved";
    case UNDO_EFFECT_MODIFIED: return prefix + ": Effect(s) Modified";
    case UNDO_LAYER_ADDED: return prefix + ": Layer(s) Added";
    case UNDO_LAYER_REMOVED: return prefix + ": Layer(s) Removed";
    case UNDO_BATCH: return label.empty() ? prefix : prefix + ": " + label;
    case UNDO_MARKER: break;
    }
    return prefix;
}

} // namespace

UndoManager::UndoManager(SequenceElements* parent)
: mParentSequence(parent), mCaptureUndo(false)
{
}

UndoManager::~UndoManager() = default;

void UndoManager::SetCaptureUndo( bool value )
{
    mCaptureUndo = value;
}

void UndoManager::RemoveUnusedMarkers()
{
    // delete any marker stragglers
    if (!mUndoSteps.empty() && mUndoSteps.back().kind == UNDO_MARKER) {
        mUndoSteps.pop_back();
        ++mHistorySerial;
    }
}

bool UndoManager::ChangeCaptured()
{
    return !mUndoSteps.empty() && mUndoSteps.back().kind != UNDO_MARKER;
}

void UndoManager::Clear() {
    ++mHistorySerial;
    mUndoSteps.clear();
    ClearRedo();
}

void UndoManager::ClearRedo() {
    mRedoSteps.clear();
}

bool UndoManager::CanUndo()
{
    RemoveUnusedMarkers();
    return !mUndoSteps.empty();
}

bool UndoManager::CanRedo()
{
    return !mRedoSteps.empty();
}

void UndoManager::Push(UNDO_ACTIONS kind, std::unique_ptr<SequenceCommand> inverse, const std::string& label)
{
    Entry e;
    e.kind = kind;
    e.label = label;
    e.inverse = std::move(inverse);
    mUndoSteps.push_back(std::move(e));
    ++mHistorySerial;
    EnforceMaxSteps();
}

void UndoManager::CreateUndoStep()
{
    ++mStepSerial;
    ClearRedo();
    RemoveUnusedMarkers();
    Push(UNDO_MARKER, nullptr);
}

void UndoManager::RecordInverse(CommandType appliedType, const std::string& label, std::unique_ptr<SequenceCommand> inverse)
{
    Push(KindFor(appliedType), std::move(inverse), label.empty() ? std::string(ElementCommandUndoLabel(appliedType)) : label);
}

void UndoManager::EnforceMaxSteps()
{
    if (mMaxSteps == 0) return;  // cap disabled
    while (mUndoSteps.size() > mMaxSteps) {
        mUndoSteps.erase(mUndoSteps.begin());
    }
}

void UndoManager::CancelLastStep() {
    if (!mUndoSteps.empty()) {
        mUndoSteps.pop_back();
        ++mHistorySerial;
    }
}

void UndoManager::UndoLastStep()
{
    ++mHistorySerial;
    mRedoSteps.push_back(Entry{});
    ProcessUndoStep(mUndoSteps, mRedoSteps);
}

void UndoManager::RedoLastStep()
{
    ++mHistorySerial;
    mUndoSteps.push_back(Entry{});
    ProcessUndoStep(mRedoSteps, mUndoSteps);
}

void UndoManager::ProcessUndoStep(std::vector<Entry> &fromList, std::vector<Entry> &toList)
{
    while (!fromList.empty()) {
        Entry entry = std::move(fromList.back());
        fromList.pop_back();
        if (entry.kind == UNDO_MARKER) {
            break;
        }
        if (!entry.inverse) {
            continue;
        }
        CommandResult result;
        std::unique_ptr<SequenceCommand> redo = entry.inverse->Apply(*mParentSequence, ApplyMode::Lenient, &result);
        if (!redo) {
            spdlog::warn("UndoManager: could not {} {}: {}", (&fromList == &mUndoSteps) ? "undo" : "redo",
                         CommandTypeName(entry.inverse->Type()), result.message);
            continue;
        }
        Entry back;
        back.kind = Opposite(entry.kind);
        back.label = entry.label;
        back.inverse = std::move(redo);
        toList.push_back(std::move(back));
    }
}

std::string UndoManager::GetUndoString()
{
    if (mUndoSteps.empty()) {
        return "Undo";
    }
    return Describe("Undo", mUndoSteps.back().kind, mUndoSteps.back().label);
}

std::string UndoManager::GetRedoString()
{
    if (mRedoSteps.empty()) {
        return "Redo";
    }
    return Describe("Redo", mRedoSteps.back().kind, mRedoSteps.back().label);
}

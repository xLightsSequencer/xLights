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
    case CommandType::SetSongStructure: return UNDO_BATCH;
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

EffectLayer* UndoManager::ResolveLayer(const std::string& element_name, int exclusive_layer_index, LayerLocator& loc) const
{
    Element* element = mParentSequence->GetElement(element_name);
    if (element == nullptr) {
        return nullptr;
    }
    EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(exclusive_layer_index);
    if (el != nullptr) {
        loc.element = element->GetUid();
        loc.layer = el->GetUid();
    }
    return el;
}

// Each capture is stored as the command that undoes it. When the target
// can't be found at capture time, an empty entry keeps the step's shape (so
// CancelLastStep and ChangeCaptured behave as before) and undoes nothing.

void UndoManager::CaptureEffectToBeDeleted( const std::string &element_name, int layer_index, const std::string &name, const std::string &settings,
                                            const std::string &palette, int startTimeMS, int endTimeMS, int Selected, bool Protected )
{
    LayerLocator loc;
    EffectLayer* el = ResolveLayer(element_name, layer_index, loc);
    if (el == nullptr) {
        Push(UNDO_EFFECT_DELETED, nullptr);
        return;
    }
    EffectState state;
    // Callers capture before removing the effect, so it is still in the layer;
    // keep its identity so undo restores it and earlier steps naming it still
    // resolve.
    for (Effect* e : el->GetEffects()) {
        if (e->GetStartTimeMS() == startTimeMS && e->GetEndTimeMS() == endTimeMS && e->GetEffectName() == name) {
            state.uid = e->GetUid();
            break;
        }
    }
    state.name = name;
    state.settings = settings;
    state.palette = palette;
    state.startTimeMS = startTimeMS;
    state.endTimeMS = endTimeMS;
    state.isProtected = Protected;
    state.selected = Selected;
    Push(UNDO_EFFECT_DELETED, std::make_unique<AddEffectCommand>(loc, std::move(state)));
}

void UndoManager::CaptureAddedEffect( const std::string &element_name, int layer_index, int id )
{
    LayerLocator loc;
    EffectLayer* el = ResolveLayer(element_name, layer_index, loc);
    Effect* e = el == nullptr ? nullptr : el->GetEffectFromID(id);
    if (e == nullptr) {
        Push(UNDO_EFFECT_ADDED, nullptr);
        return;
    }
    Push(UNDO_EFFECT_ADDED, std::make_unique<DeleteEffectCommand>(EffectLocator{ loc.element, loc.layer, e->GetUid() }));
}

void UndoManager::CaptureEffectToBeMoved( const std::string &element_name, int layer_index, int id, int startTimeMS, int endTimeMS )
{
    LayerLocator loc;
    EffectLayer* el = ResolveLayer(element_name, layer_index, loc);
    Effect* e = el == nullptr ? nullptr : el->GetEffectFromID(id);
    if (e == nullptr) {
        Push(UNDO_EFFECT_MOVED, nullptr);
        return;
    }
    Push(UNDO_EFFECT_MOVED, std::make_unique<MoveEffectCommand>(EffectLocator{ loc.element, loc.layer, e->GetUid() }, startTimeMS, endTimeMS));
}

void UndoManager::CaptureModifiedEffect( const std::string &element_name, int layer_index, int id, const std::string &settings, const std::string &palette )
{
    LayerLocator loc;
    EffectLayer* el = ResolveLayer(element_name, layer_index, loc);
    Effect* e = el == nullptr ? nullptr : el->GetEffectFromID(id);
    if (e == nullptr) {
        Push(UNDO_EFFECT_MODIFIED, nullptr);
        return;
    }
    Push(UNDO_EFFECT_MODIFIED, std::make_unique<ModifyEffectCommand>(EffectLocator{ loc.element, loc.layer, e->GetUid() }, std::nullopt,
                                                                     SettingsChange::ReplaceWith(settings), SettingsChange::ReplaceWith(palette)));
}

void UndoManager::CaptureModifiedEffect( const std::string &element_name, int layer_index, Effect *ef )
{
    LayerLocator loc;
    EffectLayer* el = ResolveLayer(element_name, layer_index, loc);
    if (el == nullptr || ef == nullptr) {
        Push(UNDO_EFFECT_MODIFIED, nullptr);
        return;
    }
    std::optional<std::string> name;
    if (ef->GetEffectIndex() >= 0) {
        name = ef->GetEffectName();
    }
    Push(UNDO_EFFECT_MODIFIED, std::make_unique<ModifyEffectCommand>(EffectLocator{ loc.element, loc.layer, ef->GetUid() }, std::move(name),
                                                                     SettingsChange::ReplaceWith(ef->GetSettingsAsString()),
                                                                     SettingsChange::ReplaceWith(ef->GetPaletteAsString())));
}

void UndoManager::EnforceMaxSteps()
{
    if (mMaxSteps == 0) return;  // cap disabled
    while (mUndoSteps.size() > mMaxSteps) {
        mUndoSteps.erase(mUndoSteps.begin());
    }
}

void UndoManager::CaptureAddedLayer( const std::string &element_name, int exclusive_layer_index, int layer_number )
{
    LayerLocator loc;
    EffectLayer* el = ResolveLayer(element_name, exclusive_layer_index, loc);
    if (el == nullptr || el->GetParentElement() == nullptr) {
        Push(UNDO_LAYER_ADDED, nullptr);
        return;
    }
    // Remove it from the element that owns it: a model-level name also finds
    // submodel and strand layers.
    Push(UNDO_LAYER_ADDED, std::make_unique<RemoveLayerCommand>(el->GetParentElement()->GetUid(), el->GetUid()));
}

void UndoManager::CaptureRemovedLayer( const std::string &element_name, int layer_number )
{
    // Captured before the layer is removed: record enough to re-create it
    // under the same id, in the same place, with its name and contents.
    Element* element = mParentSequence->GetElement(element_name);
    EffectLayer* el = element == nullptr ? nullptr : element->GetEffectLayer(layer_number);
    if (el == nullptr) {
        Push(UNDO_LAYER_REMOVED, nullptr);
        return;
    }
    SeqObjectId after = layer_number > 0 ? element->GetEffectLayer(layer_number - 1)->GetUid() : kNoSeqObjectId;
    std::vector<EffectState> effects;
    for (const Effect* e : el->GetEffects()) {
        effects.push_back(EffectState::Of(*e));
    }
    Push(UNDO_LAYER_REMOVED, std::make_unique<AddLayerCommand>(element->GetUid(), el->GetUid(), after, layer_number, el->GetLayerName(), std::move(effects)));
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

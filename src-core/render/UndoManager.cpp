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
#include "../render/Element.h"
#include "../render/SequenceElements.h"
#include <log.h>

DeletedEffectInfo::DeletedEffectInfo( const std::string &element_name_, int layer_index_, const std::string &name_, const std::string &settings_,
                                      const std::string &palette_, int startTimeMS_, int endTimeMS_, int Selected_, bool Protected_ )
: element_name(element_name_), layer_index(layer_index_), name(name_), settings(settings_),
  palette(palette_), startTimeMS(startTimeMS_), endTimeMS(endTimeMS_), Selected(Selected_), Protected(Protected_)
{
}

AddedEffectInfo::AddedEffectInfo( const std::string &element_name_, int layer_index_, int id_ )
: element_name(element_name_), layer_index(layer_index_), id(id_)
{
}

MovedEffectInfo::MovedEffectInfo( const std::string &element_name_, int layer_index_, int id_, int startTimeMS_, int endTimeMS_ )
: element_name(element_name_), layer_index(layer_index_), id(id_), startTimeMS(startTimeMS_), endTimeMS(endTimeMS_)
{
}

ModifiedEffectInfo::ModifiedEffectInfo( const std::string &element_name_, int layer_index_, int id_, const std::string &settings_, const std::string &palette_ )
: element_name(element_name_), layer_index(layer_index_), id(id_), settings(settings_), palette(palette_), effectName(""), effectType(-1)
{
}


ModifiedEffectInfo::ModifiedEffectInfo( const std::string &element_name_, int layer_index_, Effect *ef)
: element_name(element_name_), layer_index(layer_index_), id(ef->GetID()),
    settings(ef->GetSettingsAsString()), palette(ef->GetPaletteAsString()), effectName(ef->GetEffectName()), effectType(ef->GetEffectIndex())
{
}

UndoStep::UndoStep( UNDO_ACTIONS action )
: undo_action(action)
{
}

UndoStep::UndoStep( UNDO_ACTIONS action, DeletedEffectInfo* effect_info )
: undo_action(action)
{
    deleted_effect_info.push_back(effect_info);
}

UndoStep::UndoStep( UNDO_ACTIONS action, AddedEffectInfo* effect_info )
: undo_action(action)
{
    added_effect_info.push_back(effect_info);
}

UndoStep::UndoStep( UNDO_ACTIONS action, MovedEffectInfo* effect_info )
: undo_action(action)
{
    moved_effect_info.push_back(effect_info);
}

UndoStep::UndoStep( UNDO_ACTIONS action, ModifiedEffectInfo* effect_info )
: undo_action(action)
{
    modified_effect_info.push_back(effect_info);
}

UndoStep::UndoStep( UNDO_ACTIONS action, LayerInfo* li )
: undo_action(action)
{
    layer_info.push_back(li);
}

UndoStep::~UndoStep()
{
    for (auto* p : deleted_effect_info)  delete p;
    for (auto* p : added_effect_info)    delete p;
    for (auto* p : moved_effect_info)    delete p;
    for (auto* p : modified_effect_info) delete p;
    for (auto* p : layer_info)           delete p;
}

UndoManager::UndoManager(SequenceElements* parent)
: mParentSequence(parent), mCaptureUndo(false)
{
}

UndoManager::~UndoManager()
{
    for( size_t i = 0; i < mUndoSteps.size(); i++ )
    {
        delete mUndoSteps[i];
    }
}

void UndoManager::SetCaptureUndo( bool value )
{
    mCaptureUndo = value;
}

void UndoManager::RemoveUnusedMarkers()
{
    if( mUndoSteps.size() > 0 )
    {
        UndoStep* last_action = mUndoSteps.back();
        // delete any marker stragglers
        if( last_action->undo_action == UNDO_MARKER )
        {
            mUndoSteps.pop_back();
        }
    }
}

bool UndoManager::ChangeCaptured()
{
    if( mUndoSteps.size() > 0 )
    {
        UndoStep* last_action = mUndoSteps.back();
        if( last_action->undo_action != UNDO_MARKER )
        {
            return true;
        }
    }
    return false;
}

void UndoManager::Clear() {
    RemoveUnusedMarkers();
    for (size_t i = 0; i < mUndoSteps.size(); i++)
    {
        delete mUndoSteps[i];
    }
    mUndoSteps.clear();
    ClearRedo();
}

void UndoManager::ClearRedo() {
    for (size_t i = 0; i < mRedoSteps.size(); i++)
    {
        delete mRedoSteps[i];
    }
    mRedoSteps.clear();
}

bool UndoManager::CanUndo()
{
    RemoveUnusedMarkers();
    return mUndoSteps.size() > 0;
}

bool UndoManager::CanRedo()
{
    return mRedoSteps.size() > 0;
}

void UndoManager::CreateUndoStep()
{
    ClearRedo();
    RemoveUnusedMarkers();
    UndoStep* action = new UndoStep(UNDO_MARKER);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureEffectToBeDeleted( const std::string &element_name, int layer_index, const std::string &name, const std::string &settings,
                                            const std::string &palette, int startTimeMS, int endTimeMS, int Selected, bool Protected )
{
    DeletedEffectInfo* effect_undo_action = new DeletedEffectInfo( element_name, layer_index, name, settings, palette, startTimeMS, endTimeMS, Selected, Protected );
    UndoStep* action = new UndoStep(UNDO_EFFECT_DELETED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureAddedEffect( const std::string &element_name, int layer_index, int id )
{
    AddedEffectInfo* effect_undo_action = new AddedEffectInfo( element_name, layer_index, id );
    UndoStep* action = new UndoStep(UNDO_EFFECT_ADDED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureEffectToBeMoved( const std::string &element_name, int layer_index, int id, int startTimeMS, int endTimeMS )
{
    MovedEffectInfo* effect_undo_action = new MovedEffectInfo( element_name, layer_index, id, startTimeMS, endTimeMS );
    UndoStep* action = new UndoStep(UNDO_EFFECT_MOVED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureModifiedEffect( const std::string &element_name, int layer_index, int id, const std::string &settings, const std::string &palette )
{
    ModifiedEffectInfo* effect_undo_action = new ModifiedEffectInfo( element_name, layer_index, id, settings, palette );
    UndoStep* action = new UndoStep(UNDO_EFFECT_MODIFIED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureModifiedEffect( const std::string &element_name, int layer_index, Effect *ef )
{
    ModifiedEffectInfo* effect_undo_action = new ModifiedEffectInfo( element_name, layer_index, ef );
    UndoStep* action = new UndoStep(UNDO_EFFECT_MODIFIED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::EnforceMaxSteps()
{
    if (mMaxSteps == 0) return;  // cap disabled
    while (mUndoSteps.size() > mMaxSteps) {
        delete mUndoSteps.front();
        mUndoSteps.erase(mUndoSteps.begin());
    }
}
void UndoManager::CaptureAddedLayer( const std::string &element_name, int exclusive_layer_index, int layer_number )
{
    LayerInfo* li = new LayerInfo(element_name, exclusive_layer_index, layer_number);
    UndoStep* action = new UndoStep(UNDO_LAYER_ADDED, li);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureRemovedLayer( const std::string &element_name, int layer_number )
{
    LayerInfo* li = new LayerInfo(element_name, -1, layer_number);
    UndoStep* action = new UndoStep(UNDO_LAYER_REMOVED, li);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CancelLastStep() {
    if (!mUndoSteps.empty()) {
        delete mUndoSteps.back();
        mUndoSteps.pop_back();
    }
}

void UndoManager::UndoLastStep()
{
    UndoStep* action = new UndoStep(UNDO_MARKER);
    mRedoSteps.push_back(action);
    ProcessUndoStep(mUndoSteps, mRedoSteps);
}

void UndoManager::RedoLastStep()
{
    UndoStep* action = new UndoStep(UNDO_MARKER);
    mUndoSteps.push_back(action);
    ProcessUndoStep(mRedoSteps, mUndoSteps);
}

void UndoManager::ProcessUndoStep(std::vector<UndoStep*> &fromList, std::vector<UndoStep*> &toList)
{
    
    bool done = false;
    while (fromList.size() > 0 && !done)
    {
        UndoStep* next_action = fromList.back();
        switch (next_action->undo_action)
        {
        case UNDO_MARKER:
            done = true;
            break;
        case UNDO_EFFECT_DELETED:
        {
            if (next_action->deleted_effect_info.size() == 0) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. XXX");
            }
            Element* element = mParentSequence->GetElement(next_action->deleted_effect_info[0]->element_name);
            if (element != nullptr)
            {
                EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(next_action->deleted_effect_info[0]->layer_index);
                if (el != nullptr)
                {
                    Effect* eff = el->AddEffect(0,
                        next_action->deleted_effect_info[0]->name,
                        next_action->deleted_effect_info[0]->settings,
                        next_action->deleted_effect_info[0]->palette,
                        next_action->deleted_effect_info[0]->startTimeMS,
                        next_action->deleted_effect_info[0]->endTimeMS,
                        next_action->deleted_effect_info[0]->Selected,
                        next_action->deleted_effect_info[0]->Protected);

                    // Move effect to other list
                    AddedEffectInfo* effect_undo_action = new AddedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID());
                    UndoStep* action = new UndoStep(UNDO_EFFECT_ADDED, effect_undo_action);
                    toList.push_back(action);
                }
            }
        }
        break;
        case UNDO_EFFECT_ADDED:
        {
            if (next_action->added_effect_info.size() == 0) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. AAA");
            }
            Element* element = mParentSequence->GetElement(next_action->added_effect_info[0]->element_name);
            if (element != nullptr)
            {
                EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(next_action->added_effect_info[0]->layer_index);
                if (el != nullptr)
                {
                    // Move effect to other list
                    Effect* eff = el->GetEffectFromID(next_action->added_effect_info[0]->id);
                    DeletedEffectInfo* effect_undo_action = new DeletedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetEffectName(), eff->GetSettingsAsString(), eff->GetPaletteAsString(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                    UndoStep* action = new UndoStep(UNDO_EFFECT_DELETED, effect_undo_action);
                    toList.push_back(action);

                    // Delete the effect
                    el->DeleteEffect(next_action->added_effect_info[0]->id);
                }
            }
        }
        break;
        case UNDO_EFFECT_MOVED:
        {
            if (next_action->moved_effect_info.size() == 0) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. BBB");
            }
            Element* element = mParentSequence->GetElement(next_action->moved_effect_info[0]->element_name);
            if (element != nullptr)
            {
                EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(next_action->moved_effect_info[0]->layer_index);
                if (el == nullptr)
                {
                    spdlog::warn("UndoLastStep:UNDO_EFFECT_MOVED Element not found {}.", next_action->moved_effect_info[0]->layer_index);
                }
                else
                {
                    Effect* eff = el->GetEffectFromID(next_action->moved_effect_info[0]->id);
                    if (eff != nullptr)
                    {
                        // Capture for other list
                        MovedEffectInfo* effect_undo_action = new MovedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID(), eff->GetStartTimeMS(), eff->GetEndTimeMS());
                        UndoStep* action = new UndoStep(UNDO_EFFECT_MOVED, effect_undo_action);
                        toList.push_back(action);

                        // Move the effect
                        eff->SetStartTimeMS(next_action->moved_effect_info[0]->startTimeMS);
                        eff->SetEndTimeMS(next_action->moved_effect_info[0]->endTimeMS);
                    }
                }
            }
        }
        break;
        case UNDO_EFFECT_MODIFIED:
        {
            if (next_action->modified_effect_info.size() == 0) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. CCC");
            }
            Element* element = mParentSequence->GetElement(next_action->modified_effect_info[0]->element_name);
            if (element != nullptr)
            {
                EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(next_action->modified_effect_info[0]->layer_index);
                if (el == nullptr)
                {
                    spdlog::warn("UndoLastStep:UNDO_EFFECT_MODIFIED Element not found {}.", next_action->modified_effect_info[0]->layer_index);
                }
                else
                {
                    Effect* eff = el->GetEffectFromID(next_action->modified_effect_info[0]->id);
                    if (eff != nullptr)
                    {
                        // Capture for other list
                        ModifiedEffectInfo* effect_undo_action = new ModifiedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff);
                        UndoStep* action = new UndoStep(UNDO_EFFECT_MODIFIED, effect_undo_action);
                        toList.push_back(action);

                        // Modify the effect
                        if (next_action->modified_effect_info[0]->effectType >= 0) {
                            eff->SetEffectName(next_action->modified_effect_info[0]->effectName);
                            eff->SetEffectIndex(next_action->modified_effect_info[0]->effectType);
                        }
                        eff->SetSettings(next_action->modified_effect_info[0]->settings, false);
                        eff->SetPalette(next_action->modified_effect_info[0]->palette);
                    }
                }
            }
        }
        break;
        case UNDO_LAYER_ADDED:
        {
            if (next_action->layer_info.empty()) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. DDD");
                break;
            }
            Element* element = mParentSequence->GetElement(next_action->layer_info[0]->element_name);
            if (element != nullptr) {
                EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(next_action->layer_info[0]->exclusive_layer_index);
                if (el != nullptr && element->GetEffectLayerCount() > 1) {
                    int pos = el->GetLayerNumber() - 1;
                    LayerInfo* li = new LayerInfo(next_action->layer_info[0]->element_name, -1, pos);
                    UndoStep* action = new UndoStep(UNDO_LAYER_REMOVED, li);
                    toList.push_back(action);
                    element->RemoveEffectLayer(pos);
                }
            }
            mParentSequence->PopulateRowInformation();
        }
        break;
        case UNDO_LAYER_REMOVED:
        {
            if (next_action->layer_info.empty()) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. EEE");
                break;
            }
            Element* element = mParentSequence->GetElement(next_action->layer_info[0]->element_name);
            if (element != nullptr) {
                int pos = next_action->layer_info[0]->layer_number;
                EffectLayer* el;
                if (pos >= (int)element->GetEffectLayerCount()) {
                    el = element->AddEffectLayer();
                } else {
                    el = element->InsertEffectLayer(pos);
                }
                LayerInfo* li = new LayerInfo(next_action->layer_info[0]->element_name, el->GetIndex(), pos);
                UndoStep* action = new UndoStep(UNDO_LAYER_ADDED, li);
                toList.push_back(action);
                mParentSequence->PopulateRowInformation();
            }
        }
        break;
        }
        fromList.pop_back();
    }
}

std::string UndoManager::GetUndoString()
{
    std::string undo_string = "Undo";

    if (mUndoSteps.size() > 0)
    {
        UndoStep* next_action = mUndoSteps.back();
        switch (next_action->undo_action)
        {
        case UNDO_EFFECT_DELETED:
            undo_string = "Undo: Effect(s) Deleted";
            break;
        case UNDO_EFFECT_ADDED:
            undo_string = "Undo: Effect(s) Added";
            break;
        case UNDO_EFFECT_MOVED:
            undo_string = "Undo: Effect(s) Moved";
            break;
        case UNDO_EFFECT_MODIFIED:
            undo_string = "Undo: Effect(s) Modified";
            break;
        case UNDO_LAYER_ADDED:
            undo_string = "Undo: Layer(s) Added";
            break;
        case UNDO_LAYER_REMOVED:
            undo_string = "Undo: Layer(s) Removed";
            break;
        case UNDO_MARKER:
            break;
        }
    }
    return undo_string;
}

std::string UndoManager::GetRedoString()
{
    std::string redo_string = "Redo";

    if (mRedoSteps.size() > 0)
    {
        UndoStep* next_action = mRedoSteps.back();
        switch (next_action->undo_action)
        {
        case UNDO_EFFECT_DELETED:
            redo_string = "Redo: Effect(s) Deleted";
            break;
        case UNDO_EFFECT_ADDED:
            redo_string = "Redo: Effect(s) Added";
            break;
        case UNDO_EFFECT_MOVED:
            redo_string = "Redo: Effect(s) Moved";
            break;
        case UNDO_EFFECT_MODIFIED:
            redo_string = "Redo: Effect(s) Modified";
            break;
        case UNDO_LAYER_ADDED:
            redo_string = "Redo: Layer(s) Added";
            break;
        case UNDO_LAYER_REMOVED:
            redo_string = "Redo: Layer(s) Removed";
            break;
        case UNDO_MARKER:
            break;
        }
    }
    return redo_string;
}

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

UndoTarget UndoManager::ResolveTarget(const std::string& element_name, int exclusive_layer_index) const
{
    UndoTarget t;
    Element* element = mParentSequence->GetElement(element_name);
    if (element != nullptr) {
        EffectLayer* el = element->GetEffectLayerFromExclusiveIndex(exclusive_layer_index);
        if (el != nullptr) {
            t.element = element->GetUid();
            t.layer = el->GetUid();
        }
    }
    return t;
}

Element* UndoManager::FindElement(const UndoTarget& t, const std::string& element_name) const
{
    if (t.element != kNoSeqObjectId) {
        return mParentSequence->GetElementFromUid(t.element);
    }
    return mParentSequence->GetElement(element_name);
}

EffectLayer* UndoManager::FindLayer(const UndoTarget& t, const std::string& element_name, int exclusive_layer_index) const
{
    if (t.layer != kNoSeqObjectId) {
        Element* element = mParentSequence->GetElementFromUid(t.element);
        return element == nullptr ? nullptr : element->GetEffectLayerFromUid(t.layer);
    }
    Element* element = mParentSequence->GetElement(element_name);
    return element == nullptr ? nullptr : element->GetEffectLayerFromExclusiveIndex(exclusive_layer_index);
}

Effect* UndoManager::FindEffect(EffectLayer* el, const UndoTarget& t, int legacy_id)
{
    if (t.effect != kNoSeqObjectId) {
        return el->GetEffectFromUid(t.effect);
    }
    return el->GetEffectFromID(legacy_id);
}

UndoTarget UndoManager::TargetOf(EffectLayer* el, Effect* eff)
{
    UndoTarget t;
    if (el != nullptr && el->GetParentElement() != nullptr) {
        t.element = el->GetParentElement()->GetUid();
        t.layer = el->GetUid();
        if (eff != nullptr) {
            t.effect = eff->GetUid();
        }
    }
    return t;
}

void UndoManager::CaptureEffectToBeDeleted( const std::string &element_name, int layer_index, const std::string &name, const std::string &settings,
                                            const std::string &palette, int startTimeMS, int endTimeMS, int Selected, bool Protected )
{
    DeletedEffectInfo* effect_undo_action = new DeletedEffectInfo( element_name, layer_index, name, settings, palette, startTimeMS, endTimeMS, Selected, Protected );
    effect_undo_action->target = ResolveTarget(element_name, layer_index);
    if (effect_undo_action->target.layer != kNoSeqObjectId) {
        // Callers capture before removing the effect, so it is still in the
        // layer; remember its identity so undo can restore it onto the
        // re-created effect and earlier steps that name it still resolve.
        EffectLayer* el = FindLayer(effect_undo_action->target, element_name, layer_index);
        if (el != nullptr) {
            for (Effect* e : el->GetEffects()) {
                if (e->GetStartTimeMS() == startTimeMS && e->GetEndTimeMS() == endTimeMS && e->GetEffectName() == name) {
                    effect_undo_action->target.effect = e->GetUid();
                    break;
                }
            }
        }
    }
    UndoStep* action = new UndoStep(UNDO_EFFECT_DELETED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureAddedEffect( const std::string &element_name, int layer_index, int id )
{
    AddedEffectInfo* effect_undo_action = new AddedEffectInfo( element_name, layer_index, id );
    effect_undo_action->target = ResolveTarget(element_name, layer_index);
    if (EffectLayer* el = FindLayer(effect_undo_action->target, element_name, layer_index);
        el != nullptr && effect_undo_action->target.layer != kNoSeqObjectId) {
        if (Effect* e = el->GetEffectFromID(id); e != nullptr) {
            effect_undo_action->target.effect = e->GetUid();
        }
    }
    UndoStep* action = new UndoStep(UNDO_EFFECT_ADDED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureEffectToBeMoved( const std::string &element_name, int layer_index, int id, int startTimeMS, int endTimeMS )
{
    MovedEffectInfo* effect_undo_action = new MovedEffectInfo( element_name, layer_index, id, startTimeMS, endTimeMS );
    effect_undo_action->target = ResolveTarget(element_name, layer_index);
    if (EffectLayer* el = FindLayer(effect_undo_action->target, element_name, layer_index);
        el != nullptr && effect_undo_action->target.layer != kNoSeqObjectId) {
        if (Effect* e = el->GetEffectFromID(id); e != nullptr) {
            effect_undo_action->target.effect = e->GetUid();
        }
    }
    UndoStep* action = new UndoStep(UNDO_EFFECT_MOVED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureModifiedEffect( const std::string &element_name, int layer_index, int id, const std::string &settings, const std::string &palette )
{
    ModifiedEffectInfo* effect_undo_action = new ModifiedEffectInfo( element_name, layer_index, id, settings, palette );
    effect_undo_action->target = ResolveTarget(element_name, layer_index);
    if (EffectLayer* el = FindLayer(effect_undo_action->target, element_name, layer_index);
        el != nullptr && effect_undo_action->target.layer != kNoSeqObjectId) {
        if (Effect* e = el->GetEffectFromID(id); e != nullptr) {
            effect_undo_action->target.effect = e->GetUid();
        }
    }
    UndoStep* action = new UndoStep(UNDO_EFFECT_MODIFIED, effect_undo_action);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureModifiedEffect( const std::string &element_name, int layer_index, Effect *ef )
{
    ModifiedEffectInfo* effect_undo_action = new ModifiedEffectInfo( element_name, layer_index, ef );
    effect_undo_action->target = ResolveTarget(element_name, layer_index);
    if (effect_undo_action->target.layer != kNoSeqObjectId) {
        effect_undo_action->target.effect = ef->GetUid();
    }
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
    li->target = ResolveTarget(element_name, exclusive_layer_index);
    UndoStep* action = new UndoStep(UNDO_LAYER_ADDED, li);
    mUndoSteps.push_back(action);
    EnforceMaxSteps();
}

void UndoManager::CaptureRemovedLayer( const std::string &element_name, int layer_number )
{
    LayerInfo* li = new LayerInfo(element_name, -1, layer_number);
    // Captured before the layer is removed: remember its identity so undo
    // re-creates the layer under the same id.
    if (Element* element = mParentSequence->GetElement(element_name); element != nullptr) {
        if (EffectLayer* el = element->GetEffectLayer(layer_number); el != nullptr) {
            li->target.element = element->GetUid();
            li->target.layer = el->GetUid();
        }
    }
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
            DeletedEffectInfo* info = next_action->deleted_effect_info[0];
            EffectLayer* el = FindLayer(info->target, info->element_name, info->layer_index);
            if (el != nullptr)
            {
                Effect* eff = el->AddEffect(0,
                    info->name,
                    info->settings,
                    info->palette,
                    info->startTimeMS,
                    info->endTimeMS,
                    info->Selected,
                    info->Protected);

                if (eff == nullptr)
                {
                    spdlog::warn("UndoLastStep:UNDO_EFFECT_DELETED Effect could not be restored on layer {}.", info->layer_index);
                }
                else
                {
                    // Bring the effect back under its old identity so earlier
                    // steps that name it still find it - unless something live
                    // already holds that id.
                    if (info->target.effect != kNoSeqObjectId && el->GetEffectFromUid(info->target.effect) == nullptr) {
                        eff->SetUid(info->target.effect);
                    }
                    // Move effect to other list
                    AddedEffectInfo* effect_undo_action = new AddedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID());
                    effect_undo_action->target = TargetOf(el, eff);
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
            AddedEffectInfo* info = next_action->added_effect_info[0];
            EffectLayer* el = FindLayer(info->target, info->element_name, info->layer_index);
            if (el != nullptr)
            {
                // Move effect to other list
                Effect* eff = FindEffect(el, info->target, info->id);
                if (eff == nullptr)
                {
                    // The effect was already removed by something that did not
                    // capture the deletion, so there is nothing left to undo.
                    spdlog::warn("UndoLastStep:UNDO_EFFECT_ADDED Effect not found {}.", info->id);
                }
                else
                {
                    DeletedEffectInfo* effect_undo_action = new DeletedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetEffectName(), eff->GetSettingsAsString(), eff->GetPaletteAsString(), eff->GetStartTimeMS(), eff->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                    effect_undo_action->target = TargetOf(el, eff);
                    UndoStep* action = new UndoStep(UNDO_EFFECT_DELETED, effect_undo_action);
                    toList.push_back(action);

                    // Delete the effect
                    el->DeleteEffectByUid(eff->GetUid());
                }
            }
        }
        break;
        case UNDO_EFFECT_MOVED:
        {
            if (next_action->moved_effect_info.size() == 0) {
                spdlog::critical("UndoManager::ProcessUndoStep about to access past end of array. This wont end well. BBB");
            }
            MovedEffectInfo* info = next_action->moved_effect_info[0];
            if (FindElement(info->target, info->element_name) != nullptr)
            {
                EffectLayer* el = FindLayer(info->target, info->element_name, info->layer_index);
                if (el == nullptr)
                {
                    spdlog::warn("UndoLastStep:UNDO_EFFECT_MOVED Element not found {}.", info->layer_index);
                }
                else
                {
                    Effect* eff = FindEffect(el, info->target, info->id);
                    if (eff != nullptr)
                    {
                        // Capture for other list
                        MovedEffectInfo* effect_undo_action = new MovedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff->GetID(), eff->GetStartTimeMS(), eff->GetEndTimeMS());
                        effect_undo_action->target = TargetOf(el, eff);
                        UndoStep* action = new UndoStep(UNDO_EFFECT_MOVED, effect_undo_action);
                        toList.push_back(action);

                        // Move the effect
                        eff->SetStartTimeMS(info->startTimeMS);
                        eff->SetEndTimeMS(info->endTimeMS);
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
            ModifiedEffectInfo* info = next_action->modified_effect_info[0];
            if (FindElement(info->target, info->element_name) != nullptr)
            {
                EffectLayer* el = FindLayer(info->target, info->element_name, info->layer_index);
                if (el == nullptr)
                {
                    spdlog::warn("UndoLastStep:UNDO_EFFECT_MODIFIED Element not found {}.", info->layer_index);
                }
                else
                {
                    Effect* eff = FindEffect(el, info->target, info->id);
                    if (eff != nullptr)
                    {
                        // Capture for other list
                        ModifiedEffectInfo* effect_undo_action = new ModifiedEffectInfo(el->GetParentElement()->GetModelName(), el->GetIndex(), eff);
                        effect_undo_action->target = TargetOf(el, eff);
                        UndoStep* action = new UndoStep(UNDO_EFFECT_MODIFIED, effect_undo_action);
                        toList.push_back(action);

                        // Modify the effect
                        if (info->effectType >= 0) {
                            eff->SetEffectName(info->effectName);
                            eff->SetEffectIndex(info->effectType);
                        }
                        eff->SetSettings(info->settings, false);
                        eff->SetPalette(info->palette);
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
            LayerInfo* info = next_action->layer_info[0];
            EffectLayer* el = FindLayer(info->target, info->element_name, info->exclusive_layer_index);
            // Remove from the element that owns the layer: a model-level lookup
            // also finds submodel/strand layers.
            Element* owner = (el != nullptr) ? el->GetParentElement() : nullptr;
            if (owner != nullptr && owner->GetEffectLayerCount() > 1) {
                int pos = el->GetLayerNumber() - 1;
                if (pos >= 0 && pos < (int)owner->GetEffectLayerCount() && owner->GetEffectLayer(pos) == el) {
                    LayerInfo* li = new LayerInfo(info->element_name, info->exclusive_layer_index, pos);
                    li->target = TargetOf(el, nullptr);
                    UndoStep* action = new UndoStep(UNDO_LAYER_REMOVED, li);
                    toList.push_back(action);
                    owner->RemoveEffectLayer(pos);
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
            LayerInfo* info = next_action->layer_info[0];
            Element* element = FindElement(info->target, info->element_name);
            if (element != nullptr) {
                int pos = info->layer_number;
                EffectLayer* el;
                if (pos >= (int)element->GetEffectLayerCount()) {
                    el = element->AddEffectLayer();
                } else {
                    el = element->InsertEffectLayer(pos);
                }
                // Re-create the layer under its old identity so steps that
                // name it (effects deleted from it, etc.) still resolve.
                if (info->target.layer != kNoSeqObjectId && element->GetEffectLayerFromUid(info->target.layer) == nullptr) {
                    el->SetUid(info->target.layer);
                }
                LayerInfo* li = new LayerInfo(info->element_name, el->GetIndex(), pos);
                li->target = TargetOf(el, nullptr);
                UndoStep* action = new UndoStep(UNDO_LAYER_ADDED, li);
                toList.push_back(action);
                mParentSequence->PopulateRowInformation();
                // Make sure redo recreates the layers needed to support the smart copy/paste
                int oldIdx = info->exclusive_layer_index;
                if (oldIdx >= 0) {
                    int newIdx = el->GetIndex();
                    for (auto* step : fromList) {
                        if (step->undo_action == UNDO_EFFECT_DELETED) {
                            for (auto* dei : step->deleted_effect_info) {
                                if (dei->layer_index == oldIdx)
                                    dei->layer_index = newIdx;
                            }
                        }
                    }
                }
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

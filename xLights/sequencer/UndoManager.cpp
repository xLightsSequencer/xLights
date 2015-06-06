#include "UndoManager.h"
#include "Element.h"
#include "SequenceElements.h"

DeletedEffectInfo::DeletedEffectInfo( const wxString &element_name_, int layer_index_, const wxString &name_, const wxString &settings_,
                                      const wxString &palette_, double &startTime_, double &endTime_, int Selected_, bool Protected_ )
: element_name(element_name_), layer_index(layer_index_), name(name_), settings(settings_),
  palette(palette_), startTime(startTime_), endTime(endTime_), Selected(Selected_), Protected(Protected_)
{
}

AddedEffectInfo::AddedEffectInfo( const wxString &element_name_, int layer_index_, double startTime_ )
: element_name(element_name_), layer_index(layer_index_), startTime(startTime_)
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

UndoManager::UndoManager(SequenceElements* parent)
: mParentSequence(parent)
{
}

UndoManager::~UndoManager()
{
    for( int i = 0; i < mUndoSteps.size(); i++ )
    {
        delete mUndoSteps[i];
    }
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

bool UndoManager::CanUndo()
{
    RemoveUnusedMarkers();
    return mUndoSteps.size() > 0;
}

void UndoManager::CreateUndoStep()
{
    RemoveUnusedMarkers();
    UndoStep* action = new UndoStep(UNDO_MARKER);
    mUndoSteps.push_back(action);
}

void UndoManager::CaptureEffectToBeDeleted( const wxString &element_name, int layer_index, const wxString &name, const wxString &settings,
                                            const wxString &palette, double startTime, double endTime, int Selected, bool Protected )
{
    DeletedEffectInfo* effect_undo_action = new DeletedEffectInfo( element_name, layer_index, name, settings, palette, startTime, endTime, Selected, Protected );
    UndoStep* action = new UndoStep(UNDO_EFFECT_DELETED, effect_undo_action);
    mUndoSteps.push_back(action);
}

void UndoManager::CaptureAddedEffect( const wxString &element_name, int layer_index, double startTime )
{
    AddedEffectInfo* effect_undo_action = new AddedEffectInfo( element_name, layer_index, startTime + 0.005 );
    UndoStep* action = new UndoStep(UNDO_EFFECT_ADDED, effect_undo_action);
    mUndoSteps.push_back(action);
}

void UndoManager::UndoLastStep()
{
    bool done = false;
    while( mUndoSteps.size() > 0 && !done )
    {
        UndoStep* next_action = mUndoSteps.back();
        switch( next_action->undo_action )
        {
        case UNDO_MARKER:
            done = true;
            break;
        case UNDO_EFFECT_DELETED:
            {
            Element* element = mParentSequence->GetElement(next_action->deleted_effect_info[0]->element_name);
            EffectLayer* el = element->GetEffectLayer(next_action->deleted_effect_info[0]->layer_index);
            Effect* ef = el->AddEffect(0,
                                       next_action->deleted_effect_info[0]->name,
                                       next_action->deleted_effect_info[0]->settings,
                                       next_action->deleted_effect_info[0]->palette,
                                       next_action->deleted_effect_info[0]->startTime,
                                       next_action->deleted_effect_info[0]->endTime,
                                       next_action->deleted_effect_info[0]->Selected,
                                       next_action->deleted_effect_info[0]->Protected);
            }
            break;
        case UNDO_EFFECT_ADDED:
            {
            Element* element = mParentSequence->GetElement(next_action->added_effect_info[0]->element_name);
            EffectLayer* el = element->GetEffectLayer(next_action->added_effect_info[0]->layer_index);
            el->DeleteEffect(next_action->added_effect_info[0]->startTime);
            }
            break;
        }
        mUndoSteps.pop_back();
    }
}

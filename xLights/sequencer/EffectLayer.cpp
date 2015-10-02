#include "EffectLayer.h"
#include <algorithm>
#include <vector>
#include "EffectsGrid.h"
#include "Effect.h"
#include "RowHeading.h"
#include "ModelClass.h"


std::atomic_int EffectLayer::exclusive_index(0);
const wxString NamedLayer::NO_NAME("");

EffectLayer::EffectLayer(Element* parent)
{
    mParentElement = parent;
    mIndex = exclusive_index++;
}

EffectLayer::~EffectLayer()
{
    wxMutexLocker locker(lock);
    for (int x = 0; x < mEffects.size(); x++) {
        delete mEffects[x];
    }
}

int EffectLayer::GetIndex()
{
    return mIndex;
}

Effect* EffectLayer::GetEffect(int index)
{
    if(index < mEffects.size())
    {
        return mEffects[index];
    }
    else
    {
        return nullptr;
    }
}
Effect* EffectLayer::GetEffectByTime(int timeMS) {
    wxMutexLocker locker(lock);
    for(std::vector<Effect*>::iterator i = mEffects.begin(); i != mEffects.end(); i++) {
        if (timeMS >= (*i)->GetStartTimeMS() &&
            timeMS <= (*i)->GetEndTimeMS()) {
            return (*i);
        }
    }
    return nullptr;
}


Effect* EffectLayer::GetEffectFromID(int id)
{
    Effect* eff = nullptr;
    for (int x = 0; x < mEffects.size(); x++) {
        if( mEffects[x]->GetID() == id )
        {
            eff = mEffects[x];
            break;
        }
    }
    return eff;
}

void EffectLayer::RemoveEffect(int index)
{
    wxMutexLocker locker(lock);
    if(index<mEffects.size())
    {
        Effect *e = mEffects[index];
        mEffects.erase(mEffects.begin()+index);
        IncrementChangeCount(e->GetStartTimeMS(), e->GetEndTimeMS());
        delete e;
    }
}


Effect* EffectLayer::AddEffect(int id, const wxString &name, const wxString &settings, const wxString &palette,
                               int startTimeMS, int endTimeMS, int Selected, bool Protected)
{
    return AddEffect(id, Effect::GetEffectIndex(name), name, settings, palette, startTimeMS, endTimeMS, Selected, Protected);
}
Effect* EffectLayer::AddEffect(int id, int effectIndex, const wxString &name, const wxString &settings, const wxString &palette,
                               int startTimeMS, int endTimeMS, int Selected, bool Protected)
{
    wxMutexLocker locker(lock);
    Effect *e = new Effect(this, id, effectIndex, name, settings, palette, startTimeMS, endTimeMS, Selected, Protected);
    mEffects.push_back(e);
    SortEffects();
    IncrementChangeCount(startTimeMS, endTimeMS);
    return e;
}


void EffectLayer::SortEffects()
{
    std::sort(mEffects.begin(),mEffects.end(),SortEffectByStartTime);
    for (int x = 0; x < mEffects.size(); x++) {
        mEffects[x]->SetID(x);
    }
}

bool EffectLayer::IsStartTimeLinked(int index)
{
    if(index < mEffects.size() && index > 0)
    {
        return mEffects[index-1]->GetEndTimeMS() == mEffects[index]->GetStartTimeMS();
    }
    else
    {
        return false;
    }
}

bool EffectLayer::IsEndTimeLinked(int index)
{
    if(index < mEffects.size()-1)
    {
        return mEffects[index]->GetEndTimeMS() == mEffects[index+1]->GetStartTimeMS();
    }
    else
    {
        return false;
    }
}

int EffectLayer::GetMaximumEndTimeMS(int index, bool allow_collapse)
{
    if(index+1 >= mEffects.size())
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        if(mEffects[index]->GetEndTimeMS() == mEffects[index+1]->GetStartTimeMS() && allow_collapse)
        {
            return mEffects[index+1]->GetEndTimeMS();
        }
        else
        {
            return mEffects[index+1]->GetStartTimeMS();
        }
    }
}

int EffectLayer::GetMinimumStartTimeMS(int index, bool allow_collapse)
{
    if(index == 0)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        if(mEffects[index-1]->GetEndTimeMS() == mEffects[index]->GetStartTimeMS() && allow_collapse)
        {
            return mEffects[index-1]->GetStartTimeMS();
        }
        else
        {
            return mEffects[index-1]->GetEndTimeMS();
        }
    }
}

int EffectLayer::GetEffectCount()
{
    if(mEffects.empty())
        return 0;
    else
        return mEffects.size();
}

bool EffectLayer::HitTestEffectByTime(int timeMS,int &index)
{
    for(int i=0;i<mEffects.size();i++)
    {
        if (timeMS >= mEffects[i]->GetStartTimeMS() &&
            timeMS <= mEffects[i]->GetEndTimeMS())
        {
            index = i;
            return true;
        }
    }
    return false;
}

Effect* EffectLayer::GetEffectBeforeTime(int ms)
{
    int i;
    for(i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetStartTimeMS() >= ms)
        {
            break;
        }
    }
    if(i==0)
    {
        return nullptr;
    }
    else
    {
        return mEffects[i-1];
    }
}

Effect* EffectLayer::GetEffectAtTime(int timeMS)
{
    for(int i=0;i<mEffects.size();i++) {
        if (timeMS >= mEffects[i]->GetStartTimeMS() &&
            timeMS <= mEffects[i]->GetEndTimeMS()) {
            return mEffects[i];
        }
    }
    return nullptr;
}

Effect*  EffectLayer::GetEffectBeforeEmptyTime(int ms)
{
    int i;
    for(i=mEffects.size()-1; i >= 0; i--)
    {
        if( mEffects[i]->GetEndTimeMS() < ms )
        {
            break;
        }
    }
    if(i<0)
    {
        return nullptr;
    }
    else
    {
        return mEffects[i];
    }
}

Effect*  EffectLayer::GetEffectAfterEmptyTime(int ms)
{
    int i;
    for(i=0; i < mEffects.size(); i++)
    {
        if( mEffects[i]->GetStartTimeMS() > ms )
        {
            break;
        }
    }
    if(i==mEffects.size())
    {
        return nullptr;
    }
    else
    {
        return mEffects[i];
    }
}
 
bool EffectLayer::GetRangeIsClearMS(int startTimeMS, int endTimeMS)
{
    int i;
    for(i=0; i<mEffects.size();i++)
    {
        // check if start is between effect range
        if( (startTimeMS > mEffects[i]->GetStartTimeMS()) && (startTimeMS < mEffects[i]->GetEndTimeMS()) )
        {
            return false;
        }
        // check if end is between effect range
        if( (endTimeMS > mEffects[i]->GetStartTimeMS()) && (endTimeMS < mEffects[i]->GetEndTimeMS()) )
        {
            return false;
        }
        // check effect is between start and end
        if( (mEffects[i]->GetStartTimeMS() >= startTimeMS) && (mEffects[i]->GetEndTimeMS() <= endTimeMS) )
        {
            return false;
        }
    }
    return true;
}

bool EffectLayer::HasEffectsInTimeRange(int startTimeMS, int endTimeMS) {
    for(int i=0;i<mEffects.size();i++)
    {
        if(mEffects[i]->GetStartTimeMS() >= startTimeMS &&  mEffects[i]->GetStartTimeMS() < endTimeMS)
        {
            return true;
        }
        else if(mEffects[i]->GetEndTimeMS() <= endTimeMS &&  mEffects[i]->GetEndTimeMS() > startTimeMS)
        {
            return true;
        }
    }
    return false;
}

int EffectLayer::SelectEffectsInTimeRange(int startTimeMS, int endTimeMS)
{
    int num_selected = 0;
    for(int i=0;i<mEffects.size();i++)
    {
        if(mEffects[i]->GetStartTimeMS() >= startTimeMS &&  mEffects[i]->GetStartTimeMS() < endTimeMS)
        {
            mEffects[i]->SetSelected(EFFECT_SELECTED);
            num_selected++;
        }
        else if(mEffects[i]->GetEndTimeMS() <= endTimeMS &&  mEffects[i]->GetEndTimeMS() > startTimeMS)
        {
            mEffects[i]->SetSelected(EFFECT_SELECTED);
            num_selected++;
        }
    }
    return num_selected;
}


void EffectLayer::UnSelectAllEffects()
{
    for(int i=0;i<mEffects.size();i++)
    {
        mEffects[i]->SetSelected(EFFECT_NOT_SELECTED);
    }
}

Element* EffectLayer::GetParentElement()
{
    return mParentElement;

}

void EffectLayer::SetParentElement(Element* parent)
{
    mParentElement = parent;
}

void EffectLayer::GetMaximumRangeOfMovementForSelectedEffects(int &toLeft, int &toRight)
{
    toLeft = NO_MAX;
    toRight = NO_MAX;
    for(int i=0;i<mEffects.size();i++)
    {
        if(mEffects[i]->GetSelected() != EFFECT_NOT_SELECTED)
        {
            int l,r;
            GetMaximumRangeOfMovementForEffect(i,l,r);
            toLeft = toLeft<l?toLeft:l;
            toRight = toRight<r?toRight:r;
        }
    }
}

void EffectLayer::GetMaximumRangeOfMovementForEffect(int index, int &toLeft, int &toRight)
{
    switch(mEffects[index]->GetSelected())
    {
        case EFFECT_LT_SELECTED:
            GetMaximumRangeWithLeftMovement(index,toLeft,toRight);
            break;
        case EFFECT_RT_SELECTED:
            GetMaximumRangeWithRightMovement(index,toLeft,toRight);
            break;
        case EFFECT_SELECTED:
            int l1,l2,r1,r2;
            GetMaximumRangeWithLeftMovement(index,l1,r1);
            GetMaximumRangeWithRightMovement(index,l2,r2);
            toLeft = l1<l2?l1:l2;
            toRight = r1<r2?r1:r2;
            break;
        default:
            break;
    }
}

void EffectLayer::GetMaximumRangeWithLeftMovement(int index, int &toLeft, int &toRight)
{
    toRight = mEffects[index]->GetEndTimeMS() - mEffects[index]->GetStartTimeMS();
    if(index == 0)
    {
       toLeft = mEffects[index]->GetStartTimeMS();
    }
    else
    {
       if(mEffects[index-1]->GetSelected() == EFFECT_NOT_SELECTED)
       {
          toLeft = mEffects[index]->GetStartTimeMS() - mEffects[index-1]->GetEndTimeMS();
       }
       else if(mEffects[index-1]->GetSelected() == EFFECT_RT_SELECTED)
       {
          toLeft = mEffects[index]->GetStartTimeMS() - mEffects[index-1]->GetStartTimeMS();
       }
       else if(mEffects[index-1]->GetSelected() == EFFECT_SELECTED)
       {
           // Do not know so set to maximum. Let the effects to left decide
          toLeft = mEffects[index]->GetStartTimeMS();
       }
    }
}

void EffectLayer::GetMaximumRangeWithRightMovement(int index, int &toLeft, int &toRight)
{
    toLeft = mEffects[index]->GetEndTimeMS() - mEffects[index]->GetStartTimeMS();
    // Last effect, nothing to right to stop movement other then edge of screen.
    // Let grid take care of screen boundary so set to huge number
    if(index == mEffects.size()-1)
    {
       toRight = NO_MAX;
    }
    else
    {
       if(mEffects[index+1]->GetSelected() == EFFECT_NOT_SELECTED)
       {
          toRight = mEffects[index+1]->GetStartTimeMS() - mEffects[index]->GetEndTimeMS();
       }
       else if(mEffects[index+1]->GetSelected() == EFFECT_LT_SELECTED)
       {
          toRight = mEffects[index+1]->GetEndTimeMS() - mEffects[index]->GetEndTimeMS();
       }
       else if(mEffects[index+1]->GetSelected() == EFFECT_SELECTED)
       {
           // Do not know so set to maximum. Let the effects to right decide
          toRight = NO_MAX;
       }
    }
}

int EffectLayer::GetSelectedEffectCount()
{
    wxString s;
    int count=0;
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetSelected() != EFFECT_NOT_SELECTED)
        {
           count++;
        }
    }
    return count;
}

void EffectLayer::UpdateAllSelectedEffects(const wxString& palette)
{
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetSelected() != EFFECT_NOT_SELECTED )
        {
            mEffects[i]->SetPalette(palette);
        }
    }
}

void EffectLayer::MoveAllSelectedEffects(int deltaMS, UndoManager& undo_mgr)
{
    wxMutexLocker locker(lock);
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetSelected() == EFFECT_LT_SELECTED)
        {
            if( undo_mgr.GetCaptureUndo() ) {
                undo_mgr.CaptureEffectToBeMoved( mParentElement->GetName(), mIndex, mEffects[i]->GetID(),
                                                 mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS() );
            }
            mEffects[i]->SetStartTimeMS( mEffects[i]->GetStartTimeMS() + deltaMS);
        }
        else if(mEffects[i]->GetSelected() == EFFECT_RT_SELECTED)
        {
            if( undo_mgr.GetCaptureUndo() ) {
                undo_mgr.CaptureEffectToBeMoved( mParentElement->GetName(), mIndex, mEffects[i]->GetID(),
                                                 mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS() );
            }
            mEffects[i]->SetEndTimeMS( mEffects[i]->GetEndTimeMS() + deltaMS);
        }
        else if(mEffects[i]->GetSelected() == EFFECT_SELECTED)
        {
            if( undo_mgr.GetCaptureUndo() ) {
                undo_mgr.CaptureEffectToBeMoved( mParentElement->GetName(), mIndex, mEffects[i]->GetID(),
                                                 mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS() );
            }
            mEffects[i]->SetStartTimeMS( mEffects[i]->GetStartTimeMS() + deltaMS);
            mEffects[i]->SetEndTimeMS( mEffects[i]->GetEndTimeMS() + deltaMS);
        }
    }
}

void EffectLayer::DeleteSelectedEffects(UndoManager& undo_mgr)
{
    wxMutexLocker locker(lock);
    for (std::vector<Effect*>::iterator it = mEffects.begin(); it != mEffects.end(); it++) {
        if ((*it)->GetSelected() != EFFECT_NOT_SELECTED) {
            IncrementChangeCount((*it)->GetStartTimeMS(), (*it)->GetEndTimeMS());
            undo_mgr.CaptureEffectToBeDeleted( mParentElement->GetName(), mIndex, (*it)->GetEffectName(),
                                               (*it)->GetSettingsAsString(), (*it)->GetPaletteAsString(),
                                               (*it)->GetStartTimeMS(), (*it)->GetEndTimeMS(),
                                               (*it)->GetSelected(), (*it)->GetProtected() );
        }
    }
    mEffects.erase(std::remove_if(mEffects.begin(), mEffects.end(),ShouldDeleteSelected),mEffects.end());
}
void EffectLayer::DeleteEffectByIndex(int idx) {
    wxMutexLocker locker(lock);
    mEffects.erase(mEffects.begin()+idx);
}
void EffectLayer::DeleteEffect(int id)
{
    wxMutexLocker locker(lock);
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetID() == id)
        {
           mEffects.erase(mEffects.begin()+i);
           break;
        }
    }
}

bool EffectLayer::ShouldDeleteSelected(Effect *eff)
{
    return eff->GetSelected() != EFFECT_NOT_SELECTED;
}

bool EffectLayer::SortEffectByStartTime(Effect *e1,Effect *e2)
{
    return e1->GetStartTimeMS() < e2->GetStartTimeMS();
}

void EffectLayer::IncrementChangeCount(int startMS, int endMS)
{
    mParentElement->IncrementChangeCount(startMS, endMS);
}


StrandLayer::~StrandLayer() {
    for (int x = 0; x < mNodeLayers.size(); x++) {
        delete mNodeLayers[x];
    }
}

NodeLayer *StrandLayer::GetNodeLayer(int n, bool create) {
    while (create && n >= mNodeLayers.size()) {
        mNodeLayers.push_back(new NodeLayer(GetParentElement()));
    }
    if (n < mNodeLayers.size()) {
        return mNodeLayers[n];
    }
    return nullptr;
}
void StrandLayer::InitFromModel(ModelClass &model) {
    int nc = model.GetStrandLength(strand);
    SetName(model.GetStrandName(strand));
    for (int x = 0; x < mNodeLayers.size(); x++) {
        mNodeLayers[x]->SetName(model.GetNodeName(x));
    }
    while (mNodeLayers.size() < nc) {
        NodeLayer *nl = new NodeLayer(GetParentElement(), model.GetNodeName(mNodeLayers.size()));
        mNodeLayers.push_back(nl);
    }
}

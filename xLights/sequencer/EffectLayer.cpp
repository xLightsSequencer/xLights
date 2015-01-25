#include "EffectLayer.h"
#include <algorithm>
#include "EffectsGrid.h"



EffectLayer::EffectLayer(Element* parent)
{
    mParentElement = parent;
}

EffectLayer::~EffectLayer()
{
    // Place at bottom of stack
    mIndex = 1000;
}

int EffectLayer::GetIndex()
{
    return mIndex;
}

void EffectLayer::SetIndex(int index)
{
    mIndex - index;
}


Effect* EffectLayer::GetEffect(int index)
{
    if(index < mEffects.size())
    {
        return &mEffects[index];
    }
    else
    {
        nullptr;
    }
}

Effect* EffectLayer::RemoveEffect(int index)
{
    if(index<mEffects.size())
    {
        mEffects.erase(mEffects.begin()+index);
    }
}

void EffectLayer::AddEffect(int id, int effectIndex, wxString name, wxString settings,double startTime,double endTime, int Selected, bool Protected)
{
    Effect e(this);
    e.SetID(id);
    e.SetEffectIndex(effectIndex);
    e.SetEffectName(name);
    e.SetSettings(settings);
    e.SetStartTime(startTime);
    e.SetEndTime(endTime);
    e.SetProtected(Protected);
    e.SetSelected(Selected);
    mEffects.push_back(e);
    SortEffects();
}


void EffectLayer::SortEffects()
{
    std::sort(mEffects.begin(),mEffects.end());
}

bool EffectLayer::IsStartTimeLinked(int index)
{
    if(index < mEffects.size())
    {
        return mEffects[index-1].GetEndTime() == mEffects[index].GetStartTime();
    }
    else
    {
        return false;
    }
}

bool EffectLayer::IsEndTimeLinked(int index)
{
    if(index < mEffects.size())
    {
        return mEffects[index].GetEndTime() == mEffects[index+1].GetStartTime();
    }
    else
    {
        return false;
    }
}

float EffectLayer::GetMaximumEndTime(int index)
{
    if(index == mEffects.size()-1)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        if(mEffects[index].GetEndTime() == mEffects[index+1].GetStartTime())
        {
            return mEffects[index+1].GetEndTime();
        }
        else
        {
            return mEffects[index+1].GetStartTime();
        }
    }
}

float EffectLayer::GetMinimumStartTime(int index)
{
    if(index == 0)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        if(mEffects[index-1].GetEndTime() == mEffects[index].GetStartTime())
        {
            return mEffects[index-1].GetStartTime();
        }
        else
        {
            return mEffects[index-1].GetEndTime();
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

bool EffectLayer::HitTestEffect(int position,int &index, int &result)
{
    bool isHit=false;
    for(int i=0;i<mEffects.size();i++)
    {
        if(mEffects[i].GetEndPosition() - mEffects[i].GetStartPosition() > 10)
        {
            int s =  mEffects[i].GetStartPosition();
            int e =  mEffects[i].GetEndPosition();
            if (position >= mEffects[i].GetStartPosition() &&
                position <= mEffects[i].GetEndPosition())
            {
                isHit = true;
                index = i;
                if(position < mEffects[i].GetStartPosition() + 5)
                {
                    result = HIT_TEST_EFFECT_LT;
                }
                else if(position > mEffects[i].GetEndPosition() - 5)
                {
                    result = HIT_TEST_EFFECT_RT;
                }
                else
                {
                    result = HIT_TEST_EFFECT_CTR;
                }
                break;
            }
        }
    }
    return isHit;
}

int EffectLayer::GetEffectIndexThatContainsPosition(int position,int &selectionType)
{
   int index=-1;
   selectionType = EFFECT_NOT_SELECTED;
    for(int i=0;i<mEffects.size();i++)
    {
        int s =  mEffects[i].GetStartPosition();
        int e =  mEffects[i].GetEndPosition();
        if (position >= mEffects[i].GetStartPosition() &&
            position <= mEffects[i].GetEndPosition())
        {
            index = i;

            if(position < mEffects[i].GetStartPosition() + 5)
            {
                selectionType = EFFECT_LT_SELECTED;
            }
            else if(position > mEffects[i].GetEndPosition() - 5)
            {
                selectionType = EFFECT_RT_SELECTED;
            }
            else
            {
                selectionType = EFFECT_SELECTED;
            }
            break;
        }
    }
    return index;
}

void EffectLayer::SelectEffectsInPositionRange(int startX,int endX,int &FirstSelected)
{
    bool FirstSelectedFound = false;
    FirstSelected = -1;
    for(int i=0;i<mEffects.size();i++)
    {
        if(mEffects[i].GetStartPosition() < 0 &&
           mEffects[i].GetEndPosition() < 0)
        {
            continue;
        }
        int center = mEffects[i].GetStartPosition() + ((mEffects[i].GetEndPosition() - mEffects[i].GetStartPosition())/2);
        int squareWidth =  center<MINIMUM_EFFECT_WIDTH_FOR_ICON?MINIMUM_EFFECT_WIDTH_FOR_SMALL_RECT:EFFECT_ICON_WIDTH;
        int squareLeft = center - (squareWidth/2);
        int squareRight = center + (squareWidth/2);
        // If selection around icon/square
        if (startX>squareLeft && endX < squareRight)
        {
            if(mEffects[i].GetSelected()==EFFECT_NOT_SELECTED)
            {
                mEffects[i].SetSelected(EFFECT_SELECTED);
                if(!FirstSelectedFound)
                {
                    FirstSelectedFound = true;
                    FirstSelected = i;
                }
            }
            else
            {
                mEffects[i].SetSelected(EFFECT_NOT_SELECTED);
            }
        }
        else if (startX<squareLeft && endX > squareRight)
        {
            if(mEffects[i].GetSelected()==EFFECT_NOT_SELECTED)
            {
                mEffects[i].SetSelected(EFFECT_SELECTED);
                if(!FirstSelectedFound)
                {
                    FirstSelectedFound = true;
                    FirstSelected = i;
                }
            }
            else
            {
                mEffects[i].SetSelected(EFFECT_NOT_SELECTED);
            }
        }
        // If selection on left side
        else if (endX>mEffects[i].GetStartPosition() && endX<squareLeft)
        {
            if(mEffects[i].GetSelected()==EFFECT_NOT_SELECTED)
            {
                mEffects[i].SetSelected(EFFECT_LT_SELECTED);
                if(!FirstSelectedFound)
                {
                    FirstSelectedFound = true;
                    FirstSelected = i;
                }
            }
            else
            {
                mEffects[i].SetSelected(EFFECT_NOT_SELECTED);
            }
        }
        // If selection on right side
        else if (startX > squareRight && startX < mEffects[i].GetEndPosition())
        {
            if(mEffects[i].GetSelected()==EFFECT_NOT_SELECTED)
            {
                mEffects[i].SetSelected(EFFECT_RT_SELECTED);
                if(!FirstSelectedFound)
                {
                    FirstSelectedFound = true;
                    FirstSelected = i;
                }
            }
            else
            {
                mEffects[i].SetSelected(EFFECT_NOT_SELECTED);
            }
        }
    }
}

void EffectLayer::UnSelectAllEffects()
{
    for(int i=0;i<mEffects.size();i++)
    {
        mEffects[i].SetSelected(EFFECT_NOT_SELECTED);
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

void EffectLayer::GetMaximumRangeOfMovementForSelectedEffects(double &toLeft,double &toRight)
{
    toLeft = NO_MAX;
    toRight = NO_MAX;
    int effectMin = 0;
    double effectMax = 0;
    for(int i=0;i<mEffects.size();i++)
    {
        if(mEffects[i].GetSelected() != EFFECT_NOT_SELECTED)
        {
            double l,r;
            GetMaximumRangeOfMovementForEffect(i,l,r);
            toLeft = toLeft<l?toLeft:l;
            toRight = toRight<r?toRight:r;
        }
    }
    int i = 0;
}

void EffectLayer::GetMaximumRangeOfMovementForEffect(int index, double &toLeft, double &toRight)
{
    switch(mEffects[index].GetSelected())
    {
        case EFFECT_LT_SELECTED:
            GetMaximumRangeWithLeftMovement(index,toLeft,toRight);
            break;
        case EFFECT_RT_SELECTED:
            GetMaximumRangeWithRightMovement(index,toLeft,toRight);
            break;
        case EFFECT_SELECTED:
            double l1,l2,r1,r2;
            GetMaximumRangeWithLeftMovement(index,l1,r1);
            GetMaximumRangeWithRightMovement(index,l2,r2);
            toLeft = l1<l2?l1:l2;
            toRight = r1<r2?r1:r2;
            break;
        default:
            break;
    }
}

void EffectLayer::GetMaximumRangeWithLeftMovement(int index, double &toLeft, double &toRight)
{
    toRight = mEffects[index].GetEndTime() - mEffects[index].GetStartTime();
    if(index == 0)
    {
       toLeft = mEffects[index].GetStartTime();
    }
    else
    {
       if(mEffects[index-1].GetSelected() == EFFECT_NOT_SELECTED)
       {
          toLeft = mEffects[index].GetStartTime() - mEffects[index-1].GetEndTime();
       }
       else if(mEffects[index-1].GetSelected() == EFFECT_RT_SELECTED)
       {
          toLeft = mEffects[index].GetStartTime() - mEffects[index-1].GetStartTime();
       }
       else if(mEffects[index-1].GetSelected() == EFFECT_SELECTED)
       {
           // Do not know so set to maximum. Let the effects to left decide
          toLeft = mEffects[index].GetStartTime();
       }
    }
}

void EffectLayer::GetMaximumRangeWithRightMovement(int index, double &toLeft, double &toRight)
{
    toLeft = mEffects[index].GetEndTime() - mEffects[index].GetStartTime();
    // Last effect, nothing to right to stop movement other then edge of screen.
    // Let grid take care of screen boundary so set to huge number
    if(index == mEffects.size()-1)
    {
       toRight = NO_MAX;
    }
    else
    {
       if(mEffects[index+1].GetSelected() == EFFECT_NOT_SELECTED)
       {
          toRight = mEffects[index+1].GetStartTime() - mEffects[index].GetEndTime();
       }
       else if(mEffects[index+1].GetSelected() == EFFECT_LT_SELECTED)
       {
          toRight = mEffects[index+1].GetEndTime() - mEffects[index].GetEndTime();
       }
       else if(mEffects[index+1].GetSelected() == EFFECT_SELECTED)
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
        if(mEffects[i].GetSelected() != EFFECT_NOT_SELECTED)
        {
           count++;
        }
    }
    return count;
}

void EffectLayer::MoveAllSelectedEffects(double delta)
{
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i].GetSelected() == EFFECT_LT_SELECTED || mEffects[i].GetSelected() == EFFECT_SELECTED)
        {
            mEffects[i].SetStartTime( mEffects[i].GetStartTime()+ delta);
        }

        if(mEffects[i].GetSelected() == EFFECT_RT_SELECTED || mEffects[i].GetSelected() == EFFECT_SELECTED)
        {
            mEffects[i].SetEndTime( mEffects[i].GetEndTime()+ delta);
        }
    }

}


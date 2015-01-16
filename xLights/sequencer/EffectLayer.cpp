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

void EffectLayer::AddEffect(int id, int effectIndex, wxString name, wxString settings,double startTime,double endTime, bool Protected)
{
    Effect e(this);
    e.SetID(id);
    e.SetEffectIndex(effectIndex);
    e.SetEffectName(name);
    e.SetSettings(settings);
    e.SetStartTime(startTime);
    e.SetEndTime(endTime);
    e.SetProtected(Protected);
    e.SetSelected(false);
    mEffects.push_back(e);
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

int EffectLayer::GetMaximumEndTime(int index)
{
    if(index == mEffects.size()-1)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        return mEffects[index+1].GetStartTime();
    }
}

int EffectLayer::GetMinimumStartTime(int index)
{
    if(index == 0)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        return mEffects[index-1].GetEndTime();
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

int EffectLayer::GetEffectIndexThatContainsPosition(int position)
{
   int index=-1;
    for(int i=0;i<mEffects.size();i++)
    {
        int s =  mEffects[i].GetStartPosition();
        int e =  mEffects[i].GetEndPosition();
        if (position >= mEffects[i].GetStartPosition() &&
            position <= mEffects[i].GetEndPosition())
        {
            index = i;
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



#include "ElementEffects.h"
#include <algorithm>
#include <time.h>

ElementEffects::ElementEffects()
{
}

ElementEffects::~ElementEffects()
{
}

void ElementEffects::AddEffect(wxString effect,double startTime,double endTime, bool Protected)
{
    Effect_Struct es;
    srand(time(NULL));
    es.id = rand();                 // Random ID.
    es.Effect = effect;
    es.StartTime = startTime;
    es.EndTime = endTime;
    es.Protected = Protected;
    es.Selected = false;
    Effects.push_back(es);
    Sort();
}

void ElementEffects::Sort()
{
    if (Effects.size()>1)
        std::sort(Effects.begin(),Effects.end(),SortByTime);
}

bool ElementEffects::IsStartTimeLinked(int index)
{
    if(index < Effects.size())
    {
        return Effects[index-1].EndTime == Effects[index].StartTime;
    }
    else
    {
        return false;
    }
}

bool ElementEffects::IsEndTimeLinked(int index)
{
    if(index < Effects.size())
    {
        return Effects[index].EndTime == Effects[index+1].StartTime;
    }
    else
    {
        return false;
    }
}

int ElementEffects::GetMaximumEndTime(int index)
{
    if(index == Effects.size()-1)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        return Effects[index+1].StartTime;
    }
}

int ElementEffects::GetMinimumStartTime(int index)
{
    if(index == 0)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        return Effects[index-1].EndTime;
    }
}


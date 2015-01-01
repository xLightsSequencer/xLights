#include "ElementEffects.h"
#include <algorithm>
#include <time.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

ElementEffects::ElementEffects()
{
}

ElementEffects::~ElementEffects()
{
}

int ElementEffects::GetEffectIndex(wxString effectName)
{
    if(effectName=="Bars"){return EFFECT_BARS;}
    else if(effectName=="Butterfly"){return EFFECT_BUTTERFLY;}
    else if(effectName=="Circles"){return EFFECT_CIRCLES;}
    else if(effectName=="Color Wash"){return EFFECT_COLORWASH;}
    else if(effectName=="Curtain"){return EFFECT_CURTAIN;}
    else if(effectName=="Fire"){return EFFECT_FIRE;}
    else if(effectName=="Fireworks"){return EFFECT_FIREWORKS;}
    else if(effectName=="Garlands"){return EFFECT_GARLANDS;}
    else if(effectName=="Glediator"){return EFFECT_GLEDIATOR;}
    else if(effectName=="Life"){return EFFECT_LIFE;}
    else if(effectName=="Meteors"){return EFFECT_METEORS;}
    else if(effectName=="Pinwheel"){return EFFECT_PINWHEEL;}
    else if(effectName=="Ripple"){return EFFECT_RIPPLE;}
    else if(effectName=="SingleStrand"){return EFFECT_SINGLESTRAND;}
    else if(effectName=="Snowflakes"){return EFFECT_SNOWFLAKES;}
    else if(effectName=="Snowstorm"){return EFFECT_SNOWSTORM;}
    else if(effectName=="Spirals"){return EFFECT_SPIRALS;}
    else if(effectName=="Spirograph"){return EFFECT_SPIROGRAPH;}
    else if(effectName=="Tree"){return EFFECT_TREE;}
    else if(effectName=="Twinkle"){return EFFECT_TWINKLE;}
    else if(effectName=="Wave"){return EFFECT_WAVE;}
    else{return EFFECT_BARS;}
}

wxString ElementEffects::GetEffectNameFromEffectText(wxString effectText)
{
    wxString effect="";
    wxArrayString effectParameters=wxSplit(effectText,',');
    if (effectParameters.size()>0)
    {
        effect = effectParameters[0];
    }
    return effect;
}


void ElementEffects::AddEffect(int id,wxString effect,int effectIndex,double startTime,double endTime, bool Protected)
{
    Effect_Struct es;
    es.ID = id;                 // Random ID.
    es.Effect = effect;
    es.EffectIndex = effectIndex;
    es.StartTime = startTime;
    es.EndTime = endTime;
    es.Protected = Protected;
    es.Selected = false;
    mEffects.push_back(es);
    Sort();
}



void ElementEffects::Sort()
{
    if (mEffects.size()>1)
        std::sort(mEffects.begin(),mEffects.end(),SortByTime);
}

bool ElementEffects::IsStartTimeLinked(int index)
{
    if(index < mEffects.size())
    {
        return mEffects[index-1].EndTime == mEffects[index].StartTime;
    }
    else
    {
        return false;
    }
}

bool ElementEffects::IsEndTimeLinked(int index)
{
    if(index < mEffects.size())
    {
        return mEffects[index].EndTime == mEffects[index+1].StartTime;
    }
    else
    {
        return false;
    }
}

int ElementEffects::GetMaximumEndTime(int index)
{
    if(index == mEffects.size()-1)
    {
        return NO_MIN_MAX_TIME;
    }
    else
    {
        return mEffects[index+1].StartTime;
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
        return mEffects[index-1].EndTime;
    }
}

int ElementEffects::GetEffectCount()
{
    if(mEffects.empty())
        return 0;
    else
        return mEffects.size();
}

bool ElementEffects::IsEffectStartTimeInRange(int index, float startTime,float endTime)
{
    return  (mEffects[index].StartTime >= startTime &&
             mEffects[index].StartTime <= endTime)?true:false;
}

bool ElementEffects::IsEffectEndTimeInRange(int index, float startTime,float endTime)
{
    return  (mEffects[index].EndTime >= startTime &&
             mEffects[index].EndTime <= endTime)?true:false;
}


Effect_Struct* ElementEffects::GetEffect(int index)
{
    return &mEffects[index];
}

double ElementEffects::RoundToMultipleOfPeriod(double number,double frequency)
{
    double period = (double)1/frequency;
    int i = (int)(number/period);
    double d = number/period;
    if(d-(double)i < .5)
    {
        return ((double)i * period);
    }
    else
    {
        return ((double)(i+1) * period);
    }
}





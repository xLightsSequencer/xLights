#include "Effect.h"
#include "EffectLayer.h"

class xLightsFrame;

Effect::Effect(EffectLayer* parent)
{
    mParentLayer = parent;
}

Effect::~Effect()
{
}

int Effect::GetID()
{
    return mID;
}

void Effect::SetID(int id)
{
    mID = id;
}


wxString Effect::GetSettings()
{
    return mSettings;
}

void Effect::SetSettings(wxString settings)
{
    mSettings = settings;
    mDirty = true;
}

wxString Effect::GetEffectName()
{
    return mName;
}

void Effect::SetEffectName(wxString name)
{
    mName = name;
}

int Effect::GetEffectIndex()
{
    return mEffectIndex;
}

void Effect::SetEffectIndex(int effectIndex)
{
    mEffectIndex = effectIndex;
    mDirty = true;
}


double Effect::GetStartTime() const
{
    return mStartTime;
}

void Effect::SetStartTime(double startTime)
{
   mStartTime = startTime;
   mDirty = true;
}

double Effect::GetEndTime()
{
    return mEndTime;
}

void Effect::SetEndTime(double endTime)
{
    mEndTime = endTime;
    mDirty = true;
}


int Effect::GetSelected()
{
    return mSelected;
}

void Effect::SetSelected(int selected)
{
    mSelected = selected;
}

bool Effect::GetProtected()
{
    return mProtected;
}
void Effect::SetProtected(bool Protected)
{
    mProtected = Protected;
}

int Effect::GetStartPosition()
{
    return mStartPosition;
}

void Effect::SetStartPosition(int position)
{
    mStartPosition = position;
}

int Effect::GetEndPosition()
{
    return mEndPosition;
}

void Effect::SetEndPosition(int position)
{
    mEndPosition = position;
}

bool Effect::IsDirty()
{
    return mDirty;
}

void Effect::SetDirty(bool dirty)
{
    mDirty = dirty;
}

bool operator<(const Effect &e1, const Effect &e2){
    if(e1.GetStartTime() < e2.GetStartTime())
        return true;
    else
        return false;
}

int Effect::GetEffectIndex(wxString effectName)
{
    if(effectName=="Bars"){return eff_ON;}
    else if(effectName=="Bars"){return EFFECT_BARS;}
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



EffectLayer* Effect::GetParentEffectLayer()
{
    return mParentLayer;
}

void Effect::SetParentEffectLayer(EffectLayer* parent)
{
    mParentLayer = parent;
}


#include "Effect.h"
#include "EffectLayer.h"
#include "../xLightsMain.h"


Effect::Effect(EffectLayer* parent)
{
    mParentLayer = parent;
    changeCount = 0;
}

Effect::~Effect()
{
}

int Effect::GetID() const
{
    return mID;
}

void Effect::SetID(int id)
{
    mID = id;
}


void Effect::SetSettings(const wxString &settings)
{
    mSettings.Parse(settings);
    IncrementChangeCount();
    mDirty = true;
}

void Effect::SetPalette(const wxString& i)
{
    mPaletteMap.Parse(i);
    mColors.clear();
    for (int i = 1; i <= 6; i++) {
        if (mPaletteMap[wxString::Format("C_CHECKBOX_Palette%d",i)] ==  "1") {
            mColors.push_back(xlColor(mPaletteMap[wxString::Format("C_BUTTON_Palette%d",i)]));
        }
    }
    IncrementChangeCount();
    mDirty = true;
}


wxString Effect::GetEffectName() const
{
    return mName;
}

void Effect::SetEffectName(const wxString & name)
{
    mName = name;
    IncrementChangeCount();
}

int Effect::GetEffectIndex() const
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
   IncrementChangeCount();
   mDirty = true;
}

double Effect::GetEndTime() const
{
    return mEndTime;
}

void Effect::SetEndTime(double endTime)
{
    mEndTime = endTime;
    IncrementChangeCount();
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
    if(effectName=="Off"){return xLightsFrame::RGB_EFFECTS_e::eff_OFF;}
    else if(effectName=="On"){return xLightsFrame::RGB_EFFECTS_e::eff_ON;}
    else if(effectName=="Bars"){return xLightsFrame::RGB_EFFECTS_e::eff_BARS;}
    else if(effectName=="Butterfly"){return xLightsFrame::RGB_EFFECTS_e::eff_BUTTERFLY;}
    else if(effectName=="Circles"){return xLightsFrame::RGB_EFFECTS_e::eff_CIRCLES;}
    else if(effectName=="Color Wash"){return xLightsFrame::RGB_EFFECTS_e::eff_COLORWASH;}
    else if(effectName=="Curtain"){return xLightsFrame::RGB_EFFECTS_e::eff_CURTAIN;}
    else if(effectName=="Faces"){return xLightsFrame::RGB_EFFECTS_e::eff_FACES;}
    else if(effectName=="Fire"){return xLightsFrame::RGB_EFFECTS_e::eff_FIRE;}
    else if(effectName=="Fireworks"){return xLightsFrame::RGB_EFFECTS_e::eff_FIREWORKS;}
    else if(effectName=="Garlands"){return xLightsFrame::RGB_EFFECTS_e::eff_GARLANDS;}
    else if(effectName=="Glediator"){return xLightsFrame::RGB_EFFECTS_e::eff_GLEDIATOR;}
    else if(effectName=="Life"){return xLightsFrame::RGB_EFFECTS_e::eff_LIFE;}
    else if(effectName=="Meteors"){return xLightsFrame::RGB_EFFECTS_e::eff_METEORS;}
    else if(effectName=="Morph"){return xLightsFrame::RGB_EFFECTS_e::eff_MORPH;}
    else if(effectName=="Piano"){return xLightsFrame::RGB_EFFECTS_e::eff_PIANO;}
    else if(effectName=="Pictures"){return xLightsFrame::RGB_EFFECTS_e::eff_PICTURES;}
    else if(effectName=="Pinwheel"){return xLightsFrame::RGB_EFFECTS_e::eff_PINWHEEL;}
    else if(effectName=="Ripple"){return xLightsFrame::RGB_EFFECTS_e::eff_RIPPLE;}
    else if(effectName=="Shimmer"){return xLightsFrame::RGB_EFFECTS_e::eff_SHIMMER;}
    else if(effectName=="SingleStrand"){return xLightsFrame::RGB_EFFECTS_e::eff_SINGLESTRAND;}
    else if(effectName=="Snowflakes"){return xLightsFrame::RGB_EFFECTS_e::eff_SNOWFLAKES;}
    else if(effectName=="Snowstorm"){return xLightsFrame::RGB_EFFECTS_e::eff_SNOWSTORM;}
    else if(effectName=="Spirals"){return xLightsFrame::RGB_EFFECTS_e::eff_SPIRALS;}
    else if(effectName=="Spirograph"){return xLightsFrame::RGB_EFFECTS_e::eff_SPIROGRAPH;}
    else if(effectName=="Strobe"){return xLightsFrame::RGB_EFFECTS_e::eff_STROBE;}
    else if(effectName=="Text"){return xLightsFrame::RGB_EFFECTS_e::eff_TEXT;}
    else if(effectName=="Tree"){return xLightsFrame::RGB_EFFECTS_e::eff_TREE;}
    else if(effectName=="Twinkle"){return xLightsFrame::RGB_EFFECTS_e::eff_TWINKLE;}
    else if(effectName=="Wave"){return xLightsFrame::RGB_EFFECTS_e::eff_WAVE;}
    else{return xLightsFrame::RGB_EFFECTS_e::eff_OFF;}
}

EffectLayer* Effect::GetParentEffectLayer()
{
    return mParentLayer;
}

void Effect::SetParentEffectLayer(EffectLayer* parent)
{
    mParentLayer = parent;
}

void Effect::IncrementChangeCount()
{
    mParentLayer->IncrementChangeCount();
    changeCount++;
}



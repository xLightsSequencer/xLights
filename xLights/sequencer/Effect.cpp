
#include "../UtilClasses.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "../effects/EffectManager.h"

#include "../../include/globals.h"


#include <unordered_map>

class ControlRenameMap
{
public:
    ControlRenameMap()
    {
        data["E_TEXTCTRL_Morph_Start_X1"] = "";
        data["E_TEXTCTRL_Morph_Start_Y1"] = "";
        data["E_TEXTCTRL_Morph_Start_X2"] = "";
        data["E_TEXTCTRL_Morph_Start_Y2"] = "";
        data["E_TEXTCTRL_MorphStartLength"] = "";
        data["E_TEXTCTRL_Morph_End_X1"] = "";
        data["E_TEXTCTRL_Morph_End_Y1"] = "";
        data["E_TEXTCTRL_Morph_End_X2"] = "";
        data["E_TEXTCTRL_Morph_End_Y2"] = "";
        data["E_TEXTCTRL_MorphEndLength"] = "";
        data["E_TEXTCTRL_MorphDuration"] = "";
        data["E_TEXTCTRL_MorphAccel"] = "";
        data["E_TEXTCTRL_Pictures_GifSpeed"] = "";
        data["E_TEXTCTRL_PicturesXC"] = "";
        data["E_TEXTCTRL_PicturesYC"] = "";
        data["E_CHECKBOX_MorphUseHeadStartColor"] = "";
        data["E_CHECKBOX_MorphUseHeadEndColor"] = "";
        data["E_SLIDER_Chase_Spacing1"] = "";
        data["E_CHECKBOX_Shimmer_Blink_Timing"] = "";
        data["E_SLIDER_Shimmer_Blinks_Per_Row"] = "";

        data["E_NOTEBOOK_Text1"] = "";
        data["E_TEXTCTRL_Pictures_Filename"] = "E_FILEPICKER_Pictures_Filename";
        data["E_TEXTCTRL_Text_Font1"] = "E_FONTPICKER_Text_Font1";
        data["E_TEXTCTRL_Text_Font2"] = "E_FONTPICKER_Text_Font2";
        data["E_TEXTCTRL_Text_Font3"] = "E_FONTPICKER_Text_Font3";
        data["E_TEXTCTRL_Text_Font4"] = "E_FONTPICKER_Text_Font4";

        data["E_CHOICE_CoroFaces_Phoneme"] = "E_CHOICE_Faces_Phoneme";
        data["E_CHOICE_CoroFaces_Eyes"] = "E_CHOICE_Faces_Eyes";
        data["E_CHECKBOX_CoroFaces_Outline"] = "E_CHECKBOX_Faces_Outline";
        data["E_CHECKBOX_CoroFaces_InPapagayo"] = "";
        data["E_CHECKBOX_Faces_InPapagayo"] = "";
        data["E_CHOICE_CoroFaces_TimingTrack"] = "E_CHOICE_Faces_TimingTrack";
        data["E_CHOICE_CoroFaces_FaceDefinition"] = "E_CHOICE_Faces_FaceDefinition";
    }
    const void map(std::string &n) const
    {
        std::unordered_map<std::string, std::string>::const_iterator it = data.find(n);
        if (it != data.end())
        {
            n = it->second;
        }
    }
private:
    std::unordered_map<std::string, std::string> data;
} Remaps;

const std::string MapStringString::EMPTY_STRING;

void SettingsMap::RemapChangedSettingKey(std::string &n,  std::string &value)
{
    Remaps.map(n);
}


static void ParseColorMap(const SettingsMap &mPaletteMap, xlColorVector &mColors) {
    mColors.clear();
    if (!mPaletteMap.empty()) {
        if (mPaletteMap.GetBool("C_CHECKBOX_Palette1")) {
             mColors.push_back(xlColor(mPaletteMap["C_BUTTON_Palette1"]));
        }
        if (mPaletteMap.GetBool("C_CHECKBOX_Palette2")) {
            mColors.push_back(xlColor(mPaletteMap["C_BUTTON_Palette2"]));
        }
        if (mPaletteMap.GetBool("C_CHECKBOX_Palette3")) {
            mColors.push_back(xlColor(mPaletteMap["C_BUTTON_Palette3"]));
        }
        if (mPaletteMap.GetBool("C_CHECKBOX_Palette4")) {
            mColors.push_back(xlColor(mPaletteMap["C_BUTTON_Palette4"]));
        }
        if (mPaletteMap.GetBool("C_CHECKBOX_Palette5")) {
            mColors.push_back(xlColor(mPaletteMap["C_BUTTON_Palette5"]));
        }
        if (mPaletteMap.GetBool("C_CHECKBOX_Palette6")) {
            mColors.push_back(xlColor(mPaletteMap["C_BUTTON_Palette6"]));
        }
    }
}

Effect::Effect(EffectLayer* parent,int id, const std::string & name, const std::string &settings, const std::string &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected)
    : mParentLayer(parent), mID(id), mEffectIndex(-1), mName(nullptr),
      mStartTime(startTimeMS), mEndTime(endTimeMS), mSelected(Selected), mProtected(Protected)
{
    mEffectIndex = parent->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(name);
    mSettings.Parse(settings);

    if (mEndTime < mStartTime)
    {
        //should never happend, but if we load something with invalid times, make sure we can at least
        //show/select/delete the effect
        int tmp = mStartTime;
        mStartTime = mEndTime;
        mEndTime = tmp;
    }
    if (mEffectIndex == -1) {
        mName = new std::string(name);
    }

    mPaletteMap.Parse(palette);
    ParseColorMap(mPaletteMap, mColors);
}


Effect::~Effect()
{
    if (mName != nullptr)
    {
        delete mName;
    }
}


int Effect::GetID() const
{
    return mID;
}
void Effect::SetID(int id)
{
    mID = id;
}
void Effect::CopySettingsMap(SettingsMap &target, bool stripPfx) const
{
    wxMutexLocker lock(settingsLock);

    for (std::map<std::string,std::string>::const_iterator it=mSettings.begin(); it!=mSettings.end(); ++it)
    {
        std::string name = it->first;
        if (stripPfx && name[1] == '_')
        {
            name = name.substr(2);
        }
        target[name] = it->second;
    }
}
void Effect::CopyPalette(xlColorVector &target) const
{
    wxMutexLocker lock(settingsLock);
    target = mColors;
}

void Effect::SetSettings(const std::string &settings)
{
    wxMutexLocker lock(settingsLock);
    mSettings.Parse(settings);
    IncrementChangeCount();
}

std::string Effect::GetSettingsAsString() const
{
    wxMutexLocker lock(settingsLock);
    return mSettings.AsString();
}

void Effect::SetPalette(const std::string& i)
{
    wxMutexLocker lock(settingsLock);
    mPaletteMap.Parse(i);
    mColors.clear();
    IncrementChangeCount();
    if (mPaletteMap.empty())
    {
        return;
    }
    ParseColorMap(mPaletteMap, mColors);
}

std::string Effect::GetPaletteAsString() const
{
    wxMutexLocker lock(settingsLock);
    return mPaletteMap.AsString();
}

const std::string &Effect::GetEffectName() const
{
    if (mName != nullptr)
    {
        return *mName;
    }
    return GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectName(mEffectIndex);
}

void Effect::SetEffectName(const std::string & name)
{
    int idx = GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(name);
    if (mEffectIndex != idx || mEffectIndex == -1)
    {
        mEffectIndex = idx;
        if (mName != nullptr)
        {
            delete mName;
            mName = nullptr;
        }
        if (mEffectIndex == -1)
        {
            mName = new std::string(name);
        }
        IncrementChangeCount();
    }
}

int Effect::GetEffectIndex() const
{
    return mEffectIndex;
}

void Effect::SetEffectIndex(int effectIndex)
{
    if (mEffectIndex != effectIndex)
    {
        mEffectIndex = effectIndex;
        IncrementChangeCount();
    }
}

int Effect::GetStartTimeMS() const
{
    return mStartTime;
}

void Effect::SetStartTimeMS(int startTimeMS)
{
    if (startTimeMS > mStartTime)
    {
        IncrementChangeCount();
        mStartTime = startTimeMS;
    }
    else
    {
        mStartTime = startTimeMS;
        IncrementChangeCount();
    }
}
int Effect::GetEndTimeMS() const
{
    return mEndTime;
}

void Effect::SetEndTimeMS(int endTimeMS)
{
    if (endTimeMS < mEndTime)
    {
        IncrementChangeCount();
        mEndTime = endTimeMS;
    }
    else
    {
        mEndTime = endTimeMS;
        IncrementChangeCount();
    }
}


int Effect::GetSelected() const
{
    return mSelected;
}

void Effect::SetSelected(int selected)
{
    mSelected = selected;
}

bool Effect::GetProtected() const
{
    return mProtected;
}
void Effect::SetProtected(bool Protected)
{
    mProtected = Protected;
}

bool operator<(const Effect &e1, const Effect &e2)
{
    if(e1.GetStartTimeMS() < e2.GetStartTimeMS())
        return true;
    else
        return false;
}

EffectLayer* Effect::GetParentEffectLayer() const
{
    return mParentLayer;
}

void Effect::SetParentEffectLayer(EffectLayer* parent)
{
    mParentLayer = parent;
}

void Effect::IncrementChangeCount()
{
    mParentLayer->IncrementChangeCount(GetStartTimeMS(), GetEndTimeMS());
}



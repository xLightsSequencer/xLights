
#include "../UtilClasses.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "../effects/EffectManager.h"
#include "../ColorCurve.h"
#include <log4cpp/Category.hh>
#include "../../include/globals.h"
#include <unordered_map>
#include "../SequenceCheck.h"

wxDEFINE_EVENT(EVT_SETTIMINGTRACKS, wxCommandEvent);

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

        data["T_SLIDER_EffectBlur"] = "B_SLIDER_Blur";
        data["B_SLIDER_EffectBlur"] = "B_SLIDER_Blur";
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

static std::vector<std::string> CHECKBOX_IDS {
    "C_CHECKBOX_Palette1", "C_CHECKBOX_Palette2", "C_CHECKBOX_Palette3",
    "C_CHECKBOX_Palette4", "C_CHECKBOX_Palette5", "C_CHECKBOX_Palette6",
    "C_CHECKBOX_Palette7", "C_CHECKBOX_Palette8"
};
static std::vector<std::string> BUTTON_IDS {
    "C_BUTTON_Palette1", "C_BUTTON_Palette2", "C_BUTTON_Palette3",
    "C_BUTTON_Palette4", "C_BUTTON_Palette5", "C_BUTTON_Palette6",
    "C_BUTTON_Palette7", "C_BUTTON_Palette8"
};

static void ParseColorMap(const SettingsMap &mPaletteMap, xlColorVector &mColors, xlColorCurveVector& mCC) {
    mColors.clear();
    mCC.clear();
    if (!mPaletteMap.empty()) {
        int sz = BUTTON_IDS.size();
        for (int x = 0; x < sz; ++x) {
            if (mPaletteMap.GetBool(CHECKBOX_IDS[x])) {
                if (mPaletteMap[BUTTON_IDS[x]].find("Active") != std::string::npos)
                {
                    mCC.push_back(ColorCurve(mPaletteMap[BUTTON_IDS[x]]));
                    ColorCurve cv = ColorCurve(mPaletteMap[BUTTON_IDS[x]]);
                    mColors.push_back(cv.GetValueAt(0));
                }
                else
                {
                    mCC.push_back(ColorCurve(""));
                    mColors.push_back(xlColor(mPaletteMap[BUTTON_IDS[x]]));
                }
            }
        }
    }
}

Effect::Effect(EffectLayer* parent,int id, const std::string & name, const std::string &settings, const std::string &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected)
    : mParentLayer(parent), mID(id), mEffectIndex(-1), mName(nullptr),
      mStartTime(startTimeMS), mEndTime(endTimeMS), mSelected(Selected), mTagged(false), mProtected(Protected)
{
    mEffectIndex = parent->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(name);
    mSettings.Parse(settings);

    // Fixes an erroneous blank settings created by using:
    //  settings["key"] == "test val"
    // code which as a side effect creates a blank value under the key
    // an example of this is fix to issue #622
    if (mSettings.Get("T_CHOICE_Out_Transition_Type", "XXX") == "")
    {
        mSettings.erase("T_CHOICE_Out_Transition_Type");
    }
    if (mSettings.Get("Converted", "XXX") == "")
    {
        mSettings.erase("Converted");
    }

    // check for any other odd looking blank settings
    for (auto it = mSettings.begin(); it != mSettings.end(); ++it)
    {
        if (it->second == "")
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("Effect '%s' on model '%s' at time '%s' has setting '%s' with a blank value.",
                (const char *)name.c_str(),
                (const char *)parent->GetParentElement()->GetName().c_str(),
                FORMATTIME(startTimeMS),
                (const char *)it->first.c_str()
                );
        }
    }

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
    ParseColorMap(mPaletteMap, mColors, mCC);
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
    std::unique_lock<std::mutex> lock(settingsLock);

    for (std::map<std::string,std::string>::const_iterator it=mSettings.begin(); it!=mSettings.end(); ++it)
    {
        std::string name = it->first;
        if (stripPfx && name[1] == '_')
        {
            name = name.substr(2);
        }
        target[name] = it->second;
    }
    for (std::map<std::string,std::string>::const_iterator it=mPaletteMap.begin(); it!=mPaletteMap.end(); ++it)
    {
        std::string name = it->first;
        if (stripPfx && name[1] == '_'  && (name[2] == 'S' || name[2] == 'C' || name[2] == 'V')) //only need the slider, checkbox and value curve entries
        {
            name = name.substr(2);
            target[name] = it->second;
        }
    }
}
void Effect::CopyPalette(xlColorVector &target, xlColorCurveVector& newcc) const
{
    std::unique_lock<std::mutex> lock(settingsLock);
    target = mColors;
    newcc = mCC;
}

void Effect::SetSettings(const std::string &settings, bool keepxsettings)
{
    std::unique_lock<std::mutex> lock(settingsLock);

    SettingsMap x;
    if (keepxsettings)
    {
        for (auto it = mSettings.begin(); it != mSettings.end(); ++it)
        {
            if (it->first.size() > 2 && it->first[0] == 'X' && it->first[1] == '_')
            {
                x[it->first] = it->second;
            }
        }
    }
    mSettings.Parse(settings);
    if (keepxsettings)
    {
        for (auto it = x.begin(); it != x.end(); ++it)
        {
                mSettings[it->first] = it->second;
        }
    }
    IncrementChangeCount();
}

std::string Effect::GetSettingsAsString() const
{
    std::unique_lock<std::mutex> lock(settingsLock);
    return mSettings.AsString();
}

void Effect::SetPalette(const std::string& i)
{
    std::unique_lock<std::mutex> lock(settingsLock);
    mPaletteMap.Parse(i);
    mColors.clear();
    mCC.clear();
    IncrementChangeCount();
    if (mPaletteMap.empty())
    {
        return;
    }
    ParseColorMap(mPaletteMap, mColors, mCC);
}
void Effect::PaletteMapUpdated() {
    std::unique_lock<std::mutex> lock(settingsLock);
    mColors.clear();
    mCC.clear();
    IncrementChangeCount();
    if (mPaletteMap.empty())
    {
        return;
    }
    ParseColorMap(mPaletteMap, mColors, mCC);
}

std::string Effect::GetPaletteAsString() const
{
    std::unique_lock<std::mutex> lock(settingsLock);
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
        background.LockedClear();
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
        background.LockedClear();
    }
}

wxString Effect::GetDescription() const
{
    if (mSettings.Contains("X_Effect_Description"))
    {
        return mSettings["X_Effect_Description"];
    }
    return "";
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

bool Effect::OverlapsWith(int startTimeMS, int EndTimeMS)
{
    return (startTimeMS < GetEndTimeMS() && EndTimeMS > GetStartTimeMS());
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

bool Effect::GetTagged() const
{
    return mTagged;
}

void Effect::SetTagged(bool tagged)
{
    mTagged = tagged;
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



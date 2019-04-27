#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "../effects/EffectManager.h"
#include "../ColorCurve.h"
#include "../UtilFunctions.h"
#include "../ValueCurve.h"
#include "../UtilClasses.h"
#include "../RenderCache.h"
#include "../models/Model.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../effects/RenderableEffect.h"

#include <unordered_map>

#include <log4cpp/Category.hh>

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

bool rangesort(const std::pair<int, int> first, const std::pair<int, int> second)
{
    if (first.first == second.first)
    {
        return first.second < second.second;
    }

    return first.first < second.first;
}

void RangeAccumulator::ResolveOverlaps(int minSeparation)
{
    _ranges.sort(rangesort);

    auto it1 = _ranges.begin();
    auto it2 = it1;

    while (it1 != _ranges.end())
    {
        ++it2;
        if (it2 != _ranges.end())
        {
            if (it2->first - minSeparation <= it1->second)
            {
                if (it1->second < it2->second)
                {
                    it1->second = it2->second;
                }
                _ranges.erase(it2);
                it2 = it1;
                continue;
            }
        }
        ++it1;
    }
}

void RangeAccumulator::Add(int low, int high)
{
    _ranges.push_back(std::pair<int, int>(low, high));
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

void Effect::ParseColorMap(const SettingsMap &mPaletteMap, xlColorVector &mColors, xlColorCurveVector& mCC) {
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

#pragma region Constructors and Destructors

Effect::Effect(EffectLayer* parent,int id, const std::string & name, const std::string &settings, const std::string &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected)
    : mParentLayer(parent), mID(id), mEffectIndex(-1), mName(nullptr),
      mStartTime(startTimeMS), mEndTime(endTimeMS), mSelected(Selected), mTagged(false), mProtected(Protected), mCache(nullptr)
{
    mColorMask = xlColor::NilColor();
    mEffectIndex = (parent->GetParentElement() == nullptr) ? -1 : parent->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectIndex(name);
    mSettings.Parse(settings);

    Element* parentElement = parent->GetParentElement();
    if (parentElement != nullptr)
    {
        Model* model = parentElement->GetSequenceElements()->GetXLightsFrame()->AllModels[parentElement->GetModelName()];
        FixBuffer(model);
    }

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
                (const char *)(parent->GetParentElement() == nullptr ? "" : parent->GetParentElement()->GetName().c_str()),
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
    if (mCache) {
        mCache->Delete();
        mCache = nullptr;
    }
    if (mName != nullptr)
    {
        delete mName;
    }
}

#pragma endregion

void Effect::SetEffectIndex(int effectIndex)
{
    if (mEffectIndex != effectIndex)
    {
        mEffectIndex = effectIndex;
        IncrementChangeCount();
        background.LockedClear();
    }
}

const std::string &Effect::GetEffectName() const
{
    if (mName != nullptr)
    {
        return *mName;
    }
    return GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectName(mEffectIndex);
}

const std::string& Effect::GetEffectName(int index) const
{
    if (index < 0)
    {
        return GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectName(mEffectIndex);
    }
    return GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetEffectManager().GetEffectName(index);
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

wxString Effect::GetDescription() const
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    if (mSettings.Contains("X_Effect_Description"))
    {
        return mSettings["X_Effect_Description"];
    }
    return "";
}

void Effect::SetStartTimeMS(int startTimeMS)
{
    wxASSERT(!IsLocked());

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

void Effect::SetEndTimeMS(int endTimeMS)
{
    wxASSERT(!IsLocked());

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

bool Effect::OverlapsWith(int startTimeMS, int EndTimeMS)
{
    return (startTimeMS < GetEndTimeMS() && EndTimeMS > GetStartTimeMS());
}

void Effect::ConvertTo(int effectIndex)
{
    if (effectIndex != mEffectIndex)
    {
        SetEffectIndex(effectIndex);
        SettingsMap newSettings;
        // remove any E_ settings as the effect type has changed
        for (auto it : mSettings)
        {
            if (!StartsWith(it.first, "E_"))
            {
                newSettings[it.first] = it.second;
            }
        }
        mSettings = newSettings;

        std::string palette;
        std::string effectText = xLightsApp::GetFrame()->GetEffectTextFromWindows(palette);

        auto es = wxSplit(effectText, ',');
        for (auto it: es)
        {
            if (StartsWith(it, "E_"))
            {
                auto sv = wxSplit(it, '=');
                if (sv.size()==2)
                {
                    mSettings[sv[0]] = sv[1];
                }
            }
        }
    }
}

bool Effect::IsLocked() const
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    return mSettings.Contains("X_Effect_Locked");
}

void Effect::SetLocked(bool lock)
{
    std::unique_lock<std::recursive_mutex> getlock(settingsLock);
    if (lock)
    {
        mSettings["X_Effect_Locked"] = "True";
    }
    else
    {
        mSettings.erase("X_Effect_Locked");
    }
}

void Effect::IncrementChangeCount()
{
    mParentLayer->IncrementChangeCount(GetStartTimeMS(), GetEndTimeMS());
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    if (mCache) {
        mCache->Delete();
        mCache = nullptr;
    }
}

std::string Effect::GetSettingsAsString() const
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    return mSettings.AsString();
}

void Effect::SetSettings(const std::string &settings, bool keepxsettings)
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);

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

void Effect::PressButton(RenderableEffect* re, const std::string& id)
{
    bool changed = false;
    if (StartsWith(id, "E_"))
    {
        changed = re->PressButton(id, mPaletteMap, mSettings);
    }
    else
    {
        // all other button press changes need to be handled here
    }

    if (changed) IncrementChangeCount();
}

void Effect::ApplySetting(const std::string& id, const std::string& value, ValueCurve* vc, const std::string& vcid)
{
    wxString idd(id);
    if (idd.StartsWith("C_"))
    {
        if (vc != nullptr && vc->IsActive())
        {
            mPaletteMap[vcid] = vc->Serialise();
        }
        else
        {
            mPaletteMap.erase(vcid);
            mPaletteMap[id] = value;
        }
    }
    else
    {
        if (vc != nullptr && vc->IsActive())
        {
            mSettings[vcid] = vc->Serialise();
        }
        else
        {
            mSettings.erase(vcid);

            wxString wid = id;

            if (wid.Contains("FILEPICKER")) {
                wxString realid = wid.substr(0, wid.Length() - 3);
                if (wid.EndsWith("_FN")) {
                    mSettings[realid] = value;
                } else {
                    if (mSettings.Contains(realid) && mSettings.Get(realid, "") != "") {
                        wxString origName = mSettings[realid];
                        wxFileName fn(origName, origName[1] == ':' ? wxPATH_WIN : wxPATH_UNIX);
                        fn.SetPath(value);
                        wxString newName = fn.GetFullPath();
                        mSettings[realid] = newName;
                    }
                }
            } else {
                mSettings[id] = value;
            }
        }
    }
    IncrementChangeCount();
}

void Effect::CopySettingsMap(SettingsMap &target, bool stripPfx) const
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);

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

// When an effect is copied between model types the buffer may not be supported so make it valid
void Effect::FixBuffer(const Model* m)
{
    if (m == nullptr) return;

    auto styles = m->GetBufferStyles();
    auto style = mSettings.Get("B_CHOICE_BufferStyle", "Default");

    if (std::find(styles.begin(), styles.end(), style) == styles.end())
    {
        if (style.substr(0, 9) == "Per Model")
        {
            mSettings["B_CHOICE_BufferStyle"] = style.substr(10);
        }
        else
        {
            mSettings["B_CHOICE_BufferStyle"] = "Default";
        }
    }
}

std::string Effect::GetPaletteAsString() const
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    return mPaletteMap.AsString();
}

void Effect::SetPalette(const std::string& i)
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
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

// This only updates the colour palette ... preserving all the other colour settings
void Effect::SetColourOnlyPalette(const std::string& i)
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);

    // save the old palette
    auto oldPalette = mPaletteMap;

    // parse in the new one
    mPaletteMap.Parse(i);

    // copy over all the non colour entries
    for (auto it = oldPalette.begin(); it != oldPalette.end(); ++it)
    {
        wxString key(it->first);
        if (!key.StartsWith("C_BUTTON_Palette") && !key.StartsWith("C_CHECKBOX_Palette"))
        {
            mPaletteMap[it->first] = it->second;
        }
    }

    mColors.clear();
    mCC.clear();
    IncrementChangeCount();
    if (mPaletteMap.empty())
    {
        return;
    }
    ParseColorMap(mPaletteMap, mColors, mCC);
}

void Effect::CopyPalette(xlColorVector &target, xlColorCurveVector& newcc) const
{
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    target = mColors;
    newcc = mCC;
}

void Effect::PaletteMapUpdated() {
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    mColors.clear();
    mCC.clear();
    IncrementChangeCount();
    if (mPaletteMap.empty())
    {
        return;
    }
    ParseColorMap(mPaletteMap, mColors, mCC);
}

bool operator<(const Effect &e1, const Effect &e2)
{
    if (e1.GetStartTimeMS() < e2.GetStartTimeMS())
    {
        return true;
    }
    return false;
}

bool Effect::GetFrame(RenderBuffer &buffer, RenderCache &renderCache) {
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    if (mCache == nullptr) {
        mCache = renderCache.GetItem(this, &buffer);
    }
    return mCache && mCache->GetFrame(&buffer);
}

void Effect::AddFrame(RenderBuffer &buffer, RenderCache &renderCache) {
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    if (mCache) {
        mCache->AddFrame(&buffer);
    }
}

void Effect::PurgeCache(bool deleteCache) {
    std::unique_lock<std::recursive_mutex> lock(settingsLock);
    if (mCache) {
        if (!deleteCache) {
            mCache->PurgeFrames();
        }
        mCache->Delete();
        mCache = nullptr;
    }
}

#include "Effect.h"
#include "EffectLayer.h"
#include "../BitmapCache.h"

#include "../../include/globals.h"

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
    const void map(wxString &n) const
    {
        wxStringToStringHashMap::const_iterator it = data.find(n);
        if (it != data.end())
        {
            n = it->second;
        }
    }
private:
    wxStringToStringHashMap data;
} Remaps;

const wxString MapStringString::EMPTY_STRING;

void SettingsMap::RemapChangedSettingKey(wxString &n,  wxString &value)
{
    Remaps.map(n);
}

void AdjustSettingsToBeFitToTime(int effectIdx, SettingsMap &settings, int startMS, int endMS, xlColorVector &colors)
{
    int ftt = wxAtoi(settings.Get("T_CHECKBOX_FitToTime", "1"));
    switch (effectIdx)
    {
    //these effects have never used the FitToTime or speed settings, nothing to do
    case BitmapCache::eff_OFF:
    case BitmapCache::eff_GALAXY:
    case BitmapCache::eff_FAN:
    case BitmapCache::eff_MARQUEE:
    case BitmapCache::eff_MORPH:
    case BitmapCache::eff_SHOCKWAVE:
    case BitmapCache::eff_GLEDIATOR:
    case BitmapCache::eff_FACES:
        break;
    case BitmapCache::eff_STROBE:
    case BitmapCache::eff_TWINKLE:
        break;
    //these effects have been updated to have a dedicated repeat or speed or other control
    //and now ignore the FitToTime and Speed sliders, but the settings need adjusting
    case BitmapCache::eff_ON:
        if (settings.Get("E_TEXTCTRL_On_Cycles", "") == "")
        {
            float cycles = 1.0;
            if (!ftt)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                cycles = maxState / 200;
            }
            settings["E_TEXTCTRL_On_Cycles"] = wxString::Format("%0.2f", cycles);
        }
        break;
    case BitmapCache::eff_SNOWSTORM:
        if (settings.Get("E_SLIDER_Snowstorm_Speed", "") == "")
        {
            settings["E_SLIDER_Snowstorm_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_SNOWFLAKES:
        if (settings.Get("E_SLIDER_Snowflakes_Speed", "") == "")
        {
            settings["E_SLIDER_Snowflakes_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_BUTTERFLY:
        if (settings.Get("E_SLIDER_Butterfly_Speed", "") == "")
        {
            settings["E_SLIDER_Butterfly_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_CIRCLES:
        if (settings.Get("E_SLIDER_Circles_Speed", "") == "")
        {
            settings["E_SLIDER_Circles_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_LIFE:
        if (settings.Get("E_SLIDER_Life_Speed", "") == "")
        {
            settings["E_SLIDER_Life_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_METEORS:
        if (settings.Get("E_SLIDER_Meteors_Speed", "") == "")
        {
            settings["E_SLIDER_Meteors_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_TREE:
        if (settings.Get("E_SLIDER_Tree_Speed", "") == "")
        {
            settings["E_SLIDER_Tree_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_PINWHEEL:
        if (settings.Get("E_TEXTCTRL_Pinwheel_Speed", "") == "")
        {
            settings["E_TEXTCTRL_Pinwheel_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_PLASMA:
        if (settings.Get("E_TEXTCTRL_Plasma_Speed", "") == "")
        {
            settings["E_TEXTCTRL_Plasma_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_TEXT:
        if (settings.Get("E_TEXTCTRL_Text_Speed1", "") == "")
        {
            settings["E_TEXTCTRL_Text_Speed1"] = settings.Get("T_SLIDER_Speed", "10");
        }
        if (settings.Get("E_TEXTCTRL_Text_Speed2", "") == "")
        {
            settings["E_TEXTCTRL_Text_Speed2"] = settings.Get("T_SLIDER_Speed", "10");
        }
        if (settings.Get("E_TEXTCTRL_Text_Speed3", "") == "")
        {
            settings["E_TEXTCTRL_Text_Speed3"] = settings.Get("T_SLIDER_Speed", "10");
        }
        if (settings.Get("E_TEXTCTRL_Text_Speed4", "") == "")
        {
            settings["E_TEXTCTRL_Text_Speed4"] = settings.Get("T_SLIDER_Speed", "10");
        }

        if (settings.Get("E_SLIDER_Text_Position1", "") != "") {
            int pos = wxAtoi(settings.Get("E_SLIDER_Text_Position1", "50")) * 2 - 100;
            settings.erase("E_SLIDER_Text_Position1");
            settings["E_SLIDER_Text_YStart1"] = wxString::Format("%d", pos);
            settings["E_SLIDER_Text_XStart1"] = wxString::Format("%d", pos);
            settings["E_SLIDER_Text_XEnd1"] = wxString::Format("%d", pos);
            settings["E_SLIDER_Text_YEnd1"] = wxString::Format("%d", pos);
        }
        break;
    case BitmapCache::eff_WAVE:
        if (settings.Get("E_TEXTCTRL_Wave_Speed", "") == "")
        {
            settings["E_TEXTCTRL_Wave_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        break;
    case BitmapCache::eff_SPIROGRAPH:
        if (settings.Get("E_TEXTCTRL_Spirograph_Speed", "") == "")
        {
            settings["E_TEXTCTRL_Spirograph_Speed"] = settings.Get("T_SLIDER_Speed", "10");
        }
        if (settings.Get("E_CHECKBOX_Spirograph_Animate", "") != "")
        {
            int i = wxAtoi(settings.Get("E_CHECKBOX_Spirograph_Animate", "0"));
            settings["E_TEXTCTRL_Spirograph_Animate"] = (i == 0 ? "0" : "10");
            settings.erase("E_CHECKBOX_Spirograph_Animate");
        }
        break;

    case BitmapCache::eff_COLORWASH:
        if (settings.Get("E_TEXTCTRL_ColorWash_Cycles", "") == "")
        {
            double count = wxAtoi(settings.Get("E_SLIDER_ColorWash_Count", "1"));
            settings.erase("E_SLIDER_ColorWash_Count");
            if (settings["T_CHECKBOX_FitToTime"] == "1")
            {
                count = 1.0;
                settings["E_CHECKBOX_ColorWash_CircularPalette"] = "0";
            }
            else
            {
                settings["E_CHECKBOX_ColorWash_CircularPalette"] = "1";
            }
            settings["E_TEXTCTRL_ColorWash_Cycles"] = wxString::Format("%0.2f", count);
        }
        break;
    case BitmapCache::eff_FIRE:
        if (settings.Get("E_TEXTCTRL_Fire_GrowthCycles", "") == "")
        {
            bool grow = settings["E_CHECKBOX_Fire_GrowFire"] == "1";
            settings.erase("E_CHECKBOX_Fire_GrowFire");
            if (grow)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                double maxState = totalTime * speed / 50;
                double cycles = maxState / 500.0;
                settings["E_TEXTCTRL_Fire_GrowthCycles"] = wxString::Format("%0.2f", cycles);
            }
            else
            {
                settings["E_TEXTCTRL_Fire_GrowthCycles"] = "0.0";
            }
        }
        break;
    case BitmapCache::eff_FIREWORKS:
        if (settings.Get("E_SLIDER_Fireworks_Number_Explosions", "") != "")
        {
            int cnt = wxAtoi(settings.Get("E_SLIDER_Fireworks_Number_Explosions", "10"));
            settings.erase("E_SLIDER_Fireworks_Number_Explosions");
            int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
            int total = (speed * cnt) / 50;
            if (total > 50)
            {
                total = 50;
            }
            if (total < 1)
            {
                total = 1;
            }
            settings["E_SLIDER_Fireworks_Explosions"] = wxString::Format("%d", total);
        }
        break;
    case BitmapCache::eff_RIPPLE:
        if (settings.Get("E_TEXTCTRL_Ripple_Cycles", "") == "")
        {
            float cycles = 1.0;
            if (!ftt)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                cycles = maxState / 200;
            }
            settings["E_TEXTCTRL_Ripple_Cycles"] = wxString::Format("%0.2f", cycles);
        }
        break;
    case BitmapCache::eff_BARS:
        if (settings.Get("E_TEXTCTRL_Bars_Cycles", "") == "")
        {
            float cycles = 1.0;
            wxString dir = settings["E_CHOICE_Bars_Direction"];
            if (!ftt)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                if (dir.Contains("Altern"))
                {
                    cycles = maxState * 2;
                }
                else
                {
                    cycles = maxState / 200;
                }
            }
            settings["E_TEXTCTRL_Bars_Cycles"] = wxString::Format("%0.2f", cycles);
        }
        break;
    case BitmapCache::eff_SPIRALS:
        if (settings.Get("E_TEXTCTRL_Spirals_Movement", "") == "")
        {
            float cycles = 1.0;
            int dir = wxAtoi(settings.Get("E_SLIDER_Spirals_Direction", "1"));
            settings.erase("E_SLIDER_Spirals_Direction");
            if (!ftt)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                cycles = maxState / 600;
            }
            settings["E_TEXTCTRL_Spirals_Movement"] = wxString::Format("%0.2f", dir * cycles);
        }
        break;
    case BitmapCache::eff_CURTAIN:
        if (settings.Get("E_TEXTCTRL_Curtain_Speed", "") == "")
        {
            float cycles = 1.0;
            if (!ftt)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                cycles = maxState / 200;
            }
            settings["E_TEXTCTRL_Curtain_Speed"] = wxString::Format("%0.2f", cycles);
        }
        break;
    case BitmapCache::eff_SINGLESTRAND:
        if ("Skips" == settings["E_NOTEBOOK_SSEFFECT_TYPE"])
        {
            if (settings.Get("E_SLIDER_Skips_Advance", "") == "")
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                settings["E_SLIDER_Skips_Advance"] = wxString::Format("%d", speed - 1);
            }
        }
        else
        {
            wxString type = settings.Get("E_CHOICE_Chase_Type1", "Left-Right");
            if (type == "Auto reverse")
            {
                type = "Bounce from Left";
                settings["E_CHOICE_Chase_Type1"] = type;
            }
            else if (type == "Bounce" || type == "Pacman")
            {
                type = "Dual Bounce";
                settings["E_CHOICE_Chase_Type1"] = type;
            }
            if (settings.Get("E_TEXTCTRL_Chase_Rotations", "") == "")
            {
                float cycles = 1.0;
                if (!ftt)
                {
                    int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                    int totalTime = endMS - startMS;
                    int maxState = totalTime * speed / 50;
                    cycles = maxState / 250.0;
                }
                settings["E_TEXTCTRL_Chase_Rotations"] = wxString::Format("%0.2f", cycles);
            }
        }
        break;
    case BitmapCache::eff_SHIMMER:
        if (settings.Get("E_TEXTCTRL_Shimmer_Cycles", "") == "")
        {
            float cycles = 1.0;
            int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
            int totalTime = endMS - startMS;
            int maxState = totalTime * speed / 50;
            cycles = maxState / (100.0 * colors.size());
            settings["E_TEXTCTRL_Shimmer_Cycles"] = wxString::Format("%0.2f", cycles);
        }
        break;
    case BitmapCache::eff_PICTURES:
        if (settings.Get("E_TEXTCTRL_Pictures_FrameRateAdj", "") == "")
        {
            if (settings.Get("E_CHECKBOX_MovieIs20FPS", "") == "1")
            {
                settings["E_TEXTCTRL_Pictures_FrameRateAdj"] = "1.0";
            }
            else if (settings.Get("E_SLIDER_Pictures_GifSpeed", "") == "0")
            {
                settings["E_TEXTCTRL_Pictures_FrameRateAdj"] = "0.0";
            }
            else if (!ftt)
            {
                int speed = wxAtoi(settings.Get("T_SLIDER_Speed", "10"));
                int totalTime = endMS - startMS;
                int maxState = totalTime * speed / 50;
                double cycles = maxState / 300.0;
                settings["E_TEXTCTRL_Pictures_Speed"] = wxString::Format("%0.2f", cycles);
            }

            settings.erase("E_CHECKBOX_MovieIs20FPS");
            settings.erase("E_SLIDER_Pictures_GifSpeed");
        }
        break;
    case BitmapCache::eff_GARLANDS:
        //Don't attempt to map the Garlands speed settings.  In v3, the Garland speed depended on the Speed setting, the
        //Spacing setting as well as the height of the model.  We don't have the height of the model here so really
        //no way to figure out the speed or an appropriate mapping
        break;

    //these all need code updated and new sliders and such before we can map them
    //these all have state/speed requirements
    case BitmapCache::eff_PIANO:
        break;
    }
    settings.erase("T_CHECKBOX_FitToTime");
    settings.erase("T_SLIDER_Speed");
}

Effect::Effect(EffectLayer* parent,int id, int effectIndex, const wxString & name, const wxString &settings, const wxString &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected)
    : mParentLayer(parent), mID(id), mEffectIndex(effectIndex), mName(nullptr),
      mStartTime(startTimeMS), mEndTime(endTimeMS), mSelected(Selected), mProtected(Protected)
{
    int i = GetEffectIndex(name);
    mSettings.Parse(settings);

    if (mEndTime < mStartTime)
    {
        //should never happend, but if we load something with invalid times, make sure we can at least
        //show/select/delete the effect
        int tmp = mStartTime;
        mStartTime = mEndTime;
        mEndTime = tmp;
    }

    if (effectIndex == BitmapCache::eff_FACES
            && mSettings.Get("E_CHOICE_Faces_FaceDefinition", "") == ""
            && mSettings.Get("E_CHOICE_Faces_TimingTrack", "") == "")
    {
        mSettings["E_CHOICE_Faces_FaceDefinition"] = XLIGHTS_PGOFACES_FILE;
    }
    if (i == -1)
    {
        mName = new wxString(name);
    }

    mPaletteMap.Parse(palette);
    mColors.clear();
    if (!mPaletteMap.empty())
    {
        for (int i = 1; i <= 6; i++)
        {
            if (mPaletteMap[wxString::Format("C_CHECKBOX_Palette%d",i)] ==  "1")
            {
                mColors.push_back(xlColor(mPaletteMap[wxString::Format("C_BUTTON_Palette%d",i)]));
            }
        }
    }
    AdjustSettingsToBeFitToTime(mEffectIndex, mSettings, mStartTime, mEndTime, mColors);
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

    for (std::map<wxString,wxString>::const_iterator it=mSettings.begin(); it!=mSettings.end(); ++it)
    {
        wxString name = it->first;
        if (stripPfx && name[1] == '_')
        {
            name = name.AfterFirst('_');
        }
        target[name] = it->second;
    }
}
void Effect::CopyPaletteMap(SettingsMap &target, bool stripPfx) const
{
    wxMutexLocker lock(settingsLock);
    for (std::map<wxString,wxString>::const_iterator it=mPaletteMap.begin(); it!=mPaletteMap.end(); ++it)
    {
        wxString name = it->first;
        if (stripPfx && name[1] == '_')
        {
            name = name.AfterFirst('_');
        }
        target[name] = it->second;
    }
}

void Effect::SetSettings(const wxString &settings)
{
    wxMutexLocker lock(settingsLock);
    mSettings.Parse(settings);
    IncrementChangeCount();
}

wxString Effect::GetSettingsAsString() const
{
    wxMutexLocker lock(settingsLock);
    return mSettings.AsString();
}

void Effect::SetPalette(const wxString& i)
{
    wxMutexLocker lock(settingsLock);
    mPaletteMap.Parse(i);
    mColors.clear();
    IncrementChangeCount();
    if (mPaletteMap.empty())
    {
        return;
    }
    for (int i = 1; i <= 6; i++)
    {
        if (mPaletteMap[wxString::Format("C_CHECKBOX_Palette%d",i)] ==  "1")
        {
            mColors.push_back(xlColor(mPaletteMap[wxString::Format("C_BUTTON_Palette%d",i)]));
        }
    }
}

wxString Effect::GetPaletteAsString() const
{
    wxMutexLocker lock(settingsLock);
    return mPaletteMap.AsString();
}

const wxString &Effect::GetEffectName() const
{
    if (mName != nullptr)
    {
        return *mName;
    }
    return GetEffectName(mEffectIndex);
}

void Effect::SetEffectName(const wxString & name)
{
    int idx = GetEffectIndex(name);
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
            mName = new wxString(name);
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

bool operator<(const Effect &e1, const Effect &e2)
{
    if(e1.GetStartTimeMS() < e2.GetStartTimeMS())
        return true;
    else
        return false;
}

class EffectMap: public std::vector<wxString>
{
public:
    EffectMap()
    {
        resize(BitmapCache::eff_LASTEFFECT);
        at(BitmapCache::eff_OFF) = "Off";
        at(BitmapCache::eff_ON) = "On";
        at(BitmapCache::eff_BARS) = "Bars";
        at(BitmapCache::eff_BUTTERFLY) = "Butterfly";
        at(BitmapCache::eff_CIRCLES) = "Circles";
        at(BitmapCache::eff_COLORWASH) = "Color Wash";
        at(BitmapCache::eff_CURTAIN) = "Curtain";
        at(BitmapCache::eff_DMX) = "DMX";
        at(BitmapCache::eff_FACES) = "Faces";
        at(BitmapCache::eff_FAN) = "Fan";
        at(BitmapCache::eff_FIRE) = "Fire";
        at(BitmapCache::eff_FIREWORKS) = "Fireworks";
        at(BitmapCache::eff_GALAXY) = "Galaxy";
        at(BitmapCache::eff_GARLANDS) = "Garlands";
        at(BitmapCache::eff_GLEDIATOR) = "Glediator";
        at(BitmapCache::eff_LIFE) = "Life";
        at(BitmapCache::eff_LIGHTNING) = "Lightning";
        at(BitmapCache::eff_MARQUEE) = "Marquee";
        at(BitmapCache::eff_METEORS) = "Meteors";
        at(BitmapCache::eff_MORPH) = "Morph";
        at(BitmapCache::eff_PIANO) = "Piano";
        at(BitmapCache::eff_PICTURES) = "Pictures";
        at(BitmapCache::eff_PINWHEEL) = "Pinwheel";
        at(BitmapCache::eff_PLASMA) = "Plasma";
        at(BitmapCache::eff_RIPPLE) = "Ripple";
        at(BitmapCache::eff_SHIMMER) = "Shimmer";
        at(BitmapCache::eff_SHOCKWAVE) = "Shockwave";
        at(BitmapCache::eff_SINGLESTRAND) = "SingleStrand";
        at(BitmapCache::eff_SNOWFLAKES) = "Snowflakes";
        at(BitmapCache::eff_SNOWSTORM) = "Snowstorm";
        at(BitmapCache::eff_SPIRALS) = "Spirals";
        at(BitmapCache::eff_SPIROGRAPH) = "Spirograph";
        at(BitmapCache::eff_STROBE) = "Strobe";
        at(BitmapCache::eff_TEXT) = "Text";
        at(BitmapCache::eff_TREE) = "Tree";
        at(BitmapCache::eff_TWINKLE) = "Twinkle";
        at(BitmapCache::eff_WAVE) = "Wave";
    }

} effectMap;

const wxString &Effect::GetEffectName(int idx)
{
    return effectMap[idx];
}

int Effect::GetEffectIndex(const wxString &effectName)
{
    for (int x = 0; x < effectMap.size(); x++)
    {
        if (effectMap[x] == effectName)
        {
            return x;
        }
    }
    if (effectName == "CoroFaces")
    {
        //old deprecated/removed effect.  Mapped to FACES
        return BitmapCache::eff_FACES;
    }
    return -1;
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
    mParentLayer->IncrementChangeCount(GetStartTimeMS(), GetEndTimeMS());
}



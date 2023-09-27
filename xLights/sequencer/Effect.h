#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "wx/wx.h"

#include <vector>
#include <string>
#include <mutex>

#include "../ColorCurve.h" // This needs to be here
#include "../UtilClasses.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "../Color.h"

class EffectLayer;
class ValueCurve;
class RenderCacheItem;
class RenderBuffer;
class RenderCache;
class Model;
class RenderableEffect;
class xLightsFrame;
class EffectManager;

#define EFFECT_NOT_SELECTED     0
#define EFFECT_LT_SELECTED      1
#define EFFECT_RT_SELECTED      2
#define EFFECT_SELECTED         3

wxDECLARE_EVENT(EVT_SETTIMINGTRACKS, wxCommandEvent);

// An effect represents a generic effect
class Effect
{
    int mID = 0;
    short mEffectIndex = -1;
    std::string *mName = nullptr;
    int mStartTime = 0;
    int mEndTime = 0;
    int mSelected = 0;
    bool mTagged = false;
    bool mProtected = false;
    EffectLayer* mParentLayer = nullptr;
    xlColor mColorMask = xlBLACK;
    mutable std::recursive_mutex settingsLock;
    SettingsMap mSettings;
    SettingsMap mPaletteMap;
    xlColorVector mColors;
    xlColorCurveVector mCC;
    xlDisplayList background;
    RenderCacheItem *mCache = nullptr;
    wxLongLong _timeToDelete = 0;

    Effect() {}  //don't allow default or copy constructor
    static void ParseColorMap(const SettingsMap &mPaletteMap, xlColorVector &mColors, xlColorCurveVector& mCC);

public:
    Effect(EffectManager* effectManager, EffectLayer* parent, int id, const std::string & name, const std::string &settings, const std::string &palette,
        int startTimeMS, int endTimeMS, int Selected, bool Protected, bool importing = false);
    Effect(const Effect& e);
    virtual ~Effect();

    int GetID() const { return mID; }
    void SetID(int i) { mID = i; }

    bool IsTimeToDelete() const;
    void SetTimeToDelete();

    int GetEffectIndex() const { return mEffectIndex; }
    void SetEffectIndex(int effectIndex);

    const std::string &GetEffectName() const;
    const std::string &GetEffectName(int index) const;
    void SetEffectName(const std::string & name);

    wxString GetDescription() const;
    std::string GetSetting(const std::string& id) const;

    int GetStartTimeMS() const { return mStartTime; }
    void SetStartTimeMS(int startTimeMS);
    int GetEndTimeMS() const { return mEndTime; }
    void SetEndTimeMS(int endTimeMS);
    bool OverlapsWith(int startTimeMS, int EndTimeMS) const;

    void ConvertTo(int effectIndex);

    int GetSelected() const { return mSelected; }
    void SetSelected(int selected) { mSelected = selected; }

    // used to tag effects to aid in making sure they don't get processed twice for some actions
    bool GetTagged() const { return mTagged; }
    void SetTagged(bool tagged) { mTagged = tagged; }

    bool GetProtected() const { return mProtected; }
    void SetProtected(bool Protected) { mProtected = Protected; }

    bool IsModelRenderDisabled() const;
    bool IsEffectRenderDisabled() const;
    bool IsRenderDisabled() const;
    void SetEffectRenderDisabled(bool disabled);

    bool IsLocked() const;
    void SetLocked(bool lock);

    EffectLayer* GetParentEffectLayer() const { return mParentLayer; }
    void SetParentEffectLayer(EffectLayer* parent) { mParentLayer = parent; }

    void IncrementChangeCount();

    std::string GetSettingsAsString() const;
    std::string GetSettingsAsJSON() const;
    void SetSettings(const std::string &settings, bool keepxsettings, bool json = false);
    bool SettingsChanged(const std::string& settings);
    void ApplySetting(const std::string& id, const std::string& value, ValueCurve* vc, const std::string& vcid);
    bool UsesColour(const std::string& from);
    int ReplaceColours(xLightsFrame* frame, const std::string& from, const std::string& to);
    void PressButton(RenderableEffect* re, const std::string& id);
    const SettingsMap &GetSettings() const { return mSettings; }
    void CopySettingsMap(SettingsMap &target, bool stripPfx = false) const;
    void FixBuffer(const Model* m);
    bool IsPersistent() const;

    const xlColorVector &GetPalette() const { return mColors; }
    int GetPaletteSize() const { return mColors.size(); }
    const SettingsMap &GetPaletteMap() const { return mPaletteMap; }
    std::string GetPaletteAsString() const;
    std::string GetPaletteAsJSON() const;
    void SetPalette(const std::string& i);
    void SetColourOnlyPalette(const std::string& i, bool json = false);
    void CopyPalette(xlColorVector &target, xlColorCurveVector& newcc) const;
    void EraseSettingsStartingWith(const std::string& s);
    void ErasePalette();
    void EraseColourSettings();

    /* Do NOT call these on any thread other than the main thread */
    SettingsMap &GetSettings() { return mSettings; }
    xlColorVector &GetPalette() { return mColors; }
    SettingsMap &GetPaletteMap() { return mPaletteMap; }
    void PaletteMapUpdated();

    xlDisplayList &GetBackgroundDisplayList() { return background; }
    const xlDisplayList &GetBackgroundDisplayList() const { return background; }
    bool HasBackgroundDisplayList() const {
        std::lock_guard<std::recursive_mutex> lock(background.lock);
        return !background.empty();
    }

    xlColor* GetColorMask() {
        if (mColorMask.IsNilColor())
        {
            return nullptr;
        }
        return &mColorMask;
    }
    void SetColorMask(xlColor colorMask) { mColorMask = colorMask; }

    //gets the cached frame.   Returns true if the frame was filled into the buffer
    bool GetFrame(RenderBuffer &buffer, RenderCache &renderCache);
    void AddFrame(RenderBuffer &buffer, RenderCache &renderCache);
    void PurgeCache(bool deleteCachefile = false);
};

bool operator<(const Effect &e1, const Effect &e2);

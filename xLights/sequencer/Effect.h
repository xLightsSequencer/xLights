#ifndef EFFECT_H
#define EFFECT_H

#include "wx/wx.h"

#include <vector>
#include <string>
#include <mutex>

#include "ColorCurve.h"
#include "../UtilClasses.h"
#include "../DrawGLUtils.h"
#include "../Color.h"

class EffectLayer;
class ValueCurve;
class RenderCacheItem;
class RenderBuffer;
class RenderCache;

#define EFFECT_NOT_SELECTED     0
#define EFFECT_LT_SELECTED      1
#define EFFECT_RT_SELECTED      2
#define EFFECT_SELECTED         3

wxDECLARE_EVENT(EVT_SETTIMINGTRACKS, wxCommandEvent);

// An effect represents a generic effect
class Effect
{
    int mID;
    short mEffectIndex;
    std::string *mName;
    int mStartTime;
    int mEndTime;
    int mSelected;
    bool mTagged;
    bool mProtected;
    EffectLayer* mParentLayer;
    xlColor mColorMask;
    mutable std::recursive_mutex settingsLock;
    SettingsMap mSettings;
    SettingsMap mPaletteMap;
    xlColorVector mColors;
    xlColorCurveVector mCC;
    DrawGLUtils::xlDisplayList background;
    RenderCacheItem *mCache;

    Effect() {}  //don't allow default or copy constructor
    Effect(const Effect &e) {}
    static void ParseColorMap(const SettingsMap &mPaletteMap, xlColorVector &mColors, xlColorCurveVector& mCC);

public:
    Effect(EffectLayer* parent, int id, const std::string & name, const std::string &settings, const std::string &palette,
        int startTimeMS, int endTimeMS, int Selected, bool Protected);
    virtual ~Effect();

    int GetID() const { return mID; }
    void SetID(int i) { mID = i; }

    int GetEffectIndex() const { return mEffectIndex; }
    void SetEffectIndex(int effectIndex);

    const std::string &GetEffectName() const;
    const std::string &GetEffectName(int index) const;
    void SetEffectName(const std::string & name);

    wxString GetDescription() const;

    int GetStartTimeMS() const { return mStartTime; }
    void SetStartTimeMS(int startTimeMS);
    int GetEndTimeMS() const { return mEndTime; }
    void SetEndTimeMS(int endTimeMS);
    bool OverlapsWith(int startTimeMS, int EndTimeMS);

    int GetSelected() const { return mSelected; }
    void SetSelected(int selected) { mSelected = selected; }

    // used to tag effects to aid in making sure they don't get processed twice for some actions
    bool GetTagged() const { return mTagged; }
    void SetTagged(bool tagged) { mTagged = tagged; }

    bool GetProtected() const { return mProtected; }
    void SetProtected(bool Protected) { mProtected = Protected; }

    bool IsLocked() const;
    void SetLocked(bool lock);

    EffectLayer* GetParentEffectLayer() const { return mParentLayer; }
    void SetParentEffectLayer(EffectLayer* parent) { mParentLayer = parent; }

    void IncrementChangeCount();

    std::string GetSettingsAsString() const;
    void SetSettings(const std::string &settings, bool keepxsettings);
    void ApplySetting(const std::string& id, const std::string& value, ValueCurve* vc, const std::string& vcid);
    const SettingsMap &GetSettings() const { return mSettings; }
    void CopySettingsMap(SettingsMap &target, bool stripPfx = false) const;

    const xlColorVector &GetPalette() const { return mColors; }
    int GetPaletteSize() const { return mColors.size(); }
    const SettingsMap &GetPaletteMap() const { return mPaletteMap; }
    std::string GetPaletteAsString() const;
    void SetPalette(const std::string& i);
    void SetColourOnlyPalette(const std::string & i);
    void CopyPalette(xlColorVector &target, xlColorCurveVector& newcc) const;

    /* Do NOT call these on any thread other than the main thread */
    SettingsMap &GetSettings() { return mSettings; }
    xlColorVector &GetPalette() { return mColors; }
    SettingsMap &GetPaletteMap() { return mPaletteMap; }
    void PaletteMapUpdated();

    DrawGLUtils::xlDisplayList &GetBackgroundDisplayList() { return background; }
    const DrawGLUtils::xlDisplayList &GetBackgroundDisplayList() const { return background; }
    bool HasBackgroundDisplayList() const {
        std::lock_guard<std::recursive_mutex>(background.lock);
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

#endif // EFFECT_H

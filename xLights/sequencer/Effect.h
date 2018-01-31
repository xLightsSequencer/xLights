#ifndef EFFECT_H
#define EFFECT_H

#include "wx/wx.h"
#include <vector>
#include "../UtilClasses.h"
#include "../Color.h"
#include "../DrawGLUtils.h"
#include <string>
#include <mutex>

class EffectLayer;

#define EFFECT_NOT_SELECTED     0
#define EFFECT_LT_SELECTED      1
#define EFFECT_RT_SELECTED      2
#define EFFECT_SELECTED         3

wxDECLARE_EVENT(EVT_SETTIMINGTRACKS, wxCommandEvent);

class ValueCurve;

class Effect
{
    private:
        Effect();  //don't allow default or copy constructor
        Effect(const Effect &e);
    public:
    Effect(EffectLayer* parent, int id, const std::string & name, const std::string &settings, const std::string &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected);
        virtual ~Effect();

        int GetID() const;
        void SetID(int i);

        int GetEffectIndex() const;
        void SetEffectIndex(int effectIndex);

        const std::string &GetEffectName() const;
        void SetEffectName(const std::string & name);

        wxString GetDescription() const;
        int GetStartTimeMS() const;
        void SetStartTimeMS(int startTimeMS);
        bool OverlapsWith(int startTimeMS, int EndTimeMS);

        int GetEndTimeMS() const;
        void SetEndTimeMS(int endTimeMS);

        int GetSelected() const;
        void SetSelected(int selected);

        bool GetTagged() const;
        void SetTagged(bool tagged);  // used to tag effects to aid in making sure they don't get processed twice for some actions

        bool GetProtected() const;
        void SetProtected(bool Protected);
        bool IsLocked() const;
        void SetLocked(bool lock);

        EffectLayer* GetParentEffectLayer() const;
        void SetParentEffectLayer(EffectLayer* parent);

        void IncrementChangeCount();

        std::string GetSettingsAsString() const;
        void SetSettings(const std::string &settings, bool keepxsettings);
        void ApplySetting(const std::string& id, const std::string& value, ValueCurve* vc, const std::string& vcid);

        const SettingsMap &GetSettings() const { return mSettings;}
        const xlColorVector &GetPalette() const { return mColors;}
        int GetPaletteSize() const { return mColors.size(); }
        const SettingsMap &GetPaletteMap() const { return mPaletteMap;}

        /* Do NOT call these on any thread other than the main thread */
        SettingsMap &GetSettings() { return mSettings;}
        xlColorVector &GetPalette() { return mColors;}
        SettingsMap &GetPaletteMap() { return mPaletteMap;}
        void PaletteMapUpdated();

        void CopySettingsMap(SettingsMap &target, bool stripPfx = false) const;
        void CopyPalette(xlColorVector &target, xlColorCurveVector& newcc) const;

        std::string GetPaletteAsString() const;
        void SetPalette(const std::string& i);

        DrawGLUtils::xlDisplayList &GetBackgroundDisplayList() {
            return background;
        }
        const DrawGLUtils::xlDisplayList &GetBackgroundDisplayList() const {
            return background;
        }
        xlColor* GetColorMask() { 
            if (mColorMask.IsNilColor())
            {
                return nullptr;
            }
            else
            {
                return &mColorMask;
            }
        }
        void SetColorMask(xlColor colorMask)
        {
            mColorMask = colorMask;
        }
        bool HasBackgroundDisplayList() const {
            std::lock_guard<std::recursive_mutex> (background.lock);
            return !background.empty();
        }
    protected:
    private:
        int mStartTime;
        int mEndTime;
        int mSelected;
        bool mTagged;
        int mID;
        std::string *mName;
        short mEffectIndex;
        bool mProtected;
        EffectLayer* mParentLayer;
        xlColor mColorMask;

        mutable std::mutex settingsLock;
        SettingsMap mSettings;
        SettingsMap mPaletteMap;
        xlColorVector mColors;
        xlColorCurveVector mCC;

        DrawGLUtils::xlDisplayList background;
};
bool operator<(const Effect &e1, const Effect &e2);




#endif // EFFECT_H

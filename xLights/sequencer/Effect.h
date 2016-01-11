#ifndef EFFECT_H
#define EFFECT_H

#include "wx/wx.h"
#include <wx/thread.h>
#include <vector>
#include "../UtilClasses.h"
#include "../Color.h"
#include "../DrawGLUtils.h"
#include <string>

class EffectLayer;

#define EFFECT_NOT_SELECTED     0
#define EFFECT_LT_SELECTED      1
#define EFFECT_RT_SELECTED      2
#define EFFECT_SELECTED         3

class Effect
{
    private:
        Effect();  //don't allow default or copy constructor
        Effect(const Effect &e);
    public:
    Effect(EffectLayer* parent, int id, const wxString & name, const wxString &settings, const wxString &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected);
        virtual ~Effect();

        int GetID() const;
        void SetID(int i);

        int GetEffectIndex() const;
        void SetEffectIndex(int effectIndex);

        const std::string &GetEffectName() const;
        void SetEffectName(const std::string & name);

        int GetStartTimeMS() const;
        void SetStartTimeMS(int startTimeMS);

        int GetEndTimeMS() const;
        void SetEndTimeMS(int endTimeMS);

        int GetSelected() const;
        void SetSelected(int selected);

        bool GetProtected() const;
        void SetProtected(bool Protected);

        EffectLayer* GetParentEffectLayer() const;
        void SetParentEffectLayer(EffectLayer* parent);

        void IncrementChangeCount();

        wxString GetSettingsAsString() const;
        void SetSettings(const wxString &settings);

        const SettingsMap &GetSettings() const { return mSettings;}
        const xlColorVector &GetPalette() const { return mColors;}
        const SettingsMap &GetPaletteMap() const { return mPaletteMap;}

        /* Do NOT call these on any thread other than the main thread */
        SettingsMap &GetSettings() { return mSettings;}
        xlColorVector &GetPalette() { return mColors;}
        SettingsMap &GetPaletteMap() { return mPaletteMap;}

        void CopySettingsMap(SettingsMap &target, bool stripPfx = false) const;
        void CopyPaletteMap(SettingsMap &target, bool stripPfx = false) const;


        wxString GetPaletteAsString() const;
        void SetPalette(const wxString& i);

        DrawGLUtils::xlDisplayList &GetBackgroundDisplayList() {
            return background;
        }
        const DrawGLUtils::xlDisplayList &GetBackgroundDisplayList() const {
            return background;
        }
        bool HasBackgroundDisplayList() const {
            wxMutexLocker (background.lock);
            return !background.empty();
        }
    protected:
    private:
        int mStartTime;
        int mEndTime;
        int mSelected;
        int mID;
        std::string *mName;
        short mEffectIndex;
        bool mProtected;
        EffectLayer* mParentLayer;

        mutable wxMutex settingsLock;
        SettingsMap mSettings;
        SettingsMap mPaletteMap;
        xlColorVector mColors;

        DrawGLUtils::xlDisplayList background;
};
bool operator<(const Effect &e1, const Effect &e2);




#endif // EFFECT_H

#ifndef EFFECT_H
#define EFFECT_H

#include "wx/wx.h"
#include <wx/thread.h>
#include <vector>
#include "../UtilClasses.h"
#include "../Color.h"
#include "../DrawGLUtils.h"

#define EFFECT_BARS         0
#define EFFECT_BUTTERFLY    1
#define EFFECT_CIRCLES      2
#define EFFECT_COLORWASH    3
#define EFFECT_CURTAIN      4
#define EFFECT_FIRE         5
#define EFFECT_FIREWORKS    6
#define EFFECT_GARLANDS     7
#define EFFECT_GLEDIATOR    8
#define EFFECT_LIFE         9
#define EFFECT_METEORS      10
#define EFFECT_PINWHEEL     11
#define EFFECT_RIPPLE       12
#define EFFECT_SINGLESTRAND 13
#define EFFECT_SNOWFLAKES   14
#define EFFECT_SNOWSTORM    15
#define EFFECT_SPIRALS      16
#define EFFECT_SPIROGRAPH   17
#define EFFECT_TREE         18
#define EFFECT_TWINKLE      19
#define EFFECT_WAVE         20
#define NUMBER_OF_EFFECTS   21

class EffectLayer;

class Effect
{
    private:
        Effect();  //don't allow default or copy constructor
        Effect(const Effect &e);
    public:
        Effect(EffectLayer* parent, int id, int effectIndex, const wxString & name, const wxString &settings, const wxString &palette,
               int startTimeMS, int endTimeMS, int Selected, bool Protected);
        virtual ~Effect();

        int GetID() const;
        void SetID(int i);

        int GetEffectIndex() const;
        void SetEffectIndex(int effectIndex);

        const wxString &GetEffectName() const;
        void SetEffectName(const wxString & name);

        int GetStartTimeMS() const;
        void SetStartTimeMS(int startTimeMS);

        int GetEndTimeMS() const;
        void SetEndTimeMS(int endTimeMS);

        int GetSelected();
        void SetSelected(int selected);

        bool GetProtected();
        void SetProtected(bool Protected);

        int GetStartPosition();
        void SetStartPosition(int position);

        int GetEndPosition();
        void SetEndPosition(int position);

        static int GetEffectIndex(const wxString &effectName);
        static const wxString &GetEffectName(int idx);

        EffectLayer* GetParentEffectLayer();
        void SetParentEffectLayer(EffectLayer* parent);

        void IncrementChangeCount();

        wxString GetSettingsAsString() const;
        void SetSettings(const wxString &settings);

        /* Do NOT call these on any thread other than the main thread */
        const SettingsMap &GetSettings() const { return mSettings;}
        const xlColorVector GetPalette() const { return mColors;}
        const SettingsMap &GetPaletteMap() const { return mPaletteMap;}

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
        wxString *mName;
        short mEffectIndex;
        bool mProtected;
        int mStartPosition;
        int mEndPosition;
        EffectLayer* mParentLayer;

        mutable wxMutex settingsLock;
        SettingsMap mSettings;
        SettingsMap mPaletteMap;
        xlColorVector mColors;

        DrawGLUtils::xlDisplayList background;
};
bool operator<(const Effect &e1, const Effect &e2);




#endif // EFFECT_H

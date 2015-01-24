#ifndef EFFECT_H
#define EFFECT_H

#include "wx/wx.h"
#include <vector>

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
    public:
        Effect(EffectLayer* parent);
        virtual ~Effect();

        int GetID();
        void SetID(int id);

        int GetEffectIndex();
        void SetEffectIndex(int effectIndex);

        wxString GetEffectName();
        void SetEffectName(wxString name);

        double GetStartTime() const;
        void SetStartTime(double startTime);

        double GetEndTime();
        void SetEndTime(double endTime);

        wxString GetSettings();
        void SetSettings(wxString settings);

        int GetSelected();
        void SetSelected(int selected);

        bool GetProtected();
        void SetProtected(bool Protected);

        bool IsDirty();
        void SetDirty(bool dirty);

        int GetStartPosition();
        void SetStartPosition(int position);

        int GetEndPosition();
        void SetEndPosition(int position);

        static int GetEffectIndex(wxString effectName);

        EffectLayer* GetParentEffectLayer();
        void SetParentEffectLayer(EffectLayer* parent);

        void IncrementChangeCount();

        volatile int changeCount;

    protected:
    private:
        int mID;
        int mEffectIndex;
        wxString mName;
        wxString mSettings;
        double mStartTime;
        double mEndTime;
        int mSelected;
        bool mProtected;
        int mStartPosition;
        int mEndPosition;
        bool mDirty;
        EffectLayer* mParentLayer;
};

bool operator<(const Effect &e1, const Effect &e2);


#endif // EFFECT_H

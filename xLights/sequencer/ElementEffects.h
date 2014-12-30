#ifndef ELEMENTEFFECTS_H
#define ELEMENTEFFECTS_H

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

#define NO_MIN_MAX_TIME      0

struct Effect_Struct
{
    int ID;
    int EffectIndex;
    wxString Effect;
    double StartTime;
    double EndTime;
    bool Selected;
    bool Protected;
};

class ElementEffects
{
    public:
    wxString ElementName;

    void AddEffect(int id,wxString effect,int effectIndex,double startTime,double endTime, bool Protected);
    bool IsStartTimeLinked(int index);
    bool IsEndTimeLinked(int index);

    int GetMaximumEndTime(int index);
    int GetMinimumStartTime(int index);
    static int GetEffectIndex(wxString effectName);
    static wxString GetEffectNameFromEffectText(wxString effectText);
    std::vector<Effect_Struct> Effects;

    ElementEffects();
    virtual ~ElementEffects();

    void Sort();
    protected:
    private:
        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);

        static bool SortByTime(const Effect_Struct &effect1,const Effect_Struct &effect2)
        {
            return (effect1.StartTime<effect2.StartTime);
        }

};

#endif // ELEMENTEFFECTS_H

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

#include <wx/wx.h>
#include <vector>
#include "Effect.h"

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

#define HIT_TEST_EFFECT_LT      0
#define HIT_TEST_EFFECT_RT      1
#define HIT_TEST_EFFECT_CTR     2

#define EFFECT_NOT_SELECTED     0
#define EFFECT_LT_SELECTED      1
#define EFFECT_RT_SELECTED      2
#define EFFECT_SELECTED         3


struct Effect_Struct
{
    int ID;
    int EffectIndex;
    wxString Effect;
    double StartTime;
    double EndTime;
    int Selected;
    bool Protected;
    int StartPosition;
    int EndPosition;
};

class ElementEffects
{
    public:

    ElementEffects();
    virtual ~ElementEffects();

    void AddEffect(int id,wxString commonSettings,wxString layer1Settings,wxString layer2Settings,int effectIndex,double startTime,double endTime, bool Protected);
    bool IsStartTimeLinked(int index);
    bool IsEndTimeLinked(int index);

    int GetMaximumEndTime(int index);
    int GetMinimumStartTime(int index);

    int GetEffectCount();
    Effect* GetEffect(int index);

    static int GetEffectIndex(wxString effectName);
    static wxString GetEffectNameFromEffectText(wxString effectText);
    static double RoundToMultipleOfPeriod(double number,double period);

    bool IsEffectStartTimeInRange(int index, float startTime,float endTime);
    bool IsEffectEndTimeInRange(int index, float startTime,float endTime);

    bool HitTestEffect(int position,int &index, int &result);
    void SelectEffectsInPositionRange(int startX,int endX,int &FirstSelected);
    void UnSelectAllEffects();

    void Sort();
    protected:
    private:
        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);

        std::vector<Effect> mEffects;

};

#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H
#include "wx/wx.h"
#include "Effect.h"

#define EFFECT_NOT_SELECTED     0
#define EFFECT_LT_SELECTED      1
#define EFFECT_RT_SELECTED      2
#define EFFECT_SELECTED         3

#define NO_MIN_MAX_TIME      0

#define HIT_TEST_EFFECT_LT      0
#define HIT_TEST_EFFECT_RT      1
#define HIT_TEST_EFFECT_CTR     2

class Element;

class EffectLayer
{
    public:
        EffectLayer(Element* parent);
        virtual ~EffectLayer();

        void AddEffect(int id, int effectIndex, wxString name, wxString settings,double startTime,double endTime, bool Protected);

        Effect* GetEffect(int index);
        Effect* RemoveEffect(int index);

        int GetIndex();
        int GetEffectCount();
        void SetIndex(int index);
        void SortEffects();

        bool IsStartTimeLinked(int index);
        bool IsEndTimeLinked(int index);
        bool IsEffectStartTimeInRange(int index, float startTime,float endTime);
        bool IsEffectEndTimeInRange(int index, float startTime,float endTime);

        float GetMaximumEndTime(int index);
        float GetMinimumStartTime(int index);

        bool HitTestEffect(int position,int &index, int &result);
        int GetEffectIndexThatContainsPosition(int position);
        void SelectEffectsInPositionRange(int startX,int endX,int &FirstSelected);
        void UnSelectAllEffects();

        Element* GetParentElement();
        void SetParentElement(Element* parent);


    protected:
    private:

        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);

        std::vector<Effect> mEffects;
        int mIndex;
        Element* mParentElement;
};

#endif // EFFECTLAYER_H


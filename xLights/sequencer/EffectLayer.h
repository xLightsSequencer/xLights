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

#define NO_MAX                  1000
class Element;

class EffectLayer
{
    public:
        EffectLayer(Element* parent);
        virtual ~EffectLayer();

        Effect *AddEffect(int id, int effectIndex, wxString name, wxString settings, int palette,
                          double startTime, double endTime, int Selected, bool Protected);

        Effect* GetEffect(int index);
        void RemoveEffect(int index);

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
        int GetEffectIndexThatContainsPosition(int position,int &selectionType);
        Effect* GetEffectBeforePosition(int position);
        Effect* GetEffectAfterPosition(int position);

        void GetMaximumRangeOfMovementForSelectedEffects(double &toLeft,double &toRight);
        void SelectEffectsInPositionRange(int startX,int endX,int &FirstSelected);
        void SelectEffectsInTimeRange(double startTime,int endTime);
        void UnSelectAllEffects();

        Element* GetParentElement();
        void SetParentElement(Element* parent);
        int GetSelectedEffectCount();
        void MoveAllSelectedEffects(double delta);
        void DeleteSelectedEffects();
        static bool ShouldDeleteSelected(Effect *eff);
        static bool SortEffectByStartTime(Effect* e1,Effect* e2);
    
        void IncrementChangeCount();
        int getChangeCount() const { return changeCount; }

    protected:
    private:
        volatile int changeCount;

        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);
        void GetMaximumRangeOfMovementForEffect(int index, double &toLeft, double &toRight);
        void GetMaximumRangeWithLeftMovement(int index, double &toLeft, double &toRight);
        void GetMaximumRangeWithRightMovement(int index, double &toLeft, double &toRight);
        std::vector<Effect*> mEffects;
        int mIndex;
        Element* mParentElement;
};


#endif // EFFECTLAYER_H


#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H
#include "wx/wx.h"
#include "Effect.h"
#include "UndoManager.h"

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

        Effect *AddEffect(int id, const wxString &name, const wxString &settings, const wxString &palette,
                          double startTime, double endTime, int Selected, bool Protected);
        Effect *AddEffect(int id, int effectIndex, const wxString &name, const wxString &settings, const wxString &palette,
                          double startTime, double endTime, int Selected, bool Protected);
        Effect* GetEffect(int index);
        Effect* GetEffectFromID(int id);
        void RemoveEffect(int index);

        int GetIndex();
        int GetEffectCount();
        void SortEffects();

        bool IsStartTimeLinked(int index);
        bool IsEndTimeLinked(int index);
        bool IsEffectStartTimeInRange(int index, float startTime,float endTime);
        bool IsEffectEndTimeInRange(int index, float startTime,float endTime);

        float GetMaximumEndTime(int index);
        float GetMinimumStartTime(int index);

        bool HitTestEffect(int position,int &index, int &result);
        bool HitTestEffectByTime(double time,int &index);
        int GetEffectIndexThatContainsPosition(int position,int &selectionType);
        Effect* GetEffectBeforePosition(int position);
        Effect* GetEffectAfterPosition(int position);
        bool GetRangeIsClear(int startX, int endX);
        bool GetRangeIsClear(double start_time, double end_time);
        Effect* GetEffectBeforeEmptySpace(int position);
        Effect* GetEffectAfterEmptySpace(int position);

        void GetMaximumRangeOfMovementForSelectedEffects(double &toLeft,double &toRight);
        void SelectEffectsInPositionRange(int startX,int endX);
        int SelectEffectsInTimeRange(double startTime,double endTime);
        bool HasEffectsInTimeRange(double startTime,double endTime);
        void UnSelectAllEffects();

        Element* GetParentElement();
        void SetParentElement(Element* parent);
        int GetSelectedEffectCount();
        void MoveAllSelectedEffects(double delta, UndoManager& undo_mgr);
        void DeleteSelectedEffects(UndoManager& undo_mgr);
        void DeleteEffect(int id);
        void DeleteEffectByIndex(int idx);
        static bool ShouldDeleteSelected(Effect *eff);
        static bool SortEffectByStartTime(Effect* e1,Effect* e2);
        void UpdateAllSelectedEffects(const wxString& palette);

        void IncrementChangeCount(int startMS, int endMS);

    protected:
    private:
        static int exclusive_index;

        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);
        void GetMaximumRangeOfMovementForEffect(int index, double &toLeft, double &toRight);
        void GetMaximumRangeWithLeftMovement(int index, double &toLeft, double &toRight);
        void GetMaximumRangeWithRightMovement(int index, double &toLeft, double &toRight);
        std::vector<Effect*> mEffects;
        int mIndex;
        Element* mParentElement;
};

class NamedLayer: public EffectLayer {
public:
    NamedLayer(Element *parent) : EffectLayer(parent), name(nullptr) {}
    NamedLayer(Element *parent, const wxString &n) : EffectLayer(parent) {
        if ("" == n) {
            name = nullptr;
        } else {
            name = new wxString(n);
        }
    }
    virtual ~NamedLayer() { if (name != nullptr) delete name;}
    const wxString &GetName() const {
        if (name == nullptr) {
            return NO_NAME;
        }
        return *name;
    }
    void SetName(const wxString &n) {
        if (name != nullptr) {
            delete name;
            name = nullptr;
        }
        if ("" != n) {
            name = new wxString(n);
        }
    }
private:
    wxString *name;
    static const wxString NO_NAME;
};

class NodeLayer: public NamedLayer {
public:
    NodeLayer(Element *parent) : NamedLayer(parent) {}
    NodeLayer(Element *parent, const wxString &n) : NamedLayer(parent, n) {}
    virtual ~NodeLayer() {};
private:
};

class ModelClass;
class StrandLayer: public NamedLayer
{
public:
    StrandLayer(Element *parent, int s) : NamedLayer(parent), strand(s) {}
    virtual ~StrandLayer();

    int GetStrand() { return strand;}

    bool ShowNodes() { return mShowNodes;}
    void ShowNodes(bool b) { mShowNodes = b;}

    void InitFromModel(ModelClass &model);

    NodeLayer *GetNodeLayer(int n, bool create = false);
    int GetNodeLayerCount() {
        return mNodeLayers.size();
    }
private:
    int strand;
    bool mShowNodes = false;
    std::vector<NodeLayer*> mNodeLayers;
};


#endif // EFFECTLAYER_H


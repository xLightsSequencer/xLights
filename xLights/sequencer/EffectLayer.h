#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H
#include "wx/wx.h"
#include <atomic>
#include <string>
#include "Effect.h"
#include "UndoManager.h"

#define NO_MIN_MAX_TIME      0

#define NO_MAX                  1000000

class Element;
class Model;


class EffectLayer
{
    public:
        EffectLayer(Element* parent);
        virtual ~EffectLayer();

        Effect *AddEffect(int id, const std::string &name, const std::string &settings, const std::string &palette,
                          int startTimeMS, int endTimeMS, int Selected, bool Protected);
        Effect* GetEffect(int index);
        Effect* GetEffectByTime(int ms);
        Effect* GetEffectFromID(int id);
        void RemoveEffect(int index);

        int GetIndex();
        int GetEffectCount();

        bool IsStartTimeLinked(int index);
        bool IsEndTimeLinked(int index);
        bool IsEffectStartTimeInRange(int index, int startTimeMS, int endTimeMS);
        bool IsEffectEndTimeInRange(int index, int startTimeMS, int endTimeMS);

        int GetMaximumEndTimeMS(int index, bool allow_collapse, int min_period);
        int GetMinimumStartTimeMS(int index, bool allow_collapse, int min_period);

        bool HitTestEffectByTime(int timeMS,int &index);

        Effect* GetEffectAtTime(int ms);
        Effect* GetEffectBeforeTime(int ms);
        Effect* GetEffectAfterTime(int ms);
        Effect* GetEffectBeforeEmptyTime(int ms);
        Effect* GetEffectAfterEmptyTime(int ms);

        bool GetRangeIsClearMS(int startTimeMS, int endTimeMS);

        void GetMaximumRangeOfMovementForSelectedEffects(int &toLeft,int &toRight);
        int SelectEffectsInTimeRange(int startTimeMS, int endTimeMS);
        bool HasEffectsInTimeRange(int startTimeMS, int endTimeMS);
        void UnSelectAllEffects();

        Element* GetParentElement();
        void SetParentElement(Element* parent);
        int GetSelectedEffectCount();
        void MoveAllSelectedEffects(int deltaMS, UndoManager& undo_mgr);
        void DeleteSelectedEffects(UndoManager& undo_mgr);
        void DeleteEffect(int id);
        void DeleteEffectByIndex(int idx);
        static bool ShouldDeleteSelected(Effect *eff);
        static bool SortEffectByStartTime(Effect* e1,Effect* e2);
        void UpdateAllSelectedEffects(const std::string& palette);

        void IncrementChangeCount(int startMS, int endMS);

        wxMutex &GetLock() {return lock;}
    protected:
    private:
        void SortEffects();

        static std::atomic_int exclusive_index;

        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);
        void GetMaximumRangeOfMovementForEffect(int index, int &toLeft, int &toRight);
        void GetMaximumRangeWithLeftMovement(int index, int &toLeft, int &toRight);
        void GetMaximumRangeWithRightMovement(int index, int &toLeft, int &toRight);
        std::vector<Effect*> mEffects;
        int mIndex;
        Element* mParentElement;
        wxMutex lock;
};

class NamedLayer: public EffectLayer {
public:
    NamedLayer(Element *parent) : EffectLayer(parent), name(nullptr) {}
    NamedLayer(Element *parent, const std::string &n) : EffectLayer(parent) {
        if ("" == n) {
            name = nullptr;
        } else {
            name = new std::string(n);
        }
    }
    virtual ~NamedLayer() { if (name != nullptr) delete name;}
    const std::string &GetName() const {
        if (name == nullptr) {
            return NO_NAME;
        }
        return *name;
    }
    void SetName(const std::string &n) {
        if (name != nullptr) {
            delete name;
            name = nullptr;
        }
        if ("" != n) {
            name = new std::string(n);
        }
    }
private:
    std::string *name;
    static const std::string NO_NAME;
};

class NodeLayer: public NamedLayer {
public:
    NodeLayer(Element *parent) : NamedLayer(parent) {}
    NodeLayer(Element *parent, const std::string &n) : NamedLayer(parent, n) {}
    virtual ~NodeLayer() {};
private:
};

class StrandLayer: public NamedLayer
{
public:
    StrandLayer(Element *parent, int s) : NamedLayer(parent), strand(s) {}
    virtual ~StrandLayer();

    int GetStrand() { return strand;}

    bool ShowNodes() { return mShowNodes;}
    void ShowNodes(bool b) { mShowNodes = b;}

    void InitFromModel(Model &model);

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


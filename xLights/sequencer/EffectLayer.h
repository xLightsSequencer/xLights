#ifndef EFFECTLAYER_H
#define EFFECTLAYER_H
#include "wx/wx.h"
#include <atomic>
#include <string>
#include <list>
#include <mutex>
#include "Effect.h"
#include "UndoManager.h"
#include "../effects/EffectManager.h"

#define NO_MIN_MAX_TIME      0

#define NO_MAX                  1000000

class Element;
class Model;
class ValueCurve;
class EffectsGrid;

class EffectLayer
{
    public:
        EffectLayer(Element* parent);
        virtual ~EffectLayer();

        Effect *AddEffect(int id, const std::string &name, const std::string &settings, const std::string &palette,
                          int startTimeMS, int endTimeMS, int Selected, bool Protected);
        Effect* GetEffect(int index) const;
        Effect* GetEffectByTime(int ms);
        Effect* GetEffectFromID(int id);
        void RemoveEffect(int index);
        void RemoveAllEffects(UndoManager *undo_mgr);
        std::list<std::string> GetFileReferences(EffectManager& em) const;

        int SelectEffectByTypeInTimeRange(const std::string &type, int startTimeMS, int endTimeMS);
        std::vector<Effect*> GetEffectsByTypeAndTime(const std::string &type, int startTimeMS, int endTimeMS);
        std::vector<Effect*> GetAllEffectsByTime(int startTimeMS, int endTimeMS);

        int GetIndex();
        int GetEffectCount() const;

        bool IsStartTimeLinked(int index);
        bool IsEndTimeLinked(int index);
        bool IsEffectStartTimeInRange(int index, int startTimeMS, int endTimeMS);
        bool IsEffectEndTimeInRange(int index, int startTimeMS, int endTimeMS);

        int GetMaximumEndTimeMS(int index, bool allow_collapse, int min_period);
        int GetMinimumStartTimeMS(int index, bool allow_collapse, int min_period);

        bool HitTestEffectByTime(int timeMS,int &index);
        bool HitTestEffectBetweenTime(int t1MS, int t2MS);

        Effect* GetEffectAtTime(int ms);
        Effect* GetEffectBeforeTime(int ms);
        Effect* GetEffectAfterTime(int ms);
        Effect* GetEffectBeforeEmptyTime(int ms);
        Effect* GetEffectAfterEmptyTime(int ms);

        bool GetRangeIsClearMS(int startTimeMS, int endTimeMS, bool ignore_selected = false);

        void GetMaximumRangeOfMovementForSelectedEffects(int &toLeft,int &toRight);
        int SelectEffectsInTimeRange(int startTimeMS, int endTimeMS);
        bool HasEffectsInTimeRange(int startTimeMS, int endTimeMS);

        int SelectEffectsByType(const std::string & type);
        void UnSelectAllEffects();
        void SelectAllEffects();

        Element* GetParentElement();
        void SetParentElement(Element* parent);
        int GetSelectedEffectCount();
        int GetTaggedEffectCount();
        void MoveAllSelectedEffects(int deltaMS, UndoManager& undo_mgr);
        void StretchAllSelectedEffects(int deltaMS, UndoManager& undo_mgr);
        void ButtUpMoveAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr);
        void ButtUpStretchAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr);
        void TagAllSelectedEffects();
        int GetSelectedEffectCount(const std::string effectName);
        void ApplyEffectSettingToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid);
        void UnTagAllEffects();
        void DeleteSelectedEffects(UndoManager& undo_mgr);
        void DeleteEffect(int id);
        void DeleteEffectByIndex(int idx);
        static bool ShouldDeleteSelected(Effect *eff);
        static bool SortEffectByStartTime(Effect* e1,Effect* e2);
        void UpdateAllSelectedEffects(const std::string& palette);

        void IncrementChangeCount(int startMS, int endMS);

        std::recursive_mutex &GetLock() {return lock;}
    
        void CleanupAfterRender();
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
        std::list<Effect*> mEffectsToDelete;
        int mIndex;
        Element* mParentElement;
        std::recursive_mutex lock;
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


#endif // EFFECTLAYER_H


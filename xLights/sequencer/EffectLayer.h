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
class xLightsFrame;

class EffectLayer
{
    public:
        EffectLayer(Element* parent);
        virtual ~EffectLayer();

        Effect *AddEffect(int id, const std::string &name, const std::string &settings, const std::string &palette,
                          int startTimeMS, int endTimeMS, int Selected, bool Protected, bool suppress_sort = false, bool importing = false);
        Effect* GetEffect(int index) const;
        const std::vector<Effect*>& GetEffects() const { return mEffects; }
        Effect* GetEffectByTime(int ms);
        Effect* GetEffectFromID(int id);
        int GetFirstSelectedEffectStartMS() const;
        int GetLastSelectedEffectEndMS() const;
        void RemoveEffect(int index);
        void RemoveAllEffects(UndoManager *undo_mgr);
        std::list<std::string> GetFileReferences(Model* model, EffectManager& em) const;
        std::list<std::string> GetFacesUsed(EffectManager& em) const;
        bool CleanupFileLocations(xLightsFrame* frame, EffectManager& em);

        std::vector<Effect*> GetEffectsByTypeAndTime(const std::string &type, int startTimeMS, int endTimeMS);
        std::vector<Effect*> GetAllEffectsByTime(int startTimeMS, int endTimeMS);
        Effect* SelectEffectUsingDescription(std::string description);
        bool IsEffectValid(Effect* e) const;
        Effect* SelectEffectUsingTime(int time);

        int GetLayerNumber() const;
        int GetIndex() const;
        int GetEffectCount() const;

        bool IsStartTimeLinked(int index) const;
        bool IsEndTimeLinked(int index) const;
        bool IsEffectStartTimeInRange(int index, int startTimeMS, int endTimeMS) const;
        bool IsEffectEndTimeInRange(int index, int startTimeMS, int endTimeMS) const;

        int GetMaximumEndTimeMS(int index, bool allow_collapse, int min_period) const;
        int GetMinimumStartTimeMS(int index, bool allow_collapse, int min_period) const;

        bool HitTestEffectByTime(int timeMS,int &index) const;
        bool HitTestEffectBetweenTime(int t1MS, int t2MS) const;

        Effect* GetEffectAtTime(int ms) const;
        Effect* GetEffectStartingAtTime(int ms) const;
        Effect* GetEffectBeforeTime(int ms) const;
        Effect* GetEffectAfterTime(int ms) const;
        Effect* GetEffectBeforeEmptyTime(int ms) const;
        Effect* GetEffectAfterEmptyTime(int ms) const;
        std::list<Effect*> GetAllEffects() const;

        bool GetRangeIsClearMS(int startTimeMS, int endTimeMS, bool ignore_selected = false);

        void GetMaximumRangeOfMovementForSelectedEffects(int &toLeft,int &toRight);
        int SelectEffectsInTimeRange(int startTimeMS, int endTimeMS);
        bool HasEffectsInTimeRange(int startTimeMS, int endTimeMS);
        bool HasEffects();
        bool HasEffectsByType(const std::string& type) const;

        void UnSelectAllEffects();
        void SelectAllEffects();
        void ConvertEffectsToPerModel(UndoManager& undo_manager);

        Element* GetParentElement() const;
        void SetParentElement(Element* parent);
        int GetSelectedEffectCount();
        int GetTaggedEffectCount();
        void MoveAllSelectedEffects(int deltaMS, UndoManager& undo_mgr);
        void StretchAllSelectedEffects(int deltaMS, UndoManager& undo_mgr);
        void ButtUpMoveAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr);
        void ButtUpStretchAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr);
        void TagAllSelectedEffects();
        int GetSelectedEffectCount(const std::string effectName);
        std::vector<std::string> GetUsedColours(bool selectedOnly);
        int ReplaceColours(xLightsFrame* frame, const std::string& from, const std::string& to, bool selectedOnly, UndoManager& undo_mgr);
        void ApplyEffectSettingToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid, EffectManager& effectManager, RangeAccumulator& rangeAccumulator);
        void ApplyButtonPressToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::string id, EffectManager& effectManager, RangeAccumulator& rangeAccumulator);
        void RemapSelectedDMXEffectValues(EffectsGrid* effects_grid, UndoManager& undo_manager, const std::vector<std::tuple<int, int, float, int>>& dmxmappings, const EffectManager& effect_manager, RangeAccumulator& range_accumulator);
        void ConvertSelectedEffectsTo(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, EffectManager& effectManager, RangeAccumulator& rangeAccumulator);
        void UnTagAllEffects();
        void DeleteSelectedEffects(UndoManager& undo_mgr);
        void DeleteAllEffects();
        void DeleteEffect(int id);
        void DeleteEffectByIndex(int idx);
        static bool ShouldDeleteSelected(Effect* eff);
        static bool ShouldDeleteNotLocked(Effect* eff);
        static bool SortEffectByStartTime(Effect* e1,Effect* e2);
        void UpdateAllSelectedEffects(const std::string& palette);

        void IncrementChangeCount(int startMS, int endMS);

        std::recursive_mutex &GetLock() {return lock;}
    
        bool IsTimingLayer();
        bool IsFixedTimingLayer();

        void CleanupAfterRender();
        void NumberEffects();
    protected:
    private:
        void SortEffects();
        void PlayEffect(Effect* effect);

        static std::atomic_int exclusive_index;

        int EffectToLeftEndTime(int index);
        int EffectToRightStartTime(int index);
        void GetMaximumRangeOfMovementForEffect(int index, int &toLeft, int &toRight);
        void GetMaximumRangeWithLeftMovement(int index, int &toLeft, int &toRight);
        void GetMaximumRangeWithRightMovement(int index, int &toLeft, int &toRight);
        std::vector<Effect*> mEffects;
        std::list<Effect*> mEffectsToDelete;
        int mIndex = 0;
        Element* mParentElement = nullptr;
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
    std::string *name = nullptr;
    static const std::string NO_NAME;
};

class NodeLayer: public NamedLayer {
public:
    NodeLayer(Element *parent) : NamedLayer(parent) {}
    NodeLayer(Element *parent, const std::string &n) : NamedLayer(parent, n) {}
    virtual ~NodeLayer() {};
private:
};

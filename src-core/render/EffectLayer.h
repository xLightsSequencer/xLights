#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "Effect.h"
#include "UndoManager.h"
#include "../effects/EffectManager.h"
#include <atomic>
#include <functional>
#include <tuple>
#include <list>
#include <mutex>
#include <string>

#define NO_MIN_MAX_TIME 0

#define NO_MAX 1000000

class Element;
class Model;
class ValueCurve;
class EffectsGrid;
class RenderContext;

class EffectLayer {
public:
    EffectLayer(Element* parent);
    virtual ~EffectLayer();

    Effect* AddEffect(int id, const std::string& name, const std::string& settings, const std::string& palette,
                      int startTimeMS, int endTimeMS, int Selected, bool Protected, bool suppress_sort = false, bool importing = false);
    Effect* GetEffect(int index) const;
    const std::vector<Effect*>& GetEffects() const {
        return mEffects;
    }
    Effect* GetEffectByTime(int ms);
    Effect* GetEffectFromID(int id);
    Effect* GetEffectFromUid(SeqObjectId uid) const;
    size_t GetUidIndexBuildCount() const { return mUidIndexBuilds; }
    // Called by Effect::SetUid so the id index follows the new id.
    void EffectUidChanged(Effect* e, SeqObjectId oldUid);
    int GetFirstSelectedEffectStartMS() const;
    int GetLastSelectedEffectEndMS() const;
    void RemoveEffect(int index);
    void RemoveAllEffects(UndoManager* undo_mgr);
    std::list<std::string> GetFileReferences(RenderContext* ctx, Model* model, EffectManager& em) const;
    std::list<std::string> GetFacesUsed(EffectManager& em) const;
    bool CleanupFileLocations(RenderContext* ctx, EffectManager& em);

    std::vector<Effect*> GetEffectsByTypeAndTime(const std::string& type, int startTimeMS, int endTimeMS);
    std::vector<Effect*> GetAllEffectsByTime(int startTimeMS, int endTimeMS);
    Effect* SelectEffectUsingDescription(std::string description);
    bool IsEffectValid(Effect* e) const;
    Effect* SelectEffectUsingTime(int time);

    int GetLayerNumber() const;
    int GetIndex() const;
    SeqObjectId GetUid() const { return mUid; }
    // Only for restoring an identity onto a re-created layer (undo). The
    // caller guarantees no live layer already holds `uid`.
    void SetUid(SeqObjectId uid) { mUid = uid; }
    int GetEffectCount() const;

    bool IsStartTimeLinked(int index) const;
    bool IsEndTimeLinked(int index) const;
    bool IsEffectStartTimeInRange(int index, int startTimeMS, int endTimeMS) const;
    bool IsEffectEndTimeInRange(int index, int startTimeMS, int endTimeMS) const;

    int GetMaximumEndTimeMS(int index, bool allow_collapse, int min_period) const;
    int GetMinimumStartTimeMS(int index, bool allow_collapse, int min_period) const;

    // Chronological neighbor of mEffects[index] found by scanning start times rather than
    // trusting adjacent array position, so a stale sort order (e.g. after dragging an effect
    // past a sibling without a re-sort) can't return the wrong neighbor.
    Effect* GetPriorEffect(int index) const;
    Effect* GetNextEffect(int index) const;

    bool HitTestEffectByTime(int timeMS, int& index) const;
    bool HitTestEffectBetweenTime(int t1MS, int t2MS) const;

    Effect* GetEffectAtTime(int ms, const std::string& filterText = "", bool isFilterTextRegex = false) const;
    Effect* GetEffectStartingAtTime(int ms, const std::string& filterText = "", bool isFilterTextRegex = false) const;
    Effect* GetEffectBeforeTime(int ms, const std::string& filterText = "", bool isFilterTextRegex = false) const;
    Effect* GetEffectAfterTime(int ms, const std::string& filterText = "", bool isFilterTextRegex = false) const;
    Effect* GetEffectBeforeEmptyTime(int ms) const;
    Effect* GetEffectAfterEmptyTime(int ms) const;
    std::list<Effect*> GetAllEffects() const;

    bool GetRangeIsClearMS(int startTimeMS, int endTimeMS, bool ignore_selected = false);

    void GetMaximumRangeOfMovementForSelectedEffects(int& toLeft, int& toRight);
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
    void ButtUpStretchAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr);
    void TagAllSelectedEffects();
    int GetSelectedEffectCount(const std::string effectName);
    std::vector<std::string> GetUsedColours(bool selectedOnly);
    int ReplaceColours(RenderContext* ctx, const std::string& from, const std::string& to, bool selectedOnly, UndoManager& undo_mgr);
    void ApplyEffectSettingToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid, EffectManager& effectManager, RangeAccumulator& rangeAccumulator);
    void ApplyButtonPressToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::string id, EffectManager& effectManager, RangeAccumulator& rangeAccumulator);
    // Like ApplyEffectSettingToSelected but lets the caller supply arbitrary mutation logic
    // instead of a single id/value pair -- used where the settings a control edits aren't a
    // flat key on the effect (e.g. the Moving Head panel's per-fixture settings blobs).
    // The mutator returns true if it actually changed the effect, so undo/render only fire
    // for effects that were really touched.
    void ApplyCallbackToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::function<bool(Effect*)>& mutator, EffectManager& effectManager, RangeAccumulator& rangeAccumulator);
    void RemapSelectedDMXEffectValues(EffectsGrid* effects_grid, UndoManager& undo_manager, const std::vector<std::tuple<int, int, float, int, std::string>>& dmxmappings, const EffectManager& effect_manager, RangeAccumulator& range_accumulator);
    void UnTagAllEffects();
    void DeleteSelectedEffects(UndoManager& undo_mgr);
    void DeleteAllEffects();
    void DeleteEffect(int id);
    // Like DeleteEffect(id) but by stable identity; returns false if not found.
    bool DeleteEffectByUid(SeqObjectId uid);
    void DeleteEffectByIndex(int idx);
    static bool ShouldDeleteSelected(Effect* eff);
    static bool ShouldDeleteNotLocked(Effect* eff);
    static bool SortEffectByStartTime(Effect* e1, Effect* e2);
    void UpdateAllSelectedEffects(const std::string& palette);

    void IncrementChangeCount(int startMS, int endMS);

    std::recursive_mutex& GetLock() {
        return lock;
    }

    bool IsTimingLayer() const;
    bool IsFixedTimingLayer() const;

    void CleanupAfterRender();
    void NumberEffects();
    // Puts the layer's effects in `order` (the same effects, any order) and
    // keeps their position ids. False, changing nothing, if `order` isn't
    // exactly this layer's effects. The order isn't document state, but the
    // .xsq is written in it; session snapshots use this to match the host.
    bool ReorderEffects(const std::vector<Effect*>& order);

    const std::string& GetLayerName() const {
        if (name == nullptr) {
            return NO_NAME;
        }
        return *name;
    }
    void SetLayerName(const std::string& n) {
        XL_SEQ_MUTATION("EffectLayer::SetLayerName");
        if (name != nullptr) {
            delete name;
            name = nullptr;
        }
        if ("" != n) {
            name = new std::string(n);
        }
    }

protected:
private:
    std::unique_lock<std::recursive_mutex> acquireLockWaitForRender();

    void SortEffects();

    static std::atomic_int exclusive_index;

    int EffectToLeftEndTime(int index);
    int EffectToRightStartTime(int index);
    void GetMaximumRangeOfMovementForEffect(int index, int& toLeft, int& toRight);
    void GetMaximumRangeWithLeftMovement(int index, int& toLeft, int& toRight);
    void GetMaximumRangeWithRightMovement(int index, int& toLeft, int& toRight);
    std::vector<Effect*> mEffects;
    std::list<Effect*> mEffectsToDelete;
    // GetEffectFromUid index for layers with many effects, created on first
    // use. Single adds and removes update it in place (bulk edits one effect
    // at a time would otherwise rebuild it per effect); bulk changes drop it.
    // A hit is checked against the effect's current id and a miss falls back
    // to a scan, so an id changed behind its back can't make it lie.
    struct UidIndex;
    mutable std::atomic<UidIndex*> mUidIndex{ nullptr };
    mutable size_t mUidIndexBuilds = 0;
    void InvalidateUidIndex();
    void IndexAdded(Effect* e);
    void IndexRemoved(Effect* e);
    int mIndex = 0;
    SeqObjectId mUid = SeqIds::Next();
    Element* mParentElement = nullptr;
    std::recursive_mutex lock;
    std::mutex effectsToDeleteLock;

    std::string* name = nullptr;

protected:
    static const std::string NO_NAME;
};

class NodeLayer : public EffectLayer {
public:
    NodeLayer(Element* parent) :
        EffectLayer(parent) {
    }
    NodeLayer(Element* parent, const std::string& n) :
        EffectLayer(parent) {
        SetNodeName(n);
    }
    virtual ~NodeLayer() {
        if (nodeName) {
            delete nodeName;
        }
    };

    const std::string& GetNodeName() const {
        if (nodeName == nullptr) {
            return NO_NAME;
        }
        return *nodeName;
    }
    void SetNodeName(const std::string& n) {
        if (nodeName != nullptr) {
            delete nodeName;
            nodeName = nullptr;
        }
        if ("" != n) {
            nodeName = new std::string(n);
        }
    }

private:
    std::string* nodeName = nullptr;
};

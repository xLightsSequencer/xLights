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

#include <vector>
#include <atomic>
#include <mutex>
#include <string>

#include "EffectLayer.h"
#include "../effects/EffectManager.h"

enum class ElementType
{
    ELEMENT_TYPE_MODEL,
    ELEMENT_TYPE_SUBMODEL,
    ELEMENT_TYPE_STRAND,
    ELEMENT_TYPE_TIMING
};

class NetInfoClass;
class Element;
class ModelElement;
class Model;
class xLightsFrame;
class SequenceElements;

class ChangeListener {
public:
    ChangeListener() {}
    virtual ~ChangeListener() {}
    virtual void IncrementChangeCount(Element *el) = 0;
};

class Element {
public:
    Element(SequenceElements *p, const std::string &name);
    virtual ~Element();
    
    virtual ElementType GetType() const = 0;
    std::string GetTypeDescription() const;

    SequenceElements *GetSequenceElements() const {return parent;}

    const std::string &GetName() const;
    void SetName(const std::string &name);
    virtual const std::string &GetModelName() const;
    virtual std::string GetFullName() const;
    
    bool GetVisible() const {return mVisible;}
    void SetVisible(bool visible) {mVisible = visible;}

    virtual bool HasEffects() const;
 
    virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index);
    EffectLayer* GetEffectLayer(int index) const;
    const std::vector<EffectLayer*>& GetEffectLayers() const { return mEffectLayers; }
    int GetLayerNumberFromIndex(int index);
    virtual NodeLayer* GetNodeEffectLayer(int index) const = 0;
    size_t GetEffectLayerCount() const;
    std::list<std::string> GetFileReferences(Model* model, EffectManager& em) const;
    std::list<std::string> GetFacesUsed(EffectManager& em) const;
    bool CleanupFileLocations(xLightsFrame* frame, EffectManager& em);
    Effect* SelectEffectUsingDescription(std::string description);
    virtual bool IsEffectValid(Effect* e) const;
    Effect* SelectEffectUsingLayerTime(int layer, int time);
	virtual std::vector<int> GetLayersWithEffectsByTime(int startMs, int endMS) const;
    int GetSelectedEffectCount() const;
    int GetFirstSelectedEffectStartMS() const;
    int GetLastSelectedEffectEndMS() const;
    void SetRenderDisabled(bool rd);
    bool IsRenderDisabled() const
    {
        return _renderDisabled;
    }

    EffectLayer* AddEffectLayer();
    void RemoveEffectLayer(int index);
    EffectLayer* InsertEffectLayer(int index);
    bool operator==(const Element& e) const;
    bool operator<(const Element& e) const;

    bool GetCollapsed() const { return mCollapsed; }
    void SetCollapsed(bool collapsed) { mCollapsed = collapsed; }
    
    int GetIndex() const {return mIndex;}
    void SetIndex(int index) { mIndex = index;}
    int &Index() { return mIndex;}
    int Index() const { return mIndex;}
    
    std::recursive_timed_mutex &GetChangeLock() { return changeLock; }
    virtual void IncrementChangeCount(int startMs, int endMS);
    int getChangeCount() const { return changeCount; }
    
    void GetDirtyRange(int &startMs, int &endMs) const {
        startMs = dirtyStart;
        endMs = dirtyEnd;
    }
    void GetAndResetDirtyRange(int &changes, int &startMs, int &endMs) {
        changes = changeCount;
        startMs = dirtyStart;
        endMs = dirtyEnd;
        dirtyStart = dirtyEnd = -1;
    }
    void SetDirtyRange(int start, int end) {
        if (dirtyStart == -1) {
            dirtyStart = start;
            dirtyEnd = end;
        } else {
            if (dirtyEnd < end) {
                dirtyEnd = end;
            }
            if (dirtyStart > start) {
                dirtyStart = start;
            }
        }
    }
    void ClearDirtyFlags() {
        dirtyStart = dirtyEnd = -1;
    }
    virtual void CleanupAfterRender();
    
protected:
    EffectLayer* AddEffectLayerInternal();

    SequenceElements *parent = nullptr;

    std::string mName;
    int mIndex = 0;
    bool mVisible = true;
    bool mCollapsed = false;
    bool _renderDisabled = false;

    std::vector<EffectLayer*> mEffectLayers;
    std::list<EffectLayer *> mLayersToDelete;
    ChangeListener *listener = nullptr;
    volatile int changeCount = 0;
    volatile int dirtyStart = -1;
    volatile int dirtyEnd = -1;

    std::recursive_timed_mutex changeLock;
};

class TimingElement : public Element
{
public:
    TimingElement(SequenceElements *p, const std::string &name);
    virtual ~TimingElement();
    
    virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_TIMING; }
    
    const std::string &GetSubType() const { return subType; }
    void SetSubType(const std::string &st) { subType = st; }

    int GetFixedTiming() const { return mFixed; }
    void SetFixedTiming(int fixed) { mFixed = fixed; }
    bool IsFixedTiming() const { return mFixed != 0; }
    void Unfix();

    const std::string &GetViews() const { return mViews; }
    void SetViews(const std::string &views) { mViews = views; }

    bool GetActive() const { return mActive; }
    void SetActive(bool active) { mActive = active; }

    std::string GetExport() const;
    std::string GetPapagayoExport(int ms) const;
    virtual NodeLayer* GetNodeEffectLayer(int index) const override { return nullptr; }
    bool HasLyrics(int layer) const;
    bool GetMasterVisible() const { return _masterVisible; }
    void SetMasterVisible(bool visible) { _masterVisible = visible; }

private:
    int mFixed = 0;
    bool mActive = true;
    std::string mViews;
    bool _masterVisible = false;
    std::string subType = "";
};

class SubModelElement : public Element {
public:
    SubModelElement(ModelElement *model, const std::string &name);
    virtual ~SubModelElement();
    
    ModelElement *GetModelElement() const { return mParentModel;}
    
    virtual const std::string &GetModelName() const override;
    virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_SUBMODEL; }
    
    virtual std::string GetFullName() const override;
    virtual void IncrementChangeCount(int startMs, int endMS) override;
    virtual NodeLayer* GetNodeEffectLayer(int index) const override { return nullptr; }

    virtual bool HasEffects() const override;

protected:
    ModelElement *mParentModel = nullptr;
};

class StrandElement : public SubModelElement {
public:
    StrandElement(ModelElement *model, int strand);
    virtual ~StrandElement();

    void InitFromModel(Model &model);
    
    virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index) override;
    virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_STRAND; }
    virtual NodeLayer* GetNodeEffectLayer(int index) const override;
    virtual bool IsEffectValid(Effect* e) const override;

    int GetStrand() const { return mStrand; }
    
    virtual bool HasEffects() const override;
    bool ShowNodes() const { return mShowNodes;}
    void ShowNodes(bool b) { mShowNodes = b;}
    NodeLayer *GetNodeLayer(int n, bool create);
    NodeLayer *GetNodeLayer(int n) const;
    int GetNodeLayerCount() const {
        return mNodeLayers.size();
    }

    int GetNodeNumber(NodeLayer* nl);
    std::string GetStrandName() const;

    virtual std::string GetFullName() const override;
    
    virtual void CleanupAfterRender() override;

private:
    int mStrand = 0;
    bool mShowNodes = false;
    std::vector<NodeLayer*> mNodeLayers;
};

class ModelElement : public Element
{
    public:
        ModelElement(SequenceElements *p, const std::string &name, bool selected);
        ModelElement(const std::string &name);
        virtual ~ModelElement();
    
        void Init(Model &cls);
        virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_MODEL; }

        bool GetSelected();
        void SetSelected(bool selected);

        virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index) override;

        virtual bool HasEffects() const override;
        int GetSubModelAndStrandCount() const;
        int GetSubModelCount() const;
        SubModelElement *GetSubModel(int i) const;
        SubModelElement *GetSubModel(int i);
        SubModelElement *GetSubModel(const std::string &name, bool create = false);
        void RemoveSubModel(const std::string &name);
        void RemoveAllSubModels();
        void AddSubModel(SubModelElement* sme);

        bool ShowStrands() const { return mStrandsVisible;}
        void ShowStrands(bool b) { mStrandsVisible = b;}
        virtual NodeLayer* GetNodeEffectLayer(int index) const override;

        std::recursive_timed_mutex &GetRenderLock() { return changeLock; }
        int GetWaitCount() const { return waitCount; }
        void IncWaitCount() { waitCount++; }
        int DecWaitCount() { return --waitCount; }

        StrandElement *GetStrand(int strand, bool create = false);
        StrandElement *GetStrand(int strand) const;
        int GetStrandCount() const { return mStrands.size(); }
    
        virtual void CleanupAfterRender() override;

    protected:
    private:
        bool mStrandsVisible = false;
        bool mSelected = false;
        std::vector<SubModelElement*> mSubModels;
        std::vector<StrandElement*> mStrands;
        std::atomic_int waitCount;
};


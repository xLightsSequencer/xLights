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

enum ElementId : uintptr_t
{
    kInvalidElementId = 0
};

namespace detail
{
inline ElementId nextElementId()
{
    static std::atomic_uintptr_t id{ 1 };
    // Need atomicity only, so used relaxed semantics
    return ElementId(id.fetch_add(1, std::memory_order_relaxed));
}
}

class Element : public std::enable_shared_from_this<Element> {
protected:
    // To control who has visibility to be able to construct this. Use the Construct<> function only.
    enum class ConstructorKey;
public:
    Element(ConstructorKey, SequenceElements* p, const std::string& name);
    virtual ~Element();

    template <class ElementType, class... Args, std::enable_if_t<std::is_base_of_v<Element, ElementType>, bool> = false>
    static std::shared_ptr<ElementType> Construct(Args&&... args)
    {
        auto p = std::make_shared<ElementType>(ConstructorKey{}, std::forward<Args>(args)...);
        if (p)
        {
            // The constructor itself does not have a weak pointer yet, so we must add to the map after construction.
            auto& m = map();
            std::lock_guard g(m.mutex);
            m.map[p->GetId()] = std::static_pointer_cast<Element>(p);
        }
        return p;
    }

    ElementId GetId() const
    {
        return elementId;
    }

    static std::shared_ptr<Element> GetById(ElementId id)
    {
        auto& m = map();
        std::lock_guard g(m.mutex);
        auto it = m.map.find(id);
        return it != m.map.end() ? it->second.lock() : std::shared_ptr<Element>{};
    }

    virtual ElementType GetType() const = 0;
    std::string GetTypeDescription() const;

    SequenceElements *GetSequenceElements() const {return parent;}

    const std::string &GetName() const;
    void SetName(const std::string &name);
    virtual const std::string &GetModelName() const;
    virtual std::string GetFullName() const;
    
    bool GetVisible() const {return mVisible;}
    void SetVisible(bool visible) {mVisible = visible;}

    [[nodiscard]] virtual bool HasEffects() const;
    [[nodiscard]] virtual int GetEffectCount() const;
 
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

    ElementId elementId{ detail::nextElementId() };

    SequenceElements *parent = nullptr;

    std::string mName;
    int mIndex = 0;
    bool mVisible = true;
    bool mCollapsed = false;
    bool _renderDisabled = false;

    std::vector<EffectLayer*> mEffectLayers;
    std::list<EffectLayer *> mLayersToDelete;
    ChangeListener *listener = nullptr;
    std::atomic<int> changeCount = 0;
    std::atomic<int> dirtyStart = -1;
    std::atomic<int> dirtyEnd = -1;

    std::recursive_timed_mutex changeLock;

private:
    struct ElementMap
    {
        std::mutex mutex;
        std::unordered_map<ElementId, std::weak_ptr<Element>> map;
    };
    static ElementMap& map()
    {
        static ElementMap m;
        return m;
    }
};

class TimingElement : public Element
{
public:
    TimingElement(ConstructorKey, SequenceElements *p, const std::string &name);
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
    SubModelElement(ConstructorKey, ModelElement* model, const std::string& name);
    virtual ~SubModelElement();
    
    ModelElement *GetModelElement() const { return mParentModel;}
    
    virtual const std::string &GetModelName() const override;
    virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_SUBMODEL; }
    
    virtual std::string GetFullName() const override;
    virtual void IncrementChangeCount(int startMs, int endMS) override;
    virtual NodeLayer* GetNodeEffectLayer(int index) const override { return nullptr; }

    [[nodiscard]] virtual bool HasEffects() const override;

protected:
    ModelElement *mParentModel = nullptr;
};

class StrandElement : public SubModelElement {
public:
    StrandElement(ConstructorKey, ModelElement* model, int strand);
    virtual ~StrandElement();

    void InitFromModel(Model &model);
    
    virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index) override;
    virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_STRAND; }
    virtual NodeLayer* GetNodeEffectLayer(int index) const override;
    virtual bool IsEffectValid(Effect* e) const override;

    int GetStrand() const { return mStrand; }
    
    [[nodiscard]] virtual bool HasEffects() const override;
    [[nodiscard]] int GetEffectCount() const override;
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
        ModelElement(ConstructorKey, SequenceElements *p, const std::string &name, bool selected);
        ModelElement(ConstructorKey, const std::string &name);
        virtual ~ModelElement();
    
        void Init(Model &cls);
        virtual ElementType GetType() const override { return ElementType::ELEMENT_TYPE_MODEL; }

        bool GetSelected();
        void SetSelected(bool selected);

        virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index) override;

        [[nodiscard]] virtual bool HasEffects() const override;
        [[nodiscard]] int GetEffectCount() const override;
        int GetSubModelAndStrandCount() const;
        int GetSubModelCount() const;
        std::shared_ptr<SubModelElement> GetSubModel(int i) const;
        std::shared_ptr<SubModelElement> GetSubModel(const std::string &name, bool create = false);
        void RemoveSubModel(const std::string &name);
        void RemoveAllSubModels();
        void AddSubModel(std::shared_ptr<SubModelElement> sme);

        bool ShowStrands() const { return mStrandsVisible;}
        void ShowStrands(bool b) { mStrandsVisible = b;}
        virtual NodeLayer* GetNodeEffectLayer(int index) const override;

        std::recursive_timed_mutex &GetRenderLock() { return changeLock; }
        int GetWaitCount() const { return waitCount; }
        void IncWaitCount() { waitCount++; }
        int DecWaitCount() { return --waitCount; }

        std::shared_ptr<StrandElement> GetStrand(int strand, bool create = false);
        std::shared_ptr<StrandElement> GetStrand(int strand) const;
        int GetStrandCount() const { return mStrands.size(); }
    
        virtual void CleanupAfterRender() override;

    protected:
    private:
        bool mStrandsVisible = false;
        bool mSelected = false;
        std::vector<std::shared_ptr<SubModelElement>> mSubModels;
        std::vector<std::shared_ptr<StrandElement>> mStrands;
        std::atomic_int waitCount;
};


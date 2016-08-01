#ifndef ELEMENT_H
#define ELEMENT_H

#include <vector>
#include <atomic>
#include <mutex>
#include <string>

#include "EffectLayer.h"

enum ElementType
{
    ELEMENT_TYPE_MODEL,
    ELEMENT_TYPE_TIMING
};

class NetInfoClass;
class Element;
class ModelElement;
class Model;

class ChangeListener {
public:
    virtual void IncrementChangeCount(Element *el) = 0;
};

class SequenceElements;


class Element {
public:
    Element(SequenceElements *p, const std::string &name);
    virtual ~Element();
    
    virtual const std::string GetType() const = 0;

    SequenceElements *GetSequenceElements() const {return parent;}

    const std::string &GetName() const;
    void SetName(const std::string &name);
    
    
    bool GetVisible() const {return mVisible;}
    void SetVisible(bool visible) {mVisible = visible;}

    
    virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index);
    EffectLayer* GetEffectLayer(int index);
    size_t GetEffectLayerCount();
    
    EffectLayer* AddEffectLayer();
    void RemoveEffectLayer(int index);
    EffectLayer* InsertEffectLayer(int index);

    
    bool GetCollapsed() const { return mCollapsed; }
    void SetCollapsed(bool collapsed) { mCollapsed = collapsed; }
    

    int GetIndex() const {return mIndex;}
    void SetIndex(int index) { mIndex = index;}
    int &Index() { return mIndex;}
    int Index() const { return mIndex;}
    
    
    std::recursive_mutex &GetChangeLock() { return changeLock; }
    void IncrementChangeCount(int startMs, int endMS);
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
    
protected:
    SequenceElements *parent;

    std::string mName;
    int mIndex;
    bool mVisible;
    bool mCollapsed;

    std::vector<EffectLayer*> mEffectLayers;
    ChangeListener *listener;
    volatile int changeCount = 0;
    volatile int dirtyStart = -1;
    volatile int dirtyEnd = -1;

    std::recursive_mutex changeLock;
};


class TimingElement : public Element
{
public:
    TimingElement(SequenceElements *p, const std::string &name);
    virtual ~TimingElement();
    
    virtual const std::string GetType() const override { return "timing"; }

    int GetFixedTiming() const { return mFixed; }
    void SetFixedTiming(int fixed) { mFixed = fixed; }

    const std::string &GetViews() const { return mViews; }
    void SetViews(const std::string &views) { mViews = views; }

    bool GetActive() const { return mActive; }
    void SetActive(bool active) { mActive = active; }

    std::string GetExport() const;
    std::string GetPapagayoExport(int ms) const;
    
private:
    int mFixed;
    bool mActive;
    std::string mViews;
};

class ModelElement : public Element
{
    public:
        ModelElement(SequenceElements *p, const std::string &name, bool selected);
        virtual ~ModelElement();
    

        virtual const std::string GetType() const override { return "model"; }


        bool GetSelected();
        void SetSelected(bool selected);


        virtual EffectLayer* GetEffectLayerFromExclusiveIndex(int index) override;

    
        StrandLayer* GetStrandLayer(int index, bool create = false);
        int getStrandLayerCount();
        void InitStrands(Model &cls);
        bool ShowStrands() { return mStrandsVisible;}
        void ShowStrands(bool b) { mStrandsVisible = b;}
    
        std::recursive_mutex &GetRenderLock() { return changeLock; }
        int GetWaitCount();
        void IncWaitCount();
        void DecWaitCount();
    protected:
    private:
        bool mStrandsVisible = false;
        bool mSelected;
        std::vector<StrandLayer*> mStrandLayers;
    
        std::atomic_int waitCount;


};

#endif // ELEMENT_H

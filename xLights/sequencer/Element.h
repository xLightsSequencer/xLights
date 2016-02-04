#ifndef ELEMENT_H
#define ELEMENT_H

#include "wx/wx.h"
#include <vector>
#include <atomic>
#include "wx/xml/xml.h"
#include "EffectLayer.h"
#include <string>


enum ElementType
{
    ELEMENT_TYPE_MODEL,
    ELEMENT_TYPE_TIMING
};

class NetInfoClass;
class Element;
class Model;

class ChangeListener {
public:
    virtual void IncrementChangeCount(Element *el) = 0;
};

class SequenceElements;

class Element
{
    public:
        Element(SequenceElements *p, const std::string &name, const std::string &type,bool visible,bool collapsed, bool active, bool selected);
        virtual ~Element();
    
        SequenceElements *GetSequenceElements() {return parent;}

        const std::string &GetName() const ;
        void SetName(const std::string &name);

        bool GetVisible();
        void SetVisible(bool visible);

        bool GetCollapsed();
        void SetCollapsed(bool collapsed);

        bool GetActive();
        void SetActive(bool active);

        bool GetSelected();
        void SetSelected(bool selected);

        int GetFixedTiming();
        void SetFixedTiming(int fixed);

        const std::string &GetType() const;
        void SetType(const std::string &type);

        const std::string &GetViews() const;
        void SetViews(const std::string &views);

        EffectLayer* GetEffectLayerFromExclusiveIndex(int index);
        EffectLayer* GetEffectLayer(int index);
        int GetEffectLayerCount();


        EffectLayer* AddEffectLayer();
        void RemoveEffectLayer(int index);
        EffectLayer* InsertEffectLayer(int index);

        StrandLayer* GetStrandLayer(int index, bool create = false);
        int getStrandLayerCount();
        void InitStrands(Model &cls);
        bool ShowStrands() { return mStrandsVisible;}
        void ShowStrands(bool b) { mStrandsVisible = b;}

    
        int GetIndex() const {return mIndex;}
        void SetIndex(int index) { mIndex = index;}
        int &Index() { return mIndex;}
        int Index() const { return mIndex;}

        void IncrementChangeCount(int startMs, int endMS);
        int getChangeCount() const { return changeCount; }

        void GetDirtyRange(int &startMs, int &endMs) {
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
    
        wxMutex &GetRenderLock() { return renderLock; }
        int GetWaitCount();
        void IncWaitCount();
        void DecWaitCount();
    protected:
    private:
        int mIndex;
        std::string mName;
        std::string mElementType;
        std::string mViews;
        bool mVisible;
        bool mCollapsed;
        bool mStrandsVisible = false;
        bool mActive;
        bool mSelected;
        int mFixed;
        std::vector<EffectLayer*> mEffectLayers;
        std::vector<StrandLayer*> mStrandLayers;
    
        wxMutex renderLock;
        std::atomic_int waitCount;

        SequenceElements *parent;
        ChangeListener *listener;
        volatile int changeCount = 0;
        volatile int dirtyStart = -1;
        volatile int dirtyEnd = -1;
};

#endif // ELEMENT_H

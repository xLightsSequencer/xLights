#ifndef ELEMENT_H
#define ELEMENT_H

#include "wx/wx.h"
#include <vector>
#include <atomic>
#include "wx/xml/xml.h"
#include "EffectLayer.h"


enum ElementType
{
    ELEMENT_TYPE_MODEL,
    ELEMENT_TYPE_TIMING
};

class NetInfoClass;

class ChangeLister {
public:
    virtual void IncrementChangeCount() = 0;
};

class Element
{
    public:
        Element(ChangeLister *l, wxString &name, wxString &type,bool visible,bool collapsed, bool active, bool selected);
        virtual ~Element();

        wxString GetName();
        void SetName(wxString &name);

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

        wxString GetType();
        void SetType(wxString &type);

        wxString GetViews();
        void SetViews(wxString &views);

        EffectLayer* GetEffectLayerFromExclusiveIndex(int index);
        EffectLayer* GetEffectLayer(int index);
        int GetEffectLayerCount();


        EffectLayer* AddEffectLayer();
        void RemoveEffectLayer(int index);
        EffectLayer* InsertEffectLayer(int index);

        StrandLayer* GetStrandLayer(int index, bool create = false);
        int getStrandLayerCount();
        void InitStrands(wxXmlNode *node, NetInfoClass &netInfo);
        void InitStrands(ModelClass &cls);
        bool ShowStrands() { return mStrandsVisible;}
        void ShowStrands(bool b) { mStrandsVisible = b;}

        int GetIndex();
        void SetIndex(int index);

        int Index;

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
        wxString mName;
        wxString mElementType;
        wxString mViews;
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

        ChangeLister *listener;
        volatile int changeCount = 0;
        volatile int dirtyStart = -1;
        volatile int dirtyEnd = -1;
};

#endif // ELEMENT_H

#ifndef SEQUENCEELEMENTS_H
#define SEQUENCEELEMENTS_H

#include "EffectLayer.h"
#include "Element.h"
#include "wx/wx.h"
#include <vector>
#include <set>
#include <string>
#include <mutex>
#include "wx/xml/xml.h"
#include "wx/filename.h"
#include "UndoManager.h"

class xLightsXmlFile;  // forward declaration needed due to circular dependency
class SequenceViewManager;

#define CURRENT_VIEW -1
#define MASTER_VIEW 0

class Row_Information_Struct
{
public:
    Element *element;
    int Index;
    int RowNumber;
    bool Collapsed;
    int colorIndex;
    int layerIndex;
    int strandIndex = -1;
    int nodeIndex = -1;
    std::string displayName;
    bool submodel = false;
};

struct EffectRange
{
    wxDouble StartTime;
    wxDouble EndTime;
    EffectLayer* Layer;
    int Row;
};

struct EventPlayEffectArgs
{
    Element* element;
    Effect* effect;
    bool renderEffect;
};

class wxXmlNode;
class EffectLayer;
class xLightsFrame;
class EffectManager;

class SequenceElements : public ChangeListener
{
    public:
        SequenceElements(xLightsFrame *frame);
        virtual ~SequenceElements();
        bool LoadSequencerFile(xLightsXmlFile& xml_file, const wxString& ShowDir);
        void Clear();
        void PrepareViews(xLightsXmlFile& xml_file);
        Element* AddElement(const std::string &name, const std::string &type,bool visible,bool collapsed,bool active, bool selected);
        Element* AddElement(int index, const std::string &name, const std::string &type,bool visible,bool collapsed,bool active, bool selected);
        Element* GetElement(const std::string &name);
        int GetElementIndex(const std::string &name, int view = MASTER_VIEW);
        Element* GetElement(size_t index, int view = MASTER_VIEW);
        size_t GetElementCount(int view = MASTER_VIEW);
        Row_Information_Struct* GetVisibleRowInformation(size_t index);
        Row_Information_Struct* GetVisibleRowInformationFromRow(int row_number);
        size_t GetVisibleRowInformationSize();
        Row_Information_Struct* GetRowInformation(size_t index);
        Row_Information_Struct* GetRowInformationFromRow(int row_number);
        int GetRowInformationSize();
        int GetMaxModelsDisplayed();
        int GetFirstVisibleModelRow();
        int GetTotalNumberOfModelRows();
        void SetMaxRowsDisplayed(int maxRows);
        void SetVisibilityForAllModels(bool visibility, int view = MASTER_VIEW);
        void MoveSequenceElement(int index, int dest, int view);
        void MoveElementUp(const std::string &name, int view);
        void MoveElementDown(const std::string &name, int view);
        void SetFirstVisibleModelRow(int row);
        void SetCurrentView(int view);
        void AddMissingModelsToSequence(const std::string &models, bool visible = true);
        int GetCurrentView() {return mCurrentView;}
        void SetTimingVisibility(const std::string& name);
        void PopulateView(const std::string &models, int view);
        void AddView(const std::string &viewName);
        void RemoveView(int view_index);
        void AddViewToTimings(std::vector<std::string> & timings, const std::string& name);
        void AddTimingToAllViews(const std::string& timing);
        void AddTimingToView(const std::string& timing, const std::string& name);

        void RenameModelInViews(const std::string& old_name, const std::string& new_name);

        void DeleteElement(const std::string &name);
        void DeleteElementFromView(const std::string &name, int view);
        void DeleteTimingFromView(const std::string &name, int view);
        void DeleteTimingsFromView(int view);

        void PopulateRowInformation();
        void PopulateVisibleRowInformation();

        void SetSequenceEnd(int ms);
        int GetSequenceEnd();
        void ImportLyrics(TimingElement* element, wxWindow* parent);
        void BreakdownPhrase(EffectLayer* word_layer, int start_time, int end_time, const std::string& phrase);
        void BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time, const std::string& word);

        // Selected Ranges
        int GetSelectedRangeCount();
        EffectRange* GetSelectedRange(int index);
        void AddSelectedRange(EffectRange* range);
        void DeleteSelectedRange(int index);
        void ClearSelectedRanges();

        int GetSelectedTimingRow();
        void SetSelectedTimingRow(int row);

        int GetNumberOfTimingRows();
        int GetNumberOfTimingElements();
        bool ElementExists(const std::string &elementName, int view = MASTER_VIEW);
        void RenameTimingTrack(std::string oldname, std::string newname);
        bool TimingIsPartOfView(TimingElement* timing, int view);
        std::string GetViewName(int view) const;

        void SetViewsManager(SequenceViewManager* viewsManager);
        void SetModelsNode(wxXmlNode *modelsNode);
        std::string GetViewModels(const std::string &viewName) const;
        void SetEffectsNode(wxXmlNode* effectsNode);
        wxXmlNode* GetEffectsNode() { return mEffectsNode; }

        void SortElements();
        void MoveElement(int index,int destinationIndex);

        void DeactivateAllTimingElements();
        void SetFrequency(double frequency);
        double GetFrequency();
        int GetMinPeriod();

        int SelectEffectsInRowAndTimeRange(int startRow, int endRow, int startMS,int endMS);
        int SelectVisibleEffectsInRowAndTimeRange(int startRow, int endRow, int startMS,int endMS);
        int SelectEffectsInRowAndColumnRange(int startRow, int endRow, int startCol,int endCol);
        void UnSelectAllEffects();
        void UnSelectAllElements();

        EffectLayer* GetEffectLayer(Row_Information_Struct *s);
        EffectLayer* GetEffectLayer(int row);
        EffectLayer* GetVisibleEffectLayer(int row);

        virtual void IncrementChangeCount(Element *el);
        unsigned int GetChangeCount() { return mChangeCount;}
        unsigned int GetMasterViewChangeCount() { return mMasterViewChangeCount;}

        bool HasPapagayoTiming() { return hasPapagayoTiming; }

        UndoManager& get_undo_mgr() { return undo_mgr; }


        void AddRenderDependency(const std::string &layer, const std::string &model);
        bool GetElementsToRender(std::vector<Element *> &models);
    
        bool SupportsModelBlending() const { return supportsModelBlending;}
        void SetSupportsModelBlending(bool b) { supportsModelBlending = b;}

        wxFileName &GetFileName() { return mFilename;}
        EffectManager &GetEffectManager();
        xLightsFrame *GetXLightsFrame() const { return xframe;};
    protected:
    private:
        void LoadEffects(EffectLayer *layer,
                         const std::string &type,
                         wxXmlNode *effectLayerNode,
                         const std::vector<std::string> & effectStrings,
                         const std::vector<std::string> & colorPalettes);
        static bool SortElementsByIndex(const Element *element1,const Element *element2)
        {
            return (element1->GetIndex() < element2->GetIndex());
        }
        void addTimingElement(TimingElement *elem, std::vector<Row_Information_Struct> &mRowInformation,
                              int &rowIndex, int &selectedTimingRow, int &timingRowCount, int &timingColorIndex);

        void ClearAllViews();
        std::vector<std::vector <Element*> > mAllViews;

        // A vector of all the visible elements that may not be on screen
        // because they all do not fit. The timing elements will always
        // be the first in this list.
        std::vector<Row_Information_Struct> mRowInformation;
        // A vector of the visible elements that are in shown current window view
        // Scrolling up/down changes this vector. The timing elements will always
        // be the first in this list.
        std::vector<Row_Information_Struct> mVisibleRowInformation;

        std::vector<EffectRange> mSelectedRanges;
        int mSelectedTimingRow;
        SequenceViewManager* _viewsManager;
        wxXmlNode* mModelsNode;
        wxXmlNode* mEffectsNode;
        xLightsFrame *xframe;
        double mFrequency;
        int mTimingRowCount;
        int mMaxRowsDisplayed;
        int mCurrentView;
        bool hasPapagayoTiming;
        int mSequenceEndMS;
        bool supportsModelBlending;

        wxFileName mFilename;

        // mFirstVisibleModelRow=0 is first model row not the row in Row_Information struct.
        int mFirstVisibleModelRow;
        unsigned int mChangeCount;
        unsigned int mMasterViewChangeCount;
        UndoManager undo_mgr;

        std::map<std::string, std::set<std::string>> renderDependency;
        std::set<std::string> modelsToRender;
        std::mutex renderDepLock;
};


#endif // SEQUENCEELEMENTS_H

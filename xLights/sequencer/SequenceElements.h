#ifndef SEQUENCEELEMENTS_H
#define SEQUENCEELEMENTS_H

#include "EffectLayer.h"
#include "Element.h"
#include "wx/wx.h"
#include <vector>
#include <set>
#include "wx/xml/xml.h"
#include "wx/filename.h"
#include "UndoManager.h"

class xLightsXmlFile;  // forward declaration needed due to circular dependency

#define CURRENT_VIEW -1
#define MASTER_VIEW 0

class Row_Information_Struct
{
public:
    Element *element;
    int Index;
    int RowNumber;
    bool Collapsed;
    bool Active;
    int colorIndex;
    int layerIndex;
    int strandIndex = -1;
    int nodeIndex = -1;
    wxString displayName;
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

class SequenceElements : public ChangeListener
{
    public:
        SequenceElements();
        virtual ~SequenceElements();
        bool LoadSequencerFile(xLightsXmlFile& xml_file);
        void Clear();
        Element* AddElement(wxString &name, wxString &type,bool visible,bool collapsed,bool active, bool selected);
        Element* AddElement(int index,wxString &name, wxString &type,bool visible,bool collapsed,bool active, bool selected);
        Element* GetElement(const wxString &name);
        Element* GetElement(int index, int view = MASTER_VIEW);
        int GetElementCount(int view = MASTER_VIEW);
        Row_Information_Struct* GetVisibleRowInformation(int index);
        Row_Information_Struct* GetVisibleRowInformationFromRow(int row_number);
        int GetVisibleRowInformationSize();
        Row_Information_Struct* GetRowInformation(int index);
        Row_Information_Struct* GetRowInformationFromRow(int row_number);
        int GetRowInformationSize();
        int GetMaxModelsDisplayed();
        int GetFirstVisibleModelRow();
        int GetTotalNumberOfModelRows();
        void SetMaxRowsDisplayed(int maxRows);
        void SetVisibilityForAllModels(bool visibility, int view = MASTER_VIEW);
        void MoveSequenceElement(int index, int dest, int view);
        void MoveElementUp(const wxString &name, int view);
        void MoveElementDown(const wxString &name, int view);
        void SetFirstVisibleModelRow(int row);
        void SetCurrentView(int view);
        void AddMissingModelsToSequence(const wxString &models, bool visible = true);
        int GetCurrentView() {return mCurrentView;}
        void SetTimingVisibility(const wxString& name);
        void PopulateView(const wxString &models, int view);
        void AddView(const wxString &viewName);
        void RemoveView(int view_index);
        void AddViewToTimings(wxArrayString& timings, const wxString& name);
        void AddTimingToAllViews(wxString& timing);
        void AddTimingToView(wxString& timing, const wxString& name);

        void RenameModelInViews(const wxString& old_name, const wxString& new_name);

        void DeleteElement(const wxString &name);
        void DeleteElementFromView(const wxString &name, int view);
        void DeleteTimingFromView(const wxString &name, int view);

        void PopulateRowInformation();
        void PopulateVisibleRowInformation();

        void SetSequenceEnd(int ms);
        int GetSequenceEnd();
        void ImportLyrics(Element* element, wxWindow* parent);
        void BreakdownPhrase(EffectLayer* word_layer, int start_time, int end_time, const wxString& phrase);
        void BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time, const wxString& word);

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
        bool ElementExists(wxString elementName, int view = MASTER_VIEW);
        bool TimingIsPartOfView(Element* timing, int view);
        wxString GetViewName(int view);

        void SetViewsNode(wxXmlNode* viewsNode);
        void SetModelsNode(wxXmlNode *modelsNode, xLightsFrame *frame);
        wxString GetViewModels(wxString viewName);
        void SetEffectsNode(wxXmlNode* effectsNode);
        wxXmlNode* GetEffectsNode() { return mEffectsNode; }

        void SortElements();
        void MoveElement(int index,int destinationIndex);

        void DeactivateAllTimingElements();
        void SetFrequency(double frequency);
        double GetFrequency();
        int GetMinPeriod();

        int SelectEffectsInRowAndTimeRange(int startRow, int endRow, int startMS,int endMS);
        int SelectEffectsInRowAndColumnRange(int startRow, int endRow, int startCol,int endCol);
        void UnSelectAllEffects();
        void UnSelectAllElements();

        EffectLayer* GetEffectLayer(Row_Information_Struct *s);
        EffectLayer* GetEffectLayer(int row);
        EffectLayer* GetVisibleEffectLayer(int row);

        virtual void IncrementChangeCount(Element *el);
        int GetChangeCount() { return mChangeCount;}

        bool HasPapagayoTiming() { return hasPapagayoTiming; }

        UndoManager& get_undo_mgr() { return undo_mgr; }


        void AddRenderDependency(const wxString &layer, const wxString &model);
        bool GetElementsToRender(std::vector<Element *> &models);
    
        wxFileName &GetFileName() { return mFilename;}
    protected:
    private:
        void LoadEffects(EffectLayer *layer,
                         const wxString &type,
                         wxXmlNode *effectLayerNode,
                         const std::vector<wxString> & effectStrings,
                         const std::vector<wxString> & colorPalettes);
        static bool SortElementsByIndex(const Element *element1,const Element *element2)
        {
            return (element1->GetIndex() < element2->GetIndex());
        }
        void addTimingElement(Element *elem, std::vector<Row_Information_Struct> &mRowInformation,
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
        wxXmlNode* mViewsNode;
        wxXmlNode* mModelsNode;
        wxXmlNode* mEffectsNode;
        xLightsFrame *xframe;
        double mFrequency;
        int mTimingRowCount;
        int mMaxRowsDisplayed;
        int mCurrentView;
        bool hasPapagayoTiming;
        int mSequenceEndMS;
    
        wxFileName mFilename;

        // mFirstVisibleModelRow=0 is first model row not the row in Row_Information struct.
        int mFirstVisibleModelRow;
        int mChangeCount;
        UndoManager undo_mgr;

        std::map<wxString, std::set<wxString>> renderDependency;
        std::set<wxString> modelsToRender;
        wxMutex renderDepLock;
};


#endif // SEQUENCEELEMENTS_H

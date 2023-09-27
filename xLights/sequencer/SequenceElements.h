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
class TimeLine;

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
    bool LoadSequencerFile(xLightsXmlFile& xml_file, const wxString& ShowDir, bool importing = false);
    void Clear();
    void PrepareViews(xLightsXmlFile& xml_file);
    Element* AddElement(const std::string &name, const std::string &type, bool visible, bool collapsed, bool active, bool selected, bool renderDisabled);
    Element* AddElement(int index, const std::string &name, const std::string &type, bool visible, bool collapsed, bool active, bool selected, bool renderDisabled);
    Element* GetElement(const std::string &name) const;
    int GetElementIndex(const std::string &name, int view = MASTER_VIEW);
    Element* GetElement(size_t index, int view = MASTER_VIEW) const;
    TimingElement* GetTimingElement(int n);
    TimingElement* GetTimingElement(const std::string& name);
    size_t GetElementCount(int view = MASTER_VIEW) const;
    Row_Information_Struct* GetVisibleRowInformation(size_t index);
    Row_Information_Struct* GetVisibleRowInformationFromRow(int row_number);
    size_t GetVisibleRowInformationSize();
    Row_Information_Struct* GetRowInformation(size_t index);
    Row_Information_Struct* GetRowInformationFromRow(int row_number);
    std::string UniqueElementName(const std::string& basename) const;
    int GetRowInformationSize();
    int GetMaxModelsDisplayed();
    int GetFirstVisibleModelRow();
    Effect* SelectEffectUsingDescription(std::string description);
    Effect* SelectEffectUsingElementLayerTime(std::string element, int layer, int time);
    std::list<std::string> GetAllEffectDescriptions();
    std::list<std::string> GetUniqueEffectPropertyValues(const std::string& id);
    std::list<std::string> GetAllReferencedFiles();
    std::list<std::string> GetAllUsedEffectTypes() const;
    std::list<std::string> GetAllElementNamesWithEffects();
    std::list<std::string> GetAllElementNamesWithEffectsExtended(); // this also gets submodels and strands
    int GetElementLayerCount(std::string elementName, std::list<int>* layers = nullptr);
    std::list<Effect*> GetElementLayerEffects(std::string elementName, int layer);
    bool IsValidEffect(Effect* e) const;
    bool IsValidElement(Element* e) const;
    size_t GetHiddenTimingCount() const;
    void HideAllTimingTracks(bool hide);

    int GetTotalNumberOfModelRows();
    void SetMaxRowsDisplayed(int maxRows);
    void SetVisibilityForAllModels(bool visibility, int view = MASTER_VIEW);
    void MoveSequenceElement(int index, int dest, int view);
    void MoveElementUp(const std::string &name, int view);
    void MoveElementDown(const std::string &name, int view);
    int SetFirstVisibleModelRow(int row);
    void SetCurrentView(int view);
    void AddMissingModelsToSequence(const std::string &models, bool visible = true);
    int GetCurrentView() const { return mCurrentView; }
    void SetTimingVisibility(const std::string& name);
    void PopulateView(const std::string &models, int view);
    void AddView(const std::string &viewName);
    void RemoveView(int view_index);
    void AddViewToTimings(const std::vector<std::string>& timings, const std::string& name);
    void AddTimingToAllViews(const std::string& timing);
    void AddTimingToView(const std::string& timing, const std::string& name);
    void AddTimingToCurrentView(const std::string& timing);
    int GetIndexOfModelFromModelIndex(int modelIndex);
    int GetElementIndexOfTimingFromListIndex(int timingIndex);
    int GetViewCount();
    void RenameModelInViews(const std::string& old_name, const std::string& new_name);
    TimeLine* GetTimeLine() const { return _timeLine; }
    void SetTimeLine(TimeLine* timeline) { _timeLine = timeline; }

    void DeleteElement(const std::string &name);
    void DeleteElementFromView(const std::string &name, int view);
    void DeleteTimingFromView(const std::string &name, int view);
    void DeleteTimingsFromView(int view);

    void PopulateRowInformation();
    void PopulateVisibleRowInformation();

    void SetSequenceEnd(int ms);
    int GetSequenceEnd() const;
    void ImportLyrics(TimingElement* element, wxWindow* parent);
    void BreakdownPhrase(EffectLayer* word_layer, int start_time, int end_time, const std::string& phrase, UndoManager& undo_mgr);
    void BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time, const std::string& word, UndoManager& undo_mgr);

    // Selected Ranges
    size_t GetSelectedRangeCount();
    EffectRange* GetSelectedRange(int index);
    void AddSelectedRange(EffectRange* range);
    void DeleteSelectedRange(int index);
    void ClearSelectedRanges();

    int GetSelectedTimingRow();
    void SetSelectedTimingRow(int row);

    int GetNumberOfTimingRows() const;
    int GetNumberOfTimingElements();
    int GetNumberOfActiveTimingEffects();
    bool ElementExists(const std::string &elementName, int view = MASTER_VIEW);
    void RenameTimingTrack(std::string oldname, std::string newname);
    bool TimingIsPartOfView(TimingElement* timing, int view) const;
    std::string GetViewName(int view) const;

    void SetViewsManager(SequenceViewManager* viewsManager);
    void SetModelsNode(wxXmlNode *modelsNode);
    std::string GetViewModels(const std::string &viewName) const;
    void SetEffectsNode(wxXmlNode* effectsNode);
    wxXmlNode* GetEffectsNode() const { return mEffectsNode; }

    void SortElements();
    void MoveElement(int index, int destinationIndex);

    void DeactivateAllTimingElements();
    void SetFrequency(double frequency);
    double GetFrequency();
    int GetFrameMS();
    int GetMinPeriod();

    std::vector<std::string> GetUsedColours(bool selectedOnly) const;
    int ReplaceColours(xLightsFrame* frame, const std::string& from, const std::string& to, bool selectedOnly);
    int SelectEffectsInRowAndTimeRange(int startRow, int endRow, int startMS, int endMS);
    int SelectVisibleEffectsInRowAndTimeRange(int startRow, int endRow, int startMS, int endMS);
    int SelectEffectsInRowAndColumnRange(int startRow, int endRow, int startCol, int endCol);
    void SelectAllEffects();
    void SelectAllEffectsNoTiming();
    void SelectAllEffectsInRow(int row);
    void UnSelectAllEffects();
    void SelectAllElements();
    void UnSelectAllElements();

    EffectLayer* GetEffectLayer(const Row_Information_Struct *s) const;
    EffectLayer* GetEffectLayer(int row);
    EffectLayer* GetVisibleEffectLayer(int row);

    virtual void IncrementChangeCount(Element *el);
    unsigned int GetChangeCount() const { return mChangeCount; }
    unsigned int GetMasterViewChangeCount() const { return mMasterViewChangeCount; }

    bool HasPapagayoTiming() const { return hasPapagayoTiming; }

    UndoManager& get_undo_mgr() { return undo_mgr; }

    void AddRenderDependency(const std::string &layer, const std::string &model);
    bool GetElementsToRender(std::vector<Element *> &models);

    bool SupportsModelBlending() const { return supportsModelBlending; }
    void SetSupportsModelBlending(bool b) { supportsModelBlending = b; }

    wxFileName &GetFileName() { return mFilename; }
    EffectManager &GetEffectManager();
    xLightsFrame *GetXLightsFrame() const { return xframe; };
protected:
private:
    int LoadEffects(EffectLayer *layer,
        const std::string &type,
        wxXmlNode *effectLayerNode,
        const std::vector<std::string> & effectStrings,
        const std::vector<std::string> & colorPalettes,
        bool importing = false);
    static bool SortElementsByIndex(const Element *element1, const Element *element2)
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
    SequenceViewManager* _viewsManager = nullptr;
    TimeLine* _timeLine = nullptr;
    wxXmlNode* mModelsNode = nullptr;
    wxXmlNode* mEffectsNode = nullptr;
    xLightsFrame *xframe = nullptr;
    double mFrequency;
    int mTimingRowCount = 0;
    int mMaxRowsDisplayed = 0;
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


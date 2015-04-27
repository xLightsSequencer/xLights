#ifndef SEQUENCEELEMENTS_H
#define SEQUENCEELEMENTS_H

#include "EffectLayer.h"
#include "Element.h"
#include "../xLightsXmlFile.h"
#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"

class xLightsXmlFile;  // forward declaration needed due to circular dependency

class Row_Information_Struct
{
public:
    Element *element;
    int Index;
    int RowNumber;
    bool Collapsed;
    bool Active;
    bool PartOfView;
    int colorIndex;
    int layerIndex;
    int strandIndex = -1;
    int nodeIndex = -1;
    wxString displayName;
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

class SequenceElements
{
    public:
        SequenceElements();
        virtual ~SequenceElements();
        bool LoadSequencerFile(xLightsXmlFile& xml_file);
        bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
        void Clear();
        Element* AddElement(wxString &name, wxString &type,bool visible,bool collapsed,bool active, bool selected);
        Element* AddElement(int index,wxString &name, wxString &type,bool visible,bool collapsed,bool active, bool selected);
        Element* GetElement(const wxString &name);
        Element* GetElement(int index);
        int GetElementCount();
        Row_Information_Struct* GetRowInformation(int index);
        Row_Information_Struct* GetRowInformationFromRow(int row_number);
        int GetRowInformationSize();
        int GetMaxModelsDisplayed();
        int GetFirstVisibleModelRow();
        int GetLastViewIndex();
        int GetTotalNumberOfModelRows();
        void SetMaxRowsDisplayed(int maxRows);
        void SetVisibilityForAllModels(bool visibility);
        void MoveSequenceElement(int index, int dest);
        void MoveElementUp(const wxString &name);
        void MoveElementDown(const wxString &name);
        void SetFirstVisibleModelRow(int row);

        void DeleteElement(const wxString &name);

        void PopulateRowInformation();
        void PopulateVisibleRowInformation();

        // Selected Ranges
        int GetSelectedRangeCount();
        EffectRange* GetSelectedRange(int index);
        void AddSelectedRange(EffectRange* range);
        void DeleteSelectedRange(int index);
        void ClearSelectedRanges();

        int GetSelectedTimingRow();
        void SetSelectedTimingRow(int row);

        int GetNumberOfTimingRows();
        bool ElementExists(wxString elementName);

        void SetViewsNode(wxXmlNode* viewsNode);
        void SetModelsNode(wxXmlNode *modelsNode);
        wxString GetViewModels(wxString viewName);

        void SortElements();
        void MoveElement(int index,int destinationIndex);

        void DeactivateAllTimingElements();
        void SetFrequency(double frequency);
        double GetFrequency();
        void SelectEffectsInRowAndPositionRange(int startRow, int endRow, int startX,int endX, int &FirstSelected);
        Effect* GetSelectedEffectAtRowAndPosition(int row, int x,int &index, int &selectionType);
        void UnSelectAllEffects();
        void UnSelectAllElements();
    
        EffectLayer* GetEffectLayer(Row_Information_Struct *s);
        EffectLayer* GetEffectLayer(int row);

    protected:
    private:
        void LoadEffects(EffectLayer *layer,
                         const wxString &type,
                     wxXmlNode *effectLayerNode,
                     std::vector<wxString> effectStrings,
                     std::vector<wxString> colorPalettes);
        static bool SortElementsByIndex(const Element *element1,const Element *element2)
        {
            return (element1->Index<element2->Index);
        }

        std::vector<Element*> mElements;

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
        double mFrequency;
        int mTimingRowCount;
        int mMaxRowsDisplayed;

        // mFirstVisibleModelRow=0 is first model row not the row in Row_Information struct.
        int mFirstVisibleModelRow;
};


#endif // SEQUENCEELEMENTS_H

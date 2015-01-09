#ifndef SEQUENCEELEMENTS_H
#define SEQUENCEELEMENTS_H

#include "EffectLayer.h"
#include "Element.h"

#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"


struct Row_Information_Struct
{
    Element *element;
    int Index;
    int RowNumber;
    bool Collapsed;
    bool Active;
    bool PartOfView;
    int colorIndex;
    int layerIndex;
};

class SequenceElements
{
    public:
        SequenceElements();
        virtual ~SequenceElements();
        bool LoadSequencerFile(wxString filename);
        bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
        void AddElement(wxString &name, wxString &type,bool visible,bool collapsed,bool active);
        Element* GetElement(const wxString &name);
        Row_Information_Struct* GetRowInformation(int index);
        int GetRowInformationSize();

        void SetViewsNode(wxXmlNode* viewsNode);
        wxString GetViewModels(wxString viewName);

        void SortElements();
        void MoveElement(int index,int destinationIndex);
        void PopulateRowInformation();

        void DeactivateAllTimingElements();
        void SetFrequency(double frequency);
        void SelectEffectsInRowAndPositionRange(int startRow, int endRow, int startX,int endX, int &FirstSelected);
        void UnSelectAllEffects();
    protected:
    private:
    std::vector<Element> mElements;
    std::vector<Row_Information_Struct> mRowInformation;
    bool ElementExists(wxString elementName);
    wxXmlNode* mViewsNode;
    double mFrequency;

    static bool SortElementsByIndex(const Element &element1,const Element &element2)
    {
        return (element1.Index<element2.Index);
    }


};


#endif // SEQUENCEELEMENTS_H

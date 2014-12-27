#ifndef SEQUENCEELEMENTS_H
#define SEQUENCEELEMENTS_H

#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"
#include "ElementEffects.h"
#include "Element.h"

struct Row_Information_Struct
{
    int Index;
    wxString ElementName;
    wxString ElementType;
    int RowNumber;
    bool Visible;
    bool Collasped;
};


class SequenceElements
{
    public:
        SequenceElements();
        virtual ~SequenceElements();

        void LoadXMLelements(wxString ModelView,wxXmlNode* effect);
        bool SeqLoadXlightsFile(const wxString& filename, bool ChooseModels);
        void AddElement(wxString name, int type,bool visible);
        Element* GetElement(wxString name);

        Row_Information_Struct* GetRowInformation(int index);
        int GetRowInformationSize();

        void SortElements();
        void MoveElement(int index,int destinationIndex);

    protected:
    private:
    std::vector<Element> mElements;
    std::vector<Row_Information_Struct> mRowInformation;
    wxXmlNode * mDisplayElements;
    wxXmlNode * mElementEffects;

    static bool SortElementsByIndex(const Element &element1,const Element &element2)
    {
        return (element1.Index<element2.Index);
    }


};


#endif // SEQUENCEELEMENTS_H

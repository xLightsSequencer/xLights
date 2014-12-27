#ifndef ELEMENT_H
#define ELEMENT_H

#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"
#include "ElementEffects.h"

class Element
{
    public:
        Element(wxString name, int type,bool visible);
        virtual ~Element();

        wxString GetName();
        void SetName(wxString name);

        bool GetVisible();
        void SetVisible(bool visible);

        int GetType();
        void SetType(int type);

        ElementEffects* GetElementEffects();
        void SortElementEffects();

        int GetIndex();
        void SetIndex(int index);

        int Index;

    protected:
    private:
        int mElementType;
        int mIndex;
        wxString mName;
        bool mVisible;
        ElementEffects mElementEffects;

};

#endif // ELEMENT_H

#ifndef ELEMENT_H
#define ELEMENT_H

#include "wx/wx.h"
#include <vector>
#include "wx/xml/xml.h"
#include "ElementEffects.h"

enum ElementType
{
    ELEMENT_TYPE_MODEL,
    ELEMENT_TYPE_VIEW,
    ELEMENT_TYPE_TIMING
};

class Element
{
    public:
        Element(wxString &name, wxString &type,bool visible,bool collapsed, bool active);
        virtual ~Element();

        wxString GetName();
        void SetName(wxString &name);

        bool GetVisible();
        void SetVisible(bool visible);

        bool GetCollapsed();
        void SetCollapsed(bool collapsed);

        bool GetActive();
        void SetActive(bool active);

        wxString GetType();
        void SetType(wxString &type);

        ElementEffects* GetElementEffects();
        void SortElementEffects();


        int GetIndex();
        void SetIndex(int index);

        int Index;

        void AddEffect(int id,wxString &effect, int effectIndex, double startTime,double endTime, bool Protected);


    protected:
    private:
        int mIndex;
        wxString mName;
        wxString mElementType;
        bool mVisible;
        bool mCollapsed;
        bool mActive;
        ElementEffects mElementEffects;

};

#endif // ELEMENT_H

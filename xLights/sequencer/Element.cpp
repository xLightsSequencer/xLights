#include "Element.h"

Element::Element(wxString &name, wxString &type,bool visible,bool collapsed, bool active)
:mElementEffects()
{
    mName = name;
    mElementType = type;
    mVisible = visible;
    mCollapsed = collapsed;
    mActive = active;

}

Element::~Element()
{
}


wxString Element::GetName()
{
    return mName;
}

void Element::SetName(wxString &name)
{
    mName = name;
}

bool Element::GetVisible()
{
    return mVisible;
}

void Element::SetVisible(bool visible)
{
    mVisible = visible;
}

bool Element::GetCollapsed()
{
    return mCollapsed;
}

void Element::SetActive(bool active)
{
    mActive = active;
}

bool Element::GetActive()
{
    return mActive;
}

void Element::SetCollapsed(bool collapsed)
{
    mCollapsed = collapsed;
}

wxString Element::GetType()
{
    return mElementType;
}

void Element::SetType(wxString &type)
{
    mElementType = type;
}

int Element::GetIndex()
{
    return mIndex;
}

void Element::SetIndex(int index)
{
    mIndex = index;
}

ElementEffects* Element::GetElementEffects()
{
    return &mElementEffects;
}

void Element::SortElementEffects()
{
    mElementEffects.Sort();
}

void Element::AddEffect(int id,wxString &effect, int effectIndex,double startTime,double endTime, bool Protected)
{
    mElementEffects.AddEffect(id,effect,effectIndex,startTime,endTime,Protected);
}


#include "Element.h"

Element::Element(wxString name, int type,bool visible)
:mElementEffects()
{
    mName = name;
    mElementType = type;
    mVisible = visible;
}

Element::~Element()
{
}


wxString Element::GetName()
{
    return mName;
}

void Element::SetName(wxString name)
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

int Element::GetType()
{
    return mElementType;
}

void Element::SetType(int type)
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

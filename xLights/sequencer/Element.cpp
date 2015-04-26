#include "Element.h"
#include "ModelClass.h"
Element::Element(wxString &name, wxString &type,bool visible,bool collapsed, bool active, bool selected)
:   mEffectLayers(),
    mName(name),
    mElementType(type),
    mVisible(visible),
    mCollapsed(collapsed),
    mActive(active),
    mSelected(selected),
    mFixed(0)
{
}

Element::~Element()
{
    for (int x = 0; x < mEffectLayers.size(); x++) {
        delete mEffectLayers[x];
    }
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

void Element::SetSelected(bool active)
{
    mSelected = active;
}

bool Element::GetSelected()
{
    return mSelected;
}

int Element::GetFixedTiming()
{
    return mFixed;
}

void Element::SetFixedTiming(int fixed)
{
    mFixed = fixed;
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

EffectLayer* Element::GetEffectLayer(int index)
{
    if( index >= mEffectLayers.size() ) return nullptr;
    return mEffectLayers[index];
}

EffectLayer* Element::AddEffectLayer()
{
    EffectLayer* new_layer = new EffectLayer(this);
    mEffectLayers.push_back(new_layer);
    IncrementChangeCount();
    return new_layer;
}

void Element::RemoveEffectLayer(int index)
{
    EffectLayer *l = GetEffectLayer(index);
    mEffectLayers.erase(mEffectLayers.begin()+index);
    delete l;
    IncrementChangeCount();
}

int Element::GetEffectLayerCount()
{
    return mEffectLayers.size();
}


void Element::IncrementChangeCount()
{
    changeCount++;
}

void Element::InitStrands(wxXmlNode *node) {
    if (node == NULL) {
        return;
    }
    ModelClass model;
    model.SetFromXml(node);
    int ns = model.GetNumStrands();
    for (int x = 0; x < ns; x++) {
        GetStrandLayer(x, true)->InitFromModel(model);
    }
}

StrandLayer* Element::GetStrandLayer(int index, bool create) {
    while (create && index >= mStrandLayers.size()) {
        StrandLayer* new_layer = new StrandLayer(this, mStrandLayers.size());
        mStrandLayers.push_back(new_layer);
        IncrementChangeCount();
    }
    if (index >= mStrandLayers.size()) {
        return nullptr;
    }
    return mStrandLayers[index];
}
int Element::getStrandLayerCount() {
    return mStrandLayers.size();
}



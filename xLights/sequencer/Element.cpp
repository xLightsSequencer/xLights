#include "Element.h"
#include "ModelClass.h"
Element::Element(ChangeLister *l, wxString &name, wxString &type,bool visible,bool collapsed, bool active, bool selected)
:   mEffectLayers(),
    mName(name),
    mElementType(type),
    mVisible(visible),
    mCollapsed(collapsed),
    mActive(active),
    mSelected(selected),
    mFixed(0),
    listener(l),
    waitCount(0)
{
}

Element::~Element()
{
    for (int x = 0; x < mEffectLayers.size(); x++) {
        delete mEffectLayers[x];
    }
    for (int x = 0; x < mStrandLayers.size(); x++) {
        delete mStrandLayers[x];
    }
}


int Element::GetWaitCount() {
    return waitCount;
}
void Element::IncWaitCount() {
    waitCount++;
}
void Element::DecWaitCount() {
    waitCount--;
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

wxString Element::GetViews()
{
    return mViews;
}

void Element::SetViews(wxString &views)
{
    mViews = views;
}

int Element::GetIndex()
{
    return mIndex;
}

void Element::SetIndex(int index)
{
    mIndex = index;
}

EffectLayer* Element::GetEffectLayerFromExclusiveIndex(int index)
{
    for( int i = 0; i < mEffectLayers.size(); i++ )
    {
        if( mEffectLayers[i]->GetIndex() == index )
            return mEffectLayers[i];
    }

    for( int j = 0; j < mStrandLayers.size(); j++ )
    {
        if( mStrandLayers[j]->GetIndex() == index )
            return mStrandLayers[j];

        for( int k = 0; k < mStrandLayers[j]->GetNodeLayerCount(); k++ )
        {
            if( mStrandLayers[j]->GetNodeLayer(k,false)->GetIndex() == index )
                return mStrandLayers[j]->GetNodeLayer(k,false);
        }
    }

    return nullptr;
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
    IncrementChangeCount(-1, -1);
    return new_layer;
}

EffectLayer* Element::InsertEffectLayer(int index)
{
    EffectLayer* new_layer = new EffectLayer(this);
    mEffectLayers.insert(mEffectLayers.begin()+index, new_layer);
    IncrementChangeCount(-1, -1);
    return new_layer;
}

void Element::RemoveEffectLayer(int index)
{
    EffectLayer *l = GetEffectLayer(index);
    mEffectLayers.erase(mEffectLayers.begin()+index);
    delete l;
    IncrementChangeCount(-1, -1);
}

int Element::GetEffectLayerCount()
{
    return mEffectLayers.size();
}

void Element::IncrementChangeCount(int sms, int ems)
{
    SetDirtyRange(sms, ems);
    changeCount++;

    listener->IncrementChangeCount();
}
void Element::InitStrands(ModelClass &model) {
    if (model.GetDisplayAs() == "WholeHouse") {
        //no strands for a whole house model
        return;
    }
    int ns = model.GetNumStrands();
    for (int x = 0; x < ns; x++) {
        GetStrandLayer(x, true)->InitFromModel(model);
    }
}
void Element::InitStrands(wxXmlNode *node, NetInfoClass &netInfo) {
    if (node == NULL) {
        return;
    }
    ModelClass model;
    model.SetFromXml(node, netInfo);
    InitStrands(model);
}

StrandLayer* Element::GetStrandLayer(int index, bool create) {
    while (create && index >= mStrandLayers.size()) {
        StrandLayer* new_layer = new StrandLayer(this, mStrandLayers.size());
        mStrandLayers.push_back(new_layer);
        IncrementChangeCount(-1, -1);
    }
    if (index >= mStrandLayers.size()) {
        return nullptr;
    }
    return mStrandLayers[index];
}
int Element::getStrandLayerCount() {
    return mStrandLayers.size();
}



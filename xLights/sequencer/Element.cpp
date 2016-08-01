#include "Element.h"
#include "../models/Model.h"
#include <list>

Element::Element(SequenceElements *p, const std::string &name) :
mEffectLayers(),
mName(name),
mVisible(true),
listener((ChangeListener *)p),
mCollapsed(false),
parent(p)
{
    
}
Element::~Element() {
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        delete mEffectLayers[x];
    }
    mEffectLayers.clear();
}
const std::string &Element::GetName() const
{
    return mName;
}
void Element::SetName(const std::string &name)
{
    mName = name;
    changeCount++;
    listener->IncrementChangeCount(this);
}
EffectLayer* Element::GetEffectLayerFromExclusiveIndex(int index)
{
    for( size_t i = 0; i < mEffectLayers.size(); i++ )
    {
        if( mEffectLayers[i]->GetIndex() == index )
            return mEffectLayers[i];
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
    std::unique_lock<std::recursive_mutex> lock(changeLock);
    EffectLayer* new_layer = new EffectLayer(this);
    mEffectLayers.push_back(new_layer);
    IncrementChangeCount(-1, -1);
    return new_layer;
}

EffectLayer* Element::InsertEffectLayer(int index)
{
    std::unique_lock<std::recursive_mutex> lock(changeLock);
    EffectLayer* new_layer = new EffectLayer(this);
    mEffectLayers.insert(mEffectLayers.begin()+index, new_layer);
    IncrementChangeCount(-1, -1);
    return new_layer;
}

void Element::RemoveEffectLayer(int index)
{
    std::unique_lock<std::recursive_mutex> lock(changeLock);
    EffectLayer *l = GetEffectLayer(index);
    mEffectLayers.erase(mEffectLayers.begin()+index);
    delete l;
    IncrementChangeCount(-1, -1);
}

size_t Element::GetEffectLayerCount()
{
    return mEffectLayers.size();
}

void Element::IncrementChangeCount(int sms, int ems)
{
    SetDirtyRange(sms, ems);
    changeCount++;
    
    listener->IncrementChangeCount(this);
}



TimingElement::TimingElement(SequenceElements *p, const std::string &name)
: Element(p, name),
    mFixed(0)
{
    
}
TimingElement::~TimingElement() {
}



ModelElement::ModelElement(SequenceElements *l, const std::string &name, bool selected)
:   Element(l, name),
    mSelected(selected),
    waitCount(0)
{
}

ModelElement::~ModelElement()
{
    //make sure none of the render threads are rendering this model
    std::unique_lock<std::recursive_mutex> lock(changeLock);
    while (waitCount > 0) {
        lock.unlock();
        wxSleep(1);
        lock.lock();
    }
    
    for (size_t x = 0; x < mStrandLayers.size(); x++) {
        delete mStrandLayers[x];
    }
}


int ModelElement::GetWaitCount() {
    return waitCount;
}
void ModelElement::IncWaitCount() {
    waitCount++;
}
void ModelElement::DecWaitCount() {
    waitCount--;
}



std::list<Effect*> GetEffectsBetween(EffectLayer* layer, int start, int end)
{
    std::list<Effect*> res;

    for (int i = 0; i < layer->GetEffectCount(); i++)
    {
        if (layer->GetEffect(i)->GetStartTimeMS() >= start && layer->GetEffect(i)->GetEndTimeMS() <= end)
        {
            res.push_back(layer->GetEffect(i));
        }
    }

    return res;
}

EffectLayer* ModelElement::GetEffectLayerFromExclusiveIndex(int index) {
    EffectLayer *l = Element::GetEffectLayerFromExclusiveIndex(index);
    if (l != nullptr) {
        return l;
    }
    for( size_t j = 0; j < mStrandLayers.size(); j++ )
    {
        if( mStrandLayers[j]->GetIndex() == index )
        return mStrandLayers[j];
        
        for( size_t k = 0; k < mStrandLayers[j]->GetNodeLayerCount(); k++ )
        {
            if( mStrandLayers[j]->GetNodeLayer(k,false)->GetIndex() == index )
            return mStrandLayers[j]->GetNodeLayer(k,false);
        }
    }
    return nullptr;
}
std::string  TimingElement::GetPapagayoExport(int fps) const
{
    if (mEffectLayers.size() != 3) return "";
    int ms = 1000 / fps;
    auto l = mEffectLayers.begin();
    EffectLayer* l1 = *l++;
    int l1count = l1->GetEffectCount();
    EffectLayer* l2 = *l++;
    EffectLayer* l3 = *l;

    std::string res = "lipsync version 1\nxlights.wav\n";
    wxString samppersec = wxString::Format("%d", fps);
    res += std::string(samppersec.c_str()) + "\n";
    int end = mEffectLayers.front()->GetEffect(l1count - 1)->GetEndTimeMS() / ms;
    res += std::string(wxString::Format("%d", end).c_str()) + "\n";
    res += "1\n"; // only ever one voice
    res += "\tVoice 1\n\t";
    for (int i = 0; i < l1count; i++)
    {
        res += l1->GetEffect(i)->GetEffectName() + "|";
    }
    res += "\n";
    res += "\t" + std::string(wxString::Format("%d", l1count).c_str()) + "\n";
    for (int i = 0; i < l1count; i++) // each line
    {
        res += "\t\t" + l1->GetEffect(i)->GetEffectName() + "\n";
        res += "\t\t" + std::string(wxString::Format("%d", l1->GetEffect(i)->GetStartTimeMS() / ms).c_str()) + "\n";
        res += "\t\t" + std::string(wxString::Format("%d", l1->GetEffect(i)->GetEndTimeMS() / ms).c_str()) + "\n";
        std::list<Effect*> words = GetEffectsBetween(l2, l1->GetEffect(i)->GetStartTimeMS(), l1->GetEffect(i)->GetEndTimeMS());
        res += "\t\t" + std::string(wxString::Format("%d", words.size()).c_str()) + "\n";
        for (auto w = words.begin(); w != words.end(); w++)
        {
            std::list<Effect*> ph = GetEffectsBetween(l3, (*w)->GetStartTimeMS(), (*w)->GetEndTimeMS());
            res += "\t\t\t" + (*w)->GetEffectName() + " " + std::string(wxString::Format("%d", (*w)->GetStartTimeMS() / ms).c_str()) + " " + std::string(wxString::Format("%d", (*w)->GetEndTimeMS() / ms).c_str()) + " " + std::string(wxString::Format("%d", ph.size()).c_str()) + "\n";
            for (auto p = ph.begin(); p != ph.end(); p++)
            {
                res += "\t\t\t\t" + std::string(wxString::Format("%d", (*p)->GetStartTimeMS() / ms).c_str()) + " " + (*p)->GetEffectName() + "\n";
            }
        }
    }

    return res;
}

std::string TimingElement::GetExport() const
{
    std::string res = "";

    for (auto l = mEffectLayers.begin(); l != mEffectLayers.end(); ++l)
    {
        res += "   <EffectLayer>\n";
        for (int i = 0; i < (*l)->GetEffectCount(); i++)
        {
            res += "      <Effect label=\""+(*l)->GetEffect(i)->GetEffectName()+
                              "\" starttime=\"" + std::string(wxString::Format("%d",(*l)->GetEffect(i)->GetStartTimeMS()))+
                              "\" endtime=\"" + std::string(wxString::Format("%d", (*l)->GetEffect(i)->GetEndTimeMS())) + 
                              "\" />\n";
        }
        res += "   </EffectLayer>\n";
    }

    return res;
}


void ModelElement::SetSelected(bool active)
{
    mSelected = active;
}

bool ModelElement::GetSelected()
{
    return mSelected;
}

void ModelElement::InitStrands(Model &model) {
    if (model.GetDisplayAs() == "WholeHouse"
        || model.GetDisplayAs() == "ModelGroup") {
        //no strands for a whole house model
        return;
    }
    int ns = model.GetNumStrands();
    for (int x = 0; x < ns; x++) {
        GetStrandLayer(x, true)->InitFromModel(model);
    }
}

StrandLayer* ModelElement::GetStrandLayer(int index, bool create) {
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
int ModelElement::getStrandLayerCount() {
    return mStrandLayers.size();
}



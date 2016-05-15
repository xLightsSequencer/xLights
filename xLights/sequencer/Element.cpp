#include "Element.h"
#include "../models/Model.h"
#include <list>

Element::Element(SequenceElements *l, const std::string &name, const std::string &type,bool visible,bool collapsed, bool active, bool selected)
:   mEffectLayers(),
    mName(name),
    mElementType(type),
    mVisible(visible),
    mCollapsed(collapsed),
    mActive(active),
    mSelected(selected),
    mFixed(0),
    waitCount(0),
    parent(l)
{
    listener = (ChangeListener*)l;
}

Element::~Element()
{
    //make sure none of the render threads are rendering this model
    std::unique_lock<std::recursive_mutex> lock(renderLock);
    while (waitCount > 0) {
        lock.unlock();
        wxSleep(1);
        lock.lock();
    }
    
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        delete mEffectLayers[x];
    }
    for (size_t x = 0; x < mStrandLayers.size(); x++) {
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


const std::string &Element::GetName() const
{
    return mName;
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

std::string  Element::GetPapagayoExport(int fps) const
{
    if (mEffectLayers.size() != 3) return "";
    int ms = 1000 / fps;
    auto l = mEffectLayers.begin();
    EffectLayer* l1 = *l++;
    int l1count = l1->GetEffectCount();
    EffectLayer* l2 = *l++;
    int l2count = l2->GetEffectCount();
    EffectLayer* l3 = *l;
    int l3count = l3->GetEffectCount();

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

std::string Element::GetExport() const
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

void Element::SetName(const std::string &name)
{
    mName = name;
    changeCount++;
    listener->IncrementChangeCount(this);
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

const std::string &Element::GetType() const
{
    return mElementType;
}

void Element::SetType(const std::string &type)
{
    mElementType = type;
}

const std::string &Element::GetViews() const
{
    return mViews;
}

void Element::SetViews(const std::string &views)
{
    mViews = views;
}

EffectLayer* Element::GetEffectLayerFromExclusiveIndex(int index)
{
    for( size_t i = 0; i < mEffectLayers.size(); i++ )
    {
        if( mEffectLayers[i]->GetIndex() == index )
            return mEffectLayers[i];
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

    listener->IncrementChangeCount(this);
}
void Element::InitStrands(Model &model) {
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



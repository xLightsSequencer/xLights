/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "Element.h"
#include "../models/Model.h"
#include <list>
#include <numeric>
#include "UtilFunctions.h"
#include <log4cpp/Category.hh>
#include "SequenceElements.h"
#include "xLightsMain.h"

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
    while (!mLayersToDelete.empty()) {
        delete *mLayersToDelete.begin();
        mLayersToDelete.pop_front();
    }
    mEffectLayers.clear();
    listener = nullptr; // dont delete it ... as it was a passed in sequence elements pointer ... but explicitly forget it so someone doesnt delete it later
}

std::string Element::GetTypeDescription() const
{
    switch (GetType()) {
    case ElementType::ELEMENT_TYPE_MODEL: {
        Model* m = GetSequenceElements()->GetXLightsFrame()->AllModels[GetModelName()];
        if (m != nullptr) {
            if (m->GetDisplayAs() == "ModelGroup") {
                return "Model Group";
            }
            return "Model";
        }
        return "Model/Model Group";
    }
    case ElementType::ELEMENT_TYPE_SUBMODEL:
        return "Submodel";
    case ElementType::ELEMENT_TYPE_STRAND:
        return "Strand";
    case ElementType::ELEMENT_TYPE_TIMING:
        return "Timing";
    }

    return "UNKNOWN";
}

void Element::CleanupAfterRender() {
    for (auto &a : mEffectLayers) {
        a->CleanupAfterRender();
    }
    std::unique_lock<std::recursive_timed_mutex> lock(changeLock);
    while (!mLayersToDelete.empty()) {
        delete *mLayersToDelete.begin();
        mLayersToDelete.pop_front();
    }
}

std::string Element::GetFullName() const {
    return mName;
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

const std::string &Element::GetModelName() const {
    return mName;
}

bool Element::HasEffects() const
{
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        if (mEffectLayers[x]->GetEffectCount() > 0) {
            return true;
        }
    }

    return false;
}

int Element::GetEffectCount() const {

    return std::accumulate(
        mEffectLayers.begin(), 
        mEffectLayers.end(), 0, 
        [](int i, EffectLayer* l) {
        return l->GetEffectCount() + i;
        });
}

bool TimingElement::HasLyrics(int layer) const
{
    if (mEffectLayers.size() > layer) {
        for (const auto& it : mEffectLayers[layer]->GetAllEffects()) {
            if (it->GetEffectName() != "") return true;
        }
    }
    return false;
}

std::vector<int> Element::GetLayersWithEffectsByTime(int startMs, int endMS) const {
	std::vector<int> returnList;
	for (size_t x = 0; x < mEffectLayers.size(); x++) {
		if (mEffectLayers[x]->GetAllEffectsByTime(startMs, endMS).size() > 0) {
			returnList.push_back(x);
		}
	}

	return returnList;
}

int Element::GetSelectedEffectCount() const {
    int count = 0;
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        count += mEffectLayers[x]->GetSelectedEffectCount();
    }
    return count;
}

int Element::GetFirstSelectedEffectStartMS() const
{
    long startMS = -1;
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        auto start = mEffectLayers[x]->GetFirstSelectedEffectStartMS();
        if (start != -1 && (startMS == -1 || start < startMS))
        {
            startMS = start;
        }
    }
    return startMS;
}

int Element::GetLastSelectedEffectEndMS() const
{
    long endMS = -1;
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        auto end = mEffectLayers[x]->GetLastSelectedEffectEndMS();
        if (end != -1 && (endMS == -1 || end > endMS))
        {
            endMS = end;
        }
    }
    return endMS;
}

void Element::SetRenderDisabled(bool rd)
{
    _renderDisabled = rd;
}

EffectLayer* Element::GetEffectLayerFromExclusiveIndex(int index)
{
    for (size_t i = 0; i < mEffectLayers.size(); i++)
    {
        if ( mEffectLayers[i]->GetIndex() == index)
            return mEffectLayers[i];
    }
    
    return nullptr;
}

EffectLayer* Element::GetEffectLayer(int index) const
{
    if( index >= mEffectLayers.size() ) return nullptr;
    return mEffectLayers[index];
}

int Element::GetLayerNumberFromIndex(int index)
{
    int i = 0;
    for (auto it = mEffectLayers.begin(); it != mEffectLayers.end(); ++it)
    {
        if ((*it)->GetIndex() == index)
        {
            return i + 1;
        }
        i++;
    }

    return -1;
}

EffectLayer* Element::AddEffectLayer()
{
    EffectLayer* new_layer = AddEffectLayerInternal();
    IncrementChangeCount(-1, -1);
    return new_layer;
}

EffectLayer* Element::AddEffectLayerInternal()
{
    // try for 500ms to get the lock ... if i cant get it ... abort rendering and try again
    std::unique_lock<std::recursive_timed_mutex> lock(changeLock, std::defer_lock_t());
    if (!lock.try_lock_for(std::chrono::milliseconds(500)))
    {
        GetSequenceElements()->GetXLightsFrame()->AbortRender();
        lock.lock();
    }

    EffectLayer* new_layer = new EffectLayer(this);
    mEffectLayers.push_back(new_layer);
    return new_layer;
}

EffectLayer* Element::InsertEffectLayer(int index)
{
    // try for 500ms to get the lock ... if i cant get it ... abort rendering and try again
    std::unique_lock<std::recursive_timed_mutex> lock(changeLock, std::defer_lock_t());
    if (!lock.try_lock_for(std::chrono::milliseconds(500)))
    {
        GetSequenceElements()->GetXLightsFrame()->AbortRender();
        lock.lock();
    }

    EffectLayer* new_layer = new EffectLayer(this);
    mEffectLayers.insert(mEffectLayers.begin()+index, new_layer);
    IncrementChangeCount(-1, -1);
    return new_layer;
}

bool Element::operator==(const Element& e) const
{
    return e.GetType() == GetType() && e.GetName() == GetName();
}

bool Element::operator<(const Element& e) const
{
    auto myType = GetType();
    auto theirType = e.GetType();

    if (myType == theirType)
    {
        return NumberAwareStringCompare(GetName(), e.GetName()) == -1;
    }
    else
    {
        if (myType == ElementType::ELEMENT_TYPE_TIMING) return true;
    }

    return false;
}

void Element::RemoveEffectLayer(int index)
{
    // try for 500ms to get the lock ... if i cant get it ... abort rendering and try again
    std::unique_lock<std::recursive_timed_mutex> lock(changeLock, std::defer_lock_t());
    if (!lock.try_lock_for(std::chrono::milliseconds(500)))
    {
        GetSequenceElements()->GetXLightsFrame()->AbortRender();
        lock.lock();
    }

    EffectLayer *l = GetEffectLayer(index);
    mEffectLayers.erase(mEffectLayers.begin()+index);
    mLayersToDelete.push_back(l);
    IncrementChangeCount(-1, -1);
}

size_t Element::GetEffectLayerCount() const
{
    return mEffectLayers.size();
}

void Element::IncrementChangeCount(int sms, int ems)
{
    SetDirtyRange(sms, ems);
    changeCount++;
    
    listener->IncrementChangeCount(this);
}

void SubModelElement::IncrementChangeCount(int startMs, int endMS) {
    GetModelElement()->IncrementChangeCount(startMs, endMS);
}

bool SubModelElement::HasEffects() const
{
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        if (mEffectLayers[x]->GetEffectCount() > 0) {
            return true;
        }
    }
    return false;
}

TimingElement::TimingElement(SequenceElements *p, const std::string &name)
: Element(p, name),
    mFixed(0), mActive(false)
{ }

TimingElement::~TimingElement() {
}

void TimingElement::Unfix()
{
    mFixed = 0;
}

SubModelElement::SubModelElement(ModelElement *p, const std::string &name)
: Element(p->GetSequenceElements(), name), mParentModel(p)
{
    AddEffectLayerInternal();
}

SubModelElement::~SubModelElement() {
    
}


const std::string &SubModelElement::GetModelName() const {
    return mParentModel->GetModelName();
}

std::string SubModelElement::GetFullName() const {
    return GetModelName() + "/" + GetName();
}

StrandElement::StrandElement(ModelElement *p, int strand)
: SubModelElement(p, ""),
  mStrand(strand),
  mShowNodes(false)
{
}

StrandElement::~StrandElement() {
    for (size_t x = 0; x < mNodeLayers.size(); x++) {
        delete mNodeLayers[x];
    }
    mNodeLayers.clear();
}

std::string StrandElement::GetFullName() const {
    return GetModelName() + "/" + GetStrandName();
}
std::string StrandElement::GetStrandName() const {
    static const std::string STRAND = "Strand ";
    if (GetName() == "")
        return STRAND + std::to_string(mStrand + 1);
    return GetName();
}
void StrandElement::CleanupAfterRender() {
    for (auto &a : mNodeLayers) {
        a->CleanupAfterRender();
    }
    Element::CleanupAfterRender();
}

NodeLayer *StrandElement::GetNodeLayer(int n, bool create) {
    while (create && n >= mNodeLayers.size()) {
        mNodeLayers.push_back(new NodeLayer(this));
    }
    if (n < mNodeLayers.size()) {
        return mNodeLayers[n];
    }
    return nullptr;
}

NodeLayer* StrandElement::GetNodeLayer(int n) const
{
    if (n < mNodeLayers.size()) {
        return mNodeLayers[n];
    }
    return nullptr;
}

int StrandElement::GetNodeNumber(NodeLayer* nl)
{
    int nn = -1;
    for (const auto& n : mNodeLayers) {
        nn++;
        if (n == nl)
            return nn;
    }
    return -1;
}

void StrandElement::InitFromModel(Model &model) {
    int nc = model.GetStrandLength(mStrand);
    mName = model.GetStrandName(mStrand);
    for (int x = 0; x < mNodeLayers.size(); x++) {
        mNodeLayers[x]->SetName(model.GetNodeName(x));
    }
    while (mNodeLayers.size() < nc) {
        NodeLayer *nl = new NodeLayer(this, model.GetNodeName(mNodeLayers.size()));
        mNodeLayers.push_back(nl);
    }
}

EffectLayer* StrandElement::GetEffectLayerFromExclusiveIndex(int index) {
    EffectLayer *l = SubModelElement::GetEffectLayerFromExclusiveIndex(index);
    if (l != nullptr) {
        return l;
    }
    for( size_t j = 0; j < mNodeLayers.size(); j++ ) {
        if (index == mNodeLayers[j]->GetIndex()) {
            return mNodeLayers[j];
        }
    }
    return nullptr;
}

NodeLayer* StrandElement::GetNodeEffectLayer(int index) const
{
    if ( index < GetNodeLayerCount())
        return GetNodeLayer(index);
    return nullptr;
}

bool StrandElement::HasEffects() const
{
    for (size_t x = 0; x < mEffectLayers.size(); x++) {
        if (mEffectLayers[x]->GetEffectCount() > 0) {
            return true;
        }
    }

    for (size_t x = 0; x < GetNodeLayerCount(); ++x)
    {
        NodeLayer* nl = GetNodeLayer(x);

        if (nl != nullptr && nl->HasEffects()) return true;
    }

    return false;
}

int StrandElement::GetEffectCount() const {

    int sum = std::accumulate(
        mEffectLayers.begin(), 
        mEffectLayers.end(), 0, 
        [](int i, EffectLayer* l) {
        return l->GetEffectCount() + i;
        });

    int nodesum = std::accumulate(
        mNodeLayers.begin(), 
        mNodeLayers.end(), 0, 
        [](int i, NodeLayer* nl) {
            if (nl != nullptr) {
                return nl->GetEffectCount() + i;
            }
            return i;
        });

    return nodesum + sum;
}

ModelElement::ModelElement(SequenceElements *l, const std::string &name, bool selected)
:   Element(l, name),
    mSelected(selected),
    waitCount(0)
{
}

ModelElement::ModelElement(const std::string &name)
    : Element(nullptr, name),
    waitCount(0)
{
}

ModelElement::~ModelElement()
{
    //make sure none of the render threads are rendering this model
    std::unique_lock<std::recursive_timed_mutex> lock(changeLock);
    while (waitCount > 0) {
        lock.unlock();
        wxSleep(1);
        lock.lock();
    }
    for (size_t x = 0; x < mStrands.size(); x++) {
        delete mStrands[x];
    }
    mStrands.clear();
    for (size_t x = 0; x < mSubModels.size(); x++) {
        delete mSubModels[x];
    }
    mSubModels.clear();
}

// remove but dont delete all submodels ... used for submodel reordering
void ModelElement::RemoveAllSubModels()
{
    mSubModels.clear();
}

void ModelElement::CleanupAfterRender() {
    for (auto &a : mStrands) {
        a->CleanupAfterRender();
    }
    for (auto &a : mSubModels) {
        a->CleanupAfterRender();
    }
    Element::CleanupAfterRender();
}

NodeLayer* ModelElement::GetNodeEffectLayer(int index) const
{
    int startStrand = 0;
    for (int s = 0; s < GetStrandCount(); s++)
    {
        if (index < startStrand + GetStrand(s)->GetNodeLayerCount())
        {
            return GetStrand(s)->GetNodeEffectLayer(index - startStrand);
        }
        startStrand += GetStrand(s)->GetNodeLayerCount();
    }
    return nullptr;
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
    for( size_t j = 0; j < mSubModels.size(); j++ ) {
        l = mSubModels[j]->GetEffectLayerFromExclusiveIndex(index);
        if (l != nullptr) {
            return l;
        }
    }
    for( size_t j = 0; j < mStrands.size(); j++ ) {
        l = mStrands[j]->GetEffectLayerFromExclusiveIndex(index);
        if (l != nullptr) {
            return l;
        }
    }
    return nullptr;
}

bool ModelElement::HasEffects() const
{
    for (size_t x = 0; x < mEffectLayers.size(); ++x) {
        if (mEffectLayers[x]->GetEffectCount() > 0) {
            return true;
        }
    }
    
    for (size_t x = 0; x < GetStrandCount(); ++x)
    {
        StrandElement* se = GetStrand(x);

        if (se != nullptr && se->HasEffects()) return true;
    }

    for (size_t x = 0; x < GetSubModelCount(); ++x)
    {
        SubModelElement* sme = GetSubModel(x);

        if (sme != nullptr && sme->HasEffects()) return true;
    }

    return false;
}

int ModelElement::GetEffectCount() const {

    int sum = std::accumulate(
        mEffectLayers.begin(), 
        mEffectLayers.end(), 0, 
        [](int i, EffectLayer* l) {
        return l->GetEffectCount() + i;
        });

     int strand_sum = std::accumulate(
        mStrands.begin(), 
        mStrands.end(), 0, 
        [](int i, StrandElement* se) {
            if (se != nullptr) {
                return se->GetEffectCount() + i;
            }
            return i;
        });

    int sub_sum = std::accumulate(
        mSubModels.begin(), 
        mSubModels.end(), 0, 
        [](int i, SubModelElement* se) {
            if (se != nullptr) {
                return se->GetEffectCount() + i;
            }
            return i;
        });

    return strand_sum + sub_sum + sum;
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
        for (const auto& w : words)
        {
            std::list<Effect*> ph = GetEffectsBetween(l3, w->GetStartTimeMS(), w->GetEndTimeMS());
            res += "\t\t\t" + w->GetEffectName() + " " + std::string(wxString::Format("%d", w->GetStartTimeMS() / ms).c_str()) + " " + std::string(wxString::Format("%d", w->GetEndTimeMS() / ms).c_str()) + " " + std::string(wxString::Format("%d", ph.size()).c_str()) + "\n";
            for (const auto& p : ph)
            {
                res += "\t\t\t\t" + std::string(wxString::Format("%d", p->GetStartTimeMS() / ms).c_str()) + " " + p->GetEffectName() + "\n";
            }
        }
    }

    return res;
}

std::string TimingElement::GetExport() const
{
    std::string res = "";

    for (const auto& l : mEffectLayers) {
        res += "   <EffectLayer>\n";
        for (int i = 0; i < l->GetEffectCount(); i++) {
            res += "      <Effect label=\"" + XmlSafe(l->GetEffect(i)->GetEffectName()) +
                "\" starttime=\"" + std::string(wxString::Format("%d", l->GetEffect(i)->GetStartTimeMS())) +
                "\" endtime=\"" + std::string(wxString::Format("%d", l->GetEffect(i)->GetEndTimeMS())) +
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

void ModelElement::Init(Model &model) {
    if (model.GetDisplayAs() == "WholeHouse"
        || model.GetDisplayAs() == "ModelGroup") {
        //no strands for a whole house model
        return;
    }
    for (const auto& sm : model.GetSubModels()) {
        bool found = false;
        for (const auto& sm2 : mSubModels) {
            if (sm2->GetName() == sm->Name())
            {
                found = true;
                break;
            }
        }
        if (!found) {
            mSubModels.push_back(new SubModelElement(this, sm->Name()));
        }
    }
    int ns = model.GetNumStrands();
    for (int x = 0; x < ns; x++) {
        if (x >= mStrands.size()) {
            StrandElement* new_layer = new StrandElement(this, mStrands.size());
            mStrands.push_back(new_layer);
        }
        mStrands[x]->InitFromModel(model);
    }
}

StrandElement* ModelElement::GetStrand(int index, bool create) {
    while (create && index >= mStrands.size()) {
        StrandElement* new_layer = new StrandElement(this, mStrands.size());
        mStrands.push_back(new_layer);
        IncrementChangeCount(-1, -1);
    }
    if (index >= mStrands.size()) {
        return nullptr;
    }
    return mStrands[index];
}

StrandElement* ModelElement::GetStrand(int strand) const
{
    if (strand >= mStrands.size()) {
        return nullptr;
    }
    return mStrands[strand];
}

int ModelElement::GetSubModelAndStrandCount() const {
    return mSubModels.size() +  mStrands.size();
}

int ModelElement::GetSubModelCount() const {
    return mSubModels.size();
}

void ModelElement::RemoveSubModel(const std::string &name) {
    for (auto a = mSubModels.begin(); a != mSubModels.end(); ++a) {
        if (name == (*a)->GetName()) {
            delete *a;
            mSubModels.erase(a);
            break;
        }
    }
}

SubModelElement* ModelElement::GetSubModel(int i) const {
    if (i < mSubModels.size()) {
        return mSubModels[i];
    }
    i -= mSubModels.size();
    if (i >= mStrands.size()) {
        return nullptr;
    }
    return mStrands[i];
}

SubModelElement* ModelElement::GetSubModel(int i)
{
    if (i < mSubModels.size()) {
        return mSubModels[i];
    }
    i -= mSubModels.size();
    if (i >= mStrands.size()) {
        return nullptr;
    }
    return mStrands[i];
}

void ModelElement::AddSubModel(SubModelElement* sme) {
    mSubModels.push_back(sme);
}

SubModelElement* ModelElement::GetSubModel(const std::string& name, bool create)
{
    for (const auto& a : mSubModels) {
        if (name == a->GetName()) {
            return a;
        }
    }
    for (const auto& a : mStrands) {
        if (name == a->GetName()) {
            return a;
        }
    }
    if (create) {
        mSubModels.push_back(new SubModelElement(this, name));
        return mSubModels.back();
    }
    return nullptr;
}

std::list<std::string> Element::GetFileReferences(Model* model, EffectManager& em) const
{
    std::list<std::string> res;
    if (GetType() != ElementType::ELEMENT_TYPE_TIMING) {
        for (int j = 0; j < GetEffectLayerCount(); j++) {
            EffectLayer* el = GetEffectLayer(j);
            res.splice(end(res), el->GetFileReferences(model, em));
        }
    }
    return res;
}

std::list<std::string> Element::GetFacesUsed(EffectManager& em) const
{
    std::list<std::string> res;
    if (GetType() != ElementType::ELEMENT_TYPE_TIMING) {
        for (int j = 0; j < GetEffectLayerCount(); j++) {
            EffectLayer* el = GetEffectLayer(j);
            res.splice(end(res), el->GetFacesUsed(em));
        }
    }
    return res;
}

bool Element::CleanupFileLocations(xLightsFrame* frame, EffectManager& em)
{
    bool rc = false;
    if (GetType() != ElementType::ELEMENT_TYPE_TIMING) {
        for (int j = 0; j < GetEffectLayerCount(); j++) {
            EffectLayer* el = GetEffectLayer(j);
            rc = el->CleanupFileLocations(frame, em) || rc;
        }
    }
    return rc;
}

Effect* Element::SelectEffectUsingDescription(std::string description)
{
    for (int j = 0; j < GetEffectLayerCount(); j++) {
        EffectLayer* el = GetEffectLayer(j);
        Effect* e = el->SelectEffectUsingDescription(description);
        if (e != nullptr) {
            return e;
        }
    }
    return nullptr;
}

bool StrandElement::IsEffectValid(Effect* e) const
{
    for (int j = 0; j < GetEffectLayerCount(); j++) {
        EffectLayer* el = GetEffectLayer(j);
        if (el->IsEffectValid(e)) {
            return true;
        }
    }

    for (int j = 0; j < GetNodeLayerCount(); j++) {
        if (GetNodeEffectLayer(j)->IsEffectValid(e)) {
            return true;
        }
    }

    return false;
}

bool Element::IsEffectValid(Effect* e) const
{
    for (int j = 0; j < GetEffectLayerCount(); j++) {
        EffectLayer* el = GetEffectLayer(j);
        if (el->IsEffectValid(e)) {
            return true;
        }
    }
    return false;
}

Effect* Element::SelectEffectUsingLayerTime(int layer, int time)
{
    if (layer < GetEffectLayerCount()) {
        EffectLayer* el = GetEffectLayer(layer);
        Effect* e = el->SelectEffectUsingTime(time);
        if (e != nullptr) {
            return e;
        }
    }
    return nullptr;
}

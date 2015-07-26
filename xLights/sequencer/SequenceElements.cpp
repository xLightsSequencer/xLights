#include "wx/wx.h"
#include <wx/utils.h>
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "SequenceElements.h"
#include "TimeLine.h"
#include "xLightsMain.h"
#include "LyricsDialog.h"

SequenceElements::SequenceElements()
: undo_mgr(this)
{
    mSelectedTimingRow = -1;
    mTimingRowCount = 0;
    mMaxRowsDisplayed = 0;
    mFirstVisibleModelRow = 0;
    mModelsNode = nullptr;
    mChangeCount = 0;
    mCurrentView = 0;
    std::vector <Element*> master_view;
    mAllViews.push_back(master_view);  // first view must remain as master view that determines render order
    xframe = nullptr;
    hasPapagayoTiming = false;
}

SequenceElements::~SequenceElements()
{
    ClearAllViews();
}

void SequenceElements::ClearAllViews()
{
    for (int y = 0; y < mAllViews[MASTER_VIEW].size(); y++) {
        delete mAllViews[MASTER_VIEW][y];
    }

    for (int x = 0; x < mAllViews.size(); x++) {
        mAllViews[x].clear();
    }
    mAllViews.clear();
}

void SequenceElements::Clear() {
    ClearAllViews();
    mVisibleRowInformation.clear();
    mRowInformation.clear();
    mSelectedRanges.clear();

    mSelectedTimingRow = -1;
    mTimingRowCount = 0;
    mFirstVisibleModelRow = 0;
    mChangeCount = 0;
    mCurrentView = 0;
    std::vector <Element*> master_view;
    mAllViews.push_back(master_view);
    hasPapagayoTiming = false;
}

void SequenceElements::SetSequenceEnd(int ms)
{
    mSequenceEndMS = ms;
}

EffectLayer* SequenceElements::GetEffectLayer(Row_Information_Struct *s) {

    Element* e = s->element;
    if (s->strandIndex == -1) {
        return e->GetEffectLayer(s->layerIndex);
    } else if (s->nodeIndex == -1) {
        return e->GetStrandLayer(s->strandIndex);
    } else {
        return e->GetStrandLayer(s->strandIndex)->GetNodeLayer(s->nodeIndex);
    }
}

EffectLayer* SequenceElements::GetEffectLayer(int row) {
    if(row==-1) return nullptr;
    return GetEffectLayer(GetRowInformation(row));
}

EffectLayer* SequenceElements::GetVisibleEffectLayer(int row) {
    if(row==-1) return nullptr;
    return GetEffectLayer(GetVisibleRowInformation(row));
}

Element* SequenceElements::AddElement(wxString &name,wxString &type,bool visible,bool collapsed,bool active, bool selected)
{
    if(!ElementExists(name))
    {
        mAllViews[MASTER_VIEW].push_back(new Element(this, name,type,visible,collapsed,active,selected));
        IncrementChangeCount();
        return mAllViews[MASTER_VIEW][mAllViews[MASTER_VIEW].size()-1];
    }
    return NULL;
}

Element* SequenceElements::AddElement(int index,wxString &name,wxString &type,bool visible,bool collapsed,bool active, bool selected)
{
    if(!ElementExists(name) && index <= mAllViews[MASTER_VIEW].size())
    {
        mAllViews[MASTER_VIEW].insert(mAllViews[MASTER_VIEW].begin()+index,new Element(this, name,type,visible,collapsed,active,selected));
        IncrementChangeCount();
        return mAllViews[MASTER_VIEW][index];
    }
    return NULL;
}

int SequenceElements::GetElementCount(int view)
{
    return mAllViews[view].size();
}

bool SequenceElements::ElementExists(wxString elementName, int view)
{
    for(int i=0;i<mAllViews[view].size();i++)
    {
        if(mAllViews[view][i]->GetName() == elementName)
        {
            return true;
        }
    }
    return false;
}

bool SequenceElements::TimingIsPartOfView(Element* timing, int view)
{
    wxString view_name = GetViewName(view);
    wxArrayString views = wxSplit(timing->GetViews(),',');
    for(int v=0;v<views.size();v++)
    {
        wxString viewName = views[v];
        if( view_name == viewName )
        {
            return true;
        }
    }
    return false;
}

wxString SequenceElements::GetViewName(int which_view)
{
    wxString view_name = "Master View";
    int view_index = 1;
    for(wxXmlNode* view=mViewsNode->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        if( view_index == which_view )
        {
            view_name = view->GetAttribute("name");
            break;
        }
        view_index++;
    }
    return view_name;
}

void SequenceElements::SetViewsNode(wxXmlNode* viewsNode)
{
    mViewsNode = viewsNode;
}

void SequenceElements::SetModelsNode(wxXmlNode* node, xLightsFrame *f)
{
    mModelsNode = node;
    xframe = f;
}

void SequenceElements::SetEffectsNode(wxXmlNode* effectsNode)
{
    mEffectsNode = effectsNode;
}

wxString SequenceElements::GetViewModels(wxString viewName)
{
    wxString result="";
    for(wxXmlNode* view=mViewsNode->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        if(view->GetAttribute("name")==viewName)
        {
            result = view->GetAttribute("models");
            break;
        }
    }
    return result;
}

Element* SequenceElements::GetElement(const wxString &name)
{
    for(int i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        if(name == mAllViews[MASTER_VIEW][i]->GetName())
        {
            return mAllViews[MASTER_VIEW][i];
        }
    }
    return NULL;
}

Element* SequenceElements::GetElement(int index, int view)
{
    if(index < mAllViews[view].size())
    {
        return mAllViews[view][index];
    }
    else
    {
        return nullptr;
    }
}


void SequenceElements::DeleteElement(const wxString &name)
{
    // delete element pointer from all views
    for(int i=0;i<mAllViews.size();i++)
    {
        for(int j=0;j<mAllViews[i].size();j++)
        {
            if(name == mAllViews[i][j]->GetName())
            {
                mAllViews[i].erase(mAllViews[i].begin()+j);
                IncrementChangeCount();
                break;
            }
        }
    }

    // delete contents of pointer
    for(int j=0;j<mAllViews[MASTER_VIEW].size();j++)
    {
        if(name == mAllViews[MASTER_VIEW][j]->GetName())
        {
            Element *e = mAllViews[MASTER_VIEW][j];
            delete e;
            break;
        }
    }
    PopulateRowInformation();
}

void SequenceElements::DeleteElementFromView(const wxString &name, int view)
{
    // delete element pointer from all views
    for(int j=0;j<mAllViews[view].size();j++)
    {
        if(name == mAllViews[view][j]->GetName())
        {
            mAllViews[view].erase(mAllViews[view].begin()+j);
            break;
        }
    }

    PopulateRowInformation();
}

void SequenceElements::DeleteTimingFromView(const wxString &name, int view)
{
    wxString viewName = GetViewName(view);
    Element* elem = GetElement(name);
    if( elem != nullptr && elem->GetType() == "timing" )
    {
        wxString views = elem->GetViews();
        wxArrayString all_views = wxSplit(views,',');
        int found = -1;
        for( int j = 0; j < all_views.size(); j++ )
        {
            if( all_views[j] == viewName )
            {
                found = j;
                break;
            }
        }
        if( found != -1 )
        {
            all_views.erase(all_views.begin() + found);
            views = wxJoin(all_views, ',');
            elem->SetViews(views);
        }
    }
}

Row_Information_Struct* SequenceElements::GetVisibleRowInformation(int index)
{
    if(index < mVisibleRowInformation.size())
    {
        return &mVisibleRowInformation[index];
    }
    else
    {
        return NULL;
    }
}

Row_Information_Struct* SequenceElements::GetVisibleRowInformationFromRow(int row_number)
{
    for(int i=0;i<mVisibleRowInformation.size();i++)
    {
        if(row_number == mVisibleRowInformation[i].RowNumber)
        {
            return &mVisibleRowInformation[i];
        }
    }
    return NULL;
}

int SequenceElements::GetVisibleRowInformationSize()
{
    return mVisibleRowInformation.size();
}

Row_Information_Struct* SequenceElements::GetRowInformation(int index)
{
    if(index < mRowInformation.size())
    {
        return &mRowInformation[index];
    }
    else
    {
        return NULL;
    }
}

Row_Information_Struct* SequenceElements::GetRowInformationFromRow(int row_number)
{
    for(int i=0;i<mRowInformation.size();i++)
    {
        if(row_number == mRowInformation[i].Index - GetFirstVisibleModelRow())
        {
            return &mRowInformation[i];
        }
    }
    return NULL;
}

int SequenceElements::GetRowInformationSize()
{
    return mRowInformation.size();
}

void SequenceElements::SortElements()
{
    if (mAllViews[mCurrentView].size()>1)
        std::sort(mAllViews[mCurrentView].begin(),mAllViews[mCurrentView].end(),SortElementsByIndex);
}

void SequenceElements::MoveElement(int index,int destinationIndex)
{
    IncrementChangeCount();
    if(index<destinationIndex)
    {
        mAllViews[mCurrentView][index]->Index = destinationIndex;
        for(int i=index+1;i<destinationIndex;i++)
        {
            mAllViews[mCurrentView][i]->Index = i-1;
        }
    }
    else
    {
        mAllViews[mCurrentView][index]->Index = destinationIndex;
        for(int i=destinationIndex;i<index;i++)
        {
            mAllViews[mCurrentView][i]->Index = i+1;
        }
    }
    SortElements();
}

void SequenceElements::LoadEffects(EffectLayer *effectLayer,
                                   const wxString &type,
                                   wxXmlNode *effectLayerNode,
                                   std::vector<wxString> effectStrings,
                                   std::vector<wxString> colorPalettes) {
    for(wxXmlNode* effect=effectLayerNode->GetChildren(); effect!=NULL; effect=effect->GetNext())
    {
        if (effect->GetName() == "Effect")
        {
            wxString effectName;
            wxString settings;
            int id = 0;
            int effectIndex = 0;
            long palette = -1;
            bool bProtected=false;

            // Start time
            double startTime;
            effect->GetAttribute("startTime").ToDouble(&startTime);
            startTime = TimeLine::RoundToMultipleOfPeriod(startTime,mFrequency);
            // End time
            double endTime;
            effect->GetAttribute("endTime").ToDouble(&endTime);
            endTime = TimeLine::RoundToMultipleOfPeriod(endTime,mFrequency);
            // Protected
            bProtected = effect->GetAttribute("protected")=='1'?true:false;
            if(type != "timing")
            {
                // Name
                effectName = effect->GetAttribute("name");
                effectIndex = Effect::GetEffectIndex(effectName);
                // ID
                id = wxAtoi(effect->GetAttribute("id", "0"));
                if (effect->GetAttribute("ref") != "") {
                    settings = effectStrings[wxAtoi(effect->GetAttribute("ref"))];
                } else {
                    settings = effect->GetNodeContent();
                }

                wxString tmp;
                if (effect->GetAttribute("palette", &tmp)) {
                    tmp.ToLong(&palette);
                }
            }
            else
            {
                // store timing labels in name attribute
                effectName = effect->GetAttribute("label");

            }
            effectLayer->AddEffect(id,effectIndex,effectName,settings,
                                   palette == -1 ? "" : colorPalettes[palette],
                                   startTime,endTime,EFFECT_NOT_SELECTED,bProtected);
        } else if (effect->GetName() == "Node" && effectLayerNode->GetName() == "Strand") {
            EffectLayer* neffectLayer = ((StrandLayer*)effectLayer)->GetNodeLayer(wxAtoi(effect->GetAttribute("index")), true);
            if (effect->GetAttribute("name", "") != "") {
                ((NodeLayer*)neffectLayer)->SetName(effect->GetAttribute("name"));
            }

            LoadEffects(neffectLayer, type, effect, effectStrings, colorPalettes);
        }
    }

}
bool SequenceElements::LoadSequencerFile(xLightsXmlFile& xml_file)
{
    wxXmlDocument& seqDocument = xml_file.GetXmlDocument();

    wxXmlNode* root=seqDocument.GetRoot();
    std::vector<wxString> effectStrings;
    std::vector<wxString> colorPalettes;
    Clear();
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "DisplayElements")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                bool active=false;
                bool selected=false;
                bool collapsed=false;
                wxString name = element->GetAttribute("name");
                wxString type = element->GetAttribute("type");
                bool visible = element->GetAttribute("visible")=='1'?true:false;

                if (type=="timing")
                {
                    active = element->GetAttribute("active")=='1'?true:false;
                }
                else
                {
                    collapsed = element->GetAttribute("collapsed")=='1'?true:false;
                }
                Element* elem = AddElement(name,type,visible,collapsed,active,selected);
                if (type=="timing")
                {
                    wxString views = element->GetAttribute("views", "");
                    elem->SetViews(views);
                }
            }
       }
       else if (e->GetName() == "EffectDB")
       {
           effectStrings.clear();
           for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
           {
               if(elementNode->GetName()=="Effect")
               {
                   effectStrings.push_back(elementNode->GetNodeContent());
               }
           }
       }
       else if (e->GetName() == "ColorPalettes")
       {
           colorPalettes.clear();
           for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
           {
               if(elementNode->GetName()=="ColorPalette")
               {
                   colorPalettes.push_back(elementNode->GetNodeContent());
               }
           }
       }
       else if (e->GetName() == "ElementEffects")
        {
            for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
            {
                if(elementNode->GetName()=="Element")
                {
                    Element* element = GetElement(elementNode->GetAttribute("name"));
                    if (element !=NULL)
                    {
                        // check for fixed timing interval
                        int interval = 0;
                        if( elementNode->GetAttribute("type") == "timing" )
                        {
                            interval = wxAtoi(elementNode->GetAttribute("fixed"));
                        }
                        if( interval > 0 )
                        {
                            element->SetFixedTiming(interval);
                            EffectLayer* effectLayer = element->AddEffectLayer();
                            int time = 0;
                            int end_time = xml_file.GetSequenceDurationMS();
                            int startTime, endTime, next_time;
                            while( time <= end_time )
                            {
                                next_time = (time + interval <= end_time) ? time + interval : end_time;
                                startTime = TimeLine::RoundToMultipleOfPeriod(time,mFrequency);
                                endTime = TimeLine::RoundToMultipleOfPeriod(next_time,mFrequency);
                                effectLayer->AddEffect(0,0,wxEmptyString,wxEmptyString,"",startTime,endTime,EFFECT_NOT_SELECTED,false);
                                time += interval;
                            }
                        }
                        else
                        {
                            for(wxXmlNode* effectLayerNode=elementNode->GetChildren(); effectLayerNode!=NULL; effectLayerNode=effectLayerNode->GetNext())
                            {
                                if (effectLayerNode->GetName() == "EffectLayer" || effectLayerNode->GetName() == "Strand")
                                {
                                    EffectLayer* effectLayer = NULL;
                                    if (effectLayerNode->GetName() == "EffectLayer") {
                                        effectLayer = element->AddEffectLayer();
                                    } else {
                                        effectLayer = element->GetStrandLayer(wxAtoi(effectLayerNode->GetAttribute("index")), true);
                                        if (effectLayerNode->GetAttribute("name", "") != "") {
                                            ((StrandLayer*)effectLayer)->SetName(effectLayerNode->GetAttribute("name"));
                                        }
                                    }
                                    LoadEffects(effectLayer, elementNode->GetAttribute("type"), effectLayerNode, effectStrings, colorPalettes);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Select view and set current view models as visible
    int last_view = xml_file.GetLastView();
    for(wxXmlNode* view=mViewsNode->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        wxString viewName = view->GetAttribute("name");
        wxString models = view->GetAttribute("models");
        std::vector <Element*> new_view;
        mAllViews.push_back(new_view);
        int view_index = mAllViews.size()-1;
        if( view_index == last_view )
        {
            AddMissingModelsToSequence(models);
            PopulateView(models, view_index);
            SetCurrentView(view_index);
        }
    }

    if (mModelsNode != nullptr) {
        PopulateRowInformation();
    }
    // Set to the first model/view
    mFirstVisibleModelRow = 0;
    return true;
}

void SequenceElements::AddView(const wxString &viewName)
{
    std::vector <Element*> new_view;
    mAllViews.push_back(new_view);
}

void SequenceElements::RemoveView(int view_index)
{
    mAllViews[view_index].clear();
    mAllViews.erase(mAllViews.begin() + view_index);
}

void SequenceElements::SetCurrentView(int view)
{
    mCurrentView = view;
}

void SequenceElements::AddMissingModelsToSequence(const wxString &models, bool visible)
{
    if(models.length()> 0)
    {
        wxArrayString model=wxSplit(models,',');
        for(int m=0;m<model.size();m++)
        {
            wxString modelName = model[m];
            if(!ElementExists(modelName))
            {
               wxString elementType = "model";
               Element* elem = AddElement(modelName,elementType,visible,false,false,false);
               elem->AddEffectLayer();
            }
        }
    }
}

void SequenceElements::SetTimingVisibility(const wxString& name)
{
    for(int i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        Element* elem = mAllViews[MASTER_VIEW][i];
        if( elem->GetType() == "model" )
        {
            break;
        }
        if( name == "Master View" )
        {
            elem->SetVisible(true);
        }
        else
        {
            elem->SetVisible(false);
            wxArrayString views = wxSplit(elem->GetViews(),',');
            for(int v=0;v<views.size();v++)
            {
                wxString viewName = views[v];
                if( name == viewName )
                {
                    elem->SetVisible(true);
                    break;
                }
            }
        }
    }
}

void SequenceElements::AddViewToTimings(wxArrayString& timings, const wxString& name)
{
    for( int i = 0; i < timings.size(); i++ )
    {
        Element* elem = GetElement(timings[i]);
        if( elem != nullptr && elem->GetType() == "timing" )
        {
            wxString views = elem->GetViews();
            wxArrayString all_views = wxSplit(views,',');
            bool found = false;
            for( int j = 0; j < all_views.size(); j++ )
            {
                if( all_views[j] == name )
                {
                    found = true;
                    break;
                }
            }
            if( !found )
            {
                all_views.push_back(name);
                views = wxJoin(all_views, ',');
                elem->SetViews(views);
            }
        }
    }
}

void SequenceElements::PopulateView(const wxString &models, int view)
{
    mAllViews[view].clear();

    if(models.length()> 0)
    {
        wxArrayString model=wxSplit(models,',');
        for(int m=0;m<model.size();m++)
        {
            wxString modelName = model[m];
            Element* elem = GetElement(modelName);
            mAllViews[view].push_back(elem);
        }
    }
}

void SequenceElements::SetFrequency(double frequency)
{
    mFrequency = frequency;
}

double SequenceElements::GetFrequency()
{
    return mFrequency;
}


void SequenceElements::SetSelectedTimingRow(int row)
{
    mSelectedTimingRow = row;
}

int SequenceElements::GetSelectedTimingRow()
{
    return mSelectedTimingRow;
}

wxXmlNode *GetModelNode(wxXmlNode *root, const wxString & name) {
    wxXmlNode* e;
    for(e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            if (name == e->GetAttribute("name")) return e;
        }
    }
    return NULL;
}

void addModelElement(Element *elem, std::vector<Row_Information_Struct> &mRowInformation,
                     int &rowIndex, xLightsFrame *xframe,
                     std::vector <Element*> &elements,
                     bool submodel) {
    if(!elem->GetCollapsed())
    {
        for(int j =0; j<elem->GetEffectLayerCount();j++)
        {
            Row_Information_Struct ri;
            ri.element = elem;
            ri.displayName = elem->GetName();
            ri.Collapsed = elem->GetCollapsed();
            ri.Active = elem->GetActive();
            ri.colorIndex = 0;
            ri.layerIndex = j;
            ri.Index = rowIndex++;
            ri.submodel = submodel;
            mRowInformation.push_back(ri);
        }
    }
    else
    {
        Row_Information_Struct ri;
        ri.element = elem;
        ri.Collapsed = elem->GetCollapsed();
        ri.displayName = elem->GetName();
        ri.Active = elem->GetActive();
        ri.colorIndex = 0;
        ri.layerIndex = 0;
        ri.Index = rowIndex++;
        ri.submodel = submodel;
        mRowInformation.push_back(ri);
    }
    ModelClass &cls = xframe->GetModelClass(elem->GetName());
    elem->InitStrands(cls);
    if (cls.GetDisplayAs() == "WholeHouse" && elem->ShowStrands()) {
        wxString models = cls.GetModelXml()->GetAttribute("models");
        wxArrayString model=wxSplit(models,',');
        for(int m=0;m<model.size();m++) {
            for (int x = 0; x < elements.size(); x++) {
                if (elements[x]->GetName() == model[m]) {
                    addModelElement(elements[x], mRowInformation, rowIndex, xframe, elements, true);
                }
            }
        }
    } else if (elem->ShowStrands()) {
        for (int s = 0; s < elem->getStrandLayerCount(); s++) {
            StrandLayer * sl = elem->GetStrandLayer(s);
            if (elem->getStrandLayerCount() > 1) {
                Row_Information_Struct ri;
                ri.element = elem;
                ri.Collapsed = !elem->ShowStrands();
                ri.Active = elem->GetActive();
                ri.displayName = sl->GetName();

                ri.colorIndex = 0;
                ri.layerIndex = 0;
                ri.Index = rowIndex++;
                ri.strandIndex = s;
                ri.submodel = submodel;
                mRowInformation.push_back(ri);
            }

            if (sl->ShowNodes()) {
                for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                    Row_Information_Struct ri;
                    ri.element = elem;
                    ri.Collapsed = sl->ShowNodes();
                    ri.Active = !elem->GetActive();
                    ri.displayName = sl->GetNodeLayer(n)->GetName();
                    ri.colorIndex = 0;
                    ri.layerIndex = 0;
                    ri.Index = rowIndex++;
                    ri.strandIndex = s;
                    ri.nodeIndex = n;
                    ri.submodel = submodel;
                    mRowInformation.push_back(ri);
                }
            }
        }
    }
}

void SequenceElements::addTimingElement(Element *elem, std::vector<Row_Information_Struct> &mRowInformation,
                                        int &rowIndex, int &selectedTimingRow, int &timingRowCount, int &timingColorIndex) {
    if( elem->GetEffectLayerCount() > 1 )
    {
        hasPapagayoTiming = true;
    }

    if(!elem->GetCollapsed())
    {
        for(int j =0; j<elem->GetEffectLayerCount();j++)
        {
            Row_Information_Struct ri;
            ri.element = elem;
            ri.Collapsed = elem->GetCollapsed();
            ri.Active = elem->GetActive();
            ri.colorIndex = timingColorIndex;
            ri.layerIndex = j;
            if(selectedTimingRow<0 && j==0)
            {
                selectedTimingRow = ri.Active?rowIndex:-1;
            }

            ri.Index = rowIndex++;
            mRowInformation.push_back(ri);
            timingRowCount++;
        }
    }
    else
    {
        Row_Information_Struct ri;
        ri.element = elem;
        ri.Collapsed = elem->GetCollapsed();
        ri.displayName = elem->GetName();
        ri.Active = elem->GetActive();
        ri.colorIndex = timingColorIndex;
        ri.layerIndex = 0;
        ri.Index = rowIndex++;
        mRowInformation.push_back(ri);
    }
    timingColorIndex++;
}

void SequenceElements::PopulateRowInformation()
{
    int rowIndex=0;
    int timingColorIndex=0;
    mSelectedTimingRow = -1;
    mRowInformation.clear();
    mTimingRowCount = 0;

    for(int i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        Element* elem = mAllViews[MASTER_VIEW][i];
        if(elem->GetType()=="timing")
        {
            if( mCurrentView == MASTER_VIEW || TimingIsPartOfView(elem, mCurrentView))
            {
                addTimingElement(elem, mRowInformation, rowIndex, mSelectedTimingRow, mTimingRowCount, timingColorIndex);
            }
        }
    }

    for(int i=0;i<mAllViews[mCurrentView].size();i++)
    {
        Element* elem = mAllViews[mCurrentView][i];
        if(elem->GetVisible())
        {
            if (elem->GetType()=="model")
            {
                addModelElement(elem, mRowInformation, rowIndex, xframe, mAllViews[MASTER_VIEW], false);
            }
        }
    }
    PopulateVisibleRowInformation();
}

void SequenceElements::PopulateVisibleRowInformation()
{
    mVisibleRowInformation.clear();
    if(mRowInformation.size()==0){return;}
    // Add all timing element rows. They should always be first in the list
    int row=0;
    for(row=0;row<mTimingRowCount;row++)
    {
        if(row<mMaxRowsDisplayed)
        {
            mVisibleRowInformation.push_back(mRowInformation[row]);
        }
    }

    if (mFirstVisibleModelRow >= mRowInformation.size()) {
        mFirstVisibleModelRow = 0;
    }
    for(;row<mMaxRowsDisplayed && row+mFirstVisibleModelRow<mRowInformation.size();row++)
    {
        mRowInformation[row+mFirstVisibleModelRow].RowNumber = row;
        mVisibleRowInformation.push_back(mRowInformation[row+mFirstVisibleModelRow]);
    }
}

void SequenceElements::SetFirstVisibleModelRow(int row)
{
    // They all fit on screen. So set to first model element.
    if(mRowInformation.size() <= mMaxRowsDisplayed)
    {
        mFirstVisibleModelRow = 0;
    }
    else
    {
        int maxModelRowsThatCanBeDisplayed = mMaxRowsDisplayed - mTimingRowCount;
        int totalModelRowsToDisplay = mRowInformation.size() - mTimingRowCount;

        int leastRowToDisplay = (totalModelRowsToDisplay - maxModelRowsThatCanBeDisplayed);
        if(row > leastRowToDisplay)
        {
            mFirstVisibleModelRow = leastRowToDisplay;
        }
        else
        {
            mFirstVisibleModelRow = row;
        }
    }
    PopulateVisibleRowInformation();
}

int SequenceElements::GetNumberOfTimingRows()
{
    return mTimingRowCount;
}

void SequenceElements::DeactivateAllTimingElements()
{
    for(int i=0;i<mAllViews[mCurrentView].size();i++)
    {
        if(mAllViews[mCurrentView][i]->GetType()=="timing")
        {
            mAllViews[mCurrentView][i]->SetActive(false);
        }
    }
}

void SequenceElements::SelectEffectsInRowAndPositionRange(int startRow, int endRow, int startX,int endX)
{
    if(startRow<mVisibleRowInformation.size())
    {
        if(endRow>=mVisibleRowInformation.size())
        {
            endRow = mVisibleRowInformation.size()-1;
        }
        for(int i=startRow;i<=endRow;i++)
        {
            EffectLayer* effectLayer = GetEffectLayer(&mVisibleRowInformation[i]);
            effectLayer->SelectEffectsInPositionRange(startX,endX);
        }
    }
}

int SequenceElements::SelectEffectsInRowAndColumnRange(int startRow, int endRow, int startCol,int endCol)
{
    int num_selected = 0;
    if(startRow < mRowInformation.size())
    {
        if(endRow >= mRowInformation.size())
        {
            endRow = mRowInformation.size()-1;
        }
        EffectLayer* tel = GetVisibleEffectLayer(GetSelectedTimingRow());
        if( tel != nullptr )
        {
            Effect* eff1 = tel->GetEffect(startCol);
            Effect* eff2 = tel->GetEffect(endCol);
            if( eff1 != nullptr && eff2 != nullptr )
            {
                int start_time = eff1->GetStartTimeMS();
                int end_time = eff2->GetEndTimeMS();
                for(int i=startRow;i <= endRow;i++)
                {
                    EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
                    num_selected += effectLayer->SelectEffectsInTimeRange(start_time,end_time);
                }
            }
        }
    }
    return num_selected;
}

Effect* SequenceElements::GetSelectedEffectAtRowAndPosition(int row, int x,int &index, int &selectionType)
{
    EffectLayer* effectLayer = GetEffectLayer(&mVisibleRowInformation[row]);

    index = effectLayer->GetEffectIndexThatContainsPosition(x,selectionType);
    if(index<0)
    {
        return nullptr;
    }
    else
    {
        return effectLayer->GetEffect(index);
    }
}



void SequenceElements::UnSelectAllEffects()
{
    for(int i=0;i<mRowInformation.size();i++)
    {
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        effectLayer->UnSelectAllEffects();
    }
}

void SequenceElements::UnSelectAllElements()
{
    for(int i=0;i<mAllViews[mCurrentView].size();i++)
    {
        mAllViews[mCurrentView][i]->SetSelected(false);
    }
}

// Functions to manage selected ranges
int SequenceElements::GetSelectedRangeCount()
{
    return mSelectedRanges.size();
}

EffectRange* SequenceElements::GetSelectedRange(int index)
{
    return &mSelectedRanges[index];
}

void SequenceElements::AddSelectedRange(EffectRange* range)
{
    mSelectedRanges.push_back(*range);
}

void SequenceElements::DeleteSelectedRange(int index)
{
    if(index < mSelectedRanges.size())
    {
        mSelectedRanges.erase(mSelectedRanges.begin()+index);
    }
}

void SequenceElements::ClearSelectedRanges()
{
    mSelectedRanges.clear();
}

void SequenceElements::SetMaxRowsDisplayed(int maxRows)
{
    mMaxRowsDisplayed = maxRows;
}

int SequenceElements::GetMaxModelsDisplayed()
{
    return mMaxRowsDisplayed - mTimingRowCount;
}

int SequenceElements::GetTotalNumberOfModelRows()
{
    return mRowInformation.size() - mTimingRowCount;
}

int SequenceElements::GetFirstVisibleModelRow()
{
    return mFirstVisibleModelRow;
}

void SequenceElements::SetVisibilityForAllModels(bool visibility, int view)
{
    for(int i=0;i<GetElementCount(view);i++)
    {
        Element * e = GetElement(i, view);
        e->SetVisible(visibility);
    }
}

void SequenceElements::MoveSequenceElement(int index, int dest, int view)
{
    IncrementChangeCount();

    if(index<mAllViews[view].size() && dest<mAllViews[view].size())
    {
        Element* e = mAllViews[view][index];
        mAllViews[view].erase(mAllViews[view].begin()+index);
        if(index >= dest)
        {
            mAllViews[view].insert(mAllViews[view].begin()+dest,e);
        }
        else
        {
            mAllViews[view].insert(mAllViews[view].begin()+(dest-1),e);
        }
    }
}

void SequenceElements::MoveElementUp(const wxString &name, int view)
{
    IncrementChangeCount();

    for(int i=0;i<mAllViews[view].size();i++)
    {
        if(name == mAllViews[view][i]->GetName())
        {
            // found element
            if( i > 0 )
            {
                MoveSequenceElement(i, i-1, view);
            }
            break;
        }
    }
}

void SequenceElements::MoveElementDown(const wxString &name, int view)
{
    IncrementChangeCount();

    for(int i=0;i<mAllViews[view].size();i++)
    {
        if(name == mAllViews[view][i]->GetName())
        {
            // found element
            if( i < mAllViews[view].size()-1 )
            {
                MoveSequenceElement(i+1, i, view);
            }
            break;
        }
    }
}

void SequenceElements::ImportLyrics(Element* element, wxWindow* parent)
{
    LyricsDialog* dlgLyrics = new LyricsDialog(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    if (dlgLyrics->ShowModal() == wxID_OK)
    {
        // remove all existing layers from timing track
        int num_layers = element->GetEffectLayerCount();
        for( int k = num_layers-1; k >= 0; k-- )
        {
            element->RemoveEffectLayer(k);
        }
        EffectLayer* phrase_layer = element->AddEffectLayer();

        int num_phrases = dlgLyrics->TextCtrlLyrics->GetNumberOfLines();
        for( int i = 0; i < dlgLyrics->TextCtrlLyrics->GetNumberOfLines(); i++ )
        {
            wxString line = dlgLyrics->TextCtrlLyrics->GetLineText(i);
            if( line == "" )
            {
                num_phrases--;
            }
        }
        int start_time = 0;
        int end_time = mSequenceEndMS;
        int interval_ms = (end_time-start_time) / num_phrases;
        for( int i = 0; i < num_phrases; i++ )
        {
            wxString line = dlgLyrics->TextCtrlLyrics->GetLineText(i);
            if( line != "" )
            {
                xframe->dictionary.InsertSpacesAfterPunctuation(line);
                end_time = TimeLine::RoundToMultipleOfPeriod(start_time+interval_ms, GetFrequency());
                phrase_layer->AddEffect(0,0,line,wxEmptyString,"",start_time,end_time,EFFECT_NOT_SELECTED,false);
                start_time = end_time;
            }
        }
    }
}

void SequenceElements::BreakdownPhrase(EffectLayer* word_layer, EffectLayer* phoneme_layer, int start_time, int end_time, const wxString& phrase)
{
    if( phrase != "" )
    {
        xframe->dictionary.LoadDictionaries();
        wxArrayString words = wxSplit(phrase, ' ');
        int num_words = words.Count();
        int word_end_time = end_time;
        int interval_ms = (word_end_time-start_time) / num_words;
        for( int i = 0; i < num_words; i++ )
        {
            int word_end_time = TimeLine::RoundToMultipleOfPeriod(start_time+interval_ms, GetFrequency());
            if( i == num_words - 1 )
            {
                word_end_time = end_time;
            }
            word_layer->AddEffect(0,0,words[i],wxEmptyString,"",start_time,word_end_time,EFFECT_NOT_SELECTED,false);
            BreakdownWord(phoneme_layer, start_time, word_end_time, words[i]);
            start_time = word_end_time;
        }
    }
}

void SequenceElements::BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time, const wxString& word)
{
    xframe->dictionary.LoadDictionaries();
    wxArrayString phonemes;
    xframe->dictionary.BreakdownWord(word, phonemes);
    if( phonemes.Count() > 0 )
    {
        int phoneme_start_time = start_time;
        int phoneme_end_time = end_time;
        int phoneme_interval_ms = (phoneme_end_time-start_time) / phonemes.Count();
        for( int i = 0; i < phonemes.Count(); i++ )
        {
            phoneme_end_time = TimeLine::RoundToMultipleOfPeriod(phoneme_start_time+phoneme_interval_ms, GetFrequency());
            if( i == phonemes.Count() - 1 )
            {
                phoneme_end_time = end_time;
            }
            phoneme_layer->AddEffect(0,0,phonemes[i],wxEmptyString,"",phoneme_start_time,phoneme_end_time,EFFECT_NOT_SELECTED,false);
            phoneme_start_time = phoneme_end_time;
        }
    }
}


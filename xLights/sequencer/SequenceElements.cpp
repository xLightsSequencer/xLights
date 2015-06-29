#include "wx/wx.h"
#include <wx/utils.h>
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "SequenceElements.h"
#include "TimeLine.h"


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

void SequenceElements::SetViewsNode(wxXmlNode* viewsNode)
{
    mViewsNode = viewsNode;
}

void SequenceElements::SetModelsNode(wxXmlNode* node, NetInfoClass *ni)
{
    mModelsNode = node;
    netInfo = ni;
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
                Element *e = mAllViews[i][j];
                mAllViews[i].erase(mAllViews[i].begin()+j);
                IncrementChangeCount();
                break;
            }
        }
    }

    // delete contents of pointer
    for(int j=0;j<mAllViews[MASTER_VIEW].size();j++)
    {
        wxLogDebug(wxString::Format("DeleteElement: %d   %d", mAllViews[MASTER_VIEW].size()));
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
            Element *e = mAllViews[view][j];
            mAllViews[view].erase(mAllViews[view].begin()+j);
            break;
        }
    }

    PopulateRowInformation();
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
                AddElement(name,type,visible,collapsed,active,selected);
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
    // Set current view models as visible
    for(wxXmlNode* view=mViewsNode->GetChildren(); view!=NULL; view=view->GetNext() )
    {
        wxString viewName = view->GetAttribute("name");
        wxString models = view->GetAttribute("models");
        std::vector <Element*> new_view;
        mAllViews.push_back(new_view);
        int view_index = mAllViews.size()-1;

        bool isChecked = view->GetAttribute("selected")=="1"?true:false;
        if( isChecked )
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

void SequenceElements::SetCurrentView(int view)
{
    mCurrentView = view;
}

void SequenceElements::AddMissingModelsToSequence(const wxString &models)
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
               Element* elem = AddElement(modelName,elementType,false,false,false,false);
               elem->AddEffectLayer();
            }
        }
    }
}

void SequenceElements::PopulateView(const wxString &models, int view)
{
    mAllViews[view].clear();

    for(int i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        if(mAllViews[MASTER_VIEW][i]->GetType() == "timing")
        {
            mAllViews[view].push_back(mAllViews[MASTER_VIEW][i]);
        }
    }

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
void SequenceElements::PopulateRowInformation()
{
    int rowIndex=0;
    int timingColorIndex=0;
    mSelectedTimingRow = -1;
    mRowInformation.clear();
    mTimingRowCount = 0;
    for(int i=0;i<mAllViews[mCurrentView].size();i++)
    {
        if(mAllViews[mCurrentView][i]->GetVisible())
        {
            if (mAllViews[mCurrentView][i]->GetType()=="model")
            {
                if(!mAllViews[mCurrentView][i]->GetCollapsed())
                {
                    for(int j =0; j<mAllViews[mCurrentView][i]->GetEffectLayerCount();j++)
                    {
                        Row_Information_Struct ri;
                        ri.element = mAllViews[mCurrentView][i];
                        ri.displayName = mAllViews[mCurrentView][i]->GetName();
                        ri.Collapsed = mAllViews[mCurrentView][i]->GetCollapsed();
                        ri.Active = mAllViews[mCurrentView][i]->GetActive();
                        ri.PartOfView = false;
                        ri.colorIndex = 0;
                        ri.layerIndex = j;
                        ri.Index = rowIndex++;
                        mRowInformation.push_back(ri);
                    }
                }
                else
                {
                    Row_Information_Struct ri;
                    ri.element = mAllViews[mCurrentView][i];
                    ri.Collapsed = mAllViews[mCurrentView][i]->GetCollapsed();
                    ri.displayName = mAllViews[mCurrentView][i]->GetName();
                    ri.Active = mAllViews[mCurrentView][i]->GetActive();
                    ri.PartOfView = false;
                    ri.colorIndex = 0;
                    ri.layerIndex = 0;
                    ri.Index = rowIndex++;
                    mRowInformation.push_back(ri);
                }
                mAllViews[mCurrentView][i]->InitStrands(GetModelNode(mModelsNode, mAllViews[mCurrentView][i]->GetName()), *netInfo);
                if (mAllViews[mCurrentView][i]->ShowStrands()) {
                    for (int s = 0; s < mAllViews[mCurrentView][i]->getStrandLayerCount(); s++) {
                        StrandLayer * sl = mAllViews[mCurrentView][i]->GetStrandLayer(s);
                        if (mAllViews[mCurrentView][i]->getStrandLayerCount() > 1) {
                            Row_Information_Struct ri;
                            ri.element = mAllViews[mCurrentView][i];
                            ri.Collapsed = !mAllViews[mCurrentView][i]->ShowStrands();
                            ri.Active = mAllViews[mCurrentView][i]->GetActive();
                            ri.displayName = sl->GetName();

                            ri.PartOfView = false;
                            ri.colorIndex = 0;
                            ri.layerIndex = 0;
                            ri.Index = rowIndex++;
                            ri.strandIndex = s;
                            mRowInformation.push_back(ri);
                        }

                        if (sl->ShowNodes()) {
                            for (int n = 0; n < sl->GetNodeLayerCount(); n++) {
                                Row_Information_Struct ri;
                                ri.element = mAllViews[mCurrentView][i];
                                ri.Collapsed = sl->ShowNodes();
                                ri.Active = !mAllViews[mCurrentView][i]->GetActive();
                                ri.displayName = sl->GetNodeLayer(n)->GetName();
                                ri.PartOfView = false;
                                ri.colorIndex = 0;
                                ri.layerIndex = 0;
                                ri.Index = rowIndex++;
                                ri.strandIndex = s;
                                ri.nodeIndex = n;
                                mRowInformation.push_back(ri);
                            }
                        }
                    }

                }
            }
            else if (mAllViews[mCurrentView][i]->GetType()=="timing")
            {
                Row_Information_Struct ri;
                ri.element = mAllViews[mCurrentView][i];
                ri.Collapsed = mAllViews[mCurrentView][i]->GetCollapsed();
                ri.Active = mAllViews[mCurrentView][i]->GetActive();
                ri.PartOfView = false;
                ri.colorIndex = timingColorIndex;
                ri.layerIndex = 0;
                if(mSelectedTimingRow<0)
                {
                    mSelectedTimingRow = ri.Active?rowIndex:-1;
                }

                ri.Index = rowIndex++;
                mRowInformation.push_back(ri);
                timingColorIndex++;
                mTimingRowCount++;
            }
            else        // View
            {
                Row_Information_Struct ri;
                ri.element = mAllViews[mCurrentView][i];
                ri.Collapsed = mAllViews[mCurrentView][i]->GetCollapsed();
                ri.Active = mAllViews[mCurrentView][i]->GetActive();
                ri.PartOfView = false;
                ri.colorIndex = 0;
                ri.layerIndex = 0;
                ri.Index = rowIndex++;
                mRowInformation.push_back(ri);
                if(!mAllViews[mCurrentView][i]->GetCollapsed())
                {
                    // Add models/effect layers in view
                    wxString models = GetViewModels(mAllViews[mCurrentView][i]->GetName());
                    if(models.length()> 0)
                    {
                        wxArrayString model=wxSplit(models,',');
                        for(int m=0;m<model.size();m++)
                        {
                            Element* element = GetElement(model[m]);
                            if(element->GetCollapsed())
                            {
                                for(int j=0;element->GetEffectLayerCount();j++)
                                {
                                    Row_Information_Struct r;
                                    r.element = element;
                                    r.Collapsed = element->GetCollapsed();
                                    r.Active = mAllViews[mCurrentView][i]->GetActive();
                                    r.PartOfView = false;
                                    r.colorIndex = 0;
                                    r.layerIndex = j;
                                    r.Index = rowIndex++;
                                    mRowInformation.push_back(r);
                                }
                            }
                            else
                            {
                                Row_Information_Struct r;
                                r.element = element;
                                r.Collapsed = element->GetCollapsed();
                                r.Active = mAllViews[mCurrentView][i]->GetActive();
                                r.PartOfView = false;
                                r.colorIndex = 0;
                                r.layerIndex = 0;
                                r.Index = rowIndex++;
                                mRowInformation.push_back(r);
                            }
                        }
                    }
                }
            }
        }
    }
    PopulateVisibleRowInformation();
}

void SequenceElements::PopulateVisibleRowInformation()
{
    if(mRowInformation.size()==0){return;}
    int row=0;
    mVisibleRowInformation.clear();
    // Add all timing element rows. They should always be first in the list
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

void SequenceElements::SetVisibilityForAllModels(bool visibility)
{
    for(int i=0;i<GetElementCount();i++)
    {
        Element * e = GetElement(i);
        if(e->GetType() == "model")
        {
            e->SetVisible(visibility);
        }
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


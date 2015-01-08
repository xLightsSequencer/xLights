#include "wx/wx.h"
#include <wx/utils.h>
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "SequenceElements.h"


SequenceElements::SequenceElements()
{
}

SequenceElements::~SequenceElements()
{
}


void SequenceElements::AddElement(wxString &name,wxString &type,bool visible,bool collapsed,bool active)
{
    if(!ElementExists(name))
    {
        Element e(name,type,visible,collapsed,active);
        mElements.push_back(e);
    }
}

bool SequenceElements::ElementExists(wxString elementName)
{
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i].GetName() == elementName)
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
    for(int i=0;i<mElements.size();i++)
    {
        if(name == mElements[i].GetName())
        {
            return &mElements[i];
        }
    }
    return NULL;
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

int SequenceElements::GetRowInformationSize()
{
    return mRowInformation.size();
}

void SequenceElements::SortElements()
{
    if (mRowInformation.size()>1)
        std::sort(mElements.begin(),mElements.end(),SortElementsByIndex);
}

void SequenceElements::MoveElement(int index,int destinationIndex)
{
    if(index<destinationIndex)
    {
        mElements[index].Index = destinationIndex;
        for(int i=index+1;i<destinationIndex;i++)
        {
            mElements[i].Index = i-1;
        }
    }
    else
    {
        mElements[index].Index = destinationIndex;
        for(int i=destinationIndex;i<index;i++)
        {
            mElements[i].Index = i+1;
        }
    }
    SortElements();

}


bool SequenceElements::LoadSequencerFile(wxString filename)
{
    wxString tmpStr;
    wxXmlDocument seqDocument;
    double startTime;
    double endTime;
    // read xml sequence info
    wxFileName FileObj(filename);
    FileObj.SetExt("xml");
    wxString SeqXmlFileName=FileObj.GetFullPath();
    int gridCol;
    if (!FileObj.FileExists())
    {
        //if (ChooseModels) ChooseModelsForSequence();
        return false;
    }
    // read xml
    //  first fix any version specific changes
    //FixVersionDifferences(SeqXmlFileName);
    if (!seqDocument.Load(SeqXmlFileName))
    {
        wxMessageBox(_("Error loading: ")+SeqXmlFileName);
        return false;
    }
    wxXmlNode* root=seqDocument.GetRoot();
    //wxString tempstr=root->GetAttribute("BaseChannel", "1");
    //tempstr.ToLong(&SeqBaseChannel);
    //tempstr=root->GetAttribute("ChanCtrlBasic", "0");
    //SeqChanCtrlBasic=tempstr!="0";
    //tempstr=root->GetAttribute("ChanCtrlColor", "0");
    //SeqChanCtrlColor=tempstr!="0";

    mElements.clear();
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "DisplayElements")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                bool active=false;
                bool collapsed=false;
                wxString name = element->GetAttribute("name");
                wxString type = element->GetAttribute("type");
                bool visible = element->GetAttribute("visible")=='1'?true:false;

                if (type=="timing")
                {
                    active = element->GetAttribute("active")=='1'?true:false;
                }
                else if (type=="view")
                {
                    collapsed = element->GetAttribute("collapsed")=='1'?true:false;
                }
                AddElement(name,type,visible,collapsed,active);
                // Add models for each view
                if(type=="view")
                {
                    wxString models = GetViewModels(name);
                    if(models.length()> 0)
                    {
                        wxArrayString model=wxSplit(models,',');
                        for(int m=0;m<model.size();m++)
                        {
                           wxString modelName =  model[m];
                           wxString elementType = "model";
                           AddElement(modelName,elementType,false,false,false);
                        }
                    }
                }
            }
            PopulateRowInformation();
       }
       if (e->GetName() == "ElementEffects")
        {
            for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
            {
                if(elementNode->GetName()=="Element")
                {
                    Element* element = GetElement(elementNode->GetAttribute("name"));
                    if (element !=NULL)
                    {
                        for(wxXmlNode* effect=elementNode->GetChildren(); effect!=NULL; effect=effect->GetNext())
                        {
                            if (effect->GetName() == "Effect")
                            {
                                int id;
                                wxString commonSettings = effect->GetNodeContent();
                                wxString layerSettings1;
                                wxString layerSettings2;
                                wxString effectName1;
                                wxString effectName2;
                                int effectIndex;
                                // Start time
                                effect->GetAttribute("startTime").ToDouble(&startTime);
                                startTime = ElementEffects::RoundToMultipleOfPeriod(startTime,mFrequency);
                                // End time
                                effect->GetAttribute("endTime").ToDouble(&endTime);
                                endTime = ElementEffects::RoundToMultipleOfPeriod(endTime,mFrequency);
                                // Protected
                                bool bProtected = effect->GetAttribute("protected")=='1'?true:false;
                                if(elementNode->GetAttribute("type") != "timing")
                                {
                                    // ID
                                    id = wxAtoi(effect->GetAttribute("id"));
                                    // Get Effect Layers (always two for now
                                    wxXmlNode* effectLayer=effect->GetChildren();
                                    // Parse first layer
                                    wxString effectName = effectLayer->GetAttribute("name");
                                    effectIndex = ElementEffects::GetEffectIndex(effectName);
                                    layerSettings1 = effectLayer->GetNodeContent();
                                    // Parse second layer
                                    effectLayer=effectLayer->GetNext();
                                    layerSettings2 = effectLayer->GetNodeContent();
                                }
                                element->AddEffect(id,commonSettings,layerSettings1,layerSettings2,effectIndex,startTime,endTime,bProtected);
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

void SequenceElements::SetFrequency(double frequency)
{
    mFrequency = frequency;
}

void SequenceElements::PopulateRowInformation()
{
    int rowIndex=0;
    int timingColorIndex=0;

    Row_Information_Struct ri;
    mRowInformation.clear();
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i].GetVisible())
        {
            ri.element = &mElements[i];
            ri.Collapsed = mElements[i].GetCollapsed();
            ri.Active = mElements[i].GetActive();
            ri.PartOfView = false;
            if(mElements[i].GetType()=="timing")
            {
                ri.colorIndex = timingColorIndex;
                timingColorIndex++;
            }
            else
            {
                ri.colorIndex = 0;
            }
            ri.Index = rowIndex++;

            mRowInformation.push_back(ri);
            if(mElements[i].GetType()== "view" && !mElements[i].GetCollapsed())
            {
                wxString models = GetViewModels(mElements[i].GetName());
                if(models.length()> 0)
                {
                    wxArrayString model=wxSplit(models,',');
                    for(int m=0;m<model.size();m++)
                    {
                        Element* element = GetElement(model[m]);
                        ri.element = element;
                        ri.Collapsed = false;
                        ri.Active = false;              // Not used for models or
                        ri.PartOfView = true;
                        ri.Index = rowIndex++;
                        ri.colorIndex = 0;
                        mRowInformation.push_back(ri);
                    }
                }
            }
        }
    }
}

void SequenceElements::DeactivateAllTimingElements()
{
    for(int i=0;i<mElements.size();i++)
    {
        if(mElements[i].GetType()=="timing")
        {
            mElements[i].SetActive(false);
        }
    }
}

void SequenceElements::SelectEffectsInRowAndPositionRange(int startRow, int endRow, int startX,int endX, int &FirstSelected)
{
    if(startRow<mRowInformation.size())
    {
        if(endRow>=mRowInformation.size())
        {
            endRow = mRowInformation.size()-1;
        }
        for(int i=startRow;i<=endRow;i++)
        {
            ElementEffects* effects = mRowInformation[i].element->GetElementEffects();
            effects->SelectEffectsInPositionRange(startX,endX,FirstSelected);
        }
    }
}

void SequenceElements::UnSelectAllEffects()
{
    for(int i=0;i<mRowInformation.size();i++)
    {
        ElementEffects* effects = mRowInformation[i].element->GetElementEffects();
        effects->UnSelectAllEffects();
    }
}

#include "wx/wx.h"
#include <wx/utils.h>
#include <algorithm>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "SequenceElements.h"
#include "TimeLine.h"
#include "../xLightsMain.h"
#include "../LyricsDialog.h"
#include "../xLightsXmlFile.h"
#include "../effects/RenderableEffect.h"
#include "../models/SubModel.h"
#include "../models/ModelGroup.h"
#include "../UtilFunctions.h"
#include "../SequenceViewManager.h"

static const std::string STR_EMPTY("");
static const std::string STR_NAME("name");
static const std::string STR_EFFECT("Effect");
static const std::string STR_ELEMENT("Element");
static const std::string STR_EFFECTLAYER("EffectLayer");
static const std::string STR_SUBMODEL_EFFECTLAYER("SubModelEffectLayer");
static const std::string STR_COLORPALETTE("ColorPalette");
static const std::string STR_NODE("Node");
static const std::string STR_STRAND("Strand");
static const std::string STR_INDEX("index");
static const std::string STR_TYPE("type");
static const std::string STR_TIMING("timing");

static const std::string STR_STARTTIME("startTime");
static const std::string STR_ENDTIME("endTime");
static const std::string STR_PROTECTED("protected");
static const std::string STR_ID("id");
static const std::string STR_REF("ref");
static const std::string STR_PALETTE("palette");
static const std::string STR_LABEL("label");
static const std::string STR_ZERO("0");


SequenceElements::SequenceElements(xLightsFrame *f)
: undo_mgr(this), xframe(f)
{
    mSelectedTimingRow = -1;
    mTimingRowCount = 0;
    mMaxRowsDisplayed = 0;
    mFirstVisibleModelRow = 0;
    mModelsNode = nullptr;
    mChangeCount = 0;
    mMasterViewChangeCount = 0;
    mCurrentView = 0;
    std::vector <Element*> master_view;
    mAllViews.push_back(master_view);  // first view must remain as master view that determines render order
    hasPapagayoTiming = false;
    supportsModelBlending = true;
}

SequenceElements::~SequenceElements()
{
    ClearAllViews();
}

void SequenceElements::ClearAllViews()
{
    for (size_t y = 0; y < mAllViews[MASTER_VIEW].size(); y++) {
        delete mAllViews[MASTER_VIEW][y];
    }

    for (size_t x = 0; x < mAllViews.size(); x++) {
        mAllViews[x].clear();
    }
    mAllViews.clear();
    mMasterViewChangeCount++;
}

void SequenceElements::Clear() {
    ClearAllViews();
    mVisibleRowInformation.clear();
    mRowInformation.clear();
    mSelectedRanges.clear();
    undo_mgr.Clear();
    mSelectedTimingRow = -1;
    mTimingRowCount = 0;
    mFirstVisibleModelRow = 0;
    mChangeCount = 0;
    mMasterViewChangeCount++;
    mCurrentView = 0;
    supportsModelBlending = true;
    std::vector <Element*> master_view;
    mAllViews.push_back(master_view);
    hasPapagayoTiming = false;
}

void SequenceElements::SetSequenceEnd(int ms)
{
    mSequenceEndMS = ms;
}

int SequenceElements::GetSequenceEnd()
{
    return mSequenceEndMS;
}

EffectLayer* SequenceElements::GetEffectLayer(Row_Information_Struct *s) {
    if (s == nullptr) {
        return nullptr;
    }
    Element* e = s->element;
    if (s->strandIndex == -1) {
        return e->GetEffectLayer(s->layerIndex);
    } else if (s->nodeIndex == -1) {
        SubModelElement *se = dynamic_cast<SubModelElement*>(e);
        if (se == nullptr) {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.error("Expected a SubModelElment be found %d", e->GetType());
            return nullptr;
        }
        return se->GetEffectLayer(s->layerIndex);
    } else {
        StrandElement *me = dynamic_cast<StrandElement*>(e);
        if (me == nullptr) {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.error("Expected a StrandElement be found %d", e->GetType());
            return nullptr;
        }
        return me->GetNodeLayer(s->nodeIndex);
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

static Element* CreateElement(SequenceElements *se, const std::string &name, const std::string &type,
                              bool visible,bool collapsed,bool active, bool selected,
                              xLightsFrame *xframe) {
    Element *el;
    if (type == "timing") {
        TimingElement *te = new TimingElement(se, name);
        el = te;
        te->SetActive(active);
    } else {
        ModelElement *me = new ModelElement(se, name, selected);
        if (xframe != nullptr) {
            Model *model = xframe->GetModel(name);
            if (model != nullptr) {
                me->Init(*model);
            }
        }
        el = me;
    }
    el->SetCollapsed(collapsed);
    el->SetVisible(visible);
    return el;
}

Element* SequenceElements::AddElement(const std::string &name, const std::string &type,
                                      bool visible,bool collapsed,bool active, bool selected)
{
    if(!ElementExists(name)) {
        Element *el = CreateElement(this, name,type,visible,collapsed,active,selected,xframe);

        mAllViews[MASTER_VIEW].push_back(el);
        mMasterViewChangeCount++;
        IncrementChangeCount(el);
        return el;
    }
    return nullptr;
}

Element* SequenceElements::AddElement(int index, const std::string &name,
                                      const std::string &type,
                                      bool visible,bool collapsed,bool active, bool selected)
{
    if(!ElementExists(name) && index <= mAllViews[MASTER_VIEW].size())
    {
        Element *el = CreateElement(this, name,type,visible,collapsed,active,selected,xframe);
        mAllViews[MASTER_VIEW].insert(mAllViews[MASTER_VIEW].begin()+index, el);
        mMasterViewChangeCount++;
        IncrementChangeCount(el);
        return el;
    }
    return nullptr;
}

size_t SequenceElements::GetElementCount(int view)
{
    return mAllViews[view].size();
}

bool SequenceElements::ElementExists(const std::string &elementName, int view)
{
    for(size_t i=0;i<mAllViews[view].size();i++)
    {
        if (mAllViews[view][i]->GetName() == elementName)
        {
            return true;
        }
}
    return false;
}

void SequenceElements::RenameTimingTrack(std::string oldname, std::string newname)
{
    // actual timing track name already updated ... we just need to update any effects that care about the timing track name
    // faces, state, piano, vumeter

    std::vector<RenderableEffect*> effects(GetEffectManager().size());
    for (int x = 0; x < GetEffectManager().size(); x++) {
        RenderableEffect *eff = GetEffectManager()[x];
        effects[x] = eff;
    }

    for (size_t i = 0; i < GetElementCount(); i++) {
        Element* e = GetElement(i);
        if (e->GetType() == ELEMENT_TYPE_MODEL) {
            ModelElement *elem = dynamic_cast<ModelElement *>(e);
            for (int j = 0; j < elem->GetEffectLayerCount(); j++) {
                EffectLayer* layer = elem->GetEffectLayer(j);
                for (int k = 0; k < layer->GetEffectCount(); k++) {
                    Effect* eff = layer->GetEffect(k);
                    if (effects[eff->GetEffectIndex()] != nullptr) {
                        effects[eff->GetEffectIndex()]->RenameTimingTrack(oldname, newname, eff);
                    }
                }
            }
            for (int j = 0; j < elem->GetSubModelCount(); j++) {
                SubModelElement *se = elem->GetSubModel(j);
                for (int l = 0; l < se->GetEffectLayerCount(); l++) {
                    EffectLayer* layer = se->GetEffectLayer(l);
                    for (int k = 0; k < layer->GetEffectCount(); k++) {
                        Effect* eff = layer->GetEffect(k);
                        if (effects[eff->GetEffectIndex()] != nullptr) {
                            effects[eff->GetEffectIndex()]->RenameTimingTrack(oldname, newname, eff);
                        }
                    }
                }
                if (se->GetType() == ELEMENT_TYPE_STRAND) {
                    StrandElement *ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); k++) {
                        NodeLayer* nlayer = ste->GetNodeLayer(k);
                        for (int l = 0; l < nlayer->GetEffectCount(); l++) {
                            Effect* eff = nlayer->GetEffect(l);
                            if (effects[eff->GetEffectIndex()] != nullptr) {
                                effects[eff->GetEffectIndex()]->RenameTimingTrack(oldname, newname, eff);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool SequenceElements::TimingIsPartOfView(TimingElement* timing, int view)
{
    std::string view_name = GetViewName(view);
    wxArrayString views = wxSplit(timing->GetViews(),',');
    for(size_t v=0;v<views.size();v++)
    {
        if( views[v] == view_name )
        {
            return true;
        }
    }
    return false;
}

std::string SequenceElements::GetViewName(int which_view) const
{
    if (_viewsManager != nullptr && _viewsManager->GetView(which_view) != nullptr)
	    return _viewsManager->GetView(which_view)->GetName();

    return "";
}

void SequenceElements::SetViewsManager(SequenceViewManager* viewsManager)
{
    _viewsManager = viewsManager;
    _viewsManager->SetSelectedView(mCurrentView);
}

void SequenceElements::SetModelsNode(wxXmlNode* node)
{
    mModelsNode = node;
}

void SequenceElements::SetEffectsNode(wxXmlNode* effectsNode)
{
    mEffectsNode = effectsNode;
}

std::string SequenceElements::GetViewModels(const std::string &viewName) const
{
    std::string result="";

    if (viewName == "Master View")
    {
        for (auto it = mAllViews[MASTER_VIEW].begin(); it != mAllViews[MASTER_VIEW].end(); ++it)
        {
            if ((*it)->GetType() == ELEMENT_TYPE_MODEL)
            {
                result += (*it)->GetName() + ",";
            }
        }
        result = result.substr(0, result.size() - 1);
    }
    else
    {
		auto view = _viewsManager->GetView(viewName);
		result = view->GetModelsString();
    }
    return result;
}

int SequenceElements::GetElementIndex(const std::string &name, int view)
{
    for (size_t i = 0; i<mAllViews[view].size(); ++i)
    {
        Element *el = mAllViews[view][i];
        if (name == el->GetFullName())
        {
            return i;
        }
    }

    return -1;
}

Element* SequenceElements::GetElement(const std::string &name)
{
    if (mAllViews.size() == 0) return nullptr;

    for(size_t i=0; i<mAllViews[MASTER_VIEW].size(); ++i)
    {
        Element *el =  mAllViews[MASTER_VIEW][i];
        if(name == el->GetFullName())
        {
            return mAllViews[MASTER_VIEW][i];
        } else if (el->GetType() == ELEMENT_TYPE_MODEL){
            ModelElement *mel = dynamic_cast<ModelElement*>(el);
            if (mel != nullptr)
            {
                for (int x = 0; x < mel->GetSubModelCount(); ++x) {
                    SubModelElement *sme = mel->GetSubModel(x);
                    if (sme != nullptr)
                    {
                        if (sme->GetFullName() == name) {
                            return sme;
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

Element* SequenceElements::GetElement(size_t index, int view)
{
    if (view < mAllViews.size() && index < mAllViews[view].size())
    {
        return mAllViews[view][index];
    }
    else
    {
        return nullptr;
    }
}

void SequenceElements::DeleteElement(const std::string &name)
{
	_viewsManager->DeleteModel(name);

    // delete element pointer from all views
    for(size_t i=0;i<mAllViews.size();i++)
    {
        for(size_t j=0;j<mAllViews[i].size();j++)
        {
            if(name == mAllViews[i][j]->GetName())
            {
                mAllViews[i].erase(mAllViews[i].begin()+j);
                IncrementChangeCount(nullptr);
                break;
            }
        }
    }

    // delete contents of pointer
    for(size_t j=0;j<mAllViews[MASTER_VIEW].size();j++)
    {
        if(name == mAllViews[MASTER_VIEW][j]->GetName())
        {
            Element *e = mAllViews[MASTER_VIEW][j];
            delete e;
            mMasterViewChangeCount++;
            break;
        }
    }
    PopulateRowInformation();
}

void SequenceElements::DeleteElementFromView(const std::string &name, int view)
{
    // delete element pointer from all views
    for(size_t j=0;j<mAllViews[view].size();j++)
    {
        if(name == mAllViews[view][j]->GetName())
        {
            mAllViews[view].erase(mAllViews[view].begin()+j);
            break;
        }
    }
    if (view == MASTER_VIEW) {
        mMasterViewChangeCount++;
    }

    PopulateRowInformation();
}

void SequenceElements::DeleteTimingFromView(const std::string &name, int view)
{
    std::string viewName = GetViewName(view);
    TimingElement* elem = dynamic_cast<TimingElement*>(GetElement(name));
    if( elem != nullptr && elem->GetType() == ELEMENT_TYPE_TIMING)
    {
        std::string views = elem->GetViews();
        wxArrayString all_views = wxSplit(views,',');
        int found = -1;
        for( size_t j = 0; j < all_views.size(); j++ )
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

void SequenceElements::DeleteTimingsFromView(int view)
{
    std::string viewName = GetViewName(view);
    for (size_t i = 0; i < mAllViews[view].size(); i++)
    {
        Element *el = mAllViews[view][i];
        if (el->GetType() == ELEMENT_TYPE_TIMING)
        {
            TimingElement* te = dynamic_cast<TimingElement*>(el);
            std::string views = te->GetViews();
            wxArrayString all_views = wxSplit(views, ',');
            int found = -1;
            for (size_t j = 0; j < all_views.size(); j++)
            {
                if (all_views[j] == viewName)
                {
                    found = j;
                    break;
                }
            }
            if (found != -1)
            {
                all_views.erase(all_views.begin() + found);
                views = wxJoin(all_views, ',');
                te->SetViews(views);
            }
        }
    }
}

void SequenceElements::RenameModelInViews(const std::string& old_name, const std::string& new_name)
{
    // renames models in any views that have been loaded for a sequence
    for(size_t view=0; view < mAllViews.size(); view++)
    {
        for(size_t i=0; i < mAllViews[view].size(); i++)
        {
            if(mAllViews[view][i]->GetName() == old_name)
            {
                mAllViews[view][i]->SetName(new_name);
            }
        }
    }
    mMasterViewChangeCount++;
}

Row_Information_Struct* SequenceElements::GetVisibleRowInformation(size_t index)
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
    for(size_t i=0;i<mVisibleRowInformation.size();i++)
    {
        if(row_number == mVisibleRowInformation[i].RowNumber)
        {
            return &mVisibleRowInformation[i];
        }
    }
    return NULL;
}

size_t SequenceElements::GetVisibleRowInformationSize()
{
    return mVisibleRowInformation.size();
}

Row_Information_Struct* SequenceElements::GetRowInformation(size_t index)
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
    for(size_t i=0;i<mRowInformation.size();i++)
    {
        if(row_number == mRowInformation[i].Index)
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
    if (mAllViews[mCurrentView].size()>1) {
        std::sort(mAllViews[mCurrentView].begin(),mAllViews[mCurrentView].end(),SortElementsByIndex);
    }
    if (mCurrentView == MASTER_VIEW) {
        mMasterViewChangeCount++;
    }
}

void SequenceElements::MoveElement(int index,int destinationIndex)
{
    IncrementChangeCount(nullptr);
    if(index<destinationIndex)
    {
        mAllViews[mCurrentView][index]->Index() = destinationIndex;
        for(int i=index+1;i<destinationIndex;i++)
        {
            mAllViews[mCurrentView][i]->Index() = i-1;
        }
    }
    else
    {
        mAllViews[mCurrentView][index]->Index() = destinationIndex;
        for(int i=destinationIndex;i<index;i++)
        {
            mAllViews[mCurrentView][i]->Index() = i+1;
        }
    }
    SortElements();
}

void SequenceElements::LoadEffects(EffectLayer *effectLayer,
                                   const std::string &type,
                                   wxXmlNode *effectLayerNode,
                                   const std::vector<std::string> & effectStrings,
                                   const std::vector<std::string> & colorPalettes) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    for(wxXmlNode* effect=effectLayerNode->GetChildren(); effect!=NULL; effect=effect->GetNext())
    {
        if (effect->GetName() == STR_EFFECT)
        {
            std::string effectName;
            std::string settings;
            int id = 0;
            long palette = -1;
            bool bProtected=false;

            // Start time
            double startTime;
            effect->GetAttribute(STR_STARTTIME).ToDouble(&startTime);
            startTime = TimeLine::RoundToMultipleOfPeriod(startTime,mFrequency);
            // End time
            double endTime;
            effect->GetAttribute(STR_ENDTIME).ToDouble(&endTime);
            endTime = TimeLine::RoundToMultipleOfPeriod(endTime,mFrequency);
            // Protected
            bProtected = effect->GetAttribute(STR_PROTECTED)=='1'?true:false;
            if(type != STR_TIMING)
            {
                // Name
                effectName = effect->GetAttribute(STR_NAME);
                // ID
                id = wxAtoi(effect->GetAttribute(STR_ID, STR_ZERO));
                if (effect->GetAttribute(STR_REF) != STR_EMPTY) {
                    int ref = wxAtoi(effect->GetAttribute(STR_REF));
                    if (ref >= effectStrings.size())
                    {
                        logger_base.warn("Effect string not found for effect %s between %d and %d. Settings ignored.", (const char *)effectName.c_str(), (int)startTime, (int)endTime);
                        settings = "";
                    }
                    else
                    {
                        settings = effectStrings[ref];
                    }
                } else {
                    settings = effect->GetNodeContent();
                }

				if (settings.find("E_FILEPICKER_Pictures_Filename") != std::string::npos)
				{
					settings = FixEffectFileParameter("E_FILEPICKER_Pictures_Filename", settings, "");
				}
				else if (settings.find("E_FILEPICKER_Glediator_Filename") != std::string::npos)
				{
					settings = FixEffectFileParameter("E_FILEPICKER_Glediator_Filename", settings, "");
				}

                wxString tmp;
                if (effect->GetAttribute(STR_PALETTE, &tmp)) {
                    tmp.ToLong(&palette);
                }
            }
            else
            {
                // store timing labels in name attribute
                effectName = effect->GetAttribute(STR_LABEL);

            }
            effectLayer->AddEffect(id,effectName,settings,
                                   palette == -1 ? STR_EMPTY : colorPalettes[palette],
                                   startTime,endTime,EFFECT_NOT_SELECTED,bProtected);
        } else if (effect->GetName() == STR_NODE && effectLayerNode->GetName() == STR_STRAND) {
            StrandElement *se = (StrandElement*)effectLayer->GetParentElement();
            EffectLayer* neffectLayer = se->GetNodeLayer(wxAtoi(effect->GetAttribute(STR_INDEX)), true);
            if (effect->GetAttribute(STR_NAME, STR_EMPTY) != STR_EMPTY) {
                ((NodeLayer*)neffectLayer)->SetName(effect->GetAttribute(STR_NAME).ToStdString());
            }

            LoadEffects(neffectLayer, type, effect, effectStrings, colorPalettes);
        }
    }

}
bool SequenceElements::LoadSequencerFile(xLightsXmlFile& xml_file, const wxString &ShowDir)
{
    mFilename = xml_file;
    wxXmlDocument& seqDocument = xml_file.GetXmlDocument();

    wxXmlNode* root=seqDocument.GetRoot();
    std::vector<std::string> effectStrings;
    std::vector<std::string> colorPalettes;
    Clear();
    supportsModelBlending = xml_file.supportsModelBlending();
    for(wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
       if (e->GetName() == "DisplayElements")
       {
            for(wxXmlNode* element=e->GetChildren(); element!=NULL; element=element->GetNext() )
            {
                bool active=false;
                bool selected=false;
                bool collapsed=false;
                std::string name = element->GetAttribute(STR_NAME).ToStdString();
                std::string type = element->GetAttribute(STR_TYPE).ToStdString();
                bool visible = element->GetAttribute("visible")=='1'?true:false;

                if (type==STR_TIMING)
                {
                    active = element->GetAttribute("active")=='1'?true:false;
                }
                else
                {
                    collapsed = element->GetAttribute("collapsed")=='1'?true:false;
                }
                if (ElementExists(name))
                {
                    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                    logger_base.warn("Duplicate "+type+": '" + name + "'. Second instance ignored.");
                    wxMessageBox("Duplicate "+type+": '" + name + "'. Second instance ignored.", _("ERROR"));
                }
                else
                {
                    Element* elem = AddElement(name, type, visible, collapsed, active, selected);
                    if (type == STR_TIMING)
                    {
                        std::string views = element->GetAttribute("views", "").ToStdString();
                        dynamic_cast<TimingElement*>(elem)->SetViews(views);
                    }
                }
            }
       }
       else if (e->GetName() == "EffectDB")
       {
           effectStrings.clear();
           for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
           {
               if(elementNode->GetName()==STR_EFFECT)
               {
				   if (elementNode->GetNodeContent().Find("E_FILEPICKER_Pictures_Filename") >= 0)
				   {
					   elementNode->SetContent(FixEffectFileParameter("E_FILEPICKER_Pictures_Filename", elementNode->GetNodeContent(), ShowDir));
				   }
				   else if (elementNode->GetNodeContent().Find("E_TEXTCTRL_Glediator_Filename") >= 0)
				   {
					   elementNode->SetContent(FixEffectFileParameter("E_TEXTCTRL_Glediator_Filename", elementNode->GetNodeContent(), ShowDir));
				   }

                   effectStrings.push_back(elementNode->GetNodeContent().ToStdString());
               }
           }
       }
       else if (e->GetName() == "ColorPalettes")
       {
           colorPalettes.clear();
           for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
           {
               if(elementNode->GetName() == STR_COLORPALETTE)
               {
                   colorPalettes.push_back(elementNode->GetNodeContent().ToStdString());
               }
           }
       }
       else if (e->GetName() == "ElementEffects")
        {
            for(wxXmlNode* elementNode=e->GetChildren(); elementNode!=NULL; elementNode=elementNode->GetNext() )
            {
                if(elementNode->GetName()==STR_ELEMENT)
                {
                    Element* element = GetElement(elementNode->GetAttribute(STR_NAME).ToStdString());
                    if (element != nullptr)
                    {
                        // check for fixed timing interval
                        int interval = 0;
                        if( elementNode->GetAttribute(STR_TYPE) == STR_TIMING )
                        {
                            interval = wxAtoi(elementNode->GetAttribute("fixed"));
                        }
                        if( interval > 0 )
                        {
                            dynamic_cast<TimingElement*>(element)->SetFixedTiming(interval);
                            EffectLayer* effectLayer = element->AddEffectLayer();
                            int time = 0;
                            int end_time = xml_file.GetSequenceDurationMS();
                            int startTime, endTime, next_time;
                            while( time <= end_time )
                            {
                                next_time = (time + interval <= end_time) ? time + interval : end_time;
                                startTime = TimeLine::RoundToMultipleOfPeriod(time,mFrequency);
                                endTime = TimeLine::RoundToMultipleOfPeriod(next_time,mFrequency);
                                effectLayer->AddEffect(0,"","","",startTime,endTime,EFFECT_NOT_SELECTED,false);
                                time += interval;
                            }
                        }
                        else
                        {
                            for(wxXmlNode* effectLayerNode=elementNode->GetChildren(); effectLayerNode!=NULL; effectLayerNode=effectLayerNode->GetNext())
                            {

                                EffectLayer* effectLayer = NULL;
                                if (effectLayerNode->GetName() == STR_EFFECTLAYER) {
                                    effectLayer = element->AddEffectLayer();
                                } else if (effectLayerNode->GetName() == STR_SUBMODEL_EFFECTLAYER) {
                                    wxString name = effectLayerNode->GetAttribute("name");
                                    int layer = wxAtoi(effectLayerNode->GetAttribute("layer", "0"));
                                    SubModelElement *se = dynamic_cast<ModelElement*>(element)->GetSubModel(name.ToStdString(), true);
                                    while (layer >= se->GetEffectLayerCount()) {
                                        se->AddEffectLayer();
                                    }
                                    effectLayer = se->GetEffectLayer(layer);
                                } else {
                                    StrandElement *se = dynamic_cast<ModelElement*>(element)->GetStrand(wxAtoi(effectLayerNode->GetAttribute(STR_INDEX)), true);
                                    int layer = wxAtoi(effectLayerNode->GetAttribute("layer", "0"));
                                    while (layer >= se->GetEffectLayerCount()) {
                                        se->AddEffectLayer();
                                    }
                                    effectLayer = se->GetEffectLayer(layer);
                                    if (effectLayerNode->GetAttribute(STR_NAME, STR_EMPTY) != STR_EMPTY) {
                                        se->SetName(effectLayerNode->GetAttribute(STR_NAME).ToStdString());
                                    }
                                }
                                if (effectLayer != nullptr) {
                                    LoadEffects(effectLayer, elementNode->GetAttribute(STR_TYPE).ToStdString(), effectLayerNode, effectStrings, colorPalettes);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (size_t x = 0; x < GetElementCount(); x++) {
        Element *el = GetElement(x);
        if (el->GetEffectLayerCount() == 0) {
            el->AddEffectLayer();
        }
    }
    return true;
}
void SequenceElements::PrepareViews(xLightsXmlFile& xml_file) {
    // Select view and set current view models as visible
    int last_view = xml_file.GetLastView();
	auto views = _viewsManager->GetViews();
    for(auto it = views.begin(); it != views.end(); ++it)
    {
        std::vector <Element*> new_view;
        mAllViews.push_back(new_view);
        int view_index = mAllViews.size()-1;
        if( view_index == last_view )
        {
            AddMissingModelsToSequence((*it)->GetModelsString());
            PopulateView((*it)->GetModelsString(), view_index);
            SetCurrentView(view_index);
        }
    }

    if (mModelsNode != nullptr) {
        PopulateRowInformation();
    }
    // Set to the first model/view
    mFirstVisibleModelRow = 0;
}

void SequenceElements::AddView(const std::string &viewName)
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
    _viewsManager->SetSelectedView(view);
}

void SequenceElements::AddMissingModelsToSequence(const std::string &models, bool visible)
{
    if(models.length()> 0)
    {
        wxArrayString model=wxSplit(models,',');
        for(size_t m=0;m<model.size();m++)
        {
            std::string modelName = model[m].ToStdString();
            Model *model1 = xframe->AllModels[modelName];
            if (model1 != nullptr) {
                if (model1->GetDisplayAs() == "SubModel") {
                    model1 = (dynamic_cast<SubModel*>(model1))->GetParent();
                }
                if (model1 != nullptr)
                {
                    if (!ElementExists(model1->GetName()))
                    {
                        Element* elem = AddElement(model1->GetName(), "model", visible, false, false, false);
                        if (elem != nullptr)
                        {
                            elem->AddEffectLayer();
                        }
                    }
                }
            }
        }
    }
}

void SequenceElements::SetTimingVisibility(const std::string& name)
{
    for(size_t i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        Element* elem = mAllViews[MASTER_VIEW][i];
        if( elem->GetType() != ELEMENT_TYPE_TIMING )
        {
            break;
        }
        TimingElement *te = dynamic_cast<TimingElement*>(elem);
        if( name == "Master View" )
        {
            te->SetVisible(true);
        }
        else
        {
            te->SetVisible(false);
            wxArrayString views = wxSplit(te->GetViews(),',');
            for(size_t v=0;v<views.size();v++)
            {
                std::string viewName = views[v].ToStdString();
                if( name == viewName )
                {
                    te->SetVisible(true);
                    break;
                }
            }
        }
    }
}

void SequenceElements::AddTimingToAllViews(const std::string& timing)
{
	auto views = _viewsManager->GetViews();
	for (auto it = views.begin(); it != views.end(); ++it)
    {
        AddTimingToView(timing, (*it)->GetName());
    }
}

void SequenceElements::AddTimingToCurrentView(const std::string& timing)
{
    if (mCurrentView != MASTER_VIEW) // dont add it to master
    {
        AddTimingToView(timing, _viewsManager->GetView(mCurrentView)->GetName());
    }
}

int SequenceElements::GetViewCount()
{
    return _viewsManager->GetViewCount();
}

void SequenceElements::AddViewToTimings(std::vector<std::string>& timings, const std::string& name)
{
    for( size_t i = 0; i < timings.size(); i++ )
    {
        AddTimingToView(timings[i], name);
    }
}

void SequenceElements::AddTimingToView(const std::string& timing, const std::string& name)
{
    Element* elem = GetElement(timing);
    TimingElement *te = dynamic_cast<TimingElement*>(elem);
    if( elem != nullptr && elem->GetType() == ELEMENT_TYPE_TIMING )
    {
        std::string views = te->GetViews();
        wxArrayString all_views = wxSplit(views,',');
        bool found = false;
        for( size_t j = 0; j < all_views.size(); j++ )
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
            te->SetViews(views);
        }
    }
}

void SequenceElements::PopulateView(const std::string &models, int view)
{
    if (view >= mAllViews.size()) return;

    mAllViews[view].clear();

    if(models.length()> 0)
    {
        wxArrayString model=wxSplit(models,',');
        for(size_t m=0;m<model.size();m++)
        {
            std::string modelName = model[m].ToStdString();
            Element* elem = GetElement(modelName);
            if (elem != nullptr) {
                mAllViews[view].push_back(elem);
            }
        }
    }
    if (view == MASTER_VIEW) {
        mMasterViewChangeCount++;
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

int SequenceElements::GetMinPeriod()
{
    return (int)((double)1000.0/(double)mFrequency);
}

void SequenceElements::SetSelectedTimingRow(int row)
{
    mSelectedTimingRow = row;
}

int SequenceElements::GetSelectedTimingRow()
{
    return mSelectedTimingRow;
}

wxXmlNode *GetModelNode(wxXmlNode *root, const std::string & name) {
    wxXmlNode* e;
    for(e=root->GetChildren(); e!=nullptr; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            if (name == e->GetAttribute(STR_NAME)) return e;
        }
    }
    return NULL;
}

void addSubModelElement(SubModelElement *elem,
                        std::vector<Row_Information_Struct> &mRowInformation,
                        int &rowIndex, xLightsFrame *xframe,
                        std::vector <Element*> &elements) {
    if(!elem->GetCollapsed())
    {
        for(int j =0; j<elem->GetEffectLayerCount();j++)
        {
            Row_Information_Struct ri;
            ri.element = elem;
            ri.displayName = elem->GetFullName();
            ri.Collapsed = elem->GetCollapsed();
            ri.colorIndex = 0;
            ri.layerIndex = j;
            ri.Index = rowIndex++;
            ri.submodel = true;
            mRowInformation.push_back(ri);
        }
    }
    else
    {
        Row_Information_Struct ri;
        ri.element = elem;
        ri.Collapsed = elem->GetCollapsed();
        ri.displayName = elem->GetFullName();
        ri.colorIndex = 0;
        ri.layerIndex = 0;
        ri.Index = rowIndex++;
        ri.submodel = true;
        mRowInformation.push_back(ri);
    }
}
void addModelElement(ModelElement *elem, std::vector<Row_Information_Struct> &mRowInformation,
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
        ri.colorIndex = 0;
        ri.layerIndex = 0;
        ri.Index = rowIndex++;
        ri.submodel = submodel;
        mRowInformation.push_back(ri);
    }
    Model *cls = xframe->GetModel(elem->GetModelName());
    if (cls == nullptr) {
        return;
    }
    elem->Init(*cls);
    if (cls->GetDisplayAs() == "ModelGroup" && elem->ShowStrands()) {
        ModelGroup *grp = dynamic_cast<ModelGroup*>(cls);
        for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); it++) {
            std::string modelName = *it;
            std::string subModel = "";
            int slsh = modelName.find('/');
            if (slsh != -1) {
                subModel = modelName.substr(slsh + 1);
                modelName = modelName.substr(0, slsh);
            }
            for (size_t x = 0; x < elements.size(); x++) {
                if (elements[x]->GetModelName() == modelName) {
                    ModelElement *melem = dynamic_cast<ModelElement*>(elements[x]);
                    if (subModel != "") {
                        SubModelElement *selem = melem->GetSubModel(subModel);
                        addSubModelElement(selem, mRowInformation, rowIndex, xframe, elements);
                    } else {
                        addModelElement(melem, mRowInformation, rowIndex, xframe, elements, true);
                    }
                }
            }
        }
    } else if (elem->ShowStrands()) {
        for (size_t s = 0; s < elem->GetSubModelCount(); s++) {
            SubModelElement *se = elem->GetSubModel(s);
            int m = se->GetEffectLayerCount();
            if (se->GetCollapsed()) {
                m = 1;
            }
            for (int x = 0; x < m; x++) {
                Row_Information_Struct ri;
                ri.element = se;
                ri.Collapsed = se->GetCollapsed();
                ri.displayName = se->GetName();

                ri.colorIndex = 0;
                ri.layerIndex = x;
                ri.Index = rowIndex++;
                ri.strandIndex = -1;
                if (se->GetType() == ELEMENT_TYPE_STRAND) {
                    ri.strandIndex = ((StrandElement*)se)->GetStrand();
                }
                ri.submodel = submodel;
                mRowInformation.push_back(ri);
            }

            if (se->GetType() == ELEMENT_TYPE_STRAND && ((StrandElement*)se)->ShowNodes()) {
                StrandElement *ste = dynamic_cast<StrandElement*>(se);
                for (int n = 0; n < ste->GetNodeLayerCount(); n++) {
                    Row_Information_Struct ri;
                    ri.element = se;
                    ri.Collapsed = ste->ShowNodes();
                    ri.displayName = ste->GetNodeLayer(n)->GetName();
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

void SequenceElements::addTimingElement(TimingElement *elem, std::vector<Row_Information_Struct> &mRowInformation,
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
            ri.colorIndex = timingColorIndex;
            ri.layerIndex = j;
            if(selectedTimingRow<0 && j==0)
            {
                selectedTimingRow = elem->GetActive()?rowIndex:-1;
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
        ri.colorIndex = timingColorIndex;
        ri.layerIndex = 0;
        if(selectedTimingRow<0)
        {
            selectedTimingRow = elem->GetActive()?rowIndex:-1;
        }
        ri.Index = rowIndex++;
        mRowInformation.push_back(ri);
        timingRowCount++;
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

    for(size_t i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        Element* elem = mAllViews[MASTER_VIEW][i];
		if (elem != NULL)
		{
			if (elem->GetType() == ELEMENT_TYPE_TIMING)
			{
                if (mCurrentView == MASTER_VIEW || TimingIsPartOfView(dynamic_cast<TimingElement*>(elem), mCurrentView))
				{
					addTimingElement(dynamic_cast<TimingElement*>(elem), mRowInformation, rowIndex, mSelectedTimingRow, mTimingRowCount, timingColorIndex);
				}
			}
		}
    }

    for(size_t i=0;i<mAllViews[mCurrentView].size();i++)
    {
        Element* elem = mAllViews[mCurrentView][i];
		if (elem != NULL)
		{
            if (elem->GetVisible() && elem->GetType() == ELEMENT_TYPE_MODEL) {
                addModelElement(dynamic_cast<ModelElement*>(elem), mRowInformation, rowIndex, xframe, mAllViews[MASTER_VIEW], false);
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

int SequenceElements::GetNumberOfTimingElements() {
    int count = 0;
    for(size_t i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        if(mAllViews[MASTER_VIEW][i]->GetType()==ELEMENT_TYPE_TIMING)
        {
            count++;
        }
    }
    return count;
}

int SequenceElements::GetNumberOfActiveTimingEffects()
{
    int num_timing_effects = 0;
    EffectLayer* tel = GetVisibleEffectLayer(GetSelectedTimingRow());
    if( tel != nullptr )
    {
        num_timing_effects = tel->GetEffectCount();
    }
    return num_timing_effects;
}

void SequenceElements::DeactivateAllTimingElements()
{
    for(size_t i=0;i<mAllViews[mCurrentView].size();i++)
    {
        if(mAllViews[mCurrentView][i]->GetType()==ELEMENT_TYPE_TIMING)
        {
            dynamic_cast<TimingElement*>(mAllViews[mCurrentView][i])->SetActive(false);
        }
    }
}

int SequenceElements::SelectEffectsInRowAndTimeRange(int startRow, int endRow, int startMS,int endMS)
{
    int num_selected = 0;
    if(startRow<mRowInformation.size())
    {
        if(endRow>=mRowInformation.size())
        {
            endRow = mRowInformation.size()-1;
        }
        for(int i=startRow;i<=endRow;i++)
        {
            EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
            num_selected += effectLayer->SelectEffectsInTimeRange(startMS,endMS);
        }
    }
    return num_selected;
}

int SequenceElements::SelectVisibleEffectsInRowAndTimeRange(int startRow, int endRow, int startMS,int endMS)
{
    int num_selected = 0;
    if(startRow<mVisibleRowInformation.size())
    {
        if(endRow>=mVisibleRowInformation.size())
        {
            endRow = mVisibleRowInformation.size()-1;
        }
        for(int i=startRow;i<=endRow;i++)
        {
            EffectLayer* effectLayer = GetEffectLayer(&mVisibleRowInformation[i]);
            num_selected += effectLayer->SelectEffectsInTimeRange(startMS,endMS);
        }
    }
    return num_selected;
}

int SequenceElements::SelectEffectsInRowAndColumnRange(int startRow, int endRow, int startCol,int endCol)
{
    int num_selected = 0;
    EffectLayer* tel = GetVisibleEffectLayer(GetSelectedTimingRow());
    if( tel != nullptr )
    {
        Effect* eff1 = tel->GetEffect(startCol);
        Effect* eff2 = tel->GetEffect(endCol);
        if( eff1 != nullptr && eff2 != nullptr )
        {
            int startMS = eff1->GetStartTimeMS();
            int endMS = eff2->GetEndTimeMS();
            num_selected = SelectEffectsInRowAndTimeRange(startRow, endRow, startMS, endMS);
        }
    }
    return num_selected;
}

void SequenceElements::UnSelectAllEffects()
{
    for(size_t i=0;i<mRowInformation.size();i++)
    {
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        effectLayer->UnSelectAllEffects();
    }
}

void SequenceElements::UnSelectAllElements()
{
    for(size_t i=0;i<mAllViews[mCurrentView].size();i++)
    {
        if(mAllViews[mCurrentView][i]->GetType() == ELEMENT_TYPE_MODEL) {
            dynamic_cast<ModelElement*>(mAllViews[mCurrentView][i])->SetSelected(false);
        }
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
    for(size_t i=0;i<GetElementCount(view);i++)
    {
        Element * e = GetElement(i, view);
        e->SetVisible(visibility);
    }
}

int SequenceElements::GetIndexOfModelFromModelIndex(int modelIndex)
{
    int count = 0;

    for (int i = 0; i < GetElementCount(); ++i)
    {
        if (GetElement(i)->GetType() == ELEMENT_TYPE_TIMING)
        {
        }
        else
        {
            if (count == modelIndex)
            {
                return i;
            }
            count++;
        }
    }

    return -1;
}

void SequenceElements::MoveSequenceElement(int index, int dest, int view)
{
    IncrementChangeCount(nullptr);

    if(index < mAllViews[view].size() && dest < mAllViews[view].size())
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
    else if (index < mAllViews[view].size())
    {
        Element* e = mAllViews[view][index];
        mAllViews[view].erase(mAllViews[view].begin() + index);
        mAllViews[view].push_back(e);
    }

    if (view == MASTER_VIEW) {
        mMasterViewChangeCount++;
    }
}

void SequenceElements::MoveElementUp(const std::string &name, int view)
{
    IncrementChangeCount(nullptr);

    for(size_t i=0;i<mAllViews[view].size();i++)
    {
        if(name == mAllViews[view][i]->GetModelName())
        {
            // found element
            if( i > 0 )
            {
                MoveSequenceElement(i, i-1, view);
            }
            break;
        }
    }
    if (view == MASTER_VIEW) {
        mMasterViewChangeCount++;
    }
}

void SequenceElements::MoveElementDown(const std::string &name, int view)
{
    IncrementChangeCount(nullptr);

    for(size_t i=0;i<mAllViews[view].size();i++)
    {
        if(name == mAllViews[view][i]->GetModelName())
        {
            // found element
            if( i < mAllViews[view].size()-1 )
            {
                MoveSequenceElement(i+1, i, view);
            }
            break;
        }
    }
    if (view == MASTER_VIEW) {
        mMasterViewChangeCount++;
    }
}

void SequenceElements::ImportLyrics(TimingElement* element, wxWindow* parent)
{
    LyricsDialog* dlgLyrics = new LyricsDialog(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    if (dlgLyrics->ShowModal() == wxID_OK)
    {
        element->SetFixedTiming(0);
        // remove all existing layers from timing track
        int num_layers = element->GetEffectLayerCount();
        for( int k = num_layers-1; k >= 0; k-- )
        {
            element->RemoveEffectLayer(k);
        }
        EffectLayer* phrase_layer = element->AddEffectLayer();

        int total_num_phrases = dlgLyrics->TextCtrlLyrics->GetNumberOfLines();
        int num_phrases = total_num_phrases;
        for( int i = 0; i < dlgLyrics->TextCtrlLyrics->GetNumberOfLines(); i++ )
        {
            if( dlgLyrics->TextCtrlLyrics->GetLineText(i).length() == 0)
            {
                num_phrases--;
            }
        }
        int start_time = 0;
        int end_time = mSequenceEndMS;
        int interval_ms = (end_time-start_time) / num_phrases;
        for( int i = 0; i < total_num_phrases; i++ )
        {
            wxString line = dlgLyrics->TextCtrlLyrics->GetLineText(i);
            // Handle common unicode characters before falling back to ascii
            line.Replace(wxT("\u2019"),"'",true);
            line.Replace(wxT("\u0218"),"'",true);
            line.Replace(wxT("\u201c"),'"',true);
            line.Replace(wxT("\u201d"),'"',true);
            line = line.ToStdString();
            if(line == "")
            {
                line = dlgLyrics->TextCtrlLyrics->GetLineText(i).ToAscii();
                line.Replace("_","'",true);
            }
            if( line != "" )
            {
                xframe->dictionary.InsertSpacesAfterPunctuation(line);
                end_time = TimeLine::RoundToMultipleOfPeriod(start_time+interval_ms, GetFrequency());
                phrase_layer->AddEffect(0,line.ToStdString(),"","",start_time,end_time,EFFECT_NOT_SELECTED,false);
                start_time = end_time;
            }
        }
    }
}

void SequenceElements::BreakdownPhrase(EffectLayer* word_layer, int start_time, int end_time, const std::string& phrase)
{
    if( phrase != "" )
    {
        xframe->dictionary.LoadDictionaries(xframe->CurrentDir);
        wxArrayString words = wxSplit(phrase, ' ');
        int num_words = words.Count();
        double interval_ms = (end_time-start_time) / num_words;
        int word_start_time = start_time;
        for( int i = 0; i < num_words; i++ )
        {
            int word_end_time = TimeLine::RoundToMultipleOfPeriod(start_time+(interval_ms * (i + 1)), GetFrequency());
            if( i == num_words - 1  || word_end_time > end_time)
            {
                word_end_time = end_time;
            }
            word_layer->AddEffect(0,words[i].ToStdString(),"","",word_start_time,word_end_time,EFFECT_NOT_SELECTED,false);
            word_start_time = word_end_time;
        }
    }
}

void SequenceElements::BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time, const std::string& word)
{
    xframe->dictionary.LoadDictionaries(xframe->CurrentDir);
    wxArrayString phonemes;
    xframe->dictionary.BreakdownWord(word, phonemes);
    if( phonemes.Count() > 0 )
    {
        int phoneme_start_time = start_time;
        double phoneme_interval_ms = (end_time-start_time) / phonemes.Count();
        for (size_t i = 0; i < phonemes.Count(); i++ )
        {
            int phoneme_end_time = TimeLine::RoundToMultipleOfPeriod(start_time+(phoneme_interval_ms*(i + 1)), GetFrequency());
            if( i == phonemes.Count() - 1 || phoneme_end_time > end_time)
            {
                phoneme_end_time = end_time;
            }
            phoneme_layer->AddEffect(0,phonemes[i].ToStdString(),"","",phoneme_start_time,phoneme_end_time,EFFECT_NOT_SELECTED,false);
            phoneme_start_time = phoneme_end_time;
        }
    }
}

void SequenceElements::IncrementChangeCount(Element *el) {
    mChangeCount++;
    if (el != nullptr && el->GetType() == ELEMENT_TYPE_TIMING) {
        //need to check if we need to have some models re-rendered due to timing being changed
        std::unique_lock<std::mutex> locker(renderDepLock);
        std::map<std::string, std::set<std::string>>::iterator it = renderDependency.find(el->GetModelName());
        if (it != renderDependency.end()) {
            int origChangeCount, ss, es;
            el->GetAndResetDirtyRange(origChangeCount, ss, es);
            for (std::set<std::string>::iterator sit = it->second.begin(); sit != it->second.end(); sit++) {
                Element *el = this->GetElement(*sit);
                if (el != nullptr) {
                    el->IncrementChangeCount(ss, es);
                    modelsToRender.insert(*sit);
                }
            }
        }
    }
}
bool SequenceElements::GetElementsToRender(std::vector<Element *> &models) {
    std::unique_lock<std::mutex> locker(renderDepLock);
    if (!modelsToRender.empty()) {
        for (std::set<std::string>::iterator sit = modelsToRender.begin(); sit != modelsToRender.end(); sit++) {
            Element *el = this->GetElement(*sit);
            if (el != nullptr) {
                models.push_back(el);
            }
        }
        modelsToRender.clear();
        return !models.empty();
    }
    return false;
}

void SequenceElements::AddRenderDependency(const std::string &layer, const std::string &model) {
    std::unique_lock<std::mutex> locker(renderDepLock);
    renderDependency[layer].insert(model);
}

EffectManager &SequenceElements::GetEffectManager() {
    return xframe->GetEffectManager();
}


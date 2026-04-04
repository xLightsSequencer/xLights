/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>

#include <algorithm>
#include <format>

#include "SequenceElements.h"
#include "pugixml.hpp"
#include "RenderUtils.h"
#include "../ui/sequencer/TimeLine.h"
#include "../xLightsMain.h"
#include "SequenceFile.h"
#include "../effects/RenderableEffect.h"
#include "../models/SubModel.h"
#include "../models/ModelGroup.h"
#include "UtilFunctions.h"
#include "../utils/DisplayMessages.h"
#include "../utils/string_utils.h"
#include "ui/sequencer/SequenceViewManager.h"
#include "ui/media/JukeboxPanel.h"
#include "../utils/TraceLog.h"

#include <log.h>

static const std::string STR_EFFECT("Effect");
static const std::string STR_EFFECTLAYER("EffectLayer");
static const std::string STR_SUBMODEL_EFFECTLAYER("SubModelEffectLayer");
static const std::string STR_NODE("Node");
static const std::string STR_STRAND("Strand");
static const std::string STR_TIMING("timing");

SequenceElements::SequenceElements(RenderContext *ctx)
    : renderContext(ctx), mFrequency(20), mSequenceEndMS(0), undo_mgr(this)
{
    _viewsManager = nullptr;
    mSelectedTimingRow = -1;
    mTimingRowCount = 0;
    mMaxRowsDisplayed = 0;
    mFirstVisibleModelRow = 0;
    mChangeCount = 0;
    mMasterViewChangeCount = 0;
    mCurrentView = 0;
    std::vector<Element*> master_view;
    mAllViews.push_back(master_view); // first view must remain as master view that determines render order
    hasPapagayoTiming = false;
    supportsModelBlending = true;
    _timeLine = nullptr;
}

xLightsFrame* SequenceElements::GetXLightsFrame() const {
    return dynamic_cast<xLightsFrame*>(renderContext);
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
    mSequenceMedia.Clear();
    mColorPalettes.clear();
    mCurrentView = 0;
    supportsModelBlending = true;
    std::vector <Element*> master_view;
    mAllViews.push_back(master_view);
    hasPapagayoTiming = false;
    if (GetTimeLine() != nullptr)
    {
        GetTimeLine()->ClearTags();
    }
}

void SequenceElements::SetSequenceEnd(int ms)
{
    mSequenceEndMS = ms;
}

int SequenceElements::GetSequenceEnd() const
{
    return mSequenceEndMS;
}

EffectLayer* SequenceElements::GetEffectLayer(const Row_Information_Struct *s) const
{
    if (s == nullptr) {
        return nullptr;
    }
    Element* e = s->element;
    if (s->strandIndex == -1) {
        return e->GetEffectLayer(s->layerIndex);
    }
    else if (s->nodeIndex == -1) {
        SubModelElement *se = dynamic_cast<SubModelElement*>(e);
        if (se == nullptr) {
            spdlog::error("Expected a SubModelElment be found {}", (int)e->GetType());
            return nullptr;
        }
        return se->GetEffectLayer(s->layerIndex);
    }
    else {
        StrandElement *me = dynamic_cast<StrandElement*>(e);
        if (me == nullptr) {
            spdlog::error("Expected a StrandElement be found {}", (int)e->GetType());
            return nullptr;
        }
        return me->GetNodeLayer(s->nodeIndex);
    }
}

EffectLayer* SequenceElements::GetEffectLayer(int row) {
    if (row == -1) return nullptr;
    return GetEffectLayer(GetRowInformation(row));
}

EffectLayer* SequenceElements::GetVisibleEffectLayer(int row) {
    if (row == -1) return nullptr;
    return GetEffectLayer(GetVisibleRowInformation(row));
}

static Element* CreateElement(SequenceElements *se, const std::string &name, const std::string &type,
    bool visible, bool collapsed, bool active, bool selected,
    RenderContext *ctx, bool renderDisabled) {
    Element *el;
    if (type == "timing") {
        TimingElement *te = new TimingElement(se, name);
        el = te;
        te->SetActive(active);
        te->SetMasterVisible(visible);
        el->SetVisible(te->GetMasterVisible());
    } else {
        ModelElement *me = new ModelElement(se, name, selected);
        if (ctx != nullptr) {
            Model *model = ctx->GetModel(name);
            if (model != nullptr) {
                me->Init(*model);
            }
        }
        el = me;
        el->SetVisible(visible);
    }
    el->SetCollapsed(collapsed);
    el->SetRenderDisabled(renderDisabled);
    return el;
}

Element* SequenceElements::AddElement(const std::string &name, const std::string &type,
    bool visible, bool collapsed, bool active, bool selected, bool renderDisabled)
{
    if (!ElementExists(name)) {
        Element *el = CreateElement(this, name, type, visible, collapsed, active, selected, renderContext, renderDisabled);

        mAllViews[MASTER_VIEW].push_back(el);
        mMasterViewChangeCount++;
        IncrementChangeCount(el);
        return el;
    }
    spdlog::error("SequenceElements::AddElement {} failed.", name);
    return nullptr;
}

Element* SequenceElements::AddElement(int index, const std::string &name,
    const std::string &type,
    bool visible, bool collapsed, bool active, bool selected, bool renderDisabled)
{
    if (!ElementExists(name) && index >= 0 && static_cast<size_t>(index) <= mAllViews[MASTER_VIEW].size())
    {
        Element *el = CreateElement(this, name, type, visible, collapsed, active, selected, renderContext, renderDisabled);
        mAllViews[MASTER_VIEW].insert(mAllViews[MASTER_VIEW].begin() + index, el);
        mMasterViewChangeCount++;
        IncrementChangeCount(el);
        return el;
    }
    spdlog::error("SequenceElements::AddElement #2 {} failed.", name);
    return nullptr;
}

size_t SequenceElements::GetElementCount(int view) const
{
    if (view < 0 || static_cast<size_t>(view) >= mAllViews.size()) return 0;

    return mAllViews[view].size();
}

bool SequenceElements::ElementExists(const std::string& elementName, int view)
{
    if (view < 0 || static_cast<size_t>(view) >= mAllViews.size())
        return false;

    for (size_t i = 0; i < mAllViews[view].size(); ++i) {
        if (mAllViews[view][i]->GetName() == elementName) {
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
    for (size_t x = 0; x < GetEffectManager().size(); ++x) {
        RenderableEffect *eff = GetEffectManager()[x];
        effects[x] = eff;
    }

    for (size_t i = 0; i < GetElementCount(); ++i) {
        Element* e = GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement *elem = dynamic_cast<ModelElement *>(e);
            for (size_t j = 0; j < elem->GetEffectLayerCount(); ++j) {
                EffectLayer* layer = elem->GetEffectLayer(j);
                for (int k = 0; k < layer->GetEffectCount(); ++k) {
                    Effect* eff = layer->GetEffect(k);
                    if (eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                        effects[eff->GetEffectIndex()]->RenameTimingTrack(oldname, newname, eff);
                    }
                }
            }
            for (int j = 0; j < elem->GetSubModelAndStrandCount(); ++j) {
                SubModelElement *se = elem->GetSubModel(j);
                for (size_t l = 0; l < se->GetEffectLayerCount(); ++l) {
                    EffectLayer* layer = se->GetEffectLayer(l);
                    for (int k = 0; k < layer->GetEffectCount(); ++k) {
                        Effect* eff = layer->GetEffect(k);
                        if (eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                            effects[eff->GetEffectIndex()]->RenameTimingTrack(oldname, newname, eff);
                        }
                    }
                }
                if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    StrandElement *ste = dynamic_cast<StrandElement*>(se);
                    for (int k = 0; k < ste->GetNodeLayerCount(); k++) {
                        NodeLayer* nlayer = ste->GetNodeLayer(k);
                        for (int l = 0; l < nlayer->GetEffectCount(); l++) {
                            Effect* eff = nlayer->GetEffect(l);
                            if (eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                                effects[eff->GetEffectIndex()]->RenameTimingTrack(oldname, newname, eff);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool SequenceElements::TimingIsPartOfView(TimingElement* timing, int view) const
{
    std::string view_name = GetViewName(view);
    auto views = Split(timing->GetViews(), ',');
    for (size_t v = 0; v < views.size(); v++)
    {
        if (views[v] == view_name)
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


std::string SequenceElements::GetViewModels(const std::string &viewName) const
{
    std::string result="";

    if (viewName == "Master View")
    {
        for (auto it = mAllViews[MASTER_VIEW].begin(); it != mAllViews[MASTER_VIEW].end(); ++it)
        {
            if ((*it)->GetType() == ElementType::ELEMENT_TYPE_MODEL)
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
    for (size_t i = 0; i < mAllViews[view].size(); ++i)
    {
        Element *el = mAllViews[view][i];
        if (name == el->GetFullName())
        {
            return i;
        }
    }

    return -1;
}

Element* SequenceElements::GetElement(const std::string &name) const
{
    if (mAllViews.size() == 0) return nullptr;

    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); ++i)
    {
        Element *el = mAllViews[MASTER_VIEW][i];
        if (name == el->GetFullName())
        {
            return mAllViews[MASTER_VIEW][i];
        }
        else if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* mel = dynamic_cast<ModelElement*>(el);
            if (mel != nullptr)
            {
                for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                    SubModelElement* sme = mel->GetSubModel(x);
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

Element* SequenceElements::GetElement(size_t index, int view) const
{
    if (view >= 0 && static_cast<size_t>(view) < mAllViews.size() && index < mAllViews[view].size())
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
    for (size_t i = 1; i < mAllViews.size(); i++)
    {
        for (size_t j = 0; j < mAllViews[i].size(); j++)
        {
            if (name == mAllViews[i][j]->GetName())
            {
                mAllViews[i].erase(mAllViews[i].begin() + j);
                IncrementChangeCount(nullptr);
                break;
            }
        }
    }

    // delete contents of pointer
    for (size_t j = 0; j < mAllViews[MASTER_VIEW].size(); j++)
    {
        if (name == mAllViews[MASTER_VIEW][j]->GetName())
        {
            Element *e = mAllViews[MASTER_VIEW][j];
            mAllViews[MASTER_VIEW].erase(mAllViews[MASTER_VIEW].begin() + j);
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
    for (size_t j = 0; j < mAllViews[view].size(); j++)
    {
        if (name == mAllViews[view][j]->GetName())
        {
            mAllViews[view].erase(mAllViews[view].begin() + j);
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
    if (elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING)
    {
        std::string views = elem->GetViews();
        auto all_views = Split(views, ',');
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
            views = Join(all_views, ",");
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
        if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            TimingElement* te = dynamic_cast<TimingElement*>(el);
            std::string views = te->GetViews();
            auto all_views = Split(views, ',');
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
                views = Join(all_views, ",");
                te->SetViews(views);
            }
        }
    }
}

void SequenceElements::RenameModelInViews(const std::string& old_name, const std::string& new_name)
{
    // renames models in any views that have been loaded for a sequence
    for (size_t view = 0; view < mAllViews.size(); view++)
    {
        for (size_t i = 0; i < mAllViews[view].size(); i++)
        {
            if (mAllViews[view][i]->GetName() == old_name)
            {
                mAllViews[view][i]->SetName(new_name);
            }
        }
    }
    mMasterViewChangeCount++;
}

Row_Information_Struct* SequenceElements::GetVisibleRowInformation(size_t index)
{
    if (index < mVisibleRowInformation.size())
    {
        return &mVisibleRowInformation[index];
    }
    else
    {
        return nullptr;
    }
}

Row_Information_Struct* SequenceElements::GetVisibleRowInformationFromRow(int row_number)
{
    for (size_t i = 0; i < mVisibleRowInformation.size(); i++)
    {
        if (row_number == mVisibleRowInformation[i].RowNumber)
        {
            return &mVisibleRowInformation[i];
        }
    }
    return nullptr;
}

size_t SequenceElements::GetVisibleRowInformationSize()
{
    return mVisibleRowInformation.size();
}

Row_Information_Struct* SequenceElements::GetRowInformation(size_t index)
{
    if (index < mRowInformation.size())
    {
        return &mRowInformation[index];
    }
    else
    {
        return nullptr;
    }
}

Row_Information_Struct* SequenceElements::GetRowInformationFromRow(int row_number)
{
    for (size_t i = 0; i < mRowInformation.size(); i++)
    {
        if (row_number == mRowInformation[i].Index)
        {
            return &mRowInformation[i];
        }
    }
    return nullptr;
}

std::string SequenceElements::UniqueElementName(const std::string& basename) const
{
    Element* timing = GetElement(basename);

    if (timing == nullptr) return basename;

    int i = 1;
    for (;;)
    {
        std::string newName = basename + "-" + std::to_string(i);
        timing = GetElement(newName);
        if (timing == nullptr) return newName;
        i++;
    }
}

int SequenceElements::GetRowInformationSize()
{
    return mRowInformation.size();
}

void SequenceElements::SortElements()
{
    if (mAllViews[mCurrentView].size() > 1) {
        std::sort(mAllViews[mCurrentView].begin(), mAllViews[mCurrentView].end(), SortElementsByIndex);
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

int SequenceElements::LoadEffects(EffectLayer* effectLayer,
    const std::string& type,
    const pugi::xml_node& effectLayerNode,
    const std::vector<std::string>& effectStrings,
    const std::vector<std::string>& colorPalettes,
    bool importing)
{

    int loaded = 0;
    for (auto effect : effectLayerNode.children()) {
        std::string effectNodeName = effect.name();
        if (effectNodeName == STR_EFFECT) {
            std::string effectName;
            std::string settings;
            int id = 0;
            long palette = -1;

            double startTime = std::strtod(effect.attribute("startTime").as_string("0"), nullptr);
            startTime = RoundToMultipleOfPeriod(startTime, mFrequency);
            double endTime = std::strtod(effect.attribute("endTime").as_string("0"), nullptr);
            endTime = RoundToMultipleOfPeriod(endTime, mFrequency);

            if (startTime >= endTime) {
                spdlog::warn("Effect dropped as its start time was greater than or equal to its end time : '{}' : {} Layer {} Start {} End {}.",
                    effect.attribute("name").as_string(""),
                    effectLayer->GetParentElement()->GetName(),
                    effectLayer->GetLayerNumber(),
                    (int)(startTime / 1000),
                    (int)(endTime / 1000));
            } else {
                bool bProtected = effect.attribute("protected").as_bool(false);
                if (type != STR_TIMING) {
                    effectName = effect.attribute("name").as_string("");
                    id = effect.attribute("id").as_int(0);
                    std::string refStr = effect.attribute("ref").as_string("");
                    if (!refStr.empty()) {
                        int ref = std::strtol(refStr.c_str(), nullptr, 10);
                        if (ref >= (int)effectStrings.size()) {
                            spdlog::warn("Effect string not found for effect {} between {} and {}. Settings ignored.", effectName, (int)startTime, (int)endTime);
                            settings = "";
                        } else {
                            settings = effectStrings[ref];
                        }
                    } else {
                        settings = effect.text().as_string("");
                    }

                    if (settings.find("E_FILEPICKER_Glediator_Filename") != std::string::npos) {
                        settings = FixEffectFileParameter("E_FILEPICKER_Glediator_Filename", settings, "");
                    }

                    std::string palStr = effect.attribute("palette").as_string("");
                    if (!palStr.empty()) {
                        palette = std::strtol(palStr.c_str(), nullptr, 10);
                    }
                } else {
                    effectName = UnXmlSafe(effect.attribute("label").as_string(""));
                }
                std::string pal;
                if (palette != -1) {
                    if (palette >= (long)colorPalettes.size()) {
                        spdlog::warn("Color palette not found for effect {} between {} and {}. Palette ignored.",
                            effectName, (int)startTime, (int)endTime);
                        pal = "";
                    } else {
                        pal = colorPalettes[palette];
                    }
                }
                if (effectName != "Random") {
                    effectLayer->AddEffect(id, effectName, settings, pal,
                                           startTime, endTime, EFFECT_NOT_SELECTED, bProtected, false, importing);
                } else {
                    spdlog::warn("Random effect not loaded on element {} layer {} ({:.2f}-{:.2f})", effectLayer->GetParentElement()->GetName(), effectLayer->GetLayerNumber(), startTime / 1000, endTime / 1000);
                }
            }
        } else if (effectNodeName == STR_NODE && strcmp(effectLayerNode.name(), STR_STRAND.c_str()) == 0) {
            StrandElement* se = (StrandElement*)effectLayer->GetParentElement();
            EffectLayer* neffectLayer = se->GetNodeLayer(effect.attribute("index").as_int(0), true);
            std::string nodeName = effect.attribute("name").as_string("");
            if (!nodeName.empty()) {
                ((NodeLayer*)neffectLayer)->SetNodeName(nodeName);
            }
            LoadEffects(neffectLayer, type, effect, effectStrings, colorPalettes);
        }
        loaded++;
    }
    return loaded;
}

bool SequenceElements::LoadSequencerFile(SequenceFile& xml_file, pugi::xml_document& doc, const std::string& ShowDir, bool importing)
{

    renderDependency.clear();

    auto root = doc.child("xsequence");
    if (!root) root = doc.first_child();

    std::vector<std::string> effectStrings;
    std::vector<std::string> colorPalettes;
    const std::string& showDir = ShowDir;
    TraceLog::AddTraceMessage("About to clear sequence");
    Clear();
    TraceLog::AddTraceMessage("   Cleared");
    supportsModelBlending = xml_file.supportsModelBlending();

    for (auto e : root.children()) {
        std::string ename = e.name();
        TraceLog::PushTraceContext();
        TraceLog::AddTraceMessage("Processing " + ename);

        if (ename == "DisplayElements") {
            for (auto element : e.children()) {
                bool active = false;
                bool selected = false;
                bool collapsed = false;
                std::string name = element.attribute("name").as_string("");
                // trim whitespace
                while (!name.empty() && (name.front() == ' ' || name.front() == '\t')) name.erase(name.begin());
                while (!name.empty() && (name.back() == ' ' || name.back() == '\t')) name.pop_back();

                std::string type = element.attribute("type").as_string("");
                bool visible = element.attribute("visible").as_bool(false);
                bool renderDisabled = element.attribute("RenderDisabled").as_bool(false);

                if (type == STR_TIMING) {
                    active = element.attribute("active").as_bool(false);
                }
                collapsed = element.attribute("collapsed").as_bool(false);

                if (ElementExists(name)) {
                    DisplayError("Duplicate " + type + ": '" + name + "'. Second instance ignored.");
                } else {
                    Element* elem = AddElement(name, type, visible, collapsed, active, selected, renderDisabled);
                    if (type == STR_TIMING) {
                        dynamic_cast<TimingElement*>(elem)->SetViews(element.attribute("views").as_string(""));
                        dynamic_cast<TimingElement*>(elem)->SetSubType(element.attribute("subType").as_string(""));
                    }
                }
            }
        } else if (ename == "TimingTags") {
            for (auto tag : e.children("Tag")) {
                int number = tag.attribute("number").as_int(-1);
                int position = tag.attribute("position").as_int(-1);
                if (number != -1 && GetTimeLine() != nullptr) {
                    GetTimeLine()->SetTagPosition(number, position, false);
                }
            }
        } else if (ename == "EffectDB") {
            effectStrings.clear();
            for (auto elementNode : e.children("Effect")) {
                std::string content = elementNode.text().as_string("");
                if (content.find("E_TEXTCTRL_Glediator_Filename") != std::string::npos) {
                    content = FixEffectFileParameter("E_TEXTCTRL_Glediator_Filename", content, showDir);
                }
                effectStrings.push_back(content);
            }
        } else if (ename == "ColorPalettes") {
            colorPalettes.clear();
            for (auto elementNode : e.children("ColorPalette")) {
                colorPalettes.push_back(elementNode.text().as_string(""));
            }
        } else if (ename == "SequenceMedia") {
            mSequenceMedia.LoadFromXml(e);
        } else if (ename == "Jukebox") {
            if (auto* frame = GetXLightsFrame()) {
                frame->GetJukeboxPanel()->Load(e);
            }
        } else if (ename == "ElementEffects") {
            // Count effects for progress
            int count = 0;
            for (auto elementNode : e.children("Element")) {
                for (auto layerNode : elementNode.children()) {
                    count += std::ranges::distance(layerNode.children());
                }
            }

            int loaded = 0;
            for (auto elementNode : e.children("Element")) {
                std::string nm = elementNode.attribute("name").as_string("");
                // trim whitespace
                while (!nm.empty() && (nm.front() == ' ' || nm.front() == '\t')) nm.erase(nm.begin());
                while (!nm.empty() && (nm.back() == ' ' || nm.back() == '\t')) nm.pop_back();

                Element* element = GetElement(nm);
                if (element != nullptr) {
                    int interval = 0;
                    std::string elemType = elementNode.attribute("type").as_string("");
                    if (elemType == STR_TIMING) {
                        interval = elementNode.attribute("fixed").as_int(0);
                    }
                    if (interval > 0) {
                        if (interval != RoundToMultipleOfPeriod(interval, mFrequency)) {
                            int newinterval = RoundToMultipleOfPeriod(interval, mFrequency);
                            if (newinterval == 0)
                                newinterval = 1000 / mFrequency;
                            spdlog::warn("Timing interval of {}ms not a multiple of frame time so changed to {}ms.", interval, newinterval);
                            interval = newinterval;
                        }
                        dynamic_cast<TimingElement*>(element)->SetFixedTiming(interval);
                        EffectLayer* effectLayer = element->AddEffectLayer();
                        int time = 0;
                        int end_time = RoundToMultipleOfPeriod(xml_file.GetSequenceDurationMS(), mFrequency);
                        while (time < end_time) {
                            int startTime = time;
                            int endTime = time + interval;
                            effectLayer->AddEffect(0, "", "", "", startTime, endTime, EFFECT_NOT_SELECTED, false, true);
                            time += interval;
                        }
                        effectLayer->NumberEffects();
                    } else {
                        for (auto effectLayerNode : elementNode.children()) {
                            EffectLayer* effectLayer = nullptr;
                            std::string layerNodeName = effectLayerNode.name();
                            if (layerNodeName == STR_EFFECTLAYER) {
                                effectLayer = element->AddEffectLayer();
                            } else if (layerNodeName == STR_SUBMODEL_EFFECTLAYER) {
                                std::string smName = effectLayerNode.attribute("name").as_string("");
                                while (!smName.empty() && (smName.front() == ' ' || smName.front() == '\t')) smName.erase(smName.begin());
                                while (!smName.empty() && (smName.back() == ' ' || smName.back() == '\t')) smName.pop_back();
                                int layer = effectLayerNode.attribute("layer").as_int(0);
                                SubModelElement* se = dynamic_cast<ModelElement*>(element)->GetSubModel(smName, true);
                                assert(se != nullptr);
                                while (layer >= 0 && static_cast<size_t>(layer) >= se->GetEffectLayerCount()) {
                                    se->AddEffectLayer();
                                }
                                effectLayer = se->GetEffectLayer(layer);
                            } else {
                                if (dynamic_cast<ModelElement*>(element) != nullptr) {
                                    StrandElement* se = dynamic_cast<ModelElement*>(element)->GetStrand(effectLayerNode.attribute("index").as_int(0), true);
                                    int layer = effectLayerNode.attribute("layer").as_int(0);
                                    while (layer >= 0 && static_cast<size_t>(layer) >= se->GetEffectLayerCount()) {
                                        se->AddEffectLayer();
                                    }
                                    effectLayer = se->GetEffectLayer(layer);
                                    std::string strandName = effectLayerNode.attribute("name").as_string("");
                                    if (!strandName.empty()) {
                                        while (!strandName.empty() && (strandName.front() == ' ' || strandName.front() == '\t')) strandName.erase(strandName.begin());
                                        while (!strandName.empty() && (strandName.back() == ' ' || strandName.back() == '\t')) strandName.pop_back();
                                        se->SetName(strandName);
                                    }
                                } else {
                                    spdlog::error("Element {} was not a model element. This typically happens when a timing track is created with the same name as a model.", element->GetName());
                                }
                            }
                            if (effectLayer != nullptr) {
                                std::string layerName = effectLayerNode.attribute("layerName").as_string("");
                                if (!layerName.empty()) {
                                    effectLayer->SetLayerName(layerName);
                                }
                                loaded += LoadEffects(effectLayer, elemType, effectLayerNode, effectStrings, colorPalettes, importing);
                                if (count) {
                                    if (auto* frame = GetXLightsFrame()) {
                                        frame->SetStatusText(std::format("Effects Loaded: {}%.", loaded * 100 / count));
                                    }
                                }
                            } else {
                                assert(false);
                            }
                        }
                    }
                } else {
                    assert(false);
                }
            }
        }
        TraceLog::PopTraceContext();
    }
    for (size_t x = 0; x < GetElementCount(); x++) {
        Element* el = GetElement(x);
        if (el->GetEffectLayerCount() == 0) {
            el->AddEffectLayer();
        }
    }

    mColorPalettes = colorPalettes;

    spdlog::debug("Sequencer file loaded.");

    return true;
}

void SequenceElements::PrepareViews(SequenceFile& xml_file)
{
    if (_viewsManager == nullptr) {
        spdlog::critical("SequenceElements::PrepareViews called when _viewsManager was null ... this will crash");
    }

    // Select view and set current view models as visible
    int last_view = xml_file.GetLastView();
    auto views = _viewsManager->GetViews();
    for (const auto& it : views) {
        std::vector <Element*> new_view;
        mAllViews.push_back(new_view);
        int view_index = mAllViews.size() - 1;
        if (view_index == last_view) {
            AddMissingModelsToSequence(it->GetModelsString());
            PopulateView(it->GetModelsString(), view_index);
            SetCurrentView(view_index);
        }
    }

    PopulateRowInformation();
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
    assert(_viewsManager != nullptr);
    _viewsManager->SetSelectedView(view);
}

void SequenceElements::AddMissingModelsToSequence(const std::string &models, bool visible)
{
    if(models.length()> 0)
    {
        auto model=Split(models,',');
        for(size_t m=0;m<model.size();m++)
        {
            std::string modelName = Trim(model[m]);
            Model *model1 = renderContext->GetModel(modelName);
            if (model1 != nullptr) {
                if (model1->GetDisplayAs() == DisplayAsType::SubModel) {
                    model1 = (dynamic_cast<SubModel*>(model1))->GetParent();
                }
                if (model1 != nullptr)
                {
                    if (!ElementExists(model1->GetName()))
                    {
                        Element* elem = AddElement(model1->GetName(), "model", visible, false, false, false, false);
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
    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++) {
        Element* elem = mAllViews[MASTER_VIEW][i];
        TimingElement* te = dynamic_cast<TimingElement*>(elem);
        if (te != nullptr) {
            if (name == "Master View") {
                te->SetVisible(te->GetMasterVisible());
            }
            else {
                te->SetVisible(false);
                auto views = Split(te->GetViews(), ',');
                for (size_t v = 0; v < views.size(); v++) {
                    const std::string& viewName = views[v];
                    if (name == viewName) {
                        te->SetVisible(true);
                        break;
                    }
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

void SequenceElements::AddViewToTimings(const std::vector<std::string>& timings, const std::string& name)
{
    for (auto const& timing : timings) {
        AddTimingToView(timing, name);
    }
}

void SequenceElements::AddTimingToView(const std::string& timing, const std::string& name)
{
    Element* elem = GetElement(timing);
    TimingElement *te = dynamic_cast<TimingElement*>(elem);
    if( elem != nullptr && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING )
    {
        std::string views = te->GetViews();
        auto all_views = Split(views,',');
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
            views = Join(all_views, ",");
            te->SetViews(views);
        }
    }
}

void SequenceElements::PopulateView(const std::string &models, int view)
{
    if (view < 0 || static_cast<size_t>(view) >= mAllViews.size()) return;

    mAllViews[view].clear();

    if(models.length()> 0)
    {
        auto model=Split(models,',');
        for(size_t m=0;m<model.size();m++)
        {
            const std::string& modelName = model[m];
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

int SequenceElements::GetFrameMS()
{
	return GetMinPeriod();
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

void addSubModelElement(SubModelElement* elem,
    std::vector<Row_Information_Struct>& mRowInformation,
    int& rowIndex,
    std::vector <Element*>& elements) {
    if (elem == nullptr) {
        spdlog::error("addSubModelElement attempted to add null element.");
        return;
    }

    if (!elem->GetCollapsed()) {
        for (size_t j = 0; j < elem->GetEffectLayerCount(); j++) {
            Row_Information_Struct ri;
            ri.element = elem;
            ri.displayName = elem->GetFullName();
            ri.Collapsed = elem->GetCollapsed();
            ri.layerName = elem->GetEffectLayer(j)->GetLayerName();
            ri.colorIndex = 0;
            ri.layerIndex = j;
            ri.Index = rowIndex++;
            ri.submodel = true;
            mRowInformation.push_back(ri);
        }
    }
    else {
        Row_Information_Struct ri;
        ri.element = elem;
        ri.Collapsed = elem->GetCollapsed();
        ri.displayName = elem->GetFullName();
        ri.layerName = elem->GetEffectLayer(0)->GetLayerName();
        ri.colorIndex = 0;
        ri.layerIndex = 0;
        ri.Index = rowIndex++;
        ri.submodel = true;
        mRowInformation.push_back(ri);
    }
}

void addModelElement(ModelElement* elem, std::vector<Row_Information_Struct>& mRowInformation,
    int& rowIndex,
    std::vector <Element*>& elements,
    bool submodel)
{

    if (elem == nullptr) {
        spdlog::error("addModelElement attempted to add null element.");
        return;
    }

    if (!elem->GetCollapsed()) {
        for (size_t j = 0; j < elem->GetEffectLayerCount(); j++) {
            Row_Information_Struct ri;
            ri.element = elem;
            ri.displayName = elem->GetName();
            ri.layerName = elem->GetEffectLayer(j)->GetLayerName();
            ri.Collapsed = elem->GetCollapsed();
            ri.colorIndex = 0;
            ri.layerIndex = j;
            ri.Index = rowIndex++;
            ri.submodel = submodel;
            mRowInformation.push_back(ri);
        }
    }
    else {
        Row_Information_Struct ri;
        ri.element = elem;
        ri.Collapsed = elem->GetCollapsed();
        ri.displayName = elem->GetName();
        ri.layerName = elem->GetEffectLayer(0)->GetLayerName();
        ri.colorIndex = 0;
        ri.layerIndex = 0;
        ri.Index = rowIndex++;
        ri.submodel = submodel;
        mRowInformation.push_back(ri);
    }
    Model* cls = elem->GetSequenceElements()->GetRenderContext()->GetModel(elem->GetModelName());
    if (cls == nullptr) {
        spdlog::error("addModelElement model not found {}.", elem->GetModelName());
        return;
    }
    elem->Init(*cls);
    if (cls->GetDisplayAs() == DisplayAsType::ModelGroup && elem->ShowStrands()) {
        ModelGroup* grp = dynamic_cast<ModelGroup*>(cls);
        for (const auto& it : grp->ModelNames()) {
            std::string modelName = it;
            std::string subModel = "";
            int slsh = modelName.find('/');
            if (slsh != -1) {
                subModel = modelName.substr(slsh + 1);
                modelName = modelName.substr(0, slsh);
            }
            for (size_t x = 0; x < elements.size(); x++) {
                if (elements[x]->GetModelName() == modelName) {
                    ModelElement* melem = dynamic_cast<ModelElement*>(elements[x]);
                    if (subModel != "") {
                        SubModelElement* selem = melem->GetSubModel(subModel);
                        addSubModelElement(selem, mRowInformation, rowIndex, elements);
                    }
                    else {
                        addModelElement(melem, mRowInformation, rowIndex, elements, true);
                    }
                }
            }
        }
    }
    else if (elem->ShowStrands()) {
        for (int s = 0; s < elem->GetSubModelAndStrandCount(); s++) {
            SubModelElement* se = elem->GetSubModel(s);
            int m = se->GetEffectLayerCount();
            if (se->GetCollapsed()) {
                m = 1;
            }
            for (int x = 0; x < m; x++) {
                Row_Information_Struct ri;
                ri.element = se;
                ri.Collapsed = se->GetCollapsed();
                ri.displayName = se->GetName();
                ri.layerName = se->GetEffectLayer(x)->GetLayerName();

                ri.colorIndex = 0;
                ri.layerIndex = x;
                ri.Index = rowIndex++;
                ri.strandIndex = -1;
                if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                    ri.strandIndex = ((StrandElement*)se)->GetStrand();
                }
                ri.submodel = submodel;
                mRowInformation.push_back(ri);
            }

            if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND && ((StrandElement*)se)->ShowNodes()) {
                StrandElement* ste = dynamic_cast<StrandElement*>(se);
                for (int n = 0; n < ste->GetNodeLayerCount(); n++) {
                    Row_Information_Struct ri;
                    ri.element = se;
                    ri.Collapsed = ste->ShowNodes();
                    ri.displayName = ste->GetNodeLayer(n)->GetNodeName();
                    ri.layerName = se->GetEffectLayer(0)->GetLayerName();
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

void SequenceElements::addTimingElement(TimingElement* elem, std::vector<Row_Information_Struct>& mRowInformation,
    int& rowIndex, int& selectedTimingRow, int& timingRowCount, int& timingColorIndex)
{
    if (elem->GetEffectLayerCount() > 1) {
        hasPapagayoTiming = true;
    }

    if (!elem->GetCollapsed()) {
        for (size_t j = 0; j < elem->GetEffectLayerCount(); j++) {
            Row_Information_Struct ri;
            ri.element = elem;
            ri.Collapsed = elem->GetCollapsed();
            ri.colorIndex = timingColorIndex;
            ri.layerIndex = j;
            if (selectedTimingRow < 0 && j == 0) {
                selectedTimingRow = elem->GetActive() ? rowIndex : -1;
            }

            ri.Index = rowIndex++;
            mRowInformation.push_back(ri);
            timingRowCount++;
        }
    }
    else {
        Row_Information_Struct ri;
        ri.element = elem;
        ri.Collapsed = elem->GetCollapsed();
        ri.displayName = elem->GetName();
        ri.layerName = "";
        ri.colorIndex = timingColorIndex;
        ri.layerIndex = 0;
        if (selectedTimingRow < 0) {
            selectedTimingRow = elem->GetActive() ? rowIndex : -1;
        }
        ri.Index = rowIndex++;
        mRowInformation.push_back(ri);
        timingRowCount++;
    }
    timingColorIndex++;
}

void SequenceElements::PopulateRowInformation()
{
    int rowIndex = 0;
    int timingColorIndex = 0;
    mSelectedTimingRow = -1;
    mRowInformation.clear();
    mTimingRowCount = 0;

    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++)
    {
        Element* elem = mAllViews[MASTER_VIEW][i];
        if (elem != nullptr)
        {
            if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING)
            {
                if ((mCurrentView == MASTER_VIEW || TimingIsPartOfView(dynamic_cast<TimingElement*>(elem), mCurrentView)) && elem->GetVisible())
                {
                    addTimingElement(dynamic_cast<TimingElement*>(elem), mRowInformation, rowIndex, mSelectedTimingRow, mTimingRowCount, timingColorIndex);
                }
            }
        }
    }

    for (size_t i = 0; i < mAllViews[mCurrentView].size(); i++)
    {
        Element* elem = mAllViews[mCurrentView][i];
        if (elem != nullptr)
        {
            if (elem->GetVisible() && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                addModelElement(dynamic_cast<ModelElement*>(elem), mRowInformation, rowIndex, mAllViews[MASTER_VIEW], false);
            }
        }
    }
    PopulateVisibleRowInformation();
}

void SequenceElements::PopulateVisibleRowInformation()
{
    mVisibleRowInformation.clear();
    if (mRowInformation.size() == 0) { return; }
    // Add all timing element rows. They should always be first in the list
    int row;
    for (row = 0; row < mTimingRowCount; row++)
    {
        if (row < mMaxRowsDisplayed)
        {
            mVisibleRowInformation.push_back(mRowInformation[row]);
        }
    }

    if (static_cast<size_t>(mFirstVisibleModelRow) >= mRowInformation.size()) {
        mFirstVisibleModelRow = 0;
    }
    for (; row < mMaxRowsDisplayed && static_cast<size_t>(row + mFirstVisibleModelRow) < mRowInformation.size(); row++)
    {
        mRowInformation[row + mFirstVisibleModelRow].RowNumber = row;
        mVisibleRowInformation.push_back(mRowInformation[row + mFirstVisibleModelRow]);
    }
}

int SequenceElements::SetFirstVisibleModelRow(int row)
{
    int old = mFirstVisibleModelRow;

    // They all fit on screen. So set to first model element.
    if(mRowInformation.size() <= static_cast<size_t>(mMaxRowsDisplayed))
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

    return mFirstVisibleModelRow - old;
}

int SequenceElements::GetNumberOfTimingRows() const
{
    return mTimingRowCount;
}

int SequenceElements::GetNumberOfTimingElements() {
    int count = 0;
    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++)
    {
        if (mAllViews[MASTER_VIEW][i]->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            count++;
        }
    }
    return count;
}

TimingElement* SequenceElements::GetTimingElement(int n)
{
    int count = 0;
    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++)
    {
        if (mAllViews[MASTER_VIEW][i]->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            if (count == n)
            {
                return (TimingElement*)mAllViews[MASTER_VIEW][i];
            }
            count++;
        }
    }
    return nullptr;
}

TimingElement* SequenceElements::GetTimingElement(const std::string& name)
{
    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++)
    {
        if (mAllViews[MASTER_VIEW][i]->GetType() == ElementType::ELEMENT_TYPE_TIMING && mAllViews[MASTER_VIEW][i]->GetName() == name)
        {
            return (TimingElement*)mAllViews[MASTER_VIEW][i];
        }
    }
    return nullptr;
}

int SequenceElements::GetNumberOfActiveTimingEffects()
{
    int num_timing_effects = 0;
    EffectLayer* tel = GetVisibleEffectLayer(GetSelectedTimingRow());
    if (tel != nullptr)
    {
        num_timing_effects = tel->GetEffectCount();
    }
    return num_timing_effects;
}

void SequenceElements::DeactivateAllTimingElements()
{
    for(size_t i=0;i<mAllViews[MASTER_VIEW].size();i++)
    {
        if(mAllViews[MASTER_VIEW][i]->GetType()== ElementType::ELEMENT_TYPE_TIMING)
        {
            dynamic_cast<TimingElement*>(mAllViews[MASTER_VIEW][i])->SetActive(false);
        }
    }
}

int SequenceElements::SelectEffectsInRowAndTimeRange(int startRow, int endRow, int startMS, int endMS)
{
    int num_selected = 0;
    if(startRow>=0 && static_cast<size_t>(startRow)<mRowInformation.size())
    {
        if(endRow<0 || static_cast<size_t>(endRow)>=mRowInformation.size())
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
    if(startRow>=0 && static_cast<size_t>(startRow)<mVisibleRowInformation.size())
    {
        if(endRow<0 || static_cast<size_t>(endRow)>=mVisibleRowInformation.size())
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
    if (tel != nullptr)
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

void SequenceElements::SelectAllEffects()
{
    for (size_t i = 0; i < mRowInformation.size(); i++)
    {
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        effectLayer->SelectAllEffects();
    }
}

void SequenceElements::SelectAllEffectsNoTiming()
{
    for (size_t i = 0; i < mRowInformation.size(); i++)
    {
        if (mRowInformation[i].element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        if (effectLayer != nullptr)
        {
            effectLayer->SelectAllEffects();
        }
    }
}

std::vector<std::string> SequenceElements::GetUsedColours(bool selectedOnly) const
{
    std::vector<std::string> usedColours;

    for (size_t i = 0; i < mRowInformation.size(); i++) {
        if (mRowInformation[i].element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        if (effectLayer != nullptr) {
            for (const auto& it : effectLayer->GetUsedColours(selectedOnly)) {
                if (std::find(begin(usedColours), end(usedColours), it) == end(usedColours)) {
                    usedColours.push_back(it);
                }
            }
        }
    }
    return usedColours;
}

int SequenceElements::ReplaceColours(RenderContext* ctx, const std::string& from, const std::string& to, bool selectedOnly)
{
    get_undo_mgr().CreateUndoStep();
    int replaced = 0;
    for (size_t i = 0; i < mRowInformation.size(); i++) {
        if (mRowInformation[i].element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            continue;
        }
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        if (effectLayer != nullptr) {
            replaced += effectLayer->ReplaceColours(ctx, from, to, selectedOnly, get_undo_mgr());
        }
    }
    return replaced;
}

void SequenceElements::SelectAllEffectsInRow(int row)
{
    EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[row]);
    if (effectLayer != nullptr)
    {
        effectLayer->SelectAllEffects();
    }
}

void SequenceElements::UnSelectAllEffects()
{
    for (size_t i = 0; i < mRowInformation.size(); i++)
    {
        EffectLayer* effectLayer = GetEffectLayer(&mRowInformation[i]);
        if (effectLayer != nullptr)
        {
            effectLayer->UnSelectAllEffects();
        }
    }
}

void SequenceElements::SelectAllElements()
{
    for (size_t i = 0; i < mAllViews[mCurrentView].size(); i++)
    {
        if (mAllViews[mCurrentView][i]->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            dynamic_cast<ModelElement*>(mAllViews[mCurrentView][i])->SetSelected(true);
        }
    }
}

void SequenceElements::UnSelectAllElements()
{
    for(size_t i=0; i < mAllViews[mCurrentView].size(); i++)
    {
        if(mAllViews[mCurrentView][i]->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            dynamic_cast<ModelElement*>(mAllViews[mCurrentView][i])->SetSelected(false);
        }
    }
}

// Functions to manage selected ranges
size_t SequenceElements::GetSelectedRangeCount()
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
    if(index >= 0 && static_cast<size_t>(index) < mSelectedRanges.size())
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

bool SequenceElements::IsValidEffect(Effect* ef) const
{
    for (size_t i = 0; i < GetElementCount(); i++) {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            if (e->IsEffectValid(ef)) {
                return true;
            }
            if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* mel = dynamic_cast<ModelElement*>(e);
                if (mel != nullptr) {
                    for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                        SubModelElement* sme = mel->GetSubModel(x);
                        if (sme != nullptr) {
                            if (sme->IsEffectValid(ef)) {
                                return true;
                            }
                        }
                    }
                }
            }
        }
        else {
            if (e->IsEffectValid(ef)) {
                return true;
            }
        }
    }

    return false;
}

bool SequenceElements::IsValidElement(Element* e) const
{
    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++) {
        Element* elem = mAllViews[MASTER_VIEW][i];
        if (e == elem) return true;
    }

    return false;
}

size_t SequenceElements::GetHiddenTimingCount() const
{
    size_t count = 0;
    auto view = _viewsManager->GetSelectedViewIndex();
    for (size_t i = 0; i < mAllViews[view].size(); i++) {
        Element* elem = mAllViews[view][i];
        TimingElement* te = dynamic_cast<TimingElement*>(elem);
        if (te != nullptr && !te->GetVisible()) count++;
    }
    return count;
}

void SequenceElements::HideAllTimingTracks(bool hide)
{
    // This only works on timing tracks in master views because of the way we manage timing tracks in non master views
    for (size_t i = 0; i < mAllViews[MASTER_VIEW].size(); i++) {
        Element* elem = mAllViews[MASTER_VIEW][i];
        TimingElement* te = dynamic_cast<TimingElement*>(elem);
        if (te != nullptr) {
            te->SetVisible(!hide);
            te->SetMasterVisible(!hide);
        }
    }
}

Effect* SequenceElements::SelectEffectUsingDescription(std::string description)
{
    for (size_t i = 0; i < GetElementCount(); i++)
    {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            Effect* ef = e->SelectEffectUsingDescription(description);
            if (ef != nullptr)
            {
                return ef;
            }
        }
    }

    return nullptr;
}

std::list<std::string> SequenceElements::GetUniqueEffectPropertyValues(const std::string& id)
{
    std::list<std::string> res;

    for (size_t i = 0; i < GetElementCount(); i++) {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            for (size_t j = 0; j < e->GetEffectLayerCount(); j++) {
                EffectLayer* el = e->GetEffectLayer(j);
                for (int k = 0; k < el->GetEffectCount(); k++) {
                    Effect* eff = el->GetEffect(k);

                    if (eff->GetSetting(id) != "" && std::find(res.begin(), res.end(), eff->GetSetting(id)) == res.end()) {
                        res.push_back(eff->GetSetting(id));
                    }
                }
            }
        }
    }

    return res;
}

std::list<std::string> SequenceElements::GetAllEffectDescriptions()
{
    std::list<std::string> res;

    for (size_t i = 0; i < GetElementCount(); i++)
    {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            for (size_t j = 0; j < e->GetEffectLayerCount(); j++)
            {
                EffectLayer* el = e->GetEffectLayer(j);
                for (int k = 0; k < el->GetEffectCount(); k++)
                {
                    Effect* eff = el->GetEffect(k);

                    if (!eff->GetDescription().empty() && std::find(res.begin(), res.end(), eff->GetDescription()) == res.end())
                    {
                        res.push_back(eff->GetDescription());
                    }
                }
            }
        }
    }

    return res;
}

std::list<std::string> SequenceElements::GetAllReferencedFiles()
{
    std::list<std::string> res;

    for (size_t i = 0; i < GetElementCount(); i++) {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            Model* m = renderContext->GetModel(e->GetModelName());
            for (const auto& it : e->GetFileReferences(m, GetEffectManager())) {
                if (std::find(begin(res), end(res), it) == end(res)) {
                    res.push_back(it);
                }
            }
        }
    }

    return res;
}

std::list<std::string> SequenceElements::GetAllUsedEffectTypes() const
{
    std::list<std::string> res;

    for (size_t i = 0; i < GetElementCount(); i++) {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            for (size_t j = 0; j < e->GetEffectLayerCount(); j++) {
                EffectLayer* el = e->GetEffectLayer(j);
                for (int k = 0; k < el->GetEffectCount(); k++) {
                    Effect* eff = el->GetEffect(k);
                    if (std::find(res.begin(), res.end(), eff->GetEffectName()) == res.end()) {
                        res.push_back(eff->GetEffectName());
                    }
                }
            }
            if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* mel = dynamic_cast<ModelElement*>(e);
                if (mel != nullptr) {
                    for (int x = 0; x < mel->GetSubModelAndStrandCount(); ++x) {
                        SubModelElement* sme = mel->GetSubModel(x);
                        if (sme != nullptr) {
                            for (size_t j = 0; j < sme->GetEffectLayerCount(); j++) {
                                EffectLayer* el = sme->GetEffectLayer(j);
                                for (int k = 0; k < el->GetEffectCount(); k++) {
                                    Effect* eff = el->GetEffect(k);
                                    if (std::find(res.begin(), res.end(), eff->GetEffectName()) == res.end()) {
                                        res.push_back(eff->GetEffectName());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return res;
}

std::list<std::string> SequenceElements::GetAllElementNamesWithEffectsExtended()
{
    std::list<std::string> res;

    for (size_t i = 0; i < GetElementCount(); i++) {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            if (std::find(res.begin(), res.end(), e->GetFullName()) == res.end()) {
                if (e->HasEffects()) {
                    res.push_back(e->GetFullName());
                }
            }

            auto me = dynamic_cast<ModelElement*>(e);
            if (me != nullptr)
            {
                for (int i = 0; i < me->GetSubModelCount(); ++i)
                {
                    auto sm = me->GetSubModel(i);
                    if (sm->HasEffects())
                    {
                        if (std::find(res.begin(), res.end(), sm->GetFullName()) == res.end()) {
                            res.push_back(sm->GetFullName());
                        }
                    }
                }
                for (int i = 0; i < me->GetStrandCount(); ++i) {
                    auto st = me->GetStrand(i);
                    if (st->HasEffects()) {
                        if (std::find(res.begin(), res.end(), st->GetFullName()) == res.end()) {
                            res.push_back(st->GetFullName());
                        }
                    }
                }
            }
        }
    }

    return res;
}

std::list<std::string> SequenceElements::GetAllElementNamesWithEffects()
{
    std::list<std::string> res;

    for (size_t i = 0; i < GetElementCount(); i++)
    {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            if (std::find(res.begin(), res.end(), e->GetFullName()) == res.end())
            {
                if (e->HasEffects())
                {
                    res.push_back(e->GetFullName());
                }
            }
        }
    }

    return res;
}

int SequenceElements::GetElementLayerCount(std::string elementName, std::list<int>* layers)
{
    for (size_t i = 0; i < GetElementCount(); i++)
    {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            if (e->GetFullName() == elementName)
            {
                if (layers != nullptr)
                {
                    for (size_t j = 0; j < e->GetEffectLayerCount(); j++)
                    {
                        if (e->GetEffectLayer(j)->HasEffects())
                        {
                            layers->push_back(j);
                        }
                    }
                }

                return e->GetEffectLayerCount();
            }
        }
    }

    return -1;
}

std::list<Effect*> SequenceElements::GetElementLayerEffects(std::string elementName, int layer)
{
    std::list<Effect*> res;

    for (size_t i = 0; i < GetElementCount(); i++)
    {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            if (e->GetFullName() == elementName)
            {
                if (layer >= 0 && static_cast<size_t>(layer) < e->GetEffectLayerCount())
                {
                    EffectLayer* el = e->GetEffectLayer(layer);
                    return el->GetAllEffects();
                }
            }
        }
    }

    return res;
}

Effect* SequenceElements::SelectEffectUsingElementLayerTime(std::string element, int layer, int time)
{
    for (size_t i = 0; i < GetElementCount(); i++)
    {
        Element* e = GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING)
        {
            if (e->GetFullName() == element)
            {
                return e->SelectEffectUsingLayerTime(layer, time);
            }
        }
    }
    return nullptr;
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

    for (size_t i = 0; i < GetElementCount(); ++i)
    {
        if (GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_TIMING)
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

int SequenceElements::GetElementIndexOfTimingFromListIndex(int timingIndex)
{
    int count = 0;

    for (size_t i = 0; i < GetElementCount(); ++i)
    {
        if (GetElement(i)->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            if (count == timingIndex)
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

    if(index >= 0 && static_cast<size_t>(index) < mAllViews[view].size() && dest >= 0 && static_cast<size_t>(dest) < mAllViews[view].size())
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
    else if (index >= 0 && static_cast<size_t>(index) < mAllViews[view].size())
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



void SequenceElements::IncrementChangeCount(Element *el) {
    mChangeCount++;
    if (el != nullptr && el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        //need to check if we need to have some models re-rendered due to timing being changed
        std::unique_lock<std::mutex> locker(renderDepLock);
        std::map<std::string, std::set<std::string>>::iterator it = renderDependency.find(el->GetModelName());
        if (it != renderDependency.end()) {
            int origChangeCount, ss, es;
            el->GetAndResetDirtyRange(origChangeCount, ss, es);
            for (std::set<std::string>::iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
                Element *el2 = this->GetElement(*sit);
                if (el2 != nullptr) {
                    el2->IncrementChangeCount(ss, es);
                    modelsToRender.insert(*sit);
                    if (auto* frame = GetXLightsFrame()) {
                        frame->StartOutputTimer(); // start the timer so the render will trigger
                    }
                }
            }
        }
    }
}

bool SequenceElements::GetElementsToRender(std::vector<Element *> &models) {
    std::unique_lock<std::mutex> locker(renderDepLock);
    if (!modelsToRender.empty()) {
        for (std::set<std::string>::iterator sit = modelsToRender.begin(); sit != modelsToRender.end(); ++sit) {
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
    return renderContext->GetEffectManager();
}

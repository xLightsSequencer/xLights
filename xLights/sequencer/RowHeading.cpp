/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/brush.h>
#include <wx/numdlg.h>
#include <wx/confbase.h>

#include "RowHeading.h"
#include "../xLightsVersion.h"
#include "../BitmapCache.h"
#include "models/ModelGroup.h"
#include "../SelectTimingsDialog.h"
#include "models/SubModel.h"
#include "MainSequencer.h"
#include "EffectsGrid.h"
#include "ColorManager.h"
#include "SequenceElements.h"
#include "../xLightsMain.h"
#include "NewTimingDialog.h"
#include "VAMPPluginDialog.h"
#include "UtilFunctions.h"
#include "graphics/opengl/xlGLCanvas.h"

#include <log4cpp/Category.hh>

#define ICON_SPACE ScaleWithSystemDPI(25)

BEGIN_EVENT_TABLE(RowHeading, wxWindow)
EVT_LEFT_DOWN(RowHeading::mouseLeftDown)
EVT_LEFT_UP(RowHeading::mouseLeftUp)
EVT_RIGHT_DOWN(RowHeading::rightClick)
EVT_LEFT_DCLICK(RowHeading::leftDoubleClick)
EVT_ENTER_WINDOW(RowHeading::mouseEnter)
EVT_LEAVE_WINDOW(RowHeading::mouseLeave)
EVT_MOTION(RowHeading::mouseMove)
EVT_PAINT(RowHeading::render)
END_EVENT_TABLE()

// Menu constants
const long RowHeading::ID_ROW_MNU_INSERT_LAYER_ABOVE = wxNewId();
const long RowHeading::ID_ROW_MNU_INSERT_LAYER_BELOW = wxNewId();
const long RowHeading::ID_ROW_MNU_INSERT_LAYERS_BELOW = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_LAYERS = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_UNUSEDLAYERS = wxNewId();
const long RowHeading::ID_ROW_MNU_LAYER = wxNewId();
const long RowHeading::ID_ROW_MNU_PLAY_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_MODEL_SELECTED_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_RENDERED_MODEL_SELECTED_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_RENDERED_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_STRANDS = wxNewId();
const long RowHeading::ID_ROW_MNU_SHOW_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_COLLAPSEALLMODELS = wxNewId();
const long RowHeading::ID_ROW_MNU_COLLAPSEALLLAYERS = wxNewId();
const long RowHeading::ID_ROW_MNU_TOGGLE_NODES = wxNewId();
const long RowHeading::ID_ROW_MNU_CONVERT_TO_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_CREATE_TIMING_FROM_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_PROMOTE_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_CUT_ROW = wxNewId();
const long RowHeading::ID_ROW_MNU_CUT_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_COPY_ROW = wxNewId();
const long RowHeading::ID_ROW_MNU_COPY_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_PASTE_ROW = wxNewId();
const long RowHeading::ID_ROW_MNU_PASTE_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_RENDERENABLE_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_RENDERDISABLE_MODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_ROW_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_MODEL_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_MODEL_STRAND_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_MODEL_NODE_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_SELECT_ROW_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_SELECT_MODEL_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_SELECT_TIMING_EFFECTS = wxNewId();
const long RowHeading::ID_ROW_MNU_MODEL_CONVERTTOPERMODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_ROW_CONVERTTOPERMODEL = wxNewId();
const long RowHeading::ID_ROW_MNU_RENDERENABLE_ALL = wxNewId();

// Timing Track popup menu
const long RowHeading::ID_ROW_MNU_ADD_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_RENAME_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_DELETE_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_EXPORT_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_UNFIX_TIMING_TRACK = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_NOTES = wxNewId();
const long RowHeading::ID_ROW_MNU_IMPORT_LYRICS = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES = wxNewId();
const long RowHeading::ID_ROW_MNU_BREAKDOWN_TIMING_WORDS = wxNewId();
const long RowHeading::ID_ROW_MNU_REMOVE_TIMING_WORDS = wxNewId();
const long RowHeading::ID_ROW_MNU_REMOVE_TIMING_PHONEMES = wxNewId();
const long RowHeading::ID_ROW_MNU_REMOVE_TIMING_WORDS_PHONEMES = wxNewId();
const long RowHeading::ID_ROW_MNU_HIDEALLTIMING = wxNewId();
const long RowHeading::ID_ROW_MNU_SHOWALLTIMING = wxNewId();

int DEFAULT_ROW_HEADING_HEIGHT = 22;

RowHeading::RowHeading(MainSequencer* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name):
                       wxWindow((wxWindow*)parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("                Creating RowHeading");

    DOUBLE_BUFFER(this);
    wxString tooltip;

    papagayo_icon = BitmapCache::GetPapgayoIcon();
    papagayox_icon = BitmapCache::GetPapgayoXIcon();
    model_group_icon = BitmapCache::GetModelGroupIcon();
    fppCommand_icon = BitmapCache::GetFPPIcon();
    fppEffect_icon = BitmapCache::GetFPPIcon();
    
    mCanPaste = false;

    wxConfigBase* config = wxConfigBase::Get();
    int w = config->ReadLong("xLightsRowHeaderWidth", _minRowHeadingWidth);
    CallAfter(&RowHeading::SetWidth, w);
}

RowHeading::~RowHeading()
{
}

void RowHeading::ProcessTooltip(wxMouseEvent& event)
{
    int mSelectedRow = event.GetY() / DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow < mSequenceElements->GetVisibleRowInformationSize()) {
        Element* e = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->element;
        if (e != nullptr) {
            wxString layers;
            if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING && e->GetEffectLayerCount() > 1) {
                layers = wxString::Format(" [%d]", (int)e->GetEffectLayerCount());
            }

            // wxClientDC is going to be deprecated and removed as it cannot draw onto the
            // screen from all ports (example: wayland).  However, it can be used
            // at this point to query text metrics until they can introduce a replacement.
            // https://github.com/wxWidgets/wxWidgets/issues/12486
            wxClientDC dc(this);
            wxSize size = dc.GetTextExtent(e->GetName() + layers);

            // Only set tooltip if the text looks too big to display correctly
            if (size.x > getWidth() - DEFAULT_ROW_HEADING_MARGIN - ICON_SPACE) {
                auto s1 = GetToolTipText().ToStdString();
                auto s2 = e->GetName();
                if (GetToolTipText() != e->GetName() + layers) {
                    SetToolTip(e->GetName() + layers);
                }
            }
            else {
                SetToolTip("");
            }
        }
        else {
            SetToolTip("");
        }
    }
    else {
        SetToolTip("");
    }
}

void RowHeading::mouseEnter(wxMouseEvent& event)
{
    mouseMove(event);
}

void RowHeading::mouseLeave(wxMouseEvent& event)
{
    SetToolTip("");
}

void RowHeading::SetWidth(int w)
{
    auto minSize = GetMinSize();
    if (w < _minRowHeadingWidth) w = _minRowHeadingWidth;
    if (minSize.GetWidth() != w) {
        SetMinSize(wxSize(w, -1));
        GetParent()->Layout();
    }
}

void RowHeading::mouseMove(wxMouseEvent& event)
{
    ProcessTooltip(event);

    if (_dragging) {
        SetWidth(event.GetX());
    }

    auto size = GetSize();
    if (HasCapture() || (event.GetX() > size.GetWidth() - 5 && event.GetX() < size.GetWidth())) {
        SetCursor(wxCURSOR_SIZEWE);
    }
    else {
        SetCursor(wxCURSOR_ARROW);
    }
}

void RowHeading::mouseLeftUp(wxMouseEvent& event)
{
    if (_dragging) {
        auto size = GetSize();
        wxConfigBase* config = wxConfigBase::Get();
        config->Write("xLightsRowHeaderWidth", size.GetWidth());
        ReleaseMouse();
        _dragging = false;
    }
}

void RowHeading::mouseLeftDown(wxMouseEvent& event)
{
    _dragging = false;
    auto size = GetSize();
    if (event.GetX() > size.GetWidth() - 5 && event.GetX() < size.GetWidth()) {
        CaptureMouse();
        _dragging = true;
        return;
    }
    mSelectedRow = event.GetY() / DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow < mSequenceElements->GetVisibleRowInformationSize()) {
        bool result;
        Element* e = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->element;
        if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            mSequenceElements->UnSelectAllElements();
            ModelElement* me = dynamic_cast<ModelElement*>(e);
            me->SetSelected(true);
            wxCommandEvent playEvent(EVT_MODEL_SELECTED); // send model selection in case we need to switch playback to this model
            playEvent.SetString(e->GetModelName());
            wxPostEvent(GetParent(), playEvent);
            Refresh(false);
        }
        if (HitTestCollapseExpand(mSelectedRow, event.GetX(), &result)) {
            e->SetCollapsed(!result);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        } else if (HitTestTimingActive(mSelectedRow, event.GetX(), &result)) {
            mSequenceElements->DeactivateAllTimingElements();
            TimingElement* te = dynamic_cast<TimingElement*>(e);
            te->SetActive(!result);
            // Set the selected timing row.
            int selectedTimingRow = result ? mSelectedRow : -1;
            mSequenceElements->SetSelectedTimingRow(selectedTimingRow);
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}

void RowHeading::ToggleExpand(Element* element)
{
    if (element->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        ModelElement *me = dynamic_cast<ModelElement *>(element);
        me->ShowStrands(!me->ShowStrands());
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        eventRowHeaderChanged.SetString(element->GetModelName());
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if (element->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {

    }
    else if (element->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        StrandElement *se = dynamic_cast<StrandElement *>(element);
        se->ShowNodes(!se->ShowNodes());

        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        eventRowHeaderChanged.SetString(element->GetName());
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    }
    else if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        if (element->GetEffectLayerCount() > 1) {
            if (element->GetCollapsed()) {
                element->SetCollapsed(false);
            }
            else {
                element->SetCollapsed(true);
            }
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            eventRowHeaderChanged.SetString(element->GetName());
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    }
}

void RowHeading::leftDoubleClick(wxMouseEvent& event)
{
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }
    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    Element* element = ri->element;

    ToggleExpand(element);
}

void RowHeading::rightClick( wxMouseEvent& event)
{
    wxMenu mnuLayer;
    mSelectedRow = event.GetY()/DEFAULT_ROW_HEADING_HEIGHT;
    if (mSelectedRow >= mSequenceElements->GetVisibleRowInformationSize()) {
        return;
    }

    Row_Information_Struct *ri =  mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    if (ri != nullptr) {
        Element* element = ri->element;
        if (element != nullptr) {
            if (element->GetType() == ElementType::ELEMENT_TYPE_MODEL
                || element->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL
                || element->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                if (ri->nodeIndex < 0) {
                    mnuLayer.Append(ID_ROW_MNU_INSERT_LAYER_ABOVE, "Insert Layer Above");
                    mnuLayer.Append(ID_ROW_MNU_INSERT_LAYER_BELOW, "Insert Layer Below");
                    mnuLayer.Append(ID_ROW_MNU_INSERT_LAYERS_BELOW, "Insert Multiple Layers Below");

                    if (element->GetEffectLayerCount() > 1) {
                        mnuLayer.Append(ID_ROW_MNU_DELETE_LAYER, "Delete Layer");

                        mnuLayer.Append(ID_ROW_MNU_DELETE_LAYERS, "Delete Multiple Layers");
                        mnuLayer.Append(ID_ROW_MNU_DELETE_UNUSEDLAYERS, "Delete Unused Layers");
                    }
                    mnuLayer.AppendSeparator();
                }
                if (mSequenceElements->GetHiddenTimingCount() > 0 && mSequenceElements->GetCurrentView() == MASTER_VIEW) {
                    mnuLayer.Append(ID_ROW_MNU_SHOWALLTIMING, "Show All Timing Tracks");
                }
                bool canPromote = false;
                ModelElement* me = dynamic_cast<ModelElement*>(element);
                if (element->GetType() == ElementType::ELEMENT_TYPE_STRAND || element->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                    me = dynamic_cast<SubModelElement*>(element)->GetModelElement();
                }
                if (me != nullptr && me->GetSubModelAndStrandCount() > 0) {
                    if (element->GetType() != ElementType::ELEMENT_TYPE_SUBMODEL) {
                        canPromote = true;
                    }
                    mnuLayer.Append(ID_ROW_MNU_TOGGLE_STRANDS, "Toggle Strands");
                    if (ri->strandIndex >= 0) {
                        mnuLayer.Append(ID_ROW_MNU_TOGGLE_NODES, "Toggle Nodes");
                    }
                }
                else {
                    mnuLayer.Append(ID_ROW_MNU_TOGGLE_STRANDS, "Toggle Models");
                }
                mnuLayer.Append(ID_ROW_MNU_SHOW_EFFECTS, "Show All Effects");
                mnuLayer.Append(ID_ROW_MNU_COLLAPSEALLMODELS, "Collapse All Models");
                mnuLayer.Append(ID_ROW_MNU_COLLAPSEALLLAYERS, "Collapse All Layers");
                if (ri->nodeIndex > -1) {
                    StrandElement* se = dynamic_cast<StrandElement*>(element);
                    if (se != nullptr && se->GetNodeLayer(ri->nodeIndex)->GetEffectCount() == 0) {
                        mnuLayer.Append(ID_ROW_MNU_CONVERT_TO_EFFECTS, "Convert To Effect");
                    }
                }
                else {
                    if (element->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                        Model* m = mSequenceElements->GetXLightsFrame()->AllModels[ri->element->GetModelName()];
                        if (m != nullptr && m->GetDisplayAs() != "ModelGroup") {
                            mnuLayer.Append(ID_ROW_MNU_CONVERT_TO_EFFECTS, "Convert To Effect");
                        }
                    }
                    else if (element->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        mnuLayer.Append(ID_ROW_MNU_CONVERT_TO_EFFECTS, "Convert To Effect");
                    }
                }

                if (canPromote) {
                    mnuLayer.Append(ID_ROW_MNU_PROMOTE_EFFECTS, "Promote Node Effects");
                }
                mnuLayer.AppendSeparator();

                Model* m = mSequenceElements->GetXLightsFrame()->AllModels[ri->element->GetModelName()];
                wxMenu* rowMenu = new wxMenu();
                wxMenu* modelMenu = new wxMenu();
                if (element->IsRenderDisabled()) {
                    modelMenu->Append(ID_ROW_MNU_RENDERENABLE_MODEL, "Enable Render");
                } else {
                    modelMenu->Append(ID_ROW_MNU_RENDERDISABLE_MODEL, "Disable Render");
                }

                bool modelDisabled = false;
                for (size_t i = 0; i < mSequenceElements->GetElementCount(); ++i) {
                    auto e = mSequenceElements->GetElement(i);
                    if (e->IsRenderDisabled()) {
                        modelDisabled = true;
                        break;
                    }
                }
                if (modelDisabled) {
                    modelMenu->Append(ID_ROW_MNU_RENDERENABLE_ALL, "Enable Render On All Models");
                } 

                modelMenu->Append(ID_ROW_MNU_PLAY_MODEL, "Play");
                modelMenu->Append(ID_ROW_MNU_EXPORT_MODEL, "Export")->Enable(m != nullptr && m->GetDisplayAs() != "ModelGroup");
                modelMenu->Append(ID_ROW_MNU_EXPORT_RENDERED_MODEL, "Render and Export")->Enable(m != nullptr && m->GetDisplayAs() != "ModelGroup");
                modelMenu->Append(ID_ROW_MNU_EXPORT_MODEL_SELECTED_EFFECTS, "Export Selected Model Effects")->Enable(m != nullptr && m->GetDisplayAs() != "ModelGroup" && element->GetSelectedEffectCount() > 0);
                modelMenu->Append(ID_ROW_MNU_EXPORT_RENDERED_MODEL_SELECTED_EFFECTS, "Render and Export Selected Model Effects")->Enable(m != nullptr && m->GetDisplayAs() != "ModelGroup" && element->GetSelectedEffectCount() > 0);
                rowMenu->Append(ID_ROW_MNU_SELECT_ROW_EFFECTS, "Select Effects");
                modelMenu->Append(ID_ROW_MNU_SELECT_MODEL_EFFECTS, "Select Effects");
                rowMenu->Append(ID_ROW_MNU_CUT_ROW, "Cut Effects");
                modelMenu->Append(ID_ROW_MNU_CUT_MODEL, "Cut Effects");
                rowMenu->Append(ID_ROW_MNU_COPY_ROW, "Copy Effects");
                modelMenu->Append(ID_ROW_MNU_COPY_MODEL, "Copy Effects");
                wxMenuItem* menu_paste = rowMenu->Append(ID_ROW_MNU_PASTE_ROW, "Paste Effects");
                wxMenuItem* menu_pastem = modelMenu->Append(ID_ROW_MNU_PASTE_MODEL, "Paste Effects");
                if (!mCanPaste) {
                    menu_paste->Enable(false);
                    menu_pastem->Enable(false);
                }
                rowMenu->Append(ID_ROW_MNU_DELETE_ROW_EFFECTS, "Delete Effects");
                rowMenu->Append(ID_ROW_MNU_CREATE_TIMING_FROM_EFFECTS, "Create Timing From Effects");
                modelMenu->Append(ID_ROW_MNU_DELETE_MODEL_EFFECTS, "Delete Effects");
                modelMenu->Append(ID_ROW_MNU_DELETE_MODEL_STRAND_EFFECTS, "Delete Strand Effects");
                modelMenu->Append(ID_ROW_MNU_DELETE_MODEL_NODE_EFFECTS, "Delete Node Effects");

                if (m != nullptr && m->GetDisplayAs() == "ModelGroup") {
                    modelMenu->Append(ID_ROW_MNU_MODEL_CONVERTTOPERMODEL, "Convert Effects to 'Per Model'");
                    rowMenu->Append(ID_ROW_MNU_ROW_CONVERTTOPERMODEL, "Convert Effects to 'Per Model'");
                }

                mnuLayer.AppendSubMenu(modelMenu, "Model");
                mnuLayer.AppendSubMenu(rowMenu, "Row");
                rowMenu->Connect(wxEVT_MENU, (wxObjectEventFunction)&RowHeading::OnLayerPopup, nullptr, this);
                modelMenu->Connect(wxEVT_MENU, (wxObjectEventFunction)&RowHeading::OnLayerPopup, nullptr, this);
            }
            else {
                EffectLayer* el = element->GetEffectLayer(ri->layerIndex);
                if (el != nullptr) {
                    mnuLayer.Append(ID_ROW_MNU_ADD_TIMING_TRACK, "Add Timing Track");
                    mnuLayer.Append(ID_ROW_MNU_RENAME_TIMING_TRACK, "Rename Timing Track");
                    mnuLayer.Append(ID_ROW_MNU_DELETE_TIMING_TRACK, "Delete Timing Track");
                    mnuLayer.Append(ID_ROW_MNU_IMPORT_TIMING_TRACK, "Import Timing Track");
                    mnuLayer.Append(ID_ROW_MNU_EXPORT_TIMING_TRACK, "Export Timing Track");
                    mnuLayer.Append(ID_ROW_MNU_UNFIX_TIMING_TRACK, "Make Timing Track Variable")->Enable(el->IsFixedTimingLayer());
                    if (mSequenceElements->GetCurrentView() == MASTER_VIEW) {
                        mnuLayer.Append(ID_ROW_MNU_HIDEALLTIMING, "Hide All Timing Tracks");
                    }
                    mnuLayer.Append(ID_ROW_MNU_SELECT_TIMING_EFFECTS, "Select Timing Marks");
                    mnuLayer.Append(ID_ROW_MNU_IMPORT_NOTES, "Import Notes");
                    mnuLayer.AppendSeparator();
                    mnuLayer.Append(ID_ROW_MNU_IMPORT_LYRICS, "Import Lyrics");
                    TimingElement *te = dynamic_cast<TimingElement*>(element);
                    if (te->GetSubType() == "") {
                        mnuLayer.Append(ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES, "Breakdown Phrases");
                        if (element->GetEffectLayerCount() > 1) {
                            mnuLayer.Append(ID_ROW_MNU_BREAKDOWN_TIMING_WORDS, "Breakdown Words");
                        }
                        if (element->GetEffectLayerCount() == 2) {
                            mnuLayer.Append(ID_ROW_MNU_REMOVE_TIMING_WORDS, "Remove Words");
                        }
                        else if (element->GetEffectLayerCount() == 3) {
                            if (ri->layerIndex == 2) {
                                mnuLayer.Append(ID_ROW_MNU_REMOVE_TIMING_PHONEMES, "Remove Phonemes");
                            }
                            else {
                                mnuLayer.Append(ID_ROW_MNU_REMOVE_TIMING_WORDS_PHONEMES, "Remove Words and Phonemes");
                            }
                        }
                    }
                    mnuLayer.AppendSeparator();
                    mnuLayer.Append(ID_ROW_MNU_COPY_ROW, "Copy Row");
                    wxMenuItem* menu_paste = mnuLayer.Append(ID_ROW_MNU_PASTE_ROW, "Paste Row");
                    if (!mCanPaste) {
                        menu_paste->Enable(false);
                    }
                }
            }
        }

        mnuLayer.AppendSeparator();
        mnuLayer.Append(ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS, "Edit Display Elements");
        mnuLayer.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&RowHeading::OnLayerPopup, nullptr, this);
        Refresh(false);
        Update();
        PopupMenu(&mnuLayer);
    }
}

void RowHeading::OnLayerPopup(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Row_Information_Struct* ri = mSequenceElements->GetVisibleRowInformation(mSelectedRow);
    if (ri == nullptr || mSequenceElements == nullptr)
        return;

    Element* element = ri->element;
    if (element == nullptr)
        return;

    int layer_index = ri->layerIndex;
    int id = event.GetId();
    if (id == ID_ROW_MNU_INSERT_LAYER_ABOVE) {
        element->InsertEffectLayer(layer_index);
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_INSERT_LAYER_BELOW) {
        if (layer_index < element->GetEffectLayerCount() - 1) {
            element->InsertEffectLayer(layer_index + 1);
        } else {
            element->AddEffectLayer();
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_INSERT_LAYERS_BELOW) {
        int numtoinsert = wxGetNumberFromUser("Enter number of layers to insert", "Layers", "Insert multiple layers", 2, 1, 100, this, wxGetMousePosition());

        if (numtoinsert > 0) {
            if (layer_index < element->GetEffectLayerCount() - 1) {
                for (int i = 0; i < numtoinsert; i++) {
                    element->InsertEffectLayer(layer_index + 1);
                }
            } else {
                for (int i = 0; i < numtoinsert; i++) {
                    element->AddEffectLayer();
                }
            }
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_DELETE_LAYER) {
        logger_base.debug("RowHeading::OnLayerPopup Deleting layer.");
        if (mSequenceElements->GetVisibleRowInformation(mSelectedRow) != nullptr) {
            int layerIndex = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->layerIndex;

            int answer = wxYES;

            // only prompt the user if there are effects on the layer
            if (element->GetEffectLayer(layerIndex)->GetEffectCount() > 0) {
                wxString prompt = wxString::Format("Layer contains one or more effects. Are you sure you want to delete 'Layer %d' of '%s'?",
                                                   layerIndex + 1, element->GetModelName());
                wxString caption = "Confirm Layer Deletion";

                answer = wxMessageBox(prompt, caption, wxYES_NO);
            }

            if (answer == wxYES) {
                element->RemoveEffectLayer(layerIndex);
                wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
                wxPostEvent(GetParent(), eventRowHeaderChanged);
            }
        }
    } else if (id == ID_ROW_MNU_DELETE_LAYERS) {
        logger_base.debug("RowHeading::OnLayerPopup Deleting layers.");
        if (mSequenceElements->GetVisibleRowInformation(mSelectedRow) != nullptr) {
            int layerIndex = mSequenceElements->GetVisibleRowInformation(mSelectedRow)->layerIndex;
            int bottomLayer = element->GetEffectLayerCount();

            int numtoDelete = wxGetNumberFromUser("Enter number of layers to delete", "Layers", "Delete multiple layers", bottomLayer - layerIndex, 1, bottomLayer - layerIndex, this);

            if (numtoDelete > 0) {
                int startDeleteLayer = layerIndex + (numtoDelete - 1);

                bool containsEffects = false;
                bool deleteLayers = true;
                for (int deleteLayer = startDeleteLayer; deleteLayer >= layerIndex; deleteLayer--) {
                    // Check for effects
                    if (element->GetEffectLayer(deleteLayer)->GetEffectCount() > 0) {
                        containsEffects = true;
                        break;
                    }
                }

                if (containsEffects == true) {
                    wxString prompt = wxString::Format("One or more layers contain effects.  Delete?");
                    wxString caption = "Confirm Layer Deletion";

                    int answer = wxMessageBox(prompt, caption, wxYES_NO);
                    if (answer == wxNO) {
                        deleteLayers = false;
                    }
                }
                if (deleteLayers == true) {
                    for (int deleteLayer = startDeleteLayer; deleteLayer >= layerIndex; deleteLayer--) {
                        element->RemoveEffectLayer(deleteLayer);
                    }

                    // Add a new layer if the topmost layer was removed
                    if (layerIndex == 0)
                        element->AddEffectLayer();

                    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
                    wxPostEvent(GetParent(), eventRowHeaderChanged);
                }
            }
        }
    } else if (id == ID_ROW_MNU_DELETE_UNUSEDLAYERS) {
        logger_base.debug("RowHeading::OnLayerPopup Deleting unused layers.");
        bool deleted = false;
        for (int i = 0; i < element->GetEffectLayerCount(); ++i) {
            if (element->GetEffectLayer(i)->GetEffectCount() > 0) {
                // dont delete this layer
            } else {
                if (element->GetEffectLayerCount() == 1) {
                    // last layer ... dont delete it
                } else {
                    element->RemoveEffectLayer(i);
                    --i;
                    deleted = true;
                }
            }
        }
        if (deleted) {
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_ADD_TIMING_TRACK) {
        bool timing_added = false;
        xLightsXmlFile* xml_file = mSequenceElements->GetXLightsFrame()->CurrentSeqXmlFile;
        NewTimingDialog dialog(this);
        OptimiseDialogPosition(&dialog);
        dialog.Fit();
        if (xml_file->GetFrequency() < 40) {
            dialog.RemoveChoice("25ms");
        }
        if (xml_file->GetFrequency() < 20) {
            dialog.RemoveChoice("50ms");
        }

        VAMPPluginDialog vamp(this);
        std::list<std::string> plugins;
        if (xml_file->HasAudioMedia()) {
            plugins = xml_file->GetMedia()->GetVamp()->GetAvailablePlugins(xml_file->GetMedia());
            if (plugins.size() == 0) {
                dialog.Choice_New_Fixed_Timing->Append("Download Queen Mary Vamp plugins for audio analysis");
            } else {
                for (const auto& it : plugins) {
                    dialog.Choice_New_Fixed_Timing->Append(it);
                }
            }
        }

        dialog.Fit();
        OptimiseDialogPosition(&dialog);

        wxString name = "";
        if (dialog.ShowModal() == wxID_OK) {
            std::string selected_timing = dialog.GetTiming().ToStdString();

            if (selected_timing == "Download Queen Mary Vamp plugins for audio analysis") {
                DownloadVamp();
            } else {
                if (std::find(plugins.begin(), plugins.end(), selected_timing) != plugins.end()) {
                    name = vamp.ProcessPlugin(xml_file, mSequenceElements->GetXLightsFrame(), selected_timing, xml_file->GetMedia());
                    if (name != "") {
                        timing_added = true;
                    }
                } else if (selected_timing == "FPP Commands" || selected_timing == "FPP Effects") {
                    bool first = true;
                    wxString subType = selected_timing;
                    wxTextEntryDialog te(this, "Enter a name for the " + selected_timing + " track", wxGetTextFromUserPromptStr, selected_timing);
                    OptimiseDialogPosition(&te);
                    while (first || xml_file->TimingAlreadyExists(selected_timing, mSequenceElements->GetXLightsFrame()) || selected_timing == "") {
                        first = false;

                        auto base = selected_timing;

                        int suffix = 2;
                        while (xml_file->TimingAlreadyExists(selected_timing, mSequenceElements->GetXLightsFrame())) {
                            selected_timing = wxString::Format("%s_%d", base, suffix++);
                        }

                        te.SetValue(selected_timing);
                        if (te.ShowModal() == wxID_OK) {
                            selected_timing = te.GetValue();
                            selected_timing = RemoveUnsafeXmlChars(selected_timing);
                        } else {
                            selected_timing = "";
                            break;
                        }
                    }

                    if (selected_timing != "") {
                        xml_file->AddNewTimingSection(selected_timing, mSequenceElements->GetXLightsFrame(), subType);
                        timing_added = true;
                    }

                } else if (selected_timing == "Empty") {
                    bool first = true;
                    wxTextEntryDialog te(this, "Enter a name for the timing track", wxGetTextFromUserPromptStr, selected_timing);
                    OptimiseDialogPosition(&te);
                    while (first || xml_file->TimingAlreadyExists(selected_timing, mSequenceElements->GetXLightsFrame()) || selected_timing == "") {
                        first = false;

                        auto base = selected_timing;

                        int suffix = 2;
                        while (xml_file->TimingAlreadyExists(selected_timing, mSequenceElements->GetXLightsFrame())) {
                            selected_timing = wxString::Format("%s_%d", base, suffix++);
                        }

                        te.SetValue(selected_timing);
                        if (te.ShowModal() == wxID_OK) {
                            selected_timing = te.GetValue();
                            selected_timing = RemoveUnsafeXmlChars(selected_timing);
                        } else {
                            selected_timing = "";
                            break;
                        }
                    }

                    if (selected_timing != "") {
                        xml_file->AddFixedTimingSection(selected_timing, mSequenceElements->GetXLightsFrame());
                        timing_added = true;
                    }
                } else if (!xml_file->TimingAlreadyExists(selected_timing, mSequenceElements->GetXLightsFrame())) {
                    name = selected_timing;
                    if (selected_timing == "Metronome") {
                        int base_timing = xml_file->GetFrameMS();
                        wxNumberEntryDialog dlg(this, "Enter metronome timing", "Milliseconds", "Metronome timing", 10 * base_timing, base_timing, 60000);
                        OptimiseDialogPosition(&dlg);
                        if (dlg.ShowModal() == wxID_OK) {
                            int ms = (dlg.GetValue() + base_timing / 2) / base_timing * base_timing;

                            if (ms != dlg.GetValue()) {
                                DisplayWarning(wxString::Format("Timing adjusted to match sequence timing %dms -> %dms", dlg.GetValue(), ms).ToStdString());
                            }
                            wxString ttn = wxString::Format("%dms Metronome", ms);
                            if (!xml_file->TimingAlreadyExists(ttn.ToStdString(), mSequenceElements->GetXLightsFrame())) {
                                xml_file->AddFixedTimingSection(ttn.ToStdString(), mSequenceElements->GetXLightsFrame());
                                timing_added = true;
                            }
                        }
                    } else {
                        xml_file->AddFixedTimingSection(selected_timing, mSequenceElements->GetXLightsFrame());
                        timing_added = true;
                    }
                } else {
                    DisplayError(wxString::Format("Fixed Timing section %s already exists!", selected_timing).ToStdString());
                }
            }
        }
        dialog.Destroy();

        if (timing_added) {
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_RENAME_TIMING_TRACK) {
        std::string name = wxGetTextFromUser("What is the new name of the timing track?", "Timing Track Name", element->GetName()).ToStdString();
        name = RemoveUnsafeXmlChars(name);
        if (mSequenceElements->ElementExists(name)) {
            DisplayError("Timing name already exists in sequence as a model or another timing.");
        } else if (name.size() > 0) {
            mSequenceElements->GetXLightsFrame()->AbortRender(); // stop rendering in case there is an effect referring to the timing track we are about to rename
            std::string oldname = element->GetName();
            mSequenceElements->GetXLightsFrame()->RenameTimingElement(oldname, name);
        }
    } else if (id == ID_ROW_MNU_DELETE_TIMING_TRACK) {
        wxString prompt = wxString::Format("Delete 'Timing Track '%s'?", element->GetName());
        wxString caption = "Confirm Timing Track Deletion";

        int answer = wxMessageBox(prompt, caption, wxYES_NO);
        if (answer == wxYES) {
            mSequenceElements->GetXLightsFrame()->AbortRender(); // stop rendering in case there is an effect referring to the timing track we are about to delete
            mSequenceElements->DeleteElement(element->GetModelName());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_UNFIX_TIMING_TRACK) {
        TimingElement* te = dynamic_cast<TimingElement*>(element);
        te->Unfix();
    } else if (id == ID_ROW_MNU_EXPORT_TIMING_TRACK) {
        wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filetypes;
        if (mSequenceElements->GetElement(element->GetName())->GetEffectLayerCount() == 3) {
            filetypes = "Timing files (*.xtiming)|*.xtiming|Papagayo files (*.pgo)|*.pgo";
        } else {
            filetypes = "Timing files (*.xtiming)|*.xtiming";
        }
        wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, element->GetModelName(), wxEmptyString, filetypes, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (filename.IsEmpty())
            return;
        wxFileName fn(filename);
        TimingElement* te = dynamic_cast<TimingElement*>(element);
        if (fn.GetExt().Lower() == "xtiming") {
            SelectTimingsDialog dlg(this);
            for (int i = 0; i < mSequenceElements->GetNumberOfTimingElements(); i++) {
                TimingElement* te2 = mSequenceElements->GetTimingElement(i);
                dlg.CheckListBox_Timings->Insert(te2->GetName(), i);
                if (te2->GetName() == element->GetName()) {
                    dlg.CheckListBox_Timings->Check(i);
                }
            }
            OptimiseDialogPosition(&dlg);
            if (mSequenceElements->GetNumberOfTimingElements() == 1 || dlg.ShowModal() == wxID_OK) {
                wxArrayInt sel;
                dlg.CheckListBox_Timings->GetCheckedItems(sel);

                if (sel.size() > 0) {
                    wxFile f(filename);
                    logger_base.info("Saving to xtiming file %s.", (const char*)filename.c_str());
                    if (!f.Create(filename, true) || !f.IsOpened()) {
                        DisplayError(wxString::Format("Unable to create file %s. Error %d", filename, f.GetLastError()).ToStdString());
                        return;
                    }
                    wxString v = xlights_version_string;
                    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
                    if (sel.size() > 1) {
                        f.Write("<timings>\n");
                    }
                    for (size_t i = 0; i < dlg.CheckListBox_Timings->GetCount(); i++) {
                        if (dlg.CheckListBox_Timings->IsChecked(i)) {
                            TimingElement* tee = dynamic_cast<TimingElement*>(mSequenceElements->GetElement(dlg.CheckListBox_Timings->GetString(i).ToStdString()));

                            wxString td = wxString(tee->GetExport().c_str());
                            f.Write("<timing ");
                            f.Write(wxString::Format("name=\"%s\" ", XmlSafe(tee->GetName())));
                            f.Write(wxString::Format("SourceVersion=\"%s\">\n", v));
                            f.Write(td);
                            f.Write("</timing>\n");
                        }
                    }
                    if (sel.size() > 1) {
                        f.Write("</timings>\n");
                    }
                    f.Close();
                }
            }
        } else if (fn.GetExt().Lower() == "pgo") {
            wxFile f(filename);
            logger_base.info("Saving to papagayo file %s.", (const char*)filename.c_str());
            if (!f.Create(filename, true) || !f.IsOpened()) {
                DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
                return;
            }
            wxString td = wxString(te->GetPapagayoExport(mSequenceElements->GetFrequency()).c_str());
            f.Write(td);
            f.Close();
        }
    } else if (id == ID_ROW_MNU_IMPORT_TIMING_TRACK) {
        wxCommandEvent playEvent(EVT_IMPORT_TIMING);
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_IMPORT_NOTES) {
        wxCommandEvent importNotesEvent(EVT_IMPORT_NOTES);
        wxPostEvent(GetParent(), importNotesEvent);
    } else if (id == ID_ROW_MNU_IMPORT_LYRICS) {
        mSequenceElements->ImportLyrics(dynamic_cast<TimingElement*>(element), GetParent());
    } else if (id == ID_ROW_MNU_BREAKDOWN_TIMING_PHRASES) {
        int result = wxOK;
        if (element->GetEffectLayerCount() > 1) {
            result = wxMessageBox("Breakdown phrases? Any existing words and phonemes will be deleted.", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        }
        if (result == wxOK) {
            BreakdownTimingPhrases(dynamic_cast<TimingElement*>(element));
        }
    } else if (id == ID_ROW_MNU_BREAKDOWN_TIMING_WORDS) {
        int result = wxOK;
        if (element->GetEffectLayerCount() > 2) {
            result = wxMessageBox("Breakdown words? Any existing phonemes will be deleted.", "Confirm Action", wxOK | wxCANCEL | wxCENTER);
        }
        if (result == wxOK) {
            BreakdownTimingWords(dynamic_cast<TimingElement*>(element));
        }
    } else if (id == ID_ROW_MNU_REMOVE_TIMING_WORDS) {
        auto te = dynamic_cast<TimingElement*>(element);
        if (te != nullptr) {
            te->RemoveEffectLayer(1);
        }
    } else if (id == ID_ROW_MNU_REMOVE_TIMING_PHONEMES) {
        auto te = dynamic_cast<TimingElement*>(element);
        if (te != nullptr) {
            te->RemoveEffectLayer(2);
        }
    } else if (id == ID_ROW_MNU_HIDEALLTIMING) {
        mSequenceElements->HideAllTimingTracks(true);
    } else if (id == ID_ROW_MNU_SHOWALLTIMING) {
        mSequenceElements->HideAllTimingTracks(false);
    } else if (id == ID_ROW_MNU_REMOVE_TIMING_WORDS_PHONEMES) {
        auto te = dynamic_cast<TimingElement*>(element);
        if (te != nullptr) {
            te->RemoveEffectLayer(2);
            te->RemoveEffectLayer(1);
        }
    } else if (id == ID_ROW_MNU_EXPORT_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetInt(0);
        playEvent.SetString(element->GetModelName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_EXPORT_RENDERED_MODEL) {
        wxCommandEvent playEvent(EVT_EXPORT_MODEL);
        playEvent.SetInt(1);
        playEvent.SetString(element->GetModelName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_RENDERDISABLE_MODEL) {
        element->SetRenderDisabled(true);
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    } else if (id == ID_ROW_MNU_RENDERENABLE_MODEL) {
        element->SetRenderDisabled(false);
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    } else if (id == ID_ROW_MNU_RENDERENABLE_ALL) {
        for (size_t i = 0; i < mSequenceElements->GetElementCount(); ++i) {
            auto e = mSequenceElements->GetElement(i);
            if (e->IsRenderDisabled()) {
                e->SetRenderDisabled(false);
            }
        }
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    } else if (id == ID_ROW_MNU_EXPORT_MODEL_SELECTED_EFFECTS) {
        int startFrame = element->GetFirstSelectedEffectStartMS();
        int endFrame = element->GetLastSelectedEffectEndMS();
        if (startFrame != -1 && endFrame != -1) {
            xLightsXmlFile* xml_file = mSequenceElements->GetXLightsFrame()->CurrentSeqXmlFile;
            startFrame /= xml_file->GetFrameMS();
            endFrame /= xml_file->GetFrameMS();
            wxCommandEvent playEvent(EVT_EXPORT_MODEL);
            playEvent.SetInt(0);
            playEvent.SetString(element->GetModelName() + wxString::Format("|%d|%d", startFrame, endFrame));
            wxPostEvent(GetParent(), playEvent);
        }
    } else if (id == ID_ROW_MNU_EXPORT_RENDERED_MODEL_SELECTED_EFFECTS) {
        int startFrame = element->GetFirstSelectedEffectStartMS();
        int endFrame = element->GetLastSelectedEffectEndMS();
        if (startFrame != -1 && endFrame != -1) {
            xLightsXmlFile* xml_file = mSequenceElements->GetXLightsFrame()->CurrentSeqXmlFile;
            startFrame /= xml_file->GetFrameMS();
            endFrame /= xml_file->GetFrameMS();
            wxCommandEvent playEvent(EVT_EXPORT_MODEL);
            playEvent.SetInt(1);
            playEvent.SetString(element->GetModelName() + wxString::Format("|%d|%d", startFrame, endFrame));
            wxPostEvent(GetParent(), playEvent);
        }
    } else if (id == ID_ROW_MNU_PLAY_MODEL) {
        wxCommandEvent playEvent(EVT_PLAY_MODEL);
        playEvent.SetString(element->GetModelName());
        wxPostEvent(GetParent(), playEvent);
    } else if (id == ID_ROW_MNU_CUT_ROW) {
        wxCommandEvent cutRowEvent(EVT_CUT_MODEL_EFFECTS);
        cutRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), cutRowEvent);
        mCanPaste = true;
    } else if (id == ID_ROW_MNU_CUT_MODEL) {
        wxCommandEvent cutRowEvent(EVT_CUT_MODEL_EFFECTS);
        cutRowEvent.SetInt(mSelectedRow);
        cutRowEvent.SetString("All");
        wxPostEvent(GetParent(), cutRowEvent);
        mCanPaste = true;
    } else if (id == ID_ROW_MNU_COPY_ROW) {
        wxCommandEvent copyRowEvent(EVT_COPY_MODEL_EFFECTS);
        copyRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), copyRowEvent);
        mCanPaste = true;
    } else if (id == ID_ROW_MNU_COPY_MODEL) {
        wxCommandEvent copyRowEvent(EVT_COPY_MODEL_EFFECTS);
        copyRowEvent.SetInt(mSelectedRow);
        copyRowEvent.SetString("All");
        wxPostEvent(GetParent(), copyRowEvent);
        mCanPaste = true;
    } else if (id == ID_ROW_MNU_DELETE_ROW_EFFECTS) {
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        m_parent->ProcessWindowEvent(eventUnSelected);
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        if (layer_index < element->GetEffectLayerCount()) {
            if (ri->nodeIndex == -1) {
                element->GetEffectLayer(layer_index)->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
            } else {
                StrandElement* se = (StrandElement*)element;
                NodeLayer* nl = se->GetNodeLayer(ri->nodeIndex, false);
                if (nl != nullptr) {
                    nl->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
                }
            }
        }
    } else if (id == ID_ROW_MNU_SELECT_ROW_EFFECTS) {
        if (layer_index < element->GetEffectLayerCount()) {
            if (ri->nodeIndex == -1) {
                element->GetEffectLayer(layer_index)->SelectAllEffects();
            } else {
                StrandElement* se = (StrandElement*)element;
                NodeLayer* nl = se->GetNodeLayer(ri->nodeIndex, false);
                if (nl != nullptr) {
                    nl->SelectAllEffects();
                }
            }
        }
    } else if (id == ID_ROW_MNU_ROW_CONVERTTOPERMODEL) {
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        if (layer_index < element->GetEffectLayerCount()) {
            if (ri->nodeIndex == -1) {
                element->GetEffectLayer(layer_index)->ConvertEffectsToPerModel(mSequenceElements->get_undo_mgr());
            } else {
                StrandElement* se = (StrandElement*)element;
                NodeLayer* nl = se->GetNodeLayer(ri->nodeIndex, false);
                if (nl != nullptr) {
                    nl->ConvertEffectsToPerModel(mSequenceElements->get_undo_mgr());
                }
            }
        }
        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    } else if (id == ID_ROW_MNU_MODEL_CONVERTTOPERMODEL) {
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        for (int i = 0; i < element->GetEffectLayerCount(); i++) {
            element->GetEffectLayer(i)->ConvertEffectsToPerModel(mSequenceElements->get_undo_mgr());
        }

        wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
        wxPostEvent(GetParent(), eventForceRefresh);
    } else if (id == ID_ROW_MNU_SELECT_MODEL_EFFECTS) {
        for (int i = 0; i < element->GetEffectLayerCount(); i++) {
            element->GetEffectLayer(i)->SelectAllEffects();
        }
    } else if (id == ID_ROW_MNU_SELECT_TIMING_EFFECTS) {
        for (int i = 0; i < element->GetEffectLayerCount(); i++) {
            element->GetEffectLayer(i)->SelectAllEffects();
        }
    } else if (id == ID_ROW_MNU_DELETE_MODEL_EFFECTS) {
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        m_parent->ProcessWindowEvent(eventUnSelected);
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        for (int i = 0; i < element->GetEffectLayerCount(); ++i) {
            if (element->GetEffectLayer(i)->GetEffectCount() > 0) {
                element->GetEffectLayer(i)->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
            }
        }
    } else if (id == ID_ROW_MNU_DELETE_MODEL_STRAND_EFFECTS) {
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        m_parent->ProcessWindowEvent(eventUnSelected);
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        auto me = dynamic_cast<ModelElement*>(element);
        if (me != nullptr) {
            for (size_t s = 0; s < me->GetStrandCount(); ++s) {
                auto se = me->GetStrand(s);
                if (se != nullptr) {
                    for (int i = 0; i < se->GetEffectLayerCount(); ++i) {
                        if (se->GetEffectLayer(i)->GetEffectCount() > 0) {
                            se->GetEffectLayer(i)->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
                        }
                    }
                }
            }
        }
    } else if (id == ID_ROW_MNU_DELETE_MODEL_NODE_EFFECTS) {
        wxCommandEvent eventUnSelected(EVT_UNSELECTED_EFFECT);
        m_parent->ProcessWindowEvent(eventUnSelected);
        mSequenceElements->get_undo_mgr().CreateUndoStep();
        auto me = dynamic_cast<ModelElement*>(element);
        if (me != nullptr) {
            for (size_t s = 0; s < me->GetStrandCount(); ++s) {
                auto se = me->GetStrand(s);
                if (se != nullptr) {
                    for (size_t n = 0; n < se->GetNodeLayerCount(); ++n) {
                        if (se->GetNodeLayer(n)->GetEffectCount() > 0) {
                            se->GetNodeLayer(n)->RemoveAllEffects(&mSequenceElements->get_undo_mgr());
                        }
                    }
                }
            }
        }

        // force redraw
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);

    } else if (id == ID_ROW_MNU_PASTE_ROW) {
        wxCommandEvent pasteRowEvent(EVT_PASTE_MODEL_EFFECTS);
        pasteRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), pasteRowEvent);
    } else if (id == ID_ROW_MNU_PASTE_MODEL) {
        wxCommandEvent pasteRowEvent(EVT_PASTE_MODEL_EFFECTS);
        pasteRowEvent.SetString("All");
        pasteRowEvent.SetInt(mSelectedRow);
        wxPostEvent(GetParent(), pasteRowEvent);
    } else if (id == ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS) {
        wxCommandEvent displayElementEvent(EVT_SHOW_DISPLAY_ELEMENTS);
        wxPostEvent(GetParent(), displayElementEvent);
    } else if (id == ID_ROW_MNU_TOGGLE_STRANDS) {
        ModelElement* me = dynamic_cast<ModelElement*>(element);
        if (me == nullptr) {
            SubModelElement* se = dynamic_cast<SubModelElement*>(element);
            me = se->GetModelElement();
        }
        if (me != nullptr) {
            me->ShowStrands(!me->ShowStrands());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            eventRowHeaderChanged.SetString(element->GetModelName());
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_SHOW_EFFECTS) {
        logger_base.debug("RowHeading::OnLayerPopup Show effects.");
        int view = mSequenceElements->GetCurrentView();
        for (int i = 0; i < mSequenceElements->GetElementCount(view); ++i) {
            Element* e = mSequenceElements->GetElement(i, view);
            if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                if (ExpandElementIfEffects(e)) {
                    ModelElement* me = dynamic_cast<ModelElement*>(e);
                    if (me != nullptr)
                        me->ShowStrands(true);
                }
            }
        }
    } else if (id == ID_ROW_MNU_COLLAPSEALLMODELS) {
        logger_base.debug("RowHeading::OnLayerPopup Collapse all models.");
        int view = mSequenceElements->GetCurrentView();

        for (int i = 0; i < mSequenceElements->GetElementCount(view); ++i) {
            Element* e = mSequenceElements->GetElement(i, view);
            if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                ModelElement* me = dynamic_cast<ModelElement*>(e);
                if (me != nullptr) {
                    me->ShowStrands(false);
                }
            }
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_COLLAPSEALLLAYERS) {
        logger_base.debug("RowHeading::OnLayerPopup Collapse all layers.");

        int view = mSequenceElements->GetCurrentView();
        for (int i = 0; i < mSequenceElements->GetElementCount(view); ++i) {
            Element* e = mSequenceElements->GetElement(i, view);
            e->SetCollapsed(true);
        }
        wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
        wxPostEvent(GetParent(), eventRowHeaderChanged);
    } else if (id == ID_ROW_MNU_TOGGLE_NODES) {
        StrandElement* se = dynamic_cast<StrandElement*>(element);
        if (se != nullptr) {
            se->ShowNodes(!se->ShowNodes());
            wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
            wxPostEvent(GetParent(), eventRowHeaderChanged);
        }
    } else if (id == ID_ROW_MNU_CONVERT_TO_EFFECTS) {
        wxCommandEvent evt(EVT_CONVERT_DATA_TO_EFFECTS);
        evt.SetClientData(element);
        int i = ((ri->strandIndex << 16) & 0xFFFF0000) + (ri->nodeIndex & 0xFFFF);
        evt.SetInt(i);
        wxPostEvent(GetParent(), evt);
    } else if (id == ID_ROW_MNU_CREATE_TIMING_FROM_EFFECTS) {
        xLightsXmlFile* xml_file = mSequenceElements->GetXLightsFrame()->CurrentSeqXmlFile;

        wxString selectedTiming = "FromEffects";
        auto base = selectedTiming;

        int suffix = 2;
        while (xml_file->TimingAlreadyExists(selectedTiming, mSequenceElements->GetXLightsFrame())) {
            selectedTiming = wxString::Format("%s_%d", base, suffix++);
        }
        xml_file->AddFixedTimingSection(selectedTiming, mSequenceElements->GetXLightsFrame());
        auto te = mSequenceElements->GetTimingElement(selectedTiming);
        if (te != nullptr) {
            auto tl = te->GetEffectLayer(0);

            if (tl != nullptr) {
                mSequenceElements->get_undo_mgr().CreateUndoStep();
                if (layer_index < element->GetEffectLayerCount()) {
                    EffectLayer* el = nullptr;
                    if (ri->nodeIndex == -1) {
                        el = element->GetEffectLayer(layer_index);
                    } else {
                        StrandElement* se = (StrandElement*)element;
                        if (se != nullptr) {
                            el = se->GetNodeLayer(ri->nodeIndex, false);
                        }
                    }

                    if (el != nullptr) {
                        for (const auto& ef : el->GetAllEffects()) {
                            Effect* e = tl->AddEffect(0, "", "", "", ef->GetStartTimeMS(), ef->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                            mSequenceElements->get_undo_mgr().CaptureAddedEffect(te->GetName(), tl->GetIndex(), e->GetID());
                        }
                    }

                    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
                    wxPostEvent(GetParent(), eventRowHeaderChanged);
                }
            }
        }
    } else if (id == ID_ROW_MNU_PROMOTE_EFFECTS) {
        wxCommandEvent evt(EVT_PROMOTE_EFFECTS);
        SubModelElement* se = dynamic_cast<SubModelElement*>(element);
        if (se != nullptr) {
            evt.SetClientData(se->GetModelElement());
        } else {
            evt.SetClientData(element);
        }
        wxPostEvent(GetParent(), evt);
    }

    // Make sure message box is painted over by grid.
    wxCommandEvent eventForceRefresh(EVT_FORCE_SEQUENCER_REFRESH);
    wxPostEvent(GetParent(), eventForceRefresh);
}

bool RowHeading::ExpandElementIfEffects(Element* e)
{
    if (e == nullptr)
        return false;

    bool hasEffects = false;

    if (e->GetCollapsed()) {
        for (int layer = 0; layer < e->GetEffectLayerCount(); layer++) {
            EffectLayer* el = e->GetEffectLayer(layer);
            if (el->GetEffectCount() > 0) {
                e->SetCollapsed(false);
                break;
            }
        }
    }

    if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        ModelElement* me = dynamic_cast<ModelElement*>(e);
        Model* m = mSequenceElements->GetXLightsFrame()->AllModels[me->GetModelName()];

        if (m->GetDisplayAs() == "ModelGroup") {
            int view = mSequenceElements->GetCurrentView();
            ModelGroup* mg = dynamic_cast<ModelGroup*>(m);
            auto models = mg->ModelNames();
            for (auto it = models.begin(); it != models.end(); ++it) {
                ModelElement* mm = dynamic_cast<ModelElement*>(mSequenceElements->GetElement(*it));

                if (mm != nullptr && !ModelInView(*it, view)) {
                    hasEffects = mm->HasEffects();
                    hasEffects |= ExpandElementIfEffects(mSequenceElements->GetElement(*it));

                    if (hasEffects)
                        me->ShowStrands(true);
                }
            }
        } else {
            for (int i = 0; i < me->GetStrandCount(); ++i) {
                hasEffects |= ExpandElementIfEffects(me->GetStrand(i));
            }
            for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i) {
                hasEffects |= ExpandElementIfEffects(me->GetSubModel(i));
            }

            if (hasEffects)
                me->ShowStrands(true);
        }
    } else if (e->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        StrandElement* se = dynamic_cast<StrandElement*>(e);
        hasEffects = se->HasEffects();
        for (int k = 0; k < se->GetNodeLayerCount(); ++k) {
            NodeLayer* nl = se->GetNodeLayer(k, false);
            if (nl != nullptr && nl->HasEffectsInTimeRange(0, 9999999)) {
                se->ShowNodes(true);
                return true;
            }
        }
    } else {
        // Submodel
        SubModelElement* sme = dynamic_cast<SubModelElement*>(e);
        hasEffects = sme->HasEffects();
    }

    return hasEffects;
}

bool RowHeading::ModelInView(const std::string& model, int view) const
{
    for (size_t j = 0; j < mSequenceElements->GetElementCount(view); ++j) {
        if (model == mSequenceElements->GetElement(j, view)->GetName()) {
            return true;
        }
    }

    return false;
}

void RowHeading::BreakdownTimingPhrases(TimingElement* element)
{
    element->SetFixedTiming(0);
    EffectLayer* layer = element->GetEffectLayer(0);
    if (element->GetEffectLayerCount() > 1) {
        for (int k = element->GetEffectLayerCount() - 1; k > 0; --k) {
            EffectLayer* check_layer = element->GetEffectLayer(k);
            bool found_locked = false;
            for (auto&& e : check_layer->GetAllEffects()) {
                if (e->IsLocked()) {
                    found_locked = true;
                    break;
                }
            }
            if (found_locked) {
                wxMessageBox("Locked words/phonemes in the way - Can not break down phrases", "Error", wxOK);
                return;
            }
        }
    }

    // No locked elements in the way now
    if (element->GetEffectLayerCount() > 1) {
        for (int k = element->GetEffectLayerCount() - 1; k > 0; --k) {
            element->RemoveEffectLayer(k);
        }
    }
    mSequenceElements->get_undo_mgr().CreateUndoStep();
    EffectLayer* word_layer = element->AddEffectLayer();
    for (size_t i = 0; i < layer->GetEffectCount(); ++i) {
        Effect* effect = layer->GetEffect(i);
        std::string phrase = effect->GetEffectName();
        mSequenceElements->BreakdownPhrase(word_layer, effect->GetStartTimeMS(), effect->GetEndTimeMS(), phrase, mSequenceElements->get_undo_mgr());
    }
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(GetParent(), eventRowHeaderChanged);
}

void RowHeading::BreakdownTimingWords(TimingElement* element)
{
    if (element->GetEffectLayerCount() > 2) {
        EffectLayer* phoneme_layer = element->GetEffectLayer(2);
        bool found_locked = false;
        for (auto&& e : phoneme_layer->GetAllEffects()) {
            if (e->IsLocked()) {
                found_locked = true;
                break;
            }
        }
        if (found_locked) {
            wxMessageBox("Locked phonemes in the way - Can not break down words", "Error", wxOK);
            return;
        }

        element->RemoveEffectLayer(2);
    }
    mSequenceElements->get_undo_mgr().CreateUndoStep();
    EffectLayer* word_layer = element->GetEffectLayer(1);
    EffectLayer* phoneme_layer = element->AddEffectLayer();
    for (int i = 0; i < word_layer->GetEffectCount(); i++) {
        Effect* effect = word_layer->GetEffect(i);
        std::string word = effect->GetEffectName();
        mSequenceElements->BreakdownWord(phoneme_layer, effect->GetStartTimeMS(), effect->GetEndTimeMS(), word, mSequenceElements->get_undo_mgr());
    }
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    wxPostEvent(GetParent(), eventRowHeaderChanged);
}

bool RowHeading::HitTestCollapseExpand(int row, int x, bool* IsCollapsed)
{
    if (mSequenceElements->GetVisibleRowInformation(row)->element->GetType() != ElementType::ELEMENT_TYPE_TIMING &&
        x < DEFAULT_ROW_HEADING_MARGIN) {
        *IsCollapsed = mSequenceElements->GetVisibleRowInformation(row)->Collapsed;
        return true;
    }
    else {
        return false;
    }
}

bool RowHeading::HitTestTimingActive(int row,int x, bool* IsActive)
{
    if (mSequenceElements->GetVisibleRowInformation(row)->element->GetType() == ElementType::ELEMENT_TYPE_TIMING
        && x < DEFAULT_ROW_HEADING_MARGIN) {

        *IsActive = dynamic_cast<TimingElement*>(mSequenceElements->GetVisibleRowInformation(row)->element)->GetActive();
        return true;
    }
    return false;
}

void RowHeading::SetSelectedRow(int row_number)
{
    mSequenceElements->UnSelectAllElements();
    mSelectedRow = row_number;
    Refresh(false);
}

int RowHeading::GetMaxRows()
{
    int max = (int)(getHeight()/DEFAULT_ROW_HEADING_HEIGHT);
    return max;
}

static float ComputeRHFontSize() {
    // DEFAULT_ROW_HEADING_HEIGHT is either 16, 22, 30, 38, or 54
    // default size is appropriate for "22", scale others appropriately.
    // Since there are currently only 5 sizes, this would be better as
    // a switch statement, but this allows future sizes
    float fontSize = 15 * DEFAULT_ROW_HEADING_HEIGHT;
    fontSize /= 22.0f;
    if (fontSize < 9) {
        fontSize = 8;
    }
    return fontSize;
}
#ifdef __WXOSX__
static void SetFontPixelSize(wxFont &font, float f) {
    float i = font.GetPixelSize().y;
    float p = font.GetFractionalPointSize();
    // map to "points" so we can use the fractional sizing
    float points = f * p;
    points /= i;
    font.SetFractionalPointSize(points);
}
#else
static void SetFontPixelSize(wxFont &font, float f) {
    wxSize sz(0, (int)(std::round(f * 0.8f)));
    font.SetPixelSize(sz);
}
#endif

void RowHeading::render( wxPaintEvent& event )
{
#ifdef __LINUX__
    if(!IsShownOnScreen()) return;
#endif
    wxPaintDC dc(this);

    wxCoord w,h;
    dc.GetSize(&w,&h);
    xlColor rowHeaderCol = ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER);
    xlColor outlineCol(32, 32, 32);
    bool isDark = wxSystemSettings::GetAppearance().IsDark();
    if (isDark) {
        outlineCol.Set(55, 55, 55);
    }
    wxPen penOutline(outlineCol.asWxColor());

    wxBrush brush(rowHeaderCol.asWxColor(), wxBRUSHSTYLE_SOLID);
    dc.SetBrush(brush);
    dc.SetPen(penOutline);

    // If effect labels grow with row size then so should labels.
    auto font = dc.GetFont();
    
    // Note: DEFAULT_ROW_HEADING_HEIGHT is in PIXELS which is independent
    // of any scaling factor, thus, we need to set the font size in Pixels,
    // not points which would end up scaling depending on scale factor
    auto fontSize = ComputeRHFontSize();
    SetFontPixelSize(font, fontSize);
    dc.SetFont(font);

    int row = 0;
    int endY = 0;

    int dragRow = mSequenceElements->GetXLightsFrame()->GetMainSequencer()->PanelEffectGrid->GetDropRow();

    xlColor labelColor = ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER_TEXT);
    wxColor labelWxColor = labelColor.asWxColor();
    
    for (int i = 0; i < mSequenceElements->GetVisibleRowInformationSize(); i++) {
        Row_Information_Struct* rowInfo = mSequenceElements->GetVisibleRowInformation(i);
        wxString prefix;
        wxString layers;
        if (rowInfo->submodel) {
            prefix = "  ";
        }
        if (rowInfo->element->GetType() != ElementType::ELEMENT_TYPE_TIMING && rowInfo->element->GetEffectLayerCount() > 1) {
            layers = wxString::Format(" [%d]", (int)rowInfo->element->GetEffectLayerCount());
        }
        xlColor rowHeaderColor = GetHeaderColor(rowInfo, dragRow);
        wxBrush brush2(rowHeaderColor.asWxColor(), wxBRUSHSTYLE_SOLID);
        dc.SetBrush(brush2);
        int startY = DEFAULT_ROW_HEADING_HEIGHT*row;
        endY = DEFAULT_ROW_HEADING_HEIGHT*(row+1);
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.DrawRectangle(0, startY, w, DEFAULT_ROW_HEADING_HEIGHT + (i == 0 ? 0 : 1));

        Model* m = mSequenceElements->GetXLightsFrame()->AllModels[rowInfo->element->GetModelName()];
        if (m != nullptr && m->GetTagColour() != *wxBLACK) {
            wxBrush tagBrush(m->GetTagColour(), wxBRUSHSTYLE_SOLID);
            wxPen tagPen(m->GetTagColour());
            dc.SetBrush(tagBrush);
            dc.SetPen(tagPen);
            dc.DrawRectangle(2, startY + 2, DEFAULT_ROW_HEADING_MARGIN - 4, DEFAULT_ROW_HEADING_HEIGHT - 4);
        }

        dc.SetBrush(brush2);
        dc.SetPen(penOutline);
        if (labelColor.HasSufficientContrast(rowHeaderColor)) {
            dc.SetTextForeground(labelWxColor);
        } else if (xlBLACK.HasSufficientContrast(rowHeaderColor)) {
            dc.SetTextForeground(*wxBLACK);
        } else {
            dc.SetTextForeground(*wxWHITE);
        }

        // display model name in red if the model has rendering disabled
        if (rowInfo->element->IsRenderDisabled()) {
            dc.SetTextForeground(wxColour(255, 128, 128));
        }

        if (rowInfo->layerIndex > 0 || rowInfo->strandIndex >= 0) {  // If effect layer = 0
            if (isDark) {
                dc.SetPen(*wxMEDIUM_GREY_PEN);
            } else {
                dc.SetPen(*wxLIGHT_GREY_PEN);
            }
            dc.DrawLine(1,startY,w-1,startY);
            dc.DrawLine(1,startY-1,w-1,startY-1);
            dc.SetPen(*wxBLACK_PEN);
            if (rowInfo->strandIndex >= 0) {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,DEFAULT_ROW_HEADING_HEIGHT);
                wxString name = rowInfo->displayName;
                if (name == "") {
                    if (rowInfo->nodeIndex >= 0) {
                        name = wxString::Format("Node %d", rowInfo->nodeIndex + 1);
                        layers = "";
                    } else {
                        name = wxString::Format("Strand %d", rowInfo->strandIndex + 1);
                    }
                }
                if (rowInfo->nodeIndex >= 0) {
                    dc.DrawLabel(prefix + "     " + name + layers,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                } else if (rowInfo->layerIndex == 0) {
                    dc.DrawLabel(prefix + "  " + name + layers,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
                } else {
                    dc.SetPen(*wxBLUE_PEN);
                    wxString lay = wxString::Format("   [%d]", rowInfo->layerIndex + 1);
                    dc.DrawLabel(lay, r, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
                }
            } else {
                wxRect r(DEFAULT_ROW_HEADING_MARGIN, startY, w - DEFAULT_ROW_HEADING_MARGIN, DEFAULT_ROW_HEADING_HEIGHT);
                dc.SetPen(*wxBLUE_PEN);
                wxString lay = wxString::Format("   [%d]", rowInfo->layerIndex + 1);
                dc.DrawLabel(lay, r, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
            }
            dc.SetPen(penOutline);
            dc.SetBrush(brush2);
        } else {
            // Draw label
            auto name = rowInfo->element->GetName();
            if (rowInfo->element->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                prefix += "  ";

                // find the parent row so we can work out its type
                int toprow = mSequenceElements->GetFirstVisibleModelRow();
                int parent = toprow + rowInfo->RowNumber;
                bool done = false;
                while (!done && parent >= 0)
                {
                    auto maybeParent = mSequenceElements->GetRowInformationFromRow(parent);
                    if (maybeParent != nullptr && maybeParent->element->GetType() == ElementType::ELEMENT_TYPE_MODEL && !maybeParent->submodel)
                    {
                        done = true;
                    }
                    else
                    {
                        parent--;
                    }
                }

                if (done)
                {
                    Model* pm = mSequenceElements->GetXLightsFrame()->AllModels[mSequenceElements->GetRowInformationFromRow(parent)->element->GetModelName()];
                    if (pm != nullptr && pm->GetDisplayAs() == "ModelGroup")
                    {
                        name = rowInfo->element->GetFullName();
                        if (prefix.size() >= 3)
                        {
                            prefix = prefix.substr(3);
                        }
                    }
                }
            }
            wxRect r(DEFAULT_ROW_HEADING_MARGIN,startY,w-DEFAULT_ROW_HEADING_MARGIN,DEFAULT_ROW_HEADING_HEIGHT);
            dc.DrawLabel(prefix + name + layers,r,wxALIGN_CENTER_VERTICAL|wxALIGN_LEFT);
        }

        if (rowInfo->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
            if (rowInfo->element->GetEffectLayerCount() > 1
                && rowInfo->layerIndex == 0
                && rowInfo->nodeIndex == -1) {
                // The +/- to expand/contract the elements layers
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(2,startY + DEFAULT_ROW_HEADING_HEIGHT/2 - 4,9,9);
                dc.DrawLine(2,startY + DEFAULT_ROW_HEADING_HEIGHT/2,9,startY + DEFAULT_ROW_HEADING_HEIGHT/2);
                if (rowInfo->Collapsed) {
                    dc.DrawLine(6,startY + DEFAULT_ROW_HEADING_HEIGHT/2 + 4,6,startY + DEFAULT_ROW_HEADING_HEIGHT/2 - 4);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush2);
            }
            // draw Model Group icon if necessary
            Model* m = mSequenceElements->GetXLightsFrame()->AllModels[rowInfo->element->GetModelName()];
            if (m != nullptr) {
                if (m->GetDisplayAs() == "ModelGroup") {
                    dc.DrawBitmap(model_group_icon.GetBitmapFor(this), getWidth() - ICON_SPACE, startY + 3, true);
                } else if (StartsWith(m->GetStringType(), "Single Color") || m->GetStringType() == "Node Single Color") {
                    if (m->GetNodeCount() > 0) {
                        xlColor color;
                        if (m->GetDisplayAs() == "Channel Block") {
                            StrandElement* se = dynamic_cast<StrandElement*>(rowInfo->element);
                            if (se != nullptr) {
                                color = m->GetNodeMaskColor(se->GetStrand());
                            } else {
                                color = xlWHITE;
                            }
                        } else {
                            color = m->GetNodeMaskColor(0);
                        }
                        dc.SetPen(*wxBLACK_PEN);
                        dc.SetBrush(wxBrush(color.asWxColor()));
                        dc.DrawRectangle(getWidth() - ScaleWithSystemDPI(21), startY + 5, ScaleWithSystemDPI(12), ScaleWithSystemDPI(12));
                        dc.SetPen(penOutline);
                        dc.SetBrush(brush2);
                    }
                }
            }
        } else if (rowInfo->element->GetType()== ElementType::ELEMENT_TYPE_TIMING) {
            if (rowInfo->layerIndex == 0) {
                TimingElement *ti = dynamic_cast<TimingElement*>(rowInfo->element);
                dc.SetPen(*wxBLACK_PEN);
                if (ti->GetActive()) {
                    dc.SetBrush(*wxWHITE_BRUSH);
                    dc.DrawCircle(7,startY + DEFAULT_ROW_HEADING_HEIGHT/2,5);

                    dc.SetBrush(*wxGREY_BRUSH);
                    dc.DrawCircle(7,startY + DEFAULT_ROW_HEADING_HEIGHT/2,2);
                } else {
                    dc.SetBrush(*wxWHITE_BRUSH);
                    dc.DrawCircle(7,startY + DEFAULT_ROW_HEADING_HEIGHT/2,5);
                }
                dc.SetPen(penOutline);
                dc.SetBrush(brush2);
                
                if (ti->GetSubType() == "FPP Commands") {
                    dc.DrawBitmap(fppCommand_icon.GetBitmapFor(this), getWidth() - ICON_SPACE, startY + 3, true);
                } else if (ti->GetSubType() == "FPP Effects") {
                    dc.DrawBitmap(fppEffect_icon.GetBitmapFor(this), getWidth() - ICON_SPACE, startY + 3, true);
                } else if (rowInfo->element->GetEffectLayerCount() == 2) {
                    dc.DrawBitmap(papagayox_icon.GetBitmapFor(this), getWidth() - ICON_SPACE, startY + 3, true);
                } else if (rowInfo->element->GetEffectLayerCount() > 2) {
                    dc.DrawBitmap(papagayo_icon.GetBitmapFor(this), getWidth() - ICON_SPACE, startY + 3, true);
                }
            }
        }
        row++;
    }
    wxBrush b(ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER).asWxColor(),wxBRUSHSTYLE_SOLID);
    dc.SetBrush(b);
    dc.DrawRectangle(0,endY,w,h);
}

xlColor RowHeading::GetHeaderColor(Row_Information_Struct* info, int dragRow) const
{
    if (info->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        return ColorManager::instance()->GetTimingColor(info->colorIndex);
    }
    if (info->RowNumber == mSelectedRow || info->RowNumber == dragRow) {
        return ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER_SELECTED);
    }

    return ColorManager::instance()->GetColor(ColorManager::COLOR_ROW_HEADER);
}

void RowHeading::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void RowHeading::DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row)
{
}

int RowHeading::getWidth() const
{
    return GetSize().x;
}

int RowHeading::getHeight() const
{
    return GetSize().y;
}

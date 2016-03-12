#include "LayoutPanel.h"

//(*InternalHeaders(LayoutPanel)
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/string.h>
#include <wx/splitter.h>
#include <wx/intl.h>
//*)


#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "ModelPreview.h"
#include "xLightsMain.h"
#include "DrawGLUtils.h"
#include "ModelDialog.h"


//(*IdInit(LayoutPanel)
const long LayoutPanel::ID_BUTTON_SELECT_MODEL_GROUPS = wxNewId();
const long LayoutPanel::ID_BUTTON_MODELS_PREVIEW = wxNewId();
const long LayoutPanel::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long LayoutPanel::ID_CHECKBOXOVERLAP = wxNewId();
const long LayoutPanel::ID_LISTBOX_ELEMENT_LIST = wxNewId();
const long LayoutPanel::ID_PANEL2 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW1 = wxNewId();
const long LayoutPanel::ID_PANEL5 = wxNewId();
const long LayoutPanel::ID_PANEL1 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LayoutPanel,wxPanel)
	//(*EventTable(LayoutPanel)
	//*)
END_EVENT_TABLE()


const long LayoutPanel::ID_PREVIEW_ALIGN = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_PROPERTIES = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_TOP = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BOTTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_LEFT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_RIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_H_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_V_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_H_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_V_DISTRIBUTE = wxNewId();


LayoutPanel::LayoutPanel(wxWindow* parent, xLightsFrame *xl) : xlights(xl),
    m_creating_bound_rect(false), mPointSize(2), m_rotating(false), m_dragging(false), m_over_handle(0)
{
    appearanceVisible = sizeVisible = stringPropsVisible = false;

	//(*Initialize(LayoutPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizerPreview;
	wxFlexGridSizer* LeftPanelSizer;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizerPreview->AddGrowableCol(0);
	FlexGridSizerPreview->AddGrowableRow(0);
	SplitterWindow2 = new wxSplitterWindow(this, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
	SplitterWindow2->SetMinimumPaneSize(10);
	SplitterWindow2->SetSashGravity(0.5);
	LeftPanel = new wxPanel(SplitterWindow2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	LeftPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	LeftPanelSizer->AddGrowableCol(0);
	LeftPanelSizer->AddGrowableRow(1);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonSelectModelGroups = new wxButton(LeftPanel, ID_BUTTON_SELECT_MODEL_GROUPS, _("Select Model Groups"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SELECT_MODEL_GROUPS"));
	FlexGridSizer1->Add(ButtonSelectModelGroups, 1, wxALL|wxEXPAND, 5);
	ButtonModelsPreview = new wxButton(LeftPanel, ID_BUTTON_MODELS_PREVIEW, _("Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MODELS_PREVIEW"));
	FlexGridSizer1->Add(ButtonModelsPreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
	ButtonSavePreview = new wxButton(LeftPanel, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
	FlexGridSizer1->Add(ButtonSavePreview, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 5);
	CheckBoxOverlap = new wxCheckBox(LeftPanel, ID_CHECKBOXOVERLAP, _("Overlap checks enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXOVERLAP"));
	CheckBoxOverlap->SetValue(false);
	FlexGridSizer1->Add(CheckBoxOverlap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LeftPanelSizer->Add(FlexGridSizer1, 1, wxALL|wxEXPAND, 5);
	ModelSplitter = new wxSplitterWindow(LeftPanel, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	ModelSplitter->SetMinSize(wxSize(50,50));
	ModelSplitter->SetMinimumPaneSize(50);
	ModelSplitter->SetSashGravity(0.5);
	ListBoxElementList = new wxListView(ModelSplitter, ID_LISTBOX_ELEMENT_LIST, wxPoint(23,181), wxSize(100,-1), wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTBOX_ELEMENT_LIST"));
	SecondPanel = new wxPanel(ModelSplitter, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	ModelSplitter->SplitHorizontally(ListBoxElementList, SecondPanel);
	LeftPanelSizer->Add(ModelSplitter, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	LeftPanel->SetSizer(LeftPanelSizer);
	LeftPanelSizer->Fit(LeftPanel);
	LeftPanelSizer->SetSizeHints(LeftPanel);
	PreviewGLPanel = new wxPanel(SplitterWindow2, ID_PANEL1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL1"));
	PreviewGLSizer = new wxFlexGridSizer(1, 1, 0, 0);
	PreviewGLSizer->AddGrowableCol(0);
	PreviewGLSizer->AddGrowableRow(0);
	PreviewGLPanel->SetSizer(PreviewGLSizer);
	PreviewGLSizer->Fit(PreviewGLPanel);
	PreviewGLSizer->SetSizeHints(PreviewGLPanel);
	SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel);
	SplitterWindow2->SetSashPosition(175);
	FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizerPreview);
	FlexGridSizerPreview->Fit(this);
	FlexGridSizerPreview->SetSizeHints(this);

	Connect(ID_BUTTON_SELECT_MODEL_GROUPS,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonSelectModelGroupsClick);
	Connect(ID_BUTTON_MODELS_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonModelsPreviewClick);
	Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonSavePreviewClick);
	Connect(ID_CHECKBOXOVERLAP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBoxOverlapClick);
	Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnListBoxElementListItemSelect);
	Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&LayoutPanel::OnListBoxElementListColumnClick);
	Connect(ID_SPLITTERWINDOW1,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnModelSplitterSashPosChanged);
	Connect(ID_SPLITTERWINDOW2,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnSplitterWindowSashPosChanged);
	//*)

    modelPreview = new ModelPreview( (wxPanel*) PreviewGLPanel, xlights->PreviewModels, true);
    PreviewGLSizer->Add(modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewGLSizer->Fit(PreviewGLPanel);
    PreviewGLSizer->SetSizeHints(PreviewGLPanel);
    FlexGridSizerPreview->Fit(this);
    FlexGridSizerPreview->SetSizeHints(this);

    modelPreview->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftDown,0,this);
    modelPreview->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&LayoutPanel::OnPreviewLeftUp,0,this);
    modelPreview->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnPreviewRightDown,0,this);
    modelPreview->Connect(wxEVT_MOTION,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseMove,0,this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&LayoutPanel::OnPreviewMouseLeave,0,this);

    propertyEditor = new wxPropertyGrid(ModelSplitter,
                                        wxID_ANY, // id
                                        wxDefaultPosition, // position
                                        wxDefaultSize, // size
                                        // Here are just some of the supported window styles
                                        //wxPG_AUTO_SORT | // Automatic sorting after items added
                                        wxPG_SPLITTER_AUTO_CENTER | // Automatically center splitter until user manually adjusts it
                                        // Default style
                                        wxPG_DEFAULT_STYLE );
#ifdef __WXOSX__
    propertyEditor->SetExtraStyle(wxPG_EX_NATIVE_DOUBLE_BUFFERING);
#endif
    ModelSplitter->ReplaceWindow(SecondPanel, propertyEditor);
    LeftPanelSizer->Fit(LeftPanel);
    LeftPanelSizer->SetSizeHints(LeftPanel);

    //LeftPanelSizer->Add(propertyEditor, 1, wxALL|wxEXPAND, 0);
    //LeftPanelSizer->AddGrowableRow(13);
    propertyEditor->Connect(wxEVT_PG_CHANGING, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChanging,0,this);
    propertyEditor->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&LayoutPanel::OnPropertyGridChange,0,this);
    propertyEditor->SetValidationFailureBehavior(wxPG_VFB_MARK_CELL | wxPG_VFB_BEEP);
    
    wxConfigBase* config = wxConfigBase::Get();
    int msp = config->Read("LayoutModelSplitterSash", -1);
    int sp = config->Read("LayoutMainSplitterSash", -1);
    if (sp != -1) {
        SplitterWindow2->SetSashGravity(0.0);
        SplitterWindow2->SetSashPosition(sp);
    }
    if (msp != -1) {
        ModelSplitter->SetSashGravity(0.0);
        ModelSplitter->SetSashPosition(msp);
    }

}

LayoutPanel::~LayoutPanel()
{
	//(*Destroy(LayoutPanel)
	//*)
}

void LayoutPanel::OnPropertyGridChange(wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    if (name == "BkgBrightness") {
        xlights->SetPreviewBackgroundBrightness(event.GetValue().GetLong());
    } else if (name == "BkgSizeWidth") {
        xlights->SetPreviewSize(event.GetValue().GetLong(), modelPreview->GetVirtualCanvasHeight());
    } else if (name == "BkgSizeHeight") {
        xlights->SetPreviewSize(modelPreview->GetVirtualCanvasWidth(), event.GetValue().GetLong());
    } else if (name == "BkgImage") {
        xlights->SetPreviewBackgroundImage(event.GetValue().GetString());
    } else if (name == "BkgFill") {
        xlights->SetPreviewBackgroundScaled(event.GetValue().GetBool());
    } else if (selectedModel != nullptr) {
        //model property
        if ("ModelRotation" == name) {
            PreviewRotationUpdated(event.GetValue().GetInteger());
        } else if ("ModelWidth" == name) {
            double newscalex, newscaley;
            selectedModel->GetScales(newscalex, newscaley);
            newscalex *= 100.0;
            newscaley *= 100.0;

            PreviewScaleUpdated(event.GetValue().GetDouble(), newscaley);
        } else if ("ModelHeight" == name) {
            double newscalex, newscaley;
            selectedModel->GetScales(newscalex, newscaley);
            newscalex *= 100.0;
            newscaley *= 100.0;
            PreviewScaleUpdated(newscalex, event.GetValue().GetDouble());
        } else if ("ModelX" == name) {
            selectedModel->SetHcenterOffset(event.GetValue().GetDouble() / 100.0f);
            selectedModel->UpdateXmlWithScale();
            xlights->UpdatePreview();
            xlights->UnsavedRgbEffectsChanges = true;
        } else if ("ModelY" == name) {
            selectedModel->SetVcenterOffset(event.GetValue().GetDouble() / 100.0f);
            selectedModel->UpdateXmlWithScale();
            xlights->UpdatePreview();
            xlights->UnsavedRgbEffectsChanges = true;
        } else if ("ModelName" == name) {
            if (selectedModel->name != event.GetValue().GetString().ToStdString()) {
                xlights->RenameModelInViews(selectedModel->name, event.GetValue().GetString().ToStdString());
            }
        } else {
            int i = selectedModel->OnPropertyGridChange(propertyEditor, event);
            if (i & 0x0001) {
                xlights->UpdatePreview();
            }
            if (i & 0x0002) {
                xlights->UnsavedRgbEffectsChanges = true;
            }
            if (i & 0x0004) {
                clearPropGrid();
                SetupPropGrid(selectedModel);
            }
            if (i == 0) {
                printf("Did not handle %s   %s\n",
                       event.GetPropertyName().ToStdString().c_str(),
                       event.GetValue().GetString().ToStdString().c_str());
            }
        }
    }
}
void LayoutPanel::OnPropertyGridChanging(wxPropertyGridEvent& event) {
    if (selectedModel != nullptr) {
        if ("ModelName" == event.GetPropertyName()) {
            if (xlights->AllModels[event.GetValue().GetString().ToStdString()] != nullptr) {
                event.Veto();
            }
        } else {
            selectedModel->OnPropertyGridChanging(propertyEditor, event);
        }
    }
    /*
    printf("Changing %s   %s\n",
           event.GetPropertyName().ToStdString().c_str(),
           event.GetValue().GetString().ToStdString().c_str());
     */
}

void LayoutPanel::PreviewScaleUpdated(float xscale, float yscale)
{
    int sel=ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);
    m->SetScale(xscale/100.0, yscale/100.0);
    m->UpdateXmlWithScale();
    xlights->UnsavedRgbEffectsChanges = true;
    UpdatePreview();
}

void LayoutPanel::PreviewRotationUpdated(int newRotation)
{
    int sel=ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);
    m->SetModelCoord(newRotation);
    m->UpdateXmlWithScale();
    xlights->UnsavedRgbEffectsChanges = true;
    UpdatePreview();
}

void LayoutPanel::UpdatePreview()
{
    if(!modelPreview->StartDrawing(mPointSize)) return;
    if(m_creating_bound_rect)
    {
        DrawGLUtils::DrawRectangle(xlYELLOW,true,m_bound_start_x,m_bound_start_y,m_bound_end_x,m_bound_end_y);
    }
    modelPreview->Render();
    modelPreview->EndDrawing();
}


void LayoutPanel::clearPropGrid() {
    wxPGProperty *p = propertyEditor->GetPropertyByName("ModelAppearance");
    if (p != nullptr) {
        appearanceVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelSize");
    if (p != nullptr) {
        sizeVisible = propertyEditor->IsPropertyExpanded(p);
    }
    p = propertyEditor->GetPropertyByName("ModelStringProperties");
    if (p != nullptr) {
        stringPropsVisible = propertyEditor->IsPropertyExpanded(p);
    }
    propertyEditor->Clear();
}

void LayoutPanel::UnSelectAllModels()
{
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        xlights->PreviewModels[i]->Selected = false;
        xlights->PreviewModels[i]->GroupSelected = false;
    }
    UpdatePreview();

    propertyEditor->Freeze();
    clearPropGrid();
    propertyEditor->Append(new wxImageFileProperty("Background Image",
                                                   "BkgImage",
                                                   modelPreview->GetBackgroundImage()));
    propertyEditor->Append(new wxBoolProperty("Fill", "BkgFill", modelPreview->GetScaleBackgroundImage()))->SetAttribute("UseCheckbox", 1);
    wxPGProperty* prop = propertyEditor->Append(new wxUIntProperty("Width", "BkgSizeWidth", modelPreview->GetVirtualCanvasWidth()));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 4096);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxUIntProperty("Height", "BkgSizeHeight", modelPreview->GetVirtualCanvasHeight()));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 4096);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxStringProperty("Brightness",
                                                       "BkgBrightness",
                                                       wxString::Format("%d", modelPreview->GetBackgroundBrightness())));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");
    propertyEditor->Thaw();
}

void LayoutPanel::SetupPropGrid(Model *model) {
    double newscalex, newscaley;
    model->GetScales(newscalex, newscaley);
    newscalex *= 100.0;
    newscaley *= 100.0;

    propertyEditor->Freeze();
    clearPropGrid();

    wxPGProperty* prop = propertyEditor->Append(new wxStringProperty("Name", "ModelName", model->name));

    model->AddProperties(propertyEditor);

    wxPGProperty *p2 = propertyEditor->Append(new wxPropertyCategory("Size/Location", "ModelSize"));

    prop = propertyEditor->Append(new wxFloatProperty("X (%)", "ModelX", model->GetHcenterOffset() * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Y (%)", "ModelY", model->GetVcenterOffset() * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Width", "ModelWidth", newscalex));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Height", "ModelHeight", newscaley));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxIntProperty("Rotation", "ModelRotation", model->GetRotation()));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetEditor("SpinCtrl");
    if (!sizeVisible) {
        propertyEditor->Collapse(p2);
    }
    if (!appearanceVisible) {
        prop = propertyEditor->GetPropertyByName("ModelAppearance");
        if (prop != nullptr) {
            propertyEditor->Collapse(prop);
        }
    }
    if (!stringPropsVisible) {
        prop = propertyEditor->GetPropertyByName("ModelStringProperties");
        if (prop != nullptr) {
            propertyEditor->Collapse(prop);
        }
    }
    propertyEditor->Thaw();
}

void LayoutPanel::SelectModel(const std::string & name)
{

    int foundStart = 0;
    int foundEnd = 0;
    Model* m = nullptr;
    for(int i=0;i<ListBoxElementList->GetItemCount();i++)
    {
        if (name == ListBoxElementList->GetItemText(i))
        {
            if (ListBoxElementList->GetItemState(i, wxLIST_STATE_SELECTED) == false) {
                ListBoxElementList->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
                ListBoxElementList->EnsureVisible(i);
                return;
            }
            m=(Model*)ListBoxElementList->GetItemData(i);
            m->Selected = true;
            if (CheckBoxOverlap->GetValue()== true) {
                foundStart = m->GetNumberFromChannelString(m->ModelStartChannel);
                foundEnd = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(i,2).ToStdString());
            }
            SetupPropGrid(m);
            break;
        }
    }
    if (m == nullptr) {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();
    }
    selectedModel = m;
    if (CheckBoxOverlap->GetValue()) {
        for(int i=0;i<ListBoxElementList->GetItemCount();i++)
        {
            if (name != ListBoxElementList->GetItemText(i)) {
                Model* m=(Model*)ListBoxElementList->GetItemData(i);
                if (m != NULL) {
                    int startChan = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(i,1).ToStdString());
                    int endChan = m->GetNumberFromChannelString(ListBoxElementList->GetItemText(i,2).ToStdString());
                    if ((startChan >= foundStart) && (endChan <= foundEnd)) {
                        m->Overlapping = true;
                    } else if ((startChan >= foundStart) && (startChan <= foundEnd)) {
                        m->Overlapping = true;
                    } else if ((endChan >= foundStart) && (endChan <= foundEnd)) {
                        m->Overlapping = true;
                    } else {
                        m->Overlapping = false;
                    }
                }
            }
        }
    }
    UpdatePreview();
}


void LayoutPanel::OnCheckBoxOverlapClick(wxCommandEvent& event)
{
    if (CheckBoxOverlap->GetValue() == false) {
        for(int i=0;i<ListBoxElementList->GetItemCount();i++)
        {
            Model* m=(Model*)ListBoxElementList->GetItemData(i);
            if (m != NULL) {
                m->Overlapping = false;
            }
        }
    }
}

void LayoutPanel::OnButtonModelsPreviewClick(wxCommandEvent& event)
{
    // update xml with offsets and scale
    for (size_t i=0; i < xlights->PreviewModels.size(); i++)
    {
        xlights->PreviewModels[i]->UpdateXmlWithScale();
    }
    xlights->ShowModelsDialog();
    UpdatePreview();
}

void LayoutPanel::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    // update xml with offsets and scale
    for (size_t i=0; i < xlights->PreviewModels.size(); i++)
    {
        xlights->PreviewModels[i]->UpdateXmlWithScale();
    }
    xlights->SaveEffectsFile();
    xlights->SetStatusText(_("Preview layout saved"));
}


void LayoutPanel::OnButtonSelectModelGroupsClick(wxCommandEvent& event)
{
    CurrentPreviewModels dialog(this,xlights->ModelGroupsNode,xlights->ModelsNode);
    dialog.ShowModal();

    for (wxXmlNode *node = xlights->ModelGroupsNode->GetChildren(); node != nullptr; node = node->GetNext()) {
        wxString oldName = node->GetAttribute("oldName", "");
        node->DeleteAttribute("oldName");
        if (oldName != "") {
            Element* elem_to_rename = xlights->GetSequenceElements().GetElement(oldName.ToStdString());
            if( elem_to_rename != NULL ) {
                elem_to_rename->SetName(node->GetAttribute("name").ToStdString());
            }
        }
    }

    xlights->UnsavedRgbEffectsChanges=true;
    xlights->UpdateModelsList();
    UpdatePreview();
    wxCommandEvent eventRowHeaderChanged(EVT_ROW_HEADINGS_CHANGED);
    xlights->RowHeadingsChanged(eventRowHeaderChanged);
}

void LayoutPanel::OnListBoxElementListItemSelect(wxListEvent& event)
{
    UnSelectAllModels();
    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND)
    {
        ListBoxElementList->SetToolTip("");
        return;
    }
    std::string name = ListBoxElementList->GetItemText(sel).ToStdString();
    SelectModel(name);

    for (int i = 0; i < ListBoxElementList->GetItemCount(); i++)
    {
        if (name == ListBoxElementList->GetItemText(i))
        {
            Model* m = (Model*)ListBoxElementList->GetItemData(i);
            ListBoxElementList->SetToolTip(xlights->GetChannelToControllerMapping(wxAtol(m->ModelStartChannel)));
        }
    }
}

int wxCALLBACK SortElementsFunctionASC(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortColumn)
{
    Model* a = (Model *)item1;
    Model* b = (Model *)item2;

    if (sortColumn == 1) {
        int ia = a->GetNumberFromChannelString(a->ModelStartChannel);
        int ib = b->GetNumberFromChannelString(b->ModelStartChannel);
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return 0;
    } else if (sortColumn == 2) {
        int ia = a->GetLastChannel();
        int ib = b->GetLastChannel();
        if (ia > ib)
            return 1;
        if (ia < ib)
            return -1;
        return 0;
    } else {
        return strcasecmp(a->name.c_str(), b->name.c_str());
    }
    return 0;
}

int wxCALLBACK SortElementsFunctionDESC(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortColumn)
{
    return SortElementsFunctionASC(item2, item1, sortColumn);
}
void LayoutPanel::OnListBoxElementListColumnClick(wxListEvent& event)
{
    int col = event.GetColumn();
    static bool x = false;
    x = !x;
    x ? ListBoxElementList->SortItems(SortElementsFunctionASC,col):ListBoxElementList->SortItems(SortElementsFunctionDESC,col);
}

int LayoutPanel::FindModelsClicked(int x,int y,std::vector<int> &found)
{
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->HitTest(modelPreview,x,y))
        {
            found.push_back(i);
        }
    }
    return found.size();
}



bool LayoutPanel::SelectSingleModel(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x,y,found);
    if (modelCount==0)
    {
        return false;
    }
    else if(modelCount==1)
    {
        SelectModel(xlights->PreviewModels[found[0]]->name);
        mHitTestNextSelectModelIndex = 0;
        return true;
    }
    else if (modelCount>1)
    {
        for(int i=0;i<modelCount;i++)
        {
            if(mHitTestNextSelectModelIndex==i)
            {
                SelectModel(xlights->PreviewModels[found[i]]->name);
                mHitTestNextSelectModelIndex += 1;
                mHitTestNextSelectModelIndex %= modelCount;
                return true;
            }
        }
    }
    return false;
}
void LayoutPanel::SelectAllInBoundingRect()
{
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->IsContained(modelPreview,m_bound_start_x,m_bound_start_y,
                                         m_bound_end_x,m_bound_end_y))
        {
            xlights->PreviewModels[i]->GroupSelected = true;
        }
    }
}

bool LayoutPanel::SelectMultipleModels(int x,int y)
{
    std::vector<int> found;
    int modelCount = FindModelsClicked(x,y,found);
    if (modelCount==0)
    {
        return false;
    }
    else if(modelCount>0)
    {
        propertyEditor->Freeze();
        clearPropGrid();
        propertyEditor->Thaw();

        if(xlights->PreviewModels[found[0]]->Selected)
        {
            xlights->PreviewModels[found[0]]->Selected = false;
            xlights->PreviewModels[found[0]]->GroupSelected = false;
        }
        else if (xlights->PreviewModels[found[0]]->GroupSelected)
        {
            SetSelectedModelToGroupSelected();
            xlights->PreviewModels[found[0]]->Selected = true;
            SelectModel(xlights->PreviewModels[found[0]]->name);
        }
        else
        {
            xlights->PreviewModels[found[0]]->GroupSelected = true;
        }
        UpdatePreview();
        return true;
    }
    return false;
}

void LayoutPanel::SetSelectedModelToGroupSelected()
{
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->Selected)
        {
            xlights->PreviewModels[i]->Selected = false;
            xlights->PreviewModels[i]->GroupSelected = true;
        }
    }
}


void LayoutPanel::OnPreviewLeftDown(wxMouseEvent& event)
{
    int y = event.GetY();
    if (event.ControlDown())
    {
        SelectMultipleModels(event.GetX(),y);
        m_dragging = true;
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
    }
    else if (event.ShiftDown())
    {
        m_creating_bound_rect = true;
        m_bound_start_x = event.GetX();
        m_bound_start_y = modelPreview->GetVirtualCanvasHeight() - y;
    }
    else if (m_over_handle == OVER_ROTATE_HANDLE)
    {
        m_rotating = true;
    }
    else if (m_over_handle != OVER_NO_HANDLE)
    {
        m_resizing = true;
    }
    else
    {
        m_rotating = false;
        m_resizing = false;
        m_creating_bound_rect = false;

        if(!event.wxKeyboardState::ControlDown())
        {
            UnSelectAllModels();
        }

        if(SelectSingleModel(event.GetX(),y))
        {
            m_dragging = true;
            m_previous_mouse_x = event.GetX();
            m_previous_mouse_y = event.GetY();
            xlights->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        }
    }
}

void LayoutPanel::OnPreviewLeftUp(wxMouseEvent& event)
{
    int y = event.GetY();

    m_rotating = false;
    m_dragging = false;
    m_resizing = false;
    if(m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
        SelectAllInBoundingRect();
        m_creating_bound_rect = false;
        UpdatePreview();
    }
}


void LayoutPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    m_dragging = false;
}

void LayoutPanel::OnPreviewMouseMove(wxMouseEvent& event)
{
    int y = event.GetY();
    int wi,ht;

    if (m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
        UpdatePreview();
        return;
    }

    int sel=ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);

    if(m_rotating)
    {
        m->RotateWithHandles(modelPreview,event.ShiftDown(), event.GetPosition().x,y);
        m->UpdateXmlWithScale();
        SetupPropGrid(m);
        xlights->UnsavedRgbEffectsChanges = true;
        UpdatePreview();
    }
    else if(m_resizing)
    {
        m->ResizeWithHandles(modelPreview,event.GetPosition().x,y);
        double scalex, scaley;
        m->GetScales(scalex, scaley);
        m->UpdateXmlWithScale();
        xlights->UnsavedRgbEffectsChanges = true;
        SetupPropGrid(m);
        UpdatePreview();
    }
    else if (m_dragging && event.Dragging())
    {
        double delta_x = event.GetPosition().x - m_previous_mouse_x;
        double delta_y = -(event.GetPosition().y - m_previous_mouse_y);
        modelPreview->GetSize(&wi,&ht);
        if (wi > 0 && ht > 0)
        {
            for (int i=0; i<xlights->PreviewModels.size(); i++)
            {
                if(xlights->PreviewModels[i]->Selected || xlights->PreviewModels[i]->GroupSelected)
                {
                    xlights->PreviewModels[i]->AddOffset(delta_x/wi, delta_y/ht);
                    xlights->PreviewModels[i]->UpdateXmlWithScale();
                    SetupPropGrid(xlights->PreviewModels[i]);
                    xlights->UnsavedRgbEffectsChanges = true;
                }
            }
        }
        m_previous_mouse_x = event.GetPosition().x;
        m_previous_mouse_y = event.GetPosition().y;
        xlights->SetStatusText(wxString::Format("x=%d y=%d",m_previous_mouse_x,m_previous_mouse_y));
        UpdatePreview();
    }
    else
    {
        if(m->Selected)
        {
            m_over_handle = m->CheckIfOverHandles(modelPreview,event.GetPosition().x,modelPreview->GetVirtualCanvasHeight() - y);
        }
    }
}

void LayoutPanel::OnPreviewRightDown(wxMouseEvent& event)
{

    wxMenu mnu;
    wxMenu *mnuAlign;
    wxMenu *mnuDistribute;
    int selectedModelCnt = ModelsSelectedCount();
    if (selectedModelCnt > 1)
    {
        mnuAlign = new wxMenu();
        mnuAlign->Append(ID_PREVIEW_ALIGN_TOP,"Top");
        mnuAlign->Append(ID_PREVIEW_ALIGN_BOTTOM,"Bottom");
        mnuAlign->Append(ID_PREVIEW_ALIGN_LEFT,"Left");
        mnuAlign->Append(ID_PREVIEW_ALIGN_RIGHT,"Right");
        mnuAlign->Append(ID_PREVIEW_ALIGN_H_CENTER,"Horizontal Center");
        mnuAlign->Append(ID_PREVIEW_ALIGN_V_CENTER,"Vertical Center");
        mnuAlign->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);

        mnuDistribute = new wxMenu();
        mnuDistribute->Append(ID_PREVIEW_H_DISTRIBUTE,"Horizontal");
        mnuDistribute->Append(ID_PREVIEW_V_DISTRIBUTE,"Vertical");
        mnuDistribute->Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);

        mnu.Append(ID_PREVIEW_ALIGN, 	        "Align", mnuAlign,"");
        mnu.Append(ID_PREVIEW_DISTRIBUTE,"Distribute", mnuDistribute,"");
        mnu.AppendSeparator();
    }
    else if (selectedModelCnt == 0)
    {
        return;
    }
    mnu.Append(ID_PREVIEW_MODEL_PROPERTIES,"Model Properties");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);
    PopupMenu(&mnu);
}


void LayoutPanel::OnPreviewModelPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_PREVIEW_ALIGN_TOP)
    {
        PreviewModelAlignTops();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_BOTTOM)
    {
        PreviewModelAlignBottoms();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_LEFT)
    {
        PreviewModelAlignLeft();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_RIGHT)
    {
        PreviewModelAlignRight();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_H_CENTER)
    {
        PreviewModelAlignHCenter();
    }
    else if (event.GetId() == ID_PREVIEW_ALIGN_V_CENTER)
    {
        PreviewModelAlignVCenter();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_PROPERTIES)
    {
        ShowModelProperties();
    }

}

void LayoutPanel::ShowModelProperties()
{
    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* m=(Model*)ListBoxElementList->GetItemData(sel);

    wxXmlNode* e=m->GetModelXml();
    int DlgResult;
    bool ok;
    ModelDialog *dialog = new ModelDialog(this, xlights);
    dialog->SetFromXml(e, &xlights->GetNetInfo());
    dialog->TextCtrl_Name->Enable(false); // do not allow name changes; -why? -DJ
    do
    {
        ok=true;
        DlgResult=dialog->ShowModal();
        if (DlgResult == wxID_OK)
        {
            // validate inputs
            if (ok)
            {
                dialog->UpdateXml(e);
                xlights->UnsavedRgbEffectsChanges=true;
                xlights->UpdateModelsList();

            }
        }
    }
    while (DlgResult == wxID_OK && !ok);
    UpdatePreview();
    delete dialog;
}

void LayoutPanel::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int top = xlights->PreviewModels[selectedindex]->GetTop(modelPreview);
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->GroupSelected)
        {
            xlights->PreviewModels[i]->SetTop(modelPreview,top);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int bottom = xlights->PreviewModels[selectedindex]->GetBottom(modelPreview);
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->GroupSelected)
        {
            xlights->PreviewModels[i]->SetBottom(modelPreview,bottom);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int left = xlights->PreviewModels[selectedindex]->GetLeft(modelPreview);
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->GroupSelected)
        {
            xlights->PreviewModels[i]->SetLeft(modelPreview,left);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    int right = xlights->PreviewModels[selectedindex]->GetRight(modelPreview);
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->GroupSelected)
        {
            xlights->PreviewModels[i]->SetRight(modelPreview,right);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    float center = xlights->PreviewModels[selectedindex]->GetHcenterOffset();
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->GroupSelected)
        {
            xlights->PreviewModels[i]->SetHcenterOffset(center);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    float center = xlights->PreviewModels[selectedindex]->GetVcenterOffset();
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->GroupSelected)
        {
            xlights->PreviewModels[i]->SetVcenterOffset(center);
        }
    }
    UpdatePreview();
}


int LayoutPanel::GetSelectedModelIndex()
{
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->Selected)
        {
            return i;
        }
    }
    return -1;
}
int LayoutPanel::ModelsSelectedCount()
{
    int selectedModelCount=0;
    for (int i=0; i<xlights->PreviewModels.size(); i++)
    {
        if(xlights->PreviewModels[i]->Selected || xlights->PreviewModels[i]->GroupSelected)
        {
            selectedModelCount++;
        }
    }
    return selectedModelCount;
}





void LayoutPanel::OnModelSplitterSashPosChanged(wxSplitterEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutModelSplitterSash", event.GetSashPosition());
    printf("%d\n", event.GetSashPosition());
}

void LayoutPanel::OnSplitterWindowSashPosChanged(wxSplitterEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutMainSplitterSash", event.GetSashPosition());
}

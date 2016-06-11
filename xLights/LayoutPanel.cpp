#include "LayoutPanel.h"

//(*InternalHeaders(LayoutPanel)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/splitter.h>
#include <wx/font.h>
#include <wx/choice.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include <wx/clipbrd.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/tglbtn.h>
#include <wx/sstream.h>

#include "ModelPreview.h"
#include "xLightsMain.h"
#include "DrawGLUtils.h"
#include "ChannelLayoutDialog.h"
#include "models/ModelGroup.h"
#include "ModelGroupPanel.h"

#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"

#include "models/ModelImages.h"
#include "PreviewPane.h"


//(*IdInit(LayoutPanel)
const long LayoutPanel::ID_CHECKLISTBOX_MODEL_GROUPS = wxNewId();
const long LayoutPanel::ID_LISTBOX_ELEMENT_LIST = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW3 = wxNewId();
const long LayoutPanel::ID_PANEL2 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW1 = wxNewId();
const long LayoutPanel::ID_CHECKBOXOVERLAP = wxNewId();
const long LayoutPanel::ID_BUTTON_SAVE_PREVIEW = wxNewId();
const long LayoutPanel::ID_PANEL5 = wxNewId();
const long LayoutPanel::ID_STATICTEXT1 = wxNewId();
const long LayoutPanel::ID_CHOICE_PREVIEWS = wxNewId();
const long LayoutPanel::ID_PANEL1 = wxNewId();
const long LayoutPanel::ID_SPLITTERWINDOW2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LayoutPanel,wxPanel)
	//(*EventTable(LayoutPanel)
	//*)
    EVT_COMMAND(wxID_ANY, EVT_LISTITEM_CHECKED, LayoutPanel::ModelGroupChecked)
END_EVENT_TABLE()


const long LayoutPanel::ID_PREVIEW_ALIGN = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_NODELAYOUT = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTCSV = wxNewId();
const long LayoutPanel::ID_PREVIEW_MODEL_EXPORTCUSTOMMODEL = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_TOP = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_BOTTOM = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_LEFT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_RIGHT = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_H_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_ALIGN_V_CENTER = wxNewId();
const long LayoutPanel::ID_PREVIEW_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_H_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_PREVIEW_V_DISTRIBUTE = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL = wxNewId();
const long LayoutPanel::ID_MNU_DELETE_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_RENAME_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_MNU_ADD_MODEL_GROUP = wxNewId();
const long LayoutPanel::ID_PREVIEW_DELETE_ACTIVE = wxNewId();

class NewModelBitmapButton : public wxBitmapButton
{
public:

    NewModelBitmapButton(wxWindow *parent, const wxBitmap &bmp, const std::string &type)
        : wxBitmapButton(parent, wxID_ANY, bmp), bitmap(bmp), state(0), modelType(type) {
        SetToolTip("Create new " + type);
    }
    virtual ~NewModelBitmapButton() {}

    void SetState(unsigned int s) {
        if (s > 2) {
            s = 0;
        }
        state = s;
        if (state == 2) {
            SetBitmap(bitmap.ConvertToDisabled());
        } else if (state == 1) {
            const wxImage imgDisabled = bitmap.ConvertToImage().ConvertToDisabled(128);
#ifdef __WXOSX__
            SetBitmap(wxBitmap(imgDisabled, -1, bitmap.GetScaleFactor()));
#else
            SetBitmap(wxBitmap(imgDisabled));
#endif
        } else {
            SetBitmap(bitmap);
        }
    }
    unsigned int GetState() {
        return state;
    }
    const std::string &GetModelType() {
        return modelType;
    }
protected:
private:
    const std::string modelType;
    unsigned int state;
    wxBitmap bitmap;
};


#include <log4cpp/Category.hh>

LayoutPanel::LayoutPanel(wxWindow* parent, xLightsFrame *xl, wxPanel* sequencer) : xlights(xl), main_sequencer(sequencer),
    m_creating_bound_rect(false), mPointSize(2), m_moving_handle(false), m_dragging(false),
    m_over_handle(-1), selectedButton(nullptr), newModel(nullptr), selectedModel(nullptr),
    colSizesSet(false), updatingProperty(false), mNumGroups(0), mPropGridActive(true),
    mSelectedGroup(-1), currentLayoutGroup("Default"), backgroundFile("")
{
    background = nullptr;

    _lastCustomModel = "";
    appearanceVisible = sizeVisible = stringPropsVisible = false;

	//(*Initialize(LayoutPanel)
	wxFlexGridSizer* LeftPanelSizer;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* PreviewGLSizer;
	wxFlexGridSizer* FlexGridSizerPreview;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizerPreview = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizerPreview->AddGrowableCol(0);
	FlexGridSizerPreview->AddGrowableRow(0);
	SplitterWindow2 = new wxSplitterWindow(this, ID_SPLITTERWINDOW2, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW2"));
	SplitterWindow2->SetMinSize(wxSize(10,10));
	SplitterWindow2->SetMinimumPaneSize(10);
	SplitterWindow2->SetSashGravity(0.5);
	LeftPanel = new wxPanel(SplitterWindow2, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	LeftPanelSizer = new wxFlexGridSizer(0, 1, 0, 0);
	LeftPanelSizer->AddGrowableCol(0);
	LeftPanelSizer->AddGrowableRow(0);
	ModelSplitter = new wxSplitterWindow(LeftPanel, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	ModelSplitter->SetMinSize(wxSize(100,100));
	ModelSplitter->SetMinimumPaneSize(100);
	ModelSplitter->SetSashGravity(0.5);
	GroupSplitter = new wxSplitterWindow(ModelSplitter, ID_SPLITTERWINDOW3, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW3"));
	GroupSplitter->SetMinSize(wxSize(50,50));
	GroupSplitter->SetMinimumPaneSize(50);
	GroupSplitter->SetSashGravity(0.5);
	ListBoxModelGroups = new wxCheckedListCtrl(GroupSplitter, ID_CHECKLISTBOX_MODEL_GROUPS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_CHECKLISTBOX_MODEL_GROUPS"));
	ListBoxElementList = new wxListView(GroupSplitter, ID_LISTBOX_ELEMENT_LIST, wxPoint(50,7), wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING, wxDefaultValidator, _T("ID_LISTBOX_ELEMENT_LIST"));
	GroupSplitter->SplitHorizontally(ListBoxModelGroups, ListBoxElementList);
	SecondPanel = new wxPanel(ModelSplitter, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	ModelSplitter->SplitHorizontally(GroupSplitter, SecondPanel);
	LeftPanelSizer->Add(ModelSplitter, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 2);
	CheckBoxOverlap = new wxCheckBox(LeftPanel, ID_CHECKBOXOVERLAP, _("Overlap checks enabled"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOXOVERLAP"));
	CheckBoxOverlap->SetValue(false);
	LeftPanelSizer->Add(CheckBoxOverlap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonSavePreview = new wxButton(LeftPanel, ID_BUTTON_SAVE_PREVIEW, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SAVE_PREVIEW"));
	FlexGridSizer2->Add(ButtonSavePreview, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LeftPanelSizer->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	LeftPanel->SetSizer(LeftPanelSizer);
	LeftPanelSizer->Fit(LeftPanel);
	LeftPanelSizer->SetSizeHints(LeftPanel);
	PreviewGLPanel = new wxPanel(SplitterWindow2, ID_PANEL1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL1"));
	PreviewGLSizer = new wxFlexGridSizer(2, 1, 0, 0);
	PreviewGLSizer->AddGrowableCol(0);
	PreviewGLSizer->AddGrowableRow(1);
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	ToolSizer = new wxFlexGridSizer(0, 10, 0, 0);
	FlexGridSizer1->Add(ToolSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 3);
	StaticText1 = new wxStaticText(PreviewGLPanel, ID_STATICTEXT1, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText1Font(12,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxLEFT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 40);
	ChoiceLayoutGroups = new wxChoice(PreviewGLPanel, ID_CHOICE_PREVIEWS, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PREVIEWS"));
	FlexGridSizer1->Add(ChoiceLayoutGroups, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PreviewGLSizer->Add(FlexGridSizer1, 1, wxALL|wxALIGN_LEFT, 3);
	PreviewGLPanel->SetSizer(PreviewGLSizer);
	PreviewGLSizer->Fit(PreviewGLPanel);
	PreviewGLSizer->SetSizeHints(PreviewGLPanel);
	SplitterWindow2->SplitVertically(LeftPanel, PreviewGLPanel);
	SplitterWindow2->SetSashPosition(175);
	FlexGridSizerPreview->Add(SplitterWindow2, 1, wxALL|wxEXPAND, 1);
	SetSizer(FlexGridSizerPreview);
	FlexGridSizerPreview->Fit(this);
	FlexGridSizerPreview->SetSizeHints(this);

	Connect(ID_CHECKLISTBOX_MODEL_GROUPS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnListBoxModelGroupsItemSelect);
	Connect(ID_CHECKLISTBOX_MODEL_GROUPS,wxEVT_COMMAND_LIST_ITEM_DESELECTED,(wxObjectEventFunction)&LayoutPanel::OnListBoxModelGroupsItemDeselect);
	Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnListBoxElementListItemSelect);
	Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&LayoutPanel::OnListBoxElementListItemRClick);
	Connect(ID_LISTBOX_ELEMENT_LIST,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&LayoutPanel::OnListBoxElementListColumnClick);
	Connect(ID_SPLITTERWINDOW3,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnGroupSplitterSashPosChanged);
	Connect(ID_SPLITTERWINDOW1,wxEVT_COMMAND_SPLITTER_SASH_POS_CHANGED,(wxObjectEventFunction)&LayoutPanel::OnModelSplitterSashPosChanged);
	Connect(ID_CHECKBOXOVERLAP,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnCheckBoxOverlapClick);
	Connect(ID_BUTTON_SAVE_PREVIEW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayoutPanel::OnButtonSavePreviewClick);
	Connect(ID_CHOICE_PREVIEWS,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&LayoutPanel::OnChoiceLayoutGroupsSelect);
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
    propertyEditor->SetExtraStyle(wxPG_EX_NATIVE_DOUBLE_BUFFERING | wxWS_EX_PROCESS_IDLE);
#else
    propertyEditor->SetExtraStyle(wxWS_EX_PROCESS_IDLE);
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
    int gsp = config->Read("LayoutModelGroupSplitterSash", -1);
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
    if (gsp != -1) {
        GroupSplitter->SetSashGravity(0.0);
        GroupSplitter->SetSashPosition(gsp);
    }

    currentLayoutGroup = config->Read("CurrentLayoutGroup", "Default");

    wxListItem elementCol;
    elementCol.SetText(_T("Element Name"));
    elementCol.SetImage(-1);
    elementCol.SetAlign(wxLIST_FORMAT_LEFT);
    ListBoxElementList->InsertColumn(0, elementCol);

    elementCol.SetText(_T("Start Chan"));
    elementCol.SetAlign(wxLIST_FORMAT_LEFT);
    ListBoxElementList->InsertColumn(1, elementCol);

    elementCol.SetText(_T("End Chan"));
    elementCol.SetAlign(wxLIST_FORMAT_LEFT);
    ListBoxElementList->InsertColumn(2, elementCol);
    ListBoxElementList->SetColumnWidth(0,10);
    ListBoxElementList->SetColumnWidth(1,10);
    ListBoxElementList->SetColumnWidth(2,10);

    ToolSizer->SetCols(13);
    AddModelButton("Arches", arches);
    AddModelButton("Candy Canes", canes);
    AddModelButton("Circle", circles);
    AddModelButton("Custom", custom);
    AddModelButton("Icicles", icicles_xpm);
    AddModelButton("Matrix", matrix);
    AddModelButton("Single Line", singleline);
    AddModelButton("Spinner", spinner);
    AddModelButton("Star", star);
    AddModelButton("Tree", tree);
    AddModelButton("Window Frame", frame);
    AddModelButton("Wreath", wreath);
    AddModelButton("Import Custom", import);

    modelPreview->Connect(wxID_CUT, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCut,0,this);
    modelPreview->Connect(wxID_COPY, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoCopy,0,this);
    modelPreview->Connect(wxID_PASTE, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoPaste,0,this);
    modelPreview->Connect(wxID_UNDO, wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::DoUndo,0,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(LayoutPanel::OnCharHook),0,this);
    modelPreview->Connect(wxID_ANY, wxEVT_CHAR, wxKeyEventHandler(LayoutPanel::OnChar),0,this);

    ListBoxModelGroups->SetImages((char**)eye_16,(char**)eye_16_gray);
    ListBoxModelGroups->Connect(wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&LayoutPanel::OnModelGroupRightDown,0,this);

    ModelGroupWindow = new wxScrolledWindow(ModelSplitter);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    ModelGroupWindow->SetSizer(sizer);
    model_grp_panel = new ModelGroupPanel(ModelGroupWindow, xlights->AllModels, xlights);
    sizer->Add( model_grp_panel, 1, wxEXPAND | wxALL, 1 );
    ModelGroupWindow->SetScrollRate(5,5);
    ModelGroupWindow->Hide();

    mDefaultSaveBtnColor = ButtonSavePreview->GetBackgroundColour();

    ChoiceLayoutGroups->Append("Default");
    ChoiceLayoutGroups->Append("All Models");
    ChoiceLayoutGroups->Append("Unassigned");
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        ChoiceLayoutGroups->Append(grp->GetName());
    }
    ChoiceLayoutGroups->Append("<Create New Preview>");
    ChoiceLayoutGroups->SetSelection(0);
    for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
    {
        if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
        {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
}

void LayoutPanel::SetDirtyHiLight(bool dirty) {
#ifdef __WXOSX__
    if (dirty) {
        ButtonSavePreview->SetForegroundColour(wxColour(255,0,0));
        ButtonSavePreview->SetBackgroundColour(wxColour(255,0,0));
    } else {
        ButtonSavePreview->SetForegroundColour(*wxBLACK);
        ButtonSavePreview->SetBackgroundColour(mDefaultSaveBtnColor);
    }
#else
    if (dirty) {
        ButtonSavePreview->SetBackgroundColour(wxColour(255,108,108));
    } else {
        ButtonSavePreview->SetBackgroundColour(mDefaultSaveBtnColor);
    }
#endif
}

void LayoutPanel::MarkEffectsFileDirty()
{
    xlights->UnsavedRgbEffectsChanges = true;
    SetDirtyHiLight(true);
}

void LayoutPanel::AddModelButton(const std::string &type, const char *data[]) {
    wxImage image(data);
#ifdef __WXOSX__
    wxBitmap bitmap(image, -1, 2.0);
#else
    image.Rescale(24, 24, wxIMAGE_QUALITY_HIGH);
    wxBitmap bitmap(image);
#endif
    NewModelBitmapButton *button = new NewModelBitmapButton(PreviewGLPanel, bitmap, type);
    ToolSizer->Add(button, 1, wxALL, 0);
    buttons.push_back(button);
    Connect(button->GetId(), wxEVT_BUTTON, (wxObjectEventFunction)&LayoutPanel::OnNewModelTypeButtonClicked);
}

LayoutPanel::~LayoutPanel()
{
    if (background != nullptr) {
        delete background;
    }
	//(*Destroy(LayoutPanel)
	//*)
}

void LayoutPanel::OnPropertyGridChange(wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    updatingProperty = true;
    if (name == "BkgBrightness") {
        xlights->SetPreviewBackgroundBrightness(event.GetValue().GetLong());
    } else if (name == "BkgSizeWidth") {
        xlights->SetPreviewSize(event.GetValue().GetLong(), modelPreview->GetVirtualCanvasHeight());
        xlights->UpdateModelsList();
    } else if (name == "BkgSizeHeight") {
        xlights->SetPreviewSize(modelPreview->GetVirtualCanvasWidth(), event.GetValue().GetLong());
        xlights->UpdateModelsList();
    } else if (name == "BkgImage") {
        if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
            xlights->SetPreviewBackgroundImage(event.GetValue().GetString());
        } else {
            for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
                LayoutGroup* grp = (LayoutGroup*)(*it);
                if( currentLayoutGroup == grp->GetName() ) {
                    grp->SetBackgroundImage(event.GetValue().GetString());
                    modelPreview->SetbackgroundImage(event.GetValue().GetString());
                    MarkEffectsFileDirty();
                    UpdatePreview();
                    break;
                }
            }
        }
    } else if (name == "BkgFill") {
        xlights->SetPreviewBackgroundScaled(event.GetValue().GetBool());
    } else if (selectedModel != nullptr) {
        //model property
        if ("ModelName" == name) {
            if (selectedModel->name != event.GetValue().GetString().ToStdString()) {
                for(int i=0;i<ListBoxElementList->GetItemCount();i++) {
                    if (selectedModel->name == ListBoxElementList->GetItemText(i)) {
                        ListBoxElementList->SetItemText(i, event.GetValue().GetString());
                    }
                }
                if (selectedModel->name == lastModelName) {
                    lastModelName = event.GetValue().GetString().ToStdString();
                }
                if (xlights->RenameModel(selectedModel->name, event.GetValue().GetString().ToStdString())) {
                    CallAfter(&LayoutPanel::UpdateModelList, true);
                }
            }
        } else {
            int i = selectedModel->OnPropertyGridChange(propertyEditor, event);
            if (i & 0x0001) {
                xlights->UpdatePreview();
            }
            if (i & 0x0002) {
                MarkEffectsFileDirty();
            }
            if (i & 0x0004) {
                CallAfter(&LayoutPanel::resetPropertyGrid);
            }
            if (i & 0x0008) {
                CallAfter(&LayoutPanel::refreshModelList);
            }
            if (i & 0x0010) {
                // Preview assignment change so model may not exist in current preview anymore
                CallAfter(&LayoutPanel::RefreshLayout);
            }
            if (i == 0) {
                printf("Did not handle %s   %s\n",
                       event.GetPropertyName().ToStdString().c_str(),
                       event.GetValue().GetString().ToStdString().c_str());
            }
        }
    }
    updatingProperty = false;
}
void LayoutPanel::OnPropertyGridChanging(wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (selectedModel != nullptr) {
        if ("ModelName" == name) {
            if (xlights->AllModels[event.GetValue().GetString().ToStdString()] != nullptr) {
                CreateUndoPoint("ModelName", selectedModel->name, event.GetProperty()->GetValue().GetString().ToStdString());
                event.Veto();
            }
        } else {
            CreateUndoPoint("ModelProperty", selectedModel->name, name, event.GetProperty()->GetValue().GetString().ToStdString());
            selectedModel->OnPropertyGridChanging(propertyEditor, event);
        }
    } else {
        CreateUndoPoint("Background", "", name, event.GetProperty()->GetValue().GetString().ToStdString());
    }
}

void LayoutPanel::RefreshLayout()
{
    RemoveModelGroupFilters();
    DeselectModelGroupList();
    DeselectModelList();
    xlights->UpdateModelsList();
    ShowPropGrid(true);
}

void LayoutPanel::UpdatePreview()
{
    SetDirtyHiLight(xlights->UnsavedRgbEffectsChanges);
    if(!modelPreview->StartDrawing(mPointSize)) return;
    if(m_creating_bound_rect)
    {
        DrawGLUtils::DrawRectangle(xlYELLOW,true,m_bound_start_x,m_bound_start_y,m_bound_end_x,m_bound_end_y);
    }
    modelPreview->Render();
    modelPreview->EndDrawing();
}

void LayoutPanel::resetPropertyGrid() {
    clearPropGrid();
    if (selectedModel != nullptr) {
        SetupPropGrid(selectedModel);
    }
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
int wxCALLBACK MyCompareFunction(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sortData))
{
    wxString a = ((Model*)item1)->name;
    wxString b = ((Model*)item2)->name;
    return a.CmpNoCase(b);
}
void LayoutPanel::refreshModelList() {
    for (int x = 0; x < ListBoxElementList->GetItemCount(); x++) {
        Model *model = (Model*)ListBoxElementList->GetItemData(x);
        std::string start_channel = model->GetModelXml()->GetAttribute("StartChannel").ToStdString();
        model->SetModelStartChan(start_channel);
        int end_channel = model->GetLastChannel()+1;
        ListBoxElementList->SetItem(x,1, start_channel);
        ListBoxElementList->SetItem(x,2, wxString::Format(wxT("%i"),end_channel));
    }
}
void LayoutPanel::UpdateModelList(bool update_groups) {
    UnSelectAllModels();
    ListBoxElementList->DeleteAllItems();


    std::vector<Model *> models;
    std::vector<Model *> dummy_models;

    // Update all the custom previews
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if( grp->GetName() == currentLayoutGroup ) {
            UpdateModelsForPreview( currentLayoutGroup, grp, models );
        } else {
            UpdateModelsForPreview( grp->GetName(), grp, dummy_models );
        }
    }

    // update the Layout tab preview for default options
    if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
        UpdateModelsForPreview( currentLayoutGroup, nullptr, models );
    }

    for (auto it = models.begin(); it != models.end(); it++) {
        Model *model = *it;
        if (model->GetDisplayAs() == "ModelGroup") {
            continue;
        }
        long itemIndex = ListBoxElementList->InsertItem(ListBoxElementList->GetItemCount(), model->name);

        std::string start_channel = model->GetModelXml()->GetAttribute("StartChannel").ToStdString();
        model->SetModelStartChan(start_channel);
        int end_channel = model->GetLastChannel()+1;
        ListBoxElementList->SetItem(itemIndex,1, start_channel);
        ListBoxElementList->SetItem(itemIndex,2, wxString::Format(wxT("%i"),end_channel));
        ListBoxElementList->SetItemPtrData(itemIndex,(wxUIntPtr)model);
    }

    if( update_groups ) {
        UpdateModelGroupList();
    }

    ListBoxElementList->SortItems(MyCompareFunction,0);
    if (!colSizesSet) {
        ListBoxElementList->SetColumnWidth(2,wxLIST_AUTOSIZE_USEHEADER);
        ListBoxElementList->SetColumnWidth(1,wxLIST_AUTOSIZE_USEHEADER);
        ListBoxElementList->SetColumnWidth(0,wxLIST_AUTOSIZE_USEHEADER);
        colSizesSet = true;
    }

    modelPreview->SetModels(models);
    UpdatePreview();
}

void LayoutPanel::UpdateModelsForPreview(const std::string &group, LayoutGroup* layout_grp, std::vector<Model *> &prev_models)
{
    std::set<std::string> modelsAdded;
    if( mSelectedGroup == -1 ) {
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            Model *model = it->second;
            if (model->GetDisplayAs() != "ModelGroup") {
                if (group == "All Models" || model->GetLayoutGroup() == group || model->GetLayoutGroup() == "All Previews" && group != "Unassigned") {
                    prev_models.push_back(model);
                    modelsAdded.insert(model->name);
                }
            }
        }

        // add in any models that were not in preview but belong to a group that is in the preview
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            Model *model = it->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                ModelGroup *grp = (ModelGroup*)(model);
                if (group == "All Models" || model->GetLayoutGroup() == group || model->GetLayoutGroup() == "All Previews" && group != "Unassigned") {
                    for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); it++) {
                        if (modelsAdded.find(*it) == modelsAdded.end()) {
                            Model *m = xlights->AllModels[*it];
                            if (m != nullptr) {
                                modelsAdded.insert(*it);
                                prev_models.push_back(m);
                            }
                        }
                    }
                }
            }
        }
    } else {
        std::string name = ListBoxModelGroups->GetItemText(mSelectedGroup, 1).ToStdString();
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            Model *model = it->second;
            if (model->GetDisplayAs() == "ModelGroup") {
                ModelGroup *grp = (ModelGroup*)(model);
                if( grp->name == name ) {
                    for (auto it = grp->ModelNames().begin(); it != grp->ModelNames().end(); it++) {
                        if (modelsAdded.find(*it) == modelsAdded.end()) {
                            Model *m = xlights->AllModels[*it];
                            if (m != nullptr) {
                                modelsAdded.insert(*it);
                                prev_models.push_back(m);
                            }
                        }
                    }
                }
            }
        }
    }
    // only run this for layout group previews
    if( layout_grp != nullptr ) {
        layout_grp->SetModels(prev_models);
        ModelPreview* preview = layout_grp->GetModelPreview();
        if( layout_grp->GetPreviewCreated() ) {
            preview->SetModels(layout_grp->GetModels());
            if( preview->GetActive() ) {
                preview->Refresh();
                preview->Update();
            }
        }
    }
}

void LayoutPanel::AddModelGroupItem(wxString name, ModelGroup *grp, bool selected)
{
    wxListItem li;
    li.SetId(mNumGroups);
    ListBoxModelGroups->InsertItem(li);
    ListBoxModelGroups->SetItemPtrData(mNumGroups,(wxUIntPtr)grp);
    ListBoxModelGroups->SetItem(mNumGroups,1, name);
    ListBoxModelGroups->SetChecked(mNumGroups, selected);
    mNumGroups++;
}

void LayoutPanel::UpdateModelGroupList()
{
    ListBoxModelGroups->ClearAll();

	wxListItem col0;
	col0.SetId(0);
	col0.SetText( _("") );
	col0.SetWidth(28);
	ListBoxModelGroups->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText( _("Model Group") );
	col1.SetWidth(130);
	ListBoxModelGroups->InsertColumn(1, col1);

	ListBoxModelGroups->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);

    mNumGroups = 0;

    for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
        Model *model = it->second;
        if (model->GetDisplayAs() == "ModelGroup") {
            if (currentLayoutGroup == "All Models" || model->GetLayoutGroup() == currentLayoutGroup || model->GetLayoutGroup() == "All Previews" && currentLayoutGroup != "Unassigned") {
                ModelGroup *grp = (ModelGroup*)model;
                AddModelGroupItem(it->first, grp, grp->IsSelected());
            }
        }
    }
}

void LayoutPanel::ModelGroupChecked(wxCommandEvent& event)
{
    int index = (size_t)event.GetClientObject();
    bool checked = ListBoxModelGroups->IsChecked(index);
    if( mSelectedGroup != -1 ) {
        ListBoxModelGroups->SetItemState(mSelectedGroup, 0, -1);
    }
    ListBoxModelGroups->SetItemState(index, checked ? wxLIST_STATE_SELECTED : 0, -1);
}

class xlImageProperty : public wxImageFileProperty {
public:
    xlImageProperty(const wxString& label,
                    const wxString& name,
                    const wxString& value,
                    const wxImage &img)
        : lastFileName(value), wxImageFileProperty(label, name, "") {

        SetValueFromString(value);
        m_pImage = new wxImage(img);
    }
    virtual ~xlImageProperty() {}

    virtual void OnSetValue() override {
        wxFileProperty::OnSetValue();
        wxFileName fn = GetFileName();
        if (fn != lastFileName) {
            lastFileName = fn;
            delete m_pImage;
            m_pImage = new wxImage(fn.GetFullPath());
        }
    }

private:
    wxFileName lastFileName;
};

void LayoutPanel::UnSelectAllModels(bool addBkgProps)
{
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        modelPreview->GetModels()[i]->Selected = false;
        modelPreview->GetModels()[i]->GroupSelected = false;
    }
    UpdatePreview();
    selectedModel = nullptr;

    if (!updatingProperty && addBkgProps) {
        propertyEditor->Freeze();
        clearPropGrid();

        wxString preview_background_image = GetBackgroundImageForSelectedPreview();
        if (backgroundFile != preview_background_image) {
            delete background;
            background = nullptr;
        }

        if (background == nullptr) {
            backgroundFile = preview_background_image;
            background = new wxImage(backgroundFile);
        }
        wxPGProperty* p = propertyEditor->Append(new xlImageProperty("Background Image",
                                                   "BkgImage",
                                                    preview_background_image,
                                                    *background));
        p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif|All files (*.*)|*.*");
        if( currentLayoutGroup == "Default" || currentLayoutGroup == "All Models" || currentLayoutGroup == "Unassigned" ) {
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
        }
        propertyEditor->Thaw();
    }
}

void LayoutPanel::SetupPropGrid(Model *model) {
    propertyEditor->Freeze();
    clearPropGrid();

    wxPGProperty* prop = propertyEditor->Append(new wxStringProperty("Name", "ModelName", model->name));

    model->AddProperties(propertyEditor);

    wxPGProperty *p2 = propertyEditor->Append(new wxPropertyCategory("Size/Location", "ModelSize"));
    p2->GetCell(0).SetFgCol(*wxBLACK);

    model->AddSizeLocationProperties(propertyEditor);
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
    modelPreview->SetFocus();
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

void LayoutPanel::OnButtonSavePreviewClick(wxCommandEvent& event)
{
    // update xml with offsets and scale
    for (size_t i=0; i < modelPreview->GetModels().size(); i++)
    {
        modelPreview->GetModels()[i]->UpdateXmlWithScale();
    }
    xlights->SaveEffectsFile();
    xlights->SetStatusText(_("Preview layout saved"));
    SetDirtyHiLight(false);
}

void LayoutPanel::OnListBoxElementListItemSelect(wxListEvent& event)
{
    ShowPropGrid(true);
    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND)
    {
        UnSelectAllModels();
        ListBoxElementList->SetToolTip("");
        return;
    }
    std::string name = ListBoxElementList->GetItemText(sel).ToStdString();
    UnSelectAllModels(false);
    SelectModel(name);

    for (int i = 0; i < ListBoxElementList->GetItemCount(); i++)
    {
        if (name == ListBoxElementList->GetItemText(i))
        {
            Model* m = (Model*)ListBoxElementList->GetItemData(i);
            ListBoxElementList->SetToolTip(xlights->GetChannelToControllerMapping(m->GetNumberFromChannelString(m->ModelStartChannel)));
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
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->HitTest(modelPreview,x,y))
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
        SelectModel(modelPreview->GetModels()[found[0]]->name);
        mHitTestNextSelectModelIndex = 0;
        return true;
    }
    else if (modelCount>1)
    {
        for(int i=0;i<modelCount;i++)
        {
            if(mHitTestNextSelectModelIndex==i)
            {
                SelectModel(modelPreview->GetModels()[found[i]]->name);
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
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->IsContained(modelPreview,m_bound_start_x,m_bound_start_y,
                                         m_bound_end_x,m_bound_end_y))
        {
            modelPreview->GetModels()[i]->GroupSelected = true;
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

        if(modelPreview->GetModels()[found[0]]->Selected)
        {
            modelPreview->GetModels()[found[0]]->Selected = false;
            modelPreview->GetModels()[found[0]]->GroupSelected = false;
        }
        else if (modelPreview->GetModels()[found[0]]->GroupSelected)
        {
            SetSelectedModelToGroupSelected();
            modelPreview->GetModels()[found[0]]->Selected = true;
            SelectModel(modelPreview->GetModels()[found[0]]->name);
        }
        else
        {
            modelPreview->GetModels()[found[0]]->GroupSelected = true;
        }
        UpdatePreview();
        return true;
    }
    return false;
}

void LayoutPanel::SetSelectedModelToGroupSelected()
{
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected)
        {
            modelPreview->GetModels()[i]->Selected = false;
            modelPreview->GetModels()[i]->GroupSelected = true;
        }
    }
}

void LayoutPanel::OnPreviewLeftDown(wxMouseEvent& event)
{
    ShowPropGrid(true);
    modelPreview->SetFocus();
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
    else if (m_over_handle != -1)
    {
        m_moving_handle = true;
    }
    else if (selectedButton != nullptr)
    {
        //create a new model
        int wi, ht;
        modelPreview->GetVirtualCanvasSize(wi,ht);
        m_previous_mouse_x = event.GetX();
        m_previous_mouse_y = event.GetY();
        int cy = modelPreview->GetVirtualCanvasHeight() - m_previous_mouse_y;
        if (m_previous_mouse_x < wi
            && cy < ht
            && cy >= 0) {

            m_moving_handle = true;
            m_creating_bound_rect = false;
            newModel = CreateNewModel(selectedButton->GetModelType());
            newModel->SetLayoutGroup(currentLayoutGroup);

            if (newModel != nullptr) {
                newModel->Selected = true;
                if (wi > 0 && ht > 0)
                {
                    modelPreview->SetCursor(newModel->InitializeLocation(m_over_handle, event.GetPosition().x,modelPreview->GetVirtualCanvasHeight() - y));
                    newModel->UpdateXmlWithScale();
                }
                lastModelName = newModel->name;
                modelPreview->GetModels().push_back(newModel);
            }
        }
    }
    else
    {
        m_moving_handle = false;
        m_creating_bound_rect = false;

        if(!event.wxKeyboardState::ControlDown())
        {
            DeselectModelList();
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

    m_moving_handle = false;
    m_dragging = false;
    if(m_creating_bound_rect)
    {
        m_bound_end_x = event.GetPosition().x;
        m_bound_end_y = modelPreview->GetVirtualCanvasHeight() - y;
        SelectAllInBoundingRect();
        m_creating_bound_rect = false;
        UpdatePreview();
    }
    if (newModel != nullptr) {
        if (selectedButton->GetModelType() == "Import Custom")
        {
            ImportCustomModel(newModel);
            if (selectedButton->GetState() == 1)
            {
                _lastCustomModel = "";
            }
        }
        CreateUndoPoint("All", "", "");
        newModel->UpdateXmlWithScale();
        xlights->AllModels.AddModel(newModel);

        newModel->SetLayoutGroup(currentLayoutGroup == "All Models" ? "Default" : currentLayoutGroup);

        if (mSelectedGroup != -1) {
            wxString sel = ListBoxModelGroups->GetItemText(mSelectedGroup, 1);
            ModelGroup *grp = (ModelGroup*)xlights->AllModels[sel.ToStdString()];
            if (grp != nullptr) {
                grp->AddModel(newModel->name);
                model_grp_panel->UpdatePanel(sel.ToStdString());
            }
        }

        m_over_handle = -1;
        modelPreview->SetCursor(wxCURSOR_DEFAULT);
        if (selectedButton->GetState() == 1) {
            std::string name = newModel->name;
            newModel = nullptr;
            if (selectedButton != nullptr) {
                selectedButton->SetState(0);
                selectedButton = nullptr;
            }
            xlights->UpdateModelsList(false);
            UpdatePreview();
            SelectModel(name);
        } else {
            newModel = nullptr;
            xlights->UpdateModelsList(false);
            UpdatePreview();
        }
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

    Model *m = newModel;
    if (m == nullptr) {
        int sel=ListBoxElementList->GetFirstSelected();
        if (sel == wxNOT_FOUND) return;
        m=(Model*)ListBoxElementList->GetItemData(sel);
    }

    if(m_moving_handle)
    {
        if (m != newModel) {
            CreateUndoPoint("SingleModel", m->name, std::to_string(m_over_handle));
        }
        y = modelPreview->GetVirtualCanvasHeight() - y;
        m->MoveHandle(modelPreview,m_over_handle, event.ShiftDown(), event.GetPosition().x, y);
        SetupPropGrid(m);
        MarkEffectsFileDirty();
        UpdatePreview();
    }
    else if (m_dragging && event.Dragging())
    {
        double delta_x = event.GetPosition().x - m_previous_mouse_x;
        double delta_y = -(event.GetPosition().y - m_previous_mouse_y);
        modelPreview->GetVirtualCanvasSize(wi, ht);
        if (wi > 0 && ht > 0)
        {
            for (size_t i=0; i<modelPreview->GetModels().size(); i++)
            {
                if(modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected)
                {
                    CreateUndoPoint("SingleModel", m->name, "location");

                    modelPreview->GetModels()[i]->AddOffset(delta_x/wi, delta_y/ht);
                    modelPreview->GetModels()[i]->UpdateXmlWithScale();
                    SetupPropGrid(modelPreview->GetModels()[i]);
                    MarkEffectsFileDirty();
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
            modelPreview->SetCursor(m->CheckIfOverHandles(m_over_handle,event.GetPosition().x,modelPreview->GetVirtualCanvasHeight() - y));
        }
    }
}

void LayoutPanel::OnPreviewRightDown(wxMouseEvent& event)
{
    modelPreview->SetFocus();

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
    if (selectedModelCnt > 0) {
        mnu.Append(ID_PREVIEW_MODEL_NODELAYOUT,"Node Layout");
        mnu.Append(ID_PREVIEW_MODEL_EXPORTCSV,"Export CSV");
        int sel = ListBoxElementList->GetFirstSelected();
        if (sel != wxNOT_FOUND)
        {
            Model* model = (Model*)ListBoxElementList->GetItemData(sel);
            if (model->IsCustom())
            {
                mnu.Append(ID_PREVIEW_MODEL_EXPORTCUSTOMMODEL, "Export Custom Model");
            }
        }
    }

    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        if (selectedModelCnt > 0) {
            mnu.AppendSeparator();
        }
        mnu.Append(ID_PREVIEW_DELETE_ACTIVE,"Delete this Preview");
    }

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayoutPanel::OnPreviewModelPopup, NULL, this);
    PopupMenu(&mnu);
    modelPreview->SetFocus();
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
    else if (event.GetId() == ID_PREVIEW_MODEL_NODELAYOUT)
    {
        int sel = ListBoxElementList->GetFirstSelected();
        if (sel == wxNOT_FOUND) return;
        Model* md=(Model*)ListBoxElementList->GetItemData(sel);
        wxString html=md->ChannelLayoutHtml();
        ChannelLayoutDialog dialog(this);
        dialog.SetHtmlSource(html);
        dialog.ShowModal();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTCSV)
    {
        ExportModel();
    }
    else if (event.GetId() == ID_PREVIEW_MODEL_EXPORTCUSTOMMODEL)
    {
        ExportCustomModel();
    }
    else if (event.GetId() == ID_PREVIEW_DELETE_ACTIVE)
    {
        DeleteCurrentPreview();
    }
}


#define retmsg(msg)  \
{ \
wxMessageBox(msg, _("Export Error")); \
return; \
}

void LayoutPanel::ExportModel()
{
#if 0
    model name
    display as
    type of strings
#strings
#nodes
    start channel
    start node = (channel+2)/3;
    my display
    brightness
#endif // 0
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, wxEmptyString, wxEmptyString, "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    //    if (filename.IsEmpty()) retmsg(wxString("Please choose an output file name."));
    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
    f.Write(_("Model_name, Display_as, String_type, String_count, Node_count, Start_channel, Start_node, My_display, Brightness+-\n"));

    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* model=(Model*)ListBoxElementList->GetItemData(sel);
    wxString stch = model->GetModelXml()->GetAttribute("StartChannel", wxString::Format("%d?", model->NodeStartChannel(0) + 1)); //NOTE: value coming from model is probably not what is wanted, so show the base ch# instead
    f.Write(wxString::Format("\"%s\", \"%s\", \"%s\", %d, %d, %s, %d, %s\n", model->name, model->GetDisplayAs(), model->GetStringType(), model->GetNodeCount() / model->NodesPerString(), model->GetNodeCount(), stch, /*WRONG:*/ model->NodeStartChannel(0) / model->NodesPerString() + 1, model->GetLayoutGroup()));
    f.Close();
}

void LayoutPanel::ImportCustomModel(Model* newModel)
{
    if (_lastCustomModel == "")
    {
        wxString filename = wxFileSelector(_("Choose custom model file"), wxEmptyString, wxEmptyString, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        _lastCustomModel = filename;
    }
    wxXmlDocument doc(_lastCustomModel);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "custommodel")
        {
            wxString name = root->GetAttribute("name");
            wxString cm = root->GetAttribute("CustomModel");
            wxString p1 = root->GetAttribute("parm1");
            wxString p2 = root->GetAttribute("parm2");
            wxString st = root->GetAttribute("StringType");
            wxString ps = root->GetAttribute("PixelSize");
            wxString t = root->GetAttribute("Transparency");
            wxString mb = root->GetAttribute("ModelBrightness");
            wxString a = root->GetAttribute("Antialias");
            wxString sn = root->GetAttribute("StrandNames");
            wxString nn = root->GetAttribute("NodeNames");
            wxString v = root->GetAttribute("SourceVersion");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            newModel->SetProperty("CustomModel", cm);
            newModel->SetProperty("parm1", p1);
            newModel->SetProperty("parm2", p2);
            newModel->SetProperty("StringType", st);
            newModel->SetProperty("PixelSize", ps);
            newModel->SetProperty("Transparency", t);
            newModel->SetProperty("ModelBrightness", mb);
            newModel->SetProperty("Antialias", a);
            newModel->SetProperty("StrandNames", sn);
            newModel->SetProperty("NodeNames", nn);
            wxString newname = name;
            int cnt = 1;
            while (xlights->AllModels[std::string(newname.c_str())] != nullptr)
            {
                newname = name + "-" + wxString::Format("%d", cnt++);
            }
            newModel->SetProperty("name", newname, true);

            MarkEffectsFileDirty();
        }
        else
        {
            wxMessageBox("Failure loading custom model file.");
        }
    }
    else
    {
        wxMessageBox("Failure loading custom model file.");
    }
}

void LayoutPanel::ExportCustomModel()
{
    int sel = ListBoxElementList->GetFirstSelected();
    if (sel == wxNOT_FOUND) return;
    Model* model = (Model*)ListBoxElementList->GetItemData(sel);
    wxString name = model->GetModelXml()->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
    wxString cm = model->GetModelXml()->GetAttribute("CustomModel");
    wxString p1 = model->GetModelXml()->GetAttribute("parm1");
    wxString p2 = model->GetModelXml()->GetAttribute("parm2");
    wxString st = model->GetModelXml()->GetAttribute("StringType");
    wxString ps = model->GetModelXml()->GetAttribute("PixelSize");
    wxString t = model->GetModelXml()->GetAttribute("Transparency");
    wxString mb = model->GetModelXml()->GetAttribute("ModelBrightness");
    wxString a = model->GetModelXml()->GetAttribute("Antialias");
    wxString sn = model->GetModelXml()->GetAttribute("StrandNames");
    wxString nn = model->GetModelXml()->GetAttribute("NodeNames");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" />\n");
    f.Close();
}

void LayoutPanel::PreviewModelAlignTops()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int top = modelPreview->GetModels()[selectedindex]->GetTop(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetTop(modelPreview,top);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignBottoms()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int bottom = modelPreview->GetModels()[selectedindex]->GetBottom(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetBottom(modelPreview,bottom);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignLeft()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int left = modelPreview->GetModels()[selectedindex]->GetLeft(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetLeft(modelPreview,left);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignRight()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    int right = modelPreview->GetModels()[selectedindex]->GetRight(modelPreview);
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetRight(modelPreview,right);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignHCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetHcenterOffset();
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetHcenterOffset(center);
        }
    }
    UpdatePreview();
}

void LayoutPanel::PreviewModelAlignVCenter()
{
    int selectedindex = GetSelectedModelIndex();
    if (selectedindex<0)
        return;
    CreateUndoPoint("All", modelPreview->GetModels()[selectedindex]->name);
    float center = modelPreview->GetModels()[selectedindex]->GetVcenterOffset();
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->GroupSelected)
        {
            modelPreview->GetModels()[i]->SetVcenterOffset(center);
        }
    }
    UpdatePreview();
}


int LayoutPanel::GetSelectedModelIndex()
{
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected)
        {
            return i;
        }
    }
    return -1;
}
int LayoutPanel::ModelsSelectedCount()
{
    int selectedModelCount=0;
    for (size_t i=0; i<modelPreview->GetModels().size(); i++)
    {
        if(modelPreview->GetModels()[i]->Selected || modelPreview->GetModels()[i]->GroupSelected)
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
}

void LayoutPanel::OnSplitterWindowSashPosChanged(wxSplitterEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutMainSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnGroupSplitterSashPosChanged(wxSplitterEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("LayoutModelGroupSplitterSash", event.GetSashPosition());
}

void LayoutPanel::OnNewModelTypeButtonClicked(wxCommandEvent& event) {
    for (auto it = buttons.begin(); it != buttons.end(); it++) {
        if (event.GetId() == (*it)->GetId()) {
            int state = (*it)->GetState();
            (*it)->SetState(state + 1);
            if ((*it)->GetState()) {
                selectedButton = (*it);
                UnSelectAllModels();
                modelPreview->SetFocus();
            } else {
                selectedButton = nullptr;
                _lastCustomModel = "";
            }
        } else if ((*it)->GetState()) {
            (*it)->SetState(0);
        }
    }
}

Model *LayoutPanel::CreateNewModel(const std::string &type) {
    std::string t = type;
    if (t == "Import Custom")
    {
        t = "Custom";
    }
    std::string startChannel = "1";
    if (xlights->AllModels[lastModelName] != nullptr) {
        startChannel = ">" + lastModelName + ":1";
    }
    Model* m = xlights->AllModels.CreateDefaultModel(t, startChannel);

    return m;
}
void LayoutPanel::OnChar(wxKeyEvent& event) {

    wxChar uc = event.GetKeyCode();
    switch (uc) {
        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            if (selectedModel != nullptr) {
                int wi, ht;
                modelPreview->GetVirtualCanvasSize(wi, ht);
                float xpct = 0;
                float ypct = 0;
                if (uc == WXK_UP) {
                    ypct = 1.0 / ht;
                } else if (uc == WXK_DOWN) {
                    ypct = -1.0 / ht;
                } else if (uc == WXK_LEFT) {
                    xpct = -1.0 / wi;
                } else if (uc == WXK_RIGHT) {
                    xpct = 1.0 / wi;
                }
                CreateUndoPoint("SingleModel", selectedModel->name, "location");
                selectedModel->AddOffset(xpct, ypct);
                selectedModel->UpdateXmlWithScale();
                SetupPropGrid(selectedModel);
                MarkEffectsFileDirty();
                UpdatePreview();
            }
            break;
        default:
            break;
    }
}
void LayoutPanel::OnCharHook(wxKeyEvent& event) {

    wxChar uc = event.GetKeyCode();

    switch(uc) {
#ifdef __WXMSW__
        case 'z':
        case 'Z':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_UNDO);
                DoUndo(evt);
            }
            break;
        case 'x':
        case 'X':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
            }
            break;
        case 'c':
        case 'C':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_COPY);
                DoCopy(evt);
            }
            break;
        case 'v':
        case 'V':
            if (event.ControlDown()) {
                wxCommandEvent evt(wxEVT_MENU, wxID_PASTE);
                DoPaste(evt);
            }
            break;
    case WXK_INSERT:
    case WXK_NUMPAD_INSERT:
        if (event.ControlDown()) // Copy
        {
            wxCommandEvent evt(wxEVT_MENU, wxID_COPY);
            DoCopy(evt);
            event.StopPropagation();
        }
        else if (event.ShiftDown()) // Paste
        {
            wxCommandEvent evt(wxEVT_MENU, wxID_PASTE);
            DoPaste(evt);
            event.StopPropagation();
        }
        break;
#endif
        case WXK_DELETE:
#ifdef __WXMSW__
            if (event.ShiftDown()) // Cut
            {
                wxCommandEvent evt(wxEVT_MENU, wxID_CUT);
                DoCut(evt);
                event.StopPropagation();
            }
            else
#endif
            DeleteSelectedModel();
            event.StopPropagation();
            break;
        case WXK_BACK:
            DeleteSelectedModel();
            event.StopPropagation();
            break;

        case WXK_UP:
        case WXK_DOWN:
        case WXK_LEFT:
        case WXK_RIGHT:
            if (selectedModel != nullptr) {
                int wi, ht;
                modelPreview->GetVirtualCanvasSize(wi, ht);
                float xpct = 0;
                float ypct = 0;
                if (uc == WXK_UP) {
                    ypct = 1.0 / ht;
                } else if (uc == WXK_DOWN) {
                    ypct = -1.0 / ht;
                } else if (uc == WXK_LEFT) {
                    xpct = -1.0 / wi;
                } else if (uc == WXK_RIGHT) {
                    xpct = 1.0 / wi;
                }
                CreateUndoPoint("SingleModel", selectedModel->name, "location");
                selectedModel->AddOffset(xpct, ypct);
                selectedModel->UpdateXmlWithScale();
                SetupPropGrid(selectedModel);
                MarkEffectsFileDirty();
                UpdatePreview();
            }
            break;

        default:
            event.Skip();
            break;
    }
}
void LayoutPanel::DeleteSelectedModel() {
    if( selectedModel != nullptr ) {
        CreateUndoPoint("All", selectedModel->name);
        xlights->AllModels.Delete(selectedModel->name);
        selectedModel = nullptr;
        xlights->UpdateModelsList();
        MarkEffectsFileDirty();
    }
}

void LayoutPanel::DoCopy(wxCommandEvent& event) {
    if (!modelPreview->HasFocus()) {
        event.Skip();
    } else if (selectedModel != nullptr) {
        wxString copy_data;
        wxXmlDocument doc;
        wxXmlNode *parent = selectedModel->GetModelXml()->GetParent();
        wxXmlNode *next = selectedModel->GetModelXml()->GetNext();
        parent->RemoveChild(selectedModel->GetModelXml());
        doc.SetRoot(selectedModel->GetModelXml());

        wxStringOutputStream stream;
        doc.Save(stream);
        copy_data = stream.GetString();
        doc.DetachRoot();

        parent->InsertChild(selectedModel->GetModelXml(), next);

        if (!copy_data.IsEmpty() && wxTheClipboard->Open()) {
            if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data))) {
                wxMessageBox(_("Unable to copy data to clipboard."), _("Error"));
            }
            wxTheClipboard->Close();
        }
    }
}
void LayoutPanel::DoCut(wxCommandEvent& event) {
    if (!modelPreview->HasFocus()) {
        event.Skip();
    } else if (selectedModel != nullptr) {
        DoCopy(event);
        DeleteSelectedModel();
    }
}
void LayoutPanel::DoPaste(wxCommandEvent& event) {
    if (!modelPreview->HasFocus()) {
        event.Skip();
    } else {
        if (wxTheClipboard->Open()) {
            CreateUndoPoint("All", selectedModel == nullptr ? "" : selectedModel->name);
            wxTextDataObject data;
            wxTheClipboard->GetData(data);
            wxStringInputStream in(data.GetText());
            wxXmlDocument doc(in);

            wxXmlNode *nd = doc.GetRoot();
            doc.DetachRoot();

            if (xlights->AllModels[lastModelName] != nullptr
                && nd->GetAttribute("Advanced", "0") != "1") {
                std::string startChannel = ">" + lastModelName + ":1";
                nd->DeleteAttribute("StartChannel");
                nd->AddAttribute("StartChannel", startChannel);
            }


            Model *newModel = xlights->AllModels.CreateModel(nd);
            int cnt = 1;
            std::string name = newModel->name;
            while (xlights->AllModels[name] != nullptr) {
                name = newModel->name + "-" + std::to_string(cnt++);
            }
            newModel->name = name;
            newModel->GetModelXml()->DeleteAttribute("name");
            newModel->GetModelXml()->AddAttribute("name", name);
            newModel->AddOffset(0.02, 0.02);
            newModel->UpdateXmlWithScale();
            xlights->AllModels.AddModel(newModel);
            lastModelName = name;

            xlights->UpdateModelsList();
            MarkEffectsFileDirty();
            wxTheClipboard->Close();
            SelectModel(name);
        }
    }
}

void LayoutPanel::DoUndo(wxCommandEvent& event) {
    int sz = undoBuffer.size() - 1;
    if (sz >= 0) {
        UnSelectAllModels();

        if (undoBuffer[sz].type == "Background") {
            wxPropertyGridEvent event;
            event.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Background", undoBuffer[sz].key, undoBuffer[sz].data);
            event.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event.SetPropertyValue(value);
            OnPropertyGridChange(event);
            UnSelectAllModels();
        } else if (undoBuffer[sz].type == "ModelProperty") {
            SelectModel(undoBuffer[sz].model);
            wxPropertyGridEvent event;
            event.SetPropertyGrid(propertyEditor);
            wxStringProperty wsp("Model", undoBuffer[sz].key, undoBuffer[sz].data);
            event.SetProperty(&wsp);
            wxVariant value(undoBuffer[sz].data);
            event.SetPropertyValue(value);
            OnPropertyGridChange(event);
            resetPropertyGrid();
        } else if (undoBuffer[sz].type == "SingleModel") {
            Model *m = xlights->AllModels[undoBuffer[sz].model];
            if (m != nullptr) {
                wxStringInputStream min(undoBuffer[sz].data);
                wxXmlDocument mdoc(min);

                wxXmlNode *parent = m->GetModelXml()->GetParent();
                wxXmlNode *next = m->GetModelXml()->GetNext();
                parent->RemoveChild(m->GetModelXml());

                delete m->GetModelXml();
                m->SetFromXml(mdoc.GetRoot());
                mdoc.DetachRoot();
                parent->InsertChild(m->GetModelXml(), next);
                SelectModel(undoBuffer[sz].model);
            }
        } else if (undoBuffer[sz].type == "All") {
            UnSelectAllModels();

            wxStringInputStream gin(undoBuffer[sz].groups);
            wxXmlDocument gdoc;
            gdoc.Load(gin);
            wxStringInputStream min(undoBuffer[sz].models);
            wxXmlDocument mdoc(min);

            wxXmlNode *m = xlights->ModelsNode->GetChildren();
            while (m != nullptr) {
                xlights->ModelsNode->RemoveChild(m);
                delete m;
                m = xlights->ModelsNode->GetChildren();
            }
            m = mdoc.GetRoot()->GetChildren();
            while (m != nullptr) {
                mdoc.GetRoot()->RemoveChild(m);
                xlights->ModelsNode->AddChild(m);
                m = mdoc.GetRoot()->GetChildren();
            }

            m = xlights->ModelGroupsNode->GetChildren();
            while (m != nullptr) {
                xlights->ModelGroupsNode->RemoveChild(m);
                delete m;
                m = xlights->ModelGroupsNode->GetChildren();
            }
            m = gdoc.GetRoot()->GetChildren();
            while (m != nullptr) {
                gdoc.GetRoot()->RemoveChild(m);
                xlights->ModelGroupsNode->AddChild(m);
                m = gdoc.GetRoot()->GetChildren();
            }

            xlights->UpdateModelsList();
            MarkEffectsFileDirty();
            if (undoBuffer[sz].model != "") {
                SelectModel(undoBuffer[sz].model);
            }
        } else if (undoBuffer[sz].type == "ModelName") {
            std::string origName = undoBuffer[sz].model;
            std::string newName = undoBuffer[sz].key;
            if (lastModelName == newName) {
                lastModelName = origName;
            }
            xlights->RenameModel(newName, origName);

            xlights->UpdateModelsList();
            MarkEffectsFileDirty();
            SelectModel(origName);
        }
        modelPreview->SetFocus();

        undoBuffer.resize(sz);
    }
}
void LayoutPanel::CreateUndoPoint(const std::string &type, const std::string &model, const std::string &key, const std::string &data) {
    MarkEffectsFileDirty();
    int idx = undoBuffer.size();

    //printf("%s   %s   %s  %s\n", type.c_str(), model.c_str(), key.c_str(), data.c_str());
    if (idx > 0 && (type == "SingleModel" || type == "ModelProperty" || type == "Background")
        && undoBuffer[idx - 1].model == model && undoBuffer[idx - 1].key == key)  {
        //SingleModel - multi mouse movement, just record the original
        //Background/ModelProperty - multiple changes of the same property (like spinning spin button)
        return;
    }
    if (idx >= 100) {  //100 steps is more than enough IMO
        for (int x = 1; x < idx; x++) {
            undoBuffer[x-1] = undoBuffer[x];
        }
        idx--;
    }
    undoBuffer.resize(idx + 1);

    undoBuffer[idx].type = type;
    undoBuffer[idx].model = model;
    undoBuffer[idx].key = key;
    undoBuffer[idx].data = data;

    if (type == "SingleModel") {
        Model *m = newModel;
        if (m == nullptr) {
            int sel=ListBoxElementList->GetFirstSelected();
            if (sel == wxNOT_FOUND) {
                undoBuffer.resize(idx);
                return;
            }
            m=(Model*)ListBoxElementList->GetItemData(sel);
        }
        wxXmlDocument doc;
        wxXmlNode *parent = m->GetModelXml()->GetParent();
        wxXmlNode *next = m->GetModelXml()->GetNext();
        parent->RemoveChild(m->GetModelXml());
        doc.SetRoot(m->GetModelXml());
        wxStringOutputStream stream;
        doc.Save(stream);
        undoBuffer[idx].data = stream.GetString();
        doc.DetachRoot();
        parent->InsertChild(m->GetModelXml(), next);
    } else if (type == "All") {
        wxXmlDocument doc;
        wxXmlNode *parent = xlights->ModelsNode->GetParent();
        wxXmlNode *next = xlights->ModelsNode->GetNext();
        parent->RemoveChild(xlights->ModelsNode);
        doc.SetRoot(xlights->ModelsNode);
        wxStringOutputStream stream;
        doc.Save(stream);
        undoBuffer[idx].models = stream.GetString();
        doc.DetachRoot();
        parent->InsertChild(xlights->ModelsNode, next);

        wxStringOutputStream stream2;
        parent = xlights->ModelGroupsNode->GetParent();
        next = xlights->ModelGroupsNode->GetNext();
        parent->RemoveChild(xlights->ModelGroupsNode);
        doc.SetRoot(xlights->ModelGroupsNode);
        doc.Save(stream2);
        undoBuffer[idx].groups = stream2.GetString();
        stream.Reset();
        doc.DetachRoot();
        parent->InsertChild(xlights->ModelGroupsNode, next);
    }
}

void LayoutPanel::OnListBoxElementListItemRClick(wxListEvent& event)
{
    wxMenu *mnuLayer = new wxMenu();
    mnuLayer->Append(ID_MNU_DELETE_MODEL,"Delete");
    mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnModelPopup, NULL, this);
    PopupMenu(mnuLayer);
}

void LayoutPanel::OnModelPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if(id == ID_MNU_DELETE_MODEL)
    {
        DeleteSelectedModel();
    }
}

void LayoutPanel::OnModelGroupPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if(id == ID_MNU_DELETE_MODEL_GROUP)
    {
        long item = -1;
        item = ListBoxModelGroups->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if( item != -1 ) {
            wxString name = ListBoxModelGroups->GetItemText(item, 1);
            if (wxMessageBox("Are you sure you want to remove the " + name + " group?", "Confirm Remove?", wxICON_QUESTION | wxYES_NO) == wxYES) {
                xlights->AllModels.Delete(name.ToStdString());
                selectedModel = nullptr;
                mSelectedGroup = -1;
                UnSelectAllModels();
                ShowPropGrid(true);
                xlights->UpdateModelsList();
                MarkEffectsFileDirty();
            }
        }
    }
    else if(id == ID_MNU_RENAME_MODEL_GROUP)
    {
        long item = -1;
        item = ListBoxModelGroups->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if( item != -1 ) {
            wxString sel = ListBoxModelGroups->GetItemText(item, 1);
            wxTextEntryDialog dlg(this, "Enter new name for group " + sel, "Rename " + sel, sel);
            if (dlg.ShowModal() == wxID_OK) {
                wxString name = dlg.GetValue();

                while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr) {
                    wxTextEntryDialog dlg2(this, "Model or Group of name " + name + " already exists. Enter new name for group", "Enter new name for group");
                    if (dlg2.ShowModal() == wxID_OK) {
                        name = dlg2.GetValue();
                    } else {
                        return;
                    }
                }
                xlights->AllModels.Rename(sel.ToStdString(), name.ToStdString());
                xlights->UpdateModelsList();
                MarkEffectsFileDirty();
                model_grp_panel->UpdatePanel(name.ToStdString());
            }
        }
    }
    else if(id == ID_MNU_ADD_MODEL_GROUP)
    {
        wxTextEntryDialog dlg(this, "Enter name for new group", "Enter name for new group");
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue();
            while (xlights->AllModels.GetModel(name.ToStdString()) != nullptr) {
                wxTextEntryDialog dlg2(this, "Model of name " + name + " already exists. Enter name for new group", "Enter name for new group");
                if (dlg2.ShowModal() == wxID_OK) {
                    name = dlg2.GetValue();
                } else {
                    return;
                }
            }
            wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "modelGroup");
            xlights->ModelGroupsNode->AddChild(node);
            node->AddAttribute("selected", "0");
            node->AddAttribute("name", name);
            node->AddAttribute("models", "");
            node->AddAttribute("layout", "grid");
            node->AddAttribute("GridSize", "400");
            wxString grp = currentLayoutGroup == "All Models" ? "Unassigned" : currentLayoutGroup;
            node->AddAttribute("LayoutGroup", grp);

            xlights->AllModels.AddModel(xlights->AllModels.CreateModel(node));
            xlights->UpdateModelsList();
            MarkEffectsFileDirty();
            model_grp_panel->UpdatePanel(name.ToStdString());
            ShowPropGrid(false);
        }
    }
}

void LayoutPanel::OnListBoxModelGroupsItemSelect(wxListEvent& event)
{
    wxListItem li = event.GetItem();
    int index = li.GetId();
    SelectModelGroup(index);
}

void LayoutPanel::SelectModelGroup(int index)
{
    UnSelectAllModels(false);
    DeselectModelList();
    mSelectedGroup = index;

    for( int i = 0; i < ListBoxModelGroups->GetItemCount(); i++) {
        ListBoxModelGroups->SetChecked(i, false);
    }
    ListBoxModelGroups->SetChecked(index, true);

    std::string name = ListBoxModelGroups->GetItemText(index, 1).ToStdString();
    model_grp_panel->UpdatePanel(name);
    ShowPropGrid(false);
    xlights->UpdateModelsList(false);
}

void LayoutPanel::OnListBoxModelGroupsItemDeselect(wxListEvent& event)
{
    RemoveModelGroupFilters();
    xlights->UpdateModelsList(false);
}

void LayoutPanel::RemoveModelGroupFilters()
{
    for( int i = 0; i < ListBoxModelGroups->GetItemCount(); i++) {
        ListBoxModelGroups->SetChecked(i, false);
    }
    mSelectedGroup = -1;
}

void LayoutPanel::DeselectModelGroupList()
{
   long item = -1;
   while( 1 )
   {
      item = ListBoxModelGroups->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if( item == -1 )
         break;
      ListBoxModelGroups->SetItemState( item, 0, -1 );
   }
}

void LayoutPanel::DeselectModelList()
{
   long item = -1;
   while( 1 )
   {
      item = ListBoxElementList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if( item == -1 )
         break;
      ListBoxElementList->SetItemState( item, 0, -1 );
   }
}

void LayoutPanel::OnModelGroupRightDown(wxMouseEvent& event)
{
    DeselectModelGroupList();
    wxMenu *mnuLayer = new wxMenu();
    wxPoint pos = event.GetPosition();
    int flags = wxLIST_HITTEST_ONITEMLABEL;
    long index = ListBoxModelGroups->HitTest(pos,flags,NULL); // got to use it at last
    mnuLayer->Append(ID_MNU_ADD_MODEL_GROUP,"Add Group");
    mSelectedGroup = index;
    if( mSelectedGroup != -1 ) {
        ListBoxModelGroups->SetItemState( index, wxLIST_STATE_SELECTED, -1 );
        mnuLayer->Append(ID_MNU_DELETE_MODEL_GROUP,"Delete Group");
        mnuLayer->Append(ID_MNU_RENAME_MODEL_GROUP,"Rename Group");
    }
    mnuLayer->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&LayoutPanel::OnModelGroupPopup, NULL, this);
    PopupMenu(mnuLayer);
}

LayoutGroup* LayoutPanel::GetLayoutGroup(const std::string &name)
{
    for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
        LayoutGroup* grp = (LayoutGroup*)(*it);
        if( grp->GetName() == name ) {
            return grp;
        }
    }
    return nullptr;
}

void LayoutPanel::OnChoiceLayoutGroupsSelect(wxCommandEvent& event)
{
    std::string choice_layout = std::string(ChoiceLayoutGroups->GetStringSelection().c_str());
    if( choice_layout == "<Create New Preview>" ) {
        wxTextEntryDialog dlg(this, "Enter name for new preview", "Create New Preview");
        if (dlg.ShowModal() == wxID_OK) {
            wxString name = dlg.GetValue();
            while (GetLayoutGroup(name.ToStdString()) != nullptr || name == "Default" || name == "All Models" || name == "Unassigned") {
                wxTextEntryDialog dlg2(this, "Preview of name " + name + " already exists. Enter name for new preview", "Create New Preview");
                if (dlg2.ShowModal() == wxID_OK) {
                    name = dlg2.GetValue();
                } else {
                    SwitchChoiceToCurrentLayoutGroup();
                    return;
                }
            }
            wxXmlNode *node = new wxXmlNode(wxXML_ELEMENT_NODE, "layoutGroup");
            xlights->LayoutGroupsNode->AddChild(node);
            node->AddAttribute("name", name);

            mSelectedGroup = -1;
            currentLayoutGroup = name.ToStdString();
            LayoutGroup* grp = new LayoutGroup(name.ToStdString(), xlights, node, xlights->GetDefaultPreviewBackgroundImage());
            xlights->LayoutGroups.push_back(grp);
            xlights->AddPreviewOption(grp);
            ChoiceLayoutGroups->Insert(name, ChoiceLayoutGroups->GetCount()-1);
            ChoiceLayoutGroups->SetSelection(ChoiceLayoutGroups->GetCount()-2);

            xlights->UpdateModelsList();
            MarkEffectsFileDirty();
            ShowPropGrid(true);
        } else {
            SwitchChoiceToCurrentLayoutGroup();
            return;
        }
    } else {
        currentLayoutGroup = choice_layout;
        mSelectedGroup = -1;
        UpdateModelList();
    }
    modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
    UpdatePreview();

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("CurrentLayoutGroup", (wxString)currentLayoutGroup );
}

void LayoutPanel::AddPreviewChoice(const std::string &name)
{
    ChoiceLayoutGroups->Insert(name, ChoiceLayoutGroups->GetCount()-1);
    model_grp_panel->AddPreviewChoice(name);

    // see if we need to switch to this one
    wxConfigBase* config = wxConfigBase::Get();
    wxString storedLayoutGroup = config->Read("CurrentLayoutGroup", "Default");
    if( storedLayoutGroup == name ) {
        for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
        {
            if( ChoiceLayoutGroups->GetString(i) == storedLayoutGroup )
            {
                ChoiceLayoutGroups->SetSelection(i);
                modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
                UpdatePreview();
                break;
            }
        }
    }
}

const wxString& LayoutPanel::GetBackgroundImageForSelectedPreview() {
    previewBackgroundFile = xlights->GetDefaultPreviewBackgroundImage();
    if( currentLayoutGroup != "Default" && currentLayoutGroup != "All Models" && currentLayoutGroup != "Unassigned" ) {
        for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
            LayoutGroup* grp = (LayoutGroup*)(*it);
            if( currentLayoutGroup == grp->GetName() ) {
                previewBackgroundFile = grp->GetBackgroundImage();
                break;
            }
        }
    }
    return previewBackgroundFile;
}

void LayoutPanel::SwitchChoiceToCurrentLayoutGroup() {
    ChoiceLayoutGroups->SetSelection(0);
    for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
    {
        if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
        {
            ChoiceLayoutGroups->SetSelection(i);
            break;
        }
    }
}

void LayoutPanel::DeleteCurrentPreview()
{
    if (wxMessageBox("Are you sure you want to delete the " + currentLayoutGroup + " preview?", "Confirm Delete?", wxICON_QUESTION | wxYES_NO) == wxYES) {
        for (auto it = xlights->LayoutGroups.begin(); it != xlights->LayoutGroups.end(); it++) {
            LayoutGroup* grp = (LayoutGroup*)(*it);
            if (grp != nullptr) {
                if( currentLayoutGroup == grp->GetName() ) {
                    grp->GetLayoutGroupXml()->GetParent()->RemoveChild(grp->GetLayoutGroupXml());
                    xlights->LayoutGroups.erase(it);
                    delete grp->GetLayoutGroupXml();
                    delete grp;
                    break;
                }
            }
        }
        MarkEffectsFileDirty();
        mSelectedGroup = -1;
        for( int i = 0; i < ChoiceLayoutGroups->GetCount(); i++ )
        {
            if( ChoiceLayoutGroups->GetString(i) == currentLayoutGroup )
            {
                ChoiceLayoutGroups->Delete(i);
                break;
            }
        }
        // change any existing assignments to this preview to be unassigned
        for (auto it = xlights->AllModels.begin(); it != xlights->AllModels.end(); it++) {
            Model *model = it->second;
            if( model->GetLayoutGroup() == currentLayoutGroup) {
                model->SetLayoutGroup("Unassigned");
            }
        }

        currentLayoutGroup = "Default";
        ChoiceLayoutGroups->SetSelection(0);

        UpdateModelList();
        modelPreview->SetbackgroundImage(GetBackgroundImageForSelectedPreview());
        UpdatePreview();
    }
}

void LayoutPanel::ShowPropGrid(bool show)
{
    if( !mPropGridActive && show ) {
        ModelSplitter->ReplaceWindow(ModelGroupWindow, propertyEditor);
        ModelGroupWindow->Hide();
        propertyEditor->Show();
        mPropGridActive = true;
    } else if( mPropGridActive && !show) {
        ModelSplitter->ReplaceWindow(propertyEditor, ModelGroupWindow);
        propertyEditor->Hide();
        ModelGroupWindow->Show();
        mPropGridActive = false;
    }
}


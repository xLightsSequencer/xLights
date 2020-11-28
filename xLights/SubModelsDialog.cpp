/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(SubModelsDialog)
 #include <wx/button.h>
 #include <wx/checkbox.h>
 #include <wx/grid.h>
 #include <wx/intl.h>
 #include <wx/listctrl.h>
 #include <wx/notebook.h>
 #include <wx/panel.h>
 #include <wx/sizer.h>
 #include <wx/splitter.h>
 #include <wx/stattext.h>
 #include <wx/string.h>
 #include <wx/textctrl.h>
 //*)

#include <wx/dnd.h>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include <wx/tokenzr.h>

#include "SubModelsDialog.h"
#include "models/Model.h"
#include "SubBufferPanel.h"
#include "SubModelGenerateDialog.h"
#include "NodeSelectGrid.h"
#include "UtilFunctions.h"
#include "xLightsApp.h"
#include "models/ModelManager.h"
#include "xLightsMain.h"
#include "ModelPreview.h"
#include "DimmingCurve.h"
#include "AlignmentDialog.h"

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_SMDROP, wxCommandEvent);

//(*IdInit(SubModelsDialog)
const long SubModelsDialog::ID_STATICTEXT1 = wxNewId();
const long SubModelsDialog::ID_LISTCTRL_SUB_MODELS = wxNewId();
const long SubModelsDialog::ID_BUTTON3 = wxNewId();
const long SubModelsDialog::ID_BUTTON4 = wxNewId();
const long SubModelsDialog::ID_BUTTONCOPY = wxNewId();
const long SubModelsDialog::ID_BUTTON5 = wxNewId();
const long SubModelsDialog::ID_BUTTON_COPY_MODEL = wxNewId();
const long SubModelsDialog::ID_BUTTON_SUB_IMPORT = wxNewId();
const long SubModelsDialog::ID_BUTTON9 = wxNewId();
const long SubModelsDialog::ID_BUTTON10 = wxNewId();
const long SubModelsDialog::ID_PANEL4 = wxNewId();
const long SubModelsDialog::ID_STATICTEXT_NAME = wxNewId();
const long SubModelsDialog::ID_TEXTCTRL_NAME = wxNewId();
const long SubModelsDialog::ID_CHECKBOX1 = wxNewId();
const long SubModelsDialog::ID_BUTTON6 = wxNewId();
const long SubModelsDialog::ID_BUTTON8 = wxNewId();
const long SubModelsDialog::ID_GRID1 = wxNewId();
const long SubModelsDialog::ID_BUTTON1 = wxNewId();
const long SubModelsDialog::ID_BUTTON2 = wxNewId();
const long SubModelsDialog::ID_BUTTON_MOVE_UP = wxNewId();
const long SubModelsDialog::ID_BUTTON_MOVE_DOWN = wxNewId();
const long SubModelsDialog::ID_BUTTON7 = wxNewId();
const long SubModelsDialog::ID_BUTTON_DRAW_MODEL = wxNewId();
const long SubModelsDialog::ID_PANEL2 = wxNewId();
const long SubModelsDialog::ID_PANEL3 = wxNewId();
const long SubModelsDialog::ID_NOTEBOOK1 = wxNewId();
const long SubModelsDialog::ID_PANEL5 = wxNewId();
const long SubModelsDialog::ID_PANEL1 = wxNewId();
const long SubModelsDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SubModelsDialog,wxDialog)
	//(*EventTable(SubModelsDialog)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_SMDROP, SubModelsDialog::OnDrop)
END_EVENT_TABLE()

SubModelsDialog::SubModelsDialog(wxWindow* parent)
{
	//(*Initialize(SubModelsDialog)
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;
	wxPanel* Panel1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Sub Models"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(778,368));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(0);
	Panel2 = new wxPanel(this, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer9 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(1);
	StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("SubModels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer9->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl_SubModels = new wxListCtrl(Panel2, ID_LISTCTRL_SUB_MODELS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_SUB_MODELS"));
	FlexGridSizer9->Add(ListCtrl_SubModels, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer10 = new wxFlexGridSizer(4, 2, 0, 0);
	AddButton = new wxButton(Panel2, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	AddButton->SetToolTip(_("Add New Submodel"));
	FlexGridSizer10->Add(AddButton, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	DeleteButton = new wxButton(Panel2, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	DeleteButton->SetToolTip(_("Delete Selected Submodel"));
	FlexGridSizer10->Add(DeleteButton, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonCopy = new wxButton(Panel2, ID_BUTTONCOPY, _("Copy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONCOPY"));
	ButtonCopy->SetToolTip(_("Copy Selected Submodel"));
	FlexGridSizer10->Add(ButtonCopy, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	Button_Generate = new wxButton(Panel2, ID_BUTTON5, _("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	Button_Generate->SetToolTip(_("Generate SubBuffer Slices"));
	FlexGridSizer10->Add(Button_Generate, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonCopyModel = new wxButton(Panel2, ID_BUTTON_COPY_MODEL, _("Import from Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_COPY_MODEL"));
	ButtonCopyModel->SetToolTip(_("Import Submodels from another model"));
	FlexGridSizer10->Add(ButtonCopyModel, 1, wxALL|wxEXPAND, 5);
	Button_Sub_Import = new wxButton(Panel2, ID_BUTTON_SUB_IMPORT, _("Import from File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SUB_IMPORT"));
	Button_Sub_Import->SetToolTip(_("Import Submodels from an .xmodel file"));
	FlexGridSizer10->Add(Button_Sub_Import, 1, wxALL|wxEXPAND, 5);
	Button_importCustom = new wxButton(Panel2, ID_BUTTON9, _("Import Custom"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON9"));
	Button_importCustom->SetToolTip(_("Import a Custom Model On a Matix as a Submodel"));
	FlexGridSizer10->Add(Button_importCustom, 1, wxALL|wxEXPAND, 5);
	Button_ExportCustom = new wxButton(Panel2, ID_BUTTON10, _("Export as CSV"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
	Button_ExportCustom->SetToolTip(_("Export SubModel as CSV File"));
	FlexGridSizer10->Add(Button_ExportCustom, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxEXPAND|wxSHAPED|wxFIXED_MINSIZE, 5);
	Panel2->SetSizer(FlexGridSizer9);
	FlexGridSizer9->Fit(Panel2);
	FlexGridSizer9->SetSizeHints(Panel2);
	FlexGridSizer2->Add(Panel2, 0, wxEXPAND, 0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinimumPaneSize(100);
	SplitterWindow1->SetSashGravity(0.5);
	Panel3 = new wxPanel(SplitterWindow1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(1);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticTextName = new wxStaticText(Panel3, ID_STATICTEXT_NAME, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_NAME"));
	FlexGridSizer7->Add(StaticTextName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(Panel3, ID_TEXTCTRL_NAME, _("Submodel Name"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_NAME"));
	FlexGridSizer7->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	TypeNotebook = new wxNotebook(Panel3, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel1 = new wxPanel(TypeNotebook, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	FlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer8->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	LayoutCheckbox = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Vertical Buffer Layout"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	LayoutCheckbox->SetValue(false);
	FlexGridSizer6->Add(LayoutCheckbox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	Button_ReverseNodes = new wxButton(Panel1, ID_BUTTON6, _("Reverse Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer6->Add(Button_ReverseNodes, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	Button_ReverseRows = new wxButton(Panel1, ID_BUTTON8, _("Reverse Rows"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	FlexGridSizer6->Add(Button_ReverseRows, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	NodesGrid = new wxGrid(Panel1, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
	NodesGrid->CreateGrid(5,1);
	NodesGrid->EnableEditing(true);
	NodesGrid->EnableGridLines(true);
	NodesGrid->SetColLabelSize(18);
	NodesGrid->SetDefaultColSize(160, true);
	NodesGrid->SetColLabelValue(0, _("Node Ranges"));
	NodesGrid->SetRowLabelValue(0, _("Strand   1"));
	NodesGrid->SetDefaultCellFont( NodesGrid->GetFont() );
	NodesGrid->SetDefaultCellTextColour( NodesGrid->GetForegroundColour() );
	FlexGridSizer8->Add(NodesGrid, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	AddRowButton = new wxButton(Panel1, ID_BUTTON1, _("Add Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(AddRowButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteRowButton = new wxButton(Panel1, ID_BUTTON2, _("Delete Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer5->Add(DeleteRowButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MoveUp = new wxButton(Panel1, ID_BUTTON_MOVE_UP, _("^"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_UP"));
	FlexGridSizer5->Add(Button_MoveUp, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_MoveDown = new wxButton(Panel1, ID_BUTTON_MOVE_DOWN, _("v"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_DOWN"));
	FlexGridSizer5->Add(Button_MoveDown, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ReverseRow = new wxButton(Panel1, ID_BUTTON7, _("Reverse Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer5->Add(Button_ReverseRow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Draw_Model = new wxButton(Panel1, ID_BUTTON_DRAW_MODEL, _("Draw Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DRAW_MODEL"));
	FlexGridSizer5->Add(Button_Draw_Model, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 0);
	Panel1->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel1);
	FlexGridSizer4->SetSizeHints(Panel1);
	SubBufferPanelHolder = new wxPanel(TypeNotebook, ID_PANEL3, wxPoint(90,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	SubBufferSizer = new wxFlexGridSizer(1, 1, 0, 0);
	SubBufferSizer->AddGrowableCol(0);
	SubBufferSizer->AddGrowableRow(0);
	SubBufferPanelHolder->SetSizer(SubBufferSizer);
	SubBufferSizer->Fit(SubBufferPanelHolder);
	SubBufferSizer->SetSizeHints(SubBufferPanelHolder);
	TypeNotebook->AddPage(Panel1, _("Node Ranges"), false);
	TypeNotebook->AddPage(SubBufferPanelHolder, _("SubBuffer"), false);
	FlexGridSizer3->Add(TypeNotebook, 1, wxALL|wxEXPAND, 0);
	Panel3->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel3);
	FlexGridSizer3->SetSizeHints(Panel3);
	ModelPreviewPanelLocation = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	ModelPreviewPanelLocation->SetMinSize(wxDLG_UNIT(SplitterWindow1,wxSize(100,100)));
	PreviewSizer = new wxFlexGridSizer(1, 1, 0, 0);
	PreviewSizer->AddGrowableCol(0);
	PreviewSizer->AddGrowableRow(0);
	ModelPreviewPanelLocation->SetSizer(PreviewSizer);
	PreviewSizer->Fit(ModelPreviewPanelLocation);
	PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);
	SplitterWindow1->SplitVertically(Panel3, ModelPreviewPanelLocation);
	FlexGridSizer2->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 0);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	Center();

	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsBeginDrag);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsItemSelect);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsKeyDown);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsColumnClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddButtonClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteButtonClick);
	Connect(ID_BUTTONCOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_Sub_CopyClick);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_GenerateClick);
	Connect(ID_BUTTON_COPY_MODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButtonCopyModelClick);
	Connect(ID_BUTTON_SUB_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_Sub_ImportClick);
	Connect(ID_BUTTON9,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_importCustomClick);
	Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ExportCustomClick);
	Connect(ID_TEXTCTRL_NAME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SubModelsDialog::OnTextCtrl_NameText_Change);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnLayoutCheckboxClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseNodesClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseRowsClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridLabelLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_DCLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridLabelLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddRowButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteRowButtonClick);
	Connect(ID_BUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_MoveUpClick);
	Connect(ID_BUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_MoveDownClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseRowClick);
	Connect(ID_BUTTON_DRAW_MODEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_Draw_ModelClick);
	//*)

    Connect(ID_NOTEBOOK1, wxEVT_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)& SubModelsDialog::OnTypeNotebookPageChanged);
    Connect(wxID_ANY, EVT_SMDROP, (wxObjectEventFunction)&SubModelsDialog::OnDrop);
    Connect(ID_GRID1, wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellChange);
    //Connect(ID_GRID1, wxEVT_CHAR, (wxObjectEventFunction)&SubModelsDialog::OnGridChar);

    TextCtrl_Name->Bind(wxEVT_KILL_FOCUS, &SubModelsDialog::OnTextCtrl_NameText_KillFocus, this);

    wxListItem nm0;
    nm0.SetId(0);
    nm0.SetImage(-1);
    nm0.SetAlign(wxLIST_FORMAT_LEFT);
    nm0.SetText(_("SubModel"));
    ListCtrl_SubModels->InsertColumn(1, nm0);

    _parent = parent;

    SetSize(1200, 800);
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsSubModelDialogPosition", sz, loc);
    if (loc.x != -1)
    {
        if (sz.GetWidth() < 400) sz.SetWidth(400);
        if (sz.GetHeight() < 300) sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);

    modelPreview = new ModelPreview(ModelPreviewPanelLocation);
    modelPreview->SetMinSize(wxSize(100, 100));
    PreviewSizer->Add(modelPreview, 1, wxALL|wxEXPAND, 0);
    PreviewSizer->Fit(ModelPreviewPanelLocation);
    PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);

    subBufferPanel = new SubBufferPanel(SubBufferPanelHolder, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    subBufferPanel->SetMinSize(SubBufferSizer->GetSize());
    SubBufferSizer->Add(subBufferPanel, 1, wxALL|wxEXPAND, 2);
    SubBufferSizer->Fit(SubBufferPanelHolder);
    SubBufferSizer->SetSizeHints(SubBufferPanelHolder);
    Connect(subBufferPanel->GetId(),SUBBUFFER_RANGE_CHANGED,(wxObjectEventFunction)&SubModelsDialog::OnSubBufferRangeChange);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    SubModelTextDropTarget *mdt = new SubModelTextDropTarget(this, ListCtrl_SubModels, "SubModel");
    ListCtrl_SubModels->SetDropTarget(mdt);

    NodesGrid->SetSelectionBackground(*wxLIGHT_GREY);
    NodesGrid->SetCellHighlightColour(*wxLIGHT_GREY);
    NodesGrid->SetCellHighlightROPenWidth(3);
    NodesGrid->SetCellHighlightPenWidth(3);
    NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
    SetEscapeId(wxID_CANCEL);
}

SubModelsDialog::~SubModelsDialog()
{
    //(*Destroy(SubModelsDialog)
    //*)
    for (auto a= _subModels.begin(); a != _subModels.end(); ++a) {
        delete *a;
    }
    _subModels.clear();
    SaveWindowPosition("xLightsSubModelDialogPosition", this);
}

//void SubModelsDialog::OnGridChar(wxKeyEvent& event)
//{
//    CallAfter(&SubModelsDialog::ValidateWindow);
//}

void SubModelsDialog::Setup(Model *m)
{
    model = m;
    modelPreview->SetModel(m);

    if ((m->GetDisplayAs() == "Matrix" ||
        m->GetDisplayAs() == "Tree") && m->GetDefaultBufferWi() > 1 && m->GetDefaultBufferHt() > 1)
    {
        _isMatrix = true;
    }

    ReadSubModelXML(m->GetModelXml());
}

#pragma region helpers

SubModelsDialog::SubModelInfo *SubModelsDialog::GetSubModelInfo(const wxString &name) {
    for (int a = 0; a < _subModels.size(); a++) {
        if (_subModels[a]->name == name) {
            return _subModels[a];
        }
    }
    return nullptr;
}

bool SubModelsDialog::IsItemSelected(wxListCtrl* ctrl, int item) const
{
    return ctrl->GetItemState(item, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
}

void SubModelsDialog::RemoveSubModelFromList(wxString name)
{
    SubModelInfo* toDelete = GetSubModelInfo(name);
    _subModels.erase(_subModels.begin() + GetSubModelInfoIndex(name));
    delete toDelete;
}

wxString SubModelsDialog::GetSelectedName() const
{
    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrl_SubModels, i) )
        {
            return ListCtrl_SubModels->GetItemText(i);
        }
    }
    return "";
}

int SubModelsDialog::GetSelectedIndex() const
{
    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrl_SubModels, i) )
        {
            return i;
        }
    }
    return -1;
}

wxString SubModelsDialog::GetSelectedNames()
{
    wxString names = "";
    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrl_SubModels, i) )
        {
            names+=ListCtrl_SubModels->GetItemText(i)+",";
        }
    }
    if (names.EndsWith(",")) {
        names = names.RemoveLast();
    }
    return names;
}

int SubModelsDialog::GetSubModelInfoIndex(const wxString &name) {
    for (int a=0; a < _subModels.size(); a++) {
        if (_subModels[a]->name == name) {
            return a;
        }
    }
    return -1;
}

#pragma endregion helpers

void SubModelsDialog::Save()
{
    xLightsFrame* xlights = xLightsApp::GetFrame();
    wxXmlNode * root = model->GetModelXml();
    wxXmlNode * child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == "subModel") {
            wxXmlNode *n = child;
            child = child->GetNext();
            root->RemoveChild(n);
            delete n;
        } else {
            child = child->GetNext();
        }
    }

    for (auto a = _subModels.begin(); a != _subModels.end(); ++a) {
        child = new wxXmlNode(wxXML_ELEMENT_NODE, "subModel");
        child->AddAttribute("name", (*a)->name);
        child->AddAttribute("layout", (*a)->vertical ? "vertical" : "horizontal");
        child->AddAttribute("type", (*a)->isRanges ? "ranges" : "subbuffer");

        // If the submodel name has changed ... we need to rename the model
        if ((*a)->oldName != (*a)->name)
        {
            xlights->RenameModel(model->GetName() + std::string("/") + (*a)->oldName.ToStdString(), model->GetName() + std::string("/") + (*a)->name.ToStdString());
        }

        if ((*a)->isRanges) {
            for (int x = 0; x < (*a)->strands.size(); x++) {
                child->AddAttribute(wxString::Format("line%d", x), (*a)->strands[x]);
            }
        } else {
            child->AddAttribute("subBuffer", (*a)->subBuffer);
        }
        root->AddChild(child);
    }

    std::vector<std::string> submodelOrder;
    for (auto it = _subModels.begin(); it != _subModels.end(); ++it)
    {
        submodelOrder.push_back((*it)->name);
    }

    xlights->EnsureSequenceElementsAreOrderedCorrectly(model->GetName(), submodelOrder);
}

#pragma region actions

void SubModelsDialog::OnNameChoiceSelect(wxCommandEvent& event)
{
    Select(event.GetString());
}

wxString SubModelsDialog::GenerateSubModelName(wxString basename)
{
    basename += "-";

    // Work out the next available default name
    std::list<int> used;
    for (int j = 0; j < ListCtrl_SubModels->GetItemCount(); j++)
    {
        wxString name = ListCtrl_SubModels->GetItemText(j);

        if (name.StartsWith(basename) && name.Length() > basename.Length() && name.SubString(basename.Length(), name.Length() - 1).IsNumber())
        {
            int num = wxAtoi(name.SubString(basename.Length(), name.Length() - 1));
            used.push_back(num);
        }
    }

    int i = 1;
    while (std::find(used.begin(), used.end(), i) != used.end())
    {
        i++;
    }

    return wxString::Format("%s%d", basename, i);
}

void SubModelsDialog::OnAddButtonClick(wxCommandEvent& event)
{
    wxString name = GenerateSubModelName("SubModel");

    SubModelInfo* sm = new SubModelInfo(name);
    sm->vertical = false;
    sm->strands.clear();
    sm->strands.push_back("");
    sm->isRanges = true;
    _subModels.push_back(sm);

    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm);
    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsDialog::OnDeleteButtonClick(wxCommandEvent& event)
{
    int firstindex = -1;
    wxString names = GetSelectedNames();
    wxStringTokenizer tokenizer(names, ",");
    wxString msg = "";
    int count = tokenizer.CountTokens();
    if ( count > 1) {
        msg.Printf(wxT("Are you sure you want to delete %d sub models? \n\nCurrent open sequence effects will be removed as well. "), count);
    } else {
        msg = "Are you sure you want to delete sub model " + names + "? \n\nCurrent open sequence effects will be removed as well.";
    }
    int answer = wxMessageBox(msg,
                          "Delete Model",
                          wxYES_NO, this);
    // no: return
    if (answer == wxNO) {
        return;
    }

    // delete selected submodel
    while (tokenizer.HasMoreTokens()) {
        wxString token = tokenizer.GetNextToken();
        int index = GetSubModelInfoIndex(token);
        if (firstindex == -1) firstindex = index - 1;
        ListCtrl_SubModels->DeleteItem(index);
        RemoveSubModelFromList(token);
    }

    // Select the first model prior to the first deleted model
    if (firstindex < 0) firstindex = 0;
    if (ListCtrl_SubModels->GetItemCount() > 0)
    {
        Select(ListCtrl_SubModels->GetItemText(firstindex));
    }

    ValidateWindow();
}

void SubModelsDialog::OnNodesGridCellChange(wxGridEvent& event)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int r = event.GetRow();
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());
    if (sm != nullptr)
    {
        int str = (int)sm->strands.size() - 1 - r;
        if (str < 0)
        {
            logger_base.crit("SubModelsDialog::OnNodesGridCellChange submodel '%s' tried to access strand %d. This should have crashed.", (const char*)GetSelectedName().c_str(), str);
            wxASSERT(false);
        }
        else
        {
            sm->strands[str] = NodesGrid->GetCellValue(r, 0);
        }
    }
    else
    {
        logger_base.crit("SubModelsDialog::OnNodesGridCellChange submodel '%s' ... not found. This should have crashed.", (const char*)GetSelectedName().c_str());
        wxASSERT(false);
    }
    SelectRow(r);
    ValidateWindow();
}

void SubModelsDialog::OnNodesGridCellSelect(wxGridEvent& event)
{
    SelectRow(event.GetRow());
    ValidateWindow();
}

void SubModelsDialog::OnLayoutCheckboxClick(wxCommandEvent& event)
{
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());
    if (sm != nullptr)
    {
        sm->vertical = LayoutCheckbox->GetValue();
    }
}

void SubModelsDialog::OnTypeNotebookPageChanged(wxBookCtrlEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);
    if (sm != nullptr)
    {
        sm->isRanges = TypeNotebook->GetSelection() == 0;
    }
    Select(name);
}

void SubModelsDialog::OnSubBufferRangeChange(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);
    sm->isRanges = false;
    sm->subBuffer = event.GetString();
    DisplayRange(sm->subBuffer);
}


void SubModelsDialog::OnNodesGridLabelLeftClick(wxGridEvent& event)
{
    SelectRow(event.GetRow());
    if (event.GetRow() != -1) {
        NodesGrid->GoToCell(event.GetRow(), 0);
    }
}

void SubModelsDialog::OnButton_GenerateClick(wxCommandEvent& event)
{
    SubModelGenerateDialog dialog(this, model->GetDefaultBufferWi(), model->GetDefaultBufferHt(), model->GetNodeCount());

    if (dialog.ShowModal() == wxID_OK)
    {
        int last = 0;
        for (int i = 0; i < dialog.GetCount(); i++)
        {
            wxString basename = wxString(Model::SafeModelName(dialog.GetBaseName().ToStdString()));
            wxString name = GenerateSubModelName(basename);

            if (GetSubModelInfoIndex(name) != -1)
            {
                // this name clashes ... so I cant create it
            }
            else
            {
                SubModelInfo* sm = new SubModelInfo(name);
                sm->vertical = false;
                sm->strands.clear();
                sm->strands.push_back("");

                if (dialog.GetType() == "Vertical Slices")
                {
                    GenerateSegment(sm, 1, i, false, dialog.GetCount());
                }
                else if (dialog.GetType() == "Horizontal Slices")
                {
                    GenerateSegment(sm, 1, i, true, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 2 Wide")
                {
                    GenerateSegment(sm, 2, i, true, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 2 High")
                {
                    GenerateSegment(sm, 2, i, false, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 3 Wide")
                {
                    GenerateSegment(sm, 3, i, true, dialog.GetCount());
                }
                else if (dialog.GetType() == "Segments 3 High")
                {
                    GenerateSegment(sm, 3, i, false, dialog.GetCount());
                }
                else if (dialog.GetType() == "Nodes")
                {
                    sm->isRanges = true;
                    float per = (float)model->GetNodeCount() / (float)dialog.GetCount();
                    int start = last + 1;
                    int end = (i+1) * per;

                    if (i == dialog.GetCount() - 1)
                    {
                        end = model->GetNodeCount();
                    }

                    last = end;
                    if (start == end)
                    {
                        sm->strands[0] = wxString::Format("%i", start);
                    }
                    else
                    {
                        sm->strands[0] = wxString::Format("%i-%i", start, end);
                    }
                }
                _subModels.push_back(sm);
                PopulateList();
                ValidateWindow();
                //Select(name);
            }
        }
    }
}

wxString SubModelsDialog::ReverseRow(wxString row)
{
    wxString newStrand = "";
    auto nodes = wxSplit(row, ',');
    for (auto nit = nodes.rbegin(); nit != nodes.rend(); ++nit)
    {
        if (nit != nodes.rbegin()) newStrand += ",";
        if (nit->Contains('-'))
        {
            auto range = wxSplit(*nit, '-');
            if (range.size() == 2)
            {
                newStrand += range[1] + "-" + range[0];
            }
            else
            {
                // not valid so just copy
                newStrand += *nit;
            }
        }
        else
        {
            newStrand += *nit;
        }
    }

    return newStrand;
}

void SubModelsDialog::OnButton_ReverseNodesClick(wxCommandEvent& event)
{
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());

    if (sm->isRanges)
    {
        std::vector<wxString> newStrands;
        for (auto it = sm->strands.begin(); it != sm->strands.end(); ++it)
        {
            *it = ReverseRow(*it);
        }
        Select(GetSelectedName());
    }
}

void SubModelsDialog::OnListCtrl_SubModelsItemSelect(wxListEvent& event)
{
    if (ListCtrl_SubModels->GetSelectedItemCount() == 1)
    {
        Select(GetSelectedName());
    }
}

void SubModelsDialog::OnListCtrl_SubModelsColumnClick(wxListEvent& event)
{
}

void SubModelsDialog::OnListCtrl_SubModelsKeyDown(wxListEvent& event)
{
    auto key = event.GetKeyCode();
    if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
    {
        OnDeleteButtonClick(event);
    }
    if (key == WXK_DOWN || key == WXK_NUMPAD_DOWN) {
        int idx = GetSubModelInfoIndex(GetSelectedName());
        if (idx + 1 < _subModels.size()){
            UnSelectAll();
            Select(_subModels[idx+1]->name);
        }
    }
    if (key == WXK_UP || key == WXK_NUMPAD_UP) {
        int idx = GetSubModelInfoIndex(GetSelectedName());
        if (idx - 1 >= 0){
            UnSelectAll();
            Select(_subModels[idx-1]->name);
        }
    }
    ValidateWindow();
}

void SubModelsDialog::ApplySubmodelName()
{
    int index = GetSelectedIndex();
    wxASSERT(index >= 0);

    wxString name = wxString(Model::SafeModelName(TextCtrl_Name->GetValue().ToStdString()));

    if (name.IsEmpty())
    {
        TextCtrl_Name->SetBackgroundColour(*wxRED);
        return;
    }

    if (name != TextCtrl_Name->GetValue())
    {
        TextCtrl_Name->SetValue(name);
    }

    SubModelInfo* sm = (SubModelInfo*)ListCtrl_SubModels->GetItemData(index);

    bool clash = false;
    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++)
    {
        if (index != i && ListCtrl_SubModels->GetItemText(i) == name)
        {
            clash = true;
            break;
        }
    }

    if (!clash)
    {
        sm->name = name;
        ListCtrl_SubModels->SetItemText(index, name);
    }

    ValidateWindow();
}

void SubModelsDialog::OnTextCtrl_NameText_KillFocus(wxFocusEvent& event)
{
    ApplySubmodelName();
    event.Skip();
}

void SubModelsDialog::OnTextCtrl_NameText_Change(wxCommandEvent& event)
{
    //ApplySubmodelName();
}
#pragma endregion actions

void SubModelsDialog::PopulateList()
{
    ListCtrl_SubModels->Freeze();
    ListCtrl_SubModels->DeleteAllItems();

    for (int x = 0; x < _subModels.size(); x++) {
        ListCtrl_SubModels->InsertItem(x, _subModels[x]->name);
        ListCtrl_SubModels->SetItemPtrData(x, (wxUIntPtr)_subModels[x]);
    }

    ListCtrl_SubModels->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (ListCtrl_SubModels->GetColumnWidth(0) < 100) {
        ListCtrl_SubModels->SetColumnWidth(0, 100);
    }

    ListCtrl_SubModels->Thaw();
    ListCtrl_SubModels->Refresh();
}

void SubModelsDialog::ValidateWindow()
{
    if (_isMatrix)
    {
        Button_importCustom->Enable();
    }
    else
    {
        Button_importCustom->Enable(false);
    }

    if (ListCtrl_SubModels->GetItemCount() <= 0)
    {
        ListCtrl_SubModels->Disable();
        TextCtrl_Name->Disable();
        DeleteButton->Disable();
        ButtonCopy->Disable();
        NodesGrid->Disable();
        LayoutCheckbox->Disable();
        AddRowButton->Disable();
        DeleteRowButton->Disable();
        subBufferPanel->Disable();
        TypeNotebook->Disable();
        Button_ExportCustom->Disable();
    } else {
        ListCtrl_SubModels->Enable();
        Button_ExportCustom->Enable();
    }

    if (ListCtrl_SubModels->GetSelectedItemCount() > 0)
    {
        DeleteButton->Enable();
        ButtonCopy->Enable();
        NodesGrid->Enable();
        LayoutCheckbox->Enable();
        AddRowButton->Enable();
        if (NodesGrid->GetGridCursorRow() >= 0)
        {
            DeleteRowButton->Enable();
            if (NodesGrid->GetNumberRows() == 1)
            {
                Button_MoveUp->Disable();
                Button_MoveDown->Disable();
            }
            else
            {
                //if (NodesGrid->GetGridCursorRow() == 0)
                //{
                //    Button_MoveUp->Disable();
                //}
                //else
                {
                    Button_MoveUp->Enable();
                }
                //if (NodesGrid->GetGridCursorRow() == NodesGrid->GetNumberRows() - 1)
                //{
                //    Button_MoveDown->Disable();
                //}
                //else
                {
                    Button_MoveDown->Enable();
                }
            }
            Button_ReverseRow->Enable();
        }
        else
        {
            DeleteRowButton->Disable();
            Button_MoveUp->Disable();
            Button_MoveDown->Disable();
            Button_ReverseRow->Disable();
        }

        if (NodesGrid->GetNumberRows() == 1)
        {
            Button_ReverseRows->Disable();
        }
        else
        {
            Button_ReverseRows->Enable();
        }

        subBufferPanel->Enable();
        if (ListCtrl_SubModels->GetSelectedItemCount() == 1)
        {
            TypeNotebook->Enable();
            TextCtrl_Name->Enable();

            int index = GetSelectedIndex();
            bool clash = false;
            for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++)
            {
                if (index != i && ListCtrl_SubModels->GetItemText(i) == TextCtrl_Name->GetValue())
                {
                    clash = true;
                    break;
                }
            }

            if (!clash)
            {
                TextCtrl_Name->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
            else
            {
                TextCtrl_Name->SetBackgroundColour(*wxRED);
            }
        }
        else
        {
            TypeNotebook->Disable();
            TextCtrl_Name->Disable();
        }
    } else {
        TextCtrl_Name->Disable();
        DeleteButton->Disable();
        ButtonCopy->Disable();
        NodesGrid->Disable();
        LayoutCheckbox->Disable();
        AddRowButton->Disable();
        DeleteRowButton->Disable();
        subBufferPanel->Disable();
        TypeNotebook->Disable();
    }
}

void SubModelsDialog::UnSelectAll()
{
    for(int i=0; i < ListCtrl_SubModels->GetItemCount(); ++i) {
        ListCtrl_SubModels->SetItemState(i, 0, wxLIST_STATE_SELECTED);
    }
}

void SubModelsDialog::Select(const wxString &name)
{
    if (name == "") { return; }

    SubModelInfo* sm = GetSubModelInfo(name);

    int idx = GetSubModelInfoIndex(name);

    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++)
    {
        ListCtrl_SubModels->SetItemState(i, (i == idx) ? wxLIST_STATE_SELECTED : 0, wxLIST_STATE_SELECTED);
    }

    ListCtrl_SubModels->EnsureVisible(idx);
    TextCtrl_Name->SetValue(name);

    if (sm->isRanges) {
        TypeNotebook->SetSelection(0);
        LayoutCheckbox->SetValue(sm->vertical);
        NodesGrid->BeginBatch();
        if (NodesGrid->GetNumberRows() > 0) {
            NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
        }
        for (int x = sm->strands.size() - 1; x >= 0; x--) {
            int cellrow = (sm->strands.size() - 1) - x;
            NodesGrid->AppendRows(1);
            if (x == 0)
            {
                NodesGrid->SetRowLabelValue(cellrow, "Bottom");
            }
            else if (x == sm->strands.size() - 1)
            {
                NodesGrid->SetRowLabelValue(cellrow, "Top");
            }
            else
            {
                NodesGrid->SetRowLabelValue(cellrow, wxString::Format("Line %d", (x + 1)));
            }
            NodesGrid->SetCellValue(cellrow, 0, sm->strands[x]);
        }
        NodesGrid->EndBatch();
        NodesGrid->GoToCell(0, 0);
        SelectRow(0);
    } else {
        TypeNotebook->SetSelection(1);
        NodesGrid->BeginBatch();
        if (NodesGrid->GetNumberRows() > 0) {
            NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
        }
        NodesGrid->AppendRows(1); // we always need one row
        subBufferPanel->SetValue(sm->subBuffer.ToStdString());
        DisplayRange(sm->subBuffer);
    }
    ValidateWindow();
}

void SubModelsDialog::SelectAll(const wxString &names) {
    if (names == "") { return; }
    wxStringTokenizer tokenizer(names, ",");
    while (tokenizer.HasMoreTokens()) {
        int idx = GetSubModelInfoIndex(tokenizer.GetNextToken());
        for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++)
        {
            if (i==idx) {
                ListCtrl_SubModels->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
    }
    ValidateWindow();
}

void SubModelsDialog::DisplayRange(const wxString &range)
{
    float x1 = 0;
    float x2 = 100;
    float y1 = 0;
    float y2 = 100;
    if (range != "") {
        wxArrayString v = wxSplit(range, 'x');
        x1 = v.size() > 0 ? wxAtof(v[0]) : 0.0;
        y1 = v.size() > 1 ? wxAtof(v[1]) : 0.0;
        x2 = v.size() > 2 ? wxAtof(v[2]) : 100.0;
        y2 = v.size() > 3 ? wxAtof(v[3]) : 100.0;
    }

    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    x1 *= (float) model->GetDefaultBufferWi();
    x2 *= (float) model->GetDefaultBufferWi();
    y1 *= (float) model->GetDefaultBufferHt();
    y2 *= (float) model->GetDefaultBufferHt();
    x1 /= 100.0;
    x2 /= 100.0;
    y1 /= 100.0;
    y2 /= 100.0;
    x1 = std::ceil(x1);
    y1 = std::ceil(y1);

    int nn = model->GetNodeCount();
    xlColor c(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        if (model->IsNodeInBufferRange(node, x1, y1, x2, y2)) {
            model->SetNodeColor(node, xlWHITE);
        } else {
            model->SetNodeColor(node, c);
        }
    }
    model->DisplayEffectOnWindow(modelPreview, 2);
}

void SubModelsDialog::SelectRow(int r)
{
    int nn = model->GetNodeCount();
    xlColor c(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        model->SetNodeColor(node, c);
    }

    int start = r;
    int end = r;
    if (r == -1) {
        start = 0;
        end = NodesGrid->GetNumberRows() - 1;
    }
    for (int cur = start; cur <= end; cur++) {
        wxString v = NodesGrid->GetCellValue(cur, 0);
        wxStringTokenizer wtkz(v, ",");
        while (wtkz.HasMoreTokens()) {
            wxString valstr = wtkz.GetNextToken();

            int start2, end2;
            if (valstr.Contains("-")) {
                int idx = valstr.Index('-');
                start2 = wxAtoi(valstr.Left(idx));
                end2 = wxAtoi(valstr.Right(valstr.size() - idx - 1));
            } else {
                start2 = end2 = wxAtoi(valstr);
            }
            start2--;
            end2--;
            bool done = false;
            int n = start2;
            while (!done) {
                if (n >= 0 && n < (int)model->GetNodeCount()) {
                    model->SetNodeColor(n, xlWHITE);
                }
                if (start2 > end2) {
                    n--;
                    done = n < end2;
                } else {
                    n++;
                    done = n > end2;
                }
            }
        }
    }
    model->DisplayEffectOnWindow(modelPreview, 2);
}

// Value must be 0-1
// When zero then we will always get generated segments slightly overlapping
// When one then they should never overlap but you may also get some gaps
#define GENERATE_GAP 0.25

void SubModelsDialog::GenerateSegment(SubModelsDialog::SubModelInfo* sm, int segments, int segment, bool horizontal, int count)
{
    if (horizontal)
    {
        float perx = 100.0 / segments;
        int offset = segment % segments;
        float startx = offset * perx;
        float endx = startx + perx - GENERATE_GAP;
        if ((segment + 1) % segments == 0) endx = 100;

        float per = 100.0 / (count / segments);
        float start = segment / segments * per;
        float end = start + per - GENERATE_GAP;

        if ((segment + 1) / segments == count / segments) end = 100;

        sm->isRanges = false;
        sm->subBuffer = wxString::Format("%.2fx%.2fx%.2fx%.2f", startx, start, endx, end);
    }
    else
    {
        float pery = 100.0 / segments;
        int offset = segment % segments;
        float starty = offset * pery;
        float endy = starty + pery - GENERATE_GAP;
        if ((segment + 1) % segments == 0) endy = 100;

        float per = 100.0 / (count / segments);
        float start = segment / segments * per;
        float end = start + per - GENERATE_GAP;

        if ((segment + 1) / segments == count /segments) end = 100;

        sm->isRanges = false;
        sm->subBuffer = wxString::Format("%.2fx%.2fx%.2fx%.2f", start, starty, end, endy);
    }
}

void SubModelsDialog::MoveSelectedModelsTo(int indexTo)
{
    if (indexTo < 0) { return; }
    wxString names = GetSelectedNames();
    wxStringTokenizer tokenizer(names, ",");
    while (tokenizer.HasMoreTokens()) {
        wxString token = tokenizer.GetNextToken();
        int index = GetSubModelInfoIndex(token);

        SubModelInfo* sm = _subModels.at(index);
        _subModels.erase(_subModels.begin() + GetSubModelInfoIndex(sm->name));
        ListCtrl_SubModels->DeleteItem(index);
        if (indexTo >= _subModels.size()) {
            _subModels.push_back(sm);
            long idx = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
            ListCtrl_SubModels->SetItemPtrData(idx, (wxUIntPtr)sm);

        } else {
            _subModels.insert(_subModels.begin()+indexTo, sm);
            ListCtrl_SubModels->InsertItem(indexTo, sm->name);
            ListCtrl_SubModels->SetItemPtrData(indexTo, (wxUIntPtr)sm);
        }
        ++indexTo;
    }
    tokenizer.Reinit(names);
    if (tokenizer.CountTokens() > 1) {
        SelectAll(names);
    } else {
        wxString first = tokenizer.GetNextToken();
        Select(first);
    }
}

#pragma region Drag and Drop
void SubModelsDialog::OnListCtrl_SubModelsBeginDrag(wxListEvent& event)
{
    if (ListCtrl_SubModels->GetSelectedItemCount() == 0) return;

    wxString drag = "SubModel";
    for (size_t i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (IsItemSelected(ListCtrl_SubModels, i))
        {
            drag += "," + ListCtrl_SubModels->GetItemText(i);
        }
    }

    wxTextDataObject my_data(drag);
    wxDropSource dragSource(this);
    dragSource.SetData(my_data);
    dragSource.DoDragDrop(wxDrag_DefaultMove);
    SetCursor(wxCURSOR_ARROW);
}

void SubModelsDialog::OnDrop(wxCommandEvent& event)
{
    wxArrayString parms = wxSplit(event.GetString(), ',');
    int x = event.GetExtraLong() >> 16;
    int y = event.GetExtraLong() & 0xFFFF;

    switch(event.GetInt())
    {
        case 0:
        {
            // Model dropped into models (a reorder)
            int flags = wxLIST_HITTEST_ONITEM;
            long index = ListCtrl_SubModels->HitTest(wxPoint(x, y), flags, nullptr);
            MoveSelectedModelsTo(index);
            break;
        }
        default:
            break;
    }
}

wxDragResult SubModelTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    static int MINSCROLLDELAY = 10;
    static int STARTSCROLLDELAY = 300;
    static int scrollDelay = STARTSCROLLDELAY;
    static wxLongLong lastTime = wxGetUTCTimeMillis();

    if (wxGetUTCTimeMillis() - lastTime < scrollDelay)
    {
        // too soon to scroll again
    }
    else
    {
        if (_type == "SubModel" && _list->GetItemCount() > 0)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            int lastItem = _list->HitTest(wxPoint(x, y), flags, nullptr);

            for (int i = 0; i < _list->GetItemCount(); ++i)
            {
                if (i == lastItem)
                {
                    _list->SetItemState(i, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
                }
                else
                {
                    _list->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
                }
            }

            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize)
            {
                // scroll up
                if (_list->GetTopItem() > 0)
                {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem()-1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                // scroll down
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    _list->EnsureVisible(lastItem+1);
                    lastTime = wxGetUTCTimeMillis();
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else
            {
                scrollDelay = STARTSCROLLDELAY;
            }
        }
    }

    return wxDragMove;
}

bool SubModelTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_SMDROP);
    event.SetString(data); // this is the dropped string
    event.SetExtraLong(mousePos); // this is the mouse position packed into a long

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "SubModel")
    {
        if (_type == "SubModel")
        {
            event.SetInt(0);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}
#pragma endregion Drag and Drop

#pragma region Row Actions
void SubModelsDialog::OnButton_ReverseRowsClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    if (sm->strands.size() == 1)
    {
        // nothing to do
        return;
    }

    std::list<std::string> reordered;
    for (auto it = sm->strands.begin(); it != sm->strands.end(); ++it)
    {
        reordered.push_front(*it);
    }

    int i = 0;
    for (auto it = reordered.begin(); it != reordered.end(); ++it)
    {
        sm->strands[i] = *it;
        i++;
    }

    Select(GetSelectedName());
}

void SubModelsDialog::OnButton_ReverseRowClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    sm->strands[sm->strands.size() - 1 - row] = ReverseRow(sm->strands[sm->strands.size() - 1 - row]);

    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsDialog::OnButton_MoveDownClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    if (NodesGrid->GetNumberRows() == 1 || row == sm->strands.size() - 1 || row < 0)
    {
        // nothing to do
    }
    else
    {
        std::string swap = sm->strands[sm->strands.size() - 1 - row];
        sm->strands[sm->strands.size() - 1 - row] = sm->strands[sm->strands.size() - 1 - (row + 1)];
        sm->strands[sm->strands.size() - 1 - (row + 1)] = swap;
        Select(GetSelectedName());
        NodesGrid->SetGridCursor(row + 1, 0);
        SelectRow(row + 1);
        Panel3->SetFocus();
        NodesGrid->SetFocus();
    }
    ValidateWindow();
}

void SubModelsDialog::OnButton_MoveUpClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    if (NodesGrid->GetNumberRows() == 1 || row <= 0)
    {
        // nothing to do
    }
    else
    {
        std::string swap = sm->strands[sm->strands.size() - 1 - row];
        sm->strands[sm->strands.size() - 1 - row] = sm->strands[sm->strands.size() - 1 - (row - 1)];
        sm->strands[sm->strands.size() - 1 - (row - 1)] = swap;
        Select(GetSelectedName());
        NodesGrid->SetGridCursor(row - 1, 0);
        SelectRow(row - 1);
        Panel3->SetFocus();
        NodesGrid->SetFocus();
    }
    ValidateWindow();
}

void SubModelsDialog::OnAddRowButtonClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    sm->strands.insert(sm->strands.begin(), "");
    Select(GetSelectedName());
    SelectRow(NodesGrid->GetNumberRows() - 1);
    NodesGrid->SetGridCursor(NodesGrid->GetNumberRows() - 1, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    ValidateWindow();
}

void SubModelsDialog::OnDeleteRowButtonClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);

    if (NodesGrid->GetNumberRows() == 1)
    {
        sm->strands.front() = "";
        NodesGrid->SetCellValue(0, 0, "");
        Select(GetSelectedName());
        NodesGrid->SetGridCursor(0, 0);
        Panel3->SetFocus();
        NodesGrid->SetFocus();
        SelectRow(0);
    }
    else
    {
        int row = NodesGrid->GetGridCursorRow();
        int sz = sm->strands.size();
        sm->strands.erase(sm->strands.begin() + sm->strands.size() - 1 - row);
        sm->strands.resize(sz - 1);
        --row;
        Select(GetSelectedName());
        NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
        Panel3->SetFocus();
        NodesGrid->SetFocus();
        SelectRow(row >= 0 ? row : 0);
    }
    ValidateWindow();
}

#pragma endregion

void SubModelsDialog::OnButton_Sub_ImportClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xmodel files (*.xmodel)|*.xmodel", wxFD_OPEN);
    if (filename.IsEmpty()) return;
    ImportSubModel(filename);
}

//Import SubModel From xModel File
void SubModelsDialog::ImportSubModel(std::string filename)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();
        ReadSubModelXML(root);
    }
    else
    {
        DisplayError("Failure loading xModel file.");
    }
}

void SubModelsDialog::ReadSubModelXML(wxXmlNode* xmlData)
{
    bool overRide = false;
    bool showDialog = true;
    wxXmlNode * child = xmlData->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == "subModel") {
            wxString name = child->GetAttribute("name");

            SubModelInfo *sm = new SubModelInfo(name);
            sm->name = name;
            sm->oldName = name;
            sm->isRanges = child->GetAttribute("type", "ranges") == "ranges";
            sm->vertical = child->GetAttribute("layout") == "vertical";
            sm->subBuffer = child->GetAttribute("subBuffer");
            sm->strands.resize(1);
            sm->strands[0] = "";
            int x = 0;
            while (child->HasAttribute(wxString::Format("line%d", x))) {
                if (x >= sm->strands.size()) {
                    sm->strands.resize(x + 1);
                }
                sm->strands[x] = child->GetAttribute(wxString::Format("line%d", x));
                x++;
            }

            //cannot have duplicate submodels names, what to do?
            if (GetSubModelInfoIndex(name) != -1)
            {
                //Are the submodels The Same?
                SubModelInfo *prevSm = GetSubModelInfo(name);
                if (*sm == *prevSm) //skip if exactly the same
                {
                    child = child->GetNext();
                    continue;
                }
                //Ask User what to do if different
                if(showDialog)
                {
                    wxMessageDialog confirm(this, _("SubModel(s) with the Same Name Already Exist.\n Would you Like to Override Them ALL?"), _("Override SubModels"), wxYES_NO);
                    int returnCode = confirm.ShowModal();
                    if(returnCode == wxID_YES)
                        overRide = true;
                     showDialog = false;
                }
                if (overRide)
                {
                    RemoveSubModelFromList(name);
                }
                else
                {
                    //rename and add if not override
                    sm->oldName = sm->name = GenerateSubModelName(name);
                }
            }
            _subModels.push_back(sm);
        }
        child = child->GetNext();
    }
    PopulateList();
    ValidateWindow();
}

//Import Submodels from another Model
void SubModelsDialog::OnButtonCopyModelClick(wxCommandEvent& event)
{
    xLightsFrame* xlights = xLightsApp::GetFrame();

    wxArrayString choices = getModelList(&xlights->AllModels);

    wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);

    if (dlg.ShowModal() == wxID_OK)
    {
        Model *m = xlights->GetModel(dlg.GetStringSelection());
        ReadSubModelXML(m->GetModelXml());
    }
}

//Copy Selected SubModel
void SubModelsDialog::OnButton_Sub_CopyClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "")
        return;
    SubModelInfo* sm = new SubModelInfo(*GetSubModelInfo(name));

    name = GenerateSubModelName(name);

    sm->name = name;
    sm->oldName = name;
    _subModels.push_back(sm);

    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm);
    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

wxArrayString SubModelsDialog::getModelList(ModelManager* modelManager)
{
    wxArrayString choices;
    for (auto it = modelManager->begin(); it != modelManager->end(); ++it)
    {
        Model* m = it->second;
        if (m->Name() == model->Name())//Skip Current Model
            continue;
        choices.Add(m->Name());
    }
    return choices;
}

void SubModelsDialog::OnButton_Draw_ModelClick(wxCommandEvent& event)
{
    const wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    NodeSelectGrid dialog(true, name, model, sm->strands, this);

    if (dialog.ShowModal() == wxID_OK)
    {
        sm->strands = dialog.GetRowData();

        Select(GetSelectedName());
        dialog.Close();

        Panel3->SetFocus();
        NodesGrid->SetFocus();

        ValidateWindow();
    }
}

void SubModelsDialog::OnNodesGridLabelLeftDClick(wxGridEvent& event)
{
    OnNodesGridCellLeftDClick(event);
}

void SubModelsDialog::OnNodesGridCellLeftDClick(wxGridEvent& event)
{
    int row = event.GetRow();
    const wxString name = GetSelectedName();
    if (name == "" || row == -1) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    const wxString title = name + " - " + NodesGrid->GetRowLabelValue(row);
    NodeSelectGrid dialog(false, title, model, sm->strands[sm->strands.size() - 1 - row], this);

    if (dialog.ShowModal() == wxID_OK)
    {
        sm->strands[sm->strands.size() - 1 - row] = dialog.GetNodeList(false);

        Select(GetSelectedName());
        dialog.Close();

        NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
        Panel3->SetFocus();
        NodesGrid->SetFocus();
        SelectRow(row >= 0 ? row : 0);

        ValidateWindow();
    }
}

void SubModelsDialog::OnButton_importCustomClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xmodel files (*.xmodel)|*.xmodel", wxFD_OPEN);
    if (filename.IsEmpty()) return;
    ImportCustomModel(filename);
}

void SubModelsDialog::FixNodes(wxXmlNode* n, const std::string& attribute, std::map<int, int>& nodeMap)
{
    auto l = n->GetAttribute(attribute, "");
    n->DeleteAttribute(attribute);

    wxString row = "";

    auto ranges = wxSplit(l, ',');
    for (auto r : ranges)
    {
        if (r == "")
        {
            row += ",";
        }
        else if (r.Contains("-"))
        {
            auto rg = wxSplit(r, '-');
            if (rg.size() == 2)
            {
                int first = wxAtoi(rg[0]);
                int last = wxAtoi(rg[1]);
                if (first <= last)
                {
                    for (int i = first; i <= last; i++)
                    {
                        row += wxString::Format("%d,", nodeMap[i]);
                    }
                }
                else
                {
                    for (int i = first; i >= last; i--)
                    {
                        row += wxString::Format("%d,", nodeMap[i]);
                    }
                }
            }
        }
        else
        {
            int rr = wxAtoi(r);
            row += wxString::Format("%d,", nodeMap[rr]);
        }
    }
    n->AddAttribute(attribute, row);
}

void SubModelsDialog::ImportCustomModel(std::string filename)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        // it must be a 1 depth custom model or empty from pre 3D models
        if (root->GetName() == "custommodel" && (root->GetAttribute("Depth", "1") == "1" || root->GetAttribute("Depth", "").IsEmpty()))
        {
            int width = wxAtoi(root->GetAttribute("parm1", "1"));
            int height = wxAtoi(root->GetAttribute("parm2", "1"));
            int modelw = model->GetDefaultBufferWi();
            int modelh = model->GetDefaultBufferHt();

            if (modelw < width || modelh < height)
            {
                wxMessageBox("Model is too small for the custom model.");
            }
            else
            {
                AlignmentDialog dlg(this);

                if (dlg.ShowModal() == wxID_OK)
                {
                    int xStart = 1;
                    if (dlg.GetX() == AlignmentDialog::Alignment::CENTRE)
                    {
                        xStart = (float)modelw / 2.0 - (float)width / 2.0 + 1.0;
                    }
                    else if (dlg.GetX() == AlignmentDialog::Alignment::RIGHT)
                    {
                        xStart = modelw - width + 1;
                    }

                    int yStart = 0;
                    if (dlg.GetY() == AlignmentDialog::Alignment::TOP)
                    {
                        yStart = modelh - height;
                    }
                    else if (dlg.GetY() == AlignmentDialog::Alignment::MIDDLE)
                    {
                        yStart = (float)modelh / 2.0 - (float)height / 2.0;
                    }

                    std::map<int, int> nodeMap;
                    wxString name = GenerateSubModelName(root->GetAttribute("name"));
                    SubModelInfo* sm = new SubModelInfo(name);
                    sm->vertical = false;
                    sm->strands.clear();
                    sm->isRanges = true;

                    auto data = root->GetAttribute("CustomModel", "");
                    auto rows = wxSplit(data, ';');
                    int rnum = yStart;
                    for (auto r = rows.rbegin(); r != rows.rend(); ++r)
                    {
                        auto cols = wxSplit(*r, ',');
                        wxString row = "";
                        int cnum = xStart;
                        for (auto c : cols)
                        {
                            if (c == "")
                            {
                                row += ",";
                            }
                            else
                            {
                                int nn = model->GetNodeNumber(rnum, cnum);
                                row += wxString::Format("%d", nn);
                                nodeMap[wxAtoi(c)] = nn;
                            }
                            cnum++;
                        }
                        sm->strands.push_back(row);
                        rnum++;
                    }
                    _subModels.push_back(sm);
                    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
                    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm);

                    Panel3->SetFocus();
                    TextCtrl_Name->SetFocus();
                    TextCtrl_Name->SelectAll();

                    for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
                    {
                        if (n->GetName() == "subModel")
                        {
                            auto smname = n->GetAttribute("name");
                            SubModelInfo* sm2 = new SubModelInfo(name + "-" + smname);
                            sm2->vertical = n->GetAttribute("layout", "horizontal") == "vertical";
                            sm2->strands.clear();
                            sm2->isRanges = n->GetAttribute("type", "") == "ranges";
                            if (sm2->isRanges)
                            {
                                wxString row = "";
                                int line = 0;
                                while (n->HasAttribute(wxString::Format("line%d", line)))
                                {
                                    auto l = n->GetAttribute(wxString::Format("line%d", line), "");
                                    auto ranges = wxSplit(l, ',');

                                    for (auto r : ranges)
                                    {
                                        if (r == "")
                                        {
                                            row += ",";
                                        }
                                        else if (r.Contains("-"))
                                        {
                                            auto rg = wxSplit(r, '-');
                                            if (rg.size() == 2)
                                            {
                                                int first = wxAtoi(rg[0]);
                                                int last = wxAtoi(rg[1]);
                                                if (first <= last)
                                                {
                                                    for (int i = first; i <= last; i++)
                                                    {
                                                        row += wxString::Format("%d,", nodeMap[i]);
                                                    }
                                                }
                                                else
                                                {
                                                    for (int i = first; i >= last; i--)
                                                    {
                                                        row += wxString::Format("%d,", nodeMap[i]);
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            int rr = wxAtoi(r);
                                            row += wxString::Format("%d,", nodeMap[rr]);
                                        }
                                    }
                                    sm2->strands.push_back(row);
                                    line++;
                                }

                                _subModels.push_back(sm2);
                                index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm2->name);
                                ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm2);
                            }
                            else
                            {
                                // we only bring in ranges
                                delete sm2;
                            }
                        }
                        else if (n->GetName() == "faceInfo")
                        {
                            // Fix Me
                            if (n->GetAttribute("Type") == "NodeRange")
                            {
                                FixNodes(n, "Eyes-Closed", nodeMap);
                                FixNodes(n, "Eyes-Open", nodeMap);
                                FixNodes(n, "Eyes2-Closed", nodeMap);
                                FixNodes(n, "Eyes2-Open", nodeMap);
                                FixNodes(n, "Eyes3-Closed", nodeMap);
                                FixNodes(n, "Eyes3-Open", nodeMap);
                                FixNodes(n, "FaceOutline", nodeMap);
                                FixNodes(n, "FaceOutline2", nodeMap);
                                FixNodes(n, "Mouth-AI", nodeMap);
                                FixNodes(n, "Mouth-E", nodeMap);
                                FixNodes(n, "Mouth-FV", nodeMap);
                                FixNodes(n, "Mouth-O", nodeMap);
                                FixNodes(n, "Mouth-U", nodeMap);
                                FixNodes(n, "Mouth-L", nodeMap);
                                FixNodes(n, "Mouth-MBP", nodeMap);
                                FixNodes(n, "Mouth-WQ", nodeMap);
                                FixNodes(n, "Mouth-etc", nodeMap);
                                FixNodes(n, "Mouth-rest", nodeMap);

                                auto fname = n->GetAttribute("Name");
                                auto basefname = fname;

                                int suffix = 1;
                                while (model->faceInfo.find(fname) != model->faceInfo.end())
                                {
                                    fname = wxString::Format("%s-%d", basefname, suffix);
                                    suffix++;
                                }

                                model->AddFace(n);
                            }
                            else
                            {
                                // We dont handle non node range faces
                            }
                        }
                        else if (n->GetName() == "stateInfo")
                        {
                            if (n->GetAttribute("Type") == "NodeRange")
                            {
                                int i = 1;
                                while (n->HasAttribute(wxString::Format("s%d", i)))
                                {
                                    FixNodes(n, wxString::Format("s%d", i), nodeMap);
                                    i++;
                                }

                                auto sname = n->GetAttribute("Name");
                                auto basesname = sname;

                                int suffix = 1;
                                while (model->stateInfo.find(sname) != model->stateInfo.end())
                                {
                                    sname = wxString::Format("%s-%d", basesname, suffix);
                                    suffix++;
                                }

                                model->AddState(n);
                            }
                            else
                            {
                                // We dont handle non node range states
                            }
                        }
                    }
                    ValidateWindow();
                    Select(name);
                }
            }
        }
        else
        {
            wxMessageBox("Model is either not custom or has a depth that is not 1.");
        }
    }
    else
    {
        DisplayError("Failure loading xModel file.");
    }
    ValidateWindow();
}

void SubModelsDialog::OnButton_ExportCustomClick(wxCommandEvent& event)
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Save CSV File"), wxEmptyString, model->Name(), ".csv", "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    ExportSubModels(filename);
}

void SubModelsDialog::ExportSubModels(wxString const& filename)
{
    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString const header = "Name,Type,Vertical Buffer,Rows Name,Node Ranges\n";
    f.Write(header);

    for (auto sm : _subModels) {
        f.Write(sm->name + ",");
        f.Write((sm->isRanges ? "Node Ranges," : "SubBuffer," ));
        f.Write((sm->vertical ? "true," : "false,"));
        f.Write(",\n" );
        if (sm->isRanges) {
            for (int x = sm->strands.size() - 1; x >= 0; x--) {
                f.Write(",,,");
                if (x == 0) {
                    f.Write("Bottom,");
                }
                else if (x == sm->strands.size() - 1) {
                    f.Write("Top,");
                }
                else {
                    f.Write(wxString::Format("Line %d,", (x + 1)));
                }
                f.Write("\"" + sm->strands[x] + "\"\n");
            }
        }
        else {
            f.Write(",,,");
            f.Write("SubBuffer,");
            f.Write("\"" + sm->subBuffer + "\"\n");
        }
    }
    f.Close();
}

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
 #include <wx/choice.h>
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
#include <wx/numdlg.h>

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
#include "support/VectorMath.h"
#include "xLightsVersion.h"
#include "models/SubModel.h"

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_SMDROP, wxCommandEvent);

//(*IdInit(SubModelsDialog)
const long SubModelsDialog::ID_STATICTEXT1 = wxNewId();
const long SubModelsDialog::ID_LISTCTRL_SUB_MODELS = wxNewId();
const long SubModelsDialog::ID_BUTTON3 = wxNewId();
const long SubModelsDialog::ID_BUTTON4 = wxNewId();
const long SubModelsDialog::ID_BUTTONCOPY = wxNewId();
const long SubModelsDialog::ID_BUTTON_EDIT = wxNewId();
const long SubModelsDialog::ID_BUTTON_IMPORT = wxNewId();
const long SubModelsDialog::ID_BUTTON10 = wxNewId();
const long SubModelsDialog::ID_PANEL4 = wxNewId();
const long SubModelsDialog::ID_STATICTEXT_NAME = wxNewId();
const long SubModelsDialog::ID_TEXTCTRL_NAME = wxNewId();
const long SubModelsDialog::ID_STATICTEXT2 = wxNewId();
const long SubModelsDialog::ID_CHOICE_BUFFER_STYLE = wxNewId();
const long SubModelsDialog::ID_CHECKBOX1 = wxNewId();
const long SubModelsDialog::ID_GRID1 = wxNewId();
const long SubModelsDialog::ID_BUTTON6 = wxNewId();
const long SubModelsDialog::ID_BUTTON8 = wxNewId();
const long SubModelsDialog::ID_BUTTON1 = wxNewId();
const long SubModelsDialog::ID_BUTTON2 = wxNewId();
const long SubModelsDialog::ID_BUTTON_MOVE_UP = wxNewId();
const long SubModelsDialog::ID_BUTTON_MOVE_DOWN = wxNewId();
const long SubModelsDialog::ID_BUTTON7 = wxNewId();
const long SubModelsDialog::ID_BUTTON_SORT_ROW = wxNewId();
const long SubModelsDialog::ID_BUTTON_DRAW_MODEL = wxNewId();
const long SubModelsDialog::ID_PANEL2 = wxNewId();
const long SubModelsDialog::ID_PANEL3 = wxNewId();
const long SubModelsDialog::ID_NOTEBOOK1 = wxNewId();
const long SubModelsDialog::ID_PANEL5 = wxNewId();
const long SubModelsDialog::ID_PANEL1 = wxNewId();
const long SubModelsDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_MODEL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_FILE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_CUSTOM = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPORT_CSV = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPORT_XMODEL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPORT_TOOTHERS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_GENERATE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SHIFT = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_FLIP_HOR = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_FLIP_VER = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_REVERSE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_JOIN = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SORT_BY_NAME = wxNewId();

BEGIN_EVENT_TABLE(SubModelsDialog,wxDialog)
	//(*EventTable(SubModelsDialog)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_SMDROP, SubModelsDialog::OnDrop)
END_EVENT_TABLE()

SubModelsDialog::SubModelsDialog(wxWindow* parent) :
    m_creating_bound_rect(false),
    m_bound_start_x(0),
    m_bound_start_y(0),
    m_bound_end_x(0),
    m_bound_end_y(0),
    mPointSize(2)
{
	//(*Initialize(SubModelsDialog)
	wxBoxSizer* BoxSizer1;
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

	Create(parent, wxID_ANY, _("SubModels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
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
	ListCtrl_SubModels = new wxListCtrl(Panel2, ID_LISTCTRL_SUB_MODELS, wxDefaultPosition, wxSize(164,201), wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_SUB_MODELS"));
	FlexGridSizer9->Add(ListCtrl_SubModels, 0, wxALL|wxEXPAND, 5);
	FlexGridSizer10 = new wxFlexGridSizer(4, 3, 0, 0);
	AddButton = new wxButton(Panel2, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	AddButton->SetToolTip(_("Add New SubModel"));
	FlexGridSizer10->Add(AddButton, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	DeleteButton = new wxButton(Panel2, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	DeleteButton->SetToolTip(_("Delete Selected SubModel"));
	FlexGridSizer10->Add(DeleteButton, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonCopy = new wxButton(Panel2, ID_BUTTONCOPY, _("Copy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONCOPY"));
	ButtonCopy->SetToolTip(_("Copy Selected SubModel"));
	FlexGridSizer10->Add(ButtonCopy, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	Button_Edit = new wxButton(Panel2, ID_BUTTON_EDIT, _("Action..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_EDIT"));
	Button_Edit->SetToolTip(_("Perform Actions on the SubModels"));
	FlexGridSizer10->Add(Button_Edit, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	ButtonImport = new wxButton(Panel2, ID_BUTTON_IMPORT, _("Import..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
	ButtonImport->SetToolTip(_("Import SubModels"));
	FlexGridSizer10->Add(ButtonImport, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	Button_Export = new wxButton(Panel2, ID_BUTTON10, _("Export..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON10"));
	Button_Export->SetToolTip(_("Export SubModel as File"));
	FlexGridSizer10->Add(Button_Export, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
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
	TextCtrl_Name = new wxTextCtrl(Panel3, ID_TEXTCTRL_NAME, _("SubModel Name"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_NAME"));
	FlexGridSizer7->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 0);
	TypeNotebook = new wxNotebook(Panel3, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Panel1 = new wxPanel(TypeNotebook, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	FlexGridSizer8 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer8->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Buffer Style:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChoiceBufferStyle = new wxChoice(Panel1, ID_CHOICE_BUFFER_STYLE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BUFFER_STYLE"));
	FlexGridSizer6->Add(ChoiceBufferStyle, 1, wxALL, 5);
	LayoutCheckbox = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Vertical Buffer Layout"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	LayoutCheckbox->SetValue(false);
	FlexGridSizer6->Add(LayoutCheckbox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxFIXED_MINSIZE, 5);
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
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	Button_ReverseNodes = new wxButton(Panel1, ID_BUTTON6, _("Reverse Nodes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	BoxSizer1->Add(Button_ReverseNodes, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	Button_ReverseRows = new wxButton(Panel1, ID_BUTTON8, _("Reverse Rows"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	BoxSizer1->Add(Button_ReverseRows, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer8->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Button_SortRow = new wxButton(Panel1, ID_BUTTON_SORT_ROW, _("Sort Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_SORT_ROW"));
	FlexGridSizer5->Add(Button_SortRow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsItemRClick);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsKeyDown);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsColumnClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddButtonClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteButtonClick);
	Connect(ID_BUTTONCOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_Sub_CopyClick);
	Connect(ID_BUTTON_EDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_EditClick);
	Connect(ID_BUTTON_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButtonImportClick);
	Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ExportClick);
	Connect(ID_TEXTCTRL_NAME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SubModelsDialog::OnTextCtrl_NameText_Change);
	Connect(ID_CHOICE_BUFFER_STYLE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SubModelsDialog::OnChoiceBufferStyleSelect);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnLayoutCheckboxClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridLabelLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_DCLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridLabelLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellSelect);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseNodesClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseRowsClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddRowButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteRowButtonClick);
	Connect(ID_BUTTON_MOVE_UP,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_MoveUpClick);
	Connect(ID_BUTTON_MOVE_DOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_MoveDownClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ReverseRowClick);
	Connect(ID_BUTTON_SORT_ROW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_SortRowClick);
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
    ListCtrl_SubModels->InsertColumn(0, nm0);

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

    modelPreview->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&SubModelsDialog::OnPreviewLeftDown, nullptr, this);
    modelPreview->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&SubModelsDialog::OnPreviewLeftUp, nullptr, this);
    modelPreview->Connect(wxEVT_MOTION, (wxObjectEventFunction)&SubModelsDialog::OnPreviewMouseMove, nullptr, this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&SubModelsDialog::OnPreviewMouseLeave, nullptr, this);
    modelPreview->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&SubModelsDialog::OnPreviewLeftDClick, nullptr, this);

    subBufferPanel = new SubBufferPanel(SubBufferPanelHolder, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    subBufferPanel->SetMinSize(SubBufferSizer->GetSize());
    SubBufferSizer->Add(subBufferPanel, 1, wxALL|wxEXPAND, 2);
    SubBufferSizer->Fit(SubBufferPanelHolder);
    SubBufferSizer->SetSizeHints(SubBufferPanelHolder);
    Connect(subBufferPanel->GetId(),SUBBUFFER_RANGE_CHANGED,(wxObjectEventFunction)&SubModelsDialog::OnSubBufferRangeChange);
    subBufferPanel->Connect(wxEVT_SIZE, (wxObjectEventFunction)&SubModelsDialog::OnSubbufferSize, nullptr, this);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    SubModelTextDropTarget *mdt = new SubModelTextDropTarget(this, ListCtrl_SubModels, "SubModel");
    ListCtrl_SubModels->SetDropTarget(mdt);

    for (auto const& style : SubModel::GetBufferStyleList()) {
        ChoiceBufferStyle->Append(style);
    }

    NodesGrid->SetSelectionBackground(*wxLIGHT_GREY);
    NodesGrid->SetCellHighlightColour(*wxLIGHT_GREY);
    NodesGrid->SetCellHighlightROPenWidth(3);
    NodesGrid->SetCellHighlightPenWidth(3);
    NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
    SetEscapeId(wxID_CANCEL);
    EnableCloseButton(false);
}

void SubModelsDialog::OnSubbufferSize(wxSizeEvent& event)
{
    subBufferPanel->Refresh();
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

    if (/*(m->GetDisplayAs() == "Matrix" ||
        m->GetDisplayAs() == "Tree") && */ m->GetDefaultBufferWi() > 1 && m->GetDefaultBufferHt() > 1)
    {
        _isMatrix = true;
    }

    SetTitle(GetTitle() + " - " + m->GetName());

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
            names += ListCtrl_SubModels->GetItemText(i) + ",";
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
    SaveXML(model);
}

void SubModelsDialog::SaveXML(Model *m)
{
    xLightsFrame* xlights = xLightsApp::GetFrame();
    wxXmlNode * root = m->GetModelXml();
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
        child->AddAttribute("bufferstyle", (*a)->bufferStyle);

        // If the submodel name has changed ... we need to rename the model
        if ((*a)->oldName != (*a)->name)
        {
            xlights->RenameModel(m->GetName() + std::string("/") + (*a)->oldName.ToStdString(), m->GetName() + std::string("/") + (*a)->name.ToStdString());
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

    xlights->EnsureSequenceElementsAreOrderedCorrectly(m->GetName(), submodelOrder);
}

#pragma region actions

void SubModelsDialog::OnNameChoiceSelect(wxCommandEvent& event)
{
    Select(event.GetString());
}

wxString SubModelsDialog::GenerateSubModelName(wxString basename)
{
    if (IsUniqueName(basename))
        return basename;

    wxRegEx re("(\\d+)", wxRE_ADVANCED | wxRE_NEWLINE);
    if (re.Matches(basename)) {
        wxString match = re.GetMatch(basename, 1);
        int index = wxAtoi(match.ToStdString());
        basename.Replace(match, wxString::Format("%d",++index), false);
        return GenerateSubModelName(basename);
    }

    return GenerateSubModelName(wxString::Format("%s-1", basename));
}

bool SubModelsDialog::IsUniqueName(wxString const& newname) const
{
    for (int j = 0; j < ListCtrl_SubModels->GetItemCount(); j++) {
        wxString const& name = ListCtrl_SubModels->GetItemText(j);

        if (name.IsSameAs(newname)) {
            return false;
        }
    }
    return true;
}

void SubModelsDialog::OnAddButtonClick(wxCommandEvent& event)
{
    wxString name = GenerateSubModelName("SubModel-1");

    SubModelInfo* sm = new SubModelInfo(name);
    sm->vertical = false;
    sm->strands.clear();
    sm->strands.push_back("");
    sm->isRanges = true;
    sm->bufferStyle = "Default";
    _subModels.push_back(sm);

    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm);
    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
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

void SubModelsDialog::OnDeleteButtonClick(wxCommandEvent& event)
{
    int firstindex = -1;
    wxString names = GetSelectedNames();
    wxStringTokenizer tokenizer(names, ",");
    wxString msg = "";
    int count = tokenizer.CountTokens();
    if ( count > 1) {
        msg.Printf(wxT("Are you sure you want to delete %d SubModels? \n\nCurrent open sequence effects will be removed as well. "), count);
    } else {
        msg = "Are you sure you want to delete SubModel " + names + "? \n\nCurrent open sequence effects will be removed as well.";
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

void SubModelsDialog::OnButton_EditClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(SUBMODEL_DIALOG_GENERATE, "Generate Slices");
    if (ListCtrl_SubModels->GetItemCount() > 0) {
        mnu.Append(SUBMODEL_DIALOG_FLIP_VER, "Flip All SubModels Vertical");
        mnu.Append(SUBMODEL_DIALOG_FLIP_HOR, "Flip All SubModels Horizontial");
        mnu.Append(SUBMODEL_DIALOG_SHIFT, "Shift All Nodes in All SubModels");
        mnu.Append(SUBMODEL_DIALOG_REVERSE, "Reverse All Nodes in All SubModels");
    }
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& SubModelsDialog::OnEditBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void SubModelsDialog::OnButtonImportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(SUBMODEL_DIALOG_IMPORT_MODEL, "Import SubModels From Model");
    mnu.Append(SUBMODEL_DIALOG_IMPORT_FILE, "Import SubModels From File");
    if (_isMatrix) {
        mnu.Append(SUBMODEL_DIALOG_IMPORT_CUSTOM, "Import Custom Model Overlay");
    }
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& SubModelsDialog::OnImportBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void SubModelsDialog::OnButton_ExportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(SUBMODEL_DIALOG_EXPORT_CSV, "Export SubModels As CSV");
    if (ListCtrl_SubModels->GetSelectedItemCount() == 1) {
        mnu.Append(SUBMODEL_DIALOG_EXPORT_XMODEL, "Export SubModel As xModel");
    }
    mnu.Append(SUBMODEL_DIALOG_EXPORT_TOOTHERS, "Export SubModels To Other Model(s)");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& SubModelsDialog::OnExportBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void SubModelsDialog::OnImportBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_IMPORT_MODEL) {
        //Import Submodels from another Model
        xLightsFrame* xlights = xLightsApp::GetFrame();
        wxArrayString choices = getModelList(&xlights->AllModels);
        wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);
        if (dlg.ShowModal() == wxID_OK) {
            Model *m = xlights->GetModel(dlg.GetStringSelection());
            ReadSubModelXML(m->GetModelXml());
        }
    }
    else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_FILE) {
        //Import Submodels xModel File
        wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        ImportSubModel(filename);
    }
    else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_CUSTOM) {
        //Import model as a overlay on matrix
        wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        ImportCustomModel(filename);
    }
}

void SubModelsDialog::OnEditBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_GENERATE) {
        Generate();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_FLIP_VER) {
        FlipVertical();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_FLIP_HOR) {
        FlipHorizontal();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_SHIFT) {
        Shift();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_REVERSE) {
        Reverse();
    }
}

void SubModelsDialog::OnExportBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_EXPORT_CSV) {
        wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filename = wxFileSelector(_("Save CSV File"), wxEmptyString, model->Name(), ".csv", "Export files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (filename.IsEmpty()) return;

        ExportSubModels(filename);
    }
    else if (event.GetId() == SUBMODEL_DIALOG_EXPORT_XMODEL) {
        wxString name = GetSelectedName();
        if (name == "") {
            return;
        }

        wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
        wxString filename = wxFileSelector(_("Save Custom xModel File"), wxEmptyString, name, ".xmodel", "Model Files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

        if (filename.IsEmpty()) return;

        ExportSubModelAsxModel(filename, name);
    }
    else if (event.GetId() == SUBMODEL_DIALOG_EXPORT_TOOTHERS)
    {
        ExportSubmodelToOtherModels();
    }
}

void SubModelsDialog::OnListPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_JOIN) {
        JoinSelectedModels();
    } else if (event.GetId() == SUBMODEL_DIALOG_SORT_BY_NAME) {
        SortSubModelsByName();
    }
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
    if (sm != nullptr) {
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
    if (sm != nullptr) {
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

void SubModelsDialog::OnChoiceBufferStyleSelect(wxCommandEvent& event)
{
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());
    if (sm != nullptr) {
        sm->bufferStyle = ChoiceBufferStyle->GetStringSelection();
    }
}

void SubModelsDialog::OnNodesGridLabelLeftClick(wxGridEvent& event)
{
    SelectRow(event.GetRow());
    if (event.GetRow() != -1) {
        NodesGrid->GoToCell(event.GetRow(), 0);
    }
}

void SubModelsDialog::OnListCtrl_SubModelsItemRClick(wxListEvent& event)
{
    if (ListCtrl_SubModels->GetItemCount() > 0) {
        wxMenu mnu;

        mnu.Append(SUBMODEL_DIALOG_SORT_BY_NAME, "Sort by Name");
        if (ListCtrl_SubModels->GetSelectedItemCount() > 1) {
            mnu.AppendSeparator();
            mnu.Append(SUBMODEL_DIALOG_JOIN, "Join");
        }

        mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SubModelsDialog::OnListPopup, nullptr, this);
        PopupMenu(&mnu);
    }
}

void SubModelsDialog::OnButton_ReverseNodesClick(wxCommandEvent& event)
{
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());

    if (sm->isRanges)
    {
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
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int index = GetSelectedIndex();
    wxASSERT(index >= 0);

    wxString name = wxString(Model::SafeModelName(TextCtrl_Name->GetValue().ToStdString()));

    if (name.IsEmpty()) {
        TextCtrl_Name->SetBackgroundColour(*wxRED);
        return;
    }

    if (name != TextCtrl_Name->GetValue()) {
        TextCtrl_Name->SetValue(name);
    }

    SubModelInfo* sm = (SubModelInfo*)ListCtrl_SubModels->GetItemData(index);
    if (sm != nullptr) {
        bool clash = false;
        for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++) {
            if (index != i && ListCtrl_SubModels->GetItemText(i) == name) {
                clash = true;
                break;
            }
        }

        if (!clash) {
            sm->name = name;
            ListCtrl_SubModels->SetItemText(index, name);
        }
    }
    else {
        logger_base.warn("SubModelsDialog::ApplySubmodelName submodel not found for index %d.", index);
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
        Button_Export->Disable();
    } else {
        ListCtrl_SubModels->Enable();
        Button_Export->Enable();
    }

    if (ListCtrl_SubModels->GetSelectedItemCount() > 0)
    {
        DeleteButton->Enable();
        ButtonCopy->Enable();
        NodesGrid->Enable();
        if (ChoiceBufferStyle->GetSelection() == 1) {
            LayoutCheckbox->Disable();
        }
        else {
            LayoutCheckbox->Enable();
        }
        AddRowButton->Enable();
        ChoiceBufferStyle->Enable();
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
            Button_SortRow->Enable();
        }
        else
        {
            DeleteRowButton->Disable();
            Button_MoveUp->Disable();
            Button_MoveDown->Disable();
            Button_ReverseRow->Disable();
            Button_SortRow->Disable();
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
        ChoiceBufferStyle->Disable();
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

        int idx = ChoiceBufferStyle->FindString(sm->bufferStyle);
        if (idx != -1) {
            ChoiceBufferStyle->SetSelection(idx);
        }
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
        NodesGrid->ForceRefresh();
        SelectRow(-1);

    } else {
        TypeNotebook->SetSelection(1);
        NodesGrid->BeginBatch();
        if (NodesGrid->GetNumberRows() > 0) {
            NodesGrid->DeleteRows(0, NodesGrid->GetNumberRows());
        }
        NodesGrid->AppendRows(1); // we always need one row
        NodesGrid->EndBatch();
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
    model->DisplayEffectOnWindow(modelPreview, PIXEL_SIZE_ON_DIALOGS);
}

void SubModelsDialog::ClearNodeColor(Model *m)
{
    xlColor c(xlDARK_GREY);
    int nn = m->GetNodeCount();
    if (m->modelDimmingCurve) {
        m->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        m->SetNodeColor(node, c);
    }
}

void SubModelsDialog::SelectRow(int r) {
    ClearNodeColor(model);

    static wxColor priorc = wxColor(255, 100, 255);
    if (r == -1) {
        for (int i = 0; i < NodesGrid->GetNumberRows(); ++i) {
            SetNodeColor(i, xlWHITE);
        }
    } else {
        for (int i = 0; i < NodesGrid->GetNumberRows(); ++i) {
            SetNodeColor(i, r == i ? xlWHITE : priorc);
        }
        // redo the selected row to ensure any duplicated nodes are highlighted
        SetNodeColor(r, xlWHITE);
    }
    NodesGrid->Refresh();
    model->DisplayEffectOnWindow(modelPreview, PIXEL_SIZE_ON_DIALOGS);
}

bool SubModelsDialog::SetNodeColor(int row, xlColor const& c) {

    wxString v = NodesGrid->GetCellValue(row, 0);
    if (v.empty()) {
        return false;
    }
    bool found = false;
    wxStringTokenizer wtkz(v, ",");
    while (wtkz.HasMoreTokens()) {
        wxString const valstr = wtkz.GetNextToken();

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
                model->SetNodeColor(n, c);
                found = true;
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
    return found;
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

void SubModelsDialog::Generate()
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
                    int end = (i + 1) * per;

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

void SubModelsDialog::OnButton_SortRowClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    wxString oldnodes = ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);
    wxArrayString oldNodeArrray = wxSplit(oldnodes, ',');

    std::sort(oldNodeArrray.begin(), oldNodeArrray.end(),
        [](const wxString& a, const wxString& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

    sm->strands[sm->strands.size() - 1 - row] = CompressNodes(wxJoin(oldNodeArrray, ','));

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
#pragma endregion

#pragma region Priveiw Selection
void SubModelsDialog::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;

        SelectAllInBoundingRect(event.ShiftDown());
        m_creating_bound_rect = false;

        //RenderModel();
    }
}

void SubModelsDialog::OnPreviewMouseLeave(wxMouseEvent& event)
{
    m_creating_bound_rect = false;
    RenderModel();
}

void SubModelsDialog::OnPreviewLeftDown(wxMouseEvent& event)
{
    m_creating_bound_rect = true;
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    m_bound_start_x = ray_origin.x;
    m_bound_start_y = ray_origin.y;
    m_bound_end_x = m_bound_start_x;
    m_bound_end_y = m_bound_start_y;
}

void SubModelsDialog::OnPreviewLeftDClick(wxMouseEvent& event)
{
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    int x = ray_origin.x;
    int y = ray_origin.y;
    wxString stNode = model->GetNodeNear(modelPreview, wxPoint(x, y), false);
    if (stNode.IsEmpty())
        return;
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    int row = NodesGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<wxRealPoint> pts;
    wxString oldnodes = ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);
    auto oldNodeArrray = wxSplit(oldnodes, ',');

    //toggle nodes if double click
    bool found = false;
    for (auto it = oldNodeArrray.begin(); it != oldNodeArrray.end(); ++it) {
        if (*it == stNode) {
            oldNodeArrray.erase(it);//remove if in list
            found = true;
            break;
        }
    }
    if (!found) {
        oldNodeArrray.push_back(stNode);//add if not in list
    }
    sm->strands[sm->strands.size() - 1 - row] = CompressNodes(wxJoin(oldNodeArrray, ','));

    Select(GetSelectedName());
    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);

    ValidateWindow();
}

void SubModelsDialog::OnPreviewMouseMove(wxMouseEvent& event)
{
    event.ResumePropagation(1);
    event.Skip();
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;
        RenderModel();
    }
}

void SubModelsDialog::RenderModel()
{
    if (modelPreview == nullptr || !modelPreview->StartDrawing(mPointSize)) return;

    if (m_creating_bound_rect) {
        modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
    }
    model->DisplayEffectOnWindow(modelPreview, PIXEL_SIZE_ON_DIALOGS);
    modelPreview->EndDrawing();
}

void SubModelsDialog::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    VectorMath::ScreenPosToWorldRay(
        x, modelPreview->getHeight() - y,
        modelPreview->getWidth(), modelPreview->getHeight(),
        modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );
}

void SubModelsDialog::SelectAllInBoundingRect(bool shiftDwn)
{
    if (shiftDwn) {
        RemoveNodes();
        return;
    }
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm->isRanges)
        return;

    int row = NodesGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<wxRealPoint> pts;
    std::vector<int> nodes = model->GetNodesInBoundingBox(modelPreview, wxPoint(m_bound_start_x, m_bound_start_y), wxPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;
    wxString oldnodes = ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);

    auto oldNodeArrray = wxSplit(oldnodes, ',');
    for (auto const& newNode : nodes) {
        wxString stNode = wxString::Format("%d", newNode);
        bool found = false;
        for (auto const& oldNode : oldNodeArrray) {
            if (oldNode == stNode) {
                found = true;
                break;
            }
        }
        if (!found) {
            oldNodeArrray.push_back(stNode);
        }
    }

    sm->strands[sm->strands.size() - 1 - row] = CompressNodes(wxJoin(oldNodeArrray, ','));

    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);

    ValidateWindow();
}

void SubModelsDialog::RemoveNodes()
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);

    if (!sm->isRanges)
        return;

    int row = NodesGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<wxRealPoint> pts;
    std::vector<int> nodes = model->GetNodesInBoundingBox(modelPreview, wxPoint(m_bound_start_x, m_bound_start_y), wxPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;
    wxString oldnodes = ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);
    auto oldNodeArrray = wxSplit(oldnodes, ',');

    for (auto const& newNode : nodes) {
        wxString stNode = wxString::Format("%d", newNode);
        for (auto it = oldNodeArrray.begin(); it != oldNodeArrray.end(); ++it) {
            if (*it == stNode) {
                oldNodeArrray.erase(it);
                break;
            }
        }
    }

    sm->strands[sm->strands.size() - 1 - row] = CompressNodes(wxJoin(oldNodeArrray, ','));

    Select(GetSelectedName());
    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);
    ValidateWindow();
}
#pragma endregion

#pragma region Import Export Code
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
            sm->bufferStyle = child->GetAttribute("bufferstyle", "Default");
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

                if ((modelw == width && modelh == height) || dlg.ShowModal() == wxID_OK)
                {
                    int xStart = 0;
                    if (dlg.GetX() == AlignmentDialog::Alignment::CENTRE)
                    {
                        xStart = (float)modelw / 2.0 - (float)width / 2.0;
                    }
                    else if (dlg.GetX() == AlignmentDialog::Alignment::RIGHT)
                    {
                        xStart = modelw - width;
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
                                long nn = model->GetNodeNumber(rnum, cnum);
                                if (nn >= 0) {
                                    row += wxString::Format("%d,", nn+1);
                                    nodeMap[wxAtoi(c)] = nn;
                                }
                                else {
                                    row += ",";
                                }
                            }
                            cnum++;
                        }
                        // chop off one comma
                        if (row.size() > 0) row = row.Left(row.size() - 1);
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

void SubModelsDialog::ExportSubModels(wxString const& filename)
{
    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString const header = "Name,Type,Vertical Buffer,Buffer Style,Rows Name,Node Ranges\n";
    f.Write(header);

    for (auto sm : _subModels) {
        f.Write(sm->name + ",");
        f.Write((sm->isRanges ? "Node Ranges," : "SubBuffer," ));
        f.Write((sm->vertical ? "true," : "false,"));
        f.Write((sm->bufferStyle + ","));
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

void SubModelsDialog::ExportSubModelAsxModel(wxString const& filename, const std::string& name)
{
    SubModelInfo* info = GetSubModelInfo(name);

    int rows = model->GetDefaultBufferHt();
    int cols = model->GetDefaultBufferWi();

    std::vector<std::vector<int>> data;
    data.resize(rows);
    for (auto& it : data) {
        it.resize(cols);
    }

    std::list<int> nodenums;

    for (const auto& strand : info->strands) {
        auto ranges = wxSplit(strand, ',');

        for (const auto& r : ranges) {
            if (Contains(r, "-")) {
                auto ss = wxSplit(r, '-');
                if (ss.size() == 2) {
                    int start = wxAtoi(Trim(ss[0]));
                    int end = wxAtoi(Trim(ss[1]));
                    if (start > 0 && end > 0) {
                        int incr = (end - start) / std::abs(end - start);
                        for (int n = start; n != end + incr; n += incr) {

                            std::vector<wxPoint> pts;
                            model->GetNodeCoords(n-1, pts);

                            if (pts.size() > 0) {
                                data[pts[0].y][pts[0].x] = n;
                                nodenums.push_back(n);
                            }
                        }
                    }
                }
            }
            else {
                int n = wxAtoi(Trim(r));
                if (n > 0) {
                    std::vector<wxPoint> pts;
                    model->GetNodeCoords(n-1, pts);

                    if (pts.size() > 0) {
                        data[pts[0].y][pts[0].x] = n;
                        nodenums.push_back(n);
                    }
                }
            }
        }
    }
    nodenums.sort();

    // now go through and 1 base all the nodes
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (data[r][c] > 0) {
                int newnum = 0;
                for (const auto& it : nodenums) {
                    if (it == data[r][c]) {
                        data[r][c] = newnum + 1;
                        break;
                    }
                    newnum++;
                }
            }
        }
    }

    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    std::string cm;

    for (int r = rows - 1 ; r >= 0; r--) {
        for (int c = 0; c < cols; c++) {
            if (data[r][c] != 0) {
                cm += wxString::Format("%d", data[r][c]);
            }
            if (c != cols - 1) cm += ",";
        }
        if (r != 0) cm += ";";
    }

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%d\" ", cols));
    f.Write(wxString::Format("parm2=\"%d\" ", rows));
    f.Write("Depth=\"1\" ");
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", xlights_version_string));
    f.Write(" >\n");
    f.Write("</custommodel>");

    f.Close();
}
#pragma endregion

//Shift nodes  numbering 1->21, 100->120
void SubModelsDialog::Shift()
{
    wxString name = GetSelectedName();
    long min = 1;
    long max = model->GetNodeCount();

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK) {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0) {
            for (auto sm : _subModels) {
                if (sm->isRanges) {
                    for (int x = 0; x < sm->strands.size(); x++) {
                        wxString oldnodes = ExpandNodes(sm->strands[x]);
                        auto oldNodeArray = wxSplit(oldnodes, ',');
                        wxArrayString newNodeArray;
                        for (auto const& node: oldNodeArray) {
                            long val;
                            if (node.ToCLong(&val) == true) {
                                long newVal = val + scaleFactor;
                                if (newVal > max) {
                                    newVal -= max;
                                }
                                else if (newVal < min) {
                                    newVal += max;
                                }
                                newNodeArray.Add( wxString::Format("%ld", newVal) );
                            }
                        }
                        sm->strands[x] = CompressNodes(wxJoin(newNodeArray, ','));
                    }
                }
            }
            ValidateWindow();
            Select(name);

            Panel3->SetFocus();
            TextCtrl_Name->SetFocus();
            TextCtrl_Name->SelectAll();
        }
    }
}

void SubModelsDialog::FlipHorizontal()
{
    wxString name = GetSelectedName();

    for (auto a : _subModels) {
        if (a->isRanges) {
            for (auto & stand : a->strands) {
                stand = ReverseRow(stand);
            }
        }
    }

    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsDialog::FlipVertical()
{
    wxString name = GetSelectedName();

    for (auto a : _subModels) {
        if (a->isRanges) {
            if (a->strands.size() == 1) {
                continue;
            }

            std::list<std::string> reordered;
            for (auto it = a->strands.begin(); it != a->strands.end(); ++it) {
                reordered.push_front(*it);
            }

            int i = 0;
            for (auto it = reordered.begin(); it != reordered.end(); ++it) {
                a->strands[i] = *it;
                i++;
            }
        }
    }

    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

//reverse nodes  numbering 1->100, 100->1
void SubModelsDialog::Reverse()
{
    wxString name = GetSelectedName();
    long max = model->GetNodeCount() + 1;

    for (auto sm : _subModels) {
        if (sm->isRanges) {
            for (int x = 0; x < sm->strands.size(); x++) {
                wxString oldnodes = ExpandNodes(sm->strands[x]);
                auto oldNodeArray = wxSplit(oldnodes, ',');
                wxArrayString newNodeArray;
                for (auto const& node: oldNodeArray) {
                    long val;
                    if (node.ToCLong(&val) == true) {
                        long newVal = max - val;
                        newNodeArray.Add( wxString::Format("%ld", newVal) );
                    }
                }
                sm->strands[x] = CompressNodes(wxJoin(newNodeArray, ','));
            }
        }
    }
    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsDialog::JoinSelectedModels()
{
    wxString name = GenerateSubModelName("SubModel-1");

    SubModelInfo* new_sm = new SubModelInfo(name);

    new_sm->isRanges = true;

    wxString names = GetSelectedNames();
    wxStringTokenizer tokenizer(names, ",");
    while (tokenizer.HasMoreTokens()) {
        wxString token = tokenizer.GetNextToken();
        int index = GetSubModelInfoIndex(token);
        if (index == -1) {
            continue;
        }
        SubModelInfo* old_sm = _subModels.at(index);
        if (old_sm == nullptr) {
            continue;
        }
        if (!old_sm->isRanges) {
            continue;
        }
        new_sm->vertical = old_sm->vertical;

        for (auto const& stand : old_sm->strands) {
            new_sm->strands.push_back(stand);
        }
    }

    _subModels.push_back(new_sm);

    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), new_sm->name);
    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)new_sm);
    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsDialog::SortSubModelsByName()
{
    std::sort(std::begin(_subModels), std::end(_subModels), [](SubModelInfo* a, SubModelInfo* b) {
        return (wxStringNumberAwareStringCompare((*a).name, (*b).name) == -1);
    });

    PopulateList();
    ValidateWindow();
}

void SubModelsDialog::ExportSubmodelToOtherModels()
{
    if (wxMessageBox("Are you sure you want to Export this models SubModels to Other Models?\nThis will override all the other Models existing SubModels and There is no way to undo it", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    xLightsFrame* xlights = xLightsApp::GetFrame();
    wxArrayString choices = getModelList(&xlights->AllModels);

    wxMultiChoiceDialog dlg(this, "Export SubModels to Other Models", "Export SubModels", choices);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        for (auto const& idx : dlg.GetSelections()) {
            Model* m = xlights->GetModel(choices.at(idx));
            SaveXML(m);
            for (auto& it : m->GetSubModels()) {
                it->IncrementChangeCount();
            }
            m->IncrementChangeCount();
            ReloadLayout = true;
        }
    }
}

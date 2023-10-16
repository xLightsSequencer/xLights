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
 #include <wx/srchctrl.h>
 #include <wx/stattext.h>
 #include <wx/string.h>
 #include <wx/textctrl.h>
 //*)

#include <wx/dnd.h>
#include <wx/menu.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
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
#include "support/VectorMath.h"
#include "xLightsVersion.h"
#include "models/SubModel.h"
#include "CheckboxSelectDialog.h"

#include <log4cpp/Category.hh>

wxDEFINE_EVENT(EVT_SMDROP, wxCommandEvent);

//(*IdInit(SubModelsDialog)
const long SubModelsDialog::ID_CHECKBOX2 = wxNewId();
const long SubModelsDialog::ID_STATICTEXT1 = wxNewId();
const long SubModelsDialog::ID_LISTCTRL_SUB_MODELS = wxNewId();
const long SubModelsDialog::ID_SEARCHCTRL1 = wxNewId();
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
const long SubModelsDialog::ID_STATICTEXT3 = wxNewId();
//*)
const long SubModelsDialog::ID_TIMER1 = wxNewId();

const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_MODEL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_FILE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_CUSTOM = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_IMPORT_CSV = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPORT_CSV = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPORT_XMODEL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPORT_TOOTHERS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_GENERATE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SHIFT = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_FLIP_HOR = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_FLIP_VER = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_REVERSE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_JOIN = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_JOIN_SS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SPLIT = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SORT_BY_NAME = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_REMOVE_DUPLICATE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SUPPRESS_DUPLICATE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SORT_POINTS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_LR = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_TB = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_LR = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_TB = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EVEN_ROWS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SYMMETRIZE = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_SORT_POINTS_ALL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_COMBINE_STRANDS = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_EXPAND_STRANDS_ALL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_BLANKS_AS_ZERO = wxNewId();
const long SubModelsDialog::SUBMODEL_DIALOG_BLANKS_AS_EMPTY = wxNewId();

BEGIN_EVENT_TABLE(SubModelsDialog,wxDialog)
	//(*EventTable(SubModelsDialog)
    //*)
    EVT_COMMAND(wxID_ANY, EVT_SMDROP, SubModelsDialog::OnDrop)
END_EVENT_TABLE()

StretchGrid::StretchGrid (wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
			wxGrid (parent, id, pos, size, style, name)
{
    GetGridWindow()->Bind (wxEVT_SIZE, &StretchGrid::OnGridWindowSize, this);
    Bind (wxEVT_GRID_COL_SIZE, &StretchGrid::OnColHeaderSize, this);
}

StretchGrid::~StretchGrid()
{
    GetGridWindow()->Unbind (wxEVT_SIZE, &StretchGrid::OnGridWindowSize, this);
    Unbind (wxEVT_GRID_COL_SIZE, &StretchGrid::OnColHeaderSize, this);
}

void StretchGrid::OnGridWindowSize (wxSizeEvent& event)
{
    if (GetNumberCols() > 0)
        AutoSizeLastCol ();
}

void StretchGrid::OnColHeaderSize (wxGridSizeEvent& event)
{
    if (GetNumberCols() > 0)
        AutoSizeLastCol ();
}

void StretchGrid::AutoSizeLastCol ()
{
    int colWidths = 0;

    for (int i = 0; i < GetNumberCols() - 1; i++)
        colWidths += GetColWidth (i);

    int finalColWidth = GetGridWindow()->GetSize().x - colWidths;

    if (finalColWidth > 10)
        SetColSize (GetNumberCols() - 1, finalColWidth);
    else
        SetColSize (GetNumberCols() - 1, 10);
}

SubModelsDialog::SubModelsDialog(wxWindow* parent, OutputManager* om) :
    m_creating_bound_rect(false),
    m_bound_start_x(0),
    m_bound_start_y(0),
    m_bound_end_x(0),
    m_bound_end_y(0),
    mPointSize(PIXEL_SIZE_ON_DIALOGS),
    _outputManager(om)
{
	//(*Initialize(SubModelsDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer11;
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
	FlexGridSizer9 = new wxFlexGridSizer(5, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(2);
	CheckBox_OutputToLights = new wxCheckBox(Panel2, ID_CHECKBOX2, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_OutputToLights->SetValue(false);
	FlexGridSizer9->Add(CheckBox_OutputToLights, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("SubModels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer9->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl_SubModels = new wxListCtrl(Panel2, ID_LISTCTRL_SUB_MODELS, wxDefaultPosition, wxSize(200,133), wxLC_REPORT, wxDefaultValidator, _T("ID_LISTCTRL_SUB_MODELS"));
	FlexGridSizer9->Add(ListCtrl_SubModels, 0, wxALL|wxEXPAND, 5);
	SearchCtrl1 = new wxSearchCtrl(Panel2, ID_SEARCHCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SEARCHCTRL1"));
	FlexGridSizer9->Add(SearchCtrl1, 1, wxALL|wxEXPAND, 5);
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
	NodesGrid = new StretchGrid(Panel1, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
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
	FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer11->AddGrowableCol(1);
	FlexGridSizer11->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer11->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NodeNumberText = new wxStaticText(this, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDLG_UNIT(this,wxSize(50,-1)), 0, _T("ID_STATICTEXT3"));
	FlexGridSizer11->Add(NodeNumberText, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer11, 2, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	Center();

	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnCheckBox_OutputToLightsClick);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_BEGIN_DRAG,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsBeginDrag);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_ITEM_SELECTED,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsItemSelect);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsItemRClick);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_KEY_DOWN,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsKeyDown);
	Connect(ID_LISTCTRL_SUB_MODELS,wxEVT_COMMAND_LIST_COL_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnListCtrl_SubModelsColumnClick);
	Connect(ID_SEARCHCTRL1,wxEVT_COMMAND_TEXT_ENTER,(wxObjectEventFunction)&SubModelsDialog::OnButton_SearchClick);
	Connect(ID_SEARCHCTRL1,wxEVT_COMMAND_SEARCHCTRL_SEARCH_BTN,(wxObjectEventFunction)&SubModelsDialog::OnButton_SearchClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnAddButtonClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnDeleteButtonClick);
	Connect(ID_BUTTONCOPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_Sub_CopyClick);
	Connect(ID_BUTTON_EDIT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_EditClick);
	Connect(ID_BUTTON_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButtonImportClick);
	Connect(ID_BUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnButton_ExportClick);
	Connect(ID_TEXTCTRL_NAME,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SubModelsDialog::OnTextCtrl_NameText_Change);
	Connect(ID_CHOICE_BUFFER_STYLE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SubModelsDialog::OnChoiceBufferStyleSelect);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&SubModelsDialog::OnLayoutCheckboxClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellRightClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridLabelLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_RIGHT_CLICK,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellRightClick);
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
	Connect(wxID_ANY,wxEVT_INIT_DIALOG,(wxObjectEventFunction)&SubModelsDialog::OnInit);
	//*)

    Connect(ID_NOTEBOOK1, wxEVT_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)& SubModelsDialog::OnTypeNotebookPageChanged);
    Connect(wxID_ANY, EVT_SMDROP, (wxObjectEventFunction)&SubModelsDialog::OnDrop);
    Connect(ID_GRID1, wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&SubModelsDialog::OnNodesGridCellChange);
    //Connect(ID_GRID1, wxEVT_CHAR, (wxObjectEventFunction)&SubModelsDialog::OnGridChar);
    Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&SubModelsDialog::OnCancel);
    Connect(wxID_CANCEL, wxEVT_BUTTON, (wxObjectEventFunction)&SubModelsDialog::OnCancel);
    Connect(ID_TEXTCTRL_NAME, wxEVT_COMMAND_TEXT_ENTER, (wxObjectEventFunction)&SubModelsDialog::ApplySubmodelName);

    TextCtrl_Name->Bind(wxEVT_KILL_FOCUS, &SubModelsDialog::OnTextCtrl_NameText_KillFocus, this);

    wxListItem nm0;
    nm0.SetId(0);
    nm0.SetImage(-1);
    nm0.SetAlign(wxLIST_FORMAT_LEFT);
    nm0.SetText(_("SubModel"));
    ListCtrl_SubModels->InsertColumn(0, nm0);

    _parent = parent;

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

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
    }
}

/*
According to the wx forums, set size doesn't work in the object constructor
because the wxWidget base class recalulates the size and layout,
It will only works if you use OnInit event or call it later.
*/
void SubModelsDialog::OnInit(wxInitDialogEvent& event)
{
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
    }
    wxConfigBase* config = wxConfigBase::Get();
    int h = config->ReadLong("SubModelsDialogSashPosition", 0);
    if (h != 0) {
        SplitterWindow1->SetSashPosition(h);
    }

    EnsureWindowHeaderIsOnScreen(this);
    Layout();
}

void SubModelsDialog::OnSubbufferSize(wxSizeEvent& event)
{
    subBufferPanel->Refresh();
}

void SubModelsDialog::OnCancel(wxCloseEvent& event)
{
    if (wxMessageBox("Are you sure you want to close the Submodels window?", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    SubModelsDialog::EndDialog(wxID_CANCEL);
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

    wxConfigBase* config = wxConfigBase::Get();
    int i = SplitterWindow1->GetSashPosition();
    config->Write("SubModelsDialogSashPosition", i);
    config->Flush();

    StopOutputToLights();
    if (_oldOutputToLights) {
        _outputManager->StartOutput();
    }
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
        // unselect it
        ListCtrl_SubModels->SetItemState(index, 0, wxLIST_STATE_SELECTED);
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
    mnu.Append(SUBMODEL_DIALOG_IMPORT_CSV, "Import CSV as SubModel");
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
            ImportSubModelXML(m->GetModelXml());
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
    else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_CSV) {
        wxString filename = wxFileSelector(_("Choose CSV file"), wxEmptyString, wxEmptyString, wxEmptyString, "CSV Files (*.csv)|*.csv", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        ImportCSVSubModel(filename);
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
        JoinSelectedModels(false);
    }
    else if (event.GetId() == SUBMODEL_DIALOG_JOIN_SS) {
        JoinSelectedModels(true);
    }
    else if (event.GetId() == SUBMODEL_DIALOG_SORT_BY_NAME) {
        SortSubModelsByName();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_SPLIT) {
        SplitSelectedSubmodel();
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

void SubModelsDialog::OnNodesGridCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;
    if (event.GetRow() != -1) {
        //NodesGrid->GoToCell(event.GetRow(), 0);

        mnu.Append(SUBMODEL_DIALOG_REMOVE_DUPLICATE, "Remove Duplicates");
        mnu.Append(SUBMODEL_DIALOG_SUPPRESS_DUPLICATE, "Suppress Duplicates");
        mnu.Append(SUBMODEL_DIALOG_SORT_POINTS, "Sort Strand Points Geometrically...");
        mnu.AppendSeparator();
    }
    mnu.Append(SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_LR, "Remove Duplicates All Left->Right");
    mnu.Append(SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_TB, "Remove Duplicates All Top->Bottom");
    mnu.Append(SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_LR, "Suppress Duplicates All Left->Right");
    mnu.Append(SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_TB, "Suppress Duplicates All Top->Bottom");
    mnu.Append(SUBMODEL_DIALOG_EVEN_ROWS, "Uniform Row Length");
    mnu.Append(SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS, "Pivot Rows / Columns");
    mnu.Append(SUBMODEL_DIALOG_SORT_POINTS_ALL, "Geometrically Sort Points All Strands...");

    mnu.AppendSeparator();
    mnu.Append(SUBMODEL_DIALOG_SYMMETRIZE, "Symmetrize (Rotational)");
    mnu.Append(SUBMODEL_DIALOG_COMBINE_STRANDS, "Combine Strands");
    
    mnu.AppendSeparator();
    mnu.Append(SUBMODEL_DIALOG_EXPAND_STRANDS_ALL, "Expand All Strands");
    mnu.Append(SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL, "Compress All Strands");
    mnu.Append(SUBMODEL_DIALOG_BLANKS_AS_ZERO, "Convert Blanks To Zeros");
    mnu.Append(SUBMODEL_DIALOG_BLANKS_AS_EMPTY, "Convert Zeros To Empty");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SubModelsDialog::OnNodesGridPopup, nullptr, this);
    PopupMenu(&mnu);
}

void SubModelsDialog::OnNodesGridPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_REMOVE_DUPLICATE) {
        RemoveDuplicates(false);
    }
    if (event.GetId() == SUBMODEL_DIALOG_SUPPRESS_DUPLICATE) {
        RemoveDuplicates(true);
    }
    if (event.GetId() == SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_LR) {
        RemoveAllDuplicates(true, false);
    }
    if (event.GetId() == SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_TB) {
        RemoveAllDuplicates(false, false);
    }
    if (event.GetId() == SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_LR) {
        RemoveAllDuplicates(true, true);
    }
    if (event.GetId() == SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_TB) {
        RemoveAllDuplicates(false, true);
    }
    if (event.GetId() == SUBMODEL_DIALOG_EVEN_ROWS) {
        MakeRowsUniform();
    }
    if (event.GetId() == SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS) {
        PivotRowsColumns();
    }
    if (event.GetId() == SUBMODEL_DIALOG_SYMMETRIZE) {
        Symmetrize();
    }
    if (event.GetId() == SUBMODEL_DIALOG_SORT_POINTS) {
        OrderPoints(false);
    }
    if (event.GetId() == SUBMODEL_DIALOG_SORT_POINTS_ALL) {
        OrderPoints(true);
    }
    if (event.GetId() == SUBMODEL_DIALOG_COMBINE_STRANDS) {
        CombineStrands();
    }
    if (event.GetId() == SUBMODEL_DIALOG_EXPAND_STRANDS_ALL) {
        processAllStrands([](wxString str) { return ExpandNodes(str); });
    }
    if (event.GetId() == SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL) {
        processAllStrands([](wxString str) { return CompressNodes(str); });
    }
    if (event.GetId() == SUBMODEL_DIALOG_BLANKS_AS_ZERO) {
        processAllStrands([](wxString str) {
            auto ns = wxSplit(str, ',');
            for (auto i = ns.begin(); i != ns.end(); ++i) {
                if (*i == "")
                    *i = "0";
            }
            return wxJoin(ns, ',');
        });
    }
    if (event.GetId() == SUBMODEL_DIALOG_BLANKS_AS_EMPTY) {
        processAllStrands([](wxString str) {
            auto ns = wxSplit(str, ',');
            for (auto i = ns.begin(); i != ns.end(); ++i) {
                if (*i == "0")
                    *i = "";
            }
            return wxJoin(ns, ',');
        });
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
            mnu.Append(SUBMODEL_DIALOG_JOIN_SS, "Join Single Strand");
        }
        if (ListCtrl_SubModels->GetSelectedItemCount() == 1) {
            mnu.AppendSeparator();
            mnu.Append(SUBMODEL_DIALOG_SPLIT, "Split");
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

void SubModelsDialog::OnButton_SearchClick(wxCommandEvent& event)
{
    int idx = GetSubModelInfoIndex(GetSelectedName());

    if (idx == _subModels.size()) {
        idx = -1;
    }
    auto const serachTxt = SearchCtrl1->GetValue().Lower();
    if (serachTxt.empty()) {
        return;
    }

    for (int i = idx + 1; i < ListCtrl_SubModels->GetItemCount(); ++i)
    {
        if (Contains(::Lower(ListCtrl_SubModels->GetItemText(i)), serachTxt)) {
            UnSelectAll();
            Select(ListCtrl_SubModels->GetItemText(i));
            return;
        }
    }
    if (ListCtrl_SubModels->GetItemCount() > 0) {
        Select(ListCtrl_SubModels->GetItemText(0));
    }
}

static void LogAndWrite(wxFile& f, const std::string& msg)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("CheckSequence: " + msg);
    if (f.IsOpened()) {
        f.Write(msg + "\r\n");
    }
}

void SubModelsDialog::Symmetrize()
{
    // Validate that we have something to work on
    wxString mname = GetSelectedName();
    if (mname.empty()) {
        return;
    }

    int row = NodesGrid->GetGridCursorRow();

    SubModelInfo* sm = GetSubModelInfo(mname);
    if (!sm)
        return;

    // Get user input
    wxNumberEntryDialog dlg(this, "Degree of Symmetry", "", "Select Degree of Rotational Symmetry", 8, 2, 100);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    int dos = dlg.GetValue();

    wxFile f;
    wxString filename = wxFileName::CreateTempFileName("xLightsSymmetrize") + ".txt";

    bool writeToFile = true;
    bool displayInEditor = true;

    if (writeToFile || displayInEditor) {
        f.Open(filename, wxFile::write);
        if (!f.IsOpened()) {
            DisplayError("Unable to create results file for Symmetrize. Aborted.", this);
            return;
        }
    }

    int w, h;
    modelPreview->GetSize(&w, &h);

    LogAndWrite(f, wxString::Format("Processing model: %s", mname.c_str()));
    LogAndWrite(f, wxString::Format("Symmetrize DoS: %d", dos));
    LogAndWrite(f, wxString::Format("Model Dimensions (based on screen): %dx%d", w, h));

    //  Calculate point xys
    std::map<int, std::pair<float, float>> coords;
    if (!model->GetScreenLocations(modelPreview, coords)) {
        DisplayError("Model doesn't have precisely one location per node");
        return;
    }
    LogAndWrite(f, wxString::Format("Number of nodes: %d", int(coords.size())));

    //  Calculate centroid
    if (coords.empty())
        return;
    float cx = 0, cy = 0;
    std::vector<float> xsv, ysv;
    for (const auto& x : coords) {
        cx += x.second.first;
        cy += x.second.second;
        xsv.push_back(x.second.first);
        ysv.push_back(x.second.second);
    }
    cx /= float(coords.size());
    cy /= float(coords.size());
    LogAndWrite(f, wxString::Format("Centroid: %f, %f", cx, cy));

    //  Calculate radius / centroid another way
    float nx = cx, ny = cy, xx = cx, xy = cy;
    float varx = 0, vary = 0;
    for (const auto& x : coords) {
        nx = std::min(nx, x.second.first);
        xx = std::max(xx, x.second.first);
        ny = std::min(ny, x.second.second);
        xy = std::max(xy, x.second.second);

        varx += (x.second.first - cx) * (x.second.first - cx);
        vary += (x.second.second - cy) * (x.second.second - cy);
    }
    LogAndWrite(f, wxString::Format("Ranges x:%.1f-%.1f, y:%1f-%1f", nx, xx, ny, xy));
    float clx = (nx + xx) / 2;
    float cly = (ny + xy) / 2;
    LogAndWrite(f, wxString::Format("Center by extremity: %f, %f", clx, cly));

    // And another
    std::sort(xsv.begin(), xsv.end());
    std::sort(ysv.begin(), ysv.end());
    float mcx, mcy;
    if (xsv.size() % 2 == 1) {
        mcx = xsv[xsv.size() / 2];
        mcy = ysv[ysv.size() / 2];
    } else {
        mcx = (xsv[xsv.size() / 2] + xsv[xsv.size() / 2 + 1]) / 2;
        mcy = (ysv[ysv.size() / 2] + ysv[ysv.size() / 2 + 1]) / 2;
    }
    LogAndWrite(f, wxString::Format("Center by median: %f, %f", mcx, mcy));

    float dlx = xx - nx;
    float dly = xy - ny;
    if (dlx > 0 && dly > 0) {
        float aspectx = dlx / std::max(dlx, dly);
        float aspecty = dly / std::max(dlx, dly);

        LogAndWrite(f, wxString::Format("Aspect ratio by extremity: %f / %f", aspectx, aspecty));

        if (true) {
            // Variance-based aspect ratio
            float mvar = std::max(varx, vary);
            aspectx = sqrtf(varx / mvar);
            aspecty = sqrtf(vary / mvar);
            LogAndWrite(f, wxString::Format("Aspect Ratio by variance: %f / %f", aspectx, aspecty));
        }

        if (aspectx < .98 || aspecty < .98) {
            wxArrayString chs;
            chs.push_back("Yes");
            chs.push_back("No");
            wxSingleChoiceDialog dlg(this, "Squarify aspect ratio?", "Aspect Ratio", chs);
            dlg.ShowModal();
            if (dlg.GetStringSelection() == "Yes") {
                for (auto& pt : coords) {
                    pt.second.first = (pt.second.first - cx) * aspecty + cx;
                    pt.second.second = (pt.second.second - cy) * aspectx + cy;
                }
            }
        }
    }

    bool handleCenterNode = false;
    if (coords.size() % dos == 1) {
        wxArrayString chs;
        chs.push_back("Yes");
        chs.push_back("No");
        wxSingleChoiceDialog dlg(this, "Shoud a center node be identified?", "Center Node", chs);
        dlg.ShowModal();
        if (dlg.GetStringSelection() == "Yes") {
            handleCenterNode = true;
        }
    }

    //  Calculate locations in new space
    std::map<int, std::pair<float, float>> fcoords1, fcoords2;
    std::map<int, float> fturns;
    for (const auto& p : coords) {
        float dx = p.second.first - cx;
        float dy = p.second.second - cy;
        if (dx == 0 && dy == 0) {
            fcoords1[p.first] = std::make_pair(cx, cy);
            fturns[p.first] = 0;
            continue;
        }
        float rad = sqrtf(dx * dx + dy * dy);
        float ang = atan2f(dy, dx);
        if (ang <= 0) {
            ang += float(2 * PI);
        }
        ang *= float(dos);
        float turn = float(ang / (2 * PI)); // Which trip around?  We want one from each trip.
        if (turn >= dos)
            turn -= dos;
        fturns[p.first] = turn;
        while (ang >= 2 * PI)
            ang -= float(2 * PI);
        ang /= float(dos);
        fcoords1[p.first] = std::make_pair(rad * cosf(ang) + cx, rad * sinf(ang) + cy);
        if (ang < PI / dos / 2)
            ang += 2 * PI / dos;
        fcoords2[p.first] = std::make_pair(rad * cosf(ang) + cx, rad * sinf(ang) + cy);
    }
    LogAndWrite(f, wxString::Format("Transformed nodes: %d", int(fcoords1.size())));

    //  Build list that need matched, and match list
    std::set<int> nodesNeedMatch;
    std::map<int, std::vector<int>> matchIDToNodeSet; // vector index is relative turn #
    std::map<int, int> nodeToMatchIDs;

    // Copy and expand data
    int origStrands = sm->strands.size();
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        auto x = wxSplit(ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ',');
        for (auto n : x) {
            if (n == "" || n == "0")
                continue;
            nodesNeedMatch.insert(wxAtoi(n));
        }
    }

    // Handle the business of a center node, if any
    if (handleCenterNode) {
        bool first = true;
        float ndst = 0;
        int nnode = -1;
        for (const auto& pt : coords) {
            float dx = pt.second.first - cx;
            float dy = pt.second.second - cy;
            float dst = dx * dx + dy * dy;
            if (first || dst < ndst) {
                ndst = dst;
                nnode = pt.first;
            }
            first = false;
        }

        nodesNeedMatch.erase(nnode);
        nodeToMatchIDs[nnode] = matchIDToNodeSet.size();
        matchIDToNodeSet[nodeToMatchIDs[nnode]] = std::vector<int>(dos, nnode);
    }

    int radius = 0;
    //  For each of numerous search radii, calculate list per grid cell
    //  We will stop if all nodes have matches
    while (!nodesNeedMatch.empty()) {
        std::vector<std::vector<std::vector<int>>> bins; // [x][y][which]
        for (int x = 0; x < w; ++x) {
            bins.push_back(std::vector<std::vector<int>>());
            for (int y = 0; y < h; ++y) {
                bins[x].push_back(std::vector<int>());
            }
        }

        // Append to lists
        for (const auto& pt : fcoords1) {
            if (nodeToMatchIDs.count(pt.first))
                continue; // Already matched

            int bx = int(pt.second.first);
            int by = int(pt.second.second);
            for (int x = bx - radius; x <= bx + radius; ++x) {
                if (x < 0 || x >= w)
                    continue;
                for (int y = by - radius; y <= by + radius; ++y) {
                    if (y < 0 || y >= h)
                        continue;
                    bins[x][y].push_back(pt.first);
                }
            }
        }
        // Add redundant copy of some - should check if already in bin?
        for (const auto& pt : fcoords2) {
            if (nodeToMatchIDs.count(pt.first))
                continue; // Already matched

            int bx = int(pt.second.first);
            int by = int(pt.second.second);
            for (int x = bx - radius; x <= bx + radius; ++x) {
                if (x < 0 || x >= w)
                    continue;
                for (int y = by - radius; y <= by + radius; ++y) {
                    if (y < 0 || y >= h)
                        continue;
                    bins[x][y].push_back(pt.first);
                }
            }
        }

        // See if any lists are ready
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                if (int(bins[x][y].size()) < dos)
                    continue; // Quick test without looking at bins closely, not enough here
                std::vector<std::pair<float, int>> matches;
                for (int pt : bins[x][y]) {
                    if (nodeToMatchIDs.count(pt) != 0)
                        continue; // Already matched this pass
                    matches.push_back(std::make_pair(fturns[pt], pt));
                }
                if (matches.size() < dos)
                    continue;
                std::sort(matches.begin(), matches.end()); // Sort CCW

                // Try to pick
                float tgt = matches[0].first;
                int found = 0;
                for (unsigned j = 0; j < matches.size(); ++j) {
                    if (matches[j].first >= tgt - .5 && matches[j].first <= tgt + .5) {
                        ++found;
                        tgt += 1;
                    }
                    if (found == dos)
                        break;
                }
                if (found != dos)
                    continue; // On closer inspection, nope

                // OK, repeat that process and record it
                std::vector<int> matched;
                tgt = matches[0].first;
                int mid = matchIDToNodeSet.size();
                for (unsigned j = 0; j < matches.size(); ++j) {
                    if (matches[j].first >= tgt - .5 && matches[j].first <= tgt + .5) {
                        ++found;
                        tgt += 1;
                        matched.push_back(matches[j].second);
                        nodeToMatchIDs[matches[j].second] = mid;
                        nodesNeedMatch.erase(matches[j].second);
                    }
                    if (found == dos)
                        break;
                }
                matchIDToNodeSet[mid] = matched;

                LogAndWrite(f, wxString::Format("Found Match for %d at radius %d", matched[0], radius));
                for (auto n : matched) {
                    LogAndWrite(f, wxString::Format("    Member %d", n));
                }
            }
        }

        // Sanity
        ++radius;
        if (radius > 20 && !nodesNeedMatch.empty()) {
            LogAndWrite(f, wxString::Format("Maximum search radius hit: %d", radius));
            break;
        }

        wxSafeYield();
    }

    bool fail = false;
    // Report any trouble
    if (!nodesNeedMatch.empty()) {
        LogAndWrite(f, "Note the following nodes could not be matched.  Ensure that zoom in/out is reasonable, that the model is centered, and that the point locations are clean.");
        for (auto x : nodesNeedMatch) {
            LogAndWrite(f, wxString::Format("  Node %d", x));
        }
        fail = true;
    }

    // Use match list to make new strands
    for (int t = 1; !fail && t < dos; ++t) {
        for (int sn = 0; sn < origStrands; ++sn) {
            bool first = true;
            // auto x = wxSplit(ExpandNodes(sm->strands[sm->strands.size() - 1 - sn]), ',');
            auto x = wxSplit(ExpandNodes(sm->strands[sn]), ',');
            wxString str;
            for (auto n : x) {
                if (first) {
                    first = false;
                } else {
                    str += ",";
                }
                if (n == "" || n == "0")
                    continue;
                int nn = wxAtoi(n);
                // Find it
                auto& matchs = matchIDToNodeSet[nodeToMatchIDs[nn]];
                for (int ii = 0; ii < dos; ++ii) {
                    if (matchs[ii] == nn) {
                        int mapn = matchs[(ii + t) % dos];
                        str += wxString::Format("%d", mapn);
                        break;
                    }
                }
            }
            sm->strands.push_back(CompressNodes(str));
        }
    }

    // Update UI
    Select(GetSelectedName());

    if (row >= 0) {
        NodesGrid->SetGridCursor(row, 0);
    }
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();

    // Save / Display results log
    if (f.IsOpened()) {
        f.Close();
    }

    if (fail) {
        DisplayError("Symmetrize encountered errors.  See log for details.", this);

        if (displayInEditor) {
            wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension("txt");
            if (ft != nullptr) {
                wxString command = ft->GetOpenCommand(filename);

                if (command == "") {
                    DisplayError(wxString::Format("Unable to show xLights Symmetrize results '%s'. See your log for the content.", filename).ToStdString(), this);
                } else {
                    wxUnsetEnv("LD_PRELOAD");
                    wxExecute(command);
                }
                delete ft;
            } else {
                DisplayError(wxString::Format("Unable to show xLights Symmetrize results '%s'. See your log for the content.", filename).ToStdString(), this);
            }
        }
    }
}

void SubModelsDialog::processAllStrands(wxString (*func)(wxString))
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    int row = NodesGrid->GetGridCursorRow();

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Process all rows
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        sm->strands[sm->strands.size() - 1 - i] = func(sm->strands[sm->strands.size() - 1 - i]);
    }

    // Update UI
    Select(name);

    if (row >= 0) {
        NodesGrid->SetGridCursor(row, 0);
    }
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

static wxString OrderPointsI(std::map<int, std::pair<float, float>>& coords, const wxString& instr, std::pair<float, float> centroid, bool radial, float startangle, bool ccw_outside)
{
    wxArrayString inp = wxSplit(ExpandNodes(instr), ',');

    std::vector<std::pair<int, int>> nodeAndBlanksBefore;
    int blanks = 0;
    for (const auto& x : inp) {
        if (x == "" || x == "0") {
            ++blanks;
        } else {
            nodeAndBlanksBefore.push_back(std::make_pair(wxAtoi(x), blanks));
            blanks = 0;
        }
    }

    if (radial) {
        std::sort(nodeAndBlanksBefore.begin(), nodeAndBlanksBefore.end(),
                  [&coords, &centroid, startangle, ccw_outside](const std::pair<int, int>& l, const std::pair<int, int>& r) {
                      auto cl = coords[l.first];
                      auto cr = coords[r.first];

                      float dxl = cl.first - centroid.first;
                      float dyl = cl.second - centroid.second;
                      float dxr = cr.first - centroid.first;
                      float dyr = cr.second - centroid.second;

                      float dl = dxl * dxl + dyl * dyl;
                      float dr = dxr * dxr + dyr * dyr;

                      // Hum, we could use dot product along angle, instead of distance...

                      return ccw_outside ? (dl > dr) : (dl < dr);
                  });
    } else {
        std::sort(nodeAndBlanksBefore.begin(), nodeAndBlanksBefore.end(),
                  [&coords, &centroid, startangle, ccw_outside](const auto& l, const auto& r) {
                      auto cl = coords[l.first];
                      auto cr = coords[r.first];

                      float angl = atan2(cl.second - centroid.second, cl.first - centroid.first);
                      float angr = atan2(cr.second - centroid.second, cr.first - centroid.first);
                      angl -= startangle;
                      angr -= startangle;
                      while (angl < 0)
                          angl += float(2 * PI);
                      while (angr < 0)
                          angr += float(2 * PI);

                      return ccw_outside ? (angl < angr) : (angl > angr);
                  });
    }

    if (nodeAndBlanksBefore.empty())
        return instr; // All Empty
    nodeAndBlanksBefore[0].second += blanks;

    wxString res;
    for (const auto& x : nodeAndBlanksBefore) {
        for (int i = 0; i < x.second; ++i)
            res += ",";
        res += wxString::Format("%d,", x.first);
    }

    return CompressNodes(res.substr(0, res.size() - 1));
}

void SubModelsDialog::OrderPoints(bool wholesub)
{
    // Collect up selection
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm->isRanges)
        return;
    if (sm->strands.empty())
        return;

    int row = NodesGrid->GetGridCursorRow();
    if (row < 0 && !wholesub)
        return;

    // Gather the coordinates
    std::map<int, std::pair<float, float>> coords;
    if (!model->GetScreenLocations(modelPreview, coords)) {
        DisplayError("Model doesn't have precisely one location per node");
        return;
    }

    // Gather centroids
    float wcx = 0, wcy = 0, smcx = 0, smcy = 0;
    std::set<int> smpts;
    for (int crow = 0; crow < int(sm->strands.size()); ++crow) {
        auto arr = wxSplit(ExpandNodes(sm->strands[crow]), ',');
        for (auto& x : arr) {
            if (x.empty() || x == "0")
                continue;
            smpts.insert(wxAtoi(x));
        }
    }
    if (smpts.empty())
        return;
    for (const auto& pt : coords) {
        wcx += pt.second.first;
        wcy += pt.second.second;
        if (smpts.count(pt.first)) {
            smcx += pt.second.first;
            smcy += pt.second.second;
        }
    }
    wcx /= coords.size();
    wcy /= coords.size();
    smcx /= smpts.size();
    smcy /= smpts.size();

    // Gather the user's requested action
    std::pair<float, float> mctr = std::make_pair(wcx, wcy);
    auto ctr = mctr;
    bool radial = false;
    float angle = 0;
    bool ccw_outside = false;
    bool strandCentroid = false;
    bool startModelRelative = false;

    wxArrayString chs;
    chs.push_back("Circumferential|Start From Model Inside|CW Around Model Center");
    chs.push_back("Circumferential|Start From Model Inside|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model Inside|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Model Inside|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Model Inside|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model Inside|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Model Outside|CW Around Model Center");
    chs.push_back("Circumferential|Start From Model Outside|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model Outside|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Model Outside|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Model Outside|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model Outside|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Model CCW|CW Around Model Center");
    chs.push_back("Circumferential|Start From Model CCW|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model CCW|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Model CCW|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Model CCW|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model CCW|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Model CW|CW Around Model Center");
    chs.push_back("Circumferential|Start From Model CW|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model CW|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Model CW|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Model CW|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Model CW|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Up|CW Around Model Center");
    chs.push_back("Circumferential|Start From Up|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Up|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Up|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Up|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Up|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Down|CW Around Model Center");
    chs.push_back("Circumferential|Start From Down|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Down|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Down|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Down|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Down|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Left|CW Around Model Center");
    chs.push_back("Circumferential|Start From Left|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Left|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Left|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Left|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Left|CCW Around Strand Center");
    chs.push_back("Circumferential|Start From Right|CW Around Model Center");
    chs.push_back("Circumferential|Start From Right|CW Around Submodel Center");
    chs.push_back("Circumferential|Start From Right|CW Around Strand Center");
    chs.push_back("Circumferential|Start From Right|CCW Around Model Center");
    chs.push_back("Circumferential|Start From Right|CCW Around Submodel Center");
    chs.push_back("Circumferential|Start From Right|CCW Around Strand Center");

    chs.push_back("Radial|From Near To Far|Model Center");
    chs.push_back("Radial|From Near To Far|Submodel Center");
    chs.push_back("Radial|From Near To Far|Strand Center");
    chs.push_back("Radial|From Far To Near|Model Center");
    chs.push_back("Radial|From Far To Near|Submodel Center");
    chs.push_back("Radial|From Far To Near|Strand Center");

    wxSingleChoiceDialog dlg(this, "Please choose direction, start/end, and centroid", "Order Type", chs);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    if (dlg.GetStringSelection() == "Yes") {
        // handleCenterNode = true;
    }

    auto choices = wxSplit(dlg.GetStringSelection(), '|');
    if (choices.size() != 3)
        return;

    if (choices[0] == "Radial") {
        radial = true;

        if (choices[2] == "Submodel Center") {
            ctr = std::make_pair(smcx, smcy);
        } else if (choices[2] == "Strand Center") {
            strandCentroid = true;
        } else if (choices[2] == "Model Center") {
            // Leave ctr how it is
        } else {
            DisplayError(wxString::Format("Unexpected radial center %s", choices[2]), this);
            return;
        }

        if (choices[1] == "From Far To Near") {
            ccw_outside = true;
        } else if (choices[1] == "From Near To Far") {
            ccw_outside = false;
        } else {
            DisplayError(wxString::Format("Unexpected radial direction %s", choices[1]), this);
            return;
        }
    } else if (choices[0] == "Circumferential") {
        // Circumferential

        if (choices[2] == "CW Around Model Center") {
            ccw_outside = false;
        } else if (choices[2] == "CCW Around Model Center") {
            ccw_outside = true;
        } else if (choices[2] == "CW Around Submodel Center") {
            ctr = std::make_pair(smcx, smcy);
            ccw_outside = false;
        } else if (choices[2] == "CCW Around Submodel Center") {
            ctr = std::make_pair(smcx, smcy);
            ccw_outside = true;
        } else if (choices[2] == "CW Around Strand Center") {
            ccw_outside = false;
            strandCentroid = true;
        } else if (choices[2] == "CCW Around Strand Center") {
            ccw_outside = true;
            strandCentroid = true;
        } else {
            DisplayError(wxString::Format("Unexpected circumferential mode %s", choices[2]), this);
            return;        
        }

        float fdlt = 0.02f;
        if (choices[1] == "Start From Up") {
            angle = float(PI / 2);
        } else if (choices[1] == "Start From Down") {
            angle = float(3 * PI / 2);
        } else if (choices[1] == "Start From Right") {
            angle = 0;
        } else if (choices[1] == "Start From Left") {
            angle = float(PI);
        } else if (choices[1] == "Start From Up") {
            angle = float(PI / 2);
        } else if (choices[1] == "Start From Model Inside") {
            angle = 0;
            startModelRelative = true;
        } else if (choices[1] == "Start From Model Outside") {
            angle = float(PI);
            startModelRelative = true;
        } else if (choices[1] == "Start From Model CW") {
            angle = float(PI / 2);
            startModelRelative = true;
        } else if (choices[1] == "Start From Model CCW") {
            angle = float(3*PI/2);
            startModelRelative = true;
        } else {
            DisplayError(wxString::Format("Unexpected circumferential start %s", choices[1]), this);
            return;
        }
        angle += ccw_outside ? -fdlt : fdlt;
    } else {
        DisplayError(wxString::Format("Unexpected mode %s", choices[0]), this);
        return;
    }

    // Perform the work
    int minr = 0;
    int maxr = sm->strands.size() - 1;
    if (!wholesub) {
        minr = maxr = row;
    }
    for (int crow = minr; crow <= maxr; ++crow) {
        auto strand = ExpandNodes(sm->strands[sm->strands.size() - 1 - crow]);

        // Calculate strand points
        float scx = 0, scy = 0;
        int scnt = 0;
        auto srr = wxSplit(strand, ',');
        for (auto s : srr) {
            if (s.empty() || s == "0")
                continue;
            ++scnt;
            int pt = wxAtoi(s);
            scx += coords[pt].first;
            scy += coords[pt].second;
        }
        if (scnt == 0)
            continue;
        auto sctr = std::make_pair(scx / scnt, scy / scnt);

        if (strandCentroid)
            ctr = sctr;

        float uangle = angle;
        if (startModelRelative) {
            float mangle = atan2f(wcy - sctr.second, wcx - sctr.first);
            if (mangle < 0)
                mangle += float(2 * PI);
            uangle += mangle;
        }

        strand = OrderPointsI(coords, strand, ctr, radial, uangle, ccw_outside);
        sm->strands[sm->strands.size() - 1 - crow] = strand;
    }

    // Update UI
    Select(GetSelectedName());
    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);
    ValidateWindow();
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
    _selected.clear();
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
    x1 = std::round(x1);
    y1 = std::round(y1);

    int nn = model->GetNodeCount();
    xlColor c(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        if (model->IsNodeInBufferRange(node, x1, y1, x2, y2)) {
            model->SetNodeColor(node, xlWHITE);
            _selected.push_back(node);
        } else {
            model->SetNodeColor(node, c);
        }
    }
    model->DisplayEffectOnWindow(modelPreview, mPointSize);
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
    _selected.clear();
    ClearNodeColor(model);

    static wxColor priorc = wxColor(255, 100, 255);
    if (r == -1) {
        for (int i = 0; i < NodesGrid->GetNumberRows(); ++i) {
            SetNodeColor(i, xlWHITE, true);
        }
    } else {
        for (int i = 0; i < NodesGrid->GetNumberRows(); ++i) {
            SetNodeColor(i, r == i ? xlWHITE : priorc, false);
        }
        // redo the selected row to ensure any duplicated nodes are highlighted
        SetNodeColor(r, xlWHITE, true);
    }
    NodesGrid->Refresh();
    model->DisplayEffectOnWindow(modelPreview, mPointSize);
}

bool SubModelsDialog::SetNodeColor(int row, xlColor const& c, bool highlight) {

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
                if (highlight) _selected.push_back(n);
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

    bool wasOutputting = StopOutputToLights();
    { // we need to scope the dialog
        SubModelInfo* sm = GetSubModelInfo(name);
        NodeSelectGrid dialog(true, name, model, sm->strands, _outputManager, this);

        if (dialog.ShowModal() == wxID_OK) {
            sm->strands = dialog.GetRowData();

            Select(GetSelectedName());
            dialog.Close();

            Panel3->SetFocus();
            NodesGrid->SetFocus();

            ValidateWindow();
        }
    }
    if (wasOutputting)
        StartOutputToLights();
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

    bool wasOutputting = StopOutputToLights();
    { // we need to scope the dialog
        const wxString title = name + " - " + NodesGrid->GetRowLabelValue(row);
        NodeSelectGrid dialog(false, title, model, sm->strands[sm->strands.size() - 1 - row], _outputManager, this);

        if (dialog.ShowModal() == wxID_OK) {
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
    if (wasOutputting)
        StartOutputToLights();
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

        SelectAllInBoundingRect(event.ShiftDown(), event.ControlDown());
        m_creating_bound_rect = false;

        modelPreview->ReleaseMouse();

        //RenderModel();
    }
}

void SubModelsDialog::OnPreviewMouseLeave(wxMouseEvent& event)
{
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

    // Capture the mouse; this will keep it selecting even if the
    //  user temporarily leaves the preview area...
    modelPreview->CaptureMouse();
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
    wxString tt = modelPreview->GetToolTipText();
    if (tt != "") {
        tt = "Node: " + tt;
    }
    NodeNumberText->SetLabel(tt);
}

void SubModelsDialog::RenderModel()
{
    if (modelPreview == nullptr || !modelPreview->StartDrawing(mPointSize)) return;

    if (m_creating_bound_rect) {
        modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
    }
    model->DisplayEffectOnWindow(modelPreview, mPointSize);
    modelPreview->EndDrawing();
}

void SubModelsDialog::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    // Trim the mouse location to the preview area
    //   (It can go outside this area if the button is down and the mouse
    //    has been captured.)
    x = std::max(x, 0);
    y = std::max(y, 0);
    x = std::min(x, modelPreview->getWidth());
    y = std::min(y, modelPreview->getHeight());

    VectorMath::ScreenPosToWorldRay(
        x, modelPreview->getHeight() - y,
        modelPreview->getWidth(), modelPreview->getHeight(),
        modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );
}

void SubModelsDialog::SelectAllInBoundingRect(bool shiftDwn, bool ctrlDown)
{
    if (shiftDwn) {
        RemoveNodes(ctrlDown);
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

void SubModelsDialog::RemoveNodes(bool suppress)
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
        if (suppress) {
            // We're going to replace the last one with space (in case it was duplicated)
            for (auto it = oldNodeArrray.rbegin(); it != oldNodeArrray.rend(); ++it) {
                if (*it == stNode) {
                    *it = "";
                    break;
                }
            }            
        } else {
            for (auto it = oldNodeArrray.begin(); it != oldNodeArrray.end(); ++it) {
                if (*it == stNode) {
                    oldNodeArrray.erase(it);
                    // Note that this only erases once, in case it somehow got added multiple times...
                    break;
                }
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
        ImportSubModelXML(root);
    }
    else
    {
        DisplayError("Failure loading xModel file.");
    }
}

void SubModelsDialog::ReadSubModelXML(wxXmlNode* xmlData)
{
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
            }
            _subModels.push_back(sm);
        }
        child = child->GetNext();
    }
    PopulateList();
    ValidateWindow();
}

void SubModelsDialog::ImportSubModelXML(wxXmlNode* xmlData)
{
    bool overRide = false;
    bool showDialog = true;
    wxXmlNode * child = xmlData->GetChildren();

    wxArrayString choices;

    while (child != nullptr) {
        if (child->GetName() == "subModel") {
           choices.push_back(child->GetAttribute("name"));
        }
        child = child->GetNext();
    }

    CheckboxSelectDialog dlg(this, "Select SubModels to Import", choices, choices);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        auto const selection = dlg.GetSelectedItems();

        child = xmlData->GetChildren();

        while (child != nullptr) {
            if (child->GetName() == "subModel") {
                wxString name = child->GetAttribute("name");

                if (-1 == selection.Index(name)) {
                    child = child->GetNext();
                    continue;
                }

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
}

void SubModelsDialog::ImportCSVSubModel(wxString const& filename)
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);

    wxTextFile f(filename);
    if (f.Open()) {
        std::list<std::string> reverse_lines;
        wxString l = f.GetFirstLine();
        while (!f.Eof()) {
            if (!l.empty()) {
                reverse_lines.push_front(CompressNodes(l));
            }
            l = f.GetNextLine();
        }
        f.Close();

        sm->strands.resize(reverse_lines.size());
        int i {0};
        for (auto const& line : reverse_lines) {
            sm->strands[i] = line;
            i++;
        }
        ValidateWindow();
        Select(name);

        Panel3->SetFocus();
        TextCtrl_Name->SetFocus();
        TextCtrl_Name->SelectAll();
    } else {
        log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("Failed to Open File %s", (const char *)filename.c_str());
    }
}

wxArrayString SubModelsDialog::getModelList(ModelManager* modelManager)
{
    wxArrayString choices;
    for (auto it = modelManager->begin(); it != modelManager->end(); ++it)
    {
        Model* m = it->second;
        if (m->Name() == model->Name()) //Skip Current Model
            continue;
        if (m->GetDisplayAs() == "ModelGroup") // skip groups as they dont have submodels
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
            for (auto & strand : a->strands) {
                strand = ReverseRow(strand);
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

void SubModelsDialog::RemoveDuplicates(bool suppress)
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();
    wxString oldnodes = ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);

    auto oldNodeArray = wxSplit(oldnodes, ',');

    if (suppress) {
        std::set<wxString> seen;
        for (auto it = oldNodeArray.begin(); it != oldNodeArray.end(); ++it) {
            if (it->empty() || *it == "0")
                continue;
            if (seen.count(*it)) {
                *it = "";
                continue;
            }
            seen.insert(*it);
        }
    } else {
        // remove duplicated
        auto end = oldNodeArray.end();
        for (auto it = oldNodeArray.begin(); it != end; ++it) {
            end = std::remove(it + 1, end, *it);
        }
        oldNodeArray.erase(end, oldNodeArray.end());
    }

    sm->strands[sm->strands.size() - 1 - row] = CompressNodes(wxJoin(oldNodeArray, ','));
    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsDialog::RemoveAllDuplicates(bool leftright, bool suppress)
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    int row = NodesGrid->GetGridCursorRow();

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy and expand data
    std::vector<wxArrayString> data;
    size_t mlen = 0;
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        data.push_back(wxSplit(ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ','));
        mlen = std::max(mlen, data.back().size());
    }

    std::set<wxString> seen;
    if (leftright) {
        for (unsigned c = 0; c < mlen; ++c) {
            for (unsigned r = 0; r < data.size(); ++r) {
                if (data[r].size() <= c)
                    continue; // Not applicable to row
                if (data[r][c] == "" || data[r][c] == "0")
                    continue;
                if (seen.count(data[r][c])) {
                    if (suppress) {
                        data[r][c] = "";
                    } else {
                        data[r][c] = "x"; // Deal with this later
                    }
                } else {
                    seen.insert(data[r][c]);
                }
            }
        }
    } else {
        for (unsigned r = 0; r < data.size(); ++r) {
            for (unsigned c = 0; c < data[r].size(); ++c) {
                if (data[r][c] == "" || data[r][c] == "0")
                    continue;
                if (seen.count(data[r][c])) {
                    if (suppress) {
                        data[r][c] = "";
                    } else {
                        data[r][c] = "x"; // Deal with this later
                    }
                } else {
                    seen.insert(data[r][c]);
                }
            }
        }
    
    }

    // Write back
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        for (auto it = data[i].begin(); it != data[i].end();) {
            if (*it == "x") {
                it = data[i].erase(it);
            } else {
                ++it;
            }
        }
        sm->strands[sm->strands.size() - 1 - i] = CompressNodes(wxJoin(data[i], ','));
    }

    // Update UI
    Select(GetSelectedName());

    if (row >= 0) {
        NodesGrid->SetGridCursor(row, 0);
    }
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsDialog::MakeRowsUniform()
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    int row = NodesGrid->GetGridCursorRow();

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy and expand data
    std::vector<wxArrayString> data;
    size_t mlen = 0; // longest length of any row
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        data.push_back(wxSplit(ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ','));
        mlen = std::max(mlen, data.back().size());
    }

    // Write back
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        wxArrayString ndata;

        int dlt = 2 * (data[i].size());
        int D = dlt - int(mlen);
        int c = 0;
        for (unsigned s = 0; s < mlen; ++s) {
            if (D > 0) {
                ndata.push_back(data[i][c]);
                ++c;
                D -= int(2 * mlen);
            } else {
                ndata.push_back("");
            }
            D += dlt;
        }

        sm->strands[sm->strands.size() - 1 - i] = CompressNodes(wxJoin(ndata, ','));
    }

    // Update UI
    Select(GetSelectedName());

    if (row >= 0) {
        NodesGrid->SetGridCursor(row, 0);
    }
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsDialog::PivotRowsColumns()
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy and expand data
    std::vector<wxArrayString> data;
    size_t mlen = 0; // max len
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        data.push_back(wxSplit(ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ','));
        mlen = std::max(mlen, data.back().size());
    }

    // Build pivot model
    std::vector<wxArrayString> ndata;
    for (unsigned i = 0; i < mlen; ++i) {
        ndata.push_back(wxArrayString());
        for (size_t j = 0; j < data.size(); ++j) {
            wxString s = "";
            if (data[j].size() > i)
                s = data[j][i];
            ndata[i].push_back(s);
        }
    }

    // Write back
    sm->strands.clear();
    for (int i = int(mlen-1); i >= 0; --i) {
        sm->strands.push_back(CompressNodes(wxJoin(ndata[i], ',')));
    }

    // Update UI
    Select(GetSelectedName());

    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsDialog::CombineStrands()
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy, expand, and concatenate data
    wxString res = "";
    for (unsigned i = 0; i < sm->strands.size(); ++i) {
        if (i != 0) {
            res += ",";
        }
        res += ExpandNodes(sm->strands[sm->strands.size() - 1 - i]);
    }

    // Write back
    sm->strands.clear();
    sm->strands.push_back(CompressNodes(res));

    // Update UI
    Select(GetSelectedName());

    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsDialog::JoinSelectedModels(bool singlestrand)
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

        for (auto const& strand : old_sm->strands) {
            if (!singlestrand || new_sm->strands.empty()) {
                new_sm->strands.push_back(strand);
            } else {
                new_sm->strands[0] = new_sm->strands[0] + "," + strand; 
            }
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

void SubModelsDialog::SplitSelectedSubmodel()
{
    wxString names = GetSelectedNames();
    wxStringTokenizer tokenizer(names, ",");

    wxString name = "";
    SubModelInfo* old_sm = nullptr;
    while (tokenizer.HasMoreTokens()) {
        wxString token = tokenizer.GetNextToken();
        int index = GetSubModelInfoIndex(token);
        if (index == -1) {
            continue;
        }
        old_sm = _subModels.at(index);
        if (old_sm == nullptr) {
            continue;
        }
        if (!old_sm->isRanges) {
            continue;
        }
        name = token;
    }
    if (name.empty())
        return; // Something wrong, didn't get one submodel

    int idx = 1;
    for (auto const& strand : old_sm->strands) {
        wxString newname = wxString::Format("%s-%02d", name, idx);
        ++idx;
        SubModelInfo* new_sm = new SubModelInfo(newname);
        new_sm->isRanges = true;
        new_sm->vertical = old_sm->vertical;
        new_sm->strands.push_back(strand);
        _subModels.push_back(new_sm);
        long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), new_sm->name);
        ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)new_sm);
    }

    ValidateWindow();
    Select(name); // Keep old one selected... the other option would be seect all new ones

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

void SubModelsDialog::OnTimer1Trigger(wxTimerEvent& event)
{
    wxASSERT(_outputManager->IsOutputting());
    _outputManager->StartFrame(0);
    for (uint32_t n = 0; n < model->GetNodeCount(); ++n) {
        auto ch = model->NodeStartChannel(n);
        if (std::find(begin(_selected), end(_selected), n) != end(_selected)) {
            for (uint8_t c = 0; c < model->GetChanCountPerNode(); ++c) {
                _outputManager->SetOneChannel(ch++, 30);
            }
        } else {
            for (uint8_t c = 0; c < model->GetChanCountPerNode(); ++c) {
                _outputManager->SetOneChannel(ch++, 0);
            }
        }
    }
    _outputManager->EndFrame();
}

void SubModelsDialog::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        _outputManager->StartOutput();
        timer1.SetOwner(this, ID_TIMER1);
        Connect(ID_TIMER1, wxEVT_TIMER, (wxObjectEventFunction)&SubModelsDialog::OnTimer1Trigger);
        timer1.Start(50, false);
    }
}

bool SubModelsDialog::StopOutputToLights()
{
    if (timer1.IsRunning()) {
        timer1.Stop();
        _outputManager->StartFrame(0);
        _outputManager->AllOff();
        _outputManager->EndFrame();
        _outputManager->StopOutput();
        return true;
    }
    return false;
}

void SubModelsDialog::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    } else {
        StopOutputToLights();
    }
}

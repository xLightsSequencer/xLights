/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

 #include <wx/settings.h>
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

#include <wx/dnd.h>
#include <wx/menu.h>
#include <wx/mimetype.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/spinctrl.h>
#include <wx/radiobox.h>
#include <wx/tokenzr.h>
#include <wx/textfile.h>

#include <algorithm>

#include "SubModelsPanel.h"
#include <wx/progdlg.h>
#include "models/Model.h"
#include "models/CustomModel.h"
#include "sequencer/SubBufferPanel.h"
#include "model/SubModelGenerateDialog.h"
#include "model/EditSubmodelAliasesDialog.h"
#include "model/NodeSelectGrid.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"
#include "xLightsApp.h"
#include "models/ModelManager.h"
#include "xLightsMain.h"
#include "layout/ModelPreview.h"
#include "render/DimmingCurve.h"
#include "model/AlignmentDialog.h"
#include "utils/VectorMath.h"
#include "xLightsVersion.h"
#include "models/SubModel.h"
#include "shared/dialogs/CheckboxSelectDialog.h"
#include "import_export/VendorModelDialog.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializeFunctions.h"

#include "utils/NodeUtils.h"
#include "models/SubModelSymmetrize.h"

#include <log.h>

wxDEFINE_EVENT(EVT_SMDROP, wxCommandEvent);

const wxWindowID SubModelsPanel::ID_CHECKBOX2 = wxNewId();
const wxWindowID SubModelsPanel::ID_STATICTEXT1 = wxNewId();
const wxWindowID SubModelsPanel::ID_LISTCTRL_SUB_MODELS = wxNewId();
const wxWindowID SubModelsPanel::ID_SEARCHCTRL1 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON3 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON4 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTONCOPY = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON_EDIT = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON_IMPORT = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON10 = wxNewId();
const wxWindowID SubModelsPanel::ID_PANEL4 = wxNewId();
const wxWindowID SubModelsPanel::ID_STATICTEXT_NAME = wxNewId();
const wxWindowID SubModelsPanel::ID_TEXTCTRL_NAME = wxNewId();
const wxWindowID SubModelsPanel::ID_STATICTEXT2 = wxNewId();
const wxWindowID SubModelsPanel::ID_CHOICE_BUFFER_STYLE = wxNewId();
const wxWindowID SubModelsPanel::ID_CHECKBOX1 = wxNewId();
const wxWindowID SubModelsPanel::ID_GRID1 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON6 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON8 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON1 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON2 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON_MOVE_UP = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON_MOVE_DOWN = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON7 = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON_SORT_ROW = wxNewId();
const wxWindowID SubModelsPanel::ID_BUTTON_DRAW_MODEL = wxNewId();
const wxWindowID SubModelsPanel::ID_PANEL2 = wxNewId();
const wxWindowID SubModelsPanel::ID_PANEL3 = wxNewId();
const wxWindowID SubModelsPanel::ID_NOTEBOOK1 = wxNewId();
const wxWindowID SubModelsPanel::ID_PANEL5 = wxNewId();
// ID_PANEL1 was for ModelPreviewPanelLocation which lives in the shell, not the panel
const wxWindowID SubModelsPanel::ID_SPLITTERWINDOW1 = wxNewId();
const wxWindowID SubModelsPanel::ID_STATICTEXT3 = wxNewId();
const long SubModelsPanel::ID_TIMER1 = wxNewId();

const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_MODEL = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_STATE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_FACE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_FILE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_CUSTOM = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_CSV = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_LAYOUT = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_IMPORT_DOWNLOAD = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EXPORT_CSV = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EXPORT_XMODEL = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EXPORT_TOOTHERS = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_GENERATE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_ALIASES = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SHIFT = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SHIFT_SINGLE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_FLIP_HOR = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_FLIP_VER = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_REVERSE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_JOIN = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_JOIN_SS = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SPLIT = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SORT_BY_NAME = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_REMOVE_DUPLICATE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SUPPRESS_DUPLICATE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SORT_POINTS = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_LR = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_TB = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_LR = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_TB = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EVEN_ROWS = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EVEN_ROWS_FRONT = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EVEN_ROWS_REAR = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SYMMETRIZE = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_SORT_POINTS_ALL = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_COMBINE_STRANDS = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_EXPAND_STRANDS_ALL = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_BLANKS_AS_ZERO = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_BLANKS_AS_EMPTY = wxNewId();
const long SubModelsPanel::SUBMODEL_DIALOG_REMOVE_BLANKS_ZEROS = wxNewId();

const long SubModelsPanel::ID_BUTTON_PLAY_ANIM = wxNewId();
const long SubModelsPanel::ID_SLIDER_ANIM_SPEED = wxNewId();
const long SubModelsPanel::ID_SLIDER_ANIM_TRAIL = wxNewId();
const long SubModelsPanel::ID_ANIM_TIMER = wxNewId();


BEGIN_EVENT_TABLE(SubModelsPanel,wxPanel)
END_EVENT_TABLE()

SubModelsPanel::SubModelsPanel(wxWindow* parent, OutputManager* om) :
    wxPanel(parent, wxID_ANY),
    _outputManager(om),
    m_creating_bound_rect(false),
    m_bound_start_x(0),
    m_bound_start_y(0),
    m_bound_end_x(0),
    m_bound_end_y(0),
    mPointSize(PIXEL_SIZE_ON_DIALOGS)
{
    // --- Panel2: submodel list (left column) ---
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer10;

    Panel2 = new wxPanel(this, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
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
    NodeNumberText = new wxStaticText(Panel2, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer9->Add(NodeNumberText, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
    Panel2->SetSizer(FlexGridSizer9);

    // --- Panel3: name / TypeNotebook / animation controls (center column) ---
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxBoxSizer*      BoxSizer1;
    wxPanel*         Panel1;

    Panel3 = new wxPanel(this, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
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
    NodesGrid->CreateGrid(5,3);
    NodesGrid->SetMaxSize(wxSize(400,-1));
    NodesGrid->EnableEditing(true);
    NodesGrid->EnableGridLines(true);
    NodesGrid->SetColLabelSize(18);
    NodesGrid->SetDefaultColSize(160, true);
    NodesGrid->SetColLabelValue(0, _("Node Ranges"));
    NodesGrid->SetColLabelValue(1, _("Node Count"));
    NodesGrid->SetColLabelValue(2, _("Line Count"));
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
    Button_MoveUp = new wxButton(Panel1, ID_BUTTON_MOVE_UP, _T("^"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_MOVE_UP"));
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
    SubBufferPanelHolder = new wxPanel(TypeNotebook, ID_PANEL3, wxPoint(90,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    SubBufferSizer = new wxFlexGridSizer(1, 1, 0, 0);
    SubBufferSizer->AddGrowableCol(0);
    SubBufferSizer->AddGrowableRow(0);
    SubBufferPanelHolder->SetSizer(SubBufferSizer);
    TypeNotebook->AddPage(Panel1, _("Node Ranges"), false);
    TypeNotebook->AddPage(SubBufferPanelHolder, _("SubBuffer"), false);
    FlexGridSizer3->Add(TypeNotebook, 1, wxALL|wxEXPAND, 0);

    // Animation controls live in Panel3 (below TypeNotebook) instead of in preview area
    {
        wxStaticBox* animBox = new wxStaticBox(Panel3, wxID_ANY, _("Node Animation"));
        wxStaticBoxSizer* animSizer = new wxStaticBoxSizer(animBox, wxHORIZONTAL);
        animSizer->Add(new wxStaticText(animBox, wxID_ANY, _("Speed:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        Spin_AnimSpeed = new wxSpinCtrl(animBox, ID_SLIDER_ANIM_SPEED, wxEmptyString, wxDefaultPosition, wxSize(55, -1), wxSP_ARROW_KEYS, 1, 10, 9);
        animSizer->Add(Spin_AnimSpeed, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        animSizer->Add(new wxStaticText(animBox, wxID_ANY, _("Trail:")), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        Spin_AnimTrail = new wxSpinCtrl(animBox, ID_SLIDER_ANIM_TRAIL, wxEmptyString, wxDefaultPosition, wxSize(55, -1), wxSP_ARROW_KEYS, 1, 10, 5);
        animSizer->Add(Spin_AnimTrail, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        Button_PlayAnim = new wxButton(animBox, ID_BUTTON_PLAY_ANIM, _("Play"));
        animSizer->Add(Button_PlayAnim, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        FlexGridSizer3->Add(animSizer, 0, wxALL | wxEXPAND, 5);
    }

    Panel3->SetSizer(FlexGridSizer3);

    // --- Root sizer: Panel2 left, Panel3 center ---
    auto* rootSizer = new wxFlexGridSizer(1, 2, 0, 0);
    rootSizer->AddGrowableCol(1);
    rootSizer->AddGrowableRow(0);
    rootSizer->Add(Panel2, 1, wxEXPAND, 0);
    rootSizer->Add(Panel3, 1, wxALL|wxEXPAND, 5);
    SetSizer(rootSizer);
    rootSizer->Fit(this);

    // --- Event bindings (Bind, not Connect) ---
    Bind(wxEVT_CHECKBOX, &SubModelsPanel::OnCheckBox_OutputToLightsClick, this, ID_CHECKBOX2);
    Bind(wxEVT_LIST_BEGIN_DRAG,       &SubModelsPanel::OnListCtrl_SubModelsBeginDrag,    this, ID_LISTCTRL_SUB_MODELS);
    Bind(wxEVT_LIST_ITEM_SELECTED,    &SubModelsPanel::OnListCtrl_SubModelsItemSelect,   this, ID_LISTCTRL_SUB_MODELS);
    Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &SubModelsPanel::OnListCtrl_SubModelsItemRClick,   this, ID_LISTCTRL_SUB_MODELS);
    Bind(wxEVT_LIST_KEY_DOWN,         &SubModelsPanel::OnListCtrl_SubModelsKeyDown,      this, ID_LISTCTRL_SUB_MODELS);
    Bind(wxEVT_LIST_COL_CLICK,        &SubModelsPanel::OnListCtrl_SubModelsColumnClick,  this, ID_LISTCTRL_SUB_MODELS);
    Bind(wxEVT_TEXT_ENTER, &SubModelsPanel::OnButton_SearchClick,  this, ID_SEARCHCTRL1);
    Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &SubModelsPanel::OnButton_SearchClick, this, ID_SEARCHCTRL1);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnAddButtonClick,         this, ID_BUTTON3);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnDeleteButtonClick,      this, ID_BUTTON4);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_Sub_CopyClick,   this, ID_BUTTONCOPY);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_EditClick,       this, ID_BUTTON_EDIT);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButtonImportClick,      this, ID_BUTTON_IMPORT);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_ExportClick,     this, ID_BUTTON10);
    Bind(wxEVT_TEXT,   &SubModelsPanel::OnTextCtrl_NameText_Change, this, ID_TEXTCTRL_NAME);
    Bind(wxEVT_CHOICE, &SubModelsPanel::OnChoiceBufferStyleSelect, this, ID_CHOICE_BUFFER_STYLE);
    Bind(wxEVT_CHECKBOX, &SubModelsPanel::OnLayoutCheckboxClick,  this, ID_CHECKBOX1);
    Bind(wxEVT_GRID_CELL_RIGHT_CLICK,  &SubModelsPanel::OnNodesGridCellRightClick,  this, ID_GRID1);
    Bind(wxEVT_GRID_CELL_LEFT_DCLICK,  &SubModelsPanel::OnNodesGridCellLeftDClick,  this, ID_GRID1);
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK,  &SubModelsPanel::OnNodesGridLabelLeftClick,  this, ID_GRID1);
    Bind(wxEVT_GRID_LABEL_RIGHT_CLICK, &SubModelsPanel::OnNodesGridCellRightClick,  this, ID_GRID1);
    Bind(wxEVT_GRID_LABEL_LEFT_DCLICK, &SubModelsPanel::OnNodesGridLabelLeftDClick, this, ID_GRID1);
    Bind(wxEVT_GRID_SELECT_CELL,       &SubModelsPanel::OnNodesGridCellSelect,      this, ID_GRID1);
    Bind(wxEVT_GRID_CELL_CHANGED,      &SubModelsPanel::OnNodesGridCellChange,      this, ID_GRID1);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_ReverseNodesClick, this, ID_BUTTON6);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_ReverseRowsClick,  this, ID_BUTTON8);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnAddRowButtonClick,        this, ID_BUTTON1);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnDeleteRowButtonClick,     this, ID_BUTTON2);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_MoveUpClick,       this, ID_BUTTON_MOVE_UP);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_MoveDownClick,     this, ID_BUTTON_MOVE_DOWN);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_ReverseRowClick,   this, ID_BUTTON7);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_SortRowClick,      this, ID_BUTTON_SORT_ROW);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnButton_Draw_ModelClick,   this, ID_BUTTON_DRAW_MODEL);
    Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &SubModelsPanel::OnTypeNotebookPageChanged, this, ID_NOTEBOOK1);
    Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent&){ ApplySubmodelName(); }, ID_TEXTCTRL_NAME);
    Bind(wxEVT_BUTTON, &SubModelsPanel::OnPlayAnimClick,  this, ID_BUTTON_PLAY_ANIM);
    Bind(wxEVT_SPINCTRL, &SubModelsPanel::OnAnimSpeedChange, this, ID_SLIDER_ANIM_SPEED);
    Bind(wxEVT_TIMER, &SubModelsPanel::OnAnimTimerTick, this, ID_ANIM_TIMER);
    Bind(wxEVT_TIMER, &SubModelsPanel::OnTimer1Trigger, this, ID_TIMER1);
    Bind(EVT_SMDROP, &SubModelsPanel::OnDrop, this);

    TextCtrl_Name->Bind(wxEVT_KILL_FOCUS, &SubModelsPanel::OnTextCtrl_NameText_KillFocus, this);

    wxSize buttonSize = Button_ReverseRow->GetBestSize();
    FlexGridSizer5->SetMinSize(wxSize(buttonSize.GetWidth() + 30, -1));

    wxListItem nm0;
    nm0.SetId(0);
    nm0.SetImage(-1);
    nm0.SetAlign(wxLIST_FORMAT_LEFT);
    nm0.SetText(_("SubModel"));
    ListCtrl_SubModels->InsertColumn(0, nm0);

    NodesGrid->SetColFormatNumber(1);
    NodesGrid->SetColFormatNumber(2);
    for (int row = 0; row < NodesGrid->GetNumberRows(); row++) {
        NodesGrid->SetReadOnly(row, 1);
        NodesGrid->SetReadOnly(row, 2);
    }
    NodesGrid->SetColLabelSize(wxGRID_AUTOSIZE);
    NodesGrid->Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        evt.Skip();
        if (NodesGrid->GetNumberCols() < 3) return;
        const int scrollbarWidth = std::max(0, wxSystemSettings::GetMetric(wxSYS_VSCROLL_X, NodesGrid));
        const int available = evt.GetSize().GetWidth()
                             - NodesGrid->GetRowLabelSize()
                             - NodesGrid->GetColSize(1)
                             - NodesGrid->GetColSize(2)
                             - scrollbarWidth;
        if (available > NodesGrid->FromDIP(50))
            NodesGrid->SetColSize(0, available);
    });
    _parent = parent;

    // ModelPreview is NOT created here — injected later via SetModelPreview()

    subBufferPanel = new SubBufferPanel(SubBufferPanelHolder, false, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    subBufferPanel->SetMinSize(SubBufferSizer->GetSize());
    SubBufferSizer->Add(subBufferPanel, 1, wxALL|wxEXPAND, 2);
    SubBufferSizer->Fit(SubBufferPanelHolder);
    SubBufferSizer->SetSizeHints(SubBufferPanelHolder);
    subBufferPanel->Bind(SUBBUFFER_RANGE_CHANGED, &SubModelsPanel::OnSubBufferRangeChange, this);
    subBufferPanel->Bind(wxEVT_SIZE, &SubModelsPanel::OnSubbufferSize, this);

    _animTimer.SetOwner(this, ID_ANIM_TIMER);

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

}

void SubModelsPanel::OnActivate()
{
    _isActive = true;
    if (_modelPreview) {
        _modelPreview->Bind(wxEVT_LEFT_DOWN,    &SubModelsPanel::OnPreviewLeftDown,   this);
        _modelPreview->Bind(wxEVT_LEFT_UP,      &SubModelsPanel::OnPreviewLeftUp,     this);
        _modelPreview->Bind(wxEVT_MOTION,       &SubModelsPanel::OnPreviewMouseMove,  this);
        _modelPreview->Bind(wxEVT_LEAVE_WINDOW, &SubModelsPanel::OnPreviewMouseLeave, this);
        _modelPreview->Bind(wxEVT_LEFT_DCLICK,  &SubModelsPanel::OnPreviewLeftDClick, this);
        if (model) _modelPreview->SetModel(model);
    }
    if (NodesGrid && NodesGrid->GetNumberCols() >= 3) {
        NodesGrid->AutoSizeColumn(1);
        NodesGrid->AutoSizeColumn(2);
    }
}

void SubModelsPanel::OnDeactivate()
{
    _isActive = false;
    m_creating_bound_rect = false;
    if (_modelPreview) {
        _modelPreview->Unbind(wxEVT_LEFT_DOWN,    &SubModelsPanel::OnPreviewLeftDown,   this);
        _modelPreview->Unbind(wxEVT_LEFT_UP,      &SubModelsPanel::OnPreviewLeftUp,     this);
        _modelPreview->Unbind(wxEVT_MOTION,       &SubModelsPanel::OnPreviewMouseMove,  this);
        _modelPreview->Unbind(wxEVT_LEAVE_WINDOW, &SubModelsPanel::OnPreviewMouseLeave, this);
        _modelPreview->Unbind(wxEVT_LEFT_DCLICK,  &SubModelsPanel::OnPreviewLeftDClick, this);
    }
    // Don't leave this tab's timers driving the animation preview or the
    // real controller output once it's no longer visible.
    StopAnimation();
    StopOutputToLights();
    CheckBox_OutputToLights->SetValue(false);
}

void SubModelsPanel::SetModelPreview(ModelPreview* preview)
{
    _modelPreview = preview;
}

void SubModelsPanel::NotifyChange()
{
    if (_changeCallback) _changeCallback();
}

void SubModelsPanel::OnSubbufferSize(wxSizeEvent& event)
{
    subBufferPanel->Refresh();
}

SubModelsPanel::~SubModelsPanel()
{
    for (auto a = _subModels.begin(); a != _subModels.end(); ++a) {
        delete *a;
    }
    _subModels.clear();

    StopAnimation();
    StopOutputToLights();
}

int SubModelsPanel::CountNodesInRange(const wxString& range) {
    if (range.IsEmpty())
        return 0;

    int count = 0;
    wxStringTokenizer tokenizer(range, ",");
    while (tokenizer.HasMoreTokens()) {
        wxString nodeRange = tokenizer.GetNextToken();
        if (nodeRange.IsEmpty())
            continue;

        if (nodeRange.Contains("-")) {
            int dashPosition = nodeRange.Index('-');
            int start = wxAtoi(nodeRange.Left(dashPosition));
            int end = wxAtoi(nodeRange.Right(nodeRange.size() - dashPosition - 1));
            if (start > 0 && end > 0) {
                count += std::abs(end - start) + 1;
            }
        } else {
            if (wxAtoi(nodeRange) > 0)
                count++;
        }
    }
    return count;
}

// Counts total positions including placeholders (zeros and empty entries between commas).
// For ranges (e.g. "1-10"), each node in the range counts as a position.
int SubModelsPanel::CountLinesInRange(const wxString& range) {
    if (range.IsEmpty())
        return 0;

    int count = 0;
    wxStringTokenizer tokenizer(range, ",", wxTOKEN_RET_EMPTY_ALL);
    while (tokenizer.HasMoreTokens()) {
        wxString nodeRange = tokenizer.GetNextToken();
        if (nodeRange.Contains("-")) {
            int dashPosition = nodeRange.Index('-');
            int start = wxAtoi(nodeRange.Left(dashPosition));
            int end = wxAtoi(nodeRange.Right(nodeRange.size() - dashPosition - 1));
            if (start > 0 && end > 0) {
                count += std::abs(end - start) + 1;
            } else {
                count++;
            }
        } else {
            count++;
        }
    }
    return count;
}

//void SubModelsPanel::OnGridChar(wxKeyEvent& event)
//{
//    CallAfter(&SubModelsPanel::ValidateWindow);
//}

void SubModelsPanel::Setup(Model *m)
{
    model = m;
    if (_modelPreview) _modelPreview->SetModel(m);

    if (m->GetDefaultBufferWi() > 1 && m->GetDefaultBufferHt() > 1) {
        _isMatrix = true;
    }

    RetrieveSubModelInfo(m);
}

#pragma region helpers

SubModelsPanel::SubModelInfo *SubModelsPanel::GetSubModelInfo(const wxString &name) {
    for (int a = 0; a < (int)_subModels.size(); a++) {
        if (_subModels[a]->name == name) {
            return _subModels[a];
        }
    }
    return nullptr;
}

bool SubModelsPanel::IsItemSelected(wxListCtrl* ctrl, int item) const
{
    return ctrl->GetItemState(item, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED;
}

void SubModelsPanel::RemoveSubModelFromList(wxString name)
{
    int idx = GetSubModelInfoIndex(name);
    if (idx < 0) return;
    delete _subModels[idx];
    _subModels.erase(_subModels.begin() + idx);
}

wxString SubModelsPanel::GetSelectedName() const
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

int SubModelsPanel::GetSelectedIndex() const
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

wxString SubModelsPanel::GetSelectedNames()
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

int SubModelsPanel::GetSubModelInfoIndex(const wxString &name) {
    for (int a=0; a < (int)_subModels.size(); a++) {
        if (_subModels[a]->name == name) {
            return a;
        }
    }
    return -1;
}

#pragma endregion helpers

void SubModelsPanel::Save()
{
    SaveSubModelInfoIntoThisModel(model);
}

bool SubModelsPanel::HasChanges() const
{
    if (_aliasesChanged) return true;
    if (_originalSubModels.size() != _subModels.size()) return true;
    for (size_t i = 0; i < _subModels.size(); ++i) {
        if (_originalSubModels[i] != *_subModels[i]) return true;
    }
    return false;
}

std::vector<std::string> SubModelsPanel::GetCurrentSubModelNames() const
{
    std::vector<std::string> names;
    for (const auto* sm : _subModels) {
        names.push_back(sm->name.ToStdString());
    }
    return names;
}

std::string SubModelsPanel::GetSubModelRanges(const std::string& name) const
{
    for (const auto* sm : _subModels) {
        if (sm->name == name && sm->isRanges) {
            wxArrayString rows;
            for (const auto& strand : sm->strands) {
                if (!strand.empty()) rows.Add(wxString(strand));
            }
            return wxJoin(rows, ',', '\0').ToStdString();
        }
    }
    return {};
}

// The dialog save all SubModel changes into a SubModelInfo structure so that the original Model's submodels are
// not altered until the user decides to Save.  This function is also used to copy these submodels into other models.
void SubModelsPanel::SaveSubModelInfoIntoThisModel(Model *m)
{
    xLightsFrame* xlights = xLightsApp::GetFrame();

    // Create a list of all current aliases in the target model
    std::vector<std::pair<wxString, wxString>> submodelAliases;
    const std::vector<Model*>& subs = m->GetSubModels();
    for (auto& sub : subs) {
        const std::list<std::string> & aliases = sub->GetAliases();
        for (auto& alias : aliases) {
            submodelAliases.push_back(std::make_pair(sub->GetName(), alias));
        }
    }

    // Delete all current submodels from the target model
    m->RemoveAllSubModels();

    // Now create new submodels in the target model from the SubModelInfo
    for (auto a = _subModels.begin(); a != _subModels.end(); ++a) {
        SubModel *sm = new SubModel(m, (*a)->name, (*a)->vertical, (*a)->isRanges, (*a)->bufferStyle);
        m->AddSubmodel(sm);

        if ((*a)->isRanges) {
            if ((*a)->bufferStyle == KEEP_XY) {
                for (int x = 0; x < (int)(*a)->strands.size(); x++) {
                    sm->AddRangeXY( (*a)->strands[x] );
                }
                sm->CalcRangeXYBufferSize();
            } else { //default and stacked buffer styles
                for (int x = 0; x < (int)(*a)->strands.size(); x++) {
                    sm->AddDefaultBuffer( (*a)->strands[x] );
                }
            }
        } else {
            sm->AddSubbuffer((*a)->subBuffer);
        }

        // transfer aliases
        for (const auto& entry : submodelAliases) {
            if (entry.first == (*a)->name) {
                sm->AddAlias(entry.second.Lower());
            }
        }

        // If the submodel name has changed ... we need to rename the model
        if ((*a)->oldName != (*a)->name)
        {
            xlights->RenameModel(m->GetName() + std::string("/") + (*a)->oldName.ToStdString(), m->GetName() + std::string("/") + (*a)->name.ToStdString());
        }
    }

    std::vector<std::string> submodelOrder;
    for (auto it = _subModels.begin(); it != _subModels.end(); ++it)
    {
        submodelOrder.push_back((*it)->name);
    }

    xlights->EnsureSequenceElementsAreOrderedCorrectly(m->GetName(), submodelOrder);
}

#pragma region actions

void SubModelsPanel::OnNameChoiceSelect(wxCommandEvent& event)
{
    Select(event.GetString());
}

wxString SubModelsPanel::GenerateSubModelName(wxString basename)
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

bool SubModelsPanel::IsUniqueName(wxString const& newname) const
{
    for (int j = 0; j < ListCtrl_SubModels->GetItemCount(); j++) {
        wxString const& name = ListCtrl_SubModels->GetItemText(j);

        if (name.IsSameAs(newname)) {
            return false;
        }
    }
    return true;
}

void SubModelsPanel::OnAddButtonClick(wxCommandEvent& event)
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
    NotifyChange();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

//Copy Selected SubModel
void SubModelsPanel::OnButton_Sub_CopyClick(wxCommandEvent& event)
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
    NotifyChange();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsPanel::OnDeleteButtonClick(wxCommandEvent& event)
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
    NotifyChange();
}

void SubModelsPanel::OnButton_EditClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(SUBMODEL_DIALOG_GENERATE, "Generate Slices");
    mnu.Append(SUBMODEL_DIALOG_ALIASES, "Add/Edit Aliases");
    if (ListCtrl_SubModels->GetItemCount() > 0) {
        mnu.Append(SUBMODEL_DIALOG_FLIP_VER, "Flip All SubModels Vertical");
        mnu.Append(SUBMODEL_DIALOG_FLIP_HOR, "Flip All SubModels Horizontial");
        mnu.Append(SUBMODEL_DIALOG_SHIFT, "Shift All Nodes in All SubModels");
        mnu.Append(SUBMODEL_DIALOG_REVERSE, "Reverse All Nodes in All SubModels");
    }
    if (ListCtrl_SubModels->GetSelectedItemCount() == 1) {
        mnu.Append(SUBMODEL_DIALOG_SHIFT_SINGLE, "Shift All Nodes in Selected SubModel");
    }
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& SubModelsPanel::OnEditBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void SubModelsPanel::OnButtonImportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(SUBMODEL_DIALOG_IMPORT_MODEL, "Import SubModels From Model");
    mnu.Append(SUBMODEL_DIALOG_IMPORT_FILE, "Import SubModels From File");
    mnu.Append(SUBMODEL_DIALOG_IMPORT_LAYOUT, "Import SubModels From Layout");
    mnu.Append(SUBMODEL_DIALOG_IMPORT_DOWNLOAD, "Import SubModels From Downloads");
    mnu.Append(SUBMODEL_DIALOG_IMPORT_STATE, "Import SubModels From State");
    mnu.Append(SUBMODEL_DIALOG_IMPORT_FACE, "Import SubModels From Face");
    if (_isMatrix) {
        mnu.Append(SUBMODEL_DIALOG_IMPORT_CUSTOM, "Import Custom Model Overlay");
    }
    mnu.Append(SUBMODEL_DIALOG_IMPORT_CSV, "Import CSV as SubModel");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& SubModelsPanel::OnImportBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void SubModelsPanel::OnButton_ExportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    mnu.Append(SUBMODEL_DIALOG_EXPORT_CSV, "Export SubModels As CSV");
    if (ListCtrl_SubModels->GetSelectedItemCount() == 1) {
        mnu.Append(SUBMODEL_DIALOG_EXPORT_XMODEL, "Export SubModel As xModel");
    }
    mnu.Append(SUBMODEL_DIALOG_EXPORT_TOOTHERS, "Export SubModels To Other Model(s)");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& SubModelsPanel::OnExportBtnPopup, nullptr, this);
    PopupMenu(&mnu);
    event.Skip();
}

void SubModelsPanel::OnImportBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_IMPORT_MODEL) {
        //Import Submodels from another Model
        xLightsFrame* xlights = xLightsApp::GetFrame();
        wxArrayString choices = getModelList(&xlights->AllModels);
        wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);
        if (dlg.ShowModal() == wxID_OK) {
            Model *m = xlights->GetModel(dlg.GetStringSelection());
            // Convert SubModel objects directly to SubModelImportData
            std::vector<XmlSerialize::SubModelImportData> subModels;
            const std::vector<Model*>& sourceSubModels = m->GetSubModels();
            for (const auto& model : sourceSubModels) {
                SubModel* sub = dynamic_cast<SubModel*>(model);
                if (sub) {
                    XmlSerialize::SubModelImportData smData;
                    smData.name = sub->GetName();
                    smData.vertical = sub->IsVertical();
                    smData.isRanges = sub->IsRanges();
                    smData.bufferStyle = sub->GetSubModelBufferStyle();
                    
                    if (smData.isRanges) {
                        int num_ranges = sub->GetNumRanges();
                        for (int x = 0; x < num_ranges; ++x) {
                            smData.strands.push_back(sub->GetRange(x));
                        }
                    } else {
                        smData.subBuffer = sub->GetSubModelLines();
                    }
                    
                    subModels.push_back(std::move(smData));
                }
            }
            ImportSubModels(subModels);
            NotifyChange();
        }
    }
    else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_STATE) {
        // Import submodels from states
        // Get a list of states on this model
        const FaceStateData& states = model->GetStateInfo();
        wxArrayString choices;
        for (const auto& it : states)
        {
            // check the type is NodeRange
            bool ok = false;
            for (const auto& it2 : it.second)
            {
                if (it2.first == "Type" && it2.second == "NodeRange") {
                    ok = true;
                    break;
                }
            }

            if (ok) choices.Add(it.first);
        }
        if (choices.GetCount() > 0) {
            wxSingleChoiceDialog dlg(GetParent(), "", "Select State", choices);
            if (dlg.ShowModal() == wxID_OK) {
                const std::map<std::string, std::string>* state = nullptr;
                for (const auto& it : states) {
                    if (it.first == dlg.GetStringSelection()) {
                        state = &it.second;
                        break;
                    }
                }
                if (!state) return;

                // its just easier to find string in a std list
                std::list<std::string> choices2z;
                wxArrayString choices2;
                for (const auto& it : *state) {
                    if (it.first.ends_with("-Name")) {
                        if (std::find(choices2z.begin(), choices2z.end(), it.second) == choices2z.end()) {
                            choices2z.push_back(it.second);
                            choices2.Add(it.second);
                        }
                    }
                }
                if (choices2.GetCount() > 0) {
                    std::list<std::string> substates;
                    if (choices2.GetCount() == 1) {
                        substates.push_back(choices2[0]);
                    } else {
                        wxMultiChoiceDialog dlg2(GetParent(), "", "Select Sub-state(s)", choices2);
                        if (dlg2.ShowModal() == wxID_OK) {
                            for (auto i : dlg2.GetSelections())
                            {
                                substates.push_back(choices2[i]);
                            }
                        }
                    }
                    if (substates.size() > 0) {
                        std::list<std::string> nodes;
                        std::list<std::string> ss;
                        for (const auto& it : *state)
                        {
                            if (it.first.ends_with("-Name")) {
                                if (std::find(substates.begin(), substates.end(), it.second) != substates.end()) {
                                    ss.push_back(it.first.substr(0, it.first.size() - 5));
                                }
                            }
                        }
                        for (const auto& it : *state) {
                            if (std::find(ss.begin(), ss.end(), it.first) != ss.end()) {
                                nodes.push_back(it.second);
                            }
                        }
                        CreateSubmodel(dlg.GetStringSelection(), nodes);
                        NotifyChange();
                    }
                }
            }
        }
    } else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_FACE) {
        // Import submodels from faces
        // Get a list of faces on this model - node ones only
        const FaceStateData& faces = model->GetFaceInfo();
        wxArrayString choices;
        for (const auto& it : faces) {
            // check the type is NodeRange
            bool ok = false;
            for (const auto& it2 : it.second) {
                if (it2.first == "Type" && it2.second == "NodeRange") {
                    ok = true;
                    break;
                }
            }

            if (ok)
                choices.Add(it.first);
        }
        if (choices.GetCount() > 0) {
            wxSingleChoiceDialog dlg(GetParent(), "", "Select Face", choices);
            if (dlg.ShowModal() == wxID_OK) {
                const std::map<std::string, std::string>* face = nullptr;
                for (const auto& it : faces) {
                    if (it.first == dlg.GetStringSelection()) {
                        face = &it.second;
                        break;
                    }
                }
                if (!face) return;
                wxArrayString choices2;
                for (const auto& it : *face) {
                    if ((it.first.starts_with("Eyes") || it.first.starts_with("Mouth") || it.first.starts_with("Face")) && !it.first.ends_with("-Color") && it.second != "") {
                        choices2.Add(it.first);
                    }
                }
                wxMultiChoiceDialog dlg2(GetParent(), "", "Select face elements", choices2);
                if (dlg2.ShowModal() == wxID_OK) {
                    std::list<std::string> elements;
                    for (auto i : dlg2.GetSelections()) {
                        elements.push_back(choices2[i]);
                    }
                    if (elements.size() > 0)
                    {
                        std::list<std::string> nodes;
                        for (const auto& it : *face) {
                            if (std::find(elements.begin(), elements.end(), it.first) != elements.end()) {
                                nodes.push_back(it.second);
                            }
                        }
                        CreateSubmodel(dlg.GetStringSelection(), nodes);
                        NotifyChange();
                    }
                }
            }
        }
    } else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_FILE) {
        //Import Submodels xModel File
        wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        ImportSubModel(filename);
        NotifyChange();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_CUSTOM) {
        //Import model as a overlay on matrix
        wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        ImportCustomModel(filename);
        NotifyChange();
    } else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_LAYOUT) {
        ImportLayoutSubModel();
        NotifyChange();
    } else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_CSV) {
        wxString filename = wxFileSelector(_("Choose CSV file"), wxEmptyString, wxEmptyString, wxEmptyString, "CSV Files (*.csv)|*.csv", wxFD_OPEN);
        if (filename.IsEmpty()) return;
        ImportCSVSubModel(filename);
        NotifyChange();
    } else if (event.GetId() == SUBMODEL_DIALOG_IMPORT_DOWNLOAD) {
        wxString const filename = GetDownloadSubmodels();
        if (filename.IsEmpty()){
            return;
        }
        ImportSubModel(filename);
        NotifyChange();
    }
}

void SubModelsPanel::OnEditBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_GENERATE) {
        Generate();
    } else if (event.GetId() == SUBMODEL_DIALOG_ALIASES) {
        Aliases();
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
    else if (event.GetId() == SUBMODEL_DIALOG_SHIFT_SINGLE) {
        ShiftSingleSubmodel();
    }
    else if (event.GetId() == SUBMODEL_DIALOG_REVERSE) {
        Reverse();
    }
    NotifyChange();
}

void SubModelsPanel::OnExportBtnPopup(wxCommandEvent& event)
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

void SubModelsPanel::OnListPopup(wxCommandEvent& event)
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
    NotifyChange();
}

void SubModelsPanel::OnNodesGridCellChange(wxGridEvent& event)
{
    

    if (!shouldProcessGridCellChanged) {
        shouldProcessGridCellChanged = true;
        return;
    }
    int r = event.GetRow();
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());
    if (sm != nullptr) {
        int str = (int)sm->strands.size() - 1 - r;
        if (str < 0) {
            spdlog::critical("SubModelsPanel::OnNodesGridCellChange submodel '{}' tried to access strand {}. This should have crashed.", (const char*)GetSelectedName().c_str(), str);
            wxASSERT(false);
        } else {
            wxString newValue = NodesGrid->GetCellValue(r, 0);
            sm->strands[str] = newValue.ToStdString();
            NodesGrid->SetCellValue(r, 1, wxString::Format("%d", CountNodesInRange(newValue)));
            NodesGrid->SetCellValue(r, 2, wxString::Format("%d", CountLinesInRange(newValue)));
        }
    } else {
        spdlog::critical("SubModelsPanel::OnNodesGridCellChange submodel '{}' ... not found. This should have crashed.", (const char*)GetSelectedName().c_str());
        wxASSERT(false);
    }
    SelectRow(r);
    ValidateWindow();
    NotifyChange();
}

void SubModelsPanel::OnNodesGridCellSelect(wxGridEvent& event)
{
    shouldProcessGridCellChanged = true;
    SelectRow(event.GetRow());
    ValidateWindow();
}

void SubModelsPanel::OnLayoutCheckboxClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    SubModelInfo* sm = GetSubModelInfo(name);
    if (sm != nullptr) {
        sm->vertical = LayoutCheckbox->GetValue();
        applySubmodelRowLabels(name);
        NotifyChange();
    }
}

void SubModelsPanel::OnTypeNotebookPageChanged(wxBookCtrlEvent& event)
{
    StopAnimation();
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);
    if (sm != nullptr) {
        sm->isRanges = TypeNotebook->GetSelection() == 0;
        NotifyChange();
    }
    Select(name);
}

void SubModelsPanel::OnSubBufferRangeChange(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }
    SubModelInfo* sm = GetSubModelInfo(name);
    sm->isRanges = false;
    sm->subBuffer = event.GetString();
    DisplayRange(sm->subBuffer);
    NotifyChange();
}

void SubModelsPanel::OnChoiceBufferStyleSelect(wxCommandEvent& event)
{
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());
    if (sm != nullptr) {
        sm->bufferStyle = ChoiceBufferStyle->GetStringSelection();
        NotifyChange();
    }
}

void SubModelsPanel::OnNodesGridLabelLeftClick(wxGridEvent& event)
{
    SelectRow(event.GetRow());
    if (event.GetRow() != -1) {
        NodesGrid->GoToCell(event.GetRow(), 0);
    }
}

void SubModelsPanel::OnNodesGridCellRightClick(wxGridEvent& event)
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
    mnu.Append(SUBMODEL_DIALOG_EVEN_ROWS_FRONT, "Uniform Row Length - Pad Front");
    mnu.Append(SUBMODEL_DIALOG_EVEN_ROWS_REAR, "Uniform Row Length - Pad Rear");
    mnu.Append(SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS, "Pivot Rows / Columns");
    mnu.Append(SUBMODEL_DIALOG_SORT_POINTS_ALL, "Geometrically Sort Points All Strands...");

    mnu.AppendSeparator();
    if (model->IsCustom()) {
        mnu.Append(SUBMODEL_DIALOG_SYMMETRIZE, "Symmetrize (Rotational)");
        wxString mname = GetSelectedName();
        SubModelInfo* sm = mname.empty() ? nullptr : GetSubModelInfo(mname);
        bool hasNodes = sm != nullptr && !sm->strands.empty() &&
                        std::any_of(sm->strands.begin(), sm->strands.end(),
                                    [](const std::string& s) { return !s.empty(); });
        mnu.Enable(SUBMODEL_DIALOG_SYMMETRIZE, hasNodes);
    }
    {
        mnu.Append(SUBMODEL_DIALOG_COMBINE_STRANDS, "Combine Strands");
        wxString csname = GetSelectedName();
        SubModelInfo* cssm = csname.empty() ? nullptr : GetSubModelInfo(csname);
        bool hasMultipleStrands = cssm != nullptr && cssm->strands.size() > 1;
        mnu.Enable(SUBMODEL_DIALOG_COMBINE_STRANDS, hasMultipleStrands);
    }

    mnu.AppendSeparator();
    mnu.Append(SUBMODEL_DIALOG_EXPAND_STRANDS_ALL, "Expand All Strands");
    mnu.Append(SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL, "Compress All Strands");
    mnu.Append(SUBMODEL_DIALOG_BLANKS_AS_ZERO, "Convert Blanks To Zeros");
    mnu.Append(SUBMODEL_DIALOG_BLANKS_AS_EMPTY, "Convert Zeros To Empty");
    mnu.Append(SUBMODEL_DIALOG_REMOVE_BLANKS_ZEROS, "Remove Blanks/Zeros");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SubModelsPanel::OnNodesGridPopup, nullptr, this);
    PopupMenu(&mnu);
}

void SubModelsPanel::OnNodesGridPopup(wxCommandEvent& event)
{
    if (event.GetId() == SUBMODEL_DIALOG_REMOVE_DUPLICATE) {
        RemoveDuplicates(false);
    } else if (event.GetId() == SUBMODEL_DIALOG_SUPPRESS_DUPLICATE) {
        RemoveDuplicates(true);
    } else if (event.GetId() == SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_LR) {
        RemoveAllDuplicates(true, false);
    } else if (event.GetId() == SUBMODEL_DIALOG_REMOVE_ALL_DUPLICATE_TB) {
        RemoveAllDuplicates(false, false);
    } else if (event.GetId() == SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_LR) {
        RemoveAllDuplicates(true, true);
    } else if (event.GetId() == SUBMODEL_DIALOG_SUPPRESS_ALL_DUPLICATE_TB) {
        RemoveAllDuplicates(false, true);
    } else if (event.GetId() == SUBMODEL_DIALOG_EVEN_ROWS) {
        MakeRowsUniform();
    } else if (event.GetId() == SUBMODEL_DIALOG_EVEN_ROWS_FRONT) {
        MakeRowsUniformFront();
    } else if (event.GetId() == SUBMODEL_DIALOG_EVEN_ROWS_REAR) {
        MakeRowsUniformRear();
    } else if (event.GetId() == SUBMODEL_DIALOG_PIVOT_ROWS_COLUMNS) {
        PivotRowsColumns();
    } else if (event.GetId() == SUBMODEL_DIALOG_SYMMETRIZE) {
        Symmetrize();
    } else if (event.GetId() == SUBMODEL_DIALOG_SORT_POINTS) {
        OrderPoints(false);
    } else if (event.GetId() == SUBMODEL_DIALOG_SORT_POINTS_ALL) {
        OrderPoints(true);
    } else if (event.GetId() == SUBMODEL_DIALOG_COMBINE_STRANDS) {
        CombineStrands();
    } else if (event.GetId() == SUBMODEL_DIALOG_EXPAND_STRANDS_ALL) {
        processAllStrands([](const std::string& str) { return NodeUtils::ExpandNodes(str); });
    } else if (event.GetId() == SUBMODEL_DIALOG_COMPRESS_STRANDS_ALL) {
        processAllStrands([](const std::string& str) { return NodeUtils::CompressNodes(str); });
    } else if (event.GetId() == SUBMODEL_DIALOG_BLANKS_AS_ZERO) {
        processAllStrands([](const std::string& str) {
            std::vector<std::string> ns;
            Split(str, ',', ns);
            std::string result;
            for (auto& s : ns) {
                if (!result.empty()) result += ",";
                result += s.empty() ? "0" : s;
            }
            return result;
        });
    } else if (event.GetId() == SUBMODEL_DIALOG_BLANKS_AS_EMPTY) {
        processAllStrands([](const std::string& str) {
            std::vector<std::string> ns;
            Split(str, ',', ns);
            std::string result;
            for (auto& s : ns) {
                if (!result.empty()) result += ",";
                result += (s == "0") ? "" : s;
            }
            return result;
        });
    } else if (event.GetId() == SUBMODEL_DIALOG_REMOVE_BLANKS_ZEROS) {
        processAllStrands([](const std::string& str) {
            std::vector<std::string> ns;
            Split(str, ',', ns);
            std::string result;
            for (auto& s : ns) {
                if (s != "0" && !s.empty()) {
                    if (!result.empty()) result += ",";
                    result += s;
                }
            }
            return result;
        });
    }
    NotifyChange();
}

void SubModelsPanel::OnListCtrl_SubModelsItemRClick(wxListEvent& event)
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

        mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SubModelsPanel::OnListPopup, nullptr, this);
        PopupMenu(&mnu);
    }
}

void SubModelsPanel::OnButton_ReverseNodesClick(wxCommandEvent& event)
{
    SubModelInfo* sm = GetSubModelInfo(GetSelectedName());

    if (sm->isRanges)
    {
        for (auto it = sm->strands.begin(); it != sm->strands.end(); ++it)
        {
            *it = ReverseRow(wxString(*it)).ToStdString();
        }
        Select(GetSelectedName());
        NotifyChange();
    }
}

void SubModelsPanel::OnListCtrl_SubModelsItemSelect(wxListEvent& event)
{
    StopAnimation();
    shouldProcessGridCellChanged = false;
    if (ListCtrl_SubModels->GetSelectedItemCount() == 1)
    {
    // title update belongs in shell, not panel
        Select(GetSelectedName());
    }
}

void SubModelsPanel::OnListCtrl_SubModelsColumnClick(wxListEvent& event)
{
}

void SubModelsPanel::OnListCtrl_SubModelsKeyDown(wxListEvent& event)
{
    auto key = event.GetKeyCode();
    if (key == WXK_DELETE || key == WXK_NUMPAD_DELETE)
    {
        OnDeleteButtonClick(event);
    }
    if (key == WXK_DOWN || key == WXK_NUMPAD_DOWN) {
        int idx = GetSubModelInfoIndex(GetSelectedName());
        if (idx + 1 < (int)_subModels.size()){
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

void SubModelsPanel::ApplySubmodelName()
{
    

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
            NotifyChange();
        }
    }
    else {
        spdlog::warn("SubModelsPanel::ApplySubmodelName submodel not found for index {}.", index);
    }

    ValidateWindow();
}

void SubModelsPanel::OnTextCtrl_NameText_KillFocus(wxFocusEvent& event)
{
    ApplySubmodelName();
    event.Skip();
}

void SubModelsPanel::OnTextCtrl_NameText_Change(wxCommandEvent& event)
{
    //ApplySubmodelName();
}

void SubModelsPanel::OnButton_SearchClick(wxCommandEvent& event)
{
    int idx = GetSubModelInfoIndex(GetSelectedName());

    if (idx == (int)_subModels.size()) {
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
    
    spdlog::debug("CheckSequence: " + msg);
    if (f.IsOpened()) {
        f.Write(msg + "\r\n");
    }
}

void SubModelsPanel::Symmetrize()
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

    // Get user input — Degree of Symmetry and rotation direction, persisted between openings
    auto* config = GetXLightsConfig();
    int savedDos = (int)config->ReadLong("SymmetrizeDoS", 8);
    if (savedDos < 2 || savedDos > 100) {
        savedDos = 8;
    }
    bool savedClockwise = config->ReadBool("SymmetrizeClockwise", false);
    bool savedBottomToTop = config->ReadBool("SymmetrizeBottomToTop", false);

    wxDialog dlg(this, wxID_ANY, "Symmetrize (Rotational)");
    auto* spin = new wxSpinCtrl(&dlg, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                wxSP_ARROW_KEYS, 2, 100, savedDos);
    wxString dirChoices[] = { "Clockwise", "Counter-Clockwise" };
    auto* dirBox = new wxRadioBox(&dlg, wxID_ANY, "Direction", wxDefaultPosition, wxDefaultSize,
                                  2, dirChoices, 1, wxRA_SPECIFY_COLS);
    dirBox->SetSelection(savedClockwise ? 0 : 1);

    wxString orderChoices[] = { "Top to Bottom", "Bottom to Top" };
    auto* orderBox = new wxRadioBox(&dlg, wxID_ANY, "Build Order", wxDefaultPosition, wxDefaultSize,
                                    2, orderChoices, 1, wxRA_SPECIFY_COLS);
    orderBox->SetSelection(savedBottomToTop ? 1 : 0);
    orderBox->SetToolTip("Top to Bottom: generated strands are added after original");

    auto* dosRow = new wxBoxSizer(wxHORIZONTAL);
    dosRow->Add(new wxStaticText(&dlg, wxID_ANY, "Degree of Symmetry:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 6);
    dosRow->Add(spin, 0, wxALIGN_CENTER_VERTICAL);

    auto* main = new wxBoxSizer(wxVERTICAL);
    main->Add(dosRow, 0, wxALL, 10);
    main->Add(dirBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 10);
    main->Add(orderBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 10);
    main->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
    dlg.SetSizerAndFit(main);

    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    int dos = spin->GetValue();
    bool clockwise = (dirBox->GetSelection() == 0);
    bool bottomToTop = (orderBox->GetSelection() == 1);

    config->Write("SymmetrizeDoS", dos);
    config->Write("SymmetrizeClockwise", clockwise);
    config->Write("SymmetrizeBottomToTop", bottomToTop);

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
    _modelPreview->GetSize(&w, &h);

    LogAndWrite(f, wxString::Format("Processing model: %s", mname.c_str()));
    LogAndWrite(f, wxString::Format("Symmetrize DoS: %d", dos));
    LogAndWrite(f, wxString::Format("Model Dimensions (based on screen): %dx%d", w, h));

    //  Calculate point xys
    std::map<int, std::pair<float, float>> coords;
    if (!model->GetScreenLocations(_modelPreview, coords)) {
        DisplayError("Model doesn't have precisely one location per node");
        return;
    }
    LogAndWrite(f, wxString::Format("Number of nodes: %d", int(coords.size())));

    if (coords.empty())
        return;

    SubModelSymmetrize::Options opts;
    opts.degreeOfSymmetry = dos;
    opts.clockwise = clockwise;
    opts.bottomToTop = bottomToTop;
    opts.squarifyAspect = true;
    opts.handleCenterNode = true;

    if (SubModelSymmetrize::ShouldOfferSquarify(coords)) {
        wxArrayString chs;
        chs.push_back("Yes");
        chs.push_back("No");
        wxSingleChoiceDialog adlg(this, "Squarify aspect ratio?", "Aspect Ratio", chs);
        adlg.ShowModal();
        opts.squarifyAspect = (adlg.GetStringSelection() == "Yes");
    }

    if (coords.size() % dos == 1) {
        wxArrayString chs;
        chs.push_back("Yes");
        chs.push_back("No");
        wxSingleChoiceDialog cdlg(this, "Shoud a center node be identified?", "Center Node", chs);
        cdlg.ShowModal();
        opts.handleCenterNode = (cdlg.GetStringSelection() == "Yes");
    }

    SubModelSymmetrize::Result sym = SubModelSymmetrize::Symmetrize(coords, w, h, sm->strands, opts);

    bool fail = !sym.success;
    if (fail) {
        LogAndWrite(f, "Note the following nodes could not be matched.  Ensure that zoom in/out is reasonable, that the model is centered, and that the point locations are clean.");
        for (auto x : sym.unmatchedNodes) {
            LogAndWrite(f, wxString::Format("  Node %d", x));
        }
    } else {
        sm->strands = sym.strands;
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

void SubModelsPanel::processAllStrands(std::string (*func)(const std::string&))
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
    auto inp = Split(NodeUtils::ExpandNodes(instr.ToStdString()), ',');

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

    return NodeUtils::CompressNodes(res.substr(0, res.size() - 1));
}

void SubModelsPanel::OrderPoints(bool wholesub)
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
    if (!model->GetScreenLocations(_modelPreview, coords)) {
        DisplayError("Model doesn't have precisely one location per node");
        return;
    }

    // Gather centroids
    float wcx = 0, wcy = 0, smcx = 0, smcy = 0;
    std::set<int> smpts;
    for (int crow = 0; crow < int(sm->strands.size()); ++crow) {
        auto arr = wxSplit(NodeUtils::ExpandNodes(sm->strands[crow]), ',');
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
        auto strand = NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - crow]);

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

        strand = OrderPointsI(coords, strand, ctr, radial, uangle, ccw_outside).ToStdString();
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

void SubModelsPanel::PopulateList()
{
    ListCtrl_SubModels->Freeze();
    ListCtrl_SubModels->DeleteAllItems();

    for (int x = 0; x < (int)_subModels.size(); x++) {
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

void SubModelsPanel::ValidateWindow()
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

    Button_PlayAnim->Enable(
        _animPlaying ||
        (ListCtrl_SubModels->GetSelectedItemCount() == 1 && TypeNotebook->GetSelection() == 0)
    );
}

void SubModelsPanel::UnSelectAll()
{
    for(int i=0; i < ListCtrl_SubModels->GetItemCount(); ++i) {
        ListCtrl_SubModels->SetItemState(i, 0, wxLIST_STATE_SELECTED);
    }
}
void SubModelsPanel::applySubmodelRowLabels(const wxString &name) {
    if (name == "") { return; }
    SubModelInfo* sm = GetSubModelInfo(name);
    for (int x = sm->strands.size() - 1; x >= 0; x--) {
        int cellrow = (sm->strands.size() - 1) - x;
        if (sm->vertical) {
            if (x == 0) {
                NodesGrid->SetRowLabelValue(cellrow, "Left");
            } else if (x == (int)sm->strands.size() - 1) {
                NodesGrid->SetRowLabelValue(cellrow, "Right");
            } else {
                NodesGrid->SetRowLabelValue(cellrow, wxString::Format("Col %d", (x + 1)));
            }
        } else {
            if (x == 0) {
                NodesGrid->SetRowLabelValue(cellrow, "Bottom");
            } else if (x == (int)sm->strands.size() - 1) {
                NodesGrid->SetRowLabelValue(cellrow, "Top");
            } else {
                NodesGrid->SetRowLabelValue(cellrow, wxString::Format("Line %d", (x + 1)));
            }
        }
    }
}

void SubModelsPanel::Select(const wxString &name)
{
    if (name == "") { return; }

    SubModelInfo* sm = GetSubModelInfo(name);

    int idx = GetSubModelInfoIndex(name);

    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++) {
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
            NodesGrid->SetCellValue(cellrow, 0, sm->strands[x]);
            NodesGrid->SetCellValue(cellrow, 1, wxString::Format("%d", CountNodesInRange(sm->strands[x])));
            NodesGrid->SetReadOnly(cellrow, 1);
            NodesGrid->SetCellValue(cellrow, 2, wxString::Format("%d", CountLinesInRange(sm->strands[x])));
            NodesGrid->SetReadOnly(cellrow, 2);
        }

        applySubmodelRowLabels(name);
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

void SubModelsPanel::SelectAll(const wxString &names) {
    if (names == "") { return; }
    wxStringTokenizer tokenizer(names, ",");
    while (tokenizer.HasMoreTokens()) {
        int idx = GetSubModelInfoIndex(tokenizer.GetNextToken());
        for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); i++) {
            if (i==idx) {
                ListCtrl_SubModels->SetItemState(i, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            }
        }
    }
    ValidateWindow();
}

void SubModelsPanel::DisplayRange(const wxString &range)
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
    if (model->GetDimmingCurve()) {
        model->GetDimmingCurve()->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        if (model->IsNodeInBufferRange(node, x1, y1, x2, y2)) {
            model->SetNodeColor(node, xlWHITE);
            _selected.push_back(node);
        } else {
            model->SetNodeColor(node, c);
        }
    }
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

void SubModelsPanel::ClearNodeColor(Model *m)
{
    xlColor c(xlDARK_GREY);
    int nn = m->GetNodeCount();
    if (m->GetDimmingCurve()) {
        m->GetDimmingCurve()->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        m->SetNodeColor(node, c);
    }
}

void SubModelsPanel::SelectRow(int r) {
    _selected.clear();
    ClearNodeColor(model);

    static xlColor priorc(255, 100, 255);
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
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

bool SubModelsPanel::SetNodeColor(int row, xlColor const& c, bool highlight) {
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

void SubModelsPanel::GenerateSegment(SubModelsPanel::SubModelInfo* sm, int segments, int segment, bool horizontal, int count)
{
    if (horizontal) {
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
    } else {
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

void SubModelsPanel::MoveSelectedModelsTo(int indexTo)
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
        if (indexTo >= (int)_subModels.size()) {
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

void SubModelsPanel::Aliases()
{
    wxString submodelname = GetSelectedName();
    if (submodelname.empty()) {
        return;
    }

    // Newly created submodels only exist in SubModelInfo until the dialog is saved.
    // Flush the current state to the live model so the alias dialog can find the submodel.
    if (model->GetSubModel(submodelname) == nullptr) {
        SaveSubModelInfoIntoThisModel(model);
    }

    Model* sm = model->GetSubModel(submodelname);
    if (sm == nullptr)
        return;

    const std::list<std::string> before = sm->GetAliases();
    EditSubmodelAliasesDialog dlg(GetParent(), model, submodelname);
    if (dlg.ShowModal() == wxID_OK && sm->GetAliases() != before) {
        _aliasesChanged = true;
        NotifyChange();
    }
}

void SubModelsPanel::Generate()
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
                        sm->strands[0] = std::to_string(start);
                    }
                    else
                    {
                        sm->strands[0] = std::to_string(start) + "-" + std::to_string(end);
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
void SubModelsPanel::OnListCtrl_SubModelsBeginDrag(wxListEvent& event)
{
    if (ListCtrl_SubModels->GetSelectedItemCount() == 0) return;

    wxString drag = "SubModel";
    for (int i = 0; i < ListCtrl_SubModels->GetItemCount(); ++i)
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

void SubModelsPanel::OnDrop(wxCommandEvent& event)
{
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
            NotifyChange();
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
                    _list->SetItemState(i, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
                else
                    _list->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
            }

            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize)
            {
                if (_list->GetTopItem() > 0)
                {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem() - 1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    _list->EnsureVisible(lastItem + 1);
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
    event.SetString(data);
    event.SetExtraLong(mousePos);

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
void SubModelsPanel::OnButton_ReverseRowsClick(wxCommandEvent& event)
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
    NotifyChange();
}

void SubModelsPanel::OnButton_ReverseRowClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    sm->strands[sm->strands.size() - 1 - row] = ReverseRow(wxString(sm->strands[sm->strands.size() - 1 - row])).ToStdString();

    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
    NotifyChange();
}

void SubModelsPanel::OnButton_SortRowClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    auto oldnodes = NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);
    auto oldNodeArrray = Split(oldnodes, ',');

    std::sort(oldNodeArrray.begin(), oldNodeArrray.end(),
              [](const std::string& a, const std::string& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

    sm->strands[sm->strands.size() - 1 - row] = NodeUtils::CompressNodes(Join(oldNodeArrray, ","));

    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
    NotifyChange();
}

void SubModelsPanel::OnButton_MoveDownClick(wxCommandEvent& event)
{
    wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();

    if (NodesGrid->GetNumberRows() == 1 || row == (int)sm->strands.size() - 1 || row < 0)
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
        NotifyChange();
    }
    ValidateWindow();
}

void SubModelsPanel::OnButton_MoveUpClick(wxCommandEvent& event)
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
        NotifyChange();
    }
    ValidateWindow();
}

void SubModelsPanel::OnAddRowButtonClick(wxCommandEvent& event)
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
    NotifyChange();
}

void SubModelsPanel::OnDeleteRowButtonClick(wxCommandEvent& event)
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
    NotifyChange();
}

void SubModelsPanel::OnButton_Draw_ModelClick(wxCommandEvent& event)
{
    const wxString name = GetSelectedName();
    if (name == "") {
        return;
    }

    bool wasOutputting = StopOutputToLights();
    { // we need to scope the dialog
        SubModelInfo* sm = GetSubModelInfo(name);
        std::vector<wxString> wxStrands(sm->strands.begin(), sm->strands.end());
        NodeSelectGrid dialog(true, name, model, wxStrands, _outputManager, this);

        if (dialog.ShowModal() == wxID_OK) {
            auto wxRows = dialog.GetRowData();
            sm->strands.assign(wxRows.begin(), wxRows.end());

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

void SubModelsPanel::OnNodesGridLabelLeftDClick(wxGridEvent& event)
{
    OnNodesGridCellLeftDClick(event);
}

void SubModelsPanel::OnNodesGridCellLeftDClick(wxGridEvent& event)
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
            sm->strands[sm->strands.size() - 1 - row] = dialog.GetNodeList(false).ToStdString();

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

wxString SubModelsPanel::ReverseRow(wxString row)
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
void SubModelsPanel::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (!_isActive) return;
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;

        SelectAllInBoundingRect(event.ShiftDown(), event.ControlDown());
        m_creating_bound_rect = false;

        _modelPreview->ReleaseMouse();

        //RenderModel();
    }
}

void SubModelsPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    if (!_isActive) return;
    RenderModel();
}

void SubModelsPanel::OnPreviewLeftDown(wxMouseEvent& event)
{
    if (!_isActive) return;
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
    _modelPreview->CaptureMouse();
}

void SubModelsPanel::OnPreviewLeftDClick(wxMouseEvent& event)
{
    if (!_isActive) return;
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    int x = ray_origin.x;
    int y = ray_origin.y;
    wxString stNode = model->GetNodeNear(_modelPreview, xlPoint(x, y), false);
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
    auto const oldnodes = NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);
    std::vector<std::string> oldNodeArrray;
    if (!oldnodes.empty()) {
        oldNodeArrray = Split(oldnodes, ',');
    }

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
    sm->strands[sm->strands.size() - 1 - row] = NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',').ToStdString());

    Select(GetSelectedName());
    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);

    ValidateWindow();
}

void SubModelsPanel::OnPreviewMouseMove(wxMouseEvent& event)
{
    if (!_isActive) { event.Skip(); return; }
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
    wxString tt = _modelPreview->GetToolTipText();
    if (tt != "") {
        tt = "Node: " + tt;
    }
    NodeNumberText->SetLabel(tt);
}

void SubModelsPanel::RenderModel()
{
    if (_modelPreview == nullptr || !_modelPreview->StartDrawing(mPointSize)) return;

    if (m_creating_bound_rect) {
        _modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
    }
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
    _modelPreview->EndDrawing();
}

void SubModelsPanel::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    // Trim the mouse location to the preview area
    //   (It can go outside this area if the button is down and the mouse
    //    has been captured.)
    x = std::max(x, 0);
    y = std::max(y, 0);
    x = std::min(x, _modelPreview->getWidth());
    y = std::min(y, _modelPreview->getHeight());

    VectorMath::ScreenPosToWorldRay(
        x, _modelPreview->getHeight() - y,
        _modelPreview->getWidth(), _modelPreview->getHeight(),
        _modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );
}

void SubModelsPanel::SelectAllInBoundingRect(bool shiftDwn, bool ctrlDown)
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
    std::vector<int> nodes = model->GetNodesInBoundingBox(_modelPreview, xlPoint(m_bound_start_x, m_bound_start_y), xlPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;
    auto const oldnodes = NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);

    std::vector<std::string> oldNodeArrray;
    if (!oldnodes.empty()) {
        oldNodeArrray = Split(oldnodes, ',');
    }
    for (auto const& newNode : nodes) {
        auto const stNode = fmt::format("{}", newNode);
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

    sm->strands[sm->strands.size() - 1 - row] = NodeUtils::CompressNodes(Join(oldNodeArrray, ","));

    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);

    ValidateWindow();
    NotifyChange();
}

void SubModelsPanel::RemoveNodes(bool suppress)
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
    std::vector<int> nodes = model->GetNodesInBoundingBox(_modelPreview, xlPoint(m_bound_start_x, m_bound_start_y), xlPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;
    auto const oldnodes = NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);
    std::vector<std::string> oldNodeArrray;
    if (!oldnodes.empty()) {
        oldNodeArrray = Split(oldnodes, ',');
    }

    for (auto const& newNode : nodes) {
        auto const stNode = fmt::format("{}", newNode);
        if (suppress) {
            // We're going to replace the last one with space (in case it was duplicated)
            for (auto it = oldNodeArrray.rbegin(); it != oldNodeArrray.rend(); ++it) {
                if (*it == stNode) {
                    *it = "";
                    break;
                }
            }
        } else {
            oldNodeArrray.erase(std::remove(oldNodeArrray.begin(), oldNodeArrray.end(), stNode), oldNodeArrray.end());
        }
    }

    sm->strands[sm->strands.size() - 1 - row] = NodeUtils::CompressNodes(Join(oldNodeArrray, ","));

    Select(GetSelectedName());
    NodesGrid->SetGridCursor(row >= 0 ? row : 0, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();
    SelectRow(row >= 0 ? row : 0);
    ValidateWindow();
    NotifyChange();
}
#pragma endregion

#pragma region Import Export Code
//Import SubModel From xModel File
void SubModelsPanel::ImportSubModel(std::string filename)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.c_str());

    if (result)
    {
        pugi::xml_node root = doc.document_element();
        if (std::string_view(root.name()) == "models") {
            root = root.first_child();
        }
        ImportSubModelXML(root);
    }
    else
    {
        DisplayError("Failure loading xModel file.");
    }
}

void SubModelsPanel::RetrieveSubModelInfo(Model* model)
{

    const std::vector<Model*>& submodels = model->GetSubModels();

    for (const auto& m : submodels) {
        SubModel* sub = dynamic_cast<SubModel*>(m);
        SubModelInfo *sm = new SubModelInfo(sub->GetName());
        sm->isRanges = sub->IsRanges();
        sm->vertical = sub->IsVertical();
        sm->bufferStyle = sub->GetSubModelBufferStyle();
        sm->strands.resize(1);
        sm->strands[0] = "";
        if (sm->isRanges) {
            sm->subBuffer = "";
            int num_ranges = sub->GetNumRanges();
            sm->strands.resize(num_ranges);
            for (int x = 0; x < num_ranges; ++x) {
                sm->strands[x] = sub->GetRange(x);
            }
        } else {
            sm->subBuffer = sub->GetSubModelLines();
        }
        _subModels.push_back(sm);
    }

    _originalSubModels.clear();
    for (const auto* sm : _subModels) {
        _originalSubModels.push_back(*sm);
    }

    PopulateList();
    ValidateWindow();
}

void SubModelsPanel::ImportSubModelXML(pugi::xml_node xmlData)
{
    // Load submodels using XmlSerializer
    auto subModels = XmlSerialize::LoadSubModelsFromXml(xmlData);
    
    // Use the new data structure based method
    ImportSubModels(subModels);
}

void SubModelsPanel::ImportSubModels(const std::vector<XmlSerialize::SubModelImportData>& subModels)
{
    if (subModels.empty()) {
        return;
    }
    
    // Build list of submodel names for selection dialog
    wxArrayString choices;
    for (const auto& sm : subModels) {
        choices.push_back(sm.name);
    }
    
    CheckboxSelectDialog dlg(this, "Select SubModels to Import", choices, choices);
    OptimiseDialogPosition(&dlg);
    
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }
    
    auto const selection = dlg.GetSelectedItems();
    bool overRide = false;
    bool showDialog = true;
    
    // Import selected submodels
    for (const auto& smData : subModels) {
        wxString name = smData.name;
        
        // Skip if not selected
        if (selection.Index(name) == wxNOT_FOUND) {
            continue;
        }
        
        SubModelInfo* sm = new SubModelInfo(name);
        sm->name = name;
        sm->oldName = name;
        sm->isRanges = smData.isRanges;
        sm->vertical = smData.vertical;
        sm->subBuffer = wxString(smData.subBuffer);
        sm->bufferStyle = wxString(smData.bufferStyle);
        sm->strands = smData.strands;
        
        // Cannot have duplicate submodel names, what to do?
        if (GetSubModelInfoIndex(name) != -1) {
            // Are the submodels the same?
            SubModelInfo* prevSm = GetSubModelInfo(name);
            if (*sm == *prevSm) {
                // Skip if exactly the same
                delete sm;
                continue;
            }
            
            // Ask user what to do if different
            if (showDialog) {
                wxMessageDialog confirm(this, _("SubModel(s) with the Same Name Already Exist.\n Would you Like to Override Them ALL?"), _("Override SubModels"), wxYES_NO);
                int returnCode = confirm.ShowModal();
                if (returnCode == wxID_YES) {
                    overRide = true;
                }
                showDialog = false;
            }
            
            if (overRide) {
                RemoveSubModelFromList(name);
            } else {
                // Rename and add if not override
                sm->oldName = sm->name = GenerateSubModelName(name);
            }
        }
        
        _subModels.push_back(sm);
    }
    
    PopulateList();
    ValidateWindow();
}

void SubModelsPanel::ImportCSVSubModel(wxString const& filename)
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
                reverse_lines.push_front(NodeUtils::CompressNodes(l.ToStdString()));
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
        spdlog::warn("Failed to Open File {}", filename.ToStdString());
    }
}

wxArrayString SubModelsPanel::getModelList(ModelManager* modelManager)
{
    wxArrayString choices;
    for (auto it = modelManager->begin(); it != modelManager->end(); ++it)
    {
        Model* m = it->second;
        if (m->Name() == model->Name()) //Skip Current Model
            continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup) // skip groups as they dont have submodels
            continue;
        choices.Add(m->Name());
    }
    return choices;
}

void SubModelsPanel::FixNodes(pugi::xml_node n, const std::string& attribute, std::map<int, int>& nodeMap)
{
    std::string l = n.attribute(attribute).as_string();
    n.remove_attribute(attribute);

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
    n.append_attribute(attribute) = row.ToStdString();
}

void SubModelsPanel::CreateSubmodel(const std::string& name, const std::list<std::string>& nodes)
{
    SubModelInfo* sm = new SubModelInfo(GenerateSubModelName(name));
    sm->vertical = false;
    sm->strands.clear();
    sm->isRanges = true;

    for (const auto& n : nodes)
    {
        sm->strands.push_back(n);
    }

    _subModels.push_back(sm);
    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm);
    Select(sm->name);
    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
    ValidateWindow();
}

void SubModelsPanel::ImportCustomModel(std::string filename)
{
    std::optional<XmlSerialize::CustomModelImportData> customModelOpt;

    pugi::xml_document fileDoc;
    if (!fileDoc.load_file(filename.c_str())) {
        DisplayError("Failure loading xModel file or model is not a 2D custom model.");
        return;
    }

    pugi::xml_node root = fileDoc.document_element();
    if (std::string_view(root.name()) == "models") {
        // New-format xmodel: <models><model DisplayAs="Custom" ...>
        pugi::xml_node modelNode = root.first_child();
        if (!modelNode || std::string_view(modelNode.attribute("DisplayAs").as_string()) != "Custom") {
            DisplayError("Failure loading xModel file or model is not a 2D custom model.");
            return;
        }
        // Adapt to <custommodel> element so LoadCustomModelFromXml can parse it
        pugi::xml_document tempDoc;
        pugi::xml_node cmNode = tempDoc.append_child("custommodel");
        for (pugi::xml_attribute attr = modelNode.first_attribute(); attr; attr = attr.next_attribute()) {
            cmNode.append_attribute(attr.name()) = attr.value();
        }
        for (pugi::xml_node child = modelNode.first_child(); child; child = child.next_sibling()) {
            cmNode.append_copy(child);
        }
        customModelOpt = XmlSerialize::LoadCustomModelFromXml(cmNode);
    } else {
        customModelOpt = XmlSerialize::LoadCustomModelFromFile(filename);
    }

    if (!customModelOpt.has_value()) {
        DisplayError("Failure loading xModel file or model is not a 2D custom model.");
        return;
    }
    
    const auto& customModel = customModelOpt.value();
    
    // Check size compatibility
    int modelw = model->GetDefaultBufferWi();
    int modelh = model->GetDefaultBufferHt();
    
    if (modelw < customModel.width || modelh < customModel.height) {
        wxMessageBox("Model is too small for the custom model.");
        return;
    }
    
    // Get alignment from user
    AlignmentDialog dlg(this);
    if (modelw != customModel.width || modelh != customModel.height) {
        if (dlg.ShowModal() != wxID_OK) {
            return;
        }
    }
    
    // Calculate alignment offsets
    int xStart = CalculateAlignmentOffset((int)dlg.GetX(), modelw, customModel.width);
    int yStart = CalculateAlignmentOffset((int)dlg.GetY(), modelh, customModel.height);
    
    // Create node mapping and main submodel
    std::map<int, int> nodeMap;
    wxString name = GenerateSubModelName(customModel.name);
    SubModelInfo* sm = CreateSubModelFromCustomModelData(customModel, name, xStart, yStart, nodeMap);
    
    // Add the main submodel
    _subModels.push_back(sm);
    long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm->name);
    ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm);
    
    // Import nested submodels
    ImportCustomModelSubModels(customModel, name, nodeMap);
    
    // Import faces
    ImportCustomModelFaces(customModel, nodeMap);
    
    // Import states
    ImportCustomModelStates(customModel, nodeMap);
    
    // Finalize
    ValidateWindow();
    Select(name);
    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

// Helper: Calculate alignment offset
int SubModelsPanel::CalculateAlignmentOffset(int alignment, int targetSize, int sourceSize)
{
    // AlignmentDialog::Alignment enum values: LEFT=0, CENTRE=1, RIGHT=2, BOTTOM=0, MIDDLE=1, TOP=2
    if (alignment == 1) { // CENTRE or MIDDLE
        return (float)targetSize / 2.0 - (float)sourceSize / 2.0;
    } else if (alignment == 2) { // RIGHT or TOP
        return targetSize - sourceSize;
    }
    return 0; // LEFT or BOTTOM
}

// Helper: Create submodel from custom model data
SubModelsPanel::SubModelInfo* SubModelsPanel::CreateSubModelFromCustomModelData(
    const XmlSerialize::CustomModelImportData& customModel,
    const wxString& name,
    int xStart,
    int yStart,
    std::map<int, int>& nodeMap)
{
    SubModelInfo* sm = new SubModelInfo(name);
    sm->vertical = false;
    sm->strands.clear();
    sm->isRanges = true;
    
    const auto& data = customModel.modelData;
    
    int rnum = yStart;
    for (size_t row = 0; row < data[0].size(); ++row) {
        int cnum = xStart;
        wxString outRow = "";
        
        for (size_t col = 0; col < data[0][0].size(); ++col) {
            if (data[0][row][col] == 0) {
                outRow += ",";
            } else {
                long nn = model->GetNodeNumber(rnum, cnum);
                if (nn >= 0) {
                    outRow += wxString::Format("%d,", nn + 1);
                    nodeMap[data[0][row][col]] = nn + 1;
                } else {
                    outRow += ",";
                }
            }
            cnum++;
        }
        
        // Remove trailing comma
        if (outRow.size() > 0) {
            outRow = outRow.Left(outRow.size() - 1);
        }
        sm->strands.push_back(outRow.ToStdString());
        rnum++;
    }
    
    return sm;
}

// Helper: Import submodels from custom model
void SubModelsPanel::ImportCustomModelSubModels(
    const XmlSerialize::CustomModelImportData& customModel,
    const wxString& baseName,
    const std::map<int, int>& nodeMap)
{
    for (const auto& smData : customModel.subModels) {
        SubModelInfo* sm2 = new SubModelInfo(baseName + "-" + smData.name);
        sm2->vertical = smData.vertical;
        sm2->strands.clear();
        sm2->isRanges = smData.isRanges;
        
        if (sm2->isRanges) {
            // Remap nodes from custom model numbering to target model numbering
            for (const auto& strand : smData.strands) {
                wxString remappedStrand = RemapNodesInStrand(strand, nodeMap);
                sm2->strands.push_back(remappedStrand.ToStdString());
            }
            
            _subModels.push_back(sm2);
            long index = ListCtrl_SubModels->InsertItem(ListCtrl_SubModels->GetItemCount(), sm2->name);
            ListCtrl_SubModels->SetItemPtrData(index, (wxUIntPtr)sm2);
        } else {
            delete sm2;
        }
    }
}

// Helper: Remap nodes in a strand definition
wxString SubModelsPanel::RemapNodesInStrand(const wxString& strand, const std::map<int, int>& nodeMap)
{
    wxString row = "";
    auto ranges = wxSplit(strand, ',');
    
    for (const auto& r : ranges) {
        if (r.IsEmpty()) {
            row += ",";
        } else if (r.Contains("-")) {
            auto rg = wxSplit(r, '-');
            if (rg.size() == 2) {
                int first = wxAtoi(rg[0]);
                int last = wxAtoi(rg[1]);
                
                if (first <= last) {
                    for (int i = first; i <= last; i++) {
                        auto it = nodeMap.find(i);
                        if (it != nodeMap.end()) {
                            row += wxString::Format("%d,", it->second);
                        }
                    }
                } else {
                    for (int i = first; i >= last; i--) {
                        auto it = nodeMap.find(i);
                        if (it != nodeMap.end()) {
                            row += wxString::Format("%d,", it->second);
                        }
                    }
                }
            }
        } else {
            int nodeNum = wxAtoi(r);
            auto it = nodeMap.find(nodeNum);
            if (it != nodeMap.end()) {
                row += wxString::Format("%d,", it->second);
            }
        }
    }
    
    return row;
}

// Helper: Import faces from custom model
void SubModelsPanel::ImportCustomModelFaces(
    const XmlSerialize::CustomModelImportData& customModel,
    const std::map<int, int>& nodeMap)
{
    for (const auto& faceData : customModel.faces) {
        // Create a new face with remapped nodes
        std::map<std::string, std::string> remappedAttributes;
        
        for (const auto& [key, value] : faceData.attributes) {
            if (key == "Name" || key == "Type") {
                remappedAttributes[key] = value;
            } else {
                // Remap node values
                remappedAttributes[key] = RemapNodesInStrand(value, nodeMap).ToStdString();
            }
        }
        
        // Generate unique name if needed
        wxString fname = faceData.name;
        wxString basefname = fname;
        int suffix = 1;
        while (model->GetFaceInfo().find(fname.ToStdString()) != model->GetFaceInfo().end()) {
            fname = wxString::Format("%s-%d", basefname, suffix);
            suffix++;
        }
        remappedAttributes["Name"] = fname.ToStdString();
        
        // Use the new data structure based method
        model->AddFace(remappedAttributes);
    }
}

// Helper: Import states from custom model
void SubModelsPanel::ImportCustomModelStates(
    const XmlSerialize::CustomModelImportData& customModel,
    const std::map<int, int>& nodeMap)
{
    for (const auto& stateData : customModel.states) {
        // Create a new state with remapped nodes
        std::map<std::string, std::string> remappedAttributes;
        
        for (const auto& [key, value] : stateData.attributes) {
            if (key == "Name" || key == "Type") {
                remappedAttributes[key] = value;
            } else {
                // Remap node values
                remappedAttributes[key] = RemapNodesInStrand(value, nodeMap).ToStdString();
            }
        }
        
        // Generate unique name if needed
        wxString sname = stateData.name;
        wxString basesname = sname;
        int suffix = 1;
        while (model->GetStateInfo().find(sname.ToStdString()) != model->GetStateInfo().end()) {
            sname = wxString::Format("%s-%d", basesname, suffix);
            suffix++;
        }
        remappedAttributes["Name"] = sname.ToStdString();
        
        // Use the new data structure based method
        model->AddState(remappedAttributes);
    }
}

void SubModelsPanel::ExportSubModels(wxString const& filename)
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
                } else if (x == (int)sm->strands.size() - 1) {
                    f.Write("Top,");
                } else {
                    f.Write(wxString::Format("Line %d,", (x + 1)));
                }
                f.Write("\"" + sm->strands[x] + "\"\n");
            }
        } else {
            f.Write(",,,");
            f.Write("SubBuffer,");
            f.Write("\"" + sm->subBuffer + "\"\n");
        }
    }
    f.Close();
}

void SubModelsPanel::ExportSubModelAsxModel(wxString const& filename, const std::string& name)
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

                            std::vector<xlPoint> pts;
                            model->GetNodeCoords(n-1, pts);

                            if (pts.size() > 0) {
                                data[pts[0].y][pts[0].x] = n;
                                nodenums.push_back(n);
                            }
                        }
                    }
                }
            } else {
                int n = wxAtoi(Trim(r));
                if (n > 0) {
                    std::vector<xlPoint> pts;
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

    std::vector<std::vector<std::vector<int>>> data3d;
    data3d.push_back(data);

    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("CustomWidth=\"%d\" ", cols));
    f.Write(wxString::Format("CustomHeight=\"%d\" ", rows));
    f.Write("Depth=\"1\" ");
    f.Write("CustomModel=\"");
    f.Write(CustomModel::ToCustomModel(data3d));
    f.Write("\" ");
    f.Write("CustomModelCompressed=\"");
    f.Write(CustomModel::ToCompressed(data3d));
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", xlights_version_string));
    f.Write(" >\n");
    f.Write("</custommodel>");

    f.Close();
}
#pragma endregion

//Shift nodes  numbering 1->21, 100->120
void SubModelsPanel::Shift()
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
                    for (size_t x = 0; x < sm->strands.size(); x++) {
                        auto const oldnodes = NodeUtils::ExpandNodes(sm->strands[x]);
                        auto oldNodeArray = wxSplit(oldnodes, ',');
                        std::vector<std::string> newNodeArray;
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
                                newNodeArray.push_back( fmt::format("{}", newVal) );
                            }
                        }
                        sm->strands[x] = NodeUtils::CompressNodes(Join(newNodeArray, ","));
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

void SubModelsPanel::ShiftSingleSubmodel()
{
    wxString name = GetSelectedName();
    if (name == "")
        return;
    SubModelInfo* sm = GetSubModelInfo(name);

    long min = 1;
    long max = model->GetNodeCount();

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK) {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0) {
            if (sm->isRanges) {
                for (size_t x = 0; x < sm->strands.size(); x++) {
                    wxString oldnodes = NodeUtils::ExpandNodes(sm->strands[x]);
                    auto oldNodeArray = wxSplit(oldnodes, ',');
                    std::vector<std::string> newNodeArray;
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
                            newNodeArray.push_back(fmt::format("{}", newVal));
                        }
                    }
                    sm->strands[x] = NodeUtils::CompressNodes(Join(newNodeArray, ","));
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


void SubModelsPanel::FlipHorizontal()
{
    wxString name = GetSelectedName();

    for (auto a : _subModels) {
        if (a->isRanges) {
            for (auto & strand : a->strands) {
                strand = ReverseRow(wxString(strand)).ToStdString();
            }
        }
    }

    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsPanel::FlipVertical()
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
void SubModelsPanel::Reverse()
{
    wxString name = GetSelectedName();
    long max = model->GetNodeCount() + 1;

    for (auto sm : _subModels) {
        if (sm->isRanges) {
            for (size_t x = 0; x < sm->strands.size(); x++) {
                auto const oldnodes = NodeUtils::ExpandNodes(sm->strands[x]);
                auto oldNodeArray = wxSplit(oldnodes, ',');
                std::vector<std::string> newNodeArray;
                for (auto const& node: oldNodeArray) {
                    long val;
                    if (node.ToCLong(&val) == true) {
                        long newVal = max - val;
                        newNodeArray.push_back(fmt::format("{}", newVal));
                    }
                }
                sm->strands[x] = NodeUtils::CompressNodes(Join(newNodeArray, ","));
            }
        }
    }
    ValidateWindow();
    Select(name);

    Panel3->SetFocus();
    TextCtrl_Name->SetFocus();
    TextCtrl_Name->SelectAll();
}

void SubModelsPanel::RemoveDuplicates(bool suppress)
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);

    int row = NodesGrid->GetGridCursorRow();
    auto const oldnodes = NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - row]);

    auto oldNodeArray = Split(oldnodes, ',');

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

    sm->strands[sm->strands.size() - 1 - row] = NodeUtils::CompressNodes(Join(oldNodeArray, ","));
    Select(GetSelectedName());

    NodesGrid->SetGridCursor(row, 0);
    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsPanel::RemoveAllDuplicates(bool leftright, bool suppress)
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    int const row = NodesGrid->GetGridCursorRow();

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy and expand data
    std::vector<std::vector<std::string>> data;
    size_t mlen = 0;
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        data.push_back(Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ','));
        mlen = std::max(mlen, data.back().size());
    }

    std::set<std::string> seen;
    if (leftright) {
        for (size_t c = 0; c < mlen; ++c) {
            for (size_t r = 0; r < data.size(); ++r) {
                if (data[r].size() <= c) {
                    continue; // Not applicable to row
                }
                if (data[r][c] == "" || data[r][c] == "0") {
                    continue;
                }
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
        for (size_t r = 0; r < data.size(); ++r) {
            for (size_t c = 0; c < data[r].size(); ++c) {
                if (data[r][c] == "" || data[r][c] == "0") {
                    continue;
                }
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
        sm->strands[sm->strands.size() - 1 - i] = NodeUtils::CompressNodes(Join(data[i], ","));
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

void SubModelsPanel::MakeRowsUniform()
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
    std::vector<std::vector<std::string>> data;
    size_t mlen = 0; // longest length of any row
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        data.push_back(Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ','));
        mlen = std::max(mlen, data.back().size());
    }

    // Write back
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        std::vector<std::string> ndata;

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

        sm->strands[sm->strands.size() - 1 - i] = NodeUtils::CompressNodes(Join(ndata, ","));
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

void SubModelsPanel::MakeRowsUniformFront() {
    auto const name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    auto const row = NodesGrid->GetGridCursorRow();
    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm) {
        return;
    }

    size_t mlen = 0; // longest length of any row
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        auto row_data = Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ',');
        mlen = std::max(mlen, row_data.size());
    }

    // Write back
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        auto row_data = Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ',');
        int const dlt = (int)(mlen - row_data.size());
        for (int s = 0; s < dlt; ++s) {
            row_data.insert(row_data.begin(), "");
        }
        sm->strands[sm->strands.size() - 1 - i] = NodeUtils::CompressNodes(Join(row_data, ","));
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

void SubModelsPanel::MakeRowsUniformRear() {
    auto const name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    auto const row = NodesGrid->GetGridCursorRow();
    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm){
        return;
    }

    size_t mlen = 0; // longest length of any row
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        auto row_data = Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ',');
        mlen = std::max(mlen, row_data.size());
    }

    // Write back
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        auto row_data = Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ',');
        auto const dlt = mlen - row_data.size();
        for (unsigned s = 0; s < dlt; ++s) {
            row_data.push_back("");
        }
        sm->strands[sm->strands.size() - 1 - i] = NodeUtils::CompressNodes(Join(row_data, ","));
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

void SubModelsPanel::PivotRowsColumns()
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy and expand data
    std::vector<std::vector<std::string>> data;
    size_t mlen = 0; // max len
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        data.push_back(Split(NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]), ','));
        mlen = std::max(mlen, data.back().size());
    }

    // Build pivot model
    std::vector<std::vector<std::string>> ndata;
    for (size_t i = 0; i < mlen; ++i) {
        ndata.push_back(std::vector<std::string>());
        for (size_t j = 0; j < data.size(); ++j) {
            std::string s;
            if (data[j].size() > i)
                s = data[j][i];
            ndata[i].push_back(s);
        }
    }

    // Write back
    sm->strands.clear();
    for (int i = int(mlen-1); i >= 0; --i) {
        sm->strands.push_back(NodeUtils::CompressNodes(Join(ndata[i], ",")));
    }

    // Update UI
    Select(GetSelectedName());

    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsPanel::CombineStrands()
{
    wxString name = GetSelectedName();
    if (name.empty()) {
        return;
    }

    SubModelInfo* sm = GetSubModelInfo(name);
    if (!sm)
        return;

    // Copy, expand, and concatenate data
    std::string res;
    for (size_t i = 0; i < sm->strands.size(); ++i) {
        if (i != 0) {
            res += ",";
        }
        res += NodeUtils::ExpandNodes(sm->strands[sm->strands.size() - 1 - i]);
    }

    // Write back
    sm->strands.clear();
    sm->strands.push_back(NodeUtils::CompressNodes(res));

    // Update UI
    Select(GetSelectedName());

    Panel3->SetFocus();
    NodesGrid->SetFocus();

    ValidateWindow();
}

void SubModelsPanel::JoinSelectedModels(bool singlestrand)
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

void SubModelsPanel::SplitSelectedSubmodel()
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

void SubModelsPanel::SortSubModelsByName()
{
    std::sort(std::begin(_subModels), std::end(_subModels), [](SubModelInfo* a, SubModelInfo* b) {
        return (wxStringNumberAwareStringCompare((*a).name, (*b).name) == -1);
    });

    PopulateList();
    ValidateWindow();
}

void SubModelsPanel::ExportSubmodelToOtherModels()
{
    if (wxMessageBox("Are you sure you want to Export this models SubModels to Other Models?\nThis will override all the other Models existing SubModels and There is no way to undo it", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    xLightsFrame* xlights = xLightsApp::GetFrame();
    wxArrayString choices = getModelList(&xlights->AllModels);

    wxMultiChoiceDialog dlg(this, "Export SubModels to Other Models", "Choose Model(s)", choices);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        for (auto const& idx : dlg.GetSelections()) {
            Model* m = xlights->GetModel(choices.at(idx));
            SaveSubModelInfoIntoThisModel(m);
            for (auto& it : m->GetSubModels()) {
                it->IncrementChangeCount();
            }
            m->IncrementChangeCount();
            ReloadLayout = true;
        }
    }
}

void SubModelsPanel::OnTimer1Trigger(wxTimerEvent& event)
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

void SubModelsPanel::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        if (_outputManager->StartOutput()) SetConfigBool("OutputActive", true);
        timer1.SetOwner(this, ID_TIMER1);
        timer1.Start(50, false);
    }
}

bool SubModelsPanel::StopOutputToLights()
{
    if (timer1.IsRunning()) {
        timer1.Stop();
        _outputManager->StartFrame(0);
        _outputManager->AllOff();
        _outputManager->EndFrame();
        _outputManager->StopOutput();
        SetConfigBool("OutputActive", false);
        return true;
    }
    return false;
}

void SubModelsPanel::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    } else {
        StopOutputToLights();
    }
}

// OnSplitterSashPosChanging removed — SplitterWindow1 is in the shell, not the panel

void SubModelsPanel::ImportLayoutSubModel() {
#ifdef __WXOSX__
    wxString const wildcard = "*.xml";
#else
    wxString const wildcard = "xlights_rgbeffects.xml";
#endif
    wxString const filename = wxFileSelector(_("Choose RGB Effects file to import from"), wxEmptyString,
                                       XLIGHTS_RGBEFFECTS_FILE, wxEmptyString,
                                       "RGB Effects Files (xlights_rgbeffects.xml)|" + wildcard,
                                       wxFD_FILE_MUST_EXIST | wxFD_OPEN);
    if (filename.IsEmpty()) {
        return;
    }
    ReadRGBEffectsFile(filename);
}

void SubModelsPanel::ReadRGBEffectsFile(wxString const& filename) {
    pugi::xml_document _doc;
    pugi::xml_parse_result result = _doc.load_file(filename.ToStdString().c_str());
    if (result) {
        pugi::xml_node root = _doc.document_element();
        pugi::xml_node models = root.child("models");
        wxArrayString choices;
        if (models) {
            for (pugi::xml_node m = models.first_child(); m; m = m.next_sibling()) {
                wxString const mn = m.attribute("name").as_string();
                choices.Add(mn);
            }
        } else {
            return;
        }

        choices.Sort();
        wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);
        if (dlg.ShowModal() == wxID_OK) {
            for (pugi::xml_node m = models.first_child(); m; m = m.next_sibling()) {
                wxString const mn = m.attribute("name").as_string();
                if (dlg.GetStringSelection() == mn) {
                    ImportSubModelXML(m);
                    break;
                }
            }
        }
    }
}

wxString SubModelsPanel::GetDownloadSubmodels() {
    wxProgressDialog* prog = new wxProgressDialog("Model download", "Downloading models ...", 100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    prog->Show();
    prog->CenterOnParent();
    xLightsFrame* xlights = xLightsApp::GetFrame();
    VendorModelDialog dlg(this, xlights->CurrentDir);
    SetCursor(wxCURSOR_WAIT);
    if (dlg.DlgInit(prog, 0, 99)) {
        if (prog != nullptr) {
            prog->Update(100);
        }
        dlg.Button_InsertModel->SetLabelText("Select Model");
        SetCursor(wxCURSOR_DEFAULT);
        if (dlg.ShowModal() == wxID_OK) {            
            return dlg.GetModelFile();          
        }
    }
    return wxString();
}

void SubModelsPanel::ParseAnimRows()
{
    _animRows.clear();
    _animMaxSteps = 0;

    int nodeCount = (int)model->GetNodeCount();
    for (int row = 0; row < NodesGrid->GetNumberRows(); ++row) {
        wxString v = NodesGrid->GetCellValue(row, 0);
        AnimRows::value_type rowSlots;

        wxStringTokenizer wtkz(v, ",", wxTOKEN_RET_EMPTY_ALL);
        while (wtkz.HasMoreTokens()) {
            wxString token = wtkz.GetNextToken().Trim(true).Trim(false);
            if (token.Contains("-")) {
                int idx = token.Index('-');
                int startRaw = wxAtoi(token.Left(idx));
                int endRaw   = wxAtoi(token.Right(token.size() - idx - 1));
                if (startRaw > 0 && endRaw > 0) {
                    int start = startRaw - 1;
                    int end   = endRaw - 1;
                    int step  = (start <= end) ? 1 : -1;
                    for (int n = start; n != end + step; n += step) {
                        std::vector<int> slot;
                        if (n < nodeCount)
                            slot.push_back(n);
                        rowSlots.push_back(std::move(slot));
                    }
                } else {
                    rowSlots.push_back({});
                }
            } else {
                std::vector<int> slot;
                if (!token.empty()) {
                    int n = wxAtoi(token) - 1;
                    if (n >= 0 && n < nodeCount)
                        slot.push_back(n);
                }
                rowSlots.push_back(std::move(slot));
            }
        }

        _animMaxSteps = std::max(_animMaxSteps, (int)rowSlots.size());
        _animRows.push_back(std::move(rowSlots));
    }

    // build flat deduplicated node list for efficient per-tick resets
    _animSubmodelNodes.clear();
    for (auto const& row : _animRows)
        for (auto const& slot : row)
            for (int n : slot)
                _animSubmodelNodes.push_back(n);
    std::sort(_animSubmodelNodes.begin(), _animSubmodelNodes.end());
    _animSubmodelNodes.erase(
        std::unique(_animSubmodelNodes.begin(), _animSubmodelNodes.end()),
        _animSubmodelNodes.end());
}

void SubModelsPanel::StopAnimation()
{
    if (!_animPlaying)
        return;
    _animTimer.Stop();
    _animPlaying = false;
    Button_PlayAnim->SetLabel(_("Play"));
    NodesGrid->EnableEditing(true);
    SelectRow(NodesGrid->GetGridCursorRow());
}

void SubModelsPanel::OnPlayAnimClick(wxCommandEvent& event)
{
    if (_animPlaying) {
        StopAnimation();
        return;
    }

    if (TypeNotebook->GetSelection() != 0 || ListCtrl_SubModels->GetSelectedItemCount() != 1)
        return;

    ParseAnimRows();
    if (_animMaxSteps == 0)
        return;

    // one-time clear: set all nodes dark, then paint submodel white
    ClearNodeColor(model);
    for (int n : _animSubmodelNodes)
        model->SetNodeColor(n, xlWHITE);

    _animPlaying = true;
    _animStep = 0;
    _animTotalSteps = 0;
    Button_PlayAnim->SetLabel(_("Stop"));
    NodesGrid->EnableEditing(false);

    int interval = 520 - 50 * Spin_AnimSpeed->GetValue();
    _animTimer.Start(interval);
}

void SubModelsPanel::OnAnimSpeedChange(wxSpinEvent& event)
{
    if (_animPlaying) {
        int interval = 520 - 50 * Spin_AnimSpeed->GetValue();
        _animTimer.Start(interval);
    }
}

void SubModelsPanel::OnAnimTimerTick(wxTimerEvent& event)
{
    int trail = std::min(Spin_AnimTrail->GetValue(), _animTotalSteps);

    // reset only submodel nodes to white — non-submodel nodes stay dark from play-start
    for (int n : _animSubmodelNodes)
        model->SetNodeColor(n, xlWHITE);

    // paint trail first, head last — blue head fades back to white
    static const xlColor animHead(0, 120, 255);
    for (int offset = trail; offset >= 0; --offset) {
        int step = ((_animStep - offset) % _animMaxSteps + _animMaxSteps) % _animMaxSteps;
        float frac = (float)(trail - offset + 1) / (trail + 1);
        xlColor c((uint8_t)(animHead.red   * frac + 255.0f * (1.0f - frac)),
                  (uint8_t)(animHead.green * frac + 255.0f * (1.0f - frac)),
                  (uint8_t)(animHead.blue  * frac + 255.0f * (1.0f - frac)));
        for (auto const& row : _animRows) {
            if (step < (int)row.size()) {
                for (int node : row[step])
                    model->SetNodeColor(node, c);
            }
        }
    }

    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
    _animStep = (_animStep + 1) % _animMaxSteps;
    if (_animTotalSteps < Spin_AnimTrail->GetMax())
        ++_animTotalSteps;
}

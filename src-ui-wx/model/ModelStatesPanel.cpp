/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelStatesPanel.h"
#include "settings/XLightsConfigAdapter.h"
#include <wx/settings.h>
#include <wx/progdlg.h>
#include "render/SequenceFile.h"
#include "shared/utils/NodesGridCellEditor.h"
#include "layout/ModelPreview.h"
#include "render/DimmingCurve.h"
#include "model/SevenSegmentDialog.h"
#include "model/NodeSelectGrid.h"
#include "models/Model.h"
#include "models/SubModel.h"
#include "xLightsApp.h"
#include "utils/NodeUtils.h"
#include "xLightsMain.h"
#include "utils/VectorMath.h"
#include "models/CustomModel.h"
#include "utils/string_utils.h"
#include "color/xlColourData.h"
#include "import_export/VendorModelDialog.h"
#include "XmlSerializer/XmlSerializer.h"
#include "shared/utils/wxUtilities.h"

#include <log.h>
#include <algorithm>
#include <vector>

#include <wx/intl.h>
#include <wx/string.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/valtext.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/filedlg.h>
#include <wx/colordlg.h>
#include <wx/tokenzr.h>
#include <wx/listbox.h>
#include <wx/numdlg.h>

// ID definitions — use wxNewId() to avoid collisions with ModelStateDialog IDs
const wxWindowID ModelStatesPanel::ID_STATICTEXT2      = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHOICE3          = wxNewId();
const wxWindowID ModelStatesPanel::ID_BUTTON3          = wxNewId();
const wxWindowID ModelStatesPanel::ID_BUTTON_IMPORT    = wxNewId();
const wxWindowID ModelStatesPanel::ID_BUTTON4          = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHECKBOX1        = wxNewId();
const wxWindowID ModelStatesPanel::ID_BUTTON1          = wxNewId();
const wxWindowID ModelStatesPanel::ID_GRID_COROSTATES  = wxNewId();
const wxWindowID ModelStatesPanel::ID_PANEL2           = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHECKBOX2        = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHECKBOX3        = wxNewId();
const wxWindowID ModelStatesPanel::ID_BUTTON2          = wxNewId();
const wxWindowID ModelStatesPanel::ID_GRID3            = wxNewId();
const wxWindowID ModelStatesPanel::ID_PANEL6           = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHOICEBOOK1      = wxNewId();
const wxWindowID ModelStatesPanel::ID_STATICTEXT1      = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHOICE_COLOR_DRAW   = wxNewId();
const wxWindowID ModelStatesPanel::ID_CHOICE_STATE_FILTER = wxNewId();
const wxWindowID ModelStatesPanel::ID_BUTTON_PLAY_STATES  = wxNewId();
const wxWindowID ModelStatesPanel::ID_SLIDER_STATE_SPEED  = wxNewId();
const wxWindowID ModelStatesPanel::ID_STATE_ANIM_TIMER    = wxNewId();
const long       ModelStatesPanel::ID_TIMER1              = wxNewId();

const long ModelStatesPanel::STATE_DIALOG_IMPORT_SUB          = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_IMPORT_ALL_SUB      = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_COPY_STATES         = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_IMPORT_MODEL        = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_IMPORT_FILE         = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_IMPORT_DOWNLOAD     = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_COPY                = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_RENAME              = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_SHIFT               = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_REVERSE             = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_CLEAR_SELECTED_ROWS = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_CLEAR_STATES        = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_EXPORT_TOOTHERS     = wxNewId();
const long ModelStatesPanel::STATE_DIALOG_BULK_COLOR_CHANGE   = wxNewId();
const wxWindowID ModelStatesPanel::ID_MNU_ADD_STATE_BEFORE    = wxNewId();
const wxWindowID ModelStatesPanel::ID_MNU_ADD_STATE_AFTER     = wxNewId();
const wxWindowID ModelStatesPanel::ID_MNU_DELETE_STATE        = wxNewId();
const wxWindowID ModelStatesPanel::ID_MNU_MOVE_STATE_UP       = wxNewId();
const wxWindowID ModelStatesPanel::ID_MNU_MOVE_STATE_DOWN     = wxNewId();
const wxWindowID ModelStatesPanel::ID_MNU_SORT                = wxNewId();

BEGIN_EVENT_TABLE(ModelStatesPanel, wxPanel)
END_EVENT_TABLE()

enum {
    SINGLE_NODE_STATE = 0,
    NODE_RANGE_STATE
};

enum {
    NAME_COL = 0,
    CHANNEL_COL,
    COLOUR_COL
};

#ifndef wxEVT_GRID_CELL_CHANGE
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

ModelStatesPanel::ModelStatesPanel(wxWindow* parent, OutputManager* outputManager, wxWindowID id)
    : wxPanel(parent, id), _outputManager(outputManager), mPointSize(PIXEL_SIZE_ON_DIALOGS)
{
    wxButton* AddButton;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxBoxSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxPanel* CoroPanel;
    wxPanel* NodeRangePanel;

    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(1);

    FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer7->AddGrowableCol(1);

    StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    NameChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    NameChoice->SetMinSize(wxSize(50, -1));
    FlexGridSizer7->Add(NameChoice, 1, wxALL|wxEXPAND, 5);

    AddButton = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(AddButton, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);

    Button_Import = new wxButton(this, ID_BUTTON_IMPORT, _T("..."), wxDefaultPosition, wxSize(20, -1), 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
    Button_Import->SetMinSize(wxSize(20, -1));
    FlexGridSizer7->Add(Button_Import, 1, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);

    DeleteButton = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer7->Add(DeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);

    FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);

    StateTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));

    // Single Nodes page
    CoroPanel = new wxPanel(StateTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(1);

    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);

    CustomColorSingleNode = new wxCheckBox(CoroPanel, ID_CHECKBOX1, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CustomColorSingleNode->SetValue(false);
    FlexGridSizer3->Add(CustomColorSingleNode, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);

    FlexGridSizer3->Add(-1, -1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    Button_7Segment = new wxButton(CoroPanel, ID_BUTTON1, _("7 Segment Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_7Segment, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);

    SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROSTATES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROSTATES"));
    SingleNodeGrid->CreateGrid(200, 3);
    SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel, wxSize(-1, 200)));
    SingleNodeGrid->EnableEditing(true);
    SingleNodeGrid->EnableGridLines(true);
    SingleNodeGrid->SetColLabelSize(20);
    SingleNodeGrid->SetDefaultColSize(150, true);
    SingleNodeGrid->SetColLabelValue(0, _("State"));
    SingleNodeGrid->SetColLabelValue(1, _("Nodes"));
    SingleNodeGrid->SetColLabelValue(2, _("Color"));
    SingleNodeGrid->SetDefaultCellFont(SingleNodeGrid->GetFont());
    SingleNodeGrid->SetDefaultCellTextColour(SingleNodeGrid->GetForegroundColour());
    FlexGridSizer2->Add(SingleNodeGrid, 1, wxALL|wxEXPAND, 5);
    CoroPanel->SetSizer(FlexGridSizer2);

    // Node Ranges page
    NodeRangePanel = new wxPanel(StateTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    FlexGridSizer5->AddGrowableRow(1);

    // 7-column toolbar: checkboxes vertically centred; Model Display / States / Speed
    // each get a label-on-top / control-on-bottom sub-column.
    FlexGridSizer6 = new wxBoxSizer(wxHORIZONTAL);

    // Col 1: Force Custom Colors + Output to Lights stacked
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        CustomColorNodeRanges = new wxCheckBox(NodeRangePanel, ID_CHECKBOX2, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
        CustomColorNodeRanges->SetValue(false);
        col->Add(CustomColorNodeRanges, 0, wxALL|wxALIGN_LEFT, 3);
        CheckBox_OutputToLights = new wxCheckBox(NodeRangePanel, ID_CHECKBOX3, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
        CheckBox_OutputToLights->SetValue(false);
        col->Add(CheckBox_OutputToLights, 0, wxALL|wxALIGN_LEFT, 3);
        FlexGridSizer6->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    }

    // Col 3: Model Display (label / dropdown)
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        col->Add(new wxStaticText(NodeRangePanel, ID_STATICTEXT1, _("Model Display:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1")), 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 2);
        ChoiceColorDraw = new wxChoice(NodeRangePanel, ID_CHOICE_COLOR_DRAW, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_COLOR_DRAW"));
        ChoiceColorDraw->Append(_("All Colors"));
        ChoiceColorDraw->Append(_("White Only"));
        ChoiceColorDraw->SetSelection(0);
        col->Add(ChoiceColorDraw, 0, wxEXPAND, 0);
        FlexGridSizer6->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    }

    // Col 4: States (label / dropdown)
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        col->Add(new wxStaticText(NodeRangePanel, wxID_ANY, _("Select State"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 2);
        StateFilterChoice = new wxChoice(NodeRangePanel, ID_CHOICE_STATE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_STATE_FILTER"));
        StateFilterChoice->Append("");
        StateFilterChoice->SetSelection(0);
        col->Add(StateFilterChoice, 0, wxEXPAND, 0);
        FlexGridSizer6->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    }

    // Col 5: Speed (label / spinctrl)
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        col->Add(new wxStaticText(NodeRangePanel, wxID_ANY, _("Speed (ms):"), wxDefaultPosition, wxDefaultSize, 0), 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 2);
        Spin_StateSpeed = new wxSpinCtrl(NodeRangePanel, ID_SLIDER_STATE_SPEED, wxEmptyString, wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS, 50, 5000, 500);
        col->Add(Spin_StateSpeed, 0, wxEXPAND, 0);
        FlexGridSizer6->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    }

    // Col 6: Play
    Button_PlayStates = new wxButton(NodeRangePanel, ID_BUTTON_PLAY_STATES, _("Play"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PLAY_STATES"));
    FlexGridSizer6->Add(Button_PlayStates, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer6->AddStretchSpacer(1);

    // Col 7: 7 Segment Display
    Button_7Seg = new wxButton(NodeRangePanel, ID_BUTTON2, _("7 Segment Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer6->Add(Button_7Seg, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer5->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);

    NodeRangeGrid = new wxGrid(NodeRangePanel, ID_GRID3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID3"));
    NodeRangeGrid->CreateGrid(200, 3);
    NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel, wxSize(-1, 200)));
    NodeRangeGrid->EnableEditing(true);
    NodeRangeGrid->EnableGridLines(true);
    NodeRangeGrid->SetColLabelSize(20);
    NodeRangeGrid->SetDefaultColSize(150, true);
    NodeRangeGrid->SetColLabelValue(0, _("State"));
    NodeRangeGrid->SetColLabelValue(1, _("Nodes"));
    NodeRangeGrid->SetColLabelValue(2, _("Color"));
    NodeRangeGrid->SetDefaultCellFont(NodeRangeGrid->GetFont());
    NodeRangeGrid->SetDefaultCellTextColour(NodeRangeGrid->GetForegroundColour());
    FlexGridSizer5->Add(NodeRangeGrid, 1, wxALL|wxEXPAND, 5);
    NodeRangePanel->SetSizer(FlexGridSizer5);

    StateTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
    StateTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
    FlexGridSizer4->Add(StateTypeChoice, 1, wxALL|wxEXPAND, 5);

    this->SetSizer(FlexGridSizer4);
    this->Layout();

    // Wire up events using Bind() — not Connect()
    Bind(wxEVT_COMMAND_CHOICE_SELECTED,      &ModelStatesPanel::OnMatrixNameChoiceSelect,    this, ID_CHOICE3);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,       &ModelStatesPanel::OnButtonMatrixAddClicked,    this, ID_BUTTON3);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,       &ModelStatesPanel::OnButton_ImportClick,        this, ID_BUTTON_IMPORT);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,       &ModelStatesPanel::OnButtonMatrixDeleteClick,   this, ID_BUTTON4);
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED,     &ModelStatesPanel::OnCustomColorCheckboxClick,  this, ID_CHECKBOX1);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,       &ModelStatesPanel::OnButton_7SegmentClick,      this, ID_BUTTON1);
    Bind(wxEVT_GRID_CELL_LEFT_CLICK,         &ModelStatesPanel::OnSingleNodeGridCellLeftClick,  this, ID_GRID_COROSTATES);
    Bind(wxEVT_GRID_CELL_LEFT_DCLICK,        &ModelStatesPanel::OnSingleNodeGridCellLeftDClick, this, ID_GRID_COROSTATES);
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK,        &ModelStatesPanel::OnSingleNodeGridLabelLeftClick, this, ID_GRID_COROSTATES);
    Bind(wxEVT_GRID_CELL_CHANGED,            &ModelStatesPanel::OnSingleNodeGridCellChange,     this, ID_GRID_COROSTATES);
    Bind(wxEVT_GRID_SELECT_CELL,             &ModelStatesPanel::OnSingleNodeGridCellSelect,     this, ID_GRID_COROSTATES);
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED,     &ModelStatesPanel::OnCustomColorCheckboxClick,  this, ID_CHECKBOX2);
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED,     &ModelStatesPanel::OnCheckBox_OutputToLightsClick, this, ID_CHECKBOX3);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED,      &ModelStatesPanel::OnChoiceColorDrawSelect,     this, ID_CHOICE_COLOR_DRAW);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED,      &ModelStatesPanel::OnStateFilterSelect,         this, ID_CHOICE_STATE_FILTER);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,       &ModelStatesPanel::OnPlayStatesClick,           this, ID_BUTTON_PLAY_STATES);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,       &ModelStatesPanel::OnButton_7SegmentClick,      this, ID_BUTTON2);
    Bind(wxEVT_GRID_CELL_LEFT_CLICK,         &ModelStatesPanel::OnNodeRangeGridCellLeftClick,    this, ID_GRID3);
    Bind(wxEVT_GRID_CELL_RIGHT_CLICK,        &ModelStatesPanel::OnNodeRangeGridCellRightClick,   this, ID_GRID3);
    Bind(wxEVT_GRID_CELL_LEFT_DCLICK,        &ModelStatesPanel::OnNodeRangeGridCellLeftDClick,   this, ID_GRID3);
    Bind(wxEVT_GRID_LABEL_LEFT_CLICK,        &ModelStatesPanel::OnNodeRangeGridLabelLeftClick,   this, ID_GRID3);
    Bind(wxEVT_GRID_LABEL_LEFT_DCLICK,       &ModelStatesPanel::OnNodeRangeGridLabelLeftDClick,  this, ID_GRID3);
    Bind(wxEVT_GRID_CELL_CHANGED,            &ModelStatesPanel::OnNodeRangeGridCellChange,       this, ID_GRID3);
    Bind(wxEVT_GRID_SELECT_CELL,             &ModelStatesPanel::OnNodeRangeGridCellSelect,       this, ID_GRID3);
    Bind(wxEVT_GRID_RANGE_SELECT,            &ModelStatesPanel::OnNodeRangeGridRangeSelect,      this, ID_GRID3);
    Bind(wxEVT_GRID_RANGE_SELECT,            &ModelStatesPanel::OnSingleNodeGridRangeSelect,     this, ID_GRID_COROSTATES);
    Bind(wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED, &ModelStatesPanel::OnStateTypeChoicePageChanged, this, ID_CHOICEBOOK1);
    SingleNodeGrid->Bind(wxEVT_SIZE, &ModelStatesPanel::OnSingleNodeGridResize, this);
    NodeRangeGrid->Bind(wxEVT_SIZE,  &ModelStatesPanel::OnNodeRangeGridResize,  this);

    model = nullptr;
    StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);

    // Wire timers
    timer1.SetOwner(this, ID_TIMER1);
    Bind(wxEVT_TIMER, &ModelStatesPanel::OnTimer1Trigger,       this, ID_TIMER1);
    _stateAnimTimer.SetOwner(this, ID_STATE_ANIM_TIMER);
    Bind(wxEVT_TIMER, &ModelStatesPanel::OnStateAnimTimerTick,  this, ID_STATE_ANIM_TIMER);

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
        SetConfigBool("OutputActive", false);
    }
}

ModelStatesPanel::~ModelStatesPanel()
{
    StopStateAnimation();
    StopOutputToLights();
    if (_oldOutputToLights) {
        if (_outputManager->StartOutput()) SetConfigBool("OutputActive", true);
    }
}

void ModelStatesPanel::SetModelPreview(ModelPreview* preview)
{
    _modelPreview = preview;
}

void ModelStatesPanel::NotifyChange()
{
    if (_changeCallback) _changeCallback();
}

void ModelStatesPanel::SetSubModelCallbacks(std::function<std::vector<std::string>()> getNames,
                                            std::function<std::string(const std::string&)> getRanges)
{
    _getSubModelNames = std::move(getNames);
    _getSubModelRanges = std::move(getRanges);
}

void ModelStatesPanel::OnActivate()
{
    _isActive = true;
    if (_modelPreview) {
        _modelPreview->Bind(wxEVT_LEFT_DOWN,    &ModelStatesPanel::OnPreviewLeftDown,   this);
        _modelPreview->Bind(wxEVT_LEFT_UP,      &ModelStatesPanel::OnPreviewLeftUp,     this);
        _modelPreview->Bind(wxEVT_MOTION,       &ModelStatesPanel::OnPreviewMouseMove,  this);
        _modelPreview->Bind(wxEVT_LEAVE_WINDOW, &ModelStatesPanel::OnPreviewMouseLeave, this);
        _modelPreview->Bind(wxEVT_LEFT_DCLICK,  &ModelStatesPanel::OnPreviewLeftDClick, this);
        if (model) {
            _modelPreview->SetModel(model);
            CallAfter([this]() {
                if (_modelPreview && model) {
                    auto grid = (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) ? SingleNodeGrid : NodeRangeGrid;
                    SelectRow(grid, -1);
                }
            });
        }
    }
}

void ModelStatesPanel::OnDeactivate()
{
    _isActive = false;
    m_creating_bound_rect = false;
    if (_modelPreview) {
        _modelPreview->Unbind(wxEVT_LEFT_DOWN,    &ModelStatesPanel::OnPreviewLeftDown,   this);
        _modelPreview->Unbind(wxEVT_LEFT_UP,      &ModelStatesPanel::OnPreviewLeftUp,     this);
        _modelPreview->Unbind(wxEVT_MOTION,       &ModelStatesPanel::OnPreviewMouseMove,  this);
        _modelPreview->Unbind(wxEVT_LEAVE_WINDOW, &ModelStatesPanel::OnPreviewMouseLeave, this);
        _modelPreview->Unbind(wxEVT_LEFT_DCLICK,  &ModelStatesPanel::OnPreviewLeftDClick, this);
    }
}

void ModelStatesPanel::OnSingleNodeGridResize(wxSizeEvent& event)
{
    event.Skip();
    SingleNodeGrid->SetColSize(CHANNEL_COL, std::max(event.GetSize().x - SingleNodeGrid->GetColSize(NAME_COL) - SingleNodeGrid->GetColSize(COLOUR_COL) - 30 - SingleNodeGrid->GetRowLabelSize(), 10));
}

void ModelStatesPanel::OnNodeRangeGridResize(wxSizeEvent& event)
{
    event.Skip();
    NodeRangeGrid->SetColSize(CHANNEL_COL, std::max(event.GetSize().x - NodeRangeGrid->GetColSize(NAME_COL) - NodeRangeGrid->GetColSize(COLOUR_COL) - 30 - NodeRangeGrid->GetRowLabelSize(), 10));
}

void ModelStatesPanel::SetStateInfo(Model* cls, std::map<std::string, std::map<std::string, std::string>> const& finfo)
{
    NodeRangeGrid->SetColSize(COLOUR_COL, 50);
    NodeRangeGrid->SetColSize(CHANNEL_COL, std::max(NodeRangeGrid->GetSize().x - NodeRangeGrid->GetColSize(NAME_COL) - 50 - 30 - NodeRangeGrid->GetRowLabelSize(), 10));
    SingleNodeGrid->SetColSize(COLOUR_COL, 50);
    SingleNodeGrid->SetColSize(CHANNEL_COL, std::max(SingleNodeGrid->GetSize().x - SingleNodeGrid->GetColSize(NAME_COL) - 50 - 30 - SingleNodeGrid->GetRowLabelSize(), 10));
    NameChoice->Clear();
    model = cls;
    if (_modelPreview) _modelPreview->SetModel(cls);

    bool normalized = false;
    for (auto [name, info] : finfo) {
        NameChoice->Append(name);

        std::string type2 = info["Type"];
        if (type2 == "") {
            // old style, map
            if (name == "Coro" || name == "SingleNode") {
                info["Type"] = "SingleNode";
            } else if (name == "NodeRange") {
                info["Type"] = "NodeRange";
            }
            normalized = true;
        }

        stateData[name] = info;
    }
    if (normalized && _changeCallback) _changeCallback();

    if (NameChoice->GetCount() > 0) {
        DeleteButton->Enable();
        StateTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    } else {
        DeleteButton->Disable();
        StateTypeChoice->Disable();
    }

    size_t nodeCount = cls->GetNodeCount();
    _nodeNameToIndex.clear();
    _nodeNameToIndex.reserve(nodeCount);

    static constexpr size_t MAX_NODES_FOR_DROPDOWN = 5000;
    const bool buildDropdown = (nodeCount <= MAX_NODES_FOR_DROPDOWN);

    wxArrayString names;
    if (buildDropdown) {
        names.reserve(nodeCount + 1);
        names.push_back("");
        for (size_t x = 0; x < nodeCount; x++) {
            std::string nn = cls->GetNodeName(x, true);
            _nodeNameToIndex[nn].push_back(x);
            names.push_back(wxString(nn));
        }
    } else {
        for (size_t x = 0; x < nodeCount; x++) {
            std::string nn = cls->GetNodeName(x, true);
            _nodeNameToIndex[nn].push_back(x);
        }
    }

    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        wxGridCellTextEditor* neditor = new wxGridCellTextEditor();
        wxString nfilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_/\\|#");
        wxTextValidator nvalidator(wxFILTER_INCLUDE_CHAR_LIST);
        nvalidator.SetCharIncludes(nfilter);
        neditor->SetValidator(nvalidator);

        SingleNodeGrid->SetCellEditor(x, NAME_COL, neditor);
        SingleNodeGrid->SetReadOnly(x, COLOUR_COL);

        if (buildDropdown) {
            NodesGridCellEditor* editor = new NodesGridCellEditor();
            editor->names = names;
            SingleNodeGrid->SetCellEditor(x, CHANNEL_COL, editor);
        }
    }

    for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
        wxGridCellTextEditor* reditor = new wxGridCellTextEditor();
        wxString filter("0123456789,-");
        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
        validator.SetCharIncludes(filter);
        reditor->SetValidator(validator);

        wxGridCellTextEditor* neditor2 = new wxGridCellTextEditor();
        wxString nfilter2("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_/\\|#");
        wxTextValidator nvalidator2(wxFILTER_INCLUDE_CHAR_LIST);
        nvalidator2.SetCharIncludes(nfilter2);
        neditor2->SetValidator(nvalidator2);

        NodeRangeGrid->SetCellEditor(x, NAME_COL, neditor2);
        NodeRangeGrid->SetCellEditor(x, CHANNEL_COL, reditor);
        NodeRangeGrid->SetReadOnly(x, COLOUR_COL);
    }

    ValidateWindow();

    auto grid = NodeRangeGrid;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    }
    SelectRow(grid, -1);
}

std::map<std::string, std::map<std::string, std::string>> ModelStatesPanel::GetStateInfo() const
{
    if (NodeRangeGrid->IsCellEditControlShown()) {
        NodeRangeGrid->SaveEditControlValue();
        NodeRangeGrid->HideCellEditControl();
    }
    if (SingleNodeGrid->IsCellEditControlShown()) {
        SingleNodeGrid->SaveEditControlValue();
        SingleNodeGrid->HideCellEditControl();
    }
    std::map<std::string, std::map<std::string, std::string>> finfo;
    for (const auto& it : stateData) {
        if (!it.second.empty()) {
            finfo[it.first] = it.second;
        }
    }
    return finfo;
}

static bool SetGrid(wxGrid* grid, std::map<std::string, std::string>& info)
{
    bool customColor = false;
    if (info["CustomColors"] == "1") {
        grid->ShowCol(COLOUR_COL);
        customColor = true;
    } else {
        grid->HideCol(COLOUR_COL);
    }
    for (int x = 0; x < grid->GetNumberRows(); x++) {
        wxString pname = wxString::Format("s%03d", wxAtoi(grid->GetRowLabelValue(x)));
        pname.Replace(" ", "");
        if (info.find(pname) != end(info) || info.find(pname + "-Name") != end(info) || info.find(pname + "-Color") != end(info)) {
            grid->SetCellValue(x, CHANNEL_COL, info[pname.ToStdString()]);

            wxString n = info[pname.ToStdString() + "-Name"];
            grid->SetCellValue(x, NAME_COL, n);

            wxString c = info[pname.ToStdString() + "-Color"];
            if (c == "") {
                c = "#FFFFFF";
            }
            xlColor color(c);
            grid->SetCellBackgroundColour(x, COLOUR_COL, xlColorToWxColour(color));
        } else {
            grid->SetCellValue(x, CHANNEL_COL, "");
            grid->SetCellValue(x, NAME_COL, "");
            grid->SetCellBackgroundColour(x, COLOUR_COL, *wxWHITE);
        }
    }
    return customColor;
}

void ModelStatesPanel::SelectStateModel(const std::string& name)
{
    StopStateAnimation();
    StateTypeChoice->Enable();
    wxString type = stateData[name]["Type"];
    auto grid = NodeRangeGrid;
    if (type == "") {
        type = "SingleNode";
        stateData[name]["Type"] = type;
        grid = SingleNodeGrid;
    }
    if (type == "SingleNode") {
        StateTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
        std::map<std::string, std::string>& info = stateData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
        grid = SingleNodeGrid;
    } else if (type == "NodeRange") {
        StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
        std::map<std::string, std::string>& info = stateData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
    }
    SelectRow(grid, -1);
    PopulateStateFilter();
}

void ModelStatesPanel::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    ValidateWindow();
}

void ModelStatesPanel::OnButtonMatrixAddClicked(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New State Model", "Enter name for new state model definition");
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);
            NameChoice->SetStringSelection(n);
            NameChoice->Enable();
            DeleteButton->Enable();

            // set the default type of state based on model type
            if (model->GetDisplayAs() == DisplayAsType::Custom) {
                CustomModel* cm = dynamic_cast<CustomModel*>(model);
                if (cm != nullptr) {
                    if (cm->IsAllNodesUnique()) {
                        StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
                    } else {
                        StateTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
                    }
                }
            } else if (model->GetDisplayAs() == DisplayAsType::ChannelBlock) {
                StateTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
            } else {
                StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
            }
            UpdateStateType();
            NotifyChange();
        }
    }
    ValidateWindow();
}

void ModelStatesPanel::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete state model definition?", "Are you sure you want to delete " + name + "?",
                         wxICON_WARNING | wxOK, this);
    if (i == wxID_OK || i == wxOK) {
        stateData[name].clear();
        NameChoice->Delete(NameChoice->GetSelection());
        if (NameChoice->GetCount() > 0) {
            NameChoice->SetSelection(0);
            SelectStateModel(NameChoice->GetString(0).ToStdString());
        } else {
            NameChoice->SetSelection(wxNOT_FOUND);
            NameChoice->Disable();
            StateTypeChoice->Disable();
            DeleteButton->Disable();
        }
        NotifyChange();
    }
    ValidateWindow();
}

void ModelStatesPanel::OnCustomColorCheckboxClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        if (CustomColorSingleNode->IsChecked()) {
            SingleNodeGrid->ShowCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "1";
        } else {
            SingleNodeGrid->HideCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "0";
            for (auto& it : stateData[name]) {
                if (EndsWith(it.first, "-Color")) {
                    it.second = "";
                }
            }
            for (int r = 0; r < SingleNodeGrid->GetNumberRows(); r++) {
                SingleNodeGrid->SetCellBackgroundColour(r, COLOUR_COL, *wxWHITE);
            }
        }
    } else {
        if (CustomColorNodeRanges->IsChecked()) {
            NodeRangeGrid->ShowCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "1";
        } else {
            NodeRangeGrid->HideCol(COLOUR_COL);
            stateData[name]["CustomColors"] = "0";
            for (auto& it : stateData[name]) {
                if (EndsWith(it.first, "-Color")) {
                    it.second = "";
                }
            }
            for (int r = 0; r < NodeRangeGrid->GetNumberRows(); r++) {
                NodeRangeGrid->SetCellBackgroundColour(r, COLOUR_COL, *wxWHITE);
            }
        }
    }
}

void ModelStatesPanel::GetValue(wxGrid* grid, const int row, const int col, std::map<std::string, std::string>& info)
{
    wxString key = wxString::Format("s%03d", wxAtoi(grid->GetRowLabelValue(row)));
    key.Replace(" ", "");
    if (key != "") {
        if (grid->GetCellValue(row, NAME_COL) != "" || grid->GetCellBackgroundColour(row, COLOUR_COL) != *wxWHITE || grid->GetCellValue(row, CHANNEL_COL) != "") {
            if (col == COLOUR_COL) {
                key += "-Color";
                xlColor color = wxColourToXlColor(grid->GetCellBackgroundColour(row, col));
                info[key.ToStdString()] = std::string(color);
            } else if (col == NAME_COL) {
                key += "-Name";
                info[key.ToStdString()] = grid->GetCellValue(row, col).Lower();
            } else {
                info[key.ToStdString()] = grid->GetCellValue(row, col);
            }
        } else {
            // if all the values are their defaults then delete them all so we dont keep them unnecessarily
            info.erase((key + "-Color").ToStdString());
            info.erase((key + "-Name").ToStdString());
            info.erase(key.ToStdString());
        }
    }
    SelectRow(grid, row);
}

void ModelStatesPanel::ClearNodeColor(Model* m)
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

xlColor ModelStatesPanel::GetRowColor(wxGrid* grid, int const row, bool const prev, bool const force)
{
    if (force) {
        return wxColourToXlColor(grid->GetCellBackgroundColour(row, COLOUR_COL));
    }
    if (prev) {
        return xlColor(255, 100, 255);
    }
    return xlWHITE;
}

void ModelStatesPanel::SelectRow(wxGrid* grid, int const r)
{
    if (!model) return;

    _selected.clear();
    ClearNodeColor(model);

    int const draw_mode = ChoiceColorDraw ? ChoiceColorDraw->GetCurrentSelection() : 0;

    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        if (r == -1) {
            for (int i = 0; i < grid->GetNumberRows(); ++i) {
                xlColor const c = GetRowColor(grid, i, false, draw_mode == 0 && CustomColorSingleNode->IsChecked());
                SetSingleNodeColor(grid, i, c, true);
            }
        } else {
            if (draw_mode == 0) {
                for (int i = 0; i < grid->GetNumberRows(); ++i) {
                    xlColor const c = GetRowColor(grid, i, r != i, CustomColorSingleNode->IsChecked());
                    SetSingleNodeColor(grid, i, c, false);
                }
            }
            xlColor const cc = GetRowColor(grid, r, false, draw_mode == 0 && CustomColorSingleNode->IsChecked());
            SetSingleNodeColor(grid, r, cc, true);
        }
    } else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        if (r == -1) {
            for (int i = 0; i < grid->GetNumberRows(); ++i) {
                xlColor const c = GetRowColor(grid, i, false, draw_mode == 0 && CustomColorNodeRanges->IsChecked());
                SetNodeColor(grid, i, c, true);
            }
        } else {
            if (draw_mode == 0) {
                for (int i = 0; i < grid->GetNumberRows(); ++i) {
                    xlColor const c = GetRowColor(grid, i, r != i, CustomColorNodeRanges->IsChecked());
                    SetNodeColor(grid, i, c, false);
                }
            }
            xlColor const cc = GetRowColor(grid, r, false, draw_mode == 0 && CustomColorNodeRanges->IsChecked());
            SetNodeColor(grid, r, cc, true);
        }
    }
    grid->Refresh();
    if (_modelPreview) model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

std::vector<int> ModelStatesPanel::GetSelectedGridRows(wxGrid* grid, int fallbackRow)
{
    std::set<int> rowSet;
    wxArrayInt selRows = grid->GetSelectedRows();
    for (int r : selRows) rowSet.insert(r);
    wxGridCellCoordsArray topLeft = grid->GetSelectionBlockTopLeft();
    wxGridCellCoordsArray bottomRight = grid->GetSelectionBlockBottomRight();
    for (size_t i = 0; i < topLeft.size() && i < bottomRight.size(); ++i) {
        for (int r = topLeft[i].GetRow(); r <= bottomRight[i].GetRow(); ++r) {
            rowSet.insert(r);
        }
    }
    wxGridCellCoordsArray selCells = grid->GetSelectedCells();
    for (const auto& cell : selCells) {
        rowSet.insert(cell.GetRow());
    }
    if (rowSet.empty() && fallbackRow >= 0) {
        rowSet.insert(fallbackRow);
    }
    return std::vector<int>(rowSet.begin(), rowSet.end());
}

void ModelStatesPanel::SelectRows(wxGrid* grid, int fallbackRow)
{
    if (!model) return;
    if (grid == NodeRangeGrid) {
        StopStateAnimation();
        if (StateFilterChoice && StateFilterChoice->GetSelection() != 0) {
            StateFilterChoice->SetSelection(0);
        }
    }
    std::vector<int> rows = GetSelectedGridRows(grid, fallbackRow);
    if (rows.empty()) return;

    _selected.clear();
    ClearNodeColor(model);

    int const draw_mode = ChoiceColorDraw ? ChoiceColorDraw->GetCurrentSelection() : 0;
    bool const force_custom = (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE)
                               ? (draw_mode == 0 && CustomColorSingleNode->IsChecked())
                               : (draw_mode == 0 && CustomColorNodeRanges->IsChecked());

    for (int row : rows) {
        xlColor const cc = GetRowColor(grid, row, false, force_custom);
        if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE)
            SetSingleNodeColor(grid, row, cc, true);
        else
            SetNodeColor(grid, row, cc, true);
    }

    grid->Refresh();
    if (_modelPreview) model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

void ModelStatesPanel::SetSingleNodeColor(wxGrid* grid, const int row, xlColor const& c, bool highlight)
{
    wxString v = grid->GetCellValue(row, CHANNEL_COL);
    wxStringTokenizer wtkz(v, ",");
    while (wtkz.HasMoreTokens()) {
        std::string valstr = wtkz.GetNextToken().ToStdString();
        auto it = _nodeNameToIndex.find(valstr);
        if (it != _nodeNameToIndex.end()) {
            for (size_t n : it->second) {
                model->SetNodeColor(n, c);
                if (highlight) _selected.push_back(n);
            }
        }
    }
}

bool ModelStatesPanel::SetNodeColor(wxGrid* grid, int const row, xlColor const& c, bool highlight)
{
    wxString v = grid->GetCellValue(row, CHANNEL_COL);
    if (v.empty()) {
        return false;
    }

    xlColor cc(c);
    if (model->GetDimmingCurve()) {
        model->GetDimmingCurve()->apply(cc);
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
                model->SetNodeColor(n, cc);
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

void ModelStatesPanel::OnNodeRangeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), stateData[name]);
    if (event.GetCol() == NAME_COL) {
        PopulateStateFilter();
    }
    ValidateWindow();
    NotifyChange();
}

void ModelStatesPanel::OnSingleNodeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), stateData[name]);
    ValidateWindow();
    NotifyChange();
}

void ModelStatesPanel::OnStateTypeChoicePageChanged(wxChoicebookEvent& event)
{
    UpdateStateType();
}

void ModelStatesPanel::UpdateStateType()
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    stateData[name].clear();
    switch (StateTypeChoice->GetSelection()) {
        case SINGLE_NODE_STATE:
            stateData[name]["Type"] = "SingleNode";
            break;
        case NODE_RANGE_STATE:
            stateData[name]["Type"] = "NodeRange";
            break;
    }
    SelectStateModel(name);
    if (model) ClearNodeColor(model);
}

void ModelStatesPanel::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == CHANNEL_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const wxString title = name + " - " + NodeRangeGrid->GetCellValue(event.GetRow(), NAME_COL);
        bool wasOutputting = StopOutputToLights();
        {
            NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), _outputManager, this);

            if (dialog.ShowModal() == wxID_OK) {
                NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
                GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), stateData[name]);
                dialog.Close();
                ValidateWindow();
            }
        }
        if (wasOutputting)
            StartOutputToLights();
    } else if (event.GetCol() == COLOUR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL);
        auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, c);
        if (res == wxID_OK) {
            NodeRangeGrid->SetCellBackgroundColour(event.GetRow(), COLOUR_COL, color);
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), stateData[name]);
        }
    }
    SelectRow(NodeRangeGrid, event.GetRow());
}

void ModelStatesPanel::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == COLOUR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL);
        auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, c);
        if (res == wxID_OK) {
            SingleNodeGrid->SetCellBackgroundColour(event.GetRow(), COLOUR_COL, color);
            SingleNodeGrid->Refresh();
            GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), stateData[name]);
        }
    }
    SelectRow(SingleNodeGrid, event.GetRow());
}

void ModelStatesPanel::OnNodeRangeGridCellLeftClick(wxGridEvent& event)
{
    SelectRows(NodeRangeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnSingleNodeGridCellLeftClick(wxGridEvent& event)
{
    SelectRows(SingleNodeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnSingleNodeGridCellSelect(wxGridEvent& event)
{
    SelectRows(SingleNodeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnNodeRangeGridCellSelect(wxGridEvent& event)
{
    SelectRows(NodeRangeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnNodeRangeGridCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;

    mnu.Append(STATE_DIALOG_IMPORT_SUB, "Import SubModel");
    mnu.Append(STATE_DIALOG_COPY_STATES, "Import State Definition");
    mnu.Append(ID_MNU_SORT, "Sort by State");
    mnu.AppendSeparator();
    mnu.Append(ID_MNU_ADD_STATE_BEFORE, "Insert Line Before");
    mnu.Append(ID_MNU_ADD_STATE_AFTER, "Insert Line After");
    mnu.Append(ID_MNU_MOVE_STATE_UP, "Move Selected Up");
    mnu.Append(ID_MNU_MOVE_STATE_DOWN, "Move Selected Down");
    mnu.AppendSeparator();
    mnu.Append(ID_MNU_DELETE_STATE, "Delete Selected");
    mnu.Append(STATE_DIALOG_CLEAR_SELECTED_ROWS, "Clear Selected");
    mnu.Append(STATE_DIALOG_CLEAR_STATES, "Clear All Rows");

    if (CustomColorNodeRanges->IsChecked() &&
        NodeRangeGrid->GetSelectedRows().GetCount() >= 2) {
        mnu.AppendSeparator();
        mnu.Append(STATE_DIALOG_BULK_COLOR_CHANGE, "Bulk Color Change...");
    }

    mnu.Bind(wxEVT_COMMAND_MENU_SELECTED, [gridevent = event, this](wxCommandEvent& rightClkEvent) mutable {
        OnGridPopup(rightClkEvent.GetId(), gridevent);
    });
    PopupMenu(&mnu);

    event.Skip();
}

void ModelStatesPanel::OnNodeRangeGridLabelLeftDClick(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    const wxString title = name + " - " + NodeRangeGrid->GetCellValue(event.GetRow(), NAME_COL);
    bool wasOutputting = StopOutputToLights();
    {
        NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), _outputManager, this);

        if (dialog.ShowModal() == wxID_OK) {
            NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
            GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, stateData[name]);
            dialog.Close();
            ValidateWindow();
        }
    }
    if (wasOutputting)
        StartOutputToLights();
}

void ModelStatesPanel::OnSingleNodeGridLabelLeftClick(wxGridEvent& event)
{
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnNodeRangeGridLabelLeftClick(wxGridEvent& event)
{
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnNodeRangeGridRangeSelect(wxGridRangeSelectEvent& event)
{
    SelectRows(NodeRangeGrid, NodeRangeGrid->GetGridCursorRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnSingleNodeGridRangeSelect(wxGridRangeSelectEvent& event)
{
    SelectRows(SingleNodeGrid, SingleNodeGrid->GetGridCursorRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelStatesPanel::OnButton_ImportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    if (DeleteButton->IsEnabled()) {
        mnu.Append(STATE_DIALOG_COPY, "Copy");
        mnu.Append(STATE_DIALOG_RENAME, "Rename");
        mnu.AppendSeparator();
    }
    mnu.Append(STATE_DIALOG_IMPORT_MODEL, "Import From Model");
    mnu.Append(STATE_DIALOG_IMPORT_FILE, "Import From File");
    mnu.Append(STATE_DIALOG_IMPORT_ALL_SUB, "Import From SubModels");
    mnu.Append(STATE_DIALOG_IMPORT_DOWNLOAD, "Import From Downloads");
    mnu.Append(STATE_DIALOG_EXPORT_TOOTHERS, "Export State(s) To Other Model(s)");
    mnu.AppendSeparator();
    mnu.Append(STATE_DIALOG_SHIFT, "Shift Nodes");
    mnu.Append(STATE_DIALOG_REVERSE, "Reverse Nodes");

    mnu.Bind(wxEVT_MENU, &ModelStatesPanel::OnAddBtnPopup, this);
    PopupMenu(&mnu);
}

void ModelStatesPanel::AddLabel(wxString label)
{
    int free = -1;
    wxGrid* grid = nullptr;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    } else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        grid = NodeRangeGrid;
    } else {
        return;
    }

    for (int i = 0; i < grid->GetNumberRows(); i++) {
        if (grid->GetCellValue(i, NAME_COL) == label) {
            free = -1;
            break;
        }
        if (grid->GetCellValue(i, NAME_COL) == "" && free == -1) {
            free = i;
        }
    }

    if (free != -1) {
        grid->SetCellValue(free, NAME_COL, label);
        wxString key = wxString::Format("s%03d", wxAtoi(grid->GetRowLabelValue(free)));
        key.Replace(" ", "");
        if (key != "") {
            std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
            key += "-Name";
            stateData[name][key.ToStdString()] = label.Lower();
        }
        SelectRow(grid, free);
    }
}

void ModelStatesPanel::OnButton_7SegmentClick(wxCommandEvent& event)
{
    SevenSegmentDialog dialog(this);

    if (dialog.ShowModal() == wxID_OK) {
        if (dialog.CheckBox_Ones->IsChecked()) {
            for (int i = 0; i < 10; i++) {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Tens->IsChecked()) {
            AddLabel("00");
            for (int i = 10; i < 100; i += 10) {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Hundreds->IsChecked()) {
            AddLabel("000");
            for (int i = 100; i < 1000; i += 100) {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Colon->IsChecked()) {
            AddLabel("colon");
        }
        if (dialog.CheckBox_Decimal->IsChecked()) {
            AddLabel("dot");
        }
        if (dialog.CheckBox_Thousands->IsChecked()) {
            AddLabel("0000");
            for (int i = 1000; i < 10000; i += 1000) {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        NotifyChange();
    }
}

void ModelStatesPanel::ValidateWindow()
{
    if (NameChoice->GetStringSelection() == "") {
        NodeRangeGrid->Disable();
        SingleNodeGrid->Disable();
        Button_7Seg->Disable();
        Button_7Segment->Disable();
        CustomColorSingleNode->Disable();
        CustomColorNodeRanges->Disable();
    } else {
        NodeRangeGrid->Enable();
        SingleNodeGrid->Enable();
        Button_7Seg->Enable();
        Button_7Segment->Enable();
        CustomColorSingleNode->Enable();
        CustomColorNodeRanges->Enable();
    }

    if (CustomColorSingleNode->IsChecked()) {
        SingleNodeGrid->ShowCol(COLOUR_COL);
    } else {
        SingleNodeGrid->HideCol(COLOUR_COL);
    }

    if (CustomColorNodeRanges->IsChecked()) {
        NodeRangeGrid->ShowCol(COLOUR_COL);
    } else {
        NodeRangeGrid->HideCol(COLOUR_COL);
    }

    wxGrid* grid = nullptr;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    } else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        grid = NodeRangeGrid;
    }

    if (grid != nullptr) {
        for (int i = 0; i < grid->GetNumberRows(); i++) {
            if (grid->GetCellValue(i, CHANNEL_COL) != "" && grid->GetCellValue(i, NAME_COL) == "") {
                grid->SetCellBackgroundColour(i, NAME_COL, *wxRED);
                grid->SetCellBackgroundColour(i, CHANNEL_COL, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            } else if (grid->GetCellValue(i, NAME_COL) != "" && grid->GetCellValue(i, CHANNEL_COL) == "") {
                grid->SetCellBackgroundColour(i, NAME_COL, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                grid->SetCellBackgroundColour(i, CHANNEL_COL, *wxRED);
            } else {
                grid->SetCellBackgroundColour(i, NAME_COL, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
                grid->SetCellBackgroundColour(i, CHANNEL_COL, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            }
        }
    }
}

bool ModelStatesPanel::HasInvalidRows() const
{
    wxGrid* grid = nullptr;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    } else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        grid = NodeRangeGrid;
    }
    if (grid == nullptr) return false;
    for (int i = 0; i < grid->GetNumberRows(); i++) {
        bool hasName = grid->GetCellValue(i, NAME_COL) != "";
        bool hasChannel = grid->GetCellValue(i, CHANNEL_COL) != "";
        if (hasName != hasChannel) return true;
    }
    return false;
}

void ModelStatesPanel::OnGridPopup(const int rightEventID, wxGridEvent& gridEvent)
{
    if (rightEventID == STATE_DIALOG_IMPORT_SUB) {
        ImportSubmodel(gridEvent);
    } else if (rightEventID == STATE_DIALOG_COPY_STATES) {
        CopyStates(gridEvent);
    } else if (rightEventID == STATE_DIALOG_CLEAR_STATES) {
        ClearStates(gridEvent);
    } else if (rightEventID == STATE_DIALOG_CLEAR_SELECTED_ROWS) {
        ClearSelectedStates(gridEvent);
    } else if (rightEventID == ID_MNU_ADD_STATE_BEFORE) {
        AddBefore(gridEvent);
    } else if (rightEventID == ID_MNU_ADD_STATE_AFTER) {
        AddAfter(gridEvent);
    } else if (rightEventID == ID_MNU_DELETE_STATE) {
        DeleteSelected(gridEvent);
    } else if (rightEventID == ID_MNU_MOVE_STATE_UP) {
        MoveSelectedUp(gridEvent);
    } else if (rightEventID == ID_MNU_MOVE_STATE_DOWN) {
        MoveSelectedDown(gridEvent);
    } else if (rightEventID == ID_MNU_SORT) {
        SortState(gridEvent);
    } else if (rightEventID == STATE_DIALOG_BULK_COLOR_CHANGE) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const auto rows = GetSelectedGridRows(NodeRangeGrid, gridEvent.GetRow());
        wxColor seed = NodeRangeGrid->GetCellBackgroundColour(rows[0], COLOUR_COL);
        auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, seed);
        if (res == wxID_OK) {
            for (int row : rows) {
                NodeRangeGrid->SetCellBackgroundColour(row, COLOUR_COL, color);
                GetValue(NodeRangeGrid, row, COLOUR_COL, stateData[name]);
            }
            NodeRangeGrid->Refresh();
        }
    }
}

void ModelStatesPanel::ImportSubmodel(wxGridEvent& event)
{
    wxArrayString choices;
    if (_getSubModelNames) {
        for (const auto& n : _getSubModelNames()) choices.Add(n);
    } else {
        for (Model* sm : model->GetSubModels()) choices.Add(sm->Name());
    }

    if (choices.GetCount() == 0) {
        wxMessageBox("No SubModels Found.");
        return;
    }

    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    wxMultiChoiceDialog dlg(GetParent(), "", "Select SubModel", choices);

    if (dlg.ShowModal() == wxID_OK) {
        wxArrayString allNodes;
        for (auto const& idx : dlg.GetSelections()) {
            wxString smName = choices.at(idx);
            wxString nodes;
            if (_getSubModelRanges) {
                nodes = _getSubModelRanges(smName.ToStdString());
            } else {
                Model* sm = model->GetSubModel(smName.ToStdString());
                if (sm == nullptr) {
                    spdlog::error(
                        "Strange ... ModelStatesPanel::ImportSubmodel returned no model "
                        "for %s but it was in the list we gave the user.",
                        (const char*)smName.c_str());
                    continue;
                }
                nodes = getSubmodelNodes(sm);
            }
            if (!nodes.IsEmpty()) allNodes.Add(nodes);
        }
        const auto newNodes = wxJoin(allNodes, ',', '\0');

        auto newNodeArrray = wxSplit(NodeUtils::ExpandNodes(newNodes), ',');

        std::sort(newNodeArrray.begin(), newNodeArrray.end(),
            [](const wxString& a, const wxString& b) {
                return wxAtoi(a) < wxAtoi(b);
            });

        newNodeArrray.erase(std::unique(newNodeArrray.begin(), newNodeArrray.end()), newNodeArrray.end());

        NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(newNodeArrray, ',')));
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, stateData[name]);
        dlg.Close();
        ValidateWindow();
    }
}

wxString ModelStatesPanel::getSubmodelNodes(Model* sm)
{
    SubModel* subModel = dynamic_cast<SubModel*>(sm);
    if (subModel == nullptr) {
        return "";
    }

    if (!subModel->IsRanges()) {
        return "";
    }

    wxArrayString rows;
    int numRanges = subModel->GetNumRanges();
    for (int i = 0; i < numRanges; i++) {
        std::string range = subModel->GetRange(i);
        if (!range.empty()) {
            rows.Add(range);
        }
    }

    return wxJoin(rows, ',', '\0');
}

void ModelStatesPanel::OnAddBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == STATE_DIALOG_IMPORT_MODEL) {
        ImportStatesFromModel();
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_IMPORT_FILE) {
        const wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) {
            return;
        }
        ImportStates(filename);
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_COPY) {
        CopyStateData();
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_RENAME) {
        RenameState();
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_SHIFT) {
        ShiftStateNodes();
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_REVERSE) {
        ReverseStateNodes();
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_IMPORT_ALL_SUB) {
        ImportStatesFromSubModels();
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_IMPORT_DOWNLOAD) {
        const wxString filename = GetDownloadStates();
        if (filename.IsEmpty()) {
            return;
        }
        ImportStates(filename);
        NotifyChange();
    } else if (event.GetId() == STATE_DIALOG_EXPORT_TOOTHERS) {
        ExportStatesToOtherModels();
        NotifyChange();
    }
}

void ModelStatesPanel::ImportStatesFromModel()
{
    xLightsFrame* xlights = xLightsApp::GetFrame();

    const wxArrayString choices = getModelList(&xlights->AllModels);

    wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);

    if (dlg.ShowModal() == wxID_OK) {
        Model* m = xlights->GetModel(dlg.GetStringSelection());
        if (m == nullptr) return;
        if (m->GetStateInfo().size() == 0) {
            wxMessageBox(dlg.GetStringSelection() + " contains no states, skipping");
            return;
        }

        AddStates(m->GetStateInfo());
        overRide = false;
        showDialog = true;

        NameChoice->Enable();
        StateTypeChoice->Enable();
        DeleteButton->Enable();

        NameChoice->SetSelection(NameChoice->GetCount() - 1);
        NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
        SelectStateModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
        ValidateWindow();
    }
}

void ModelStatesPanel::ImportStates(const wxString& filename)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.ToStdString().c_str());

    if (result) {
        pugi::xml_node root = doc.document_element();
        bool stateFound = false;

        auto processStateInfoChildren = [&](pugi::xml_node parent) {
            for (pugi::xml_node n : parent.children("stateInfo")) {
                std::map<std::string, std::map<std::string, std::string>> stateInfo;
                XmlSerialize::DeserializeStateInfo(n, stateInfo);
                if (stateInfo.size() == 0)
                    continue;
                stateFound = true;
                AddStates(stateInfo);
            }
        };

        std::string_view rootName = root.name();
        if (rootName == "models") {
            for (pugi::xml_node model : root.children("model"))
                processStateInfoChildren(model);
        } else {
            processStateInfoChildren(root);
        }
        overRide = false;
        showDialog = true;

        if (stateFound) {
            NameChoice->Enable();
            StateTypeChoice->Enable();
            DeleteButton->Enable();

            NameChoice->SetSelection(NameChoice->GetCount() - 1);
            NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
            SelectStateModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
            ValidateWindow();
        } else {
            DisplayError(filename + " contains no states.");
        }
    } else {
        DisplayError(filename + " Failure loading xModel file.");
    }
}

void ModelStatesPanel::ImportStatesFromSubModels()
{
    std::vector<std::string> subNames;
    if (_getSubModelNames) {
        subNames = _getSubModelNames();
    } else {
        for (Model* sm : model->GetSubModels()) subNames.push_back(sm->Name());
    }

    if (subNames.empty()) {
        wxMessageBox("No SubModels Found.");
        return;
    }
    wxTextEntryDialog dlg(this, "New State Model", "Enter name for new state model definition");
    if (dlg.ShowModal() == wxID_OK) {
        std::string name = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(name) == wxNOT_FOUND) {
            NameChoice->Append(name);
            NameChoice->SetStringSelection(name);
            NameChoice->Enable();
            DeleteButton->Enable();
            StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
            UpdateStateType();

            int idx { 0 };
            for (const auto& smName : subNames) {
                auto subname = cleanSubName(smName);
                wxString nodes;
                if (_getSubModelRanges) {
                    nodes = _getSubModelRanges(smName);
                } else {
                    Model* sm = model->GetSubModel(smName);
                    nodes = sm ? getSubmodelNodes(sm) : wxString{};
                }
                auto newNodeArrray = wxSplit(NodeUtils::ExpandNodes(nodes), ',');

                std::sort(newNodeArrray.begin(), newNodeArrray.end(),
                          [](const wxString& a, const wxString& b) {
                              return wxAtoi(a) < wxAtoi(b);
                          });

                newNodeArrray.erase(std::unique(newNodeArrray.begin(), newNodeArrray.end()), newNodeArrray.end());
                NodeRangeGrid->SetCellValue(idx, NAME_COL, subname);
                NodeRangeGrid->SetCellValue(idx, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(newNodeArrray, ',')));
                GetValue(NodeRangeGrid, idx, NAME_COL, stateData[name]);
                GetValue(NodeRangeGrid, idx, CHANNEL_COL, stateData[name]);
                ++idx;
                if (idx >= 200) {
                    break;
                }
            }
            NodeRangeGrid->Refresh();
        }
    }
}

std::string ModelStatesPanel::cleanSubName(std::string name)
{
    name = ::Lower(name);
    Replace(name, " ", "_");
    return name;
}

void ModelStatesPanel::AddStates(std::map<std::string, std::map<std::string, std::string>> const& states)
{
    for (const auto& state : states) {
        auto fname = state.first;

        if (NameChoice->FindString(fname) != wxNOT_FOUND) {
            if (showDialog) {
                wxMessageDialog confirm(this, _("State(s) with the Same Name Already Exist.\n Would you Like to Override Them ALL?"), _("Override States"), wxYES_NO);
                int returnCode = confirm.ShowModal();
                if (returnCode == wxID_YES)
                    overRide = true;
                showDialog = false;
            }
            if (!overRide) {
                const auto basefname = fname;
                int suffix = 1;
                while (NameChoice->FindString(fname) != wxNOT_FOUND) {
                    fname = wxString::Format("%s-%d", basefname, suffix);
                    suffix++;
                }
                NameChoice->Append(fname);
            }
        } else {
            NameChoice->Append(fname);
        }

        stateData[fname] = state.second;
    }
}

wxArrayString ModelStatesPanel::getModelList(ModelManager* modelManager)
{
    wxArrayString choices;
    for (auto it = modelManager->begin(); it != modelManager->end(); ++it) {
        Model* m = it->second;
        if (m->Name() == model->Name())
            continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup)
            continue;
        choices.Add(m->Name());
    }
    return choices;
}

void ModelStatesPanel::CopyStates(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    wxArrayString choices;

    for (auto [k, v] : stateData) {
        if (v["Type"] != stateData[name]["Type"]) {
            continue;
        }
        choices.push_back(ToWXString(k));
    }

    if (choices.empty()) {
        DisplayError("No State Definitions Found.");
        return;
    }

    wxMultiChoiceDialog dlg(GetParent(), "", "Select States", choices);
    if (dlg.ShowModal() == wxID_OK) {
        int stateIdx { 1 };
        for (auto const& idx : dlg.GetSelections()) {
            auto sd = stateData[choices.at(idx)];
            if (sd["CustomColors"] == "1") {
                stateData[name]["CustomColors"] = "1";
            }

            for (int x = 200; x >= 0; --x) {
                std::string pname = wxString::Format("s%03d", x);
                if (stateData[name].contains(pname)) {
                    stateIdx = x + 1;
                    break;
                }
            }
            for (int x = 1; x <= 200; x++) {
                std::string pname = wxString::Format("s%03d", x);
                if (sd.find(pname) != end(sd) || sd.find(pname + "-Name") != end(sd) || sd.find(pname + "-Color") != end(sd)) {
                    auto val = sd[pname];
                    if (val.empty()) {
                        continue;
                    }

                    auto n = sd[pname + "-Name"];
                    if (n.empty()) {
                        continue;
                    }
                    auto c = sd[pname + "-Color"];

                    if ("1" != stateData[name]["CustomColors"]) {
                        c = "";
                    }

                    std::string newname = wxString::Format("s%03d", stateIdx);
                    for (int x2 = stateIdx; x2 <= 200; x2++) {
                        if (stateData[name].contains(newname) ||
                            stateData[name].contains(newname + "-Name") ||
                            stateData[name].contains(newname + "-Color")) {
                            ++stateIdx;
                            newname = wxString::Format("s%03d", stateIdx);
                        } else {
                            break;
                        }
                    }
                    stateData[name].insert({ newname, val });
                    stateData[name].insert({ newname + "-Name", n });
                    stateData[name].insert({ newname + "-Color", c });
                    ++stateIdx;
                } else {
                    ++stateIdx;
                }
            }
        }
        SelectStateModel(name);
        ValidateWindow();
    }
}

void ModelStatesPanel::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (!_isActive) return;
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;

        SelectAllInBoundingRect(event.ShiftDown());
        m_creating_bound_rect = false;

        if (_modelPreview) _modelPreview->ReleaseMouse();
        ValidateWindow();
    }
}

void ModelStatesPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    if (!_isActive) return;
    RenderModel();
}

void ModelStatesPanel::OnPreviewLeftDown(wxMouseEvent& event)
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

    if (_modelPreview) _modelPreview->CaptureMouse();
}

void ModelStatesPanel::OnPreviewLeftDClick(wxMouseEvent& event)
{
    if (!_isActive) return;
    if (!_modelPreview) return;
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    int x = ray_origin.x;
    int y = ray_origin.y;
    wxString stNode = model->GetNodeNear(_modelPreview, xlPoint(x, y), false);
    if (stNode.IsEmpty())
        return;
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (stateData[name]["Type"] == "SingleNode") {
        int row = SingleNodeGrid->GetGridCursorRow();
        if (row < 0)
            return;

        std::string node = model->GetNodeName(wxAtoi(stNode) - 1, true);
        SingleNodeGrid->SetCellValue(row, CHANNEL_COL, node);
        SingleNodeGrid->Refresh();
        GetValue(SingleNodeGrid, row, CHANNEL_COL, stateData[name]);
    } else if (stateData[name]["Type"] == "NodeRange") {
        int row = NodeRangeGrid->GetGridCursorRow();
        if (row < 0)
            return;
        wxString oldnodes = NodeUtils::ExpandNodes(NodeRangeGrid->GetCellValue(row, CHANNEL_COL));
        auto oldNodeArrray = wxSplit(oldnodes, ',');

        bool found = false;
        for (auto it = oldNodeArrray.begin(); it != oldNodeArrray.end(); ++it) {
            if (*it == stNode) {
                oldNodeArrray.erase(it);
                found = true;
                break;
            }
        }
        if (!found) {
            oldNodeArrray.push_back(stNode);
        }
        std::sort(oldNodeArrray.begin(), oldNodeArrray.end(),
            [](const wxString& a, const wxString& b) {
                return wxAtoi(a) < wxAtoi(b);
            });

        NodeRangeGrid->SetCellValue(row, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',')));
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, row, CHANNEL_COL, stateData[name]);
    }
    ValidateWindow();
}

void ModelStatesPanel::OnPreviewMouseMove(wxMouseEvent& event)
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
}

void ModelStatesPanel::RenderModel()
{
    if (_modelPreview == nullptr || !_modelPreview->StartDrawing(mPointSize)) return;

    if (m_creating_bound_rect) {
        _modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
    }
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
    _modelPreview->EndDrawing();
}

void ModelStatesPanel::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    if (!_modelPreview) return;
    // Trim the mouse location to the preview area
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

void ModelStatesPanel::SelectAllInBoundingRect(bool shiftDwn)
{
    if (shiftDwn) {
        RemoveNodes();
        return;
    }
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (stateData[name]["Type"] != "NodeRange") {
        return;
    }

    int row = NodeRangeGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<int> nodes = model->GetNodesInBoundingBox(_modelPreview, xlPoint(m_bound_start_x, m_bound_start_y), xlPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;

    wxString oldnodes = NodeUtils::ExpandNodes(NodeRangeGrid->GetCellValue(row, CHANNEL_COL));

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

    std::sort(oldNodeArrray.begin(), oldNodeArrray.end(),
        [](const wxString& a, const wxString& b) {
            return wxAtoi(a) < wxAtoi(b);
        });

    NodeRangeGrid->SetCellValue(row, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',')));
    NodeRangeGrid->Refresh();
    GetValue(NodeRangeGrid, row, CHANNEL_COL, stateData[name]);
}

void ModelStatesPanel::RemoveNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (stateData[name]["Type"] != "NodeRange") {
        return;
    }
    int row = NodeRangeGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<int> nodes = model->GetNodesInBoundingBox(_modelPreview, xlPoint(m_bound_start_x, m_bound_start_y), xlPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;
    wxString oldnodes = NodeUtils::ExpandNodes(NodeRangeGrid->GetCellValue(row, CHANNEL_COL));
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

    std::sort(oldNodeArrray.begin(), oldNodeArrray.end(),
        [](const wxString& a, const wxString& b) {
            return wxAtoi(a) < wxAtoi(b);
        });

    NodeRangeGrid->SetCellValue(row, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',')));
    NodeRangeGrid->Refresh();
    GetValue(NodeRangeGrid, row, CHANNEL_COL, stateData[name]);
}

void ModelStatesPanel::CopyStateData()
{
    auto const index = NameChoice->GetSelection();
    if (index == -1)
        return;
    auto const& currentName = NameChoice->GetString(index);
    wxTextEntryDialog dlg(this, "Copy State", "Enter name for copied state definition", currentName);
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);

            stateData[n] = stateData[currentName];

            NameChoice->SetSelection(NameChoice->GetCount() - 1);
            NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
            SelectStateModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
        } else {
            DisplayError("'" + n + "' state definition already exists.");
        }
    }
}

void ModelStatesPanel::RenameState()
{
    auto const index = NameChoice->GetSelection();
    if (index == -1)
        return;
    auto const& currentName = NameChoice->GetString(index);
    wxTextEntryDialog dlg(this, "Rename State", "Enter new name for state definition", currentName);
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Delete(index);
            NameChoice->Insert(n, index);

            auto state = std::move(stateData[currentName]);
            stateData[n] = std::move(state);
            stateData.erase(currentName);

            NameChoice->SetSelection(index);
            NameChoice->SetStringSelection(NameChoice->GetString(index));
            SelectStateModel(NameChoice->GetString(index).ToStdString());
        } else {
            DisplayError("'" + n + "' state definition already exists.");
        }
    }
}

void ModelStatesPanel::ShiftStateNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (stateData[name]["Type"] != "NodeRange") {
        return;
    }
    long min = 0;
    long max = model->GetNodeCount();

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK) {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0) {
            NodeUtils::ShiftNodes(stateData[name], scaleFactor, min, max);
            SelectStateModel(name);
            ClearNodeColor(model);
        }
    }
}

void ModelStatesPanel::ReverseStateNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (stateData[name]["Type"] != "NodeRange") {
        return;
    }

    long max = model->GetNodeCount() + 1;

    NodeUtils::ReverseNodes(stateData[name], max);
    SelectStateModel(name);
    ClearNodeColor(model);
}

void ModelStatesPanel::OnTimer1Trigger(wxTimerEvent& event)
{
    if (!model) return;
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

void ModelStatesPanel::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        if (_outputManager->StartOutput()) SetConfigBool("OutputActive", true);
        timer1.Start(50, false);
    }
}

bool ModelStatesPanel::StopOutputToLights()
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

void ModelStatesPanel::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    } else {
        StopOutputToLights();
    }
}

void ModelStatesPanel::OnChoiceColorDrawSelect(wxCommandEvent& event)
{
    if (!ChoiceColorDraw) return;
    wxGrid* grid = nullptr;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    } else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        grid = NodeRangeGrid;
    } else {
        return;
    }
    int const row = grid->GetGridCursorRow();
    if (row < 0) {
        return;
    }
    SelectRow(grid, row);
}

void ModelStatesPanel::PopulateStateFilter()
{
    if (!StateFilterChoice) return;
    wxString selected = StateFilterChoice->GetStringSelection();
    StateFilterChoice->Clear();
    StateFilterChoice->Append(""); // empty = no filter
    StateFilterChoice->Append("<ALL>");
    if (NameChoice->GetCount() > 0 && NameChoice->GetSelection() != wxNOT_FOUND) {
        std::string defName = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        std::set<std::string> seen;
        std::vector<std::string> names;
        for (const auto& [k, v] : stateData[defName]) {
            if (EndsWith(k, "-Name") && !v.empty() && seen.find(v) == seen.end()) {
                seen.insert(v);
                names.push_back(v);
            }
        }
        std::sort(names.begin(), names.end());
        for (const auto& n : names) {
            StateFilterChoice->Append(wxString(n));
        }
    }
    if (!selected.empty() && StateFilterChoice->SetStringSelection(selected)) {
        // kept prior selection
    } else {
        StateFilterChoice->SetSelection(0);
    }
}

void ModelStatesPanel::UpdateStateFilter()
{
    if (!StateFilterChoice || !model) return;
    wxString sel = StateFilterChoice->GetStringSelection();
    if (sel.empty()) {
        SelectRows(NodeRangeGrid, NodeRangeGrid->GetGridCursorRow());
        return;
    }
    _selected.clear();
    ClearNodeColor(model);
    int const draw_mode = ChoiceColorDraw ? ChoiceColorDraw->GetCurrentSelection() : 0;
    bool const force_custom = draw_mode == 0 && CustomColorNodeRanges->IsChecked();
    if (sel == "<ALL>") {
        for (int i = 0; i < NodeRangeGrid->GetNumberRows(); ++i) {
            xlColor const c = GetRowColor(NodeRangeGrid, i, false, force_custom);
            SetNodeColor(NodeRangeGrid, i, c, true);
        }
    } else {
        for (int i = 0; i < NodeRangeGrid->GetNumberRows(); ++i) {
            if (NodeRangeGrid->GetCellValue(i, NAME_COL) == sel) {
                xlColor const c = GetRowColor(NodeRangeGrid, i, false, force_custom);
                SetNodeColor(NodeRangeGrid, i, c, true);
            }
        }
    }
    NodeRangeGrid->Refresh();
    if (_modelPreview) model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

void ModelStatesPanel::OnStateFilterSelect(wxCommandEvent& event)
{
    UpdateStateFilter();
}

void ModelStatesPanel::BuildStateAnimSteps()
{
    _stateAnimSteps.clear();

    // Use explicitly highlighted rows as the source when 2+ rows are selected;
    // otherwise fall back to every non-empty row in the grid.
    std::vector<int> source = GetSelectedGridRows(NodeRangeGrid, -1);
    if (source.size() < 2) {
        source.clear();
        for (int i = 0; i < NodeRangeGrid->GetNumberRows(); ++i) {
            if (!NodeRangeGrid->GetCellValue(i, NAME_COL).empty())
                source.push_back(i);
        }
    }

    // Group source rows by distinct state name and build one step per name.
    std::vector<std::string> nameOrder;
    std::map<std::string, std::vector<int>> nameToRows;
    for (int i : source) {
        wxString n = NodeRangeGrid->GetCellValue(i, NAME_COL);
        if (n.empty()) continue;
        std::string ns = n.ToStdString();
        if (nameToRows.find(ns) == nameToRows.end()) nameOrder.push_back(ns);
        nameToRows[ns].push_back(i);
    }
    std::sort(nameOrder.begin(), nameOrder.end());
    for (const auto& n : nameOrder)
        _stateAnimSteps.push_back(nameToRows[n]);
}

void ModelStatesPanel::StopStateAnimation()
{
    if (!_stateAnimPlaying) return;
    _stateAnimTimer.Stop();
    _stateAnimPlaying = false;
    if (Button_PlayStates) Button_PlayStates->SetLabel(_("Play"));
    NodeRangeGrid->EnableEditing(true);
    SelectRow(NodeRangeGrid, -1);
}

void ModelStatesPanel::OnPlayStatesClick(wxCommandEvent& event)
{
    if (_stateAnimPlaying) {
        StopStateAnimation();
        return;
    }
    BuildStateAnimSteps();
    if (_stateAnimSteps.empty()) return;

    _stateAnimPlaying = true;
    _stateAnimStep = 0;
    Button_PlayStates->SetLabel(_("Stop"));
    NodeRangeGrid->EnableEditing(false);

    _stateAnimTimer.Start(Spin_StateSpeed->GetValue(), false);
}

void ModelStatesPanel::OnStateAnimTimerTick(wxTimerEvent& event)
{
    if (!model || _stateAnimSteps.empty()) {
        StopStateAnimation();
        return;
    }
    _selected.clear();
    ClearNodeColor(model);
    int const draw_mode = ChoiceColorDraw ? ChoiceColorDraw->GetCurrentSelection() : 0;
    bool const force_custom = draw_mode == 0 && CustomColorNodeRanges->IsChecked();
    const auto& rows = _stateAnimSteps[_stateAnimStep % (int)_stateAnimSteps.size()];
    for (int i : rows) {
        if (i < NodeRangeGrid->GetNumberRows()) {
            xlColor const c = GetRowColor(NodeRangeGrid, i, false, force_custom);
            SetNodeColor(NodeRangeGrid, i, c, true);
        }
    }
    NodeRangeGrid->Refresh();
    if (_modelPreview) model->DisplayEffectOnWindow(_modelPreview, mPointSize);
    _stateAnimStep = (_stateAnimStep + 1) % (int)_stateAnimSteps.size();
}

void ModelStatesPanel::ClearStates(wxGridEvent& event)
{
    UpdateStateType();
    ValidateWindow();
}

void ModelStatesPanel::ClearSelectedStates(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty()) {
        return;
    }

    if (stateData[name]["Type"] != "NodeRange") {
        return;
    }
    for (int k = 0; k < NodeRangeGrid->GetNumberCols(); k++) {
        for (int i = NodeRangeGrid->GetNumberRows() - 1; i >= 0; i--) {
            if (NodeRangeGrid->IsInSelection(i, k)) {
                NodeRangeGrid->SetCellValue(i, k, wxEmptyString);

                if (k == COLOUR_COL) {
                    NodeRangeGrid->SetCellBackgroundColour(i, k, *wxWHITE);
                }
                GetValue(NodeRangeGrid, i, k, stateData[name]);
            }
        }
    }
    ValidateWindow();
    NodeRangeGrid->Refresh();
}

void ModelStatesPanel::AddBefore(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty() || stateData[name]["Type"] != "NodeRange") {
        return;
    }

    std::set<int> selectedRows;
    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        for (int row = 0; row < NodeRangeGrid->GetNumberRows(); row++) {
            if (NodeRangeGrid->IsInSelection(row, col)) {
                selectedRows.insert(row);
            }
        }
    }

    if (selectedRows.empty()) {
        int cursorRow = NodeRangeGrid->GetGridCursorRow();
        if (cursorRow >= 0 && cursorRow < NodeRangeGrid->GetNumberRows()) {
            selectedRows.insert(cursorRow);
        }
    }

    if (selectedRows.empty()) {
        return;
    }

    int firstRow = *selectedRows.begin();
    NodeRangeGrid->InsertRows(firstRow, 1);

    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        NodeRangeGrid->SetCellValue(firstRow, col, wxEmptyString);
        if (col == COLOUR_COL) {
            NodeRangeGrid->SetCellBackgroundColour(firstRow, col, *wxWHITE);
        }
    }

    for (int row = firstRow; row < NodeRangeGrid->GetNumberRows(); row++) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            GetValue(NodeRangeGrid, row, col, stateData[name]);
        }
    }

    ValidateWindow();
    NodeRangeGrid->Refresh();
}

void ModelStatesPanel::AddAfter(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty() || stateData[name]["Type"] != "NodeRange") {
        return;
    }

    std::set<int> selectedRows;
    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        for (int row = 0; row < NodeRangeGrid->GetNumberRows(); row++) {
            if (NodeRangeGrid->IsInSelection(row, col)) {
                selectedRows.insert(row);
            }
        }
    }

    if (selectedRows.empty()) {
        int cursorRow = NodeRangeGrid->GetGridCursorRow();
        if (cursorRow >= 0 && cursorRow < NodeRangeGrid->GetNumberRows()) {
            selectedRows.insert(cursorRow);
        }
    }

    if (selectedRows.empty()) {
        return;
    }

    int lastRow = *selectedRows.rbegin();
    NodeRangeGrid->InsertRows(lastRow + 1, 1);

    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        NodeRangeGrid->SetCellValue(lastRow + 1, col, wxEmptyString);
        if (col == COLOUR_COL) {
            NodeRangeGrid->SetCellBackgroundColour(lastRow + 1, col, *wxWHITE);
        }
    }

    for (int row = lastRow + 1; row < NodeRangeGrid->GetNumberRows(); row++) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            GetValue(NodeRangeGrid, row, col, stateData[name]);
        }
    }

    ValidateWindow();
    NodeRangeGrid->Refresh();
}

void ModelStatesPanel::DeleteSelected(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty() || stateData[name]["Type"] != "NodeRange") {
        return;
    }

    std::set<int, std::greater<int>> selectedRows;
    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        for (int row = 0; row < NodeRangeGrid->GetNumberRows(); row++) {
            if (NodeRangeGrid->IsInSelection(row, col)) {
                selectedRows.insert(row);
            }
        }
    }

    if (selectedRows.empty()) {
        int cursorRow = NodeRangeGrid->GetGridCursorRow();
        if (cursorRow >= 0 && cursorRow < NodeRangeGrid->GetNumberRows()) {
            selectedRows.insert(cursorRow);
        }
    }

    if (selectedRows.empty()) {
        return;
    }

    for (int row : selectedRows) {
        NodeRangeGrid->DeleteRows(row, 1);
    }

    for (int row = 0; row < NodeRangeGrid->GetNumberRows(); row++) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            GetValue(NodeRangeGrid, row, col, stateData[name]);
        }
    }

    ValidateWindow();
    NodeRangeGrid->Refresh();
}

void ModelStatesPanel::MoveSelectedUp(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty() || stateData[name]["Type"] != "NodeRange") {
        return;
    }

    std::vector<std::pair<int, int>> selectedCells;
    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        for (int row = 0; row < NodeRangeGrid->GetNumberRows(); row++) {
            if (NodeRangeGrid->IsInSelection(row, col)) {
                selectedCells.emplace_back(row, col);
            }
        }
    }

    if (selectedCells.empty()) {
        int cursorRow = NodeRangeGrid->GetGridCursorRow();
        int cursorCol = NodeRangeGrid->GetGridCursorCol();
        if (cursorRow >= 0 && cursorRow < NodeRangeGrid->GetNumberRows() && cursorCol >= 0) {
            selectedCells.emplace_back(cursorRow, cursorCol);
        }
    }

    if (selectedCells.empty()) {
        return;
    }

    std::set<int> selectedRows;
    for (const auto& cell : selectedCells) {
        selectedRows.insert(cell.first);
    }

    if (*selectedRows.begin() == 0) {
        return;
    }

    for (int row : selectedRows) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            wxString temp = NodeRangeGrid->GetCellValue(row - 1, col);
            NodeRangeGrid->SetCellValue(row - 1, col, NodeRangeGrid->GetCellValue(row, col));
            NodeRangeGrid->SetCellValue(row, col, temp);

            if (col == COLOUR_COL) {
                wxColour colour = NodeRangeGrid->GetCellBackgroundColour(row - 1, col);
                NodeRangeGrid->SetCellBackgroundColour(row - 1, col, NodeRangeGrid->GetCellBackgroundColour(row, col));
                NodeRangeGrid->SetCellBackgroundColour(row, col, colour);
            }
        }
    }

    for (int row : selectedRows) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            GetValue(NodeRangeGrid, row - 1, col, stateData[name]);
            GetValue(NodeRangeGrid, row, col, stateData[name]);
        }
    }

    NodeRangeGrid->ClearSelection();
    bool firstSelection = true;
    for (const auto& cell : selectedCells) {
        int newRow = cell.first - 1;
        if (newRow >= 0) {
            NodeRangeGrid->SelectBlock(newRow, cell.second, newRow, cell.second, !firstSelection);
            if (firstSelection) {
                NodeRangeGrid->SetGridCursor(newRow, cell.second);
                firstSelection = false;
            }
        }
    }

    ValidateWindow();
    NodeRangeGrid->Refresh();
}

void ModelStatesPanel::MoveSelectedDown(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty() || stateData[name]["Type"] != "NodeRange") {
        return;
    }

    std::vector<std::pair<int, int>> selectedCells;
    for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
        for (int row = 0; row < NodeRangeGrid->GetNumberRows(); row++) {
            if (NodeRangeGrid->IsInSelection(row, col)) {
                selectedCells.emplace_back(row, col);
            }
        }
    }

    if (selectedCells.empty()) {
        int cursorRow = NodeRangeGrid->GetGridCursorRow();
        int cursorCol = NodeRangeGrid->GetGridCursorCol();
        if (cursorRow >= 0 && cursorRow < NodeRangeGrid->GetNumberRows() && cursorCol >= 0) {
            selectedCells.emplace_back(cursorRow, cursorCol);
        }
    }

    if (selectedCells.empty()) {
        return;
    }

    std::set<int, std::greater<int>> selectedRows;
    for (const auto& cell : selectedCells) {
        selectedRows.insert(cell.first);
    }

    if (*selectedRows.begin() == NodeRangeGrid->GetNumberRows() - 1) {
        return;
    }

    for (int row : selectedRows) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            wxString temp = NodeRangeGrid->GetCellValue(row + 1, col);
            NodeRangeGrid->SetCellValue(row + 1, col, NodeRangeGrid->GetCellValue(row, col));
            NodeRangeGrid->SetCellValue(row, col, temp);

            if (col == COLOUR_COL) {
                wxColour colour = NodeRangeGrid->GetCellBackgroundColour(row + 1, col);
                NodeRangeGrid->SetCellBackgroundColour(row + 1, col, NodeRangeGrid->GetCellBackgroundColour(row, col));
                NodeRangeGrid->SetCellBackgroundColour(row, col, colour);
            }
        }
    }

    for (int row : selectedRows) {
        for (int col = 0; col < NodeRangeGrid->GetNumberCols(); col++) {
            GetValue(NodeRangeGrid, row + 1, col, stateData[name]);
            GetValue(NodeRangeGrid, row, col, stateData[name]);
        }
    }

    NodeRangeGrid->ClearSelection();
    bool firstSelection = true;
    for (const auto& cell : selectedCells) {
        int newRow = cell.first + 1;
        if (newRow < NodeRangeGrid->GetNumberRows()) {
            NodeRangeGrid->SelectBlock(newRow, cell.second, newRow, cell.second, !firstSelection);
            if (firstSelection) {
                NodeRangeGrid->SetGridCursor(newRow, cell.second);
                firstSelection = false;
            }
        }
    }

    ValidateWindow();
    NodeRangeGrid->Refresh();
}

void ModelStatesPanel::SortState(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name.empty() || stateData[name]["Type"] != "NodeRange") {
        return;
    }

    int numRows = NodeRangeGrid->GetNumberRows();
    int numCols = NodeRangeGrid->GetNumberCols();
    if (numRows == 0 || numCols == 0) {
        return;
    }

    struct RowData {
        std::vector<wxString> values;
        std::vector<wxColour> colours;
        int originalIndex;
    };

    std::vector<RowData> rows;
    rows.reserve(numRows);
    for (int row = 0; row < numRows; ++row) {
        RowData rowData;
        rowData.values.resize(numCols);
        rowData.colours.resize(numCols, *wxWHITE);
        for (int col = 0; col < numCols; ++col) {
            if (col == COLOUR_COL) {
                rowData.colours[col] = NodeRangeGrid->GetCellBackgroundColour(row, col);
            } else {
                rowData.values[col] = NodeRangeGrid->GetCellValue(row, col);
            }
        }
        rowData.originalIndex = row;
        rows.push_back(rowData);
    }

    std::stable_sort(rows.begin(), rows.end(), [](const RowData& a, const RowData& b) {
        bool aEmpty = a.values[0].IsEmpty();
        bool bEmpty = b.values[0].IsEmpty();
        if (aEmpty != bEmpty) {
            return bEmpty;
        }
        if (!aEmpty && !bEmpty) {
            double aNum, bNum;
            bool aIsNum = wxString(a.values[0]).ToDouble(&aNum);
            bool bIsNum = wxString(b.values[0]).ToDouble(&bNum);
            if (aIsNum && bIsNum) {
                return aNum < bNum;
            }
            if (a.values[0] != b.values[0]) {
                return a.values[0] < b.values[0];
            }
        }
        return a.originalIndex < b.originalIndex;
    });

    NodeRangeGrid->BeginBatch();

    if (numRows > 0) {
        NodeRangeGrid->DeleteRows(0, numRows);
    }
    NodeRangeGrid->AppendRows(numRows);

    std::map<std::string, std::string> tempStateData;
    for (int row = 0; row < numRows; ++row) {
        wxString key = wxString::Format("s%03d", row + 1);
        for (int col = 0; col < numCols; ++col) {
            wxString value = rows[row].values[col];
            if (col == COLOUR_COL && (value != "" || rows[row].colours[col] != *wxWHITE)) {
                tempStateData[key.ToStdString() + "-Color"] = std::string(wxColourToXlColor(rows[row].colours[col]));
            } else if (col == NAME_COL && value != "") {
                tempStateData[key.ToStdString() + "-Name"] = value.Lower().ToStdString();
            } else if (col == CHANNEL_COL && value != "") {
                tempStateData[key.ToStdString()] = value.ToStdString();
            }
        }
    }

    for (auto it = stateData[name].begin(); it != stateData[name].end();) {
        if (it->first.find("s") == 0) {
            it = stateData[name].erase(it);
        } else {
            ++it;
        }
    }

    stateData[name].insert(tempStateData.begin(), tempStateData.end());

    for (int row = 0; row < numRows; ++row) {
        NodeRangeGrid->SetRowLabelValue(row, wxString::Format("%d", row + 1));
        for (int col = 0; col < numCols; ++col) {
            NodeRangeGrid->SetCellValue(row, col, rows[row].values[col]);
            if (col == COLOUR_COL) {
                NodeRangeGrid->SetCellBackgroundColour(row, col, rows[row].colours[col]);
            }
        }
    }

    NodeRangeGrid->EndBatch();
    NodeRangeGrid->ForceRefresh();
    ValidateWindow();
}

wxString ModelStatesPanel::GetDownloadStates()
{
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

void ModelStatesPanel::ExportStatesToOtherModels()
{
    if (wxMessageBox("Are you sure you want to Export this model's States to other models?\nThis will override all the other model's existing states and there is no way to undo it.", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }

    xLightsFrame* xlights = xLightsApp::GetFrame();
    wxArrayString choices = getModelList(&xlights->AllModels);

    wxMultiChoiceDialog dlg(this, "Export States to Other Models", "Choose Model(s)", choices);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        std::map<std::string, std::map<std::string, std::string>> sourceStates = GetStateInfo();
        for (auto const& idx : dlg.GetSelections()) {
            Model* targetModel = xlights->GetModel(choices.at(idx));
            targetModel->SetStateInfo(sourceStates);
            targetModel->IncrementChangeCount();
        }
    }
}

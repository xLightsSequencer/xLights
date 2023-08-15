/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelStateDialog.h"
#include "xLightsXmlFile.h"
#include "NodesGridCellEditor.h"
#include "ModelPreview.h"
#include "DimmingCurve.h"
#include "SevenSegmentDialog.h"
#include "NodeSelectGrid.h"
#include "models/Model.h"
#include "xLightsApp.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "support/VectorMath.h"
#include "models/CustomModel.h"

#include <log4cpp/Category.hh>

//(*InternalHeaders(ModelStateDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

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

//(*IdInit(ModelStateDialog)
const long ModelStateDialog::ID_STATICTEXT2 = wxNewId();
const long ModelStateDialog::ID_CHOICE3 = wxNewId();
const long ModelStateDialog::ID_BUTTON3 = wxNewId();
const long ModelStateDialog::ID_BUTTON_IMPORT = wxNewId();
const long ModelStateDialog::ID_BUTTON4 = wxNewId();
const long ModelStateDialog::ID_CHECKBOX1 = wxNewId();
const long ModelStateDialog::ID_BUTTON1 = wxNewId();
const long ModelStateDialog::ID_GRID_COROSTATES = wxNewId();
const long ModelStateDialog::ID_PANEL2 = wxNewId();
const long ModelStateDialog::ID_CHECKBOX2 = wxNewId();
const long ModelStateDialog::ID_CHECKBOX3 = wxNewId();
const long ModelStateDialog::ID_BUTTON2 = wxNewId();
const long ModelStateDialog::ID_GRID3 = wxNewId();
const long ModelStateDialog::ID_PANEL6 = wxNewId();
const long ModelStateDialog::ID_CHOICEBOOK1 = wxNewId();
const long ModelStateDialog::ID_PANEL5 = wxNewId();
const long ModelStateDialog::ID_PANEL_PREVIEW = wxNewId();
const long ModelStateDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)
const long ModelStateDialog::ID_TIMER1 = wxNewId();

const long ModelStateDialog::STATE_DIALOG_IMPORT_SUB = wxNewId();
const long ModelStateDialog::STATE_DIALOG_COPY_STATES = wxNewId();
const long ModelStateDialog::STATE_DIALOG_IMPORT_MODEL = wxNewId();
const long ModelStateDialog::STATE_DIALOG_IMPORT_FILE = wxNewId();
const long ModelStateDialog::STATE_DIALOG_COPY = wxNewId();
const long ModelStateDialog::STATE_DIALOG_RENAME = wxNewId();
const long ModelStateDialog::STATE_DIALOG_SHIFT = wxNewId();
const long ModelStateDialog::STATE_DIALOG_REVERSE = wxNewId();

BEGIN_EVENT_TABLE(ModelStateDialog,wxDialog)
	//(*EventTable(ModelStateDialog)
	//*)
END_EVENT_TABLE()

wxColourData ModelStateDialog::_colorData;

enum {
    SINGLE_NODE_STATE = 0,
    NODE_RANGE_STATE
};

enum {
    //ROWTITLE_COL = 0,
    NAME_COL = 0,
    CHANNEL_COL,
    COLOUR_COL
};


#include "models/Model.h"

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif


ModelStateDialog::ModelStateDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size):
    mPointSize(PIXEL_SIZE_ON_DIALOGS), _outputManager(outputManager)
{
	//(*Initialize(ModelStateDialog)
	wxButton* AddButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* PreviewSizer;
	wxPanel* CoroPanel;
	wxPanel* NodeRangePanel;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("State Definition"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinimumPaneSize(100);
	SplitterWindow1->SetSashGravity(0.5);
	Panel3 = new wxPanel(SplitterWindow1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticText3 = new wxStaticText(Panel3, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NameChoice = new wxChoice(Panel3, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	NameChoice->SetMinSize(wxDLG_UNIT(Panel3,wxSize(100,-1)));
	FlexGridSizer7->Add(NameChoice, 1, wxALL|wxEXPAND, 5);
	AddButton = new wxButton(Panel3, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer7->Add(AddButton, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Import = new wxButton(Panel3, ID_BUTTON_IMPORT, _("..."), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
	Button_Import->SetMinSize(wxSize(20,-1));
	FlexGridSizer7->Add(Button_Import, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteButton = new wxButton(Panel3, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer7->Add(DeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	StateTypeChoice = new wxChoicebook(Panel3, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	CoroPanel = new wxPanel(StateTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	CustomColorSingleNode = new wxCheckBox(CoroPanel, ID_CHECKBOX1, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CustomColorSingleNode->SetValue(false);
	FlexGridSizer3->Add(CustomColorSingleNode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_7Segment = new wxButton(CoroPanel, ID_BUTTON1, _("7 Segment Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_7Segment, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROSTATES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROSTATES"));
	SingleNodeGrid->CreateGrid(200,3);
	SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel,wxSize(-1,200)));
	SingleNodeGrid->EnableEditing(true);
	SingleNodeGrid->EnableGridLines(true);
	SingleNodeGrid->SetColLabelSize(20);
	SingleNodeGrid->SetDefaultColSize(150, true);
	SingleNodeGrid->SetColLabelValue(0, _("State"));
	SingleNodeGrid->SetColLabelValue(1, _("Nodes"));
	SingleNodeGrid->SetColLabelValue(2, _("Color"));
	SingleNodeGrid->SetDefaultCellFont( SingleNodeGrid->GetFont() );
	SingleNodeGrid->SetDefaultCellTextColour( SingleNodeGrid->GetForegroundColour() );
	FlexGridSizer2->Add(SingleNodeGrid, 1, wxALL|wxEXPAND, 5);
	CoroPanel->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(CoroPanel);
	FlexGridSizer2->SetSizeHints(CoroPanel);
	NodeRangePanel = new wxPanel(StateTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer6->AddGrowableCol(1);
	CustomColorNodeRanges = new wxCheckBox(NodeRangePanel, ID_CHECKBOX2, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CustomColorNodeRanges->SetValue(false);
	FlexGridSizer6->Add(CustomColorNodeRanges, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_OutputToLights = new wxCheckBox(NodeRangePanel, ID_CHECKBOX3, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_OutputToLights->SetValue(false);
	FlexGridSizer6->Add(CheckBox_OutputToLights, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_7Seg = new wxButton(NodeRangePanel, ID_BUTTON2, _("7 Segment Display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer6->Add(Button_7Seg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);
	NodeRangeGrid = new wxGrid(NodeRangePanel, ID_GRID3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID3"));
	NodeRangeGrid->CreateGrid(200,3);
	NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel,wxSize(-1,200)));
	NodeRangeGrid->EnableEditing(true);
	NodeRangeGrid->EnableGridLines(true);
	NodeRangeGrid->SetColLabelSize(20);
	NodeRangeGrid->SetDefaultColSize(150, true);
	NodeRangeGrid->SetColLabelValue(0, _("State"));
	NodeRangeGrid->SetColLabelValue(1, _("Nodes"));
	NodeRangeGrid->SetColLabelValue(2, _("Color"));
	NodeRangeGrid->SetDefaultCellFont( NodeRangeGrid->GetFont() );
	NodeRangeGrid->SetDefaultCellTextColour( NodeRangeGrid->GetForegroundColour() );
	FlexGridSizer5->Add(NodeRangeGrid, 1, wxALL|wxEXPAND, 5);
	NodeRangePanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(NodeRangePanel);
	FlexGridSizer5->SetSizeHints(NodeRangePanel);
	StateTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
	StateTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
	FlexGridSizer4->Add(StateTypeChoice, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(Panel3, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(Panel3, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer4->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(Panel3);
	FlexGridSizer4->SetSizeHints(Panel3);
	ModelPreviewPanelLocation = new wxPanel(SplitterWindow1, ID_PANEL_PREVIEW, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_PREVIEW"));
	PreviewSizer = new wxFlexGridSizer(0, 1, 0, 0);
	PreviewSizer->AddGrowableCol(0);
	PreviewSizer->AddGrowableRow(0);
	ModelPreviewPanelLocation->SetSizer(PreviewSizer);
	PreviewSizer->Fit(ModelPreviewPanelLocation);
	PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);
	SplitterWindow1->SplitVertically(Panel3, ModelPreviewPanelLocation);
	FlexGridSizer1->Add(SplitterWindow1, 0, wxEXPAND, 0);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelStateDialog::OnMatrixNameChoiceSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButtonMatrixAddClicked);
	Connect(ID_BUTTON_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButton_ImportClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButtonMatrixDeleteClick);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCustomColorCheckboxClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButton_7SegmentClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellLeftClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellLeftDClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridLabelLeftClick);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellChange);
	Connect(ID_GRID_COROSTATES,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelStateDialog::OnSingleNodeGridCellSelect);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCustomColorCheckboxClick);
	Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnCheckBox_OutputToLightsClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelStateDialog::OnButton_7SegmentClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellLeftClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellRightClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridLabelLeftClick);
	Connect(ID_GRID3,wxEVT_GRID_LABEL_LEFT_DCLICK,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridLabelLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellChange);
	Connect(ID_GRID3,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelStateDialog::OnNodeRangeGridCellSelect);
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&ModelStateDialog::OnStateTypeChoicePageChanged);
	//*)

    model = nullptr;

    modelPreview = new ModelPreview(ModelPreviewPanelLocation);
    modelPreview->SetMinSize(wxSize(150, 150));
    PreviewSizer->Add(modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewSizer->Fit(ModelPreviewPanelLocation);
    PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);

    modelPreview->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&ModelStateDialog::OnPreviewLeftDown, nullptr, this);
    modelPreview->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&ModelStateDialog::OnPreviewLeftUp, nullptr, this);
    modelPreview->Connect(wxEVT_MOTION, (wxObjectEventFunction)&ModelStateDialog::OnPreviewMouseMove, nullptr, this);
    modelPreview->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&ModelStateDialog::OnPreviewMouseLeave, nullptr, this);
    modelPreview->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&ModelStateDialog::OnPreviewLeftDClick, nullptr, this);

    StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    SetEscapeId(wxID_CANCEL);
    EnableCloseButton(false);

    ValidateWindow();

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
    }
}

ModelStateDialog::~ModelStateDialog()
{
    //(*Destroy(ModelStateDialog)
    //*)
    StopOutputToLights();
    if (_oldOutputToLights) {
        _outputManager->StartOutput();
    }
}

void ModelStateDialog::SetStateInfo(Model* cls, std::map<std::string, std::map<std::string, std::string>>& finfo)
{
    NodeRangeGrid->SetColSize(COLOUR_COL, 50);
    SingleNodeGrid->SetColSize(COLOUR_COL, 50);
    NameChoice->Clear();
    model = cls;
    modelPreview->SetModel(cls);

    SetTitle(GetTitle() + " - " + cls->GetName());

    for (auto it = finfo.begin(); it != finfo.end(); ++it) {
        std::string name = it->first;
        std::map<std::string, std::string>& info = it->second;

        NameChoice->Append(name);

        std::string type2 = info["Type"];
        if (type2 == "") {
            // old style, map
            if (name == "Coro" || name == "SingleNode") {
                info["Type"] = "SingleNode";
            } else if (name == "NodeRange") {
                info["Type"] = "NodeRange";
            }
        }

        stateData[name] = info;
    }

    if (NameChoice->GetCount() > 0) {
        DeleteButton->Enable();
        StateTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    } else {
        DeleteButton->Disable();
        StateTypeChoice->Disable();
    }

    wxArrayString names;
    names.push_back("");
    for (size_t x = 0; x < cls->GetNodeCount(); x++) {
        wxString nn = cls->GetNodeName(x, true);
        names.push_back(nn);
    }

    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        wxGridCellTextEditor* neditor = new wxGridCellTextEditor();
        wxString nfilter("abcdefghijklmnopqrstuvwxyz0123456789-_/\\|#");
        wxTextValidator nvalidator(wxFILTER_INCLUDE_CHAR_LIST);
        nvalidator.SetCharIncludes(nfilter);
        neditor->SetValidator(nvalidator);

        NodesGridCellEditor* editor = new NodesGridCellEditor();
        editor->names = names;

        SingleNodeGrid->SetCellEditor(x, NAME_COL, neditor);
        SingleNodeGrid->SetCellEditor(x, CHANNEL_COL, editor);
        SingleNodeGrid->SetReadOnly(x, COLOUR_COL);
    }

    for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
        wxGridCellTextEditor* reditor = new wxGridCellTextEditor();
        wxString filter("0123456789,-");
        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
        validator.SetCharIncludes(filter);
        reditor->SetValidator(validator);

        wxGridCellTextEditor* neditor2 = new wxGridCellTextEditor();
        wxString nfilter2("abcdefghijklmnopqrstuvwxyz0123456789-_/\\|#");
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

void ModelStateDialog::GetStateInfo(std::map< std::string, std::map<std::string, std::string> > &finfo) {
    finfo.clear();
    for (const auto& it : stateData) {
        if (!it.second.empty()) {
            finfo[it.first] = it.second;
        }
    }
}

static bool SetGrid(wxGrid *grid, std::map<std::string, std::string> &info) {
    bool customColor = false;
    if (info["CustomColors"] == "1") {
        grid->ShowCol(COLOUR_COL);
        customColor = true;
    } else {
        grid->HideCol(COLOUR_COL);
    }
    for (int x = 0; x < grid->GetNumberRows(); x++) {
        wxString pname = "s" + grid->GetRowLabelValue(x);
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
            grid->SetCellBackgroundColour(x, COLOUR_COL, color.asWxColor());
        } else {
            grid->SetCellValue(x, CHANNEL_COL, "");
            grid->SetCellValue(x, NAME_COL, "");
            grid->SetCellBackgroundColour(x, COLOUR_COL, *wxWHITE);
        }
    }
    return customColor;
}

void ModelStateDialog::SelectStateModel(const std::string &name) {
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
        std::map<std::string, std::string> &info = stateData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
        grid = SingleNodeGrid;
    } else if (type == "NodeRange") {
        StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
        std::map<std::string, std::string> &info = stateData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
    }
    SelectRow(grid, -1);
}

void ModelStateDialog::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectStateModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    ValidateWindow();
}

void ModelStateDialog::OnButtonMatrixAddClicked(wxCommandEvent& event)
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
            if (model->GetDisplayAs() == "Custom") {
                CustomModel* cm = dynamic_cast<CustomModel*>(model);
                if (cm != nullptr) {
                    if (cm->IsAllNodesUnique()) {
                        StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
                    } else {
                        StateTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
                    }
                }
            } else if (model->GetDisplayAs() == "Channel Block") {
                StateTypeChoice->ChangeSelection(SINGLE_NODE_STATE);
            } else {
                StateTypeChoice->ChangeSelection(NODE_RANGE_STATE);
            }
            UpdateStateType();
        }
    }
    ValidateWindow();
}

void ModelStateDialog::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete state model definition?", "Are you sure you want to delete " + name + "?",
                         wxICON_WARNING | wxOK , this);
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
    }
    ValidateWindow();
}

void ModelStateDialog::OnCustomColorCheckboxClick(wxCommandEvent& event)
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

void ModelStateDialog::GetValue(wxGrid *grid, const int row, const int col, std::map<std::string, std::string> &info) {
    wxString key = "s" + grid->GetRowLabelValue(row).ToStdString();
    key.Replace(" ", "");
    if (key != "")
    {
        if (grid->GetCellValue(row, NAME_COL) != "" || grid->GetCellBackgroundColour(row, COLOUR_COL) != *wxWHITE || grid->GetCellValue(row, CHANNEL_COL) != "") {
            if (col == COLOUR_COL) {
                key += "-Color";
                xlColor color = grid->GetCellBackgroundColour(row, col);
                info[key.ToStdString()] = color;
            } else if (col == NAME_COL) {
                key += "-Name";
                info[key.ToStdString()] = grid->GetCellValue(row, col).Lower();
            } else {
                info[key.ToStdString()] = grid->GetCellValue(row, col);
            }
        }
        else {
            // if all the values are their defaults then delete them all so we dont keep them unnecessarily
            info.erase((key + "-Color").ToStdString());
            info.erase((key + "-Name").ToStdString());
            info.erase(key.ToStdString());
        }
    }
    SelectRow(grid, row);
}

void ModelStateDialog::ClearNodeColor(Model* m) {
    xlColor c(xlDARK_GREY);
    int nn = m->GetNodeCount();
    if (m->modelDimmingCurve) {
        m->modelDimmingCurve->apply(c);
    }
    for (int node = 0; node < nn; node++) {
        m->SetNodeColor(node, c);
    }
}

xlColor ModelStateDialog::GetRowColor(wxGrid* grid, int const row, bool const prev, bool const force) {
    if (force) {
        return xlColor(grid->GetCellBackgroundColour(row, COLOUR_COL));
    }
    if (prev) {
        return xlColor(255, 100, 255);
    }    
    return xlWHITE;
}

void ModelStateDialog::SelectRow(wxGrid* grid, int const r) {

    _selected.clear();
    ClearNodeColor(model);

    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        if (r == -1) {
            for (int i = 0; i < grid->GetNumberRows(); ++i) {
                xlColor const c = GetRowColor(grid, i, false, CustomColorSingleNode->IsChecked());
                SetSingleNodeColor(grid, i, c, true);
            }
        } else {
            for (int i = 0; i < grid->GetNumberRows(); ++i) {
                xlColor const c = GetRowColor(grid, i, r != i, CustomColorSingleNode->IsChecked());
                SetSingleNodeColor(grid, i, c, false);
            }
            // redo the selected row to ensure it is white
            xlColor const cc = GetRowColor(grid, r, false, CustomColorSingleNode->IsChecked());
            SetSingleNodeColor(grid, r, cc, true);
        }
    } else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        if (r == -1) {
            for (int i = 0; i < grid->GetNumberRows(); ++i) {
                xlColor const c = GetRowColor(grid, i, false, CustomColorNodeRanges->IsChecked());
                SetNodeColor(grid, i, c, true);
            }
        } else {
            for (int i = 0; i < grid->GetNumberRows(); ++i) {
                xlColor const c = GetRowColor(grid, i, r != i, CustomColorNodeRanges->IsChecked());
                SetNodeColor(grid, i, c, false);
            }
            // redo the selected row to ensure it is white
            xlColor const cc = GetRowColor(grid, r, false, CustomColorNodeRanges->IsChecked());
            SetNodeColor(grid, r, cc, true);
        }
    }
    grid->Refresh();
    model->DisplayEffectOnWindow(modelPreview, mPointSize);
}

void ModelStateDialog::SetSingleNodeColor(wxGrid* grid, const int row, xlColor const& c, bool highlight) {
    wxString v = grid->GetCellValue(row, CHANNEL_COL);
    wxStringTokenizer wtkz(v, ",");
    while (wtkz.HasMoreTokens()) {
        wxString valstr = wtkz.GetNextToken();
        for (size_t n = 0; n < model->GetNodeCount(); n++) {
            wxString ns = model->GetNodeName(n, true);
            if (ns == valstr) {
                model->SetNodeColor(n, c);
                if (highlight) _selected.push_back(n);
            }
        }
    }
}

bool ModelStateDialog::SetNodeColor(wxGrid* grid, int const row, xlColor const& c, bool highlight) {

    wxString v = grid->GetCellValue(row, CHANNEL_COL);
    if (v.empty()) {
        return false;
    }

    xlColor cc(c);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(cc);
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

void ModelStateDialog::OnNodeRangeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), stateData[name]);
}

void ModelStateDialog::OnSingleNodeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), stateData[name]);
}

void ModelStateDialog::OnStateTypeChoicePageChanged(wxChoicebookEvent& event)
{
    UpdateStateType();
}

void ModelStateDialog::UpdateStateType()
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
    ClearNodeColor(model);
}

void ModelStateDialog::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == CHANNEL_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const wxString title = name + " - " + NodeRangeGrid->GetCellValue(event.GetRow(), NAME_COL);
        bool wasOutputting = StopOutputToLights();
        { // we need to scope the dialog
            NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), _outputManager, this);

            if (dialog.ShowModal() == wxID_OK) {
                NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
                GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), stateData[name]);
                dialog.Close();
            }
        }
        if (wasOutputting)
            StartOutputToLights();
    }
    else if (event.GetCol() == COLOUR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK) {
            _colorData = dlg.GetColourData();
            NodeRangeGrid->SetCellBackgroundColour(event.GetRow(), COLOUR_COL, dlg.GetColourData().GetColour());
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), stateData[name]);
        }
    }
    SelectRow(NodeRangeGrid, event.GetRow());
}

void ModelStateDialog::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == COLOUR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOUR_COL);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK) {
            _colorData = dlg.GetColourData();
            SingleNodeGrid->SetCellBackgroundColour(event.GetRow(), COLOUR_COL, dlg.GetColourData().GetColour());
            SingleNodeGrid->Refresh();
            GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), stateData[name]);
        }
    }
    SelectRow(SingleNodeGrid, event.GetRow());
}

void ModelStateDialog::OnNodeRangeGridCellLeftClick(wxGridEvent& event)
{
    SelectRow(NodeRangeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnSingleNodeGridCellLeftClick(wxGridEvent& event)
{
    SelectRow(SingleNodeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnSingleNodeGridCellSelect(wxGridEvent& event)
{
    SelectRow(SingleNodeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnNodeRangeGridCellSelect(wxGridEvent& event)
{
    SelectRow(NodeRangeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip(); // continue the event
}

void ModelStateDialog::OnNodeRangeGridCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;

    mnu.Append(STATE_DIALOG_IMPORT_SUB, "Import SubModel");
    mnu.Append(STATE_DIALOG_COPY_STATES, "Copy States");

    mnu.Bind(wxEVT_COMMAND_MENU_SELECTED, [gridevent = event, this](wxCommandEvent & rightClkEvent) mutable {
        OnGridPopup(rightClkEvent.GetId(), gridevent);
    });
    PopupMenu(&mnu);

    event.Skip();
}

void ModelStateDialog::OnNodeRangeGridLabelLeftDClick(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    const wxString title = name + " - " + NodeRangeGrid->GetCellValue(event.GetRow(), NAME_COL);
    bool wasOutputting = StopOutputToLights();
    { // we need to scope the dialog
        NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), _outputManager, this);

        if (dialog.ShowModal() == wxID_OK) {
            NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
            GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, stateData[name]);
            dialog.Close();
        }
    }
    if (wasOutputting)
        StartOutputToLights();
}

void ModelStateDialog::OnSingleNodeGridLabelLeftClick(wxGridEvent& event) {
    SelectRow(SingleNodeGrid, -1);
}

void ModelStateDialog::OnNodeRangeGridLabelLeftClick(wxGridEvent& event) {
    SelectRow(NodeRangeGrid, -1);
}

void ModelStateDialog::OnButton_ImportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    if (DeleteButton->IsEnabled())
    {
        mnu.Append(STATE_DIALOG_COPY, "Copy");
        mnu.Append(STATE_DIALOG_RENAME, "Rename");
        mnu.AppendSeparator();
    }
    mnu.Append(STATE_DIALOG_IMPORT_MODEL, "Import From Model");
    mnu.Append(STATE_DIALOG_IMPORT_FILE, "Import From File");
    mnu.AppendSeparator();
    mnu.Append(STATE_DIALOG_SHIFT, "Shift Nodes");
    mnu.Append(STATE_DIALOG_REVERSE, "Reverse Nodes");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& ModelStateDialog::OnAddBtnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void ModelStateDialog::AddLabel(wxString label)
{
    int free = -1;
    wxGrid* grid = nullptr;
    if (StateTypeChoice->GetSelection() == SINGLE_NODE_STATE) {
        grid = SingleNodeGrid;
    }
    else if (StateTypeChoice->GetSelection() == NODE_RANGE_STATE) {
        grid = NodeRangeGrid;
    } else {
        return;
    }

    for (int i = 0; i < grid->GetNumberRows(); i++)
    {
        if (grid->GetCellValue(i, NAME_COL) == label)
        {
            free = -1;
            break;
        }

        if (grid->GetCellValue(i,NAME_COL) == "" && free == -1)
        {
            free = i;
        }
    }

    if (free != -1)
    {
        grid->SetCellValue(free, NAME_COL, label);
        wxString key = "s" + grid->GetRowLabelValue(free).ToStdString();
        key.Replace(" ", "");
        if (key != "")
        {
            std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
            key += "-Name";
            stateData[name][key.ToStdString()] = label.Lower();
        }
        SelectRow(grid, free);
    }
}

void ModelStateDialog::OnButton_7SegmentClick(wxCommandEvent& event)
{
    SevenSegmentDialog dialog(this);

    if (dialog.ShowModal() == wxID_OK)
    {
        if (dialog.CheckBox_Ones->IsChecked())
        {
            for (int i = 0; i < 10; i++)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Tens->IsChecked())
        {
            AddLabel("00");
            for (int i = 10; i < 100; i += 10)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Hundreds->IsChecked())
        {
            AddLabel("000");
            for (int i = 100; i < 1000; i += 100)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
        if (dialog.CheckBox_Colon->IsChecked())
        {
            AddLabel("colon");
        }
        if (dialog.CheckBox_Decimal->IsChecked())
        {
            AddLabel("dot");
        }
        if (dialog.CheckBox_Thousands->IsChecked())
        {
            AddLabel("0000");
            for (int i = 1000; i < 10000; i += 1000)
            {
                AddLabel(wxString::Format(wxT("%i"), i));
            }
        }
    }
}

void ModelStateDialog::ValidateWindow()
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
}

void ModelStateDialog::OnGridPopup(const int rightEventID, wxGridEvent& gridEvent)
{
    if (rightEventID == STATE_DIALOG_IMPORT_SUB) {
        ImportSubmodel(gridEvent);
    } else if (rightEventID == STATE_DIALOG_COPY_STATES) {
        CopyStates(gridEvent);
    }
}

void ModelStateDialog::ImportSubmodel(wxGridEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxArrayString choices;
    for (Model* sm : model->GetSubModels()) {
        choices.Add(sm->Name());
    }

    // don't offer a choice if there is nothing to choose
    if (choices.GetCount() == 0) {
        wxMessageBox("No SubModels Found.");
        return;
    }

    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    wxMultiChoiceDialog dlg(GetParent(), "", "Select SubModel", choices);

    if (dlg.ShowModal() == wxID_OK) {
        wxArrayString allNodes;
        for (auto const& idx : dlg.GetSelections()) {
            Model* sm = model->GetSubModel(choices.at(idx));
            if (sm == nullptr) {
                logger_base.error(
                    "Strange ... ModelStateDialog::ImportSubmodel returned no model "
                    "for %s but it was in the list we gave the user.",
                    (const char*)choices.at(idx).c_str());
                continue;
            }
            const auto nodes = getSubmodelNodes(sm);
            if (!nodes.IsEmpty()) {
                allNodes.Add(nodes);
            }
        }
        const auto newNodes = wxJoin(allNodes, ',', '\0');

        auto newNodeArrray = wxSplit(ExpandNodes(newNodes), ',');

        //sort
        std::sort(newNodeArrray.begin(), newNodeArrray.end(),
            [](const wxString& a, const wxString& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

        //make unique
        newNodeArrray.erase(std::unique(newNodeArrray.begin(), newNodeArrray.end()), newNodeArrray.end());

        NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, CompressNodes(wxJoin(newNodeArrray, ',')));
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, stateData[name]);
        dlg.Close();
    }
}

wxString ModelStateDialog::getSubmodelNodes(Model* sm)
{
    wxXmlNode* root = sm->GetModelXml();
    wxString row = "";

    if (root->GetName() == "subModel")
    {
        bool isRanges = root->GetAttribute("type", "") == "ranges";
        if (isRanges)
        {
            wxArrayString rows;
            int line = 0;
            while (root->HasAttribute(wxString::Format("line%d", line)))
            {
                auto l = root->GetAttribute(wxString::Format("line%d", line), "");
                rows.Add(l);
                line++;
            }
            row = wxJoin(rows, ',','\0');
        }
    }

    return row;
}

void ModelStateDialog::OnAddBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == STATE_DIALOG_IMPORT_MODEL)
    {
        ImportStatesFromModel();
    }
    else if (event.GetId() == STATE_DIALOG_IMPORT_FILE)
    {
        const wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;

        ImportStates(filename);
    }
    else if (event.GetId() == STATE_DIALOG_COPY)
    {
        CopyStateData();
    }
    else if (event.GetId() == STATE_DIALOG_RENAME)
    {
        RenameState();
    }
    else if(event.GetId() == STATE_DIALOG_SHIFT)
    {
        ShiftStateNodes();
    }
    else if(event.GetId() == STATE_DIALOG_REVERSE)
    {
        ReverseStateNodes();
    }
}

void ModelStateDialog::ImportStatesFromModel()
{
    xLightsFrame* xlights = xLightsApp::GetFrame();

    const wxArrayString choices = getModelList(&xlights->AllModels);

    wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);

    if (dlg.ShowModal() == wxID_OK)
    {
        Model* m = xlights->GetModel(dlg.GetStringSelection());
        if (m->stateInfo.size() == 0)
        {
            wxMessageBox(dlg.GetStringSelection() + " contains no states, skipping");
            return;
        }

        AddStates(m->stateInfo);

        NameChoice->Enable();
        StateTypeChoice->Enable();
        DeleteButton->Enable();

        NameChoice->SetSelection(NameChoice->GetCount() - 1);
        NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
        SelectStateModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
        ValidateWindow();
    }
}

void ModelStateDialog::ImportStates(const wxString & filename)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        std::map<std::string, std::map<std::string, std::string> > newStateInfo = stateData;
        wxXmlNode* root = doc.GetRoot();
        bool stateFound = false;

        for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == "stateInfo")
            {
                std::map<std::string, std::map<std::string, std::string> > stateInfo;
                Model::ParseStateInfo(n, stateInfo);
                if (stateInfo.size() == 0)
                {
                    continue;
                }
                stateFound = true;
                AddStates(stateInfo);
            }
        }

        if (stateFound)
        {
            NameChoice->Enable();
            StateTypeChoice->Enable();
            DeleteButton->Enable();

            NameChoice->SetSelection(NameChoice->GetCount() - 1);
            NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
            SelectStateModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
            ValidateWindow();
        }
        else
        {
            DisplayError(filename + " contains no states.");
        }
    }
    else
    {
        DisplayError(filename + " Failure loading xModel file.");
    }
}

void ModelStateDialog::AddStates(std::map<std::string, std::map<std::string, std::string> > states)
{
    bool overRide = false;
    bool showDialog = true;

    for (const auto& state : states)
    {
        auto fname = state.first;

        if (NameChoice->FindString(fname) != wxNOT_FOUND)
        {
            if (showDialog)
            {
                wxMessageDialog confirm(this, _("State(s) with the Same Name Already Exist.\n Would you Like to Override Them ALL?"), _("Override States"), wxYES_NO);
                int returnCode = confirm.ShowModal();
                if (returnCode == wxID_YES)
                    overRide = true;
                showDialog = false;
            }
            if (!overRide)
            {
                const auto basefname = fname;
                int suffix = 1;
                while (NameChoice->FindString(fname) != wxNOT_FOUND)
                {
                    fname = wxString::Format("%s-%d", basefname, suffix);
                    suffix++;
                }
                NameChoice->Append(fname);
            }
        }
        else
        {
            NameChoice->Append(fname);
        }

        stateData[fname] = state.second;
    }
}

wxArrayString ModelStateDialog::getModelList(ModelManager * modelManager)
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

void ModelStateDialog::CopyStates(wxGridEvent& event)
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
        wxArrayString allNodes;
        int stateIdx { 1 };
        for (auto const& idx : dlg.GetSelections()) {
            auto sd = stateData[choices.at(idx)];
            for (int x = 1; x <= 200; x++) {
                std::string pname = "s" + std::to_string(x);
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
                    
                    std::string newname = "s" + std::to_string(stateIdx);
                    stateData[name].insert({ newname, val });
                    stateData[name].insert({ newname + "-Name", n });
                    stateData[name].insert({ newname + "-Color", c });
                    ++stateIdx;
                } 
            }
        }
        SelectStateModel(name);
        ValidateWindow();
    }
}

void ModelStateDialog::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;

        SelectAllInBoundingRect(event.ShiftDown());
        m_creating_bound_rect = false;

        modelPreview->ReleaseMouse();
    }
}

void ModelStateDialog::OnPreviewMouseLeave(wxMouseEvent& event)
{
    RenderModel();
}

void ModelStateDialog::OnPreviewLeftDown(wxMouseEvent& event)
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

void ModelStateDialog::OnPreviewLeftDClick(wxMouseEvent& event)
{
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    int x = ray_origin.x;
    int y = ray_origin.y;
    wxString stNode = model->GetNodeNear(modelPreview, wxPoint(x, y), false);
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
    }
    else if (stateData[name]["Type"] == "NodeRange") {
        int row = NodeRangeGrid->GetGridCursorRow();
        if (row < 0)
            return;
        wxString oldnodes = ExpandNodes(NodeRangeGrid->GetCellValue(row, CHANNEL_COL));
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
        std::sort(oldNodeArrray.begin(), oldNodeArrray.end(),
            [](const wxString& a, const wxString& b)
            {
                return wxAtoi(a) < wxAtoi(b);
            });

        NodeRangeGrid->SetCellValue(row, CHANNEL_COL, CompressNodes(wxJoin(oldNodeArrray, ',')));
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, row, CHANNEL_COL, stateData[name]);
    }
}

void ModelStateDialog::OnPreviewMouseMove(wxMouseEvent& event)
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

void ModelStateDialog::RenderModel()
{
    if (modelPreview == nullptr || !modelPreview->StartDrawing(mPointSize)) return;

    if (m_creating_bound_rect) {
        modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
    }
    model->DisplayEffectOnWindow(modelPreview, mPointSize);
    modelPreview->EndDrawing();
}

void ModelStateDialog::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
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

void ModelStateDialog::SelectAllInBoundingRect(bool shiftDwn)
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

    std::vector<wxRealPoint> pts;
    std::vector<int> nodes = model->GetNodesInBoundingBox(modelPreview, wxPoint(m_bound_start_x, m_bound_start_y), wxPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;

    wxString oldnodes = ExpandNodes(NodeRangeGrid->GetCellValue(row, CHANNEL_COL));

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
        [](const wxString& a, const wxString& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

    NodeRangeGrid->SetCellValue(row, CHANNEL_COL, CompressNodes(wxJoin(oldNodeArrray, ',')));
    NodeRangeGrid->Refresh();
    GetValue(NodeRangeGrid, row, CHANNEL_COL, stateData[name]);
}

void ModelStateDialog::RemoveNodes()
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

    std::vector<wxRealPoint> pts;
    std::vector<int> nodes = model->GetNodesInBoundingBox(modelPreview, wxPoint(m_bound_start_x, m_bound_start_y), wxPoint(m_bound_end_x, m_bound_end_y));
    if (nodes.size() == 0)
        return;
    wxString oldnodes = ExpandNodes(NodeRangeGrid->GetCellValue(row, CHANNEL_COL));
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
        [](const wxString& a, const wxString& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

    NodeRangeGrid->SetCellValue(row, CHANNEL_COL, CompressNodes(wxJoin(oldNodeArrray, ',')));
    NodeRangeGrid->Refresh();
    GetValue(NodeRangeGrid, row, CHANNEL_COL, stateData[name]);
}

void ModelStateDialog::CopyStateData()
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

void ModelStateDialog::RenameState()
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

void ModelStateDialog::ShiftStateNodes()
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
            ShiftNodes(stateData[name], scaleFactor, min, max);
            SelectStateModel(name);
            ClearNodeColor(model);
        }
    }
}

void ModelStateDialog::ReverseStateNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (stateData[name]["Type"] != "NodeRange") {
        return;
    }

    long max = model->GetNodeCount() + 1;

    ReverseNodes(stateData[name], max);
    SelectStateModel(name);
    ClearNodeColor(model);
}

void ModelStateDialog::OnTimer1Trigger(wxTimerEvent& event)
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

void ModelStateDialog::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        _outputManager->StartOutput();
        timer1.SetOwner(this, ID_TIMER1);
        Connect(ID_TIMER1, wxEVT_TIMER, (wxObjectEventFunction)&ModelStateDialog::OnTimer1Trigger);
        timer1.Start(50, false);
    }
}

bool ModelStateDialog::StopOutputToLights()
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

void ModelStateDialog::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    }
    else {
        StopOutputToLights();
    }
}

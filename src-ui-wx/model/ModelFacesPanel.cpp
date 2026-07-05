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
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/numdlg.h>

#include "AI.xpm"
#include "E.xpm"
#include "ETC.xpm"
#include "FV.xpm"
#include "L.xpm"
#include "MBP.xpm"
#include "O.xpm"
#include "REST.xpm"
#include "U.xpm"
#include "WQ.xpm"

#include "ModelFacesPanel.h"
#include "render/SequenceFile.h"
#include "shared/utils/NodesGridCellEditor.h"
#include "layout/ModelPreview.h"
#include "render/DimmingCurve.h"
#include "utils/NodeUtils.h"
#include "utils/ExternalHooks.h"
#include "model/MatrixFaceDownloadDialog.h"
#include "xLightsMain.h"
#include "model/NodeSelectGrid.h"
#include "models/Model.h"
#include "models/Node.h"
#include "models/SubModel.h"
#include "xLightsApp.h"
#include "utils/VectorMath.h"
#include "models/CustomModel.h"
#include "outputs/OutputManager.h"
#include "color/xlColourData.h"
#include "shared/utils/wxUtilities.h"
#include "XmlSerializer/XmlSerializer.h"

#include <log.h>

#define CHANNEL_COL 0
#define COLOR_COL 1

// ID definitions — use wxNewId() to avoid collisions with ModelFaceDialog IDs
const wxWindowID ModelFacesPanel::ID_STATICTEXT2    = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICE3        = wxNewId();
const wxWindowID ModelFacesPanel::ID_BUTTON_ADD     = wxNewId();
const wxWindowID ModelFacesPanel::ID_BUTTON_MORE    = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL4         = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHECKBOX1      = wxNewId();
const wxWindowID ModelFacesPanel::ID_GRID_COROFACES = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL2         = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL8         = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHECKBOX2      = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHECKBOX3      = wxNewId();
const wxWindowID ModelFacesPanel::ID_GRID3          = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL6         = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL7         = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICE2        = wxNewId();
const wxWindowID ModelFacesPanel::ID_BUTTON1        = wxNewId();
const wxWindowID ModelFacesPanel::ID_GRID1          = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL3         = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICEBOOK1    = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL5         = wxNewId();
const wxWindowID ModelFacesPanel::ID_PANEL1         = wxNewId();
const long       ModelFacesPanel::ID_TIMER1         = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICE_STATE_OUTLINE  = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICE_OUTLINE_FILTER = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICE_EYES_FILTER    = wxNewId();
const wxWindowID ModelFacesPanel::ID_CHOICE_MOUTH_FILTER   = wxNewId();

const long ModelFacesPanel::FACES_DIALOG_IMPORT_SUB     = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_IMPORT_MODEL   = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_IMPORT_FILE    = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_EXPORT_TOOTHERS = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_COPY           = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_RENAME         = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_DELETE         = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_SHIFT          = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_REVERSE        = wxNewId();
const long ModelFacesPanel::FACES_DIALOG_BULK_COLOR_CHANGE = wxNewId();

BEGIN_EVENT_TABLE(ModelFacesPanel, wxPanel)
END_EVENT_TABLE()

#define MIN_COL_SIZE 10

enum {
    SINGLE_NODE_FACE = 0,
    NODE_RANGE_FACE,
    MATRIX_FACE
};

#ifndef wxEVT_GRID_CELL_CHANGE
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

ModelFacesPanel::ModelFacesPanel(wxWindow* parent, OutputManager* outputManager, wxWindowID id)
    : wxPanel(parent, id), _outputManager(outputManager), mPointSize(PIXEL_SIZE_ON_DIALOGS)
{
    wxButton* AddButton;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer10;
    wxPanel* CoroPanel;
    wxPanel* NodeRangePanel;
    wxStaticText* StaticText2;

    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(1);

    FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer7->AddGrowableCol(1);
    FlexGridSizer7->AddGrowableRow(0);

    StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    NameChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    NameChoice->SetMinSize(wxDLG_UNIT(this, wxSize(100, -1)));
    FlexGridSizer7->Add(NameChoice, 1, wxALL|wxEXPAND, 5);

    AddButton = new wxButton(this, ID_BUTTON_ADD, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_ADD"));
    FlexGridSizer7->Add(AddButton, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    ButtonMore = new wxButton(this, ID_BUTTON_MORE, _T("..."), wxDefaultPosition, wxSize(20, -1), 0, wxDefaultValidator, _T("ID_BUTTON_MORE"));
    ButtonMore->SetMinSize(wxSize(20, -1));
    FlexGridSizer7->Add(ButtonMore, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);

    FaceTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));

    // Single Nodes page
    CoroPanel = new wxPanel(FaceTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    FlexGridSizer2->AddGrowableRow(0);

    Panel_SingleNode = new wxPanel(CoroPanel, ID_PANEL4, wxDefaultPosition, wxSize(36, 0), wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer2->Add(Panel_SingleNode, 1, wxALL|wxEXPAND, 0);

    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    FlexGridSizer8->AddGrowableRow(1);

    CustomColorSingleNode = new wxCheckBox(CoroPanel, ID_CHECKBOX1, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CustomColorSingleNode->SetValue(false);
    FlexGridSizer8->Add(CustomColorSingleNode, 1, wxALL|wxEXPAND, 5);

    SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
    SingleNodeGrid->CreateGrid(13, 2);
    SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel, wxSize(-1, 200)));
    SingleNodeGrid->EnableEditing(true);
    SingleNodeGrid->EnableGridLines(true);
    SingleNodeGrid->SetColLabelSize(20);
    SingleNodeGrid->SetRowLabelSize(150);
    SingleNodeGrid->SetDefaultColSize(200, true);
    SingleNodeGrid->SetColLabelValue(0, _("Nodes"));
    SingleNodeGrid->SetColLabelValue(1, _("Color"));
    SingleNodeGrid->SetRowLabelValue(0, _("Face Outline"));
    SingleNodeGrid->SetRowLabelValue(1, _("Mouth - AI"));
    SingleNodeGrid->SetRowLabelValue(2, _("Mouth - E"));
    SingleNodeGrid->SetRowLabelValue(3, _("Mouth - etc"));
    SingleNodeGrid->SetRowLabelValue(4, _("Mouth - FV"));
    SingleNodeGrid->SetRowLabelValue(5, _("Mouth - L"));
    SingleNodeGrid->SetRowLabelValue(6, _("Mouth - MBP"));
    SingleNodeGrid->SetRowLabelValue(7, _("Mouth - O"));
    SingleNodeGrid->SetRowLabelValue(8, _("Mouth - rest"));
    SingleNodeGrid->SetRowLabelValue(9, _("Mouth - U"));
    SingleNodeGrid->SetRowLabelValue(10, _("Mouth - WQ"));
    SingleNodeGrid->SetRowLabelValue(11, _("Eyes - Open"));
    SingleNodeGrid->SetRowLabelValue(12, _("Eyes - Closed"));
    SingleNodeGrid->SetDefaultCellFont(SingleNodeGrid->GetFont());
    SingleNodeGrid->SetDefaultCellTextColour(SingleNodeGrid->GetForegroundColour());
    FlexGridSizer8->Add(SingleNodeGrid, 1, wxALL|wxEXPAND, 0);

    FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
    CoroPanel->SetSizer(FlexGridSizer2);

    // Node Ranges page
    NodeRangePanel = new wxPanel(FaceTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(1);
    FlexGridSizer5->AddGrowableRow(0);

    Panel_NodeRanges = new wxPanel(NodeRangePanel, ID_PANEL8, wxDefaultPosition, wxSize(36, 0), wxTAB_TRAVERSAL, _T("ID_PANEL8"));
    FlexGridSizer5->Add(Panel_NodeRanges, 1, wxALL|wxEXPAND, 0);

    FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer9->AddGrowableCol(0);
    FlexGridSizer9->AddGrowableRow(1);

    wxBoxSizer* NRToolbar = new wxBoxSizer(wxHORIZONTAL);

    // Col 0: checkboxes stacked vertically
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        CustomColorNodeRanges = new wxCheckBox(NodeRangePanel, ID_CHECKBOX2, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
        CustomColorNodeRanges->SetValue(false);
        col->Add(CustomColorNodeRanges, 0, wxALL|wxALIGN_LEFT, 3);
        CheckBox_OutputToLights = new wxCheckBox(NodeRangePanel, ID_CHECKBOX3, _("Output to Lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
        CheckBox_OutputToLights->SetValue(false);
        col->Add(CheckBox_OutputToLights, 0, wxALL|wxALIGN_LEFT, 3);
        NRToolbar->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    }

    // Col 1: Use States as Outline (label on top, dropdown below)
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        wxStaticText* StaticTextStateOutline = new wxStaticText(NodeRangePanel, wxID_ANY, _("Use States as Outline:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
        col->Add(StaticTextStateOutline, 0, wxALL|wxALIGN_LEFT, 2);
        StateOutlineChoice = new wxChoice(NodeRangePanel, ID_CHOICE_STATE_OUTLINE, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0, wxDefaultValidator, _T("ID_CHOICE_STATE_OUTLINE"));
        StateOutlineChoice->Append(wxEmptyString);
        StateOutlineChoice->SetSelection(0);
        col->Add(StateOutlineChoice, 0, wxEXPAND|wxALL, 2);
        NRToolbar->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    }

    // Col 2: Outline quick-preview
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        wxStaticText* lbl = new wxStaticText(NodeRangePanel, wxID_ANY, _("Outline:"), wxDefaultPosition, wxDefaultSize, 0);
        col->Add(lbl, 0, wxALL|wxALIGN_LEFT, 2);
        OutlineFilterChoice = new wxChoice(NodeRangePanel, ID_CHOICE_OUTLINE_FILTER, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0, wxDefaultValidator);
        OutlineFilterChoice->Append(wxEmptyString);
        OutlineFilterChoice->Append(_("All"));
        OutlineFilterChoice->Append(_("Outline"));
        OutlineFilterChoice->Append(_("Outline2"));
        OutlineFilterChoice->SetSelection(0);
        col->Add(OutlineFilterChoice, 0, wxEXPAND|wxALL, 2);
        NRToolbar->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    }

    // Col 3: Eyes quick-preview
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        wxStaticText* lbl = new wxStaticText(NodeRangePanel, wxID_ANY, _("Eyes:"), wxDefaultPosition, wxDefaultSize, 0);
        col->Add(lbl, 0, wxALL|wxALIGN_LEFT, 2);
        EyesFilterChoice = new wxChoice(NodeRangePanel, ID_CHOICE_EYES_FILTER, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0, wxDefaultValidator);
        EyesFilterChoice->Append(wxEmptyString);
        EyesFilterChoice->Append(_("Open"));
        EyesFilterChoice->Append(_("Closed"));
        EyesFilterChoice->SetSelection(0);
        col->Add(EyesFilterChoice, 0, wxEXPAND|wxALL, 2);
        NRToolbar->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    }

    // Col 4: Mouth quick-preview
    {
        wxBoxSizer* col = new wxBoxSizer(wxVERTICAL);
        wxStaticText* lbl = new wxStaticText(NodeRangePanel, wxID_ANY, _("Mouth:"), wxDefaultPosition, wxDefaultSize, 0);
        col->Add(lbl, 0, wxALL|wxALIGN_LEFT, 2);
        MouthFilterChoice = new wxChoice(NodeRangePanel, ID_CHOICE_MOUTH_FILTER, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0, wxDefaultValidator);
        MouthFilterChoice->Append(wxEmptyString);
        MouthFilterChoice->Append(_("AI"));
        MouthFilterChoice->Append(_("E"));
        MouthFilterChoice->Append(_("etc"));
        MouthFilterChoice->Append(_("FV"));
        MouthFilterChoice->Append(_("L"));
        MouthFilterChoice->Append(_("MBP"));
        MouthFilterChoice->Append(_("O"));
        MouthFilterChoice->Append(_("rest"));
        MouthFilterChoice->Append(_("U"));
        MouthFilterChoice->Append(_("WQ"));
        MouthFilterChoice->SetSelection(0);
        col->Add(MouthFilterChoice, 0, wxEXPAND|wxALL, 2);
        NRToolbar->Add(col, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    }

    NRToolbar->AddStretchSpacer(1);

    FlexGridSizer9->Add(NRToolbar, 1, wxALL|wxEXPAND, 5);

    NodeRangeGrid = new wxGrid(NodeRangePanel, ID_GRID3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID3"));
    NodeRangeGrid->CreateGrid(28, 2);
    NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel, wxSize(-1, 200)));
    NodeRangeGrid->EnableEditing(true);
    NodeRangeGrid->EnableGridLines(true);
    NodeRangeGrid->SetColLabelSize(20);
    NodeRangeGrid->SetRowLabelSize(150);
    NodeRangeGrid->SetDefaultColSize(200, true);
    NodeRangeGrid->SetColLabelValue(0, _("Nodes"));
    NodeRangeGrid->SetColLabelValue(1, _("Color"));
    NodeRangeGrid->SetRowLabelValue(0, _("Face Outline"));
    NodeRangeGrid->SetRowLabelValue(1, _("Face Outline 2"));
    NodeRangeGrid->SetRowLabelValue(2, _("Mouth - AI"));
    NodeRangeGrid->SetRowLabelValue(3, _("Mouth - E"));
    NodeRangeGrid->SetRowLabelValue(4, _("Mouth - etc"));
    NodeRangeGrid->SetRowLabelValue(5, _("Mouth - FV"));
    NodeRangeGrid->SetRowLabelValue(6, _("Mouth - L"));
    NodeRangeGrid->SetRowLabelValue(7, _("Mouth - MBP"));
    NodeRangeGrid->SetRowLabelValue(8, _("Mouth - O"));
    NodeRangeGrid->SetRowLabelValue(9, _("Mouth - rest"));
    NodeRangeGrid->SetRowLabelValue(10, _("Mouth - U"));
    NodeRangeGrid->SetRowLabelValue(11, _("Mouth - WQ"));
    NodeRangeGrid->SetRowLabelValue(12, _("Eyes - Open"));
    NodeRangeGrid->SetRowLabelValue(13, _("Eyes - Open 2"));
    NodeRangeGrid->SetRowLabelValue(14, _("Eyes - Open 3"));
    NodeRangeGrid->SetRowLabelValue(15, _("Eyes - Closed"));
    NodeRangeGrid->SetRowLabelValue(16, _("Eyes - Closed 2"));
    NodeRangeGrid->SetRowLabelValue(17, _("Eyes - Closed 3"));
    NodeRangeGrid->SetRowLabelValue(18, _("Mouth - AI 2"));
    NodeRangeGrid->SetRowLabelValue(19, _("Mouth - E 2"));
    NodeRangeGrid->SetRowLabelValue(20, _("Mouth - etc 2"));
    NodeRangeGrid->SetRowLabelValue(21, _("Mouth - FV 2"));
    NodeRangeGrid->SetRowLabelValue(22, _("Mouth - L 2"));
    NodeRangeGrid->SetRowLabelValue(23, _("Mouth - MBP 2"));
    NodeRangeGrid->SetRowLabelValue(24, _("Mouth - O 2"));
    NodeRangeGrid->SetRowLabelValue(25, _("Mouth - rest 2"));
    NodeRangeGrid->SetRowLabelValue(26, _("Mouth - U 2"));
    NodeRangeGrid->SetRowLabelValue(27, _("Mouth - WQ 2"));
    NodeRangeGrid->SetDefaultCellFont(NodeRangeGrid->GetFont());
    NodeRangeGrid->SetDefaultCellTextColour(NodeRangeGrid->GetForegroundColour());
    FlexGridSizer9->Add(NodeRangeGrid, 1, wxALL|wxEXPAND, 0);

    FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
    NodeRangePanel->SetSizer(FlexGridSizer5);

    // Matrix page
    Matrix = new wxPanel(FaceTypeChoice, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    FlexGridSizer3->AddGrowableRow(0);

    Panel_Matrix = new wxPanel(Matrix, ID_PANEL7, wxDefaultPosition, wxSize(36, 0), wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    FlexGridSizer3->Add(Panel_Matrix, 1, wxALL|wxEXPAND, 0);

    FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer10->AddGrowableCol(0);
    FlexGridSizer10->AddGrowableRow(1);

    FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer6->AddGrowableCol(1);
    FlexGridSizer6->AddGrowableRow(0);

    StaticText2 = new wxStaticText(Matrix, wxID_ANY, _("Image Placement:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    MatrixImagePlacementChoice = new wxChoice(Matrix, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    MatrixImagePlacementChoice->SetSelection(MatrixImagePlacementChoice->Append(_("Centered")));
    MatrixImagePlacementChoice->Append(_("Scaled"));
    MatrixImagePlacementChoice->Append(_("Scale Keep Aspect Ratio"));
    MatrixImagePlacementChoice->Append(_("Scale Keep Aspect Ratio Crop"));
    FlexGridSizer6->Add(MatrixImagePlacementChoice, 1, wxALL|wxEXPAND, 5);

    Button_DownloadImages = new wxButton(Matrix, ID_BUTTON1, _("Download Images"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer6->Add(Button_DownloadImages, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);

    FlexGridSizer10->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 5);

    MatrixModelsGrid = new wxGrid(Matrix, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
    MatrixModelsGrid->CreateGrid(10, 2);
    MatrixModelsGrid->SetMinSize(wxDLG_UNIT(Matrix, wxSize(-1, 200)));
    MatrixModelsGrid->EnableEditing(true);
    MatrixModelsGrid->EnableGridLines(true);
    MatrixModelsGrid->SetColLabelSize(20);
    MatrixModelsGrid->SetRowLabelSize(150);
    MatrixModelsGrid->SetDefaultColSize(200, true);
    MatrixModelsGrid->SetColLabelValue(0, _("Eyes Open"));
    MatrixModelsGrid->SetColLabelValue(1, _("Eyes Closed"));
    MatrixModelsGrid->SetRowLabelValue(0, _("Mouth - AI"));
    MatrixModelsGrid->SetRowLabelValue(1, _("Mouth - E"));
    MatrixModelsGrid->SetRowLabelValue(2, _("Mouth - etc"));
    MatrixModelsGrid->SetRowLabelValue(3, _("Mouth - FV"));
    MatrixModelsGrid->SetRowLabelValue(4, _("Mouth - L"));
    MatrixModelsGrid->SetRowLabelValue(5, _("Mouth - MBP"));
    MatrixModelsGrid->SetRowLabelValue(6, _("Mouth - O"));
    MatrixModelsGrid->SetRowLabelValue(7, _("Mouth - rest"));
    MatrixModelsGrid->SetRowLabelValue(8, _("Mouth - U"));
    MatrixModelsGrid->SetRowLabelValue(9, _("Mouth - WQ"));
    MatrixModelsGrid->SetDefaultCellFont(MatrixModelsGrid->GetFont());
    MatrixModelsGrid->SetDefaultCellTextColour(MatrixModelsGrid->GetForegroundColour());
    FlexGridSizer10->Add(MatrixModelsGrid, 1, wxALL|wxEXPAND, 0);

    FlexGridSizer3->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
    Matrix->SetSizer(FlexGridSizer3);

    FaceTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
    FaceTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
    FaceTypeChoice->AddPage(Matrix, _("Matrix"), false);
    FlexGridSizer4->Add(FaceTypeChoice, 1, wxALL|wxEXPAND, 5);

    this->SetSizer(FlexGridSizer4);
    this->Layout();

    // Wire up events
    Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ModelFacesPanel::OnMatrixNameChoiceSelect, this, ID_CHOICE3);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ModelFacesPanel::OnButtonMatrixAddClicked, this, ID_BUTTON_ADD);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ModelFacesPanel::OnButtonMoreClick, this, ID_BUTTON_MORE);
    Panel_SingleNode->Bind(wxEVT_PAINT, &ModelFacesPanel::Paint, this);
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &ModelFacesPanel::OnCustomColorCheckboxClick, this, ID_CHECKBOX1);
    Bind(wxEVT_GRID_CELL_LEFT_CLICK, &ModelFacesPanel::OnSingleNodeGridCellLeftClick, this, ID_GRID_COROFACES);
    Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &ModelFacesPanel::OnSingleNodeGridCellLeftDClick, this, ID_GRID_COROFACES);
    Bind(wxEVT_GRID_CELL_CHANGED, &ModelFacesPanel::OnSingleNodeGridCellChange, this, ID_GRID_COROFACES);
    Bind(wxEVT_GRID_SELECT_CELL, &ModelFacesPanel::OnSingleNodeGridCellSelect, this, ID_GRID_COROFACES);
    Bind(wxEVT_GRID_RANGE_SELECT, &ModelFacesPanel::OnSingleNodeGridRangeSelect, this, ID_GRID_COROFACES);
    SingleNodeGrid->Bind(wxEVT_SIZE, &ModelFacesPanel::OnSingleNodeGridResize, this);
    Panel_NodeRanges->Bind(wxEVT_PAINT, &ModelFacesPanel::Paint, this);
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &ModelFacesPanel::OnCustomColorCheckboxClick, this, ID_CHECKBOX2);
    Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &ModelFacesPanel::OnCheckBox_OutputToLightsClick, this, ID_CHECKBOX3);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ModelFacesPanel::OnStateOutlineChoiceSelect, this, ID_CHOICE_STATE_OUTLINE);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ModelFacesPanel::OnOutlineFilterSelect, this, ID_CHOICE_OUTLINE_FILTER);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ModelFacesPanel::OnEyesFilterSelect,    this, ID_CHOICE_EYES_FILTER);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ModelFacesPanel::OnMouthFilterSelect,   this, ID_CHOICE_MOUTH_FILTER);
    Bind(wxEVT_GRID_CELL_LEFT_CLICK, &ModelFacesPanel::OnNodeRangeGridCellLeftClick, this, ID_GRID3);
    Bind(wxEVT_GRID_CELL_RIGHT_CLICK, &ModelFacesPanel::OnNodeRangeGridCellRightClick, this, ID_GRID3);
    Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &ModelFacesPanel::OnNodeRangeGridCellLeftDClick, this, ID_GRID3);
    Bind(wxEVT_GRID_LABEL_LEFT_DCLICK, &ModelFacesPanel::OnNodeRangeGridLabelLeftDClick, this, ID_GRID3);
    Bind(wxEVT_GRID_CELL_CHANGED, &ModelFacesPanel::OnNodeRangeGridCellChange, this, ID_GRID3);
    Bind(wxEVT_GRID_SELECT_CELL, &ModelFacesPanel::OnNodeRangeGridCellSelect, this, ID_GRID3);
    Bind(wxEVT_GRID_RANGE_SELECT, &ModelFacesPanel::OnNodeRangeGridRangeSelect, this, ID_GRID3);
    NodeRangeGrid->Bind(wxEVT_SIZE, &ModelFacesPanel::OnNodeRangeGridResize, this);
    Panel_Matrix->Bind(wxEVT_PAINT, &ModelFacesPanel::Paint, this);
    Bind(wxEVT_COMMAND_CHOICE_SELECTED, &ModelFacesPanel::OnMatricImagePlacementChoiceSelect, this, ID_CHOICE2);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &ModelFacesPanel::OnButton_DownloadImagesClick, this, ID_BUTTON1);
    Bind(wxEVT_GRID_CELL_LEFT_CLICK, &ModelFacesPanel::OnMatrixModelsGridCellLeftClick1, this, ID_GRID1);
    Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &ModelFacesPanel::OnMatrixModelsGridCellLeftClick, this, ID_GRID1);
    Bind(wxEVT_GRID_LABEL_LEFT_DCLICK, &ModelFacesPanel::OnMatrixModelsGridLabelLeftDClick, this, ID_GRID1);
    Bind(wxEVT_GRID_CELL_CHANGED, &ModelFacesPanel::OnMatrixModelsGridCellChange, this, ID_GRID1);
    Bind(wxEVT_GRID_SELECT_CELL, &ModelFacesPanel::OnMatrixModelsGridCellSelect, this, ID_GRID1);
    MatrixModelsGrid->Bind(wxEVT_SIZE, &ModelFacesPanel::OnMatrixModelsGridResize, this);
    Bind(wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED, &ModelFacesPanel::OnFaceTypeChoicePageChanged, this, ID_CHOICEBOOK1);

    model = nullptr;
    MatrixImagePlacementChoice->SetStringSelection("Scaled");
    FaceTypeChoice->ChangeSelection(NODE_RANGE_FACE);

    // Wire timer
    timer1.SetOwner(this, ID_TIMER1);
    Bind(wxEVT_TIMER, &ModelFacesPanel::OnTimer1Trigger, this, ID_TIMER1);

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
        SetConfigBool("OutputActive", false);
    }
}

ModelFacesPanel::~ModelFacesPanel()
{
    StopOutputToLights();
    if (_oldOutputToLights) {
        if (_outputManager->StartOutput()) SetConfigBool("OutputActive", true);
    }
}

void ModelFacesPanel::SetModelPreview(ModelPreview* preview)
{
    _modelPreview = preview;
}

void ModelFacesPanel::NotifyChange()
{
    if (_changeCallback) _changeCallback();
}

void ModelFacesPanel::OnActivate()
{
    _isActive = true;
    if (_modelPreview) {
        _modelPreview->Bind(wxEVT_LEFT_DOWN,    &ModelFacesPanel::OnPreviewLeftDown,   this);
        _modelPreview->Bind(wxEVT_LEFT_UP,      &ModelFacesPanel::OnPreviewLeftUp,     this);
        _modelPreview->Bind(wxEVT_MOTION,       &ModelFacesPanel::OnPreviewMouseMove,  this);
        _modelPreview->Bind(wxEVT_LEAVE_WINDOW, &ModelFacesPanel::OnPreviewMouseLeave, this);
        _modelPreview->Bind(wxEVT_LEFT_DCLICK,  &ModelFacesPanel::OnPreviewLeftDClick, this);
        if (model) {
            _modelPreview->SetModel(model);
            CallAfter([this]() { UpdatePreview("", *wxWHITE); });
        }
    }
}

void ModelFacesPanel::SetSubModelCallbacks(std::function<std::vector<std::string>()> getNames,
                                           std::function<std::string(const std::string&)> getRanges)
{
    _getSubModelNames = std::move(getNames);
    _getSubModelRanges = std::move(getRanges);
}

void ModelFacesPanel::RefreshStateOutlineChoice(const std::map<std::string, std::map<std::string, std::string>>& stateInfo)
{
    if (!StateOutlineChoice) return;
    wxString current = StateOutlineChoice->GetStringSelection();
    StateOutlineChoice->Clear();
    StateOutlineChoice->Append(wxEmptyString);
    for (const auto& stEntry : stateInfo) {
        StateOutlineChoice->Append(stEntry.first);
    }
    int idx = StateOutlineChoice->FindString(current);
    StateOutlineChoice->SetSelection(idx != wxNOT_FOUND ? idx : 0);
}

void ModelFacesPanel::OnDeactivate()
{
    _isActive = false;
    m_creating_bound_rect = false;
    if (_modelPreview) {
        _modelPreview->Unbind(wxEVT_LEFT_DOWN,    &ModelFacesPanel::OnPreviewLeftDown,   this);
        _modelPreview->Unbind(wxEVT_LEFT_UP,      &ModelFacesPanel::OnPreviewLeftUp,     this);
        _modelPreview->Unbind(wxEVT_MOTION,       &ModelFacesPanel::OnPreviewMouseMove,  this);
        _modelPreview->Unbind(wxEVT_LEAVE_WINDOW, &ModelFacesPanel::OnPreviewMouseLeave, this);
        _modelPreview->Unbind(wxEVT_LEFT_DCLICK,  &ModelFacesPanel::OnPreviewLeftDClick, this);
    }
}

void ModelFacesPanel::SetFaceInfo(Model* cls, std::map<std::string, std::map<std::string, std::string>> const& finfo)
{
    NodeRangeGrid->SetColSize(1, 50);
    NodeRangeGrid->SetColSize(0, std::max(NodeRangeGrid->GetSize().x - 50 - NodeRangeGrid->GetRowLabelSize(), MIN_COL_SIZE));
    SingleNodeGrid->SetColSize(1, 50);
    SingleNodeGrid->SetColSize(0, std::max(SingleNodeGrid->GetSize().x - 50 - SingleNodeGrid->GetRowLabelSize(), MIN_COL_SIZE));
    MatrixModelsGrid->SetColSize(0, std::max((MatrixModelsGrid->GetSize().x - MatrixModelsGrid->GetRowLabelSize()) / 2, MIN_COL_SIZE));
    MatrixModelsGrid->SetColSize(1, std::max((MatrixModelsGrid->GetSize().x - MatrixModelsGrid->GetRowLabelSize()) / 2, MIN_COL_SIZE));
    NameChoice->Clear();
    model = cls;
    if (_modelPreview) _modelPreview->SetModel(cls);

    for (auto [name, info] : finfo) {

        NameChoice->Append(name);

        std::string type2 = info["Type"];
        if (type2 == "") {
            //old style, map
            if (name == "Coro" || name == "SingleNode") {
                info["Type"] = "SingleNode";
            } else if (name == "NodeRange") {
                info["Type"] = "NodeRange";
            } else {
                info["Type"] = "Matrix";
            }
        }

        // Only call FileUtils::FixFile for Matrix faces - NodeRange and SingleNode faces
        // store node numbers, not file paths
        if (info["Type"] == "Matrix") {
            for (std::map<std::string, std::string>::iterator it2 = info.begin(); it2 != info.end(); ++it2)
            {
                if (it2->first.substr(0, 5) == "Mouth" || it2->first.substr(0, 4) == "Eyes")
                {
                    it2->second = FileUtils::FixFile("", it2->second);
                }
            }
        }

        faceData[name] = info;
    }

    if (NameChoice->GetCount() > 0) {
        FaceTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectFaceModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    } else {
        FaceTypeChoice->Disable();
    }

    size_t nodeCount = cls->GetNodeCount();
    _nodeNameToIndex.clear();
    _nodeNameToIndex.reserve(nodeCount);

    static constexpr size_t MAX_NODES_FOR_DROPDOWN = 5000;
    const bool buildDropdown = (nodeCount <= MAX_NODES_FOR_DROPDOWN);

    if (buildDropdown) {
        wxArrayString names;
        names.reserve(nodeCount + 1);
        names.push_back("");
        for (size_t x = 0; x < nodeCount; x++) {
            std::string nn = cls->GetNodeName(x, true);
            _nodeNameToIndex[nn].push_back(x);
            names.push_back(wxString(nn));
        }
        NodesGridCellEditor* editor = new NodesGridCellEditor();
        editor->names = names;
        SingleNodeGrid->SetDefaultEditor(editor);
    } else {
        for (size_t x = 0; x < nodeCount; x++) {
            std::string nn = cls->GetNodeName(x, true);
            _nodeNameToIndex[nn].push_back(x);
        }
    }
    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        SingleNodeGrid->SetReadOnly(x, 1);
    }

    wxGridCellTextEditor* reditor = new wxGridCellTextEditor();
    wxString filter("0123456789,-");
    wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    validator.SetCharIncludes(filter);
    reditor->SetValidator(validator);
    NodeRangeGrid->SetDefaultEditor(reditor);
    for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
        NodeRangeGrid->SetReadOnly(x, 1);
    }

    StateOutlineChoice->Clear();
    StateOutlineChoice->Append(wxEmptyString);
    for (const auto& stEntry : cls->GetStateInfo()) {
        StateOutlineChoice->Append(stEntry.first);
    }
    StateOutlineChoice->SetSelection(0);
    ResetFaceFilterDropdowns();

    if (_modelPreview) CallAfter([this]() { UpdatePreview("", *wxWHITE); });

    std::list<std::string> warnings = cls->CheckModelSettings();
    if (!warnings.empty()) {
        std::string warningsStr = Join(warnings, "\n");
        wxMessageBox(wxString(warningsStr), "Warnings", wxOK | wxICON_WARNING);
    }
}

std::map<std::string, std::map<std::string, std::string>> ModelFacesPanel::GetFaceInfo() const
{
    if (SingleNodeGrid->IsCellEditControlShown()) {
        SingleNodeGrid->SaveEditControlValue();
        SingleNodeGrid->HideCellEditControl();
    }
    if (NodeRangeGrid->IsCellEditControlShown()) {
        NodeRangeGrid->SaveEditControlValue();
        NodeRangeGrid->HideCellEditControl();
    }
    std::map<std::string, std::map<std::string, std::string>> finfo;

    for (auto it = faceData.begin(); it != faceData.end(); ++it) {
        if (!it->second.empty()) {
            finfo[it->first] = it->second;
        }
    }
    return finfo;
}

static bool SetGrid(wxGrid* grid, std::map<std::string, std::string>& info)
{
    bool customColor = false;
    if (info["CustomColors"] == "1") {
        grid->ShowCol(1);
        grid->SetColSize(0, std::max(grid->GetSize().x - grid->GetColSize(1) - grid->GetRowLabelSize(), MIN_COL_SIZE));
        customColor = true;
    } else {
        grid->HideCol(1);
        grid->SetColSize(0, std::max(grid->GetSize().x - grid->GetRowLabelSize(), MIN_COL_SIZE));
    }
    for (int x = 0; x < grid->GetNumberRows(); x++) {
        wxString pname = grid->GetRowLabelValue(x);
        pname.Replace(" ", "");
        grid->SetCellValue(x, 0, info[pname.ToStdString()]);

        wxString c = info[pname.ToStdString() + "-Color"];
        if (c == "") {
            c = "#FFFFFF";
        }
        xlColor color(c);
        grid->SetCellBackgroundColour(x, 1, xlColorToWxColour(color));
        wxString pname1 = grid->GetRowLabelValue(x);
        if (pname1.Contains("Mouth") && pname1.EndsWith("2")) {
            customColor ? grid->ShowRow(x) : grid->HideRow(x);
        }
    }
    return customColor;
}

void ModelFacesPanel::SelectFaceModel(const std::string& name)
{
    FaceTypeChoice->Enable();
    wxString type = faceData[name]["Type"];
    if (type == "") {
        type = "SingleNode";
        faceData[name]["Type"] = type;
    }
    if (type == "SingleNode") {
        FaceTypeChoice->ChangeSelection(SINGLE_NODE_FACE);
        std::map<std::string, std::string>& info = faceData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
    } else if (type == "NodeRange") {
        FaceTypeChoice->ChangeSelection(NODE_RANGE_FACE);
        std::map<std::string, std::string>& info = faceData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
        UpdateFaceFilterDropdownItems();
    } else {
        FaceTypeChoice->ChangeSelection(MATRIX_FACE);
        for (int r = 0; r < MatrixModelsGrid->GetNumberRows(); r++) {
            for (int c = 0; c < MatrixModelsGrid->GetNumberCols(); c++) {
                wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
                key.Replace(" ", "");
                MatrixModelsGrid->SetCellValue(r, c, faceData[name][key.ToStdString()]);
                ValidateMatrixGrid(r, c);
            }
        }

        std::string w = "Centered";
        if (faceData[name].find("ImagePlacement") != faceData[name].end())
        {
            w = faceData[name]["ImagePlacement"];
        }
        MatrixImagePlacementChoice->SetStringSelection(w);
    }
}

void ModelFacesPanel::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectFaceModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
}

void ModelFacesPanel::OnButtonMatrixAddClicked(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "New Face", "Enter name for new face definition");
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);
            NameChoice->SetStringSelection(n);
            SelectFaceModel(n);
            NameChoice->Enable();
            FaceTypeChoice->Enable();

            if (model->GetDisplayAs() == DisplayAsType::Matrix || model->GetDisplayAs() == DisplayAsType::Tree) {
                FaceTypeChoice->ChangeSelection(MATRIX_FACE);
                wxChoicebookEvent event;
                OnFaceTypeChoicePageChanged(event);
            } else if (model->GetDisplayAs() == DisplayAsType::Custom) {
                CustomModel* cm = dynamic_cast<CustomModel*>(model);
                if (cm != nullptr) {
                    if (cm->IsAllNodesUnique()) {
                        FaceTypeChoice->ChangeSelection(NODE_RANGE_FACE);
                        wxChoicebookEvent event;
                        OnFaceTypeChoicePageChanged(event);
                    } else {
                        FaceTypeChoice->ChangeSelection(SINGLE_NODE_FACE);
                        wxChoicebookEvent event;
                        OnFaceTypeChoicePageChanged(event);
                    }
                }
            } else if (model->GetDisplayAs() == DisplayAsType::ChannelBlock) {
                FaceTypeChoice->ChangeSelection(SINGLE_NODE_FACE);
                wxChoicebookEvent event;
                OnFaceTypeChoicePageChanged(event);
            } else {
                FaceTypeChoice->ChangeSelection(NODE_RANGE_FACE);
                wxChoicebookEvent event;
                OnFaceTypeChoicePageChanged(event);
            }
        }
    }
    NotifyChange();
}

void ModelFacesPanel::OnFaceDeleteSelected()
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete face definition?", "Are you sure you want to delete " + name + "?",
                         wxICON_WARNING | wxOK | wxCANCEL, this);
    if (i == wxID_OK || i == wxOK) {
        faceData[name].clear();
        NameChoice->Delete(NameChoice->GetSelection());
        if (NameChoice->GetCount() > 0) {
            NameChoice->SetSelection(0);
            SelectFaceModel(NameChoice->GetString(0).ToStdString());
        } else {
            NameChoice->SetSelection(wxNOT_FOUND);
            NameChoice->Disable();
            FaceTypeChoice->Disable();
        }
        NotifyChange();
    }
}

void ModelFacesPanel::OnMatrixModelsGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    key.Replace(" ", "");
    wxString value = MatrixModelsGrid->GetCellValue(r, c);
    value.Trim();
    value.Trim(true);
    if (value.StartsWith("\"")) {
        value = value.SubString(1, value.size());
    }
    if (value.EndsWith("\"")) {
        value = value.Truncate(value.size() - 1);
    }

    if (!value.IsEmpty()) {
        TryToFindPath(value);
    }

    faceData[name][key.ToStdString()] = value;
    MatrixModelsGrid->SetCellValue(r, c, value);
    if (!value.IsEmpty()) {
        TryToSetAllMatrixModels(name, key.ToStdString(), value, r, c);
    }
    ValidateMatrixGrid(r, c);
    NotifyChange();
}

void ModelFacesPanel::OnMatrixModelsGridCellLeftClick(wxGridEvent& event)
{
    SelectMatrixImage(event.GetRow(), event.GetCol());
}

void ModelFacesPanel::OnMatrixModelsGridLabelLeftDClick(wxGridEvent& event)
{
    if (event.GetRow() < 0) {
        return;
    }
    SelectMatrixImage(event.GetRow(), 0);
}

static const wxString strSupportedImageTypes = "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                               ";*.webp"
                                               "|All files (*.*)|*.*";

void ModelFacesPanel::SelectMatrixImage(int r, int c)
{
    std::string const name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();

    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    wxFileName fname(MatrixModelsGrid->GetCellValue(r, c));
    wxFileDialog dlg(this, "Choose Image File for " + key, fname.GetPath(),
                     wxEmptyString,
                     strSupportedImageTypes,
                     wxFD_OPEN | wxFD_CHANGE_DIR);
    if (dlg.ShowModal() == wxID_OK) {
        wxString new_filename = dlg.GetPath();
        ObtainAccessToURL(new_filename.ToStdString());
        key.Replace(" ", "");
        faceData[name][key.ToStdString()] = new_filename;
        MatrixModelsGrid->SetCellValue(r, c, new_filename);
        ValidateMatrixGrid(r, c);

        TryToSetAllMatrixModels(name, key.ToStdString(), new_filename.ToStdString(), r, c);
    }
}

std::string FP_GenerateKey(int col, std::string key)
{
    if (col == 0)
    {
        return "Mouth-" + key + "-EyesOpen";
    }
    else
    {
        return "Mouth-" + key + "-EyesClosed";
    }
}

std::string FP_ExtractKey(wxString key)
{
    return key.AfterFirst('-').BeforeFirst('-').ToStdString();
}

// replace the count'th occurrence of key with phoneme and return the position
wxFileName FP_GetFileNamePhoneme(wxFileName fn, std::string key, int count, std::string phoneme)
{
    wxString base = fn.GetName();
    for (int i = 0; i <= count - 1; i++)
    {
        base.Replace(key, ":", false);
    }
    base.Replace(key, phoneme, false);
    base.Replace(":", key);

    wxFileName fn2(fn);
    fn2.SetName(base);

    return fn2;
}

void ModelFacesPanel::DoSetPhonemes(wxFileName fn, std::string actualkey, std::string key, int count, int row, int col, std::string name, std::list<std::string> phonemes, std::string setPhoneme)
{
    if (key == setPhoneme) return;

    for (auto it = phonemes.begin(); it != phonemes.end(); ++it)
    {
        wxFileName fn2 = FP_GetFileNamePhoneme(fn, actualkey, count, *it);
        if (FileExists(fn2) && (faceData[name][FP_GenerateKey(col, setPhoneme)] == "" || !FileExists(faceData[name][FP_GenerateKey(col, setPhoneme)])))
        {
            faceData[name][FP_GenerateKey(col, setPhoneme)] = fn2.GetFullPath();
            MatrixModelsGrid->SetCellValue(row, col, fn2.GetFullPath());
            ValidateMatrixGrid(row, col);
        }
    }
}

std::list<std::string> FP_GetPhonemes(std::string key)
{
    if (key == "AI")
    {
        std::list<std::string> phonemes = { "AI", "Ai", "ai", "A", "a", "A,I", "a,i" };
        return phonemes;
    }
    else if (key == "E")
    {
        std::list<std::string> phonemes = { "E", "e" };
        return phonemes;
    }
    else if (key == "etc")
    {
        std::list<std::string> phonemes = { "etc", "ETC", "Etc", "ect", "ECT", "Ect" };
        return phonemes;
    }
    else if (key == "FV")
    {
        std::list<std::string> phonemes = { "FV", "Fv", "fv", "F", "f", "F,V", "f,v" };
        return phonemes;
    }
    else if (key == "L")
    {
        std::list<std::string> phonemes = { "L", "l" };
        return phonemes;
    }
    else if (key == "MBP")
    {
        std::list<std::string> phonemes = { "MBP", "Mbp", "mbp" };
        return phonemes;
    }
    else if (key == "O")
    {
        std::list<std::string> phonemes = { "O", "o" };
        return phonemes;
    }
    else if (key == "rest")
    {
        std::list<std::string> phonemes = { "rest", "Rest", "REST" };
        return phonemes;
    }
    else if (key == "U")
    {
        std::list<std::string> phonemes = { "U", "u" };
        return phonemes;
    }
    else if (key == "WQ")
    {
        std::list<std::string> phonemes = { "WQ", "wq", "Wq", "W", "w", "W,Q", "w,q" };
        return phonemes;
    }
    else
    {
        wxASSERT(false);
        std::list<std::string> phonemes = {};
        return phonemes;
    }
}

void ModelFacesPanel::DoSetMatrixModels(wxFileName fn, std::string actualkey, std::string key, int count, int col, std::string name)
{
    int i = 0;
    for (auto it = _phonemes.begin(); it != _phonemes.end(); ++it)
    {
        DoSetPhonemes(fn, actualkey, key, count, i++, col, name, FP_GetPhonemes(*it), *it);
    }
}

void ModelFacesPanel::TryToFindPath(wxString& filename) const
{
    if (filename.IsEmpty()) {
        return;
    }
    if (FileExists(filename)) {
        return;
    }
    wxFileName fn = wxFileName(filename);
    wxArrayString files;
    GetAllFilesInDir(fn.GetPath(), files, fn.GetName() + ".*", wxDIR_DEFAULT);
    if (files.size() > 0) {
        filename = files[0];
        return;
    }

    GetAllFilesInDir(xLightsFrame::CurrentDir, files, fn.GetName() + ".*", wxDIR_DEFAULT);
    if (files.size() > 0) {
        filename = files[0];
        return;
    }

    GetAllFilesInDir(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "DownloadedFaces", files, fn.GetName() + ".*", wxDIR_DEFAULT);
    if (files.size() > 0) {
        filename = files[0];
        return;
    }
}

void ModelFacesPanel::TryToSetAllMatrixModels(std::string name, std::string key, std::string new_filename, int row, int col)
{
    if (new_filename.empty()) {
        return;
    }
    wxFileName fn = wxFileName(new_filename);

    std::string k = FP_ExtractKey(key);

    auto phonemes = FP_GetPhonemes(k);

    bool done = false;
    for (auto it = phonemes.begin(); !done && it != phonemes.end(); ++it)
    {
        int replacecount = fn.GetName().Replace(*it, "etc", true);

        for (int i = 0; i < replacecount; i++)
        {
            const std::list<std::string> findList = FP_GetPhonemes("etc");
            for (const auto& phen : findList)
            {
                const wxFileName fn2 = FP_GetFileNamePhoneme(fn, *it, i, phen);
                if (FileExists(fn2))
                {
                    DoSetMatrixModels(fn, *it, k, i, col, name);
                    done = true;
                    break;
                }
            }
        }
    }
}

void ModelFacesPanel::ValidateMatrixGrid(int r, int c) const
{
    wxString value = MatrixModelsGrid->GetCellValue(r, c);
    if (!FileExists(value) && !value.empty()) {
        MatrixModelsGrid->SetCellBackgroundColour(r, c, *wxRED);
    } else {
        MatrixModelsGrid->SetCellBackgroundColour(r, c, wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
    }
}

bool ModelFacesPanel::IsValidPhoneme(const std::string phoneme) const
{
    for (auto it = _phonemes.begin(); it != _phonemes.end(); ++it)
    {
        if (wxString(*it).Lower() == wxString(phoneme).Lower()) return true;
    }
    return false;
}

int ModelFacesPanel::GetRowForPhoneme(const std::string phoneme) const
{
    int row = 0;
    for (auto it = _phonemes.begin(); it != _phonemes.end(); ++it)
    {
        if (wxString(*it).Lower() == wxString(phoneme).Lower()) return row;
        row++;
    }
    return -1;
}

void ModelFacesPanel::OnMatricImagePlacementChoiceSelect(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    faceData[name]["ImagePlacement"] = MatrixImagePlacementChoice->GetStringSelection();
}

void ModelFacesPanel::OnCustomColorCheckboxClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE) {
        if (CustomColorSingleNode->IsChecked()) {
            SingleNodeGrid->ShowCol(1);
            SingleNodeGrid->SetColSize(0, std::max(SingleNodeGrid->GetSize().x - SingleNodeGrid->GetColSize(1) - SingleNodeGrid->GetRowLabelSize(), MIN_COL_SIZE));
            faceData[name]["CustomColors"] = "1";
        } else {
            SingleNodeGrid->HideCol(1);
            SingleNodeGrid->SetColSize(0, std::max(SingleNodeGrid->GetSize().x - SingleNodeGrid->GetRowLabelSize(), MIN_COL_SIZE));
            faceData[name]["CustomColors"] = "0";
            for (auto& it : faceData[name])
            {
                if (EndsWith(it.first, "-Color")) {
                    it.second = "";
                }
            }
            for (int r = 0; r < SingleNodeGrid->GetNumberRows(); r++) {
                SingleNodeGrid->SetCellBackgroundColour(r, COLOR_COL, *wxWHITE);
            }
        }
    } else {
        if (CustomColorNodeRanges->IsChecked()) {
            NodeRangeGrid->ShowCol(1);
            NodeRangeGrid->SetColSize(0, std::max(NodeRangeGrid->GetSize().x - NodeRangeGrid->GetColSize(1) - NodeRangeGrid->GetRowLabelSize(), MIN_COL_SIZE));
            faceData[name]["CustomColors"] = "1";
            for (int r = 0; r < NodeRangeGrid->GetNumberRows(); r++) {
                wxString pname1 = NodeRangeGrid->GetRowLabelValue(r);
                if (pname1.Contains("Mouth") && pname1.EndsWith("2")) {
                    NodeRangeGrid->ShowRow(r);
                }
            }
        } else {
            NodeRangeGrid->HideCol(1);
            NodeRangeGrid->SetColSize(0, std::max(NodeRangeGrid->GetSize().x - NodeRangeGrid->GetRowLabelSize(), MIN_COL_SIZE));
            faceData[name]["CustomColors"] = "0";
            for (auto& it : faceData[name]) {
                if (EndsWith(it.first, "-Color")) {
                    it.second = "";
                }
            }
            for (int r = 0; r < NodeRangeGrid->GetNumberRows(); r++) {
                NodeRangeGrid->SetCellBackgroundColour(r, COLOR_COL, *wxWHITE);
                wxString pname1 = NodeRangeGrid->GetRowLabelValue(r);
                if (pname1.Contains("Mouth") && pname1.EndsWith("2")) {
                    NodeRangeGrid->HideRow(r);
                }
            }
        }
    }
}

void ModelFacesPanel::GetValue(wxGrid* grid, const int row, const int col, std::map<std::string, std::string>& info)
{
    wxString key = grid->GetRowLabelValue(row).ToStdString();
    key.Replace(" ", "");
    if (col == 1) {
        key += "-Color";
        xlColor color = wxColourToXlColor(grid->GetCellBackgroundColour(row, col));
        info[key.ToStdString()] = std::string(color);
    } else {
        info[key.ToStdString()] = grid->GetCellValue(row, col);
        if (info["Type"] == "NodeRange") {
            auto nodeArray = wxSplit(grid->GetCellValue(row, col), ',');
            std::sort(nodeArray.begin(), nodeArray.end(),
                [](const wxString& a, const wxString& b) {
                    return wxAtoi(a) < wxAtoi(b);
                });
            grid->SetCellValue(row, col, NodeUtils::CompressNodes(wxJoin(nodeArray, ',')));
        }
    }
    UpdatePreview(grid->GetCellValue(row, CHANNEL_COL).ToStdString(), grid->GetCellBackgroundColour(row, COLOR_COL));
}

void ModelFacesPanel::ApplyStateOutlineOverlay()
{
    if (!model || !StateOutlineChoice) return;
    wxString sel = StateOutlineChoice->GetStringSelection();
    if (sel.empty()) return;

    std::string stateName = sel.ToStdString();
    const auto& stateInfoMap = model->GetStateInfo();
    auto stIt = stateInfoMap.find(stateName);
    if (stIt == stateInfoMap.end()) return;

    const auto& sts = stIt->second;
    auto findVal = [](const std::map<std::string, std::string>& m, const std::string& k) -> std::string {
        auto it = m.find(k);
        return it != m.end() ? it->second : "";
    };

    if (findVal(sts, "CustomColors") != "1") return;
    if (findVal(sts, "Type") != "NodeRange") return;

    const auto& stateNodes = model->GetStateInfoNodes();
    auto outerIt = stateNodes.find(stateName);
    if (outerIt == stateNodes.end()) return;

    for (int i = 1; i <= 200; i++) {
        std::string k = wxString::Format("s%03d", i).ToStdString();
        if (findVal(sts, k).empty()) continue;

        std::string c = findVal(sts, k + "-Color");
        xlColor colour = xlColor(c);
        if (c.empty()) colour = xlWHITE;
        if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(colour);

        auto innerIt = outerIt->second.find(k);
        if (innerIt == outerIt->second.end()) continue;

        for (int nodeIdx : innerIt->second) {
            if (nodeIdx >= 0 && nodeIdx < (int)model->GetNodeCount())
                model->SetNodeColor(nodeIdx, colour);
        }
    }
}

void ModelFacesPanel::UpdatePreview(const std::string& channels, wxColor c)
{
    if (!_modelPreview || !model) return;

    _selected.clear();

    int nn = model->GetNodeCount();
    xlColor cb(xlDARK_GREY);
    xlColor cc = wxColourToXlColor(c);
    if (model->GetDimmingCurve()) {
        model->GetDimmingCurve()->apply(cb);
        model->GetDimmingCurve()->apply(cc);
    }
    for (int node = 0; node < nn; ++node) {
        model->SetNodeColor(node, cb);
    }

    if (FaceTypeChoice->GetSelection() == MATRIX_FACE) {
        _modelPreview->SetbackgroundImage("");
        if (!channels.empty()) {
            wxString imgPath(channels);
            wxImage img(imgPath);
            if (img.IsOk() && img.GetWidth() > 0 && img.GetHeight() > 0) {
                int bufW = model->GetDefaultBufferWi();
                int bufH = model->GetDefaultBufferHt();
                int imgW = img.GetWidth();
                int imgH = img.GetHeight();
                wxString placement = MatrixImagePlacementChoice->GetStringSelection();
                int scaledW, scaledH, offsetX, offsetY;
                if (placement == "Centered") {
                    scaledW = imgW;
                    scaledH = imgH;
                    offsetX = (bufW - scaledW) / 2;
                    offsetY = (bufH - scaledH) / 2;
                } else if (placement == "Scaled") {
                    scaledW = bufW;
                    scaledH = bufH;
                    offsetX = 0;
                    offsetY = 0;
                    img = img.Scale(scaledW, scaledH, wxIMAGE_QUALITY_NEAREST);
                } else if (placement == "Scale Keep Aspect Ratio Crop") {
                    float scale = std::max((float)bufW / imgW, (float)bufH / imgH);
                    scaledW = (int)(imgW * scale);
                    scaledH = (int)(imgH * scale);
                    img = img.Scale(scaledW, scaledH, wxIMAGE_QUALITY_NEAREST);
                    offsetX = (bufW - scaledW) / 2;
                    offsetY = (bufH - scaledH) / 2;
                } else {
                    float scale = std::min((float)bufW / imgW, (float)bufH / imgH);
                    scaledW = (int)(imgW * scale);
                    scaledH = (int)(imgH * scale);
                    img = img.Scale(scaledW, scaledH, wxIMAGE_QUALITY_NEAREST);
                    offsetX = (bufW - scaledW) / 2;
                    offsetY = (bufH - scaledH) / 2;
                }
                int nodeCount = (int)model->GetNodeCount();
                for (int n = 0; n < nodeCount; ++n) {
                    NodeBaseClass* node = model->GetNode(n);
                    if (node && !node->Coords.empty()) {
                        int imgX = node->Coords[0].bufX - offsetX;
                        int imgY = scaledH - 1 - (node->Coords[0].bufY - offsetY);
                        if (imgX >= 0 && imgX < scaledW && imgY >= 0 && imgY < scaledH) {
                            xlColor col(img.GetRed(imgX, imgY), img.GetGreen(imgX, imgY), img.GetBlue(imgX, imgY));
                            model->SetNodeColor(n, col);
                        }
                    }
                }
            }
        }
        ApplyStateOutlineOverlay();
        model->DisplayEffectOnWindow(_modelPreview, mPointSize);
        return;
    }
    _modelPreview->SetbackgroundImage("");

    // Paint active filter-dropdown rows first so the current row renders on top.
    if (FaceTypeChoice->GetSelection() == NODE_RANGE_FACE) {
        for (int frow : GetActiveFilterRows()) {
            std::string fchan = NodeRangeGrid->GetCellValue(frow, CHANNEL_COL).ToStdString();
            if (fchan.empty()) continue;
            xlColor fc = wxColourToXlColor(NodeRangeGrid->GetCellBackgroundColour(frow, COLOR_COL));
            if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(fc);
            std::list<int> fch = model->ParseFaceNodes(fchan);
            for (const auto& it : fch) {
                if (it < nn) model->SetNodeColor(it, fc);
            }
        }
    }

    if (channels != "") {
        if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE) {
            wxStringTokenizer wtkz(channels, ",");
            while (wtkz.HasMoreTokens())
            {
                std::string valstr = wtkz.GetNextToken().ToStdString();
                auto it = _nodeNameToIndex.find(valstr);
                if (it != _nodeNameToIndex.end()) {
                    for (size_t idx : it->second) {
                        model->SetNodeColor(idx, cc);
                        _selected.push_back(idx);
                    }
                }
            }
        }
        else if (FaceTypeChoice->GetSelection() == NODE_RANGE_FACE) {
            std::list<int> ch = model->ParseFaceNodes(channels);
            for (const auto& it : ch)
            {
                if (it < (int)model->GetNodeCount())
                {
                    model->SetNodeColor(it, cc);
                    _selected.push_back(it);
                }
            }
        }
    }
    ApplyStateOutlineOverlay();
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

std::vector<int> ModelFacesPanel::GetSelectedGridRows(wxGrid* grid, int fallbackRow)
{
    std::set<int> rowSet;

    // Row-label selections (click / drag on row headers)
    wxArrayInt selRows = grid->GetSelectedRows();
    for (int r : selRows) rowSet.insert(r);

    // Block selections (click+drag across cells)
    wxGridCellCoordsArray topLeft = grid->GetSelectionBlockTopLeft();
    wxGridCellCoordsArray bottomRight = grid->GetSelectionBlockBottomRight();
    for (size_t i = 0; i < topLeft.size() && i < bottomRight.size(); ++i) {
        for (int r = topLeft[i].GetRow(); r <= bottomRight[i].GetRow(); ++r) {
            rowSet.insert(r);
        }
    }

    // Individual cell selections
    wxGridCellCoordsArray selCells = grid->GetSelectedCells();
    for (const auto& cell : selCells) {
        rowSet.insert(cell.GetRow());
    }

    if (rowSet.empty() && fallbackRow >= 0) {
        rowSet.insert(fallbackRow);
    }

    return std::vector<int>(rowSet.begin(), rowSet.end());
}

void ModelFacesPanel::UpdatePreviewRows(wxGrid* grid, int fallbackRow)
{
    if (!_modelPreview || !model) return;

    std::vector<int> rows = GetSelectedGridRows(grid, fallbackRow);
    if (rows.empty()) return;

    // Matrix face rows are image paths — multi-select doesn't meaningfully apply;
    // delegate to the existing single-row path using the top-most selected row.
    if (FaceTypeChoice->GetSelection() == MATRIX_FACE) {
        UpdatePreview(grid->GetCellValue(rows[0], CHANNEL_COL).ToStdString(),
                      grid->GetCellBackgroundColour(rows[0], COLOR_COL));
        return;
    }

    _selected.clear();
    _modelPreview->SetbackgroundImage("");

    int nn = model->GetNodeCount();
    xlColor cb(xlDARK_GREY);
    if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(cb);
    for (int node = 0; node < nn; ++node) {
        model->SetNodeColor(node, cb);
    }

    // Paint active filter-dropdown rows first so the clicked row paints on top.
    // This lets the user see which nodes are already claimed by the filter selection
    // while editing the currently selected row.
    if (grid == NodeRangeGrid) {
        for (int frow : GetActiveFilterRows()) {
            std::string channels = NodeRangeGrid->GetCellValue(frow, CHANNEL_COL).ToStdString();
            if (channels.empty()) continue;
            xlColor cc = wxColourToXlColor(NodeRangeGrid->GetCellBackgroundColour(frow, COLOR_COL));
            if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(cc);
            std::list<int> ch = model->ParseFaceNodes(channels);
            for (const auto& it : ch) {
                if (it < nn) model->SetNodeColor(it, cc);
            }
        }
    }

    int faceType = FaceTypeChoice->GetSelection();
    for (int row : rows) {
        std::string channels = grid->GetCellValue(row, CHANNEL_COL).ToStdString();
        if (channels.empty()) continue;

        xlColor cc = wxColourToXlColor(grid->GetCellBackgroundColour(row, COLOR_COL));
        if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(cc);

        if (faceType == SINGLE_NODE_FACE) {
            wxStringTokenizer wtkz(channels, ",");
            while (wtkz.HasMoreTokens()) {
                std::string valstr = wtkz.GetNextToken().ToStdString();
                auto it = _nodeNameToIndex.find(valstr);
                if (it != _nodeNameToIndex.end()) {
                    for (size_t idx : it->second) {
                        model->SetNodeColor(idx, cc);
                        _selected.push_back(idx);
                    }
                }
            }
        } else { // NODE_RANGE_FACE
            std::list<int> ch = model->ParseFaceNodes(channels);
            for (const auto& it : ch) {
                if (it < nn) {
                    model->SetNodeColor(it, cc);
                    _selected.push_back(it);
                }
            }
        }
    }

    ApplyStateOutlineOverlay();
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

void ModelFacesPanel::OnNodeRangeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), faceData[name]);
    if (event.GetCol() == CHANNEL_COL) UpdateFaceFilterDropdownItems();
    NotifyChange();
}

void ModelFacesPanel::OnSingleNodeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), faceData[name]);
    NotifyChange();
}

void ModelFacesPanel::OnFaceTypeChoicePageChanged(wxChoicebookEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    faceData[name].clear();
    switch (FaceTypeChoice->GetSelection()) {
        case SINGLE_NODE_FACE:
            faceData[name]["Type"] = "SingleNode";
            break;
        case NODE_RANGE_FACE:
            faceData[name]["Type"] = "NodeRange";
            break;
        case MATRIX_FACE:
            faceData[name]["Type"] = "Matrix";
            break;
    }
    SelectFaceModel(name);
    UpdatePreview("", *wxWHITE);
}

void ModelFacesPanel::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetRow() < 0) return;

    if (event.GetCol() == CHANNEL_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const wxString title = name + " - " + NodeRangeGrid->GetRowLabelValue(event.GetRow());
        bool wasOutputting = StopOutputToLights();
        {
            NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), _outputManager, this);

            if (dialog.ShowModal() == wxID_OK) {
                NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
                NodeRangeGrid->Refresh();
                GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), faceData[name]);
                UpdateFaceFilterDropdownItems();
                dialog.Close();
            }
        }
        if (wasOutputting)
            StartOutputToLights();
    }
    else if (event.GetCol() == COLOR_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL);
        auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, c);
        if (res == wxID_OK) {
            NodeRangeGrid->SetCellBackgroundColour(event.GetRow(), COLOR_COL, color);
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), faceData[name]);
        }
    }
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
}

void ModelFacesPanel::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetRow() < 0) return;

    if (event.GetCol() == COLOR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL);
        auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, c);
        if (res == wxID_OK) {
            SingleNodeGrid->SetCellBackgroundColour(event.GetRow(), COLOR_COL, color);
            SingleNodeGrid->Refresh();
            GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), faceData[name]);
        }
    }
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
}

void ModelFacesPanel::OnMatrixModelsGridCellSelect(wxGridEvent& event)
{
    UpdatePreview(MatrixModelsGrid->GetCellValue(event.GetRow(), event.GetCol()).ToStdString(), *wxWHITE);
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnMatrixModelsGridCellLeftClick1(wxGridEvent& event)
{
    UpdatePreview(MatrixModelsGrid->GetCellValue(event.GetRow(), event.GetCol()).ToStdString(), *wxWHITE);
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnSingleNodeGridCellSelect(wxGridEvent& event)
{
    UpdatePreviewRows(SingleNodeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnSingleNodeGridCellLeftClick(wxGridEvent& event)
{
    UpdatePreviewRows(SingleNodeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnNodeRangeGridCellLeftClick(wxGridEvent& event)
{
    UpdatePreviewRows(NodeRangeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnNodeRangeGridCellSelect(wxGridEvent& event)
{
    UpdatePreviewRows(NodeRangeGrid, event.GetRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnNodeRangeGridRangeSelect(wxGridRangeSelectEvent& event)
{
    UpdatePreviewRows(NodeRangeGrid, NodeRangeGrid->GetGridCursorRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::OnSingleNodeGridRangeSelect(wxGridRangeSelectEvent& event)
{
    UpdatePreviewRows(SingleNodeGrid, SingleNodeGrid->GetGridCursorRow());
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFacesPanel::PaintFace(wxDC& dc, int x, int y, const char* xpm[])
{
    wxImage i(xpm);
    wxBitmap bmp(i);
    dc.DrawBitmap(bmp, x, y);
}

void ModelFacesPanel::Paint(wxPaintEvent& event)
{
    wxPanel* p = (wxPanel*)event.GetEventObject();
    wxPaintDC dc(p);

    wxGrid* grid = nullptr;

    if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE)
    {
        grid = SingleNodeGrid;
        if (event.GetEventObject() != Panel_SingleNode) return;
    }
    else if (FaceTypeChoice->GetSelection() == NODE_RANGE_FACE)
    {
        grid = NodeRangeGrid;
        if (event.GetEventObject() != Panel_NodeRanges) return;
    }
    else if (FaceTypeChoice->GetSelection() == MATRIX_FACE)
    {
        grid = MatrixModelsGrid;
        if (event.GetEventObject() != Panel_Matrix) return;
    }
    else
    {
        return;
    }

    int x = 2;
    int dummy = 0;
    int y = 0;
    grid->GetPosition(&dummy, &y);
    y += grid->GetColLabelSize();

    for (int i = 0; i < grid->GetNumberRows(); i++)
    {
        wxString CellValue = grid->GetRowLabelValue(i);

        if (CellValue.EndsWith("AI"))
        {
            PaintFace(dc, x, y, AI_xpm);
        }
        else if (CellValue.EndsWith("E"))
        {
            PaintFace(dc, x, y, E_xpm);
        }
        else if (CellValue.EndsWith("etc"))
        {
            PaintFace(dc, x, y, ETC_xpm);
        }
        else if (CellValue.EndsWith("FV"))
        {
            PaintFace(dc, x, y, FV_xpm);
        }
        else if (CellValue.EndsWith("L"))
        {
            PaintFace(dc, x, y, L_xpm);
        }
        else if (CellValue.EndsWith("MBP"))
        {
            PaintFace(dc, x, y, MBP_xpm);
        }
        else if (CellValue.EndsWith("O"))
        {
            PaintFace(dc, x, y, O_xpm);
        }
        else if (CellValue.EndsWith("rest"))
        {
            PaintFace(dc, x, y, REST_xpm);
        }
        else if (CellValue.EndsWith("U"))
        {
            PaintFace(dc, x, y, U_xpm);
        }
        else if (CellValue.EndsWith("WQ"))
        {
            PaintFace(dc, x, y, WQ_xpm);
        }
        y += grid->GetRowHeight(i);
    }
}

std::string FP_FixPhonemeCase(const std::string p)
{
    wxString pp = wxString(p).Lower();

    if (pp == "ai") return "AI";
    if (pp == "etc") return "etc";
    if (pp == "fv") return "FV";
    if (pp == "wq") return "WQ";
    if (pp == "rest") return "rest";
    if (pp == "u") return "U";
    if (pp == "o") return "O";
    if (pp == "mbp") return "MBP";
    if (pp == "e") return "E";
    if (pp == "l") return "L";

    return "";
}

void ModelFacesPanel::OnButton_DownloadImagesClick(wxCommandEvent& event)
{
    if (xLightsFrame::CurrentDir == "") {
        wxMessageBox("Show folder is not valid. Face image download aborted.");
        return;
    }

    MatrixFaceDownloadDialog dlg(this);
    if (dlg.DlgInit(model->GetDefaultBufferWi(), model->GetDefaultBufferHt()))
    {
        if (dlg.ShowModal() == wxID_OK)
        {
            std::string faceZip = dlg.GetFaceFile();

            wxString const dir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + "DownloadedFaces";
            if (!wxDir::Exists(dir))
            {
                wxMkDir(dir, wxS_DIR_DEFAULT);
            }

            std::list<std::string> files;

            wxFileInputStream fin(faceZip);
            wxZipInputStream zin(fin);
            wxZipEntry* ent = zin.GetNextEntry();
            while (ent != nullptr)
            {
                if (ent->IsDir()) {
                    wxString dirname = dir + wxFileName::GetPathSeparator() + ent->GetName();
                    if (!wxDirExists(dirname)) {
                        spdlog::debug("Extracting dir {}:{} to {}.", (const char*)faceZip.c_str(), (const char*)ent->GetName().c_str(), (const char*)dirname.c_str());
                        wxFileName::Mkdir(dirname, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
                    }
                } else {
                    wxString filename = dir + wxFileName::GetPathSeparator() + ent->GetName();
                    files.push_back(filename);

                    if (!FileExists(filename))
                    {
#ifdef __WXMSW__
                        if (filename.length() > MAX_PATH) {
                            spdlog::warn("Target filename longer than {} chars ({}). This will likely fail. {}.", MAX_PATH, (int)filename.length(), (const char*)filename.c_str());
                        }
#endif

                        spdlog::debug("Extracting {}:{} to {}.", (const char*)faceZip.c_str(), (const char*)ent->GetName().c_str(), (const char*)filename.c_str());
                        wxFileOutputStream fout(filename);
                        zin.Read(fout);
                    }
                    if (!FileExists(filename)) {
                        spdlog::error("File extract failed.");
                    }
                }
                ent = zin.GetNextEntry();
            }

            std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();

            bool error = false;
            for (const auto& it : files)
            {
                wxFileName fn(it);
                wxString basefn = fn.GetName().Lower();
                bool eyesclosed = false;
                if (basefn.EndsWith("_eo"))
                {
                    basefn = basefn.SubString(0, basefn.Length() - 4);
                    eyesclosed = false;
                } else if (basefn.EndsWith("_ec"))
                {
                    basefn = basefn.SubString(0, basefn.Length() - 4);
                    eyesclosed = true;
                }

                std::string phoneme = FP_FixPhonemeCase(basefn.AfterLast('_').ToStdString());

                if (phoneme == "" || !IsValidPhoneme(phoneme))
                {
                    spdlog::warn("Phoneme '{}' was not known. File {} ignored.", phoneme, it);
                    error = true;
                }
                else
                {
                    std::string key = "Mouth-" + phoneme + "-" + (eyesclosed ? "EyesClosed" : "EyesOpen");
                    faceData[name][key] = it;
                    MatrixModelsGrid->SetCellValue(GetRowForPhoneme(phoneme), (eyesclosed ? 1 : 0), it);
                    ValidateMatrixGrid(GetRowForPhoneme(phoneme), (eyesclosed ? 1 : 0));
                }
            }

            if (error)
            {
                DisplayError("One or more images could not be mapped to the model due to issues with the names of files within the zip file. See log for details.");
            }
        }
    }
}

void ModelFacesPanel::OnNodeRangeGridCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;

    mnu.Append(FACES_DIALOG_IMPORT_SUB, "Import SubModel");

    if (CustomColorNodeRanges->IsChecked() &&
        NodeRangeGrid->GetSelectedRows().GetCount() >= 2) {
        mnu.AppendSeparator();
        mnu.Append(FACES_DIALOG_BULK_COLOR_CHANGE, "Bulk Color Change...");
    }

    mnu.Bind(wxEVT_COMMAND_MENU_SELECTED, [gridevent = event, this](wxCommandEvent& rightClkEvent) mutable {
        OnGridPopup(rightClkEvent.GetId(), gridevent);
    });
    PopupMenu(&mnu);

    event.Skip();
}

void ModelFacesPanel::OnNodeRangeGridLabelLeftDClick(wxGridEvent& event)
{
    if (event.GetRow() < 0) return;

    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    const wxString title = name + " - " + NodeRangeGrid->GetRowLabelValue(event.GetRow());
    bool wasOutputting = StopOutputToLights();
    {
        NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), _outputManager, this);

        if (dialog.ShowModal() == wxID_OK) {
            NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, faceData[name]);
            UpdateFaceFilterDropdownItems();
            dialog.Close();
        }
    }
    if (wasOutputting)
        StartOutputToLights();
}

void ModelFacesPanel::OnGridPopup(const int rightEventID, wxGridEvent& gridEvent)
{
    if (rightEventID == FACES_DIALOG_IMPORT_SUB) {
        ImportSubmodel(gridEvent);
    } else if (rightEventID == FACES_DIALOG_BULK_COLOR_CHANGE) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const auto rows = GetSelectedGridRows(NodeRangeGrid, gridEvent.GetRow());
        wxColor seed = NodeRangeGrid->GetCellBackgroundColour(rows[0], COLOR_COL);
        auto const& [res, color] = xlColourData::INSTANCE.ShowColorDialog(this, seed);
        if (res == wxID_OK) {
            for (int row : rows) {
                NodeRangeGrid->SetCellBackgroundColour(row, COLOR_COL, color);
                GetValue(NodeRangeGrid, row, COLOR_COL, faceData[name]);
            }
            NodeRangeGrid->Refresh();
        }
    }
}

void ModelFacesPanel::ImportSubmodel(wxGridEvent& event)
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
                        "Strange ... ModelFacesPanel::ImportSubmodel returned no model "
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
            [](const wxString& a, const wxString& b)
            {
                return wxAtoi(a) < wxAtoi(b);
            });

        newNodeArrray.erase(std::unique(newNodeArrray.begin(), newNodeArrray.end()), newNodeArrray.end());

        NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(newNodeArrray, ',')));
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, faceData[name]);
        UpdateFaceFilterDropdownItems();
        dlg.Close();
    }
}

wxString ModelFacesPanel::getSubmodelNodes(Model* sm)
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

void ModelFacesPanel::OnAddBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == FACES_DIALOG_IMPORT_MODEL)
    {
        ImportFacesFromModel();
        NotifyChange();
    }
    else if (event.GetId() == FACES_DIALOG_IMPORT_FILE)
    {
        const wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xModel Files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;

        ImportFaces(filename);
        NotifyChange();
    }
    else if (event.GetId() == FACES_DIALOG_COPY)
    {
        CopyFaceData();
        NotifyChange();
    }
    else if (event.GetId() == FACES_DIALOG_RENAME)
    {
        RenameFace();
        NotifyChange();
    }
    else if (event.GetId() == FACES_DIALOG_DELETE)
    {
        OnFaceDeleteSelected(); // NotifyChange() called internally
    }
    else if (event.GetId() == FACES_DIALOG_SHIFT)
    {
        ShiftFaceNodes();
        NotifyChange();
    }
    else if (event.GetId() == FACES_DIALOG_REVERSE)
    {
        ReverseFaceNodes();
        NotifyChange();
    } else if (event.GetId() == FACES_DIALOG_EXPORT_TOOTHERS) {
        ExportFacesToOtherModels();
        NotifyChange();
    }
}

void ModelFacesPanel::ImportFacesFromModel()
{
    xLightsFrame* xlights = xLightsApp::GetFrame();

    const wxArrayString choices = getModelList(&xlights->AllModels);

    wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);

    if (dlg.ShowModal() == wxID_OK)
    {
        Model* m = xlights->GetModel(dlg.GetStringSelection());
        if (m == nullptr) return;
        if (m->GetFaceInfo().size() == 0)
        {
            wxMessageBox(dlg.GetStringSelection() + " contains no signing faces, skipping");
            return;
        }

        AddFaces(m->GetFaceInfo());

        NameChoice->Enable();
        FaceTypeChoice->Enable();

        NameChoice->SetSelection(NameChoice->GetCount() - 1);
        NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
        SelectFaceModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
    }
}

void ModelFacesPanel::ImportFaces(const wxString& filename)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename.ToStdString().c_str());

    if (!result)
    {
        DisplayError(filename + " Failure loading xModel file.");
        return;
    }

    pugi::xml_node root = doc.document_element();
    if (!root)
    {
        DisplayError(filename + " contains invalid XML structure.");
        return;
    }

    FaceStateData allFaces;
    auto processFaceInfoChildren = [&](pugi::xml_node parent) {
        for (pugi::xml_node node : parent.children("faceInfo"))
            XmlSerialize::DeserializeFaceInfo(node, allFaces);
    };

    std::string_view rootName = root.name();
    if (rootName == "models") {
        for (pugi::xml_node model : root.children("model"))
            processFaceInfoChildren(model);
    } else {
        processFaceInfoChildren(root);
    }

    if (allFaces.empty())
    {
        DisplayError(filename + " contains no faces.");
        return;
    }

    AddFaces(allFaces);

    NameChoice->Enable();
    FaceTypeChoice->Enable();
    NameChoice->SetSelection(NameChoice->GetCount() - 1);
    NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
    SelectFaceModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
}

void ModelFacesPanel::AddFaces(std::map<std::string, std::map<std::string, std::string>> const& faces)
{
    bool overRide = false;
    bool showDialog = true;

    for (const auto& face : faces)
    {
        auto fname = face.first;

        if (NameChoice->FindString(fname) != wxNOT_FOUND)
        {
            if (showDialog)
            {
                wxMessageDialog confirm(this, _("Faces(s) with the Same Name Already Exist.\n Would you Like to Override Them ALL?"), _("Override Faces"), wxYES_NO);
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

        faceData[fname] = face.second;
    }
}

void ModelFacesPanel::CopyFaceData()
{
    auto const index = NameChoice->GetSelection();
    if (index == -1)
        return;
    auto const& currentName = NameChoice->GetString(index);
    wxTextEntryDialog dlg(this, "Copy Face", "Enter name for copied face definition", currentName);
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Append(n);

            faceData[n] = faceData[currentName];

            NameChoice->SetSelection(NameChoice->GetCount() - 1);
            NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
            SelectFaceModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
        } else {
            DisplayError("'" + n + "' face definition already exists.");
        }
    }
}

void ModelFacesPanel::RenameFace()
{
    auto const index = NameChoice->GetSelection();
    if (index == -1)
        return;
    auto const& currentName = NameChoice->GetString(index);
    wxTextEntryDialog dlg(this, "Rename Face", "Enter new name for face definition", currentName);
    if (dlg.ShowModal() == wxID_OK) {
        std::string n = dlg.GetValue().ToStdString();
        if (NameChoice->FindString(n) == wxNOT_FOUND) {
            NameChoice->Delete(index);
            NameChoice->Insert(n, index);

            auto face = std::move(faceData[currentName]);
            faceData[n] = std::move(face);
            faceData.erase(currentName);
            _renamedFaces.emplace_back(currentName.ToStdString(), n);

            NameChoice->SetSelection(index);
            NameChoice->SetStringSelection(NameChoice->GetString(index));
            SelectFaceModel(NameChoice->GetString(index).ToStdString());
        } else {
            DisplayError("'" + n + "' face definition already exists.");
        }
    }
}

wxArrayString ModelFacesPanel::getModelList(ModelManager* modelManager)
{
    wxArrayString choices;
    for (auto it = modelManager->begin(); it != modelManager->end(); ++it)
    {
        Model* m = it->second;
        if (m->Name() == model->Name())
            continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup)
            continue;
        choices.Add(m->Name());
    }
    return choices;
}

void ModelFacesPanel::OnPreviewLeftUp(wxMouseEvent& event)
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
    }
}

void ModelFacesPanel::OnPreviewMouseLeave(wxMouseEvent& event)
{
    if (!_isActive) return;
    m_creating_bound_rect = false;
    RenderModel();
}

void ModelFacesPanel::OnPreviewLeftDown(wxMouseEvent& event)
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
}

void ModelFacesPanel::OnPreviewLeftDClick(wxMouseEvent& event)
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

    if (faceData[name]["Type"] == "SingleNode") {
        int row = SingleNodeGrid->GetGridCursorRow();
        if (row < 0)
            return;

        std::string node = model->GetNodeName(wxAtoi(stNode) - 1, true);
        SingleNodeGrid->SetCellValue(row, CHANNEL_COL, node);
        SingleNodeGrid->Refresh();
        GetValue(SingleNodeGrid, row, CHANNEL_COL, faceData[name]);
    }
    else if (faceData[name]["Type"] == "NodeRange") {
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
            [](const wxString& a, const wxString& b)
            {
                return wxAtoi(a) < wxAtoi(b);
            });

        NodeRangeGrid->SetCellValue(row, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',')));
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, row, CHANNEL_COL, faceData[name]);
    }
}

void ModelFacesPanel::OnPreviewMouseMove(wxMouseEvent& event)
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

void ModelFacesPanel::RenderModel()
{
    if (_modelPreview == nullptr || !_modelPreview->StartDrawing(mPointSize)) return;

    if (m_creating_bound_rect) {
        _modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
    }
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
    _modelPreview->EndDrawing();
}

void ModelFacesPanel::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    if (!_modelPreview) return;
    VectorMath::ScreenPosToWorldRay(
        x, _modelPreview->getHeight() - y,
        _modelPreview->getWidth(), _modelPreview->getHeight(),
        _modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );
}

void ModelFacesPanel::SelectAllInBoundingRect(bool shiftDwn)
{
    if (shiftDwn) {
        RemoveNodes();
        return;
    }
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (faceData[name]["Type"] != "NodeRange") {
        return;
    }

    int row = NodeRangeGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<wxRealPoint> pts;
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
        [](const wxString& a, const wxString& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

    NodeRangeGrid->SetCellValue(row, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',')));
    NodeRangeGrid->Refresh();
    GetValue(NodeRangeGrid, row, CHANNEL_COL, faceData[name]);
    UpdateFaceFilterDropdownItems();
}

void ModelFacesPanel::RemoveNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (faceData[name]["Type"] != "NodeRange") {
        return;
    }
    int row = NodeRangeGrid->GetGridCursorRow();
    if (row < 0)
        return;

    std::vector<wxRealPoint> pts;
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
        [](const wxString& a, const wxString& b)
        {
            return wxAtoi(a) < wxAtoi(b);
        });

    NodeRangeGrid->SetCellValue(row, CHANNEL_COL, NodeUtils::CompressNodes(wxJoin(oldNodeArrray, ',')));
    NodeRangeGrid->Refresh();
    GetValue(NodeRangeGrid, row, CHANNEL_COL, faceData[name]);
    UpdateFaceFilterDropdownItems();
}

void ModelFacesPanel::ShiftFaceNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (faceData[name]["Type"] != "NodeRange") {
        return;
    }
    long min = 0;
    long max = model->GetNodeCount();

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK) {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0) {
            NodeUtils::ShiftNodes(faceData[name], scaleFactor, min, max);
            SelectFaceModel(name);
            UpdatePreview("", *wxWHITE);
        }
    }
}

void ModelFacesPanel::ReverseFaceNodes()
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (name == "") {
        return;
    }

    if (faceData[name]["Type"] != "NodeRange") {
        return;
    }

    long max = model->GetNodeCount() + 1;

    NodeUtils::ReverseNodes(faceData[name], max);
    SelectFaceModel(name);
    UpdatePreview("", *wxWHITE);
}

void ModelFacesPanel::OnTimer1Trigger(wxTimerEvent& event)
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

void ModelFacesPanel::StartOutputToLights()
{
    if (!timer1.IsRunning()) {
        if (_outputManager->StartOutput()) SetConfigBool("OutputActive", true);
        timer1.Start(50, false);
    }
}

bool ModelFacesPanel::StopOutputToLights()
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

void ModelFacesPanel::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        StartOutputToLights();
    } else {
        StopOutputToLights();
    }
}

void ModelFacesPanel::ResetFaceFilterDropdowns()
{
    if (OutlineFilterChoice) OutlineFilterChoice->SetSelection(0);
    if (EyesFilterChoice)    EyesFilterChoice->SetSelection(0);
    if (MouthFilterChoice)   MouthFilterChoice->SetSelection(0);
}

// Helper: returns true if any of the given NodeRangeGrid rows have a non-empty channel value.
static bool AnyRowHasNodes(wxGrid* grid, std::initializer_list<int> rowIndices)
{
    for (int r : rowIndices) {
        if (r < grid->GetNumberRows() && !grid->GetCellValue(r, CHANNEL_COL).IsEmpty())
            return true;
    }
    return false;
}

void ModelFacesPanel::UpdateFaceFilterDropdownItems()
{
    if (!OutlineFilterChoice || !EyesFilterChoice || !MouthFilterChoice) return;

    // --- Outline ---
    {
        wxString prev = OutlineFilterChoice->GetStringSelection();
        bool hasOutline  = AnyRowHasNodes(NodeRangeGrid, {0});
        bool hasOutline2 = AnyRowHasNodes(NodeRangeGrid, {1});
        OutlineFilterChoice->Clear();
        OutlineFilterChoice->Append(wxEmptyString);
        if (hasOutline && hasOutline2) {
            OutlineFilterChoice->Append(_("All"));
            OutlineFilterChoice->Append(_("Outline"));
            OutlineFilterChoice->Append(_("Outline2"));
        } else if (hasOutline) {
            OutlineFilterChoice->Append(_("Outline"));
        } else if (hasOutline2) {
            OutlineFilterChoice->Append(_("Outline2"));
        }
        int idx = OutlineFilterChoice->FindString(prev);
        OutlineFilterChoice->SetSelection(idx != wxNOT_FOUND ? idx : 0);
    }

    // --- Eyes ---
    {
        wxString prev = EyesFilterChoice->GetStringSelection();
        bool hasOpen   = AnyRowHasNodes(NodeRangeGrid, {12, 13, 14});
        bool hasClosed = AnyRowHasNodes(NodeRangeGrid, {15, 16, 17});
        EyesFilterChoice->Clear();
        EyesFilterChoice->Append(wxEmptyString);
        if (hasOpen)   EyesFilterChoice->Append(_("Open"));
        if (hasClosed) EyesFilterChoice->Append(_("Closed"));
        int idx = EyesFilterChoice->FindString(prev);
        EyesFilterChoice->SetSelection(idx != wxNOT_FOUND ? idx : 0);
    }

    // --- Mouth ---
    // Primary rows 2-11, secondary rows 18-27 share the same phoneme order.
    {
        static const wxString phonemes[] = { "AI","E","etc","FV","L","MBP","O","rest","U","WQ" };
        wxString prev = MouthFilterChoice->GetStringSelection();
        MouthFilterChoice->Clear();
        MouthFilterChoice->Append(wxEmptyString);
        for (int i = 0; i < 10; ++i) {
            if (AnyRowHasNodes(NodeRangeGrid, {2 + i, 18 + i}))
                MouthFilterChoice->Append(phonemes[i]);
        }
        int idx = MouthFilterChoice->FindString(prev);
        MouthFilterChoice->SetSelection(idx != wxNOT_FOUND ? idx : 0);
    }
}

std::vector<int> ModelFacesPanel::GetActiveFilterRows() const
{
    std::vector<int> rows;

    if (OutlineFilterChoice) {
        wxString sel = OutlineFilterChoice->GetStringSelection();
        if (sel == "All")           { rows.push_back(0); rows.push_back(1); }
        else if (sel == "Outline")  { rows.push_back(0); }
        else if (sel == "Outline2") { rows.push_back(1); }
    }

    if (EyesFilterChoice) {
        wxString sel = EyesFilterChoice->GetStringSelection();
        if (sel == "Open")         { rows.push_back(12); rows.push_back(13); rows.push_back(14); }
        else if (sel == "Closed")  { rows.push_back(15); rows.push_back(16); rows.push_back(17); }
    }

    if (MouthFilterChoice) {
        const wxString phonemes[] = { "AI","E","etc","FV","L","MBP","O","rest","U","WQ" };
        wxString sel = MouthFilterChoice->GetStringSelection();
        for (int i = 0; i < 10; ++i) {
            if (sel == phonemes[i]) {
                rows.push_back(2 + i);
                rows.push_back(18 + i);
                break;
            }
        }
    }

    return rows;
}

void ModelFacesPanel::UpdateFaceQuickPreview()
{
    if (!model || !_modelPreview) return;

    std::vector<int> rows = GetActiveFilterRows();

    if (rows.empty()) {
        UpdatePreviewRows(NodeRangeGrid, NodeRangeGrid->GetGridCursorRow());
        return;
    }

    _selected.clear();
    _modelPreview->SetbackgroundImage("");

    int nn = model->GetNodeCount();
    xlColor cb(xlDARK_GREY);
    if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(cb);
    for (int node = 0; node < nn; ++node) {
        model->SetNodeColor(node, cb);
    }

    for (int row : rows) {
        std::string channels = NodeRangeGrid->GetCellValue(row, CHANNEL_COL).ToStdString();
        if (channels.empty()) continue;
        xlColor cc = wxColourToXlColor(NodeRangeGrid->GetCellBackgroundColour(row, COLOR_COL));
        if (model->GetDimmingCurve()) model->GetDimmingCurve()->apply(cc);
        std::list<int> ch = model->ParseFaceNodes(channels);
        for (const auto& it : ch) {
            if (it < nn) {
                model->SetNodeColor(it, cc);
                _selected.push_back(it);
            }
        }
    }

    ApplyStateOutlineOverlay();
    model->DisplayEffectOnWindow(_modelPreview, mPointSize);
}

void ModelFacesPanel::OnOutlineFilterSelect(wxCommandEvent& event)
{
    UpdateFaceQuickPreview();
}

void ModelFacesPanel::OnEyesFilterSelect(wxCommandEvent& event)
{
    UpdateFaceQuickPreview();
}

void ModelFacesPanel::OnMouthFilterSelect(wxCommandEvent& event)
{
    UpdateFaceQuickPreview();
}

void ModelFacesPanel::OnStateOutlineChoiceSelect(wxCommandEvent& event)
{
    int faceType = FaceTypeChoice->GetSelection();
    if (faceType == SINGLE_NODE_FACE) {
        UpdatePreviewRows(SingleNodeGrid, SingleNodeGrid->GetGridCursorRow());
    } else if (faceType == NODE_RANGE_FACE) {
        UpdatePreviewRows(NodeRangeGrid, NodeRangeGrid->GetGridCursorRow());
    } else {
        UpdatePreview("", *wxWHITE);
    }
}

void ModelFacesPanel::OnMatrixModelsGridResize(wxSizeEvent& event)
{
    MatrixModelsGrid->SetColSize(0, std::max((event.GetSize().x - MatrixModelsGrid->GetRowLabelSize()) / 2, MIN_COL_SIZE));
    MatrixModelsGrid->SetColSize(1, std::max((event.GetSize().x - MatrixModelsGrid->GetRowLabelSize()) / 2, MIN_COL_SIZE));
}

void ModelFacesPanel::OnSingleNodeGridResize(wxSizeEvent& event)
{
    SingleNodeGrid->SetColSize(0, std::max(event.GetSize().x - SingleNodeGrid->GetColSize(1) - 30 - SingleNodeGrid->GetRowLabelSize(), MIN_COL_SIZE));
}

void ModelFacesPanel::OnNodeRangeGridResize(wxSizeEvent& event)
{
    NodeRangeGrid->SetColSize(0, std::max(event.GetSize().x - NodeRangeGrid->GetColSize(1) - 30 - NodeRangeGrid->GetRowLabelSize(), MIN_COL_SIZE));
}

void ModelFacesPanel::ExportFacesToOtherModels()
{
    if (wxMessageBox("Are you sure you want to Export this model's Face definitions to other models?\nThis will override all the other model's existing faces and there is no way to undo it.", "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }

    xLightsFrame* xlights = xLightsApp::GetFrame();
    wxArrayString choices = getModelList(&xlights->AllModels);

    wxMultiChoiceDialog dlg(this, "Export Face Definitions to Other Models", "Choose Model(s)", choices);
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        std::map<std::string, std::map<std::string, std::string>> sourceFaces = GetFaceInfo();
        for (auto const& idx : dlg.GetSelections()) {
            Model* targetModel = xlights->GetModel(choices.at(idx));
            targetModel->SetFaceInfo(sourceFaces);
            targetModel->IncrementChangeCount();
        }
    }
}

void ModelFacesPanel::OnButtonMoreClick(wxCommandEvent& event)
{
    wxMenu mnu;
    if (NameChoice->GetCount() > 0)
    {
        mnu.Append(FACES_DIALOG_COPY, "Copy");
        mnu.Append(FACES_DIALOG_RENAME, "Rename");
        mnu.Append(FACES_DIALOG_DELETE, "Delete");
        mnu.AppendSeparator();
    }
    mnu.Append(FACES_DIALOG_IMPORT_MODEL, "Import From Model");
    mnu.Append(FACES_DIALOG_IMPORT_FILE, "Import From File");
    mnu.Append(FACES_DIALOG_EXPORT_TOOTHERS, "Export Faces To Other Model(s)");
    mnu.AppendSeparator();
    mnu.Append(FACES_DIALOG_SHIFT, "Shift Nodes");
    mnu.Append(FACES_DIALOG_REVERSE, "Reverse Nodes");

    mnu.Bind(wxEVT_MENU, &ModelFacesPanel::OnAddBtnPopup, this);
    PopupMenu(&mnu);
}

void ModelFacesPanel::OnButtonImportClick(wxCommandEvent& event)
{
    // placeholder — import is handled via OnButtonMoreClick popup
}

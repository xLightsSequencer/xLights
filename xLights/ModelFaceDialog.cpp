/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 //(*InternalHeaders(ModelFaceDialog)
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
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/dir.h>

#include "../include/AI.xpm"
#include "../include/E.xpm"
#include "../include/ETC.xpm"
#include "../include/FV.xpm"
#include "../include/L.xpm"
#include "../include/MBP.xpm"
#include "../include/O.xpm"
#include "../include/REST.xpm"
#include "../include/U.xpm"
#include "../include/WQ.xpm"

#include "ModelFaceDialog.h"
#include "xLightsXmlFile.h"
#include "NodesGridCellEditor.h"
#include "ModelPreview.h"
#include "DimmingCurve.h"
#include "UtilFunctions.h"
#include "MatrixFaceDownloadDialog.h"
#include "xLightsMain.h"
#include "NodeSelectGrid.h"
#include "models/Model.h"
#include "xLightsApp.h"

#include <log4cpp/Category.hh>

#define CHANNEL_COL 0
#define COLOR_COL 1

//(*IdInit(ModelFaceDialog)
const long ModelFaceDialog::ID_STATICTEXT2 = wxNewId();
const long ModelFaceDialog::ID_CHOICE3 = wxNewId();
const long ModelFaceDialog::ID_BUTTON3 = wxNewId();
const long ModelFaceDialog::ID_BUTTON_IMPORT = wxNewId();
const long ModelFaceDialog::ID_BUTTON4 = wxNewId();
const long ModelFaceDialog::ID_PANEL4 = wxNewId();
const long ModelFaceDialog::ID_CHECKBOX1 = wxNewId();
const long ModelFaceDialog::ID_GRID_COROFACES = wxNewId();
const long ModelFaceDialog::ID_PANEL2 = wxNewId();
const long ModelFaceDialog::ID_PANEL5 = wxNewId();
const long ModelFaceDialog::ID_CHECKBOX2 = wxNewId();
const long ModelFaceDialog::ID_GRID3 = wxNewId();
const long ModelFaceDialog::ID_PANEL6 = wxNewId();
const long ModelFaceDialog::ID_PANEL7 = wxNewId();
const long ModelFaceDialog::ID_CHOICE2 = wxNewId();
const long ModelFaceDialog::ID_BUTTON1 = wxNewId();
const long ModelFaceDialog::ID_GRID1 = wxNewId();
const long ModelFaceDialog::ID_PANEL3 = wxNewId();
const long ModelFaceDialog::ID_CHOICEBOOK1 = wxNewId();
const long ModelFaceDialog::ID_PANEL1 = wxNewId();
//*)

const long ModelFaceDialog::FACES_DIALOG_IMPORT_SUB = wxNewId();
const long ModelFaceDialog::FACES_DIALOG_IMPORT_MODEL = wxNewId();
const long ModelFaceDialog::FACES_DIALOG_IMPORT_FILE = wxNewId();
const long ModelFaceDialog::FACES_DIALOG_COPY = wxNewId();
const long ModelFaceDialog::FACES_DIALOG_RENAME = wxNewId();

BEGIN_EVENT_TABLE(ModelFaceDialog,wxDialog)
	//(*EventTable(ModelFaceDialog)
	//*)
END_EVENT_TABLE()

wxColourData ModelFaceDialog::_colorData;

enum {
    SINGLE_NODE_FACE = 0,
    NODE_RANGE_FACE,
    MATRIX_FACE
};

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

ModelFaceDialog::ModelFaceDialog(wxWindow* parent,wxWindowID id, const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelFaceDialog)
	wxButton* AddButton;
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
	wxFlexGridSizer* PreviewSizer;
	wxPanel* CoroPanel;
	wxPanel* NodeRangePanel;
	wxStaticText* StaticText2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Face Definition"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	FlexGridSizer7 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer7->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NameChoice = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	NameChoice->SetMinSize(wxDLG_UNIT(this,wxSize(100,-1)));
	FlexGridSizer7->Add(NameChoice, 1, wxALL|wxEXPAND, 5);
	AddButton = new wxButton(this, ID_BUTTON3, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer7->Add(AddButton, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonImport = new wxButton(this, ID_BUTTON_IMPORT, _("..."), wxDefaultPosition, wxSize(20,-1), 0, wxDefaultValidator, _T("ID_BUTTON_IMPORT"));
	ButtonImport->SetMinSize(wxSize(20,-1));
	FlexGridSizer7->Add(ButtonImport, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	DeleteButton = new wxButton(this, ID_BUTTON4, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer7->Add(DeleteButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 5);
	FaceTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	CoroPanel = new wxPanel(FaceTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(0);
	Panel_SingleNode = new wxPanel(CoroPanel, ID_PANEL4, wxDefaultPosition, wxSize(36,0), wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer2->Add(Panel_SingleNode, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	FlexGridSizer8->AddGrowableRow(1);
	CustomColorSingleNode = new wxCheckBox(CoroPanel, ID_CHECKBOX1, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CustomColorSingleNode->SetValue(false);
	FlexGridSizer8->Add(CustomColorSingleNode, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SingleNodeGrid = new wxGrid(CoroPanel, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
	SingleNodeGrid->CreateGrid(13,2);
	SingleNodeGrid->SetMinSize(wxDLG_UNIT(CoroPanel,wxSize(-1,200)));
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
	SingleNodeGrid->SetDefaultCellFont( SingleNodeGrid->GetFont() );
	SingleNodeGrid->SetDefaultCellTextColour( SingleNodeGrid->GetForegroundColour() );
	FlexGridSizer8->Add(SingleNodeGrid, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 0);
	CoroPanel->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(CoroPanel);
	FlexGridSizer2->SetSizeHints(CoroPanel);
	NodeRangePanel = new wxPanel(FaceTypeChoice, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableRow(0);
	Panel_NodeRanges = new wxPanel(NodeRangePanel, ID_PANEL5, wxDefaultPosition, wxSize(36,0), wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer5->Add(Panel_NodeRanges, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer9->AddGrowableCol(0);
	FlexGridSizer9->AddGrowableRow(1);
	CustomColorNodeRanges = new wxCheckBox(NodeRangePanel, ID_CHECKBOX2, _("Force Custom Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CustomColorNodeRanges->SetValue(false);
	FlexGridSizer9->Add(CustomColorNodeRanges, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	NodeRangeGrid = new wxGrid(NodeRangePanel, ID_GRID3, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID3"));
	NodeRangeGrid->CreateGrid(13,2);
	NodeRangeGrid->SetMinSize(wxDLG_UNIT(NodeRangePanel,wxSize(-1,200)));
	NodeRangeGrid->EnableEditing(true);
	NodeRangeGrid->EnableGridLines(true);
	NodeRangeGrid->SetColLabelSize(20);
	NodeRangeGrid->SetRowLabelSize(150);
	NodeRangeGrid->SetDefaultColSize(200, true);
	NodeRangeGrid->SetColLabelValue(0, _("Nodes"));
	NodeRangeGrid->SetColLabelValue(1, _("Color"));
	NodeRangeGrid->SetRowLabelValue(0, _("Face Outline"));
	NodeRangeGrid->SetRowLabelValue(1, _("Mouth - AI"));
	NodeRangeGrid->SetRowLabelValue(2, _("Mouth - E"));
	NodeRangeGrid->SetRowLabelValue(3, _("Mouth - etc"));
	NodeRangeGrid->SetRowLabelValue(4, _("Mouth - FV"));
	NodeRangeGrid->SetRowLabelValue(5, _("Mouth - L"));
	NodeRangeGrid->SetRowLabelValue(6, _("Mouth - MBP"));
	NodeRangeGrid->SetRowLabelValue(7, _("Mouth - O"));
	NodeRangeGrid->SetRowLabelValue(8, _("Mouth - rest"));
	NodeRangeGrid->SetRowLabelValue(9, _("Mouth - U"));
	NodeRangeGrid->SetRowLabelValue(10, _("Mouth - WQ"));
	NodeRangeGrid->SetRowLabelValue(11, _("Eyes - Open"));
	NodeRangeGrid->SetRowLabelValue(12, _("Eyes - Closed"));
	NodeRangeGrid->SetDefaultCellFont( NodeRangeGrid->GetFont() );
	NodeRangeGrid->SetDefaultCellTextColour( NodeRangeGrid->GetForegroundColour() );
	FlexGridSizer9->Add(NodeRangeGrid, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 0);
	NodeRangePanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(NodeRangePanel);
	FlexGridSizer5->SetSizeHints(NodeRangePanel);
	Matrix = new wxPanel(FaceTypeChoice, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableRow(0);
	Panel_Matrix = new wxPanel(Matrix, ID_PANEL7, wxDefaultPosition, wxSize(36,0), wxTAB_TRAVERSAL, _T("ID_PANEL7"));
	FlexGridSizer3->Add(Panel_Matrix, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	FlexGridSizer10->AddGrowableRow(1);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText2 = new wxStaticText(Matrix, wxID_ANY, _("Image Placement:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MatrixImagePlacementChoice = new wxChoice(Matrix, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	MatrixImagePlacementChoice->SetSelection( MatrixImagePlacementChoice->Append(_("Centered")) );
	MatrixImagePlacementChoice->Append(_("Scaled"));
	FlexGridSizer6->Add(MatrixImagePlacementChoice, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DownloadImages = new wxButton(Matrix, ID_BUTTON1, _("Download Images"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer6->Add(Button_DownloadImages, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MatrixModelsGrid = new wxGrid(Matrix, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	MatrixModelsGrid->CreateGrid(10,2);
	MatrixModelsGrid->SetMinSize(wxDLG_UNIT(Matrix,wxSize(-1,200)));
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
	MatrixModelsGrid->SetDefaultCellFont( MatrixModelsGrid->GetFont() );
	MatrixModelsGrid->SetDefaultCellTextColour( MatrixModelsGrid->GetForegroundColour() );
	FlexGridSizer10->Add(MatrixModelsGrid, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3->Add(FlexGridSizer10, 1, wxALL|wxEXPAND, 0);
	Matrix->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Matrix);
	FlexGridSizer3->SetSizeHints(Matrix);
	FaceTypeChoice->AddPage(CoroPanel, _("Single Nodes"), false);
	FaceTypeChoice->AddPage(NodeRangePanel, _("Node Ranges"), false);
	FaceTypeChoice->AddPage(Matrix, _("Matrix"), false);
	FlexGridSizer4->Add(FaceTypeChoice, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer4->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	ModelPreviewPanelLocation = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	PreviewSizer = new wxFlexGridSizer(1, 1, 0, 0);
	PreviewSizer->AddGrowableCol(0);
	PreviewSizer->AddGrowableRow(0);
	ModelPreviewPanelLocation->SetSizer(PreviewSizer);
	PreviewSizer->Fit(ModelPreviewPanelLocation);
	PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);
	FlexGridSizer1->Add(ModelPreviewPanelLocation, 1, wxALL|wxEXPAND, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixNameChoiceSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonMatrixAddClicked);
	Connect(ID_BUTTON_IMPORT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonImportClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButtonMatrixDeleteClick);
	Panel_SingleNode->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ModelFaceDialog::Paint,0,this);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnCustomColorCheckboxClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelFaceDialog::OnSingleNodeGridCellLeftClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnSingleNodeGridCellLeftDClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnSingleNodeGridCellChange);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelFaceDialog::OnSingleNodeGridCellSelect);
	Panel_NodeRanges->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ModelFaceDialog::Paint,0,this);
	Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnCustomColorCheckboxClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellLeftClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellRightClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_LABEL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridLabelLeftDClick);
	Connect(ID_GRID3,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellChange);
	Connect(ID_GRID3,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelFaceDialog::OnNodeRangeGridCellSelect);
	Panel_Matrix->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ModelFaceDialog::Paint,0,this);
	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelFaceDialog::OnMatricImagePlacementChoiceSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelFaceDialog::OnButton_DownloadImagesClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellLeftClick1);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellChange);
	Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ModelFaceDialog::OnMatrixModelsGridCellSelect);
	Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&ModelFaceDialog::OnFaceTypeChoicePageChanged);
	//*)

    model = nullptr;

    modelPreview = new ModelPreview(ModelPreviewPanelLocation);
    modelPreview->SetMinSize(wxSize(150, 150));
    PreviewSizer->Add(modelPreview, 1, wxALL | wxEXPAND, 0);
    PreviewSizer->Fit(ModelPreviewPanelLocation);
    PreviewSizer->SetSizeHints(ModelPreviewPanelLocation);

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();
    SetEscapeId(wxID_CANCEL);
}

ModelFaceDialog::~ModelFaceDialog()
{
	//(*Destroy(ModelFaceDialog)
	//*)
}

void ModelFaceDialog::SetFaceInfo(Model *cls, std::map< std::string, std::map<std::string, std::string> > &finfo) {
    NodeRangeGrid->SetColSize(1, 50);
    SingleNodeGrid->SetColSize(1, 50);
    NameChoice->Clear();
    model = cls;
    modelPreview->SetModel(cls);

    for (std::map< std::string, std::map<std::string, std::string> >::iterator it = finfo.begin();
         it != finfo.end(); ++it) {

        std::string name = it->first;
        std::map<std::string, std::string> &info = it->second;

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

		for (std::map<std::string, std::string>::iterator it2 = info.begin(); it2 != info.end(); ++it2)
		{
			if (it2->first.substr(0, 5) == "Mouth" || it2->first.substr(0, 4) == "Eyes")
			{
				it2->second = FixFile("", it2->second);
			}
		}

        faceData[name] = info;
    }

    if (NameChoice->GetCount() > 0) {
        DeleteButton->Enable();
        FaceTypeChoice->Enable();
        NameChoice->SetSelection(0);
        SelectFaceModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
    } else {
        DeleteButton->Disable();
        FaceTypeChoice->Disable();
    }

    wxArrayString names;
    names.push_back("");
    for (size_t x = 0; x < cls->GetNodeCount(); x++) {
        wxString nn = cls->GetNodeName(x, true);
        names.push_back(nn);
    }

    NodesGridCellEditor *editor = new NodesGridCellEditor();
    editor->names = names;
    SingleNodeGrid->SetDefaultEditor(editor);
    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        SingleNodeGrid->SetReadOnly(x, 1);
    }

    wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    wxString filter("0123456789,-");
    wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    validator.SetCharIncludes(filter);
    reditor->SetValidator(validator);
    NodeRangeGrid->SetDefaultEditor(reditor);
    for (int x = 0; x < NodeRangeGrid->GetNumberRows(); x++) {
        NodeRangeGrid->SetReadOnly(x, 1);
    }
}

void ModelFaceDialog::GetFaceInfo(std::map< std::string, std::map<std::string, std::string> > &finfo) {
    if (SingleNodeGrid->IsCellEditControlShown()) {
        SingleNodeGrid->SaveEditControlValue();
        SingleNodeGrid->HideCellEditControl();
    }
    if (NodeRangeGrid->IsCellEditControlShown()) {
        NodeRangeGrid->SaveEditControlValue();
        NodeRangeGrid->HideCellEditControl();
    }
    finfo.clear();
    for (std::map<std::string, std::map<std::string, std::string> >::iterator it = faceData.begin();
         it != faceData.end(); ++it) {
        if (!it->second.empty()) {
            finfo[it->first] = it->second;
        }
    }
}

static bool SetGrid(wxGrid *grid, std::map<std::string, std::string> &info) {
    bool customColor = false;
    if (info["CustomColors"] == "1") {
        grid->ShowCol(1);
        customColor = true;
    } else {
        grid->HideCol(1);
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
        grid->SetCellBackgroundColour(x, 1, color.asWxColor());
    }
    return customColor;
}

void ModelFaceDialog::SelectFaceModel(const std::string &name) {
    FaceTypeChoice->Enable();
    wxString type = faceData[name]["Type"];
    if (type == "") {
        type = "SingleNode";
        faceData[name]["Type"] = type;
    }
    if (type == "SingleNode") {
        FaceTypeChoice->ChangeSelection(SINGLE_NODE_FACE);
        std::map<std::string, std::string> &info = faceData[name];
        CustomColorSingleNode->SetValue(SetGrid(SingleNodeGrid, info));
    } else if (type == "NodeRange") {
        FaceTypeChoice->ChangeSelection(NODE_RANGE_FACE);
        std::map<std::string, std::string> &info = faceData[name];
        CustomColorNodeRanges->SetValue(SetGrid(NodeRangeGrid, info));
    } else {
        FaceTypeChoice->ChangeSelection(MATRIX_FACE);
        for (int r = 0; r < MatrixModelsGrid->GetNumberRows(); r++) {
            for (int c = 0; c < MatrixModelsGrid->GetNumberCols(); c++) {
                wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
                key.Replace(" ", "");
                MatrixModelsGrid->SetCellValue(r, c, faceData[name][key.ToStdString()]);
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


void ModelFaceDialog::OnMatrixNameChoiceSelect(wxCommandEvent& event)
{
    SelectFaceModel(NameChoice->GetString(NameChoice->GetSelection()).ToStdString());
}

void ModelFaceDialog::OnButtonMatrixAddClicked(wxCommandEvent& event)
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
            DeleteButton->Enable();
        }
    }
}
void ModelFaceDialog::OnButtonMatrixDeleteClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int i = wxMessageBox("Delete face definition?", "Are you sure you want to delete " + name + "?",
                         wxICON_WARNING | wxOK , this);
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
            DeleteButton->Disable();
        }
    }
}

void ModelFaceDialog::OnMatrixModelsGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    key.Replace(" ", "");
    faceData[name][key.ToStdString()] = MatrixModelsGrid->GetCellValue(r, c);
}

static const wxString strSupportedImageTypes = "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg|All files (*.*)|*.*";
void ModelFaceDialog::OnMatrixModelsGridCellLeftClick(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    int r = event.GetRow();
    int c = event.GetCol();
    wxString key = MatrixModelsGrid->GetRowLabelValue(r) + "-" + MatrixModelsGrid->GetColLabelValue(c);
    wxFileName fname(MatrixModelsGrid->GetCellValue(r, c));
    wxFileDialog dlg(this, "Choose Image File for " + key, fname.GetPath(),
                     wxEmptyString,
                     strSupportedImageTypes,
                     wxFD_OPEN | wxFD_CHANGE_DIR);
    if (dlg.ShowModal() == wxID_OK) {
        wxString new_filename = dlg.GetPath();
        key.Replace(" ", "");
        faceData[name][key.ToStdString()] = new_filename;
        MatrixModelsGrid->SetCellValue(r, c, new_filename);

        TryToSetAllMatrixModels(name, key.ToStdString(), new_filename.ToStdString(), r, c);
    }
}

std::string GenerateKey(int col, std::string key)
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

std::string ExtractKey(wxString key)
{
    return key.AfterFirst('-').BeforeFirst('-').ToStdString();
}

// replace the count'th occurrence of key with "etc" and return the position
wxFileName GetFileNamePhoneme(wxFileName fn, std::string key, int count, std::string phoneme)
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

void ModelFaceDialog::DoSetPhonemes(wxFileName fn, std::string actualkey, std::string key, int count, int row, int col, std::string name, std::list<std::string> phonemes, std::string setPhoneme)
{
    if (key == setPhoneme) return;

    for (auto it = phonemes.begin(); it != phonemes.end(); ++it)
    {
        wxFileName fn2 = GetFileNamePhoneme(fn, actualkey, count, *it);
        if (fn2.Exists() && (faceData[name][GenerateKey(col, setPhoneme)] == "" || !wxFileName::FileExists(faceData[name][GenerateKey(col, setPhoneme)])))
        {
            faceData[name][GenerateKey(col, setPhoneme)] = fn2.GetFullPath();
            MatrixModelsGrid->SetCellValue(row, col, fn2.GetFullPath());
        }
    }
}

std::list<std::string> GetPhonemes(std::string key)
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
        std::list<std::string> phonemes = { "etc", "ETC", "Etc", "ect", "ECT" , "Ect" };
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
        std::list<std::string> phonemes = { };
        return phonemes;
    }
}

void ModelFaceDialog::DoSetMatrixModels(wxFileName fn, std::string actualkey, std::string key, int count, int col, std::string name)
{
    int i = 0;
    for (auto it = _phonemes.begin(); it != _phonemes.end(); ++it)
    {
        DoSetPhonemes(fn, actualkey, key, count, i++, col, name, GetPhonemes(*it), *it);
    }
}

void ModelFaceDialog::TryToSetAllMatrixModels(std::string name, std::string key, std::string new_filename, int row, int col)
{
    wxFileName fn = wxFileName(new_filename);

    std::string k = ExtractKey(key);

    auto phonemes = GetPhonemes(k);

    bool done = false;
    // try each of the possible variants in the phoneme
    for (auto it = phonemes.begin(); !done && it != phonemes.end(); ++it)
    {
        int replacecount = fn.GetName().Replace(*it, "etc", true);

        // because some file systems are case sensitive try some common variants
        for (int i = 0; i < replacecount; i++)
        {
            //get list of etc Phonemes
            const std::list<std::string> findList = GetPhonemes("etc");
            //loop through and find the "etc" file
            for(const auto &phen :findList)
            {
                const wxFileName fn2 = GetFileNamePhoneme(fn, *it, i, phen);
                if (fn2.Exists())
                {
                    DoSetMatrixModels(fn, *it, k, i, col, name);
                    done = true;
                    break;
                }
            }
        }
    }
}

bool ModelFaceDialog::IsValidPhoneme(const std::string phoneme) const
{
    for (auto it = _phonemes.begin(); it != _phonemes.end(); ++it)
    {
        if (wxString(*it).Lower() == wxString(phoneme).Lower()) return true;
    }

    return false;
}

int ModelFaceDialog::GetRowForPhoneme(const std::string phoneme) const
{
    int row = 0;

    for (auto it = _phonemes.begin(); it != _phonemes.end(); ++it)
    {
        if (wxString(*it).Lower() == wxString(phoneme).Lower()) return row;
        row++;
    }

    return -1;
}

void ModelFaceDialog::OnMatricImagePlacementChoiceSelect(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    faceData[name]["ImagePlacement"] = MatrixImagePlacementChoice->GetStringSelection();
}

void ModelFaceDialog::OnCustomColorCheckboxClick(wxCommandEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE) {
        if (CustomColorSingleNode->IsChecked()) {
            SingleNodeGrid->ShowCol(1);
            faceData[name]["CustomColors"] = "1";
        } else {
            SingleNodeGrid->HideCol(1);
            faceData[name]["CustomColors"] = "0";
        }
    } else {
        if (CustomColorNodeRanges->IsChecked()) {
            NodeRangeGrid->ShowCol(1);
            faceData[name]["CustomColors"] = "1";
        } else {
            NodeRangeGrid->HideCol(1);
            faceData[name]["CustomColors"] = "0";
        }
    }
}

void ModelFaceDialog::GetValue(wxGrid *grid, const int row, const int col, std::map<std::string, std::string> &info) {
    wxString key = grid->GetRowLabelValue(row).ToStdString();
    key.Replace(" ", "");
    if (col == 1) {
        key += "-Color";
        xlColor color = grid->GetCellBackgroundColour(row, col);
        info[key.ToStdString()] = color;
    } else {
        info[key.ToStdString()] = grid->GetCellValue(row, col);
    }
    UpdatePreview(grid->GetCellValue(row, CHANNEL_COL).ToStdString(), grid->GetCellBackgroundColour(row, COLOR_COL));
}

void ModelFaceDialog::UpdatePreview(const std::string& channels, wxColor c)
{
    int nn = model->GetNodeCount();
    xlColor cb(xlDARK_GREY);
    if (model->modelDimmingCurve) {
        model->modelDimmingCurve->apply(cb);
    }
    for (int node = 0; node < nn; node++) {
        model->SetNodeColor(node, cb);
    }

    // now highlight selected
    if (channels != "")
    {
        if (FaceTypeChoice->GetSelection() == SINGLE_NODE_FACE) {
            wxStringTokenizer wtkz(channels, ",");
            while (wtkz.HasMoreTokens())
            {
                wxString valstr = wtkz.GetNextToken();
                for (size_t n = 0; n < model->GetNodeCount(); n++) {
                    wxString ns = model->GetNodeName(n, true);
                    if (ns == valstr) {
                        model->SetNodeColor(n, c);
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
                    model->SetNodeColor(it, c);
                }
            }
        }
    }
    model->DisplayEffectOnWindow(modelPreview, 2);
}

void ModelFaceDialog::OnNodeRangeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), faceData[name]);
}

void ModelFaceDialog::OnSingleNodeGridCellChange(wxGridEvent& event)
{
    std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), faceData[name]);
}

void ModelFaceDialog::OnFaceTypeChoicePageChanged(wxChoicebookEvent& event)
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

void ModelFaceDialog::OnNodeRangeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == CHANNEL_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        const wxString title = name + " - " + NodeRangeGrid->GetRowLabelValue(event.GetRow());
        NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), this);

        if (dialog.ShowModal() == wxID_OK)
        {
            NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), faceData[name]);
            dialog.Close();
        }
    }
    else if (event.GetCol() == COLOR_COL) {
        const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK) {
            _colorData = dlg.GetColourData();
            NodeRangeGrid->SetCellBackgroundColour(event.GetRow(), COLOR_COL, dlg.GetColourData().GetColour());
            NodeRangeGrid->Refresh();
            GetValue(NodeRangeGrid, event.GetRow(), event.GetCol(), faceData[name]);
        }
    }
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
}

void ModelFaceDialog::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() == COLOR_COL) {
        std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
        wxColor c = SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL);
        _colorData.SetColour(c);
        wxColourDialog dlg(this, &_colorData);
        if (dlg.ShowModal() == wxID_OK) {
            _colorData = dlg.GetColourData();
            SingleNodeGrid->SetCellBackgroundColour(event.GetRow(), COLOR_COL, dlg.GetColourData().GetColour());
            SingleNodeGrid->Refresh();
            GetValue(SingleNodeGrid, event.GetRow(), event.GetCol(), faceData[name]);
        }
    }
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
}

void ModelFaceDialog::OnMatrixModelsGridCellSelect(wxGridEvent& event)
{
    UpdatePreview("", *wxWHITE);
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFaceDialog::OnMatrixModelsGridCellLeftClick1(wxGridEvent& event)
{
    UpdatePreview("", *wxWHITE);
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFaceDialog::OnSingleNodeGridCellSelect(wxGridEvent& event)
{
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFaceDialog::OnSingleNodeGridCellLeftClick(wxGridEvent& event)
{
    UpdatePreview(SingleNodeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), SingleNodeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFaceDialog::OnNodeRangeGridCellLeftClick(wxGridEvent& event)
{
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFaceDialog::OnNodeRangeGridCellSelect(wxGridEvent& event)
{
    UpdatePreview(NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL).ToStdString(), NodeRangeGrid->GetCellBackgroundColour(event.GetRow(), COLOR_COL));
    event.ResumePropagation(1);
    event.Skip();
}

void ModelFaceDialog::PaintFace(wxDC& dc, int x, int y, const char* xpm[])
{
    wxImage i(xpm);
    wxBitmap bmp(i);

    dc.DrawBitmap(bmp, x, y);
}

void ModelFaceDialog::Paint(wxPaintEvent& event)
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

std::string FixPhonemeCase(const std::string p)
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

void ModelFaceDialog::OnButton_DownloadImagesClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    MatrixFaceDownloadDialog dlg(this);
    if (dlg.DlgInit(model->GetDefaultBufferWi(), model->GetDefaultBufferHt()))
    {
        if (dlg.ShowModal() == wxID_OK)
        {
            std::string faceZip = dlg.GetFaceFile();

            // create folder if necessary
            std::string dir = xLightsFrame::CurrentDir.ToStdString() + "/DownloadedFaces";
            if (!wxDir::Exists(dir))
            {
                wxMkDir(dir, wxS_DIR_DEFAULT);
            }

            std::list<std::string> files;

            // extract all the files in the zip file into that directory
            wxFileInputStream fin(faceZip);
            wxZipInputStream zin(fin);
            wxZipEntry *ent = zin.GetNextEntry();
            while (ent != nullptr)
            {
                std::string filename = dir + "/" + ent->GetName().ToStdString();
                files.push_back(filename);

                if (!wxFile::Exists(filename))
                {
                    wxFileOutputStream fout(filename);
                    zin.Read(fout);
                }
                ent = zin.GetNextEntry();
            }

            std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();

            bool error = false;
            for (auto it = files.begin(); it != files.end(); ++it)
            {
                wxFileName fn(*it);
                wxString basefn = fn.GetName().Lower();
                bool eyesclosed = false;
                if (basefn.EndsWith("_eo"))
                {
                    basefn = basefn.SubString(0, basefn.Length() - 4);
                    eyesclosed = false;
                }
                else if (basefn.EndsWith("_ec"))
                {
                    basefn = basefn.SubString(0, basefn.Length() - 4);
                    eyesclosed = true;
                }

                std::string phoneme = FixPhonemeCase(basefn.AfterLast('_').ToStdString());

                if (phoneme == "" || !IsValidPhoneme(phoneme))
                {
                    logger_base.warn("Phoneme '%s' was not known. File %s ignored.", (const char *)phoneme.c_str(), (const char *)it->c_str());
                    error = true;
                }
                else
                {
                    std::string key = "Mouth-" + phoneme + "-" + (eyesclosed ? "EyesClosed" : "EyesOpen");
                    faceData[name][key] = *it;
                    MatrixModelsGrid->SetCellValue(GetRowForPhoneme(phoneme), (eyesclosed ? 1 : 0), *it);
                }
            }

            if (error)
            {
                DisplayError("One or more images could not be mapped to the model due to issues with the names of files within the zip file. See log for details.");
            }
        }
    }
}

void ModelFaceDialog::OnNodeRangeGridCellRightClick(wxGridEvent& event)
{
    wxMenu mnu;

    mnu.Append(FACES_DIALOG_IMPORT_SUB, "Import SubModel");

    mnu.Bind(wxEVT_COMMAND_MENU_SELECTED, [gridevent = event, this](wxCommandEvent & rightClkEvent) mutable {
        OnGridPopup(rightClkEvent.GetId(), gridevent);
    });
    PopupMenu(&mnu);

    event.Skip();
}

void ModelFaceDialog::OnNodeRangeGridLabelLeftDClick(wxGridEvent& event)
{
    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    const wxString title = name + " - " + NodeRangeGrid->GetRowLabelValue(event.GetRow());
    NodeSelectGrid dialog(true, title, model, NodeRangeGrid->GetCellValue(event.GetRow(), CHANNEL_COL), this);

    if (dialog.ShowModal() == wxID_OK)
    {
        NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, dialog.GetNodeList());
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, faceData[name]);
        dialog.Close();
    }
}


void ModelFaceDialog::OnGridPopup(const int rightEventID, wxGridEvent& gridEvent)
{
    if (rightEventID == FACES_DIALOG_IMPORT_SUB)
    {
        ImportSubmodel(gridEvent);
    }
}

void ModelFaceDialog::ImportSubmodel(wxGridEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxArrayString choices;
    for (Model* sm : model->GetSubModels())
    {
        choices.Add(sm->Name());
    }

    // don't offer a choice if there is nothing to choose
    if (choices.GetCount() == 0) {
        wxMessageBox("No submodels found.");
        return;
    }

    const std::string name = NameChoice->GetString(NameChoice->GetSelection()).ToStdString();
    wxSingleChoiceDialog dlg(GetParent(), "", "Select SubModel", choices);

    if (dlg.ShowModal() == wxID_OK)
    {
        Model* sm = model->GetSubModel(dlg.GetStringSelection());
        if (sm == nullptr)             {
            logger_base.error("Strange ... ModelFaceDialog::ImportSubmodel returned no model for %s but it was in the list we gave the user.", (const char*)dlg.GetStringSelection().c_str());
            dlg.Close();
            return;
        }
        const auto nodes = getSubmodelNodes(sm);
        NodeRangeGrid->SetCellValue(event.GetRow(), CHANNEL_COL, nodes);
        NodeRangeGrid->Refresh();
        GetValue(NodeRangeGrid, event.GetRow(), CHANNEL_COL, faceData[name]);
        dlg.Close();
    }
}

wxString ModelFaceDialog::getSubmodelNodes(Model* sm)
{
    if (sm == nullptr) return "";

    wxXmlNode* root = sm->GetModelXml();
    wxString row = "";

    if (root->GetName() == "subModel") {
        if (root->GetAttribute("type", "") == "ranges") {
            wxArrayString rows;
            int line = 0;
            while (root->HasAttribute(wxString::Format("line%d", line))) {
                auto l = root->GetAttribute(wxString::Format("line%d", line), "");
                rows.Add(l);
                line++;
            }
            row = wxJoin(rows, ',', '\0');
        }
    }

    return row;
}

void ModelFaceDialog::OnAddBtnPopup(wxCommandEvent& event)
{
    if (event.GetId() == FACES_DIALOG_IMPORT_MODEL)
    {
        ImportFacesFromModel();
    }
    else if (event.GetId() == FACES_DIALOG_IMPORT_FILE)
    {
        const wxString filename = wxFileSelector(_("Choose Model file"), wxEmptyString, wxEmptyString, wxEmptyString, "xmodel files (*.xmodel)|*.xmodel", wxFD_OPEN);
        if (filename.IsEmpty()) return;

        ImportFaces(filename);
    }
    else if (event.GetId() == FACES_DIALOG_COPY)
    {
        CopyFaceData();
    }
    else if (event.GetId() == FACES_DIALOG_RENAME)
    {
        RenameFace();
    }
}

void ModelFaceDialog::ImportFacesFromModel()
{
    xLightsFrame* xlights = xLightsApp::GetFrame();

    const wxArrayString choices = getModelList(&xlights->AllModels);

    wxSingleChoiceDialog dlg(GetParent(), "", "Select Model", choices);

    if (dlg.ShowModal() == wxID_OK)
    {
        Model* m = xlights->GetModel(dlg.GetStringSelection());
        if (m->faceInfo.size() == 0)
        {
            wxMessageBox(dlg.GetStringSelection() + " contains no signing faces, skipping");
            return;
        }

        AddFaces(m->faceInfo);

        NameChoice->Enable();
        FaceTypeChoice->Enable();
        DeleteButton->Enable();

        NameChoice->SetSelection(NameChoice->GetCount() - 1);
        NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
        SelectFaceModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());

    }
}

void ModelFaceDialog::ImportFaces(const wxString& filename) 
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();
        bool facesFound = false;
        if (root->GetName() == "custommodel")
        {
            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "faceInfo")
                {
                    std::map<std::string, std::map<std::string, std::string> > faceInfo;
                    Model::ParseFaceInfo(n, faceInfo);
                    if (faceInfo.size() == 0)
                    {
                        continue;
                    }
                    facesFound = true;
                    AddFaces(faceInfo);
                }
            }
        }
        if (facesFound)
        {
            NameChoice->Enable();
            FaceTypeChoice->Enable();
            DeleteButton->Enable();

            NameChoice->SetSelection(NameChoice->GetCount()-1);
            NameChoice->SetStringSelection(NameChoice->GetString(NameChoice->GetCount() - 1));
            SelectFaceModel(NameChoice->GetString(NameChoice->GetCount() - 1).ToStdString());
        }
        else
        {
            DisplayError(filename + " contains no faces.");
        }
    }
    else
    {
        DisplayError(filename + " Failure loading xModel file.");
    }
}

void ModelFaceDialog::AddFaces(std::map<std::string, std::map<std::string, std::string> > faces)
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

void ModelFaceDialog::OnButtonImportClick(wxCommandEvent& event)
{
    wxMenu mnu;
    if (DeleteButton->IsEnabled())
    {
        mnu.Append(FACES_DIALOG_COPY, "Copy");
        mnu.Append(FACES_DIALOG_RENAME, "Rename");
        mnu.AppendSeparator();
    }
    mnu.Append(FACES_DIALOG_IMPORT_MODEL, "Import From Model");
    mnu.Append(FACES_DIALOG_IMPORT_FILE, "Import From File");

    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& ModelFaceDialog::OnAddBtnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void ModelFaceDialog::CopyFaceData()
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

void ModelFaceDialog::RenameFace()
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

            auto const face = std::move(faceData[currentName]);
            faceData[n] = std::move(face);
            faceData.erase(currentName);

            NameChoice->SetSelection(index);
            NameChoice->SetStringSelection(NameChoice->GetString(index));
            SelectFaceModel(NameChoice->GetString(index).ToStdString());
        } else {
            DisplayError("'" + n + "' face definition already exists.");
        }
    }
}

wxArrayString ModelFaceDialog::getModelList(ModelManager* modelManager)
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

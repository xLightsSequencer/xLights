/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "StrandNodeNamesDialog.h"

//(*InternalHeaders(StrandNodeNamesDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <sstream>
#include "models/Model.h"

//(*IdInit(StrandNodeNamesDialog)
const long StrandNodeNamesDialog::ID_GRID2 = wxNewId();
const long StrandNodeNamesDialog::ID_GRID1 = wxNewId();
const long StrandNodeNamesDialog::ID_BUTTONOK = wxNewId();
const long StrandNodeNamesDialog::ID_BUTTONCANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(StrandNodeNamesDialog,wxDialog)
	//(*EventTable(StrandNodeNamesDialog)
	//*)
END_EVENT_TABLE()

StrandNodeNamesDialog::StrandNodeNamesDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(StrandNodeNamesDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableCol(1);
	FlexGridSizer2->AddGrowableRow(1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Strand Names"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Node Names"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StrandsGrid = new wxGrid(this, ID_GRID2, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID2"));
	StrandsGrid->CreateGrid(10,1);
	StrandsGrid->EnableEditing(true);
	StrandsGrid->EnableGridLines(true);
	StrandsGrid->SetDefaultCellFont( StrandsGrid->GetFont() );
	StrandsGrid->SetDefaultCellTextColour( StrandsGrid->GetForegroundColour() );
	FlexGridSizer2->Add(StrandsGrid, 1, wxALL|wxEXPAND, 5);
	NodesGrid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	NodesGrid->CreateGrid(10,1);
	NodesGrid->EnableEditing(true);
	NodesGrid->EnableGridLines(true);
	NodesGrid->SetDefaultCellFont( NodesGrid->GetFont() );
	NodesGrid->SetDefaultCellTextColour( NodesGrid->GetForegroundColour() );
	FlexGridSizer2->Add(NodesGrid, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTONOK, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONOK"));
	ButtonOk->SetDefault();
	FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTONCANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONCANCEL"));
	FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_GRID2,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&StrandNodeNamesDialog::OnStrandsGridCellChanged);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&StrandNodeNamesDialog::OnNodesGridCellChanged);
	Connect(ID_BUTTONOK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrandNodeNamesDialog::OnButtonOkClick);
	Connect(ID_BUTTONCANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&StrandNodeNamesDialog::OnButtonCancelClick);
	//*)

    SetEscapeId(ID_BUTTONCANCEL);

    ValidateWindow();
}

StrandNodeNamesDialog::~StrandNodeNamesDialog()
{
	//(*Destroy(StrandNodeNamesDialog)
	//*)
}

void StrandNodeNamesDialog::Setup(const Model *md, const std::string &nodeNames, const std::string &strandNames) {
    std::vector<wxString> strands;
    std::vector<wxString> nodes;
    wxString tempstr = strandNames;
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        strands.push_back(t2);
    }
    if (strands.size() < md->GetNumStrands()) {
        strands.resize(md->GetNumStrands());
    }
    StrandsGrid->BeginBatch();
    StrandsGrid->SetMaxSize(StrandsGrid->GetSize());
    StrandsGrid->HideColLabels();
    StrandsGrid->DeleteRows(0, 10);
    StrandsGrid->AppendRows(strands.size());
    StrandsGrid->SetRowLabelSize(40);

    for (int x = 0; x < strands.size(); x++) {
        StrandsGrid->SetCellValue(x, 0, strands[x]);
    }
    StrandsGrid->EndBatch();

    tempstr=nodeNames;
    nodes.clear();
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr[0] == ',') {
            t2 = "";
            tempstr = tempstr(1, tempstr.length());
        } else if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(",") - 1);
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        } else {
            tempstr = "";
        }
        nodes.push_back(t2);
    }
    if (nodes.size() < md->GetNodeCount()) {
        nodes.resize(md->GetNodeCount());
    }
    NodesGrid->BeginBatch();
    NodesGrid->SetMaxSize(StrandsGrid->GetSize());
    NodesGrid->HideColLabels();
    NodesGrid->DeleteRows(0, 10);
    NodesGrid->SetRowLabelSize(40);

    NodesGrid->AppendRows(nodes.size());
    for (int x = 0; x < nodes.size(); x++) {
        NodesGrid->SetCellValue(x, 0, nodes[x]);
    }
    NodesGrid->EndBatch();

    ValidateWindow();
}

std::string StrandNodeNamesDialog::GetNodeNames() {
    std::string nodeNames;
    for (int x = NodesGrid->GetNumberRows(); x > 0; x--) {
        if ("" != NodesGrid->GetCellValue(x-1,0) || nodeNames.size() > 0) {
            if (nodeNames.size() > 0) {
                nodeNames = "," + nodeNames;
            }
            nodeNames = NodesGrid->GetCellValue(x-1,0) + nodeNames;
        }
    }
    return nodeNames;
}

std::string StrandNodeNamesDialog::GetStrandNames() {
    std::string strandNames;
    for (int x = StrandsGrid->GetNumberRows(); x > 0; x--) {
        if ("" != StrandsGrid->GetCellValue(x-1,0) || strandNames.size() > 0) {
            if (strandNames.size() > 0) {
                strandNames = "," + strandNames;
            }
            strandNames = StrandsGrid->GetCellValue(x-1,0) + strandNames;
        }
    }
    return strandNames;
}

void StrandNodeNamesDialog::ValidateWindow()
{
    bool valid = true;

    std::vector<std::string> st;

    for (int i = 0; i < StrandsGrid->GetNumberRows(); i++) {
        if ("" != StrandsGrid->GetCellValue(i, 0)) {
            st.push_back(StrandsGrid->GetCellValue(i, 0));
        }
    }
    std::sort(begin(st), end(st));
    if (std::unique(begin(st), end(st)) != end(st))
    {
        valid = false;
    }

    if (valid)
    {
        std::vector<std::string> no;
        for (int i = 0; i < NodesGrid->GetNumberRows(); i++) {
            if ("" != NodesGrid->GetCellValue(i, 0)) {
                no.push_back(NodesGrid->GetCellValue(i, 0));
            }
        }

        std::sort(begin(no), end(no));
        if (std::unique(begin(no), end(no)) != end(no))
        {
            valid = false;
        }
    }

    if (valid)
    {
        ButtonOk->Enable();
    }
    else
    {
        ButtonOk->Enable(false);
    }
}
void StrandNodeNamesDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void StrandNodeNamesDialog::OnButtonOkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void StrandNodeNamesDialog::OnStrandsGridCellChanged(wxGridEvent& event)
{
    ValidateWindow();
}

void StrandNodeNamesDialog::OnNodesGridCellChanged(wxGridEvent& event)
{
    ValidateWindow();
}

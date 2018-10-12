#include "StrandNodeNamesDialog.h"

//(*InternalHeaders(StrandNodeNamesDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <sstream>
#include "models/Model.h"

//(*IdInit(StrandNodeNamesDialog)
const long StrandNodeNamesDialog::ID_GRID2 = wxNewId();
const long StrandNodeNamesDialog::ID_GRID1 = wxNewId();
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
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

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
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

    SetEscapeId(wxID_CANCEL);
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


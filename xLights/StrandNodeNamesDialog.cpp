#include "StrandNodeNamesDialog.h"

//(*InternalHeaders(StrandNodeNamesDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

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
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("id"));
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
	FlexGridSizer2->Add(StrandsGrid, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	NodesGrid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	NodesGrid->CreateGrid(10,1);
	NodesGrid->EnableEditing(true);
	NodesGrid->EnableGridLines(true);
	NodesGrid->SetDefaultCellFont( NodesGrid->GetFont() );
	NodesGrid->SetDefaultCellTextColour( NodesGrid->GetForegroundColour() );
	FlexGridSizer2->Add(NodesGrid, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

StrandNodeNamesDialog::~StrandNodeNamesDialog()
{
	//(*Destroy(StrandNodeNamesDialog)
	//*)
}


#include "LMSImportChannelMapDialog.h"

//(*InternalHeaders(LMSImportChannelMapDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(LMSImportChannelMapDialog)
const long LMSImportChannelMapDialog::ID_CHECKBOX1 = wxNewId();
const long LMSImportChannelMapDialog::ID_GRID1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(LMSImportChannelMapDialog,wxDialog)
	//(*EventTable(LMSImportChannelMapDialog)
	//*)
END_EVENT_TABLE()


LMSImportChannelMapDialog::LMSImportChannelMapDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(LMSImportChannelMapDialog)
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Map Channels"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetMaxSize(wxDLG_UNIT(parent,wxSize(-1,500)));
	Sizer = new wxFlexGridSizer(0, 1, 0, 0);
	Sizer->AddGrowableCol(0);
	Sizer->AddGrowableRow(1);
	MapByStrand = new wxCheckBox(this, ID_CHECKBOX1, _("Map by Strand/CCR"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	MapByStrand->SetValue(false);
	Sizer->Add(MapByStrand, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ChannelMapGrid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID1"));
	ChannelMapGrid->CreateGrid(10,2);
	ChannelMapGrid->SetMaxSize(wxDLG_UNIT(this,wxSize(-1,440)));
	ChannelMapGrid->EnableEditing(true);
	ChannelMapGrid->EnableGridLines(true);
	ChannelMapGrid->SetColLabelValue(0, _("Model Node"));
	ChannelMapGrid->SetColLabelValue(1, _("LMS Channel"));
	ChannelMapGrid->SetDefaultCellFont( ChannelMapGrid->GetFont() );
	ChannelMapGrid->SetDefaultCellTextColour( ChannelMapGrid->GetForegroundColour() );
	Sizer->Add(ChannelMapGrid, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	Sizer->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(Sizer);
	Sizer->Fit(this);
	Sizer->SetSizeHints(this);

	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&LMSImportChannelMapDialog::OnMapByStrandClick);
	//*)
}

LMSImportChannelMapDialog::~LMSImportChannelMapDialog()
{
	//(*Destroy(LMSImportChannelMapDialog)
	//*)
}
void LMSImportChannelMapDialog::SetNames(const wxArrayString *cn,
                                         const wxArrayString *sn,
                                         const wxArrayString *nn,
                                         const wxArrayString *ccrn) {
    channelNames = cn;
    strandNames = sn;
    nodeNames = nn;
    ccrNames = ccrn;
    SetupByNode();
}
void LMSImportChannelMapDialog::SetupByNode() {
    MapByStrand->SetValue(false);
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(*channelNames);
    ChannelMapGrid->SetDefaultEditor(editor);
    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    ChannelMapGrid->InsertRows(0, nodeNames->size() + 1);
    for (int x = 0; x < nodeNames->size(); x++) {
        ChannelMapGrid->SetCellValue(x,0,(*nodeNames)[x]);
        ChannelMapGrid->SetReadOnly(x, 0);
    }
    ChannelMapGrid->SetReadOnly(nodeNames->size(), 0);
    ChannelMapGrid->SetReadOnly(nodeNames->size(), 1);
    ChannelMapGrid->SetRowLabelSize(0);
    ChannelMapGrid->AutoSizeColumns();
    ChannelMapGrid->SetColSize(1, ChannelMapGrid->GetColSize(1) * 2);
    Sizer->Layout();
    Layout();
    Fit();
}
void LMSImportChannelMapDialog::SetupByStrand() {
    MapByStrand->SetValue(true);
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor(*ccrNames);
    ChannelMapGrid->SetDefaultEditor(editor);
    ChannelMapGrid->DeleteRows(0, ChannelMapGrid->GetNumberRows());
    ChannelMapGrid->InsertRows(0, strandNames->size() + 1);
    for (int x = 0; x < strandNames->size(); x++) {
        ChannelMapGrid->SetCellValue(x,0,(*strandNames)[x]);
        ChannelMapGrid->SetReadOnly(x, 0);
    }
    ChannelMapGrid->SetReadOnly(strandNames->size(), 0);
    ChannelMapGrid->SetReadOnly(strandNames->size(), 1);
    ChannelMapGrid->SetRowLabelSize(0);
    ChannelMapGrid->AutoSizeColumns();
    ChannelMapGrid->SetColSize(1, ChannelMapGrid->GetColSize(1) * 2);
    Sizer->Layout();
    Layout();
    Fit();
}



void LMSImportChannelMapDialog::OnMapByStrandClick(wxCommandEvent& event)
{
    if (MapByStrand->GetValue()) {
        SetupByStrand();
    } else {
        SetupByNode();
    }
}

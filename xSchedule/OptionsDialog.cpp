#include "OptionsDialog.h"

//(*InternalHeaders(OptionsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(OptionsDialog)
const long OptionsDialog::ID_CHECKBOX1 = wxNewId();
const long OptionsDialog::ID_CHECKBOX3 = wxNewId();
const long OptionsDialog::ID_CHECKBOX2 = wxNewId();
const long OptionsDialog::ID_STATICTEXT1 = wxNewId();
const long OptionsDialog::ID_GRID1 = wxNewId();
const long OptionsDialog::ID_BUTTON4 = wxNewId();
const long OptionsDialog::ID_BUTTON8 = wxNewId();
const long OptionsDialog::ID_BUTTON3 = wxNewId();
const long OptionsDialog::ID_STATICTEXT2 = wxNewId();
const long OptionsDialog::ID_GRID2 = wxNewId();
const long OptionsDialog::ID_BUTTON5 = wxNewId();
const long OptionsDialog::ID_BUTTON6 = wxNewId();
const long OptionsDialog::ID_BUTTON7 = wxNewId();
const long OptionsDialog::ID_BUTTON1 = wxNewId();
const long OptionsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(OptionsDialog,wxDialog)
	//(*EventTable(OptionsDialog)
	//*)
END_EVENT_TABLE()

OptionsDialog::OptionsDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(OptionsDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(4);
	CheckBox_RunOnMachineStartup = new wxCheckBox(this, ID_CHECKBOX1, _("Run on machine startup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_RunOnMachineStartup->SetValue(false);
	FlexGridSizer1->Add(CheckBox_RunOnMachineStartup, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SendOffWhenNotRunning = new wxCheckBox(this, ID_CHECKBOX3, _("Send data when not running sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_SendOffWhenNotRunning->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SendOffWhenNotRunning, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Sync = new wxCheckBox(this, ID_CHECKBOX2, _("Use ArtNet/E1.31 Synchronisation Protocols"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_Sync->SetValue(false);
	FlexGridSizer1->Add(CheckBox_Sync, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	FlexGridSizer3->AddGrowableRow(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Projectors:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	Grid_Projectors = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	Grid_Projectors->CreateGrid(0,2);
	Grid_Projectors->EnableEditing(true);
	Grid_Projectors->EnableGridLines(true);
	Grid_Projectors->SetDefaultColSize(100, true);
	Grid_Projectors->SetColLabelValue(0, _("IP Address"));
	Grid_Projectors->SetColLabelValue(1, _("Password"));
	Grid_Projectors->SetDefaultCellFont( Grid_Projectors->GetFont() );
	Grid_Projectors->SetDefaultCellTextColour( Grid_Projectors->GetForegroundColour() );
	FlexGridSizer3->Add(Grid_Projectors, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_AddProjector = new wxButton(this, ID_BUTTON4, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer4->Add(Button_AddProjector, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ProjectorEdit = new wxButton(this, ID_BUTTON8, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	FlexGridSizer4->Add(Button_ProjectorEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_DeleteProjector = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer4->Add(Button_DeleteProjector, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableRow(0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Buttons:    "), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer5->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	Grid_Buttons = new wxGrid(this, ID_GRID2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID2"));
	Grid_Buttons->CreateGrid(0,1);
	Grid_Buttons->EnableEditing(true);
	Grid_Buttons->EnableGridLines(true);
	Grid_Buttons->SetDefaultColSize(200, true);
	Grid_Buttons->SetColLabelValue(0, _("Command"));
	Grid_Buttons->SetDefaultCellFont( Grid_Buttons->GetFont() );
	Grid_Buttons->SetDefaultCellTextColour( Grid_Buttons->GetForegroundColour() );
	FlexGridSizer5->Add(Grid_Buttons, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	Button_ButtonAdd = new wxButton(this, ID_BUTTON5, _("Add"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer6->Add(Button_ButtonAdd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ButtonEdit = new wxButton(this, ID_BUTTON6, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer6->Add(Button_ButtonEdit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_ButtonDelete = new wxButton(this, ID_BUTTON7, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer6->Add(Button_ButtonDelete, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnGrid_ProjectorsCellLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnGrid_ProjectorsLabelLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&OptionsDialog::OnGrid_ProjectorsCellSelect);
	Grid_Projectors->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&OptionsDialog::OnGrid_ProjectorsKeyDown,0,this);
	Grid_Projectors->Connect(wxEVT_SIZE,(wxObjectEventFunction)&OptionsDialog::OnGrid_ProjectorsResize,0,this);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_AddProjectorClick);
	Connect(ID_BUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ProjectorEditClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_DeleteProjectorClick);
	Connect(ID_GRID2,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnGrid_ButtonsCellLeftClick);
	Connect(ID_GRID2,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&OptionsDialog::OnGrid_ButtonsLabelLeftClick);
	Connect(ID_GRID2,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&OptionsDialog::OnGrid_ButtonsCellSelect);
	Grid_Buttons->Connect(wxEVT_KEY_DOWN,(wxObjectEventFunction)&OptionsDialog::OnGrid_ButtonsKeyDown,0,this);
	Grid_Buttons->Connect(wxEVT_SIZE,(wxObjectEventFunction)&OptionsDialog::OnGrid_ButtonsResize,0,this);
	Connect(ID_BUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonAddClick);
	Connect(ID_BUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonEditClick);
	Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_ButtonDeleteClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&OptionsDialog::OnButton_CancelClick);
	//*)
}

OptionsDialog::~OptionsDialog()
{
	//(*Destroy(OptionsDialog)
	//*)
}


void OptionsDialog::OnButton_OkClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnGrid_ProjectorsCellLeftClick(wxGridEvent& event)
{
}

void OptionsDialog::OnGrid_ProjectorsLabelLeftClick(wxGridEvent& event)
{
}

void OptionsDialog::OnGrid_ProjectorsKeyDown(wxKeyEvent& event)
{
}

void OptionsDialog::OnGrid_ProjectorsCellSelect(wxGridEvent& event)
{
}

void OptionsDialog::OnGrid_ButtonsCellLeftClick(wxGridEvent& event)
{
}

void OptionsDialog::OnGrid_ButtonsLabelLeftClick(wxGridEvent& event)
{
}

void OptionsDialog::OnGrid_ButtonsCellSelect(wxGridEvent& event)
{
}

void OptionsDialog::OnGrid_ButtonsKeyDown(wxKeyEvent& event)
{
}

void OptionsDialog::OnGrid_ButtonsResize(wxSizeEvent& event)
{
}

void OptionsDialog::OnGrid_ProjectorsResize(wxSizeEvent& event)
{
}

void OptionsDialog::OnButton_AddProjectorClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnButton_ProjectorEditClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnButton_DeleteProjectorClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnButton_ButtonAddClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnButton_ButtonEditClick(wxCommandEvent& event)
{
}

void OptionsDialog::OnButton_ButtonDeleteClick(wxCommandEvent& event)
{
}

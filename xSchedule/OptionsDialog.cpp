#include "OptionsDialog.h"

//(*InternalHeaders(OptionsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(OptionsDialog)
const long OptionsDialog::ID_CHECKBOX1 = wxNewId();
const long OptionsDialog::ID_CHECKBOX2 = wxNewId();
const long OptionsDialog::ID_CHECKBOX3 = wxNewId();
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
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	CheckBox_RunOnMachineStartup = new wxCheckBox(this, ID_CHECKBOX1, _("Run on machine startup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_RunOnMachineStartup->SetValue(false);
	FlexGridSizer1->Add(CheckBox_RunOnMachineStartup, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_StartOnLaunch = new wxCheckBox(this, ID_CHECKBOX2, _("Start sequence immediately on launch"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox_StartOnLaunch->SetValue(false);
	FlexGridSizer1->Add(CheckBox_StartOnLaunch, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_SendOffWhenNotRunning = new wxCheckBox(this, ID_CHECKBOX3, _("Send off data when not running sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
	CheckBox_SendOffWhenNotRunning->SetValue(false);
	FlexGridSizer1->Add(CheckBox_SendOffWhenNotRunning, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

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

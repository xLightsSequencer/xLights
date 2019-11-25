#include "PlayListItemRunProcessPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemRunProcess.h"

//(*InternalHeaders(PlayListItemRunProcessPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemRunProcessPanel)
const long PlayListItemRunProcessPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemRunProcessPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemRunProcessPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemRunProcessPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemRunProcessPanel::ID_CHECKBOX1 = wxNewId();
const long PlayListItemRunProcessPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemRunProcessPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemRunProcessPanel,wxPanel)
	//(*EventTable(PlayListItemRunProcessPanel)
	//*)
END_EVENT_TABLE()

PlayListItemRunProcessPanel::PlayListItemRunProcessPanel(wxWindow* parent, PlayListItemRunProcess* process, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _process = process;

	//(*Initialize(PlayListItemRunProcessPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_RunProcessName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_RunProcessName, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Command:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
	TextCtrl_Command = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Command, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_WaitForCompletion = new wxCheckBox(this, ID_CHECKBOX1, _("Wait for completion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_WaitForCompletion->SetValue(false);
	FlexGridSizer1->Add(CheckBox_WaitForCompletion, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunProcessPanel::OnTextCtrl_RunProcessNameText);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunProcessPanel::OnTextCtrl_CommandText);
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PlayListItemRunProcessPanel::OnCheckBox_WaitForCompletionClick);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunProcessPanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_RunProcessName->SetValue(process->GetRawName());
    TextCtrl_Command->SetValue(process->GetCommand());
    TextCtrl_Command->SetToolTip(PlayListItemRunProcess::GetTooltip());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)process->GetDelay() / 1000.0));
    CheckBox_WaitForCompletion->SetValue(process->GetWaitForCompletion());
}

PlayListItemRunProcessPanel::~PlayListItemRunProcessPanel()
{
	//(*Destroy(PlayListItemRunProcessPanel)
	//*)
    _process->SetName(TextCtrl_RunProcessName->GetValue().ToStdString());
    _process->SetCommand(TextCtrl_Command->GetValue().ToStdString());
    _process->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
    _process->SetWaitForCompletion(CheckBox_WaitForCompletion->GetValue());
}


void PlayListItemRunProcessPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemRunProcessPanel::OnCheckBox_WaitForCompletionClick(wxCommandEvent& event)
{
}

void PlayListItemRunProcessPanel::OnTextCtrl_CommandText(wxCommandEvent& event)
{
}

void PlayListItemRunProcessPanel::OnTextCtrl_RunProcessNameText(wxCommandEvent& event)
{
    _process->SetName(TextCtrl_RunProcessName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

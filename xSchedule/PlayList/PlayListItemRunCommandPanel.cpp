/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemRunCommandPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemRunCommand.h"
#include "../xScheduleMain.h"
#include "../CommandManager.h"
#include "../ScheduleManager.h"

//(*InternalHeaders(PlayListItemRunCommandPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemRunCommandPanel)
const long PlayListItemRunCommandPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemRunCommandPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemRunCommandPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemRunCommandPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemRunCommandPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemRunCommandPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemRunCommandPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemRunCommandPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemRunCommandPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemRunCommandPanel::ID_TEXTCTRL5 = wxNewId();
const long PlayListItemRunCommandPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemRunCommandPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemRunCommandPanel,wxPanel)
	//(*EventTable(PlayListItemRunCommandPanel)
	//*)
END_EVENT_TABLE()

PlayListItemRunCommandPanel::PlayListItemRunCommandPanel(wxWindow* parent, PlayListItemRunCommand* Command, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _Command = Command;

	//(*Initialize(PlayListItemRunCommandPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_RunCommandName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_RunCommandName, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Command:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Command = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_Command, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Parameter 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Parm1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Parm1, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Parameter 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Parm2 = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_Parm2, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Parameter 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Parm3 = new wxTextCtrl(this, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer1->Add(TextCtrl_Parm3, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunCommandPanel::OnTextCtrl_RunCommandNameText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemRunCommandPanel::OnChoice_CommandSelect);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunCommandPanel::OnTextCtrl_Parm1Text);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunCommandPanel::OnTextCtrl_Parm2Text);
	Connect(ID_TEXTCTRL5,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunCommandPanel::OnTextCtrl_Parm3Text);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemRunCommandPanel::OnTextCtrl_DelayText);
	//*)

    auto commands = Command->GetCommands();
    for (auto it = commands.begin(); it != commands.end(); ++it)
    {
        Choice_Command->AppendString(*it);
    }

    TextCtrl_RunCommandName->SetValue(Command->GetRawName());
    Choice_Command->SetStringSelection(Command->GetCommand());
    TextCtrl_Parm1->SetValue(Command->GetParm1());
    TextCtrl_Parm2->SetValue(Command->GetParm2());
    TextCtrl_Parm3->SetValue(Command->GetParm3());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)Command->GetDelay() / 1000.0));

    ValidateWindow();
}

PlayListItemRunCommandPanel::~PlayListItemRunCommandPanel()
{
	//(*Destroy(PlayListItemRunCommandPanel)
	//*)
    _Command->SetName(TextCtrl_RunCommandName->GetValue().ToStdString());
    _Command->SetCommand(Choice_Command->GetStringSelection().ToStdString());
    _Command->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
    _Command->SetParm1(TextCtrl_Parm1->GetValue().ToStdString());
    _Command->SetParm2(TextCtrl_Parm2->GetValue().ToStdString());
    _Command->SetParm3(TextCtrl_Parm3->GetValue().ToStdString());
}

void PlayListItemRunCommandPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemRunCommandPanel::OnTextCtrl_RunCommandNameText(wxCommandEvent& event)
{
    _Command->SetName(TextCtrl_RunCommandName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemRunCommandPanel::OnTextCtrl_Parm1Text(wxCommandEvent& event)
{
}

void PlayListItemRunCommandPanel::OnTextCtrl_Parm2Text(wxCommandEvent& event)
{
}

void PlayListItemRunCommandPanel::OnTextCtrl_Parm3Text(wxCommandEvent& event)
{
}

void PlayListItemRunCommandPanel::OnChoice_CommandSelect(wxCommandEvent& event)
{
    Command* c = xScheduleFrame::GetScheduleManager()->GetCommand(Choice_Command->GetStringSelection().ToStdString());
    Choice_Command->SetToolTip(c->GetParametersTip());
    ValidateWindow();
}

void PlayListItemRunCommandPanel::ValidateWindow()
{
    Command* c = xScheduleFrame::GetScheduleManager()->GetCommand(Choice_Command->GetStringSelection().ToStdString());

    if (c != nullptr)
    {
        if (c->_parms == 0)
        {
            TextCtrl_Parm1->Enable(false);
            TextCtrl_Parm2->Enable(false);
            TextCtrl_Parm3->Enable(false);
        }
        else if (c->_parms == 1)
        {
            TextCtrl_Parm1->Enable(true);
            TextCtrl_Parm2->Enable(false);
            TextCtrl_Parm3->Enable(false);
        }
        else if (c->_parms == 2)
        {
            TextCtrl_Parm1->Enable(true);
            TextCtrl_Parm2->Enable(true);
            TextCtrl_Parm3->Enable(false);
        }
        else if (c->_parms >= 3)
        {
            TextCtrl_Parm1->Enable(true);
            TextCtrl_Parm2->Enable(true);
            TextCtrl_Parm3->Enable(true);
        }
    }
}

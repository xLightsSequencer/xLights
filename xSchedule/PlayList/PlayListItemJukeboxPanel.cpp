/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemJukeboxPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemJukebox.h"

//(*InternalHeaders(PlayListItemJukeboxPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemJukeboxPanel)
const long PlayListItemJukeboxPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemJukeboxPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemJukeboxPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemJukeboxPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemJukeboxPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemJukeboxPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemJukeboxPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemJukeboxPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemJukeboxPanel,wxPanel)
	//(*EventTable(PlayListItemJukeboxPanel)
	//*)
END_EVENT_TABLE()

PlayListItemJukeboxPanel::PlayListItemJukeboxPanel(wxWindow* parent, PlayListItemJukebox* jukebox, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _jukebox = jukebox;

	//(*Initialize(PlayListItemJukeboxPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_JukeboxName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_JukeboxName, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Port = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice_Port->SetSelection( Choice_Port->Append(_("A")) );
	Choice_Port->Append(_("B"));
	FlexGridSizer1->Add(Choice_Port, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Button:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Button = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 50, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Button->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Button, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemJukeboxPanel::OnTextCtrl_JukeboxNameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemJukeboxPanel::OnTextCtrl_DelayText);
	//*)

    SpinCtrl_Button->SetValue(jukebox->GetButton());
    TextCtrl_JukeboxName->SetValue(jukebox->GetRawName());
    Choice_Port->SetStringSelection(jukebox->GetPortName());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)jukebox->GetDelay() / 1000.0));

    ValidateWindow();
}

PlayListItemJukeboxPanel::~PlayListItemJukeboxPanel()
{
	//(*Destroy(PlayListItemJukeboxPanel)
	//*)
    _jukebox->SetName(TextCtrl_JukeboxName->GetValue().ToStdString());
    _jukebox->SetPortName(Choice_Port->GetStringSelection().ToStdString());
    _jukebox->SetButton(SpinCtrl_Button->GetValue());
    _jukebox->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}

void PlayListItemJukeboxPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemJukeboxPanel::OnTextCtrl_JukeboxNameText(wxCommandEvent& event)
{
    _jukebox->SetName(TextCtrl_JukeboxName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemJukeboxPanel::ValidateWindow()
{
}

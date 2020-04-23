/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemARTNetTriggerPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemARTNetTrigger.h"

//(*InternalHeaders(PlayListItemARTNetTriggerPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemARTNetTriggerPanel)
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_SPINCTRL3 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemARTNetTriggerPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemARTNetTriggerPanel,wxPanel)
	//(*EventTable(PlayListItemARTNetTriggerPanel)
	//*)
END_EVENT_TABLE()

PlayListItemARTNetTriggerPanel::PlayListItemARTNetTriggerPanel(wxWindow* parent, PlayListItemARTNetTrigger* ARTNetTrigger, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _ARTNetTrigger = ARTNetTrigger;

	//(*Initialize(PlayListItemARTNetTriggerPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_ARTNetTriggerName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_ARTNetTriggerName, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, _("255.255.255.255"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("OEM:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_OEM = new wxSpinCtrl(this, ID_SPINCTRL1, _T("65535"), wxDefaultPosition, wxDefaultSize, 0, 0, 65535, 65535, _T("ID_SPINCTRL1"));
	SpinCtrl_OEM->SetValue(_T("65535"));
	FlexGridSizer1->Add(SpinCtrl_OEM, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Key = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Key->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Key, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Sub Key:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl1_SubKey = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 255, 1, _T("ID_SPINCTRL3"));
	SpinCtrl1_SubKey->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl1_SubKey, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Data:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Data = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_Data, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemARTNetTriggerPanel::OnTextCtrl_ARTNetTriggerNameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemARTNetTriggerPanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_ARTNetTriggerName->SetValue(ARTNetTrigger->GetRawName());
    TextCtrl_IPAddress->SetValue(ARTNetTrigger->GetIP());
    SpinCtrl_OEM->SetValue(ARTNetTrigger->GetOEM());
    SpinCtrl_Key->SetValue(ARTNetTrigger->GetKey());
    SpinCtrl1_SubKey->SetValue(ARTNetTrigger->GetSubKey());
    TextCtrl_Data->SetValue(ARTNetTrigger->GetData());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)ARTNetTrigger->GetDelay() / 1000.0));
}

PlayListItemARTNetTriggerPanel::~PlayListItemARTNetTriggerPanel()
{
	//(*Destroy(PlayListItemARTNetTriggerPanel)
	//*)
    _ARTNetTrigger->SetName(TextCtrl_ARTNetTriggerName->GetValue().ToStdString());
    _ARTNetTrigger->SetOEM(SpinCtrl_OEM->GetValue());
    _ARTNetTrigger->SetKey(SpinCtrl_Key->GetValue());
    _ARTNetTrigger->SetSubKey(SpinCtrl1_SubKey->GetValue());
    _ARTNetTrigger->SetIP(TextCtrl_IPAddress->GetValue().ToStdString());
    _ARTNetTrigger->SetData(TextCtrl_Data->GetValue().ToStdString());
    _ARTNetTrigger->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}

void PlayListItemARTNetTriggerPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemARTNetTriggerPanel::OnTextCtrl_ARTNetTriggerNameText(wxCommandEvent& event)
{
    _ARTNetTrigger->SetName(TextCtrl_ARTNetTriggerName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemARTNetTriggerPanel::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
}

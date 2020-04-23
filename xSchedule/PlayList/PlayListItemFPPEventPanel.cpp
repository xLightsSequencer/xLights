/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemFPPEventPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemFPPEvent.h"

//(*InternalHeaders(PlayListItemFPPEventPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemFPPEventPanel)
const long PlayListItemFPPEventPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemFPPEventPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemFPPEventPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemFPPEventPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemFPPEventPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemFPPEventPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemFPPEventPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemFPPEventPanel::ID_SPINCTRL2 = wxNewId();
const long PlayListItemFPPEventPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemFPPEventPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemFPPEventPanel,wxPanel)
	//(*EventTable(PlayListItemFPPEventPanel)
	//*)
END_EVENT_TABLE()

PlayListItemFPPEventPanel::PlayListItemFPPEventPanel(wxWindow* parent, PlayListItemFPPEvent* fppevent, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _fppevent = fppevent;

	//(*Initialize(PlayListItemFPPEventPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_FPPEventName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_FPPEventName, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, _("255.255.255.255"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Id - Major:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Major = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 24, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_Major->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Major, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Id - Minor:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Minor = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 24, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_Minor->SetValue(_T("1"));
	FlexGridSizer1->Add(SpinCtrl_Minor, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFPPEventPanel::OnTextCtrl_FPPEventNameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemFPPEventPanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_FPPEventName->SetValue(fppevent->GetRawName());
    TextCtrl_IPAddress->SetValue(fppevent->GetIP());
    SpinCtrl_Major->SetValue(fppevent->GetMajor());
    SpinCtrl_Minor->SetValue(fppevent->GetMinor());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)fppevent->GetDelay() / 1000.0));
}

PlayListItemFPPEventPanel::~PlayListItemFPPEventPanel()
{
	//(*Destroy(PlayListItemFPPEventPanel)
	//*)
    _fppevent->SetName(TextCtrl_FPPEventName->GetValue().ToStdString());
    _fppevent->SetMajor(SpinCtrl_Major->GetValue());
    _fppevent->SetMinor(SpinCtrl_Minor->GetValue());
    _fppevent->SetIP(TextCtrl_IPAddress->GetValue().ToStdString());
    _fppevent->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
}

void PlayListItemFPPEventPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemFPPEventPanel::OnTextCtrl_FPPEventNameText(wxCommandEvent& event)
{
    _fppevent->SetName(TextCtrl_FPPEventName->GetValue().ToStdString());
    wxCommandEvent e(EVT_UPDATEITEMNAME);
    wxPostEvent(GetParent()->GetParent()->GetParent()->GetParent(), e);
}

void PlayListItemFPPEventPanel::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
}

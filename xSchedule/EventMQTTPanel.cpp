/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventMQTTPanel.h"
#include "EventDialog.h"
#include "events/EventMQTT.h"
#include "../xLights/UtilFunctions.h"

//(*InternalHeaders(EventMQTTPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EventMQTTPanel)
const long EventMQTTPanel::ID_STATICTEXT2 = wxNewId();
const long EventMQTTPanel::ID_TEXTCTRL2 = wxNewId();
const long EventMQTTPanel::ID_STATICTEXT3 = wxNewId();
const long EventMQTTPanel::ID_SPINCTRL1 = wxNewId();
const long EventMQTTPanel::ID_STATICTEXT6 = wxNewId();
const long EventMQTTPanel::ID_TEXTCTRL5 = wxNewId();
const long EventMQTTPanel::ID_STATICTEXT1 = wxNewId();
const long EventMQTTPanel::ID_TEXTCTRL1 = wxNewId();
const long EventMQTTPanel::ID_STATICTEXT4 = wxNewId();
const long EventMQTTPanel::ID_TEXTCTRL3 = wxNewId();
const long EventMQTTPanel::ID_STATICTEXT5 = wxNewId();
const long EventMQTTPanel::ID_TEXTCTRL4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EventMQTTPanel,wxPanel)
	//(*EventTable(EventMQTTPanel)
	//*)
END_EVENT_TABLE()

EventMQTTPanel::EventMQTTPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EventMQTTPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Broker IP:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IP = new wxTextCtrl(this, ID_TEXTCTRL2, _("127.0.0.1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_IP, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Port = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1883"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 1883, _T("ID_SPINCTRL1"));
	SpinCtrl_Port->SetValue(_T("1883"));
	FlexGridSizer1->Add(SpinCtrl_Port, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Client Id:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_ClientId = new wxTextCtrl(this, ID_TEXTCTRL5, _("xSchedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer1->Add(TextCtrl_ClientId, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Topic:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Path = new wxTextCtrl(this, ID_TEXTCTRL1, _("xSchedule/Event"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Path, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("User Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Username = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Username, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Password:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EventMQTTPanel::OnTextCtrl_IPText);
	Connect(ID_TEXTCTRL5,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EventMQTTPanel::OnTextCtrl_ClientIdText);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EventMQTTPanel::OnTextCtrl_PathText);
	//*)
}

EventMQTTPanel::~EventMQTTPanel()
{
	//(*Destroy(EventMQTTPanel)
	//*)
}

bool EventMQTTPanel::ValidateWindow()
{
    return (TextCtrl_ClientId->GetValue() != "" && IsIPValid(TextCtrl_IP->GetValue()) && TextCtrl_Path->GetValue().Trim().Trim(false) != "" && TextCtrl_Path->GetValue()[0] != '/');
}

void EventMQTTPanel::Save(EventBase* event)
{
    EventMQTT* e = (EventMQTT*)event;
    e->SetTopic(TextCtrl_Path->GetValue().ToStdString());
    e->SetBrokerIP(TextCtrl_IP->GetValue().ToStdString());
    e->SetBrokerPort(SpinCtrl_Port->GetValue());
    e->SetUsername(TextCtrl_Username->GetValue().ToStdString());
    e->SetPassword(TextCtrl_Password->GetValue().ToStdString());
    e->SetClientId(TextCtrl_ClientId->GetValue().ToStdString());
}

void EventMQTTPanel::Load(EventBase* event)
{
    EventMQTT* e = (EventMQTT*)event;
    TextCtrl_Path->SetValue(e->GetTopic());
    TextCtrl_IP->SetValue(e->GetBrokerIP());
    SpinCtrl_Port->SetValue(e->GetBrokerPort());
    TextCtrl_Username->SetValue(e->GetUsername());
    TextCtrl_Password->SetValue(e->GetPassword());
    TextCtrl_ClientId->SetValue(e->GetClientId());
}

void EventMQTTPanel::OnTextCtrl_PathText(wxCommandEvent& event)
{
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}

void EventMQTTPanel::OnTextCtrl_IPText(wxCommandEvent& event)
{
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}

void EventMQTTPanel::OnTextCtrl_ClientIdText(wxCommandEvent& event)
{
    ((EventDialog*)GetParent()->GetParent()->GetParent()->GetParent())->ValidateWindow();
}

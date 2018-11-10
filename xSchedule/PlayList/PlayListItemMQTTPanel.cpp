#include "PlayListItemMQTTPanel.h"
#include "PlayListDialog.h"
#include "PlayListItemMQTT.h"

//(*InternalHeaders(PlayListItemMQTTPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemMQTTPanel)
const long PlayListItemMQTTPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT9 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL7 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL1 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL4 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemMQTTPanel::ID_SPINCTRL1 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL5 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL10 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT8 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL6 = wxNewId();
const long PlayListItemMQTTPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemMQTTPanel::ID_TEXTCTRL2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemMQTTPanel,wxPanel)
	//(*EventTable(PlayListItemMQTTPanel)
	//*)
END_EVENT_TABLE()

PlayListItemMQTTPanel::PlayListItemMQTTPanel(wxWindow* parent, PlayListItemMQTT* MQTT, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _MQTT = MQTT;

	//(*Initialize(PlayListItemMQTTPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FlexGridSizer1->AddGrowableRow(7);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_OSCName = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_OSCName, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Client Id:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer1->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_ClientId = new wxTextCtrl(this, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL7"));
	FlexGridSizer1->Add(TextCtrl_ClientId, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Topic:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Path = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Path, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Broker IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Port = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1883"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 1883, _T("ID_SPINCTRL1"));
	SpinCtrl_Port->SetValue(_T("1883"));
	FlexGridSizer1->Add(SpinCtrl_Port, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("User name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Username = new wxTextCtrl(this, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer1->Add(TextCtrl_Username, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Password:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(this, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL10"));
	FlexGridSizer1->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Data:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer1->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Data = new wxTextCtrl(this, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, _T("ID_TEXTCTRL6"));
	FlexGridSizer1->Add(TextCtrl_Data, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL2, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemMQTTPanel::OnTextCtrl_RunProcessNameText);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemMQTTPanel::OnTextCtrl_DelayText);
	//*)

    TextCtrl_ClientId->SetValue(MQTT->GetClientId());
    TextCtrl_OSCName->SetValue(MQTT->GetRawName());
    TextCtrl_IPAddress->SetValue(MQTT->GetIPAddress());
    TextCtrl_Path->SetValue(MQTT->GetTopic());
    TextCtrl_Path->SetToolTip(PlayListItem::GetTagHint());
    SpinCtrl_Port->SetValue(MQTT->GetPort());
    TextCtrl_Username->SetValue(MQTT->GetUsername());
    TextCtrl_Password->SetValue(MQTT->GetPassword());
    TextCtrl_Data->SetValue(MQTT->GetData());
    TextCtrl_Data->SetToolTip(PlayListItemMQTT::GetTooltip());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)MQTT->GetDelay() / 1000.0));

    ValidateWindow();
}

PlayListItemMQTTPanel::~PlayListItemMQTTPanel()
{
	//(*Destroy(PlayListItemMQTTPanel)
	//*)
    _MQTT->SetName(TextCtrl_OSCName->GetValue().ToStdString());
    _MQTT->SetIPAddress(TextCtrl_IPAddress->GetValue().ToStdString());
    _MQTT->SetTopic(TextCtrl_Path->GetValue().ToStdString());
    _MQTT->SetPort(SpinCtrl_Port->GetValue());
    _MQTT->SetUserName(TextCtrl_Username->GetValue().ToStdString());
    _MQTT->SetPassword(TextCtrl_Password->GetValue().ToStdString());
    _MQTT->SetData(TextCtrl_Data->GetValue().ToStdString());
    _MQTT->SetDelay(wxAtof(TextCtrl_Delay->GetValue())*1000);
    _MQTT->SetClientId(TextCtrl_ClientId->GetValue().ToStdString());
}

void PlayListItemMQTTPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

void PlayListItemMQTTPanel::OnTextCtrl_RunProcessNameText(wxCommandEvent& event)
{
    _MQTT->SetName(TextCtrl_OSCName->GetValue().ToStdString());
    ((PlayListDialog*)GetParent()->GetParent()->GetParent()->GetParent())->UpdateTree();
}

void PlayListItemMQTTPanel::OnChoice_TypeSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemMQTTPanel::ValidateWindow()
{
}
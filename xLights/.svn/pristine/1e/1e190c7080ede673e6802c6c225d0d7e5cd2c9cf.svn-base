#include "E131Dialog.h"

//(*InternalHeaders(E131Dialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(E131Dialog)
const long E131Dialog::ID_STATICTEXT4 = wxNewId();
const long E131Dialog::ID_STATICTEXT5 = wxNewId();
const long E131Dialog::ID_RADIOBUTTON1 = wxNewId();
const long E131Dialog::ID_RADIOBUTTON2 = wxNewId();
const long E131Dialog::ID_STATICTEXT1 = wxNewId();
const long E131Dialog::ID_TEXTCTRL_IP_ADDR = wxNewId();
const long E131Dialog::ID_STATICTEXT2 = wxNewId();
const long E131Dialog::ID_SPINCTRL1 = wxNewId();
const long E131Dialog::ID_STATICTEXT3 = wxNewId();
const long E131Dialog::ID_SPINCTRL2 = wxNewId();
const long E131Dialog::ID_STATICTEXT6 = wxNewId();
const long E131Dialog::ID_SPINCTRL_LAST_CHANNEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(E131Dialog,wxDialog)
	//(*EventTable(E131Dialog)
	//*)
END_EVENT_TABLE()

E131Dialog::E131Dialog(wxWindow* parent)
{
	//(*Initialize(E131Dialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("E1.31 Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Sets up an E1.31 connection over ethernet.\n\nSupported devices include those made\nby j1sys, SanDevices, and Entec. Also\nsupports the Lynx EtherDongle, and \nE1.31 projects on DIYC.\n\nThe universe numbers entered here\nshould match the universe numbers \ndefined on your E1.31 device."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Method"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	RadioButtonMulticast = new wxRadioButton(this, ID_RADIOBUTTON1, _("Multicast"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	RadioButtonMulticast->SetValue(true);
	BoxSizer1->Add(RadioButtonMulticast, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	RadioButtonUnicast = new wxRadioButton(this, ID_RADIOBUTTON2, _("Unicast"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	BoxSizer1->Add(RadioButtonUnicast, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlIpAddr = new wxTextCtrl(this, ID_TEXTCTRL_IP_ADDR, _("MULTICAST"), wxDefaultPosition, wxSize(115,28), 0, wxDefaultValidator, _T("ID_TEXTCTRL_IP_ADDR"));
	TextCtrlIpAddr->Disable();
	FlexGridSizer2->Add(TextCtrlIpAddr, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Starting Universe #"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_StartUniv = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_StartUniv->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_StartUniv, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("# of Universes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_NumUniv = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 63999, 1, _T("ID_SPINCTRL2"));
	SpinCtrl_NumUniv->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_NumUniv, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Last Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_LastChannel = new wxSpinCtrl(this, ID_SPINCTRL_LAST_CHANNEL, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 512, _T("ID_SPINCTRL_LAST_CHANNEL"));
	SpinCtrl_LastChannel->SetValue(_T("512"));
	FlexGridSizer2->Add(SpinCtrl_LastChannel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&E131Dialog::OnRadioButtonMulticastSelect);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&E131Dialog::OnRadioButtonUnicastSelect);
	//*)
}

E131Dialog::~E131Dialog()
{
	//(*Destroy(E131Dialog)
	//*)
}


void E131Dialog::OnRadioButtonUnicastSelect(wxCommandEvent& event)
{
    TextCtrlIpAddr->Clear();
    TextCtrlIpAddr->Enable(true);
}

void E131Dialog::OnRadioButtonMulticastSelect(wxCommandEvent& event)
{
    TextCtrlIpAddr->SetValue(_("MULTICAST"));
    TextCtrlIpAddr->Enable(false);
}

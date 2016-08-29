#include "ArtNetDialog.h"

//(*InternalHeaders(ArtNetDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/socket.h>
#include <wx/regex.h>

//(*IdInit(ArtNetDialog)
const long ArtNetDialog::ID_STATICTEXT1 = wxNewId();
const long ArtNetDialog::ID_STATICTEXT2 = wxNewId();
const long ArtNetDialog::ID_TEXTCTRL1 = wxNewId();
const long ArtNetDialog::ID_STATICTEXT4 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL1 = wxNewId();
const long ArtNetDialog::ID_STATICTEXT5 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL2 = wxNewId();
const long ArtNetDialog::ID_STATICTEXT6 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL3 = wxNewId();
const long ArtNetDialog::ID_STATICTEXT7 = wxNewId();
const long ArtNetDialog::ID_SPINCTRL4 = wxNewId();
const long ArtNetDialog::ID_STATICTEXT3 = wxNewId();
const long ArtNetDialog::ID_TEXTCTRL2 = wxNewId();
const long ArtNetDialog::ID_BUTTON1 = wxNewId();
const long ArtNetDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ArtNetDialog,wxDialog)
	//(*EventTable(ArtNetDialog)
	//*)
END_EVENT_TABLE()

ArtNetDialog::ArtNetDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ArtNetDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("ArtNET Setup"), wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Sets up an ArtNET connection over ethernet.\n\nThe universe numbers entered here\nshould match the universe numbers \ndefined on your ArtNET device."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlIPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrlIPAddress->SetMaxLength(15);
	FlexGridSizer2->Add(TextCtrlIPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Network"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlNet = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 127, 0, _T("ID_SPINCTRL1"));
	SpinCtrlNet->SetValue(_T("0"));
	FlexGridSizer2->Add(SpinCtrlNet, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Subnet"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlSubnet = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxDefaultSize, 0, 0, 15, 0, _T("ID_SPINCTRL2"));
	SpinCtrlSubnet->SetValue(_T("0"));
	FlexGridSizer2->Add(SpinCtrlSubnet, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Universe"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlUniverse = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 0, 15, 1, _T("ID_SPINCTRL3"));
	SpinCtrlUniverse->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrlUniverse, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Last Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlChannels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 512, _T("ID_SPINCTRL4"));
	SpinCtrlChannels->SetValue(_T("512"));
	FlexGridSizer2->Add(SpinCtrlChannels, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlDescription = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrlDescription, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ArtNetDialog::OnTextCtrlIPAddressText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtNetDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ArtNetDialog::OnButtonCancelClick);
	//*)

    ValidateWindow();
}

ArtNetDialog::~ArtNetDialog()
{
	//(*Destroy(ArtNetDialog)
	//*)
}


void ArtNetDialog::OnTextCtrlIPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void ArtNetDialog::OnButtonOkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void ArtNetDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void ArtNetDialog::ValidateWindow()
{
    wxString ips = TextCtrlIPAddress->GetValue().Trim(false).Trim(true);
    if (ips == "")
    {
        ButtonOk->Disable();
    }
    else
    {
        wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");
        //wxRegEx regex("^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");

        if (regxIPAddr.Matches(ips))
        {
            ButtonOk->Enable();
        }
        else
        {
            ButtonOk->Disable();
        }
    }
}
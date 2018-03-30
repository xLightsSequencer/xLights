#include "DDPDialog.h"

//(*InternalHeaders(DDPDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "DDPOutput.h"
#include "OutputManager.h"
#include <wx/regex.h>

//(*IdInit(DDPDialog)
const long DDPDialog::ID_STATICTEXT1 = wxNewId();
const long DDPDialog::ID_STATICTEXT2 = wxNewId();
const long DDPDialog::ID_TEXTCTRL1 = wxNewId();
const long DDPDialog::ID_STATICTEXT5 = wxNewId();
const long DDPDialog::ID_SPINCTRL1 = wxNewId();
const long DDPDialog::ID_STATICTEXT7 = wxNewId();
const long DDPDialog::ID_SPINCTRL4 = wxNewId();
const long DDPDialog::ID_STATICTEXT4 = wxNewId();
const long DDPDialog::ID_SPINCTRL_ChannelsPerPacket = wxNewId();
const long DDPDialog::ID_CHECKBOX_KEEPCHANNELS = wxNewId();
const long DDPDialog::ID_STATICTEXT3 = wxNewId();
const long DDPDialog::ID_TEXTCTRL2 = wxNewId();
const long DDPDialog::ID_CHECKBOX1 = wxNewId();
const long DDPDialog::ID_BUTTON1 = wxNewId();
const long DDPDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(DDPDialog,wxDialog)
	//(*EventTable(DDPDialog)
	//*)
END_EVENT_TABLE()

DDPDialog::DDPDialog(wxWindow* parent, DDPOutput* DDP, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _DDP = DDP;
    _outputManager = outputManager;

	//(*Initialize(DDPDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, id, _("DDP Setup"), wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Sets up an DDP connection over ethernet.\n\nId if unique amongst all your output universes \nthen you can use #id:startchannel for start \nchannel identification"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlIPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrlIPAddress->SetMaxLength(15);
	FlexGridSizer2->Add(TextCtrlIPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl1 = new wxSpinCtrl(this, ID_SPINCTRL1, _T("64001"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 64001, _T("ID_SPINCTRL1"));
	SpinCtrl1->SetValue(_T("64001"));
	FlexGridSizer2->Add(SpinCtrl1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Num Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer4->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlChannels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 1000000, 512, _T("ID_SPINCTRL4"));
	SpinCtrlChannels->SetValue(_T("512"));
	FlexGridSizer4->Add(SpinCtrlChannels, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Channels per packet"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer4->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_ChannelsPerPacket = new wxSpinCtrl(this, ID_SPINCTRL_ChannelsPerPacket, _T("1440"), wxDefaultPosition, wxDefaultSize, 0, 1, 1440, 1440, _T("ID_SPINCTRL_ChannelsPerPacket"));
	SpinCtrl_ChannelsPerPacket->SetValue(_T("1440"));
	FlexGridSizer4->Add(SpinCtrl_ChannelsPerPacket, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxKeepChannels = new wxCheckBox(this, ID_CHECKBOX_KEEPCHANNELS, _("Keep Channel Numbers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_KEEPCHANNELS"));
	CheckBoxKeepChannels->SetValue(true);
	FlexGridSizer4->Add(CheckBoxKeepChannels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlDescription = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer4->Add(TextCtrlDescription, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox1 = new wxCheckBox(this, ID_CHECKBOX1, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox1->SetValue(false);
	FlexGridSizer4->Add(CheckBox1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DDPDialog::OnTextCtrlIPAddressText);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DDPDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DDPDialog::OnButtonCancelClick);
	//*)

    wxASSERT(DDP != nullptr);

    SpinCtrlChannels->SetValue(DDP->GetChannels());
    SpinCtrl_ChannelsPerPacket->SetValue(DDP->GetChannelsPerPacket());
    CheckBoxKeepChannels->SetValue(DDP->IsKeepChannelNumbers());
    TextCtrlDescription->SetValue(DDP->GetDescription());
    TextCtrlIPAddress->SetValue(DDP->GetIP());
    CheckBox1->SetValue(DDP->IsSuppressDuplicateFrames());
    SpinCtrl1->SetValue(DDP->GetId());

    ButtonOk->SetDefault();

    ValidateWindow();
}

DDPDialog::~DDPDialog()
{
	//(*Destroy(DDPDialog)
	//*)
}


void DDPDialog::OnTextCtrlIPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void DDPDialog::OnButtonOkClick(wxCommandEvent& event)
{
    _DDP->SetIP(TextCtrlIPAddress->GetValue().ToStdString());
    _DDP->SetChannels(SpinCtrlChannels->GetValue());
    _DDP->SetChannelsPerPacket(SpinCtrl_ChannelsPerPacket->GetValue());
    _DDP->SetDescription(TextCtrlDescription->GetValue().ToStdString());
    _DDP->SetSuppressDuplicateFrames(CheckBox1->IsChecked());
    _DDP->KeepChannelNumber(CheckBoxKeepChannels->IsChecked());
    _DDP->SetId(SpinCtrl1->GetValue());

    EndDialog(wxID_OK);
}

void DDPDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void DDPDialog::ValidateWindow()
{
    std::string ips = TextCtrlIPAddress->GetValue().Trim(false).Trim(true).ToStdString();
    if (ips == "")
    {
        ButtonOk->Disable();
    }
    else
    {
        if (IPOutput::IsIPValidOrHostname(ips, true))
        {
            ButtonOk->Enable();
        }
        else
        {
            ButtonOk->Disable();
        }
    }
}


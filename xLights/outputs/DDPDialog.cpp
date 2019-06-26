#include "DDPDialog.h"

//(*InternalHeaders(DDPDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "DDPOutput.h"
#include "OutputManager.h"
#include "../controllers/ControllerRegistry.h"
#include <wx/regex.h>
#include "../xLights/UtilFunctions.h"
#include "../xLightsMain.h"

//(*IdInit(DDPDialog)
const long DDPDialog::ID_STATICTEXT1 = wxNewId();
const long DDPDialog::ID_TEXTCTRL1 = wxNewId();
const long DDPDialog::ID_SPINCTRL1 = wxNewId();
const long DDPDialog::ID_SPINCTRL4 = wxNewId();
const long DDPDialog::ID_SPINCTRL_ChannelsPerPacket = wxNewId();
const long DDPDialog::ID_CHECKBOX_KEEPCHANNELS = wxNewId();
const long DDPDialog::ID_CHECKBOX1 = wxNewId();
const long DDPDialog::ID_TEXTCTRL2 = wxNewId();
const long DDPDialog::ID_CHOICE1 = wxNewId();
const long DDPDialog::ID_CHECKBOX_AUTO_CHANNELS = wxNewId();
const long DDPDialog::ID_TEXTCTRL3 = wxNewId();
const long DDPDialog::ID_BUTTON1 = wxNewId();
const long DDPDialog::ID_BUTTON2 = wxNewId();
const long DDPDialog::ID_BUTTON3 = wxNewId();
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
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;

	Create(parent, id, _("DDP Setup"), wxDefaultPosition, wxDefaultSize, wxCAPTION, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Sets up an DDP connection over ethernet.\n\nId if unique amongst all your output universes \nthen you can use #id:startchannel for start \nchannel identification"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlIPAddress = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrlIPAddress->SetMaxLength(15);
	FlexGridSizer2->Add(TextCtrlIPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Id"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl1 = new wxSpinCtrl(this, ID_SPINCTRL1, _T("64001"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 64001, _T("ID_SPINCTRL1"));
	SpinCtrl1->SetValue(_T("64001"));
	FlexGridSizer2->Add(SpinCtrl1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText7 = new wxStaticText(this, wxID_ANY, _("Num Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlChannels = new wxSpinCtrl(this, ID_SPINCTRL4, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 1000000, 512, _T("ID_SPINCTRL4"));
	SpinCtrlChannels->SetValue(_T("512"));
	FlexGridSizer4->Add(SpinCtrlChannels, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Channels per packet"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_ChannelsPerPacket = new wxSpinCtrl(this, ID_SPINCTRL_ChannelsPerPacket, _T("1440"), wxDefaultPosition, wxDefaultSize, 0, 1, 1440, 1440, _T("ID_SPINCTRL_ChannelsPerPacket"));
	SpinCtrl_ChannelsPerPacket->SetValue(_T("1440"));
	FlexGridSizer4->Add(SpinCtrl_ChannelsPerPacket, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxKeepChannels = new wxCheckBox(this, ID_CHECKBOX_KEEPCHANNELS, _("Keep Channel Numbers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_KEEPCHANNELS"));
	CheckBoxKeepChannels->SetValue(true);
	FlexGridSizer4->Add(CheckBoxKeepChannels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox1 = new wxCheckBox(this, ID_CHECKBOX1, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox1->SetValue(false);
	FlexGridSizer4->Add(CheckBox1, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrlDescription = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer4->Add(TextCtrlDescription, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Controller Type"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ControllerTypeChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer4->Add(ControllerTypeChoice, 1, wxALL|wxEXPAND, 5);
	StaticText8 = new wxStaticText(this, wxID_ANY, _("Use Auto Start Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox_Auto_Channels = new wxCheckBox(this, ID_CHECKBOX_AUTO_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_CHANNELS"));
	CheckBox_Auto_Channels->SetValue(false);
	FlexGridSizer4->Add(CheckBox_Auto_Channels, 1, wxALL|wxEXPAND, 5);
	StaticText9 = new wxStaticText(this, wxID_ANY, _("FPP Proxy Host/IP"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FPPProxyHost = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer4->Add(FPPProxyHost, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
	FlexGridSizer3 = new wxFlexGridSizer(1, 0, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	VisualizeButton = new wxButton(this, ID_BUTTON3, _("Visualize"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer3->Add(VisualizeButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&DDPDialog::OnTextCtrlIPAddressText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&DDPDialog::OnControllerTypeChoiceSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DDPDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DDPDialog::OnButtonCancelClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&DDPDialog::OnVisualizeButtonClick);
	//*)

    wxASSERT(DDP != nullptr);

    SpinCtrlChannels->SetValue(DDP->GetChannels());
    SpinCtrl_ChannelsPerPacket->SetValue(DDP->GetChannelsPerPacket());
    CheckBoxKeepChannels->SetValue(DDP->IsKeepChannelNumbers());
    TextCtrlDescription->SetValue(DDP->GetDescription());
    TextCtrlIPAddress->SetValue(DDP->GetIP());
    CheckBox1->SetValue(DDP->IsSuppressDuplicateFrames());
    SpinCtrl1->SetValue(DDP->GetId());


    ControllerTypeChoice->Append("Unknown");
    int idx = 0;
    int x = 0;
    for (auto &a : ControllerRegistry::GetControllerIds()) {
        const ControllerRules *rules = ControllerRegistry::GetRulesForController(a);
        if (rules && rules->GetSupportedInputProtocols().count("DDP") != 0) {
            x++;
            ControllerTypeChoice->Append(rules->GetControllerDescription(), (void*)rules);
            if (a == DDP->GetControllerId()) {
                idx = x;
            }
        }
    }
    ControllerTypeChoice->SetSelection(idx);
    FPPProxyHost->SetValue(_DDP->GetFPPProxyIP());

    VisualizeButton->Enable(idx != 0);
    SetEscapeId(ButtonCancel->GetId());
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

void DDPDialog::OnButtonOkClick(wxCommandEvent& event) {
    SaveDDPFields();
    EndDialog(wxID_OK);
}
void DDPDialog::SaveDDPFields() {
    _DDP->SetIP(TextCtrlIPAddress->GetValue().ToStdString());
    _DDP->SetChannels(SpinCtrlChannels->GetValue());
    _DDP->SetChannelsPerPacket(SpinCtrl_ChannelsPerPacket->GetValue());
    _DDP->SetDescription(TextCtrlDescription->GetValue().ToStdString());
    _DDP->SetSuppressDuplicateFrames(CheckBox1->IsChecked());
    _DDP->KeepChannelNumber(CheckBoxKeepChannels->IsChecked());
    _DDP->SetId(SpinCtrl1->GetValue());
    _DDP->SetFPPProxyIP(FPPProxyHost->GetValue());
    int idx = ControllerTypeChoice->GetSelection();
    if (idx == 0) {
        _DDP->SetControllerId("");
    } else {
        const ControllerRules *rules = (const ControllerRules *)ControllerTypeChoice->GetClientData(idx);
        _DDP->SetControllerId(rules->GetControllerId());
    }
}

void DDPDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void DDPDialog::ValidateWindow()
{
    std::string ips = TextCtrlIPAddress->GetValue().Trim(false).Trim(true).ToStdString();
    if (ips == "") {
        ButtonOk->Disable();
    } else {
        if (IsIPValidOrHostname(ips, true)) {
            ButtonOk->Enable();
        } else {
            ButtonOk->Disable();
        }
    }
}


void DDPDialog::OnVisualizeButtonClick(wxCommandEvent& event)
{
    SaveDDPFields();
    xLightsFrame *parent = (xLightsFrame*)GetParent();
    parent->VisualiseOutput(_DDP, this);
}

void DDPDialog::OnControllerTypeChoiceSelect(wxCommandEvent& event)
{
    int idx = ControllerTypeChoice->GetSelection();
    VisualizeButton->Enable(idx != 0);
}

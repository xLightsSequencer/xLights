#include "E131Dialog.h"

//(*InternalHeaders(E131Dialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "E131Output.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"
#include "../controllers/ControllerRegistry.h"
#include "../xLightsMain.h"


//(*IdInit(E131Dialog)
const long E131Dialog::ID_STATICTEXT4 = wxNewId();
const long E131Dialog::ID_RADIOBUTTON1 = wxNewId();
const long E131Dialog::ID_RADIOBUTTON2 = wxNewId();
const long E131Dialog::ID_TEXTCTRL_IP_ADDR = wxNewId();
const long E131Dialog::ID_SPINCTRL1 = wxNewId();
const long E131Dialog::ID_SPINCTRL2 = wxNewId();
const long E131Dialog::ID_CHECKBOX1 = wxNewId();
const long E131Dialog::ID_SPINCTRL_LAST_CHANNEL = wxNewId();
const long E131Dialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long E131Dialog::ID_CHECKBOX2 = wxNewId();
const long E131Dialog::ID_SPINCTRL_PRIORITY = wxNewId();
const long E131Dialog::ID_CHOICE1 = wxNewId();
const long E131Dialog::ID_CHECKBOX_AUTO_CHANNELS = wxNewId();
const long E131Dialog::ID_TEXTCTRL1 = wxNewId();
const long E131Dialog::ID_BUTTON1 = wxNewId();
const long E131Dialog::ID_BUTTON2 = wxNewId();
const long E131Dialog::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(E131Dialog,wxDialog)
    //(*EventTable(E131Dialog)
    //*)
END_EVENT_TABLE()

E131Dialog::E131Dialog(wxWindow* parent, E131Output* e131, OutputManager* outputManager, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _e131 = e131;
    _outputManager = outputManager;

    //(*Initialize(E131Dialog)
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticText* ControllerLabel;
    wxStaticText* OneOutputLabel;
    wxStaticText* StaticText10;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText3;
    wxStaticText* StaticText4;
    wxStaticText* StaticText5;
    wxStaticText* StaticText6;
    wxStaticText* StaticText7;
    wxStaticText* StaticText8;
    wxStaticText* StaticText9;

    Create(parent, wxID_ANY, _("E1.31 Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    DescriptionStaticText = new wxStaticText(this, ID_STATICTEXT4, _("Sets up an E1.31 connection over ethernet.\n\nSupported devices include those made by Falcon, j1sys,\nSanDevices, Advatek Lights, and Entec. Also supports the\n Lynx EtherDongle, and E1.31 projects on DIYC.\n\nThe universe numbers entered here should match the \nuniverse numbers defined on your E1.31 device."), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(DescriptionStaticText, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText5 = new wxStaticText(this, wxID_ANY, _("Method"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    RadioButtonMulticast = new wxRadioButton(this, ID_RADIOBUTTON1, _("Multicast"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    RadioButtonMulticast->SetValue(true);
    BoxSizer1->Add(RadioButtonMulticast, 1, wxALL|wxALIGN_LEFT, 5);
    RadioButtonUnicast = new wxRadioButton(this, ID_RADIOBUTTON2, _("Unicast"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    BoxSizer1->Add(RadioButtonUnicast, 1, wxALL|wxALIGN_LEFT, 5);
    FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, wxID_ANY, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlIpAddr = new wxTextCtrl(this, ID_TEXTCTRL_IP_ADDR, _("MULTICAST"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_IP_ADDR"));
    TextCtrlIpAddr->Disable();
    FlexGridSizer2->Add(TextCtrlIpAddr, 1, wxALL|wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, wxID_ANY, _("Starting Universe #"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_StartUniv = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 63999, 1, _T("ID_SPINCTRL1"));
    SpinCtrl_StartUniv->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_StartUniv, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, wxID_ANY, _("# of Universes"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_NumUniv = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 63999, 1, _T("ID_SPINCTRL2"));
    SpinCtrl_NumUniv->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_NumUniv, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    OneOutputLabel = new wxStaticText(this, wxID_ANY, _("One Output"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(OneOutputLabel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    MultiE131CheckBox = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    MultiE131CheckBox->SetValue(false);
    FlexGridSizer2->Add(MultiE131CheckBox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, wxID_ANY, _("Universe Size"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_LastChannel = new wxSpinCtrl(this, ID_SPINCTRL_LAST_CHANNEL, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 512, _T("ID_SPINCTRL_LAST_CHANNEL"));
    SpinCtrl_LastChannel->SetValue(_T("512"));
    FlexGridSizer2->Add(SpinCtrl_LastChannel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
    TextCtrl_Description->SetMaxLength(64);
    FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, wxID_ANY, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SuppressDuplicates = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_SuppressDuplicates->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SuppressDuplicates, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(this, wxID_ANY, _("Priority"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Priority = new wxSpinCtrl(this, ID_SPINCTRL_PRIORITY, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 200, 100, _T("ID_SPINCTRL_PRIORITY"));
    SpinCtrl_Priority->SetValue(_T("100"));
    FlexGridSizer2->Add(SpinCtrl_Priority, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ControllerLabel = new wxStaticText(this, wxID_ANY, _("Controller Type"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(ControllerLabel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ControllerChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(ControllerChoice, 1, wxALL|wxEXPAND, 5);
    StaticText4 = new wxStaticText(this, wxID_ANY, _("Use Auto Start Channels"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Auto_Channels = new wxCheckBox(this, ID_CHECKBOX_AUTO_CHANNELS, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_CHANNELS"));
    CheckBox_Auto_Channels->SetValue(false);
    FlexGridSizer2->Add(CheckBox_Auto_Channels, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, wxID_ANY, _("FPP Proxy IP/Host"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FPPProxyIP = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer2->Add(FPPProxyIP, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    VisualizeButton = new wxButton(this, ID_BUTTON3, _("Visualize"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer3->Add(VisualizeButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&E131Dialog::OnRadioButtonMulticastSelect);
    Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&E131Dialog::OnRadioButtonUnicastSelect);
    Connect(ID_TEXTCTRL_IP_ADDR,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&E131Dialog::OnTextCtrlIpAddrText);
    Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&E131Dialog::OnSpinCtrl_NumUnivChange);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&E131Dialog::OnMultiE131CheckBoxClick);
    Connect(ID_TEXTCTRL_DESCRIPTION,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&E131Dialog::OnTextCtrl_DescriptionText);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&E131Dialog::OnControllerChoiceSelect);
    Connect(ID_CHECKBOX_AUTO_CHANNELS,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&E131Dialog::OnCheckBox_Auto_ChannelsClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&E131Dialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&E131Dialog::OnButton_CancelClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&E131Dialog::OnVisualizeButtonClick);
    //*)

    CheckBox_SuppressDuplicates->SetValue(_e131->IsSuppressDuplicateFrames());
    SpinCtrl_StartUniv->SetValue(_e131->GetUniverse());
    SpinCtrl_NumUniv->SetValue(_e131->GetUniverses());
    MultiE131CheckBox->SetValue(_e131->IsOutputCollection());
    SpinCtrl_Priority->SetValue(_e131->GetPriority());
	CheckBox_Auto_Channels->SetValue(_e131->IsAutoLayoutModels());
    if (_e131->GetIP() != "") {
        MultiE131CheckBox->SetValue(true);
        MultiE131CheckBox->Hide();
        OneOutputLabel->Hide();
        SpinCtrl_NumUniv->Enable(true);
    } else {
        MultiE131CheckBox->SetValue(true); // default to multi to encourage the use of this setup
        MultiE131CheckBox->Enable(true);
        SpinCtrl_NumUniv->Enable(true);
    }
    SpinCtrl_LastChannel->SetValue(_e131->GetChannels());
    TextCtrl_Description->SetValue(_e131->GetDescription());

    if (wxString(_e131->GetIP().c_str()).StartsWith("239.255.") || _e131->GetIP() == "MULTICAST" || _e131->GetIP() == "") {
        TextCtrlIpAddr->SetValue("MULTICAST");
        TextCtrlIpAddr->Enable(false);
        RadioButtonMulticast->SetValue(true);
    } else {
        TextCtrlIpAddr->SetValue(_e131->GetIP());
        TextCtrlIpAddr->Enable(true);
        RadioButtonUnicast->SetValue(true);
    }

    ControllerChoice->Append("Unknown");
    int idx = 0;
    int x = 0;
    for (auto &a : ControllerRegistry::GetControllerIds()) {
        const ControllerRules *rules = ControllerRegistry::GetRulesForController(a);
        if (rules && rules->GetSupportedInputProtocols().count("E131") != 0) {
            x++;
            ControllerChoice->Append(rules->GetControllerDescription(), (void*)rules);
            if (a == _e131->GetControllerId()) {
                idx = x;
            }
        }
    }
    ControllerChoice->SetSelection(idx);
    VisualizeButton->Enable(idx != 0);

    FPPProxyIP->SetValue(_e131->GetFPPProxyIP());

    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    SetEscapeId(Button_Cancel->GetId());
    Button_Ok->SetDefault();
    ValidateWindow();
}

E131Dialog::~E131Dialog()
{
    //(*Destroy(E131Dialog)
    //*)
}


void E131Dialog::OnRadioButtonUnicastSelect(wxCommandEvent& event)
{
    TextCtrlIpAddr->SetValue(_e131->GetIP());
    TextCtrlIpAddr->Enable(true);
    ValidateWindow();
}

void E131Dialog::OnRadioButtonMulticastSelect(wxCommandEvent& event)
{
    TextCtrlIpAddr->SetValue(_("MULTICAST"));
    TextCtrlIpAddr->Enable(false);
    ValidateWindow();
}

void E131Dialog::OnSpinCtrl_NumUnivChange(wxSpinEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnTextCtrlIpAddrText(wxCommandEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnMultiE131CheckBoxClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnCheckBox_Auto_ChannelsClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnButton_OkClick(wxCommandEvent& event)
{
    SaveFields();
    EndDialog(wxID_OK);
}

void E131Dialog::SaveFields() {
    _e131->SetIP(TextCtrlIpAddr->GetValue().ToStdString());
    _e131->SetUniverse(SpinCtrl_StartUniv->GetValue());
    _e131->SetChannels(SpinCtrl_LastChannel->GetValue());
    _e131->SetDescription(TextCtrl_Description->GetValue().ToStdString());
    _e131->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
    _e131->SetPriority(SpinCtrl_Priority->GetValue());
	_e131->SetAutoStartChannels(CheckBox_Auto_Channels->IsChecked());
    _e131->SetFPPProxyIP(FPPProxyIP->GetValue());
    int idx = ControllerChoice->GetSelection();
    if (idx == 0) {
        _e131->SetControllerId("");
    } else {
        const ControllerRules *rules = (const ControllerRules *)ControllerChoice->GetClientData(idx);
        _e131->SetControllerId(rules->GetControllerId());
    }

    if (SpinCtrl_NumUniv->GetValue() > 1 && !MultiE131CheckBox->GetValue()) {
        Output* last = _e131;
        for (int i = 1; i < SpinCtrl_NumUniv->GetValue(); i++) {
            E131Output* e = new E131Output();
            e->SetIP(TextCtrlIpAddr->GetValue().ToStdString());
            e->SetUniverse(SpinCtrl_StartUniv->GetValue() + i);
            e->SetChannels(SpinCtrl_LastChannel->GetValue());
            e->SetDescription(TextCtrl_Description->GetValue().ToStdString());
            e->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
            e->SetPriority(SpinCtrl_Priority->GetValue());
            _outputManager->AddOutput(e, last);
            last = e;
        }
    } else {
        _e131->CreateMultiUniverses(SpinCtrl_NumUniv->GetValue());
    }
    _outputManager->SomethingChanged();
}

void E131Dialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void E131Dialog::ValidateWindow()
{
    if (TextCtrlIpAddr->GetValue().IsEmpty() ||
        ((RadioButtonUnicast->GetValue() && !IsIPValidOrHostname(TextCtrlIpAddr->GetValue().ToStdString(), true)) ||
         SpinCtrl_StartUniv->GetValue() + SpinCtrl_NumUniv->GetValue() >= 64000) ||
		(CheckBox_Auto_Channels->IsChecked() && TextCtrl_Description->GetValue().empty()) ||
		(CheckBox_Auto_Channels->IsChecked() && !IsUniqueDescription(TextCtrl_Description->GetValue(), _e131, _outputManager)) ||
			(CheckBox_Auto_Channels->IsChecked() && !IsUniqueIP(TextCtrlIpAddr->GetValue(), _e131, _outputManager)))
	{
        Button_Ok->Enable(false);
    } else {
        Button_Ok->Enable();
    }
}

void E131Dialog::OnVisualizeButtonClick(wxCommandEvent& event)
{
    SaveFields();
    xLightsFrame *parent = (xLightsFrame*)GetParent();
    parent->VisualiseOutput(_e131, this);
}

void E131Dialog::OnControllerChoiceSelect(wxCommandEvent& event)
{
    int idx = ControllerChoice->GetSelection();
    VisualizeButton->Enable(idx != 0);
}

bool E131Dialog::IsUniqueDescription(const std::string& newDescription, E131Output* output, OutputManager* outputManager)
{
	for (auto it : outputManager->GetOutputs())
	{
		if (it->GetType() == "E131" &&  it != output && newDescription == it->GetDescription())
		{
			return false;
		}
	}
	return true;
}

bool E131Dialog::IsUniqueIP(const std::string& newIP, E131Output* output, OutputManager* outputManager)
{
	for (auto it : outputManager->GetOutputs())
	{
		if (it->GetType() == "E131" && it != output && (newIP == it->GetIP() || newIP == it->GetResolvedIP()))
		{
			return false;
		}
	}
	return true;
}

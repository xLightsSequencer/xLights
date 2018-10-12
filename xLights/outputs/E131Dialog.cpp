#include "E131Dialog.h"

//(*InternalHeaders(E131Dialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "E131Output.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"

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
const long E131Dialog::ID_STATICTEXT7 = wxNewId();
const long E131Dialog::ID_CHECKBOX1 = wxNewId();
const long E131Dialog::ID_STATICTEXT6 = wxNewId();
const long E131Dialog::ID_SPINCTRL_LAST_CHANNEL = wxNewId();
const long E131Dialog::ID_STATICTEXT8 = wxNewId();
const long E131Dialog::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long E131Dialog::ID_STATICTEXT9 = wxNewId();
const long E131Dialog::ID_CHECKBOX2 = wxNewId();
const long E131Dialog::ID_BUTTON1 = wxNewId();
const long E131Dialog::ID_BUTTON2 = wxNewId();
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

    Create(parent, wxID_ANY, _("E1.31 Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Sets up an E1.31 connection over ethernet.\n\nSupported devices include those made\nby Falcon, j1sys, SanDevices, Advatek Lights,\nand Entec. Also supports the Lynx \nEtherDongle, and E1.31 projects on DIYC.\n\nThe universe numbers entered here\nshould match the universe numbers \ndefined on your E1.31 device."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Method"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxVERTICAL);
    RadioButtonMulticast = new wxRadioButton(this, ID_RADIOBUTTON1, _("Multicast"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    RadioButtonMulticast->SetValue(true);
    BoxSizer1->Add(RadioButtonMulticast, 1, wxALL|wxALIGN_LEFT, 5);
    RadioButtonUnicast = new wxRadioButton(this, ID_RADIOBUTTON2, _("Unicast"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    BoxSizer1->Add(RadioButtonUnicast, 1, wxALL|wxALIGN_LEFT, 5);
    FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlIpAddr = new wxTextCtrl(this, ID_TEXTCTRL_IP_ADDR, _("MULTICAST"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_IP_ADDR"));
    TextCtrlIpAddr->Disable();
    FlexGridSizer2->Add(TextCtrlIpAddr, 1, wxALL|wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Starting Universe #"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_StartUniv = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 63999, 1, _T("ID_SPINCTRL1"));
    SpinCtrl_StartUniv->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_StartUniv, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("# of Universes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_NumUniv = new wxSpinCtrl(this, ID_SPINCTRL2, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 63999, 1, _T("ID_SPINCTRL2"));
    SpinCtrl_NumUniv->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_NumUniv, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("One Output"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    MultiE131CheckBox = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    MultiE131CheckBox->SetValue(false);
    FlexGridSizer2->Add(MultiE131CheckBox, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Last Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_LastChannel = new wxSpinCtrl(this, ID_SPINCTRL_LAST_CHANNEL, _T("512"), wxDefaultPosition, wxDefaultSize, 0, 1, 512, 512, _T("ID_SPINCTRL_LAST_CHANNEL"));
    SpinCtrl_LastChannel->SetValue(_T("512"));
    FlexGridSizer2->Add(SpinCtrl_LastChannel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
    TextCtrl_Description->SetMaxLength(64);
    FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_SuppressDuplicates = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_SuppressDuplicates->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SuppressDuplicates, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&E131Dialog::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&E131Dialog::OnButton_CancelClick);
    //*)

    CheckBox_SuppressDuplicates->SetValue(_e131->IsSuppressDuplicateFrames());
    SpinCtrl_StartUniv->SetValue(_e131->GetUniverse());
    SpinCtrl_NumUniv->SetValue(_e131->GetUniverses());
    MultiE131CheckBox->SetValue(_e131->IsOutputCollection());
    if (_e131->GetIP() != "")
    {
        MultiE131CheckBox->Enable(false);
        if (!MultiE131CheckBox->GetValue()) {
            SpinCtrl_NumUniv->Enable(false);
        }
    }
    else
    {
        MultiE131CheckBox->Enable(true);
        SpinCtrl_NumUniv->Enable(true);
    }
    SpinCtrl_LastChannel->SetValue(_e131->GetChannels());
    TextCtrl_Description->SetValue(_e131->GetDescription());

    if (wxString(_e131->GetIP().c_str()).StartsWith("239.255.") || _e131->GetIP() == "MULTICAST" || _e131->GetIP() == "")
    {
        TextCtrlIpAddr->SetValue("MULTICAST");
        TextCtrlIpAddr->Enable(false);
        RadioButtonMulticast->SetValue(true);
    }
    else
    {
        TextCtrlIpAddr->SetValue(_e131->GetIP());
        TextCtrlIpAddr->Enable(true);
        RadioButtonUnicast->SetValue(true);
    }

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
}

void E131Dialog::OnTextCtrlIpAddrText(wxCommandEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnMultiE131CheckBoxClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void E131Dialog::OnButton_OkClick(wxCommandEvent& event)
{
    _e131->SetIP(TextCtrlIpAddr->GetValue().ToStdString());
    _e131->SetUniverse(SpinCtrl_StartUniv->GetValue());
    _e131->SetChannels(SpinCtrl_LastChannel->GetValue());
    _e131->SetDescription(TextCtrl_Description->GetValue().ToStdString());
    _e131->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());

    if (SpinCtrl_NumUniv->GetValue() > 1 && !MultiE131CheckBox->GetValue())
    {
        Output* last = _e131;
        for (int i = 1; i < SpinCtrl_NumUniv->GetValue(); i++)
        {
            E131Output* e = new E131Output();
            e->SetIP(TextCtrlIpAddr->GetValue().ToStdString());
            e->SetUniverse(SpinCtrl_StartUniv->GetValue() + i);
            e->SetChannels(SpinCtrl_LastChannel->GetValue());
            e->SetDescription(TextCtrl_Description->GetValue().ToStdString());
            e->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
            _outputManager->AddOutput(e, last);
            last = e;
        }
    }
    else
    {
        if (SpinCtrl_NumUniv->GetValue() > 1)
        {
            _e131->CreateMultiUniverses(SpinCtrl_NumUniv->GetValue());
        }
    }

    EndDialog(wxID_OK);
}

void E131Dialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void E131Dialog::ValidateWindow()
{
    if (TextCtrlIpAddr->GetValue().IsEmpty() ||
        ((RadioButtonUnicast->GetValue() && !IsIPValidOrHostname(TextCtrlIpAddr->GetValue().ToStdString(), true)) ||
         SpinCtrl_StartUniv->GetValue() + SpinCtrl_NumUniv->GetValue() >= 64000)
        )
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable();
    }
}

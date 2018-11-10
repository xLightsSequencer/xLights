#include "SerialPortWithRate.h"

//(*InternalHeaders(SerialPortWithRate)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/valtext.h>
#include <wx/msgdlg.h>

#include "SerialOutput.h"
#include "OutputManager.h"

//(*IdInit(SerialPortWithRate)
const long SerialPortWithRate::ID_CHOICE_PROTOCOL = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT_EXPLANATION = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT4 = wxNewId();
const long SerialPortWithRate::ID_SPINCTRL1 = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT_PORT = wxNewId();
const long SerialPortWithRate::ID_CHOICE_PORT = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT_RATE = wxNewId();
const long SerialPortWithRate::ID_CHOICE_BAUD_RATE = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT3 = wxNewId();
const long SerialPortWithRate::ID_TEXTCTRL_LAST_CHANNEL = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT1 = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT2 = wxNewId();
const long SerialPortWithRate::ID_TEXTCTRL_DESCRIPTION = wxNewId();
const long SerialPortWithRate::ID_CHECKBOX1 = wxNewId();
const long SerialPortWithRate::ID_BUTTON1 = wxNewId();
const long SerialPortWithRate::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SerialPortWithRate,wxDialog)
    //(*EventTable(SerialPortWithRate)
    //*)
END_EVENT_TABLE()

SerialPortWithRate::SerialPortWithRate(wxWindow* parent, SerialOutput** serial, OutputManager* outputManager, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _original = *serial;
    _serial = serial;
    _outputManager = outputManager;

    //(*Initialize(SerialPortWithRate)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;

    Create(parent, wxID_ANY, _("USB Setup"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Network Type (protocol)"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    ChoiceProtocol = new wxChoice(this, ID_CHOICE_PROTOCOL, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PROTOCOL"));
    ChoiceProtocol->SetSelection( ChoiceProtocol->Append(_("DMX")) );
    ChoiceProtocol->Append(_("Pixelnet"));
    ChoiceProtocol->Append(_("LOR"));
    ChoiceProtocol->Append(_("D-Light"));
    ChoiceProtocol->Append(_("Renard"));
    ChoiceProtocol->Append(_("OpenDMX"));
    FlexGridSizer3->Add(ChoiceProtocol, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextExplanation = new wxStaticText(this, ID_STATICTEXT_EXPLANATION, _("DMX controllers (or LOR or D-Light controllers in DMX mode)\nattached to an Entec DMX USB Pro, Lynx DMX dongle,\nDIYC RPM, DMXking.com, or DIY Blinky dongle.\n\nLast Channel should be 512 or less, unless you are using\na DIY Blinky dongle (in which case it can be up to 3036).\n\nId if unique amongst all your output universes then you can \nuse #id:startchannel for start channel identification"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_EXPLANATION"));
    FlexGridSizer3->Add(StaticTextExplanation, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Connection Details"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Id"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_Id = new wxSpinCtrl(this, ID_SPINCTRL1, _T("64001"), wxDefaultPosition, wxDefaultSize, 0, 1, 65535, 64001, _T("ID_SPINCTRL1"));
    SpinCtrl_Id->SetValue(_T("64001"));
    FlexGridSizer2->Add(SpinCtrl_Id, 1, wxALL|wxEXPAND, 5);
    StaticTextPort = new wxStaticText(this, ID_STATICTEXT_PORT, _("Port"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_PORT"));
    FlexGridSizer2->Add(StaticTextPort, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ChoicePort = new wxChoice(this, ID_CHOICE_PORT, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PORT"));
    FlexGridSizer2->Add(ChoicePort, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextRate = new wxStaticText(this, ID_STATICTEXT_RATE, _("Baud Rate"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_RATE"));
    FlexGridSizer2->Add(StaticTextRate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ChoiceBaudRate = new wxChoice(this, ID_CHOICE_BAUD_RATE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_BAUD_RATE"));
    ChoiceBaudRate->Append(_("9600"));
    ChoiceBaudRate->Append(_("19200"));
    ChoiceBaudRate->Append(_("38400"));
    ChoiceBaudRate->Append(_("57600"));
    ChoiceBaudRate->Append(_("115200"));
    ChoiceBaudRate->Append(_("250000"));
    FlexGridSizer2->Add(ChoiceBaudRate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Last Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    TextCtrlLastChannel = new wxTextCtrl(this, ID_TEXTCTRL_LAST_CHANNEL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_LAST_CHANNEL"));
    TextCtrlLastChannel->SetMaxLength(5);
    FlexGridSizer4->Add(TextCtrlLastChannel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("(3/rgb)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer4->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 2);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Description"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Description = new wxTextCtrl(this, ID_TEXTCTRL_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_DESCRIPTION"));
    TextCtrl_Description->SetMaxLength(64);
    FlexGridSizer2->Add(TextCtrl_Description, 1, wxALL|wxEXPAND, 5);
    CheckBox_SuppressDuplicates = new wxCheckBox(this, ID_CHECKBOX1, _("Suppress duplicate frames"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_SuppressDuplicates->SetValue(false);
    FlexGridSizer2->Add(CheckBox_SuppressDuplicates, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer5->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer5->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE_PROTOCOL,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SerialPortWithRate::OnChoiceProtocolSelect);
    Connect(ID_CHOICE_PORT,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SerialPortWithRate::OnChoicePortSelect);
    Connect(ID_CHOICE_BAUD_RATE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SerialPortWithRate::OnChoiceBaudRateSelect);
    Connect(ID_TEXTCTRL_LAST_CHANNEL,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SerialPortWithRate::OnTextCtrlLastChannelText);
    Connect(ID_TEXTCTRL_DESCRIPTION,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&SerialPortWithRate::OnTextCtrl_DescriptionText);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SerialPortWithRate::OnButton_OkClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SerialPortWithRate::OnButton_CancelClick);
    //*)

    ChoiceProtocol->Append(_("Syncrolight Serial"));

    MainSizer=FlexGridSizer1;

    auto ports = SerialOutput::GetPossibleSerialPorts();
    for (auto it = ports.begin(); it != ports.end(); ++it)
    {
        ChoicePort->Append(*it);
    }

    FlexGridSizer1->Fit(this);

    CheckBox_SuppressDuplicates->SetValue((*_serial)->IsSuppressDuplicateFrames());
    ChoiceProtocol->SetStringSelection((*_serial)->GetType());
    ChoicePort->SetStringSelection((*_serial)->GetCommPort());
    if ((*_serial)->GetBaudRate() == 0)
    {
        // dont select anything
    }
    else
    {
        ChoiceBaudRate->SetStringSelection(wxString::Format(wxT("%i"), (*_serial)->GetBaudRate()));
    }
    TextCtrlLastChannel->SetValue(wxString::Format(wxT("%ld"), (*_serial)->GetChannels()));
    TextCtrl_Description->SetValue((*_serial)->GetDescription());
    SpinCtrl_Id->SetValue((*_serial)->GetId());
    ProtocolChange();

    Button_Ok->SetDefault();
    SetEscapeId(Button_Cancel->GetId());
    ValidateWindow();
}

SerialPortWithRate::~SerialPortWithRate()
{
    //(*Destroy(SerialPortWithRate)
    //*)
}

void SerialPortWithRate::ProtocolChange()
{
    StaticTextExplanation->SetLabel((*_serial)->GetSetupHelp());
    ChoiceBaudRate->Enable((*_serial)->AllowsBaudRateSetting());
    ValidateWindow();
}

void SerialPortWithRate::OnChoiceProtocolSelect(wxCommandEvent& event)
{
    auto tmp = *_serial;
    *_serial = (*_serial)->Mutate(ChoiceProtocol->GetStringSelection().ToStdString());
    
    // dont delete it if it is the original output we were called with
    if (tmp != _original)
    {
        delete (tmp);
    }
    ProtocolChange();
    ValidateWindow();
}

void SerialPortWithRate::OnTextCtrl_DescriptionText(wxCommandEvent& event)
{
}

void SerialPortWithRate::OnButton_OkClick(wxCommandEvent& event)
{
    (*_serial)->SetCommPort(ChoicePort->GetStringSelection().ToStdString());
    if (ChoiceBaudRate->IsEnabled())
    {
        (*_serial)->SetBaudRate(wxAtoi(ChoiceBaudRate->GetStringSelection()));
    }
    (*_serial)->SetChannels(wxAtoi(TextCtrlLastChannel->GetValue()));
    (*_serial)->SetDescription(TextCtrl_Description->GetValue().ToStdString());
    (*_serial)->SetSuppressDuplicateFrames(CheckBox_SuppressDuplicates->IsChecked());
    (*_serial)->SetId(SpinCtrl_Id->GetValue());

    EndDialog(wxID_OK);
}

void SerialPortWithRate::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void SerialPortWithRate::OnChoicePortSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void SerialPortWithRate::OnChoiceBaudRateSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

void SerialPortWithRate::OnTextCtrlLastChannelText(wxCommandEvent& event)
{
    ValidateWindow();
}

void SerialPortWithRate::ValidateWindow()
{
    if (ChoicePort->GetStringSelection().IsEmpty() || 
        (ChoiceBaudRate->IsEnabled() && ChoiceBaudRate->GetStringSelection() == "") ||
        wxAtoi(TextCtrlLastChannel->GetValue()) <= 0 ||
        wxAtoi(TextCtrlLastChannel->GetValue()) > (*_serial)->GetMaxChannels()
        )
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}

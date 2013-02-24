#include "SerialPortWithRate.h"

//(*InternalHeaders(SerialPortWithRate)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)
#include <wx/valtext.h>
#include <wx/msgdlg.h>

//(*IdInit(SerialPortWithRate)
const long SerialPortWithRate::ID_CHOICE_PROTOCOL = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT_EXPLANATION = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT_PORT = wxNewId();
const long SerialPortWithRate::ID_CHOICE_PORT = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT_RATE = wxNewId();
const long SerialPortWithRate::ID_CHOICE_BAUD_RATE = wxNewId();
const long SerialPortWithRate::ID_STATICTEXT3 = wxNewId();
const long SerialPortWithRate::ID_TEXTCTRL_LAST_CHANNEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(SerialPortWithRate,wxDialog)
    //(*EventTable(SerialPortWithRate)
    //*)
END_EVENT_TABLE()

SerialPortWithRate::SerialPortWithRate(wxWindow* parent)
{
    //(*Initialize(SerialPortWithRate)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

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
    FlexGridSizer3->Add(ChoiceProtocol, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticTextExplanation = new wxStaticText(this, ID_STATICTEXT_EXPLANATION, _("DMX controllers (or LOR or D-Light controllers in DMX mode)\nattached to an Entec DMX USB Pro, Lynx DMX dongle,\nDIYC RPM, DMXking.com, or DIY Blinky dongle.\n\nLast Channel should be 512 or less, unless you are using\na DIY Blinky dongle (in which case it can be up to 3036)."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_EXPLANATION"));
    FlexGridSizer3->Add(StaticTextExplanation, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Connection Details"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
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
    FlexGridSizer2->Add(ChoiceBaudRate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Last Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrlLastChannel = new wxTextCtrl(this, ID_TEXTCTRL_LAST_CHANNEL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_LAST_CHANNEL"));
    TextCtrlLastChannel->SetMaxLength(5);
    FlexGridSizer2->Add(TextCtrlLastChannel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICE_PROTOCOL,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&SerialPortWithRate::OnChoiceProtocolSelect);
    //*)

    MainSizer=FlexGridSizer1;
    wxArrayString ports;
    PopulatePortChooser(&ports);
    ChoicePort->Append(ports);
#ifndef __WXOSX__
    ChoiceProtocol->Append(_("OpenDMX"));
#endif
    FlexGridSizer1->Fit(this);
}

SerialPortWithRate::~SerialPortWithRate()
{
    //(*Destroy(SerialPortWithRate)
    //*)
}

wxString SerialPortWithRate::GetRateString()
{
    wxString NetName = ChoiceProtocol->GetStringSelection();
    if (NetName == _("OpenDMX"))
    {
        return wxT("250000");
    }
    else if (ChoiceBaudRate->IsEnabled())
    {
        return ChoiceBaudRate->GetStringSelection();
    }
    else
    {
        return wxT("n/a");
    }
}

void SerialPortWithRate::ProtocolChange()
{
    wxString NetName = ChoiceProtocol->GetStringSelection();
    bool EnableRate;
    if (NetName == _("LOR"))
    {
        SetLabel(_("LOR controllers attached to any LOR dongle.\nMax of 8 channels at 9600 baud.\nMax of 48 channels at 57600 baud.\nMax of 96 channels at 115200 baud.\nRun your controllers in DMX mode for higher throughput."));
        EnableRate=true;
    }
    else if (NetName == _("D-Light"))
    {
        SetLabel(_("D-Light controllers attached to a D-Light dongle.\nMax of 8 channels at 9600 baud.\nMax of 48 channels at 57600 baud.\nMax of 96 channels at 115200 baud.\nRun your controllers in DMX mode for higher throughput."));
        EnableRate=true;
    }
    else if (NetName == _("Renard"))
    {
        SetLabel(_("Renard controllers connected to a serial port or\na USB dongle with virtual comm port. 2 stop bits\nare set automatically.\nMax of 42 channels at 9600 baud.\nMax of 260 channels at 57600 baud."));
        EnableRate=true;
    }
    else if (NetName == _("OpenDMX"))
    {
        SetLabel(_("DMX controllers (or LOR or D-Light controllers in DMX mode)\nattached to an LOR dongle, D-Light dongle, HolidayCoro\nprogramming cable, or Entec Open DMX dongle"));
        EnableRate=false;
    }
    else if (NetName == _("DMX"))
    {
        SetLabel(_("DMX controllers (or LOR or D-Light controllers in DMX mode)\nattached to an Entec DMX USB Pro, Lynx DMX dongle,\nDIYC RPM, DMXking.com, or DIY Blinky dongle.\n\nLast Channel should be 512 or less, unless you are using\na DIY Blinky dongle (in which case it can be up to 3036)."));
        EnableRate=false;
    }
    else if (NetName == _("Pixelnet"))
    {
        SetLabel(_("Pixelnet hub attached to a\nLynx Pixelnet dongle"));
        EnableRate=false;
    }
    ChoiceBaudRate->Enable(EnableRate);
}

void SerialPortWithRate::SetLabel(const wxString& newlabel)
{
    StaticTextExplanation->SetLabel(newlabel);
    //MainSizer->Fit(this);
}

void SerialPortWithRate::PopulatePortChooser(wxArrayString *chooser)
{
    chooser->Add(wxT("NotConnected"));
#ifdef __WXMSW__
    // Windows
    chooser->Add(wxT("COM1"));
    chooser->Add(wxT("COM2"));
    chooser->Add(wxT("COM3"));
    chooser->Add(wxT("COM4"));
    chooser->Add(wxT("COM5"));
    chooser->Add(wxT("COM6"));
    chooser->Add(wxT("COM7"));
    chooser->Add(wxT("COM8"));
    chooser->Add(wxT("COM9"));
    chooser->Add(wxT("\\\\.\\COM10"));
    chooser->Add(wxT("\\\\.\\COM11"));
    chooser->Add(wxT("\\\\.\\COM12"));
    chooser->Add(wxT("\\\\.\\COM13"));
    chooser->Add(wxT("\\\\.\\COM14"));
    chooser->Add(wxT("\\\\.\\COM15"));
    chooser->Add(wxT("\\\\.\\COM16"));
    chooser->Add(wxT("\\\\.\\COM17"));
    chooser->Add(wxT("\\\\.\\COM18"));
    chooser->Add(wxT("\\\\.\\COM19"));
    chooser->Add(wxT("\\\\.\\COM20"));
#elif __WXOSX__
    // no standard device names for USB-serial converters on OS/X
    // scan /dev directory for candidates
    wxArrayString output,errors;
    wxExecute(wxT("ls -1 /dev"), output, errors, wxEXEC_SYNC);
    if (!errors.IsEmpty())
    {
        wxMessageBox(errors.Last(), _("Error"));
    }
    else if (output.IsEmpty())
    {
        wxMessageBox(_("no devices found"), _("Error"));
    }
    else
    {
        for (int i=0; i<output.Count(); i++)
        {
            if (output[i].StartsWith(wxT("cu.")))
            {
                chooser->Add(wxT("/dev/") + output[i]);
            }
        }
    }
#else
    // Linux
    chooser->Add(wxT("/dev/ttyS0"));
    chooser->Add(wxT("/dev/ttyS1"));
    chooser->Add(wxT("/dev/ttyS2"));
    chooser->Add(wxT("/dev/ttyS3"));
    chooser->Add(wxT("/dev/ttyUSB0"));
    chooser->Add(wxT("/dev/ttyUSB1"));
    chooser->Add(wxT("/dev/ttyUSB2"));
    chooser->Add(wxT("/dev/ttyUSB3"));
    chooser->Add(wxT("/dev/ttyUSB4"));
    chooser->Add(wxT("/dev/ttyUSB5"));
#endif
}


void SerialPortWithRate::OnChoiceProtocolSelect(wxCommandEvent& event)
{
    ProtocolChange();
}

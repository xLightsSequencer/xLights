/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemProjectorPanel.h"
#include "PlayListItemProjector.h"
#include "PlayListDialog.h"

#include "../ProjectorCodes.h"
#include "../../xLights/outputs/SerialOutput.h"

//(*InternalHeaders(PlayListItemProjectorPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(PlayListItemProjectorPanel)
const long PlayListItemProjectorPanel::ID_STATICTEXT1 = wxNewId();
const long PlayListItemProjectorPanel::ID_CHOICE2 = wxNewId();
const long PlayListItemProjectorPanel::ID_STATICTEXT2 = wxNewId();
const long PlayListItemProjectorPanel::ID_CHOICE3 = wxNewId();
const long PlayListItemProjectorPanel::ID_STATICTEXT7 = wxNewId();
const long PlayListItemProjectorPanel::ID_CHOICE4 = wxNewId();
const long PlayListItemProjectorPanel::ID_STATICTEXT5 = wxNewId();
const long PlayListItemProjectorPanel::ID_TEXTCTRL2 = wxNewId();
const long PlayListItemProjectorPanel::ID_STATICTEXT4 = wxNewId();
const long PlayListItemProjectorPanel::ID_TEXTCTRL3 = wxNewId();
const long PlayListItemProjectorPanel::ID_STATICTEXT6 = wxNewId();
const long PlayListItemProjectorPanel::ID_CHOICE1 = wxNewId();
const long PlayListItemProjectorPanel::ID_STATICTEXT3 = wxNewId();
const long PlayListItemProjectorPanel::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PlayListItemProjectorPanel,wxPanel)
	//(*EventTable(PlayListItemProjectorPanel)
	//*)
END_EVENT_TABLE()

PlayListItemProjectorPanel::PlayListItemProjectorPanel(wxWindow* parent, PlayListItemProjector* Projector, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _Projector = Projector;

	//(*Initialize(PlayListItemProjectorPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Projector Protocol:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_ProjectorProtocol = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer1->Add(Choice_ProjectorProtocol, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Command:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Command = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
	FlexGridSizer1->Add(Choice_Command, 1, wxALL|wxEXPAND, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Parameter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer1->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Parameter = new wxChoice(this, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE4"));
	Choice_Parameter->SetSelection( Choice_Parameter->Append(_("1")) );
	Choice_Parameter->Append(_("2"));
	Choice_Parameter->Append(_("3"));
	Choice_Parameter->Append(_("4"));
	Choice_Parameter->Append(_("5"));
	Choice_Parameter->Append(_("6"));
	Choice_Parameter->Append(_("7"));
	Choice_Parameter->Append(_("8"));
	Choice_Parameter->Append(_("9"));
	FlexGridSizer1->Add(Choice_Parameter, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("IP Address:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer1->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer1->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Password:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer1->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer1->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Comm Port:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer1->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_CommPort = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	FlexGridSizer1->Add(Choice_CommPort, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer1->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Delay = new wxTextCtrl(this, ID_TEXTCTRL1, _("0.000"), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Delay, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemProjectorPanel::OnChoice_ProjectorProtocolSelect);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemProjectorPanel::OnTextCtrl_IPAddressText);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PlayListItemProjectorPanel::OnChoice_CommPortSelect);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&PlayListItemProjectorPanel::OnTextCtrl_DelayText);
	//*)

    auto ports = SerialOutput::GetPossibleSerialPorts();
    for (auto it = ports.begin(); it != ports.end(); ++it)
    {
        Choice_CommPort->Append(*it);
    }
    Choice_CommPort->SetSelection(0);

    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                Choice_ProjectorProtocol->Append(*it2);
            }
        }
    }

    Choice_ProjectorProtocol->SetStringSelection(Projector->GetProjectorProtocol());
    PopulateCommands(Choice_ProjectorProtocol->GetStringSelection());
    Choice_Command->SetStringSelection(Projector->GetCommand());
    TextCtrl_IPAddress->SetValue(Projector->GetIPAddress());
    TextCtrl_Password->SetValue(Projector->GetPassword());
    TextCtrl_Delay->SetValue(wxString::Format(wxT("%.3f"), (float)Projector->GetDelay() / 1000.0));
    Choice_CommPort->SetStringSelection(Projector->GetCommPort());
    Choice_Parameter->SetStringSelection(Projector->GetParameter());
    ValidateWindow();
}

PlayListItemProjectorPanel::~PlayListItemProjectorPanel()
{
	//(*Destroy(PlayListItemProjectorPanel)
	//*)
    _Projector->SetProjectorProtocol(Choice_ProjectorProtocol->GetStringSelection().ToStdString());
    _Projector->SetCommand(Choice_Command->GetStringSelection().ToStdString());
    _Projector->SetCommPort(Choice_CommPort->GetStringSelection().ToStdString());
    _Projector->SetIPAddress(TextCtrl_IPAddress->GetValue().ToStdString());
    _Projector->SetPassword(TextCtrl_Password->GetValue().ToStdString());
    _Projector->SetParameter(Choice_Parameter->GetStringSelection().ToStdString());
    _Projector->SetDelay(wxAtof(TextCtrl_Delay->GetValue()) * 1000);
    int baudRate;
    int charBits;
    int stopBits;
    std::string parity;
    std::string ipProtocol;
    std::string commandData;
    int port;
    GetConfig(Choice_ProjectorProtocol->GetStringSelection(), Choice_Command->GetStringSelection(), baudRate, charBits, parity, stopBits, port, ipProtocol, commandData);
    _Projector->SetBaudRate(baudRate);
    _Projector->SetCharBits(charBits);
    _Projector->SetStopBits(stopBits);
    _Projector->SetParity(parity);
    _Projector->SetIPProtocol(ipProtocol);
    _Projector->SetPort(port);
    _Projector->SetCommandData(commandData);
}

void PlayListItemProjectorPanel::OnTextCtrl_DelayText(wxCommandEvent& event)
{
}

bool PlayListItemProjectorPanel::HasPassword(wxString protocol)
{
    bool foundProtocol = false;
    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        bool hasPassword = false;
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                if (*it2 == protocol)
                {
                    if (hasPassword) return true;
                    foundProtocol = true;
                }
            }
            else if (*it2 == "PASSWORD")
            {
                if (foundProtocol)
                {
                    return true;
                }
                hasPassword = true;
            }
        }
        if (foundProtocol) return false;
    }

    return false;
}

bool PlayListItemProjectorPanel::HasParameter(wxString protocol)
{
    bool foundProtocol = false;
    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        bool hasParameter = false;
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                if (*it2 == protocol)
                {
                    if (hasParameter) return true;
                    foundProtocol = true;
                }
            }
            else if (*it2 == "PARAMETER")
            {
                if (foundProtocol)
                {
                    return true;
                }
                hasParameter = true;
            }
        }
        if (foundProtocol) return false;
    }

    return false;
}

void PlayListItemProjectorPanel::PopulateCommands(wxString protocol)
{
    wxString oldCommand = Choice_Command->GetStringSelection();

    Choice_Command->Clear();

    bool foundProtocol = false;
    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                if (*it2 == protocol)
                {
                    foundProtocol = true;
                    break;
                }
            }
        }
        if (foundProtocol)
        {
            for (auto it2 = it->begin(); it2 != it->end(); ++it2)
            {
                if (*it2 == "NAME" ||
                    *it2 == "IP" ||
                    *it2 == "PASSWORD" ||
                    *it2 == "PROTOCOL" ||
                    *it2 == "PORT" ||
                    *it2 == "BAUD_RATE" ||
                    *it2 == "CHAR_BITS" ||
                    *it2 == "STOP_BITS" ||
                    *it2 == "PARITY"
                    )
                {
                    ++it2;
                }
                else
                {
                    Choice_Command->Append(*it2);
                    ++it2;
                }
            }
            Choice_Command->SetStringSelection(oldCommand);
            return;
        }
    }

    Choice_Command->SetStringSelection(oldCommand);
}

void PlayListItemProjectorPanel::GetConfig(wxString protocol, wxString command, int& baudRate, int& charBits, std::string& parity, int& stopBits, int& port, std::string& ipProtocol, std::string& commandData)
{
    bool foundProtocol = false;
    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                if (*it2 == protocol)
                {
                    foundProtocol = true;
                    break;
                }
            }
        }
        if (foundProtocol)
        {
            for (auto it2 = it->begin(); it2 != it->end(); ++it2)
            {
                auto value = it2;
                ++value;

                if (*it2 == command)
                {
                    commandData = *value;
                }
                else if (*it2 == "PROTOCOL")
                {
                    ipProtocol = *value;
                }
                else if (*it2 == "PORT")
                {
                    port = wxAtoi(*value);
                }
                else if (*it2 == "BAUD_RATE")
                {
                    baudRate = wxAtoi(*value);
                }
                else if (*it2 == "CHAR_BITS")
                {
                    charBits = wxAtoi(*value);
                }
                else if (*it2 == "STOP_BITS")
                {
                    stopBits = wxAtoi(*value);
                }
                else if (*it2 == "PARITY")
                {
                    parity = *value;
                }
                ++it2;
            }
            return;
        }
    }
}

bool PlayListItemProjectorPanel::HasIP(wxString protocol)
{
    bool foundProtocol = false;
    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        bool hasIP = false;
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                if (*it2 == protocol)
                {
                    if (hasIP) return true;
                    foundProtocol = true;
                }
            }
            else if (*it2 == "IP")
            {
                if (foundProtocol)
                {
                    return true;
                }
                hasIP = true;
            }
        }
        if (foundProtocol) return false;
    }

    return false;
}

bool PlayListItemProjectorPanel::HasCommPort(wxString protocol)
{
    bool foundProtocol = false;
    for (auto it = __projectorCodes.begin(); it != __projectorCodes.end(); ++it)
    {
        bool hasCommPort = false;
        for (auto it2 = it->begin(); it2 != it->end(); ++it2)
        {
            if (*it2 == "NAME")
            {
                ++it2;
                if (*it2 == protocol)
                {
                    if (hasCommPort) return true;
                    foundProtocol = true;
                }
            }
            else if (*it2 == "BAUD_RATE")
            {
                if (foundProtocol)
                {
                    return true;
                }
                hasCommPort = true;
            }
        }
        if (foundProtocol) return false;
    }

    return false;
}

void PlayListItemProjectorPanel::ValidateWindow()
{
    if (HasPassword(Choice_ProjectorProtocol->GetStringSelection()))
    {
        TextCtrl_Password->Enable();
    }
    else
    {
        TextCtrl_Password->Disable();
    }

    if (HasIP(Choice_ProjectorProtocol->GetStringSelection()))
    {
        TextCtrl_IPAddress->Enable();
    }
    else
    {
        TextCtrl_IPAddress->Disable();
    }

    if (HasCommPort(Choice_ProjectorProtocol->GetStringSelection()))
    {
        Choice_CommPort->Enable();
    }
    else
    {
        Choice_CommPort->Disable();
    }

    if (HasParameter(Choice_ProjectorProtocol->GetStringSelection()))
    {
        Choice_Parameter->Enable();
    }
    else
    {
        Choice_Parameter->Disable();
    }
}

void PlayListItemProjectorPanel::OnChoice_ProjectorProtocolSelect(wxCommandEvent& event)
{
    PopulateCommands(Choice_ProjectorProtocol->GetStringSelection());
    ValidateWindow();
}

void PlayListItemProjectorPanel::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void PlayListItemProjectorPanel::OnChoice_CommPortSelect(wxCommandEvent& event)
{
    ValidateWindow();
}

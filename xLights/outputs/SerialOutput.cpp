#include "SerialOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "SerialPortWithRate.h"
#include "LOROutput.h"
#include "DLightOutput.h"
#include "RenardOutput.h"
#include "DMXOutput.h"
#include "PixelNetOutput.h"
#include "OpenDMXOutput.h"
#include "OpenPixelNetOutput.h"
#include <wx/msgdlg.h>

SerialOutput::SerialOutput(wxXmlNode* node) : Output(node)
{
    _setType = "";
    _serial = nullptr;
    strcpy(_serialConfig, "8N1");
    _commPort = node->GetAttribute("ComPort", "").ToStdString();
    if (node->GetAttribute("BaudRate", "n/a") == "n/a")
    {
        _baudRate = 0;
    }
    else
    {
        _baudRate = wxAtoi(node->GetAttribute("BaudRate", ""));
    }
}

SerialOutput::SerialOutput(SerialOutput* output) : Output(output)
{
    _setType = "";
    _serial = nullptr;
    strcpy(_serialConfig, "8N1");
    _commPort = output->GetCommPort();
    _baudRate = output->GetBaudRate();
}

void SerialOutput::Save(wxXmlNode* node)
{
    if (_commPort != "")
    {
        node->AddAttribute("ComPort", _commPort.c_str());
    }

    if (_baudRate != 0)
    {
        node->AddAttribute("BaudRate", wxString::Format("%d", _baudRate));
    }
    else
    {
        node->AddAttribute("BaudRate", "n/a");
    }

    Output::Save(node);
}

SerialOutput::SerialOutput() : Output()
{
    _serial = nullptr;
    strcpy(_serialConfig, "8N1");
    _commPort = "";
    _baudRate = 0;
}

SerialOutput::~SerialOutput()
{
    if (_serial != nullptr) delete _serial;
}

std::string SerialOutput::GetAvailableSerialPorts()
{
    wxString res = "";

#ifdef __WXMSW__ 
    TCHAR valname[32];
    TCHAR portname[32];
    DWORD vallen = sizeof(valname);
    DWORD portlen = sizeof(portname);
    HKEY hkey = nullptr;
    DWORD err = 0;

    //enum serial comm ports (more user friendly, especially if USB-to-serial ports change):
    //logic based on http://www.cplusplus.com/forum/windows/73821/
    if (!(err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hkey)))
    {
        for (DWORD inx = 0; !(err = RegEnumValue(hkey, inx, (LPTSTR)valname, &vallen, nullptr, nullptr, (LPBYTE)portname, &portlen)) || (err == ERROR_MORE_DATA); ++inx)
        {
            if (err == ERROR_MORE_DATA)
            {
                portname[sizeof(portname) / sizeof(portname[0]) - 1] = '\0';
            }
            //need to enlarge read buf if this happens; just truncate string for now
            //                            debug(3, "found port[%d] %d:'%s' = %d:'%s', err 0x%x", inx, vallen, valname, portlen, portname, err);
            res += _(", ") + portname;
            vallen = sizeof(valname);
            portlen = sizeof(portname);
        }
        if (err && (err != ERROR_NO_MORE_ITEMS))
        {
            res = wxString::Format(", error %d (can't get serial comm ports from registry)", err);
        }
        if (hkey) RegCloseKey(hkey);
        if (!res.empty())
        {
            res = "\n(available ports: " + res.substr(2) + ")";
        }
    }
    else
    {
        res = "\n(no available ports)";
    }
#else
    res = "";
#endif // __WXMSW__

    return res.ToStdString();
}

bool SerialOutput::Open()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _ok = Output::Open();

    if (_commPort == "")
    {
        return false;
    }

    _serial = new SerialPort();

    int errcode = _serial->Open(_commPort, _baudRate, _serialConfig);
    if (errcode < 0)
    {
        logger_base.warn("Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);
        _ok = false;

        wxString msg = wxString::Format(_("Error occurred while connecting to %s network on %s %s \n\n") +
                                           _("Things to check:\n") +
                                           _("1. Are all required cables plugged in?\n") +
                                           _("2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.\n") +
                                           _("3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n") +
                                           _("Unable to open serial port %s. Error code = %d"),
                                           (const char *)GetType().c_str(), 
                                           (const char *)GetCommPort().c_str(), 
                                           (const char *)GetAvailableSerialPorts().c_str(), 
                                           (const char *)_commPort.c_str(), 
                                           errcode);
        wxMessageBox(msg, _("Communication Error"), wxOK);
    }

    return _ok;
}

void SerialOutput::Close()
{
    if (_serial != nullptr)
    {
        delete _serial;
        _serial = nullptr;
    }
}

size_t SerialOutput::TxNonEmptyCount() const
{
    return _serial ? _serial->WaitingToWrite() : 0;
}

bool SerialOutput::TxEmpty() const
{
    if (_serial) return (_serial->WaitingToWrite() == 0);
    return true;
}

bool SerialOutput::operator==(const SerialOutput& output) const
{
    if (GetType() != output.GetType()) return false;

    return _commPort == output.GetCommPort();
}

std::string SerialOutput::GetChannelMapping(int ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%i"), ch)) + " maps to ...\n";

    int channeloffset = ch - GetStartChannel() + 1;

    res += "Type: " + GetType() + "\n";
    res += "ComPort: " + _commPort + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%i"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}

std::string SerialOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += GetType() + " " + _commPort;
    res += " [1-" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%i"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

SerialOutput* SerialOutput::Mutate(SerialOutput* output)
{
    if (output->GetType() != output->_setType && output->_setType != "")
    {
        if (output->_setType == OUTPUT_DMX)
        {
            return new DMXOutput(output);
        }
        else if (output->_setType == OUTPUT_PIXELNET)
        {
            return new PixelNetOutput(output);
        }
        else if (output->_setType == OUTPUT_LOR)
        {
            return new LOROutput(output);
        }
        else if (output->_setType == OUTPUT_DLIGHT)
        {
            return new DLightOutput(output);
        }
        else if (output->_setType == OUTPUT_RENARD)
        {
            return new RenardOutput(output);
        }
        else if (output->_setType == OUTPUT_OPENDMX)
        {
            return new OpenDMXOutput(output);
        }
        else if (output->_setType == OUTPUT_OPENPIXELNET)
        {
            return new OpenPixelNetOutput(output);
        }
    }

    return nullptr;
}

#ifndef EXCLUDENETWORKUI

int SerialOutput::ConfigureWithMutate(wxWindow* parent, OutputManager& outputManager, SerialOutput** output)
{
    if (*output == nullptr)
    {
        *output = new DMXOutput();
    }

    int res = (*output)->Configure(parent, outputManager);

    if (res == wxID_OK)
    {
        (*output) = SerialOutput::Mutate(*output);
    }

    return res;
}

// This is a bit funky as we will need to create a serial output then mutate it into the correct output type
int SerialOutput::Configure(wxWindow* parent, OutputManager& outputManager)
{
    SerialPortWithRate dlg(parent);
    dlg.ChoiceProtocol->SetStringSelection(GetType());
    dlg.ChoicePort->SetStringSelection(_commPort.c_str());
    dlg.ChoiceBaudRate->SetStringSelection(wxString::Format("%d", _baudRate));
    dlg.TextCtrlLastChannel->SetValue(wxString::Format("%d",_channels));
    dlg.TextCtrl_Description->SetValue(_description.c_str());
    dlg.ProtocolChange();

    int res = dlg.ShowModal();

    if (res == wxID_OK)
    {
        _setType = dlg.ChoiceProtocol->GetStringSelection().ToStdString();
        _commPort = dlg.ChoicePort->GetStringSelection().ToStdString();
        _baudRate = wxAtoi(dlg.GetRateString());
        _channels = wxAtoi(dlg.TextCtrlLastChannel->GetValue());
        _description = dlg.TextCtrl_Description->GetValue().ToStdString();
    }

    return res;
}
#endif


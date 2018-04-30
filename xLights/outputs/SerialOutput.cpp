#include "SerialOutput.h"

#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "SerialPortWithRate.h"
#include "LOROutput.h"
#include "LOROptimisedOutput.h"
#include "DLightOutput.h"
#include "RenardOutput.h"
#include "DMXOutput.h"
#include "PixelNetOutput.h"
#include "OpenDMXOutput.h"
#include "OpenPixelNetOutput.h"
#include <wx/msgdlg.h>

#pragma region Constructors and Destructors
SerialOutput::SerialOutput(wxXmlNode* node) : Output(node)
{
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
    SetId(wxAtoi(node->GetAttribute("Id", "0")));
}

SerialOutput::SerialOutput(SerialOutput* output) : Output(output)
{
    _serial = nullptr;
    strcpy(_serialConfig, "8N1");
    _commPort = output->GetCommPort();
    _baudRate = output->GetBaudRate();
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
#pragma endregion Constructors and Destructors

#pragma region Save
void SerialOutput::Save(wxXmlNode* node)
{
    if (_commPort != "")
    {
        node->AddAttribute("ComPort", _commPort.c_str());
    }

    if (_baudRate != 0)
    {
        node->AddAttribute("BaudRate", wxString::Format(wxT("%i"), _baudRate));
    }
    else
    {
        node->AddAttribute("BaudRate", "n/a");
    }

    node->AddAttribute("Id", wxString::Format(wxT("%i"), GetId()));

    Output::Save(node);
}

wxXmlNode* SerialOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}
#pragma endregion Save

#pragma region Getters and Setters
std::string SerialOutput::GetBaudRateString() const
{
    if (_baudRate == 0)
    {
        return "n/a";
    }
    else
    {
        return Output::GetBaudRateString();
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

std::string SerialOutput::GetChannelMapping(long ch) const
{
    std::string res = "Channel " + std::string(wxString::Format(wxT("%ld"), ch)) + " maps to ...\n";

    long channeloffset = ch - GetStartChannel() + 1;

    res += "Type: " + GetType() + "\n";
    res += "ComPort: " + _commPort + "\n";
    res += "Channel: " + std::string(wxString::Format(wxT("%ld"), channeloffset)) + "\n";

    if (!_enabled) res += " INACTIVE";

    return res;
}

std::string SerialOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += GetType() + " " + _commPort;
    res += " [1-" + std::string(wxString::Format(wxT("%ld"), (long)_channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%ld"), (long)GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%li"), (long)GetEndChannel())) + ") ";
    res += _description;

    return res;
}

std::string SerialOutput::GetPingDescription() const
{
    return GetCommPort() + " " + GetDescription();
}
#pragma endregion Getters and Setters

#pragma region Static Functions
std::list<std::string> SerialOutput::GetPossibleSerialPorts()
{
    std::list<std::string> res;

    res.push_back("NotConnected");

#ifdef __WXMSW__
    // Windows
    res.push_back("COM1");
    res.push_back("COM2");
    res.push_back("COM3");
    res.push_back("COM4");
    res.push_back("COM5");
    res.push_back("COM6");
    res.push_back("COM7");
    res.push_back("COM8");
    res.push_back("COM9");
    res.push_back("\\\\.\\COM10");
    res.push_back("\\\\.\\COM11");
    res.push_back("\\\\.\\COM12");
    res.push_back("\\\\.\\COM13");
    res.push_back("\\\\.\\COM14");
    res.push_back("\\\\.\\COM15");
    res.push_back("\\\\.\\COM16");
    res.push_back("\\\\.\\COM17");
    res.push_back("\\\\.\\COM18");
    res.push_back("\\\\.\\COM19");
    res.push_back("\\\\.\\COM20");
#elif __WXOSX__
    // no standard device names for USB-serial converters on OS/X
    // scan /dev directory for candidates
    wxArrayString output, errors;
    wxExecute("ls -1 /dev", output, errors, wxEXEC_SYNC);
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
        for (int i = 0; i<output.Count(); i++)
        {
            if (output[i].StartsWith("cu."))
            {
                res.push_back("/dev/" + output[i].ToStdString());
            }
        }
    }
#else
    // Linux
    res.push_back("/dev/ttyS0");
    res.push_back("/dev/ttyS1");
    res.push_back("/dev/ttyS2");
    res.push_back("/dev/ttyS3");
    res.push_back("/dev/ttyUSB0");
    res.push_back("/dev/ttyUSB1");
    res.push_back("/dev/ttyUSB2");
    res.push_back("/dev/ttyUSB3");
    res.push_back("/dev/ttyUSB4");
    res.push_back("/dev/ttyUSB5");
#endif

    return res;
}

std::list<std::string> SerialOutput::GetPossibleBaudRates()
{
    std::list<std::string> res;

    res.push_back("9600");
    res.push_back("19200");
    res.push_back("38400");
    res.push_back("57600");
    res.push_back("115200");
    res.push_back("128000");
    res.push_back("250000");
    res.push_back("256000");
    res.push_back("500000");
    res.push_back("512000");
    res.push_back("1000000");
    res.push_back("1024000");

    return res;
}

std::list<std::string> SerialOutput::GetAvailableSerialPorts()
{
    std::list<std::string> res;

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
            res.push_back(wxString::Format("%s", portname).ToStdString());
            vallen = sizeof(valname);
            portlen = sizeof(portname);
        }
        if (err && (err != ERROR_NO_MORE_ITEMS))
        {
            res.push_back(wxString::Format("Error %d (can't get serial comm ports from registry)", err).ToStdString());
        }
        if (hkey) RegCloseKey(hkey);
    }
    else
    {
        res.push_back("(no available ports)");
    }
#elif __WXOSX__
    // no standard device names for USB-serial converters on OS/X
    // scan /dev directory for candidates
    wxArrayString output, errors;
    wxExecute("ls -1 /dev", output, errors, wxEXEC_SYNC);
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
        for (int i = 0; i<output.Count(); i++)
        {
            if (output[i].StartsWith("cu."))
            {
                res.push_back("/dev/" + output[i].ToStdString());
            }
        }
    }
#else
    res.push_back("port enumeration not supported on Linux");
#endif // __WXMSW__

    return res;
}
#pragma endregion Static Functions

#pragma region Start and Stop
bool SerialOutput::Open()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _ok = Output::Open();

    if (_commPort == "")
    {
        return false;
    }

    if (_commPort == "NotConnected")
    {
        logger_base.warn("Serial port %s for %s not opened as it is tagged as not connected '%s'.", (const char *)_commPort.c_str(), (const char *)GetType().c_str(), (const char *)_description.c_str());
        // dont set ok to false ... while this is not really open it is not an error as the user meant it to be not connected.
    }
    else
    {
        _serial = new SerialPort();

        logger_base.debug("Opening serial port %s. Baud rate = %d. Config = %s.", (const char *)_commPort.c_str(), _baudRate, (const char *)_serialConfig);

        int errcode = _serial->Open(_commPort, _baudRate, _serialConfig);
        if (errcode < 0)
        {
            logger_base.warn("Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);
            _ok = false;

            std::string p = "";
            auto ports = GetAvailableSerialPorts();
            for (auto it = ports.begin(); it != ports.end(); ++it)
            {
                if (p != "") p += ", ";
                p += *it;
            }

            wxString msg = wxString::Format(_("Error occurred while connecting to %s network on %s (Available Ports %s) \n\n") +
                _("Things to check:\n") +
                _("1. Are all required cables plugged in?\n") +
                _("2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.\n") +
                _("3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n") +
                _("Unable to open serial port %s. Error code = %d"),
                (const char *)GetType().c_str(),
                (const char *)GetCommPort().c_str(),
                (const char *)p.c_str(),
                (const char *)_commPort.c_str(),
                errcode);
            wxMessageBox(msg, _("Communication Error"), wxOK);
        }
        else
        {
            logger_base.debug("    Serial port %s open.", (const char *)_commPort.c_str());
        }
    }

    return _ok;
}

void SerialOutput::Close()
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_serial != nullptr)
    {
        // throw away any pending data
        _serial->Purge();

        int i = 0;
        while( !TxEmpty() && (i < 10) )
        {
            wxMilliSleep(5);
            i++;
        }
        _serial->Close();
        delete _serial;
        _serial = nullptr;
        logger_base.debug("    Serial port %s closed.", (const char *)_commPort.c_str());
    }
}
#pragma endregion Start and Stop

#pragma region Operator
bool SerialOutput::operator==(const SerialOutput& output) const
{
    if (GetType() != output.GetType()) return false;

    return _commPort == output.GetCommPort();
}
#pragma endregion Operator

SerialOutput* SerialOutput::Mutate(const std::string& newtype)
{
    if (newtype == OUTPUT_DMX)
    {
        return new DMXOutput(this);
    }
    else if (newtype == OUTPUT_PIXELNET)
    {
        return new PixelNetOutput(this);
    }
    else if (newtype == OUTPUT_LOR)
    {
        return new LOROutput(this);
    }
    else if (newtype == OUTPUT_LOR_OPT)
    {
        return new LOROptimisedOutput(this);
    }
    else if (newtype == OUTPUT_DLIGHT)
    {
        return new DLightOutput(this);
    }
    else if (newtype == OUTPUT_RENARD)
    {
        return new RenardOutput(this);
    }
    else if (newtype == OUTPUT_OPENDMX)
    {
        return new OpenDMXOutput(this);
    }
    else if (newtype == OUTPUT_OPENPIXELNET)
    {
        return new OpenPixelNetOutput(this);
    }


    return nullptr;
}

PINGSTATE SerialOutput::Ping() const
{
    if (_serial != nullptr && _ok)
    {
        return PINGSTATE::PING_OPEN;
    }
    else
    {
        PINGSTATE res = PINGSTATE::PING_ALLFAILED;
        SerialPort* serial = new SerialPort();

        int errcode = serial->Open(_commPort, _baudRate, _serialConfig);
        if (errcode >= 0)
        {
            res = PINGSTATE::PING_OPENED;
            serial->Close();
        }

        delete serial;
        return res;
    }

    return PINGSTATE::PING_ALLFAILED;
}

#pragma region UI
#ifndef EXCLUDENETWORKUI
// This is a bit funky as we will need to create a serial output then mutate it into the correct output type
Output* SerialOutput::Configure(wxWindow* parent, OutputManager* outputManager)
{
    SerialOutput* result = this;

    SerialPortWithRate dlg(parent, &result, outputManager);

    int res = dlg.ShowModal();

    if (res == wxID_CANCEL)
    {
        return nullptr;
    }

    return result;
}
#endif
#pragma endregion UI


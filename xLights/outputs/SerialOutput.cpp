
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/msgdlg.h>

#include "SerialOutput.h"
#include "LOROutput.h"
#include "LOROptimisedOutput.h"
#include "DLightOutput.h"
#include "RenardOutput.h"
#include "DMXOutput.h"
#include "xxxSerialOutput.h"
#include "PixelNetOutput.h"
#include "OpenDMXOutput.h"
#include "OpenPixelNetOutput.h"
#include "GenericSerialOutput.h"
#include "OutputManager.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

#pragma region Private Functions
void SerialOutput::Save(wxXmlNode* node) {

    if (_commPort != "") {
        node->AddAttribute("ComPort", _commPort.c_str());
    }

    if (_baudRate != 0) {
        node->AddAttribute("BaudRate", wxString::Format(wxT("%i"), _baudRate));
    }
    else {
        node->AddAttribute("BaudRate", "n/a");
    }

    Output::Save(node);
}
#pragma endregion

#pragma region Constructors and Destructors
SerialOutput::SerialOutput(wxXmlNode* node) : Output(node) {
    
    strcpy(_serialConfig, "8N1");
    _commPort = node->GetAttribute("ComPort", "").ToStdString();
    if (node->GetAttribute("BaudRate", "n/a") == "n/a") {
        _baudRate = 0;
    }
    else {
        _baudRate = wxAtoi(node->GetAttribute("BaudRate", ""));
    }
    SetId(wxAtoi(node->GetAttribute("Id", "0")));
}

SerialOutput::SerialOutput(const SerialOutput& from) :
    Output(from)
{
    strcpy(_serialConfig, "8N1");
    _commPort = from.GetCommPort();
    _baudRate = from.GetBaudRate();
}

SerialOutput::SerialOutput() : Output() {

    strcpy(_serialConfig, "8N1");
    _commPort = GetPossibleSerialPorts().front();
    _baudRate = GetDefaultBaudRate();
}

SerialOutput::~SerialOutput() {

    if (_serial != nullptr) delete _serial;
}

wxXmlNode* SerialOutput::Save() {

    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}
#pragma endregion 

#pragma region static Functions
std::list<std::string> SerialOutput::GetPossibleSerialPorts() {

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
    if (!errors.IsEmpty()) {
        DisplayError(errors.Last());
    }
    else if (output.IsEmpty()) {
        DisplayError(_("no devices found"));
    }
    else {
        for (int i = 0; i < output.Count(); i++) {
            if (output[i].StartsWith("cu.")) {
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
    res.push_back("/dev/ttyACM0");
    res.push_back("/dev/ttyACM1");
    res.push_back("/dev/ttyACM2");
    res.push_back("/dev/ttyACM3");
    res.push_back("/dev/ttyACM4");
    res.push_back("/dev/ttyACM5");
#endif

    return res;
}

std::list<std::string> SerialOutput::GetPossibleBaudRates() {

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

std::list<std::string> SerialOutput::GetAvailableSerialPorts() {

    std::list<std::string> res;

#ifdef __WXMSW__
    TCHAR valname[_MAX_PATH];
    TCHAR portname[_MAX_PATH];
    DWORD vallen = sizeof(valname);
    DWORD portlen = sizeof(portname);
    HKEY hkey = nullptr;
    DWORD err = 0;

    //enum serial comm ports (more user friendly, especially if USB-to-serial ports change):
    //logic based on http://www.cplusplus.com/forum/windows/73821/
    if (!((err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hkey)))) {
        for (DWORD inx = 0; !((err = RegEnumValue(hkey, inx, (LPTSTR)valname, &vallen, nullptr, nullptr, (LPBYTE)portname, &portlen))) || (err == ERROR_MORE_DATA); ++inx)
        {
            if (err == ERROR_MORE_DATA) {
                portname[sizeof(portname) - 1] = '\0';
            }
            //need to enlarge read buf if this happens; just truncate string for now
            //                            debug(3, "found port[%d] %d:'%s' = %d:'%s', err 0x%x", inx, vallen, valname, portlen, portname, err);
            res.push_back(wxString::Format("%s", portname).ToStdString());
            vallen = sizeof(valname);
            portlen = sizeof(portname);
        }
        if (err && (err != ERROR_NO_MORE_ITEMS)) {
            res.push_back(wxString::Format("Error %d (can't get serial comm ports from registry)", err).ToStdString());
        }
        if (hkey) RegCloseKey(hkey);
    }
    else {
        res.push_back("(no available ports)");
    }
#elif __WXOSX__
    // no standard device names for USB-serial converters on OS/X
    // scan /dev directory for candidates
    wxArrayString output, errors;
    wxExecute("ls -1 /dev", output, errors, wxEXEC_SYNC);
    if (!errors.IsEmpty()) {
        DisplayError(errors.Last());
    } else if (output.IsEmpty()) {
        res.push_back("(no available ports)");
        DisplayError(_("no devices found"));
    } else {
        for (int i = 0; i < output.Count(); i++) {
            if (output[i].StartsWith("cu.") && !output[i].StartsWith("cu.B")) {
                res.push_back("/dev/" + output[i].ToStdString());
            }
        }
    }
#else
    res.push_back("port enumeration not supported on Linux");
#endif // __WXMSW__

    return res;
}
#pragma endregion 

#pragma region Getters and Setters
std::string SerialOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += GetType();
    res += " [1-" + std::string(wxString::Format(wxT("%d"), _channels)) + "] ";
    res += "(" + std::string(wxString::Format(wxT("%d"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%i"), GetEndChannel())) + ") ";

    return res;
}

size_t SerialOutput::TxNonEmptyCount() const {
    return (_serial != nullptr) ? _serial->WaitingToWrite() : 0;
}

bool SerialOutput::TxEmpty() const {
    if (_serial != nullptr) return (_serial->WaitingToWrite() == 0);
    return true;
}

Output::PINGSTATE SerialOutput::Ping() const {

    if (_serial != nullptr && _ok) {
        return Output::PINGSTATE::PING_OPEN;
    }
    else {

        Output::PINGSTATE res = Output::PINGSTATE::PING_ALLFAILED;
        
        SerialPort* serial = new SerialPort();
        int errcode = serial->Open(_commPort, _baudRate, _serialConfig);
        if (errcode >= 0) {
            res = Output::PINGSTATE::PING_OPENED;
            serial->Close();
        }

        delete serial;
        return res;
    }
}
#pragma endregion

#pragma region Operators
bool SerialOutput::operator==(const SerialOutput& output) const {

    if (GetType() != output.GetType()) return false;

    return _commPort == output.GetCommPort();
}
#pragma endregion

#pragma region Start and Stop
bool SerialOutput::Open() {

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _ok = Output::Open();

    if (_commPort == "") {
        return false;
    }

    if (_commPort == "NotConnected") {

        logger_base.warn("Serial port %s for %s not opened as it is tagged as not connected.", (const char *)_commPort.c_str(), (const char *)GetType().c_str());
        // dont set ok to false ... while this is not really open it is not an error as the user meant it to be not connected.
    }
    else {
        _serial = new SerialPort();

        logger_base.debug("Opening serial port %s. Baud rate = %d. Config = %s.", (const char *)_commPort.c_str(), _baudRate, (const char *)_serialConfig);

        int errcode = _serial->Open(_commPort, _baudRate, _serialConfig);
        if (errcode < 0) {
            delete _serial;
            _serial = nullptr;

            logger_base.warn("Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);
            _ok = false;

            std::string p = "";
            auto ports = GetAvailableSerialPorts();
            for (const auto& it : ports)
            {
                if (p != "") p += ", ";
                p += it;
            }

            if (OutputManager::IsInteractive()) {
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
                DisplayError(msg);
            }
        }
        else {
            logger_base.debug("    Serial port %s open.", (const char *)_commPort.c_str());
        }
    }

    return _ok;
}

void SerialOutput::Close() {

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_serial != nullptr) {
        // throw away any pending data
        _serial->Purge();

        // wait until the die time has passed
        while (wxGetUTCTimeMillis() < _dieTime) {
            wxMilliSleep(5);
        }

        // wait until pending data to send is sent
        int i = 0;
        while( !TxEmpty() && (i < 200) ) {
            wxMilliSleep(5);
            i++;
        }

        _serial->Close();
        delete _serial;
        _serial = nullptr;
        logger_base.debug("    Serial port %s closed in %d milliseconds.", (const char *)_commPort.c_str(), i * 5);
    }
}
#pragma endregion

#pragma region Frame Handling
void SerialOutput::StartFrame(long msec) {

    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_enabled) return;

    if (!_ok && OutputManager::IsRetryOpen()) {
        _ok = SerialOutput::Open();
        if (_ok) {
            logger_base.debug("SerialOutput: Open retry successful. %s.", (const char *)_commPort.c_str());
        }
    }

    _timer_msec = msec;
}
#pragma endregion

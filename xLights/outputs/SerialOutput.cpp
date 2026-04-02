/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <chrono>
#include <filesystem>
#include <thread>

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
#include "UtilFunctions.h"
#include "../ui/wxUtilities.h"

#include "serial.h"

#include <log.h>

#pragma region Private Functions
void SerialOutput::SaveAttr(pugi::xml_node node) {

    if (_commPort != "") {
        node.append_attribute("ComPort") = _commPort;
    }

    if (_baudRate != 0) {
        node.append_attribute("BaudRate") = _baudRate;
    }
    else {
        node.append_attribute("BaudRate") = "n/a";
    }

    Output::SaveAttr(node);
}
#pragma endregion

#pragma region Constructors and Destructors
SerialOutput::SerialOutput(pugi::xml_node node) : Output(node) {

    strcpy(_serialConfig, "8N1");
    _commPort = node.attribute("ComPort").as_string("");
    if (std::string_view(node.attribute("BaudRate").as_string("n/a")) == "n/a") {
        _baudRate = 0;
    }
    else {
        _baudRate = node.attribute("BaudRate").as_int(0);
    }
    SetId(node.attribute("Id").as_int(0));
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

pugi::xml_node SerialOutput::Save(pugi::xml_node parent) {

    pugi::xml_node node = parent.append_child("network");
    SaveAttr(node);

    return node;
}
#pragma endregion

#pragma region static Functions
std::list<std::string> SerialOutput::GetPossibleSerialPorts() {

    std::list<std::string> res;

    res.push_back("NotConnected");
#ifdef _WIN32
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
#elif defined(__APPLE__)
    // no standard device names for USB-serial converters on OS/X
    // scan /dev directory for candidates
    {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator("/dev", ec)) {
            auto name = entry.path().filename().string();
            if (name.starts_with("cu.")) {
                res.push_back("/dev/" + name);
            }
        }
        if (ec) {
            DisplayError("no devices found");
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

#ifdef _WIN32
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
            res.push_back(std::string(portname, portname + portlen / sizeof(TCHAR) - 1));
            vallen = sizeof(valname);
            portlen = sizeof(portname);
        }
        if (err && (err != ERROR_NO_MORE_ITEMS)) {
            res.push_back(std::format("Error {} (can't get serial comm ports from registry)", err));
        }
        if (hkey) RegCloseKey(hkey);
    }
    else {
        res.push_back("(no available ports)");
    }
#elif defined(__APPLE__)
    // no standard device names for USB-serial converters on OS/X
    // scan /dev directory for candidates
    {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator("/dev", ec)) {
            auto name = entry.path().filename().string();
            if (name.starts_with("cu.") && !name.starts_with("cu.B")) {
                res.push_back("/dev/" + name);
            }
        }
        if (ec) {
            res.push_back("(no available ports)");
            DisplayError("no devices found");
        }
    }
#else
    res.push_back("port enumeration not supported on Linux");
#endif // _WIN32

    return res;
}
#pragma endregion 

#pragma region Getters and Setters
std::string SerialOutput::GetLongDescription() const {

    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += GetType();
    res += " [1-" + std::to_string(_channels) + "] ";
    res += "(" + std::to_string(GetStartChannel()) + "-" + std::to_string(GetEndChannel()) + ") ";

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

    

    _ok = Output::Open();

    if (_commPort == "") {
        return false;
    }

    if (_commPort == "NotConnected") {

        spdlog::warn("Serial port {} for {} not opened as it is tagged as not connected.", _commPort, GetType());
        // dont set ok to false ... while this is not really open it is not an error as the user meant it to be not connected.
    }
    else {
        _serial = new SerialPort();

        spdlog::debug("Opening serial port {}. Baud rate = {}. Config = {}.", _commPort, _baudRate, _serialConfig);

        int errcode = _serial->Open(_commPort, _baudRate, _serialConfig);
        if (errcode < 0) {
            delete _serial;
            _serial = nullptr;

            spdlog::warn("Unable to open serial port {}. Error code = {}", _commPort, errcode);
            _ok = false;

            std::string p = "";
            auto ports = GetAvailableSerialPorts();
            for (const auto& it : ports)
            {
                if (p != "") p += ", ";
                p += it;
            }

            if (OutputManager::IsInteractive()) {
                std::string msg = std::format(
                    "Error occurred while connecting to {} network on {} (Available Ports {}) \n\n"
                    "Things to check:\n"
                    "1. Are all required cables plugged in?\n"
                    "2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.\n"
                    "3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n"
                    "Unable to open serial port {}. Error code = {}",
                    GetType(),
                    GetCommPort(),
                    p,
                    _commPort,
                    errcode);
                DisplayError(msg);
            }
        }
        else {
            spdlog::debug("    Serial port {} open.", _commPort);
        }
    }

    return _ok;
}

void SerialOutput::Close() {
    if (_serial != nullptr) {
        // throw away any pending data
        _serial->Purge();

        // wait until the die time has passed
        while (GetCurrentTimeMillis() < _dieTime) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        // wait until pending data to send is sent
        int i = 0;
        while( !TxEmpty() && (i < 200) ) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            i++;
        }

        _serial->Close();
        delete _serial;
        _serial = nullptr;
        spdlog::debug("    Serial port {} closed in {} milliseconds.", _commPort, i * 5);
    }
}
#pragma endregion

#pragma region Frame Handling
void SerialOutput::StartFrame(long msec) {

    

    if (!_enabled) return;

    if (!_ok && OutputManager::IsRetryOpen()) {
        _ok = SerialOutput::Open();
        if (_ok) {
            spdlog::debug("SerialOutput: Open retry successful. {}.", _commPort);
        }
    }

    _timer_msec = msec;
}
#pragma endregion

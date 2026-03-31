/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemProjector.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemProjectorPanel.h"
#include "../../xLights/utils/AudioManager.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "../md5.h"
#include <log.h>
#include "../../xLights/outputs/SerialOutput.h"
#include "../../xLights/utils/UtilFunctions.h"
#include "../../xLights/ui/wxUtilities.h"

std::string PlayListItemProjector::BuildSerialConfiguration()
{
    return wxString::Format("%d%s%d", _charBits, _parity == "NONE" ? "N" : (_parity == "EVEN" ? "E" : "O"), _stopBits).ToStdString();
}

unsigned char* PlayListItemProjector::PrepareData(const std::string s, int& used)
{
    wxString working(s);

    unsigned char* buffer = (unsigned char*)malloc(working.size());
    used = 0;

    for (int i = 0; i < (int)working.size(); i++)
    {
        if (working[i] == '\\')
        {
            if (i + 1 < (int)working.size())
            {
                if (working[i + 1] == '\\')
                {
                    buffer[used++] = working[i];
                    ++i; // skip the second '\\'
                }
                if (working[i + 1] == 'x' || working[i + 1] == 'X')
                {
                    // up to next 2 characters if 0-F will be treated as a hex code
                    ++i;
                    ++i;
                    if (i + 1 < (int)working.size() && isHexChar(working[i]) && isHexChar(working[i + 1]))
                    {
                        buffer[used++] = (char)HexToChar(working[i], working[i + 1]);
                        ++i;
                    }
                    else if (i < (int)working.size() && isHexChar(working[i]))
                    {
                        buffer[used++] = (char)HexToChar(working[i]);
                    }
                    else
                    {
                        // \x was not followed by a hex digit so put in \x
                        buffer[used++] = '\\';
                        buffer[used++] = 'x';
                        --i;
                    }
                }
            }
            else
            {
                buffer[used++] = working[i];
            }
        }
        else
        {
            buffer[used] = working[i];
        }
    }

    unsigned char* res = (unsigned char*)malloc(used);
    memcpy(res, buffer, used);
    free(buffer);
    return res;
}

void PlayListItemProjector::ExecuteSerialCommand()
{
    int dbuffsize;
    unsigned char* dbuffer = PrepareData(_commandData, dbuffsize);

    if (dbuffer != nullptr)
    {
        spdlog::info("Sending serial {}.", _commandData);

        if (_commPort == "NotConnected")
        {
            spdlog::warn("Serial port {} not opened for {} as it is tagged as not connected.", _commPort, GetNameNoTime());
            // dont set ok to false ... while this is not really open it is not an error as the user meant it to be not connected.
        }
        else
        {
            SerialPort* serial = new SerialPort();

            std::string configuration = BuildSerialConfiguration();

            spdlog::debug("Opening serial port {}. Baud rate = {}. Config = {}.", _commPort, _baudRate, configuration);

            int errcode = serial->Open(_commPort, _baudRate, configuration.c_str());
            if (errcode < 0)
            {
                delete serial;
                serial = nullptr;

                spdlog::warn("Unable to open serial port {}. Error code = {}", _commPort, errcode);

                std::string p = "";
                auto ports = SerialOutput::GetAvailableSerialPorts();
                for (auto it = ports.begin(); it != ports.end(); ++it)
                {
                    if (p != "") p += ", ";
                    p += *it;
                }

                [[maybe_unused]] wxString msg = wxString::Format(_("Error occurred while connecting to %s (Available Ports %s) \n\n") +
                    _("Things to check:\n") +
                    _("1. Are all required cables plugged in?\n") +
                    _("2. Is there another program running that is accessing the port (like the LOR Control Panel)? If so, then you must close the other program and then restart xLights.\n") +
                    _("3. If this is a USB dongle, are the FTDI Virtual COM Port drivers loaded?\n\n") +
                    _("Unable to open serial port %s. Error code = %d"),
                    (const char *)GetCommPort().c_str(),
                    (const char *)p.c_str(),
                    (const char *)_commPort.c_str(),
                    errcode);
                //wxMessageBox(msg, _("Communication Error"), wxOK);
            }
            else
            {
                spdlog::debug("    Serial port {} open.", _commPort);
            }

            if (serial != nullptr && serial->IsOpen())
            {
                serial->Write((char *)dbuffer, dbuffsize);

                int i = 0;
                while (serial->WaitingToWrite() != 0 && (i < 10))
                {
                    wxMilliSleep(5);
                    ++i;
                }
                serial->Close();
                delete serial;
                serial = nullptr;
                spdlog::debug("    Serial port {} closed.", _commPort);
            }
        }

        delete dbuffer;
    }
}

PlayListItemProjector::PlayListItemProjector(wxXmlNode* node) : PlayListItem(node)
{
    _socket = nullptr;
    _command = "ON";
    _commandData = "set_power_state 1";
    _parity = "none";
    _projectorProtocol = "PJLINK";
    _ipProtocol = "PJLINK";
    _ipAddress = "";
    _password = "";
    _commPort = "COMM1";
    _port = 0;
    _baudRate = 19200;
    _charBits = 8;
    _stopBits = 1;
    _parameter = "1";

    _hash = "";
    _started = false;

    PlayListItemProjector::Load(node);
}

void PlayListItemProjector::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _command = node->GetAttribute("Command", "");
    _commandData = node->GetAttribute("CommandData", "");
    _parity = node->GetAttribute("Parity", "none");
    _projectorProtocol = node->GetAttribute("Protocol", "PJLINK");
    _ipProtocol = node->GetAttribute("IPProtocol", "PJLINK");
    _ipAddress = node->GetAttribute("IPAddress", "");
    _parameter = node->GetAttribute("Parameter", "");
    _password = node->GetAttribute("Password", "");
    _commPort = node->GetAttribute("CommPort", "COMM1");
    _port = wxAtoi(node->GetAttribute("Port", "0"));
    _baudRate = wxAtoi(node->GetAttribute("BaudRate", "19200"));
    _charBits = wxAtoi(node->GetAttribute("CharBits", "8"));
    _stopBits = wxAtoi(node->GetAttribute("StopBits", "1"));
}

PlayListItemProjector::PlayListItemProjector() : PlayListItem()
{
    _type = "PLIProjector";
    _command = "ON";
    _commandData = "set_power_state 1";
    _parity = "none";
    _projectorProtocol = "PJLINK";
    _ipProtocol = "PJLINK";
    _ipAddress = "";
    _password = "";
    _commPort = "COMM1";
    _port = 0;
    _baudRate = 19200;
    _charBits = 8;
    _stopBits = 1;
    _hash = "";
    _parameter = "1";
    _started = false;
    _socket = nullptr;
}

PlayListItem* PlayListItemProjector::Copy(const bool isClone) const
{
    PlayListItemProjector* res = new PlayListItemProjector();
    res->_command = _command;
    res->_commandData = _commandData;
    res->_parity = _parity;
    res->_projectorProtocol = _projectorProtocol;
    res->_ipProtocol = _ipProtocol;
    res->_ipAddress = _ipAddress;
    res->_password = _password;
    res->_commPort = _commPort;
    res->_port = _port;
    res->_baudRate = _baudRate;
    res->_charBits = _charBits;
    res->_stopBits = _stopBits;
    res->_parameter = _parameter;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemProjector::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Command", _command);
    node->AddAttribute("CommandData", _commandData);
    node->AddAttribute("Parity", _parity);
    node->AddAttribute("Protocol", _projectorProtocol);
    node->AddAttribute("IPProtocol", _ipProtocol);
    node->AddAttribute("IPAddress", _ipAddress);
    node->AddAttribute("Password", _password);
    node->AddAttribute("CommPort", _commPort);
    node->AddAttribute("Parameter", _parameter);
    node->AddAttribute("Port", wxString::Format("%d", _port));
    node->AddAttribute("BaudRate", wxString::Format("%d", _baudRate));
    node->AddAttribute("CharBits", wxString::Format("%d", _charBits));
    node->AddAttribute("StopBits", wxString::Format("%d", _stopBits));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemProjector::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemProjectorPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemProjector::GetTitle() const
{
    return "Projector";
}

std::string PlayListItemProjector::GetNameNoTime() const
{
    if (_command == "")
    {
        return "Projector";
    }
    else
    {
        return _command;
    }
}

void PlayListItemProjector::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        spdlog::debug("Projector executing command {}", _command);

        _started = true;
        if (_ipProtocol == "PJLINK")
        {
            ExecutePJLinkCommand();
        }
        else if (_ipProtocol == "TCP")
        {
            ExecuteTCPCommand();
        }
        else if (_ipProtocol == "SERIAL")
        {
            ExecuteSerialCommand();
        }
    }
}

void PlayListItemProjector::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}

void PlayListItemProjector::ExecutePJLinkCommand()
{
    PJLinkLogin();
    if (_command == "ON")
    {
        SendPJLinkCommand("%1POWR 1\r");
    }
    else if (_command == "OFF")
    {
        SendPJLinkCommand("%1POWR 0\r");
    }
    else if (_command == "RGB")
    {
        SendPJLinkCommand("%1INPT 1" + _parameter+"\r");
    }
    else if (_command == "VIDEO")
    {
        SendPJLinkCommand("%1INPT 2" + _parameter + "\r");
    }
    else if (_command == "DIGITAL")
    {
        SendPJLinkCommand("%1INPT 3" + _parameter + "\r");
    }
    else if (_command == "Change Input to STORAGE")
    {
        SendPJLinkCommand("%1INPT 4" + _parameter + "\r");
    }
    else if (_command == "NETWORK")
    {
        SendPJLinkCommand("%1INPT 5" + _parameter + "\r");
    }
    else if (_command == "VIDEO MUTE ON")
    {
        SendPJLinkCommand("%1AVMT 11\r");
    }
    else if (_command == "VIDEO MUTE OFF")
    {
        SendPJLinkCommand("%1AVMT 10\r");
    }
    else if (_command == "AUDIO MUTE ON")
    {
        SendPJLinkCommand("%1AVMT 21\r");
    }
    else if (_command == "AUDIO MUTE OFF")
    {
        SendPJLinkCommand("%1AVMT 20\r");
    }
    PJLinkLogout();
}

void PlayListItemProjector::ExecuteTCPCommand()
{
    int dbuffsize;
    unsigned char* dbuffer = PrepareData(_commandData, dbuffsize);

    if (dbuffer != nullptr)
    {
        spdlog::info("Sending TCP {}:{} {}.", _ipAddress, _port, _commandData);
        wxIPV4address address;
        address.Hostname(_ipAddress);
        address.Service(_port);
        _socket = new wxSocketClient();
        if (_socket != nullptr)
        {
            // wait for up to 1/2 second for connection
            if (_socket->Connect(address, false) || _socket->WaitOnConnect(0, 500))
            {
                spdlog::info("Projector connected.");

                _socket->Write(dbuffer, dbuffsize);

                _socket->WaitForRead(0, 500);

                wxCharBuffer buffer(100);
                _socket->Peek(buffer.data(), buffer.length());

                // wait for up to 100ms for data
                int i = 0;
                while (_socket->LastCount() == 0 && i < 100)
                {
                    wxMilliSleep(1);
                    _socket->Peek(buffer.data(), buffer.length());
                    ++i;
                }

                _socket->ReadMsg(buffer.data(), buffer.length());
                int read = _socket->GetLastIOReadSize();
                wxString response(buffer);
                spdlog::info("Projector response '{}' len: {}.", response.ToStdString(), read);
            }
            else
            {
                spdlog::warn("Projector unable to connect.");
            }
        }
        else
        {
            spdlog::warn("Projector unable to connect.");
        }
    }
    else
    {
        spdlog::warn("Projector unable to prepare data.");
    }
}

bool PlayListItemProjector::PJLinkLogin()
{
    PJLinkLogout();

    _hash = "";

    // ScheduleOptions* options = xScheduleFrame::GetScheduleManager()->GetOptions();

    std::string ip = _ipAddress;
    std::string password = _password;

    if (ip == "")
    {
        spdlog::error("Projector tried to log into projector with no IP address.");
        return false;
    }

    spdlog::info("Projector logging into {}.", ip);

    wxIPV4address address;
    address.Hostname(ip);
    address.Service(4352);
    _socket = new wxSocketClient();
    if (_socket != nullptr)
    {
        // wait for up to 1/2 second for connection
        if (_socket->Connect(address, false) || _socket->WaitOnConnect(0, 500))
        {
            spdlog::info("Projector connected.");

            _socket->WaitForRead(0, 500);

            wxCharBuffer buffer(100);
            _socket->Peek(buffer.data(), buffer.length());

            // wait for up to 100ms for data
            int i = 0;
            while (_socket->LastCount() == 0 && i < 100)
            {
                wxMilliSleep(1);
                _socket->Peek(buffer.data(), buffer.length());
                ++i;
            }

            _socket->ReadMsg(buffer.data(), buffer.length());
            // int read = _socket->GetLastIOReadSize();

            wxString response(buffer);
            spdlog::info("Projector response '{}'", response.ToStdString());

            if  (response.Left(8) == "Projector 0")
            {
                // no security
                spdlog::info("Projector No Security.");
                return true;
            }
            else if (response.Left(8) == "Projector 1")
            {
                wxString random = response.SubString(9, response.size());

                if (random.Contains("\r"))
                {
                    random = random.Left(random.Find('\r'));
                }
                spdlog::info("Projector random '{}'.", random.ToStdString());

                random = random + password;

                _hash = md5(random.ToStdString());

                spdlog::info("Projector session hash '{}'.", _hash);
            }
        }
        else
        {
            spdlog::error("Projector refused connection.");
            delete _socket;
            _socket = nullptr;
        }
    }
    else
    {
        spdlog::error("Projector refused connection {}:4352.", ip);
    }

    return _socket != nullptr;
}

bool PlayListItemProjector::SendPJLinkCommand(const std::string& command)
{
    if (_socket == nullptr) return false;

    std::string cmd = _hash + command;

    spdlog::info("Projector sending command {}.", cmd);

    _socket->Write(cmd.c_str(), cmd.size());

    _socket->WaitForRead(0, 500);

    wxCharBuffer buffer(100);
    _socket->Peek(buffer.data(), buffer.length());

    // wait for up to 100ms for data
    int i = 0;
    while (_socket->LastCount() == 0 && i < 100)
    {
        wxMilliSleep(1);
        _socket->Peek(buffer.data(), buffer.length());
        ++i;
    }

    _socket->ReadMsg(buffer.data(), buffer.length());
    int read = _socket->GetLastIOReadSize();
    wxString response(buffer);
    spdlog::info("Projector response '{}' len: {}.", response.ToStdString(), read);

    return true;
}

void PlayListItemProjector::PJLinkLogout()
{
    if (_socket == nullptr) return;

    spdlog::info("Projector logging out.");

    if (_socket != nullptr)
    {
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _hash = "";
}

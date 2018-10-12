#include "PlayListItemProjector.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemProjectorPanel.h"
#include "../../xLights/AudioManager.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "../md5.h"
#include <log4cpp/Category.hh>
#include "../../xLights/outputs/SerialOutput.h"
#include "../../xLights/UtilFunctions.h"

std::string PlayListItemProjector::BuildSerialConfiguration()
{
    return wxString::Format("%d%s%d", _charBits, _parity == "NONE" ? "N" : (_parity == "EVEN" ? "E" : "O"), _stopBits).ToStdString();
}

unsigned char* PlayListItemProjector::PrepareData(const std::string s, int& used)
{
    wxString working(s);

    unsigned char* buffer = (unsigned char*)malloc(working.size());
    used = 0;

    for (int i = 0; i < working.size(); i++)
    {
        if (working[i] == '\\')
        {
            if (i + 1 < working.size())
            {
                if (working[i + 1] == '\\')
                {
                    buffer[used++] = working[i];
                    i++; // skip the second '\\'
                }
                if (working[i + 1] == 'x' || working[i + 1] == 'X')
                {
                    // up to next 2 characters if 0-F will be treated as a hex code
                    i++;
                    i++;
                    if (i + 1 < working.size() && isHexChar(working[i]) && isHexChar(working[i + 1]))
                    {
                        buffer[used++] = (char)HexToChar(working[i], working[i + 1]);
                        i++;
                    }
                    else if (i < working.size() && isHexChar(working[i]))
                    {
                        buffer[used++] = (char)HexToChar(working[i]);
                    }
                    else
                    {
                        // \x was not followed by a hex digit so put in \x
                        buffer[used++] = '\\';
                        buffer[used++] = 'x';
                        i--;
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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int dbuffsize;
    unsigned char* dbuffer = PrepareData(_commandData, dbuffsize);

    if (dbuffer != nullptr)
    {
        logger_base.info("Sending serial %s.", (const char *)_commandData.c_str());

        if (_commPort == "NotConnected")
        {
            logger_base.warn("Serial port %s not opened for %s as it is tagged as not connected.", (const char *)_commPort.c_str(), (const char *)GetNameNoTime().c_str());
            // dont set ok to false ... while this is not really open it is not an error as the user meant it to be not connected.
        }
        else
        {
            SerialPort* serial = new SerialPort();

            std::string configuration = BuildSerialConfiguration();

            logger_base.debug("Opening serial port %s. Baud rate = %d. Config = %s.", (const char *)_commPort.c_str(), _baudRate, (const char *)configuration.c_str());

            int errcode = serial->Open(_commPort, _baudRate, configuration.c_str());
            if (errcode < 0)
            {
                delete serial;
                serial = nullptr;

                logger_base.warn("Unable to open serial port %s. Error code = %d", (const char *)_commPort.c_str(), errcode);

                std::string p = "";
                auto ports = SerialOutput::GetAvailableSerialPorts();
                for (auto it = ports.begin(); it != ports.end(); ++it)
                {
                    if (p != "") p += ", ";
                    p += *it;
                }

                wxString msg = wxString::Format(_("Error occurred while connecting to %s (Available Ports %s) \n\n") +
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
                logger_base.debug("    Serial port %s open.", (const char *)_commPort.c_str());
            }

            if (serial != nullptr && serial->IsOpen())
            {
                serial->Write((char *)dbuffer, dbuffsize);

                int i = 0;
                while (serial->WaitingToWrite() != 0 && (i < 10))
                {
                    wxMilliSleep(5);
                    i++;
                }
                serial->Close();
                delete serial;
                serial = nullptr;
                logger_base.debug("    Serial port %s closed.", (const char *)_commPort.c_str());
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

PlayListItem* PlayListItemProjector::Copy() const
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
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemProjector::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIProjector");

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

void PlayListItemProjector::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("Projector executing command %s", (const char *)_command.c_str());

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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int dbuffsize;
    unsigned char* dbuffer = PrepareData(_commandData, dbuffsize);

    if (dbuffer != nullptr)
    {
        logger_base.info("Sending TCP %s:%d %s.", (const char*)_ipAddress.c_str(), _port, (const char *)_commandData.c_str());
        wxIPV4address address;
        address.Hostname(_ipAddress);
        address.Service(_port);
        _socket = new wxSocketClient();
        if (_socket != nullptr)
        {
            // wait for up to 1/2 second for connection
            if (_socket->Connect(address, false) || _socket->WaitOnConnect(0, 500))
            {
                logger_base.info("Projector connected.");

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
                    i++;
                }

                _socket->ReadMsg(buffer.data(), buffer.length());
                int read = _socket->GetLastIOReadSize();
                wxString response(buffer);
                logger_base.info("Projector response '%s' len: %d.", (const char*)response.c_str(), read);
            }
            else
            {
                logger_base.warn("Projector unable to connect.");
            }
        }
        else
        {
            logger_base.warn("Projector unable to connect.");
        }
    }
    else
    {
        logger_base.warn("Projector unable to prepare data.");
    }
}

bool PlayListItemProjector::PJLinkLogin()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    PJLinkLogout();

    _hash = "";

    // ScheduleOptions* options = xScheduleFrame::GetScheduleManager()->GetOptions();

    std::string ip = _ipAddress;
    std::string password = _password;

    if (ip == "")
    {
        logger_base.error("Projector tried to log into projector with no IP address.");
        return false;
    }

    logger_base.info("Projector logging into %s.", (const char*)ip.c_str());

    wxIPV4address address;
    address.Hostname(ip);
    address.Service(4352);
    _socket = new wxSocketClient();
    if (_socket != nullptr)
    {
        // wait for up to 1/2 second for connection
        if (_socket->Connect(address, false) || _socket->WaitOnConnect(0, 500))
        {
            logger_base.info("Projector connected.");

            _socket->WaitForRead(0, 500);

            wxCharBuffer buffer(100);
            _socket->Peek(buffer.data(), buffer.length());

            // wait for up to 100ms for data
            int i = 0;
            while (_socket->LastCount() == 0 && i < 100)
            {
                wxMilliSleep(1);
                _socket->Peek(buffer.data(), buffer.length());
                i++;
            }

            _socket->ReadMsg(buffer.data(), buffer.length());
            // int read = _socket->GetLastIOReadSize();

            wxString response(buffer);
            logger_base.info("Projector response '%s'", (const char*)response.c_str());

            if  (response.Left(8) == "Projector 0")
            {
                // no security
                logger_base.info("Projector No Security.");
                return true;
            }
            else if (response.Left(8) == "Projector 1")
            {
                wxString random = response.SubString(9, response.size());

                if (random.Contains("\r"))
                {
                    random = random.Left(random.Find('\r'));
                }
                logger_base.info("Projector random '%s'.", (const char*)random.c_str());

                random = random + password;

                _hash = md5(random.ToStdString());

                logger_base.info("Projector session hash '%s'.", (const char*)_hash.c_str());
            }
        }
        else
        {
            logger_base.error("Projector refused connection.");
            delete _socket;
            _socket = nullptr;
        }
    }
    else
    {
        logger_base.error("Projector refused connection %s:4352.", (const char *)ip.c_str());
    }

    return _socket != nullptr;
}

bool PlayListItemProjector::SendPJLinkCommand(const std::string& command)
{
    if (_socket == nullptr) return false;

    std::string cmd = _hash + command;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Projector sending command %s.", (const char*)cmd.c_str());

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
        i++;
    }

    _socket->ReadMsg(buffer.data(), buffer.length());
    int read = _socket->GetLastIOReadSize();
    wxString response(buffer);
    logger_base.info("Projector response '%s' len: %d.", (const char*)response.c_str(), read);

    return true;
}

void PlayListItemProjector::PJLinkLogout()
{
    if (_socket == nullptr) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Projector logging out.");

    if (_socket != nullptr)
    {
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _hash = "";
}

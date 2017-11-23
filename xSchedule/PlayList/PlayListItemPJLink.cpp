#include "PlayListItemPJLink.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>
#include "PlayListItemPJLinkPanel.h"
#include "../../xLights/AudioManager.h"
#include "../xScheduleMain.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "../md5.h"
#include <log4cpp/Category.hh>
#include "../Projector.h"

PlayListItemPJLink::PlayListItemPJLink(wxXmlNode* node) : PlayListItem(node)
{
    _socket = nullptr;
    _command = "";
    _parameter = "1";
    _projector = "";
    _started = false;
    PlayListItemPJLink::Load(node);
}

void PlayListItemPJLink::Load(wxXmlNode* node)
{
    PlayListItem::Load(node);
    _command = node->GetAttribute("Command", "");
    _parameter = node->GetAttribute("Parameter", "1");
    _projector = node->GetAttribute("Projector", "1");
}

PlayListItemPJLink::PlayListItemPJLink() : PlayListItem()
{
    _command = "";
    _parameter = "1";
    _projector = "";
    _started = false;
    _socket = nullptr;
}

PlayListItem* PlayListItemPJLink::Copy() const
{
    PlayListItemPJLink* res = new PlayListItemPJLink();
    res->_command = _command;
    res->_parameter = _parameter;
    res->_projector = _projector;
    PlayListItem::Copy(res);

    return res;
}

wxXmlNode* PlayListItemPJLink::Save()
{
    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "PLIPJLink");

    node->AddAttribute("Projector", _projector);
    node->AddAttribute("Command", _command);
    node->AddAttribute("Parameter", _parameter);
    PlayListItem::Save(node);

    return node;
}

void PlayListItemPJLink::Configure(wxNotebook* notebook)
{
    notebook->AddPage(new PlayListItemPJLinkPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemPJLink::GetTitle() const
{
    return "PJLink";
}

std::string PlayListItemPJLink::GetNameNoTime() const
{
    if (_command == "")
    {
        return "PJLink";
    }
    else
    {
        return _command;
    }
}

void PlayListItemPJLink::Frame(wxByte* buffer, size_t size, size_t ms, size_t framems, bool outputframe)
{
    if (ms >= _delay && !_started)
    {
        _started = true;
        ExecutePJLinkCommand();
    }
}

void PlayListItemPJLink::Start(long stepLengthMS)
{
    PlayListItem::Start(stepLengthMS);

    _started = false;
}

void PlayListItemPJLink::ExecutePJLinkCommand()
{
    Login();
    if (_command == "Power On")
    {
        SendCommand("%1POWR 1\r");
    }
    else if (_command == "Power Off")
    {
        SendCommand("%1POWR 0\r");
    }
    else if (_command == "Change Input to RGB")
    {
        SendCommand("%1INPT 1"+_parameter+"\r");
    }
    else if (_command == "Change Input to VIDEO")
    {
        SendCommand("%1INPT 2" + _parameter + "\r");
    }
    else if (_command == "Change Input to DIGITAL")
    {
        SendCommand("%1INPT 3" + _parameter + "\r");
    }
    else if (_command == "Change Input to STORAGE")
    {
        SendCommand("%1INPT 4" + _parameter + "\r");
    }
    else if (_command == "Change Input to NETWORK")
    {
        SendCommand("%1INPT 5" + _parameter + "\r");
    }
    else if (_command == "Video Mute On")
    {
        SendCommand("%1AVMT 11\r");
    }
    else if (_command == "Video Mute Off")
    {
        SendCommand("%1AVMT 10\r");
    }
    else if (_command == "Audio Mute On")
    {
        SendCommand("%1AVMT 21\r");
    }
    else if (_command == "Audio Mute Off")
    {
        SendCommand("%1AVMT 20\r");
    }
    Logout();
}

std::list<Projector*> PlayListItemPJLink::GetProjectors()
{
    return xScheduleFrame::GetScheduleManager()->GetOptions()->GetProjectors();
}

bool PlayListItemPJLink::Login()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Logout();

    _hash = "";

    ScheduleOptions* options = xScheduleFrame::GetScheduleManager()->GetOptions();

    if (options->GetProjector(_projector) == nullptr)
    {
        logger_base.error("PJLink tried to log into non existent projector '%s'.", (const char *)_projector.c_str());
        return false;
    }

    std::string ip = options->GetProjector(_projector)->GetIP();
    std::string password = options->GetProjector(_projector)->GetPassword();

    if (ip == "")
    {
        logger_base.error("PJLink tried to log into projector '%s' with no IP address.", (const char *)_projector.c_str());
        return false;
    }

    logger_base.info("PJLink logging into %s.", (const char*)ip.c_str());

    wxIPV4address address;
    address.Hostname(ip);
    address.Service(4352);
    _socket = new wxSocketClient();
    if (_socket != nullptr)
    {
        // wait for up to 1/2 second for connection
        if (_socket->Connect(address, false) || _socket->WaitOnConnect(0, 500))
        {
            logger_base.info("PJLink connected.");

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
            logger_base.info("PJLink response '%s'", (const char*)response.c_str());

            if  (response.Left(8) == "PJLINK 0")
            {
                // no security
                logger_base.info("PJLink No Security.");
                return true;
            }
            else if (response.Left(8) == "PJLINK 1")
            {
                wxString random = response.SubString(9, response.size());

                if (random.Contains("\r"))
                {
                    random = random.Left(random.Find('\r'));
                }
                logger_base.info("PJLink random '%s'.", (const char*)random.c_str());

                random = random + password;

                _hash = md5(random.ToStdString());

                logger_base.info("PJLink session hash '%s'.", (const char*)_hash.c_str());
            }
        }
        else
        {
            logger_base.error("PJLink refused connection.");
            delete _socket;
            _socket = nullptr;
        }
    }
    else
    {
        logger_base.error("PJLink refused connection %s:4352.", (const char *)ip.c_str());
    }

    return _socket != nullptr;
}

bool PlayListItemPJLink::SendCommand(const std::string& command)
{
    if (_socket == nullptr) return false;

    std::string cmd = _hash + command;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("PJLink sending command %s.", (const char*)cmd.c_str());

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
    logger_base.info("PJLink response '%s' len: %d.", (const char*)response.c_str(), read);

    return true;
}

void PlayListItemPJLink::Logout()
{
    if (_socket == nullptr) return;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("PJLink logging out.");

    if (_socket != nullptr)
    {
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _hash = "";
}

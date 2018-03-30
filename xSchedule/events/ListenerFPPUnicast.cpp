#include "ListenerFPPUnicast.h"
#include <log4cpp/Category.hh>
#include <wx/socket.h>
#include "ListenerManager.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../Control.h"

bool ListenerFPPUnicast::IsValidHeader(wxByte* buffer)
{
    return  buffer[0] == 'F' &&
            buffer[1] == 'P' &&
            buffer[2] == 'P' &&
            buffer[3] == ',' &&
            buffer[4] == '1' &&
            buffer[5] == ',' &&
            buffer[6] == '0';
}

ListenerFPPUnicast::ListenerFPPUnicast(ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _socket = nullptr;
}

void ListenerFPPUnicast::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Unicast listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerFPPUnicast::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("FPP Unicast listener stopping.");
    _stop = true;
    if (_thread != nullptr)
    {
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        _thread->Stop();
    }
}

void ListenerFPPUnicast::StartProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxIPV4address localaddr;
    if (IPOutput::GetLocalIP() == "")
    {
        localaddr.AnyAddress();
    }
    else
    {
        localaddr.Hostname(IPOutput::GetLocalIP());
    }
    localaddr.Service(FPP_CTRL_CSV_PORT);

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_NONE);
    if (_socket == nullptr)
    {
        logger_base.error("Error opening datagram for FPP Unicast reception.");
    }
    else if (!_socket->IsOk())
    {
        logger_base.error("Error opening datagram for FPP Unicast reception. OK : FALSE");
        delete _socket;
        _socket = nullptr;
    }
    else if (_socket->Error())
    {
        logger_base.error("Error opening datagram for FPP Unicast reception. %d : %s", _socket->LastError(), (const char*)IPOutput::DecodeError(_socket->LastError()).c_str());
        delete _socket;
        _socket = nullptr;
    }
    else
    {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("FPP Unicast reception datagram opened successfully.");
    }
}

void ListenerFPPUnicast::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("FPP Unicast Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
}

void ListenerFPPUnicast::Poll()
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr)
    {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        //wxStopWatch sw;
        //logger_base.debug("Trying to read FPP unicast packet.");
        _socket->Read(&buffer[0], sizeof(buffer));
        if (_stop) return;
        //logger_base.debug(" Read done. %ldms", sw.Time());

        if (IsValidHeader(buffer))
        {
            static wxString lastMessage = "";

            char* cr = strchr((char*)buffer, '\n');
            if ((size_t)cr - (size_t)&buffer[0] < sizeof(buffer))
            {
                *cr = 0x00;
            }

            wxString msg = wxString(buffer);

            if (msg == lastMessage)
            {
                // we dont want to double process
            }
            else
            {
                lastMessage = msg;
                wxArrayString components = wxSplit(msg, ',');

                if (components.size() >= 5)
                {
                    //uint8_t packetType = wxAtoi(components[3]);
                    std::string fileName = components[4].ToStdString();
                    if (components.size() >= 7)
                    {
                        int secondsElapsed = ((float)wxAtoi(components[5]) * 1000 + (float)wxAtoi(components[6])) / 1000.0;
                        _listenerManager->Sync(fileName, secondsElapsed * 1000, GetType());
                    }
                    // logger_base.debug("Pkt %s.", (const char *)msg.c_str());
                }
            }
        }
    }
}

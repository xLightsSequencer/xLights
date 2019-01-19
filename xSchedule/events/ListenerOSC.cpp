#include "ListenerOSC.h"
#include "ListenerManager.h"
#include "../../xLights/outputs/IPOutput.h"
#include "../ScheduleManager.h"
#include "../ScheduleOptions.h"
#include "../../xLights/UtilFunctions.h"
#include "../OSCPacket.h"

#include <wx/socket.h>

#include <log4cpp/Category.hh>

ListenerOSC::ListenerOSC(ListenerManager* listenerManager) : ListenerBase(listenerManager)
{
    _frameMS = 50;
    _socket = nullptr;
}

void ListenerOSC::Start()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("OSC listener starting.");
    _thread = new ListenerThread(this);
}

void ListenerOSC::Stop()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!_stop)
    {
        logger_base.debug("OSC listener stopping.");
        if (_socket != nullptr)
            _socket->SetTimeout(0);
        if (_thread != nullptr)
        {
            _stop = true;
            _thread->Stop();
        }
    }
}

void ListenerOSC::StartProcess()
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
    localaddr.Service(_listenerManager->GetScheduleManager()->GetOptions()->GetOSCOptions()->GetClientPort());

    _socket = new wxDatagramSocket(localaddr, wxSOCKET_BROADCAST);
    if (_socket == nullptr)
    {
        logger_base.error("Error opening datagram for OSC reception. %s", (const char *)localaddr.IPAddress().c_str());
    }
    else if (!_socket->IsOk())
    {
        logger_base.error("Error opening datagram for OSC reception. %s OK : FALSE", (const char *)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    }
    else if (_socket->Error())
    {
        logger_base.error("Error opening datagram for OSC reception. %d : %s %s", _socket->LastError(), (const char*)DecodeIPError(_socket->LastError()).c_str(), (const char *)localaddr.IPAddress().c_str());
        delete _socket;
        _socket = nullptr;
    }
    else
    {
        _socket->SetTimeout(1);
        _socket->Notify(false);
        logger_base.info("OSC reception datagram opened successfully.");
        _isOk = true;
    }
}

void ListenerOSC::StopProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_socket != nullptr) {
        logger_base.info("OSC Listener closed.");
        _socket->Close();
        delete _socket;
        _socket = nullptr;
    }
    _isOk = false;
}

void ListenerOSC::Poll()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_socket != nullptr)
    {
        unsigned char buffer[2048];
        memset(buffer, 0x00, sizeof(buffer));

        //wxStopWatch sw;
        //logger_base.debug("Trying to read OSC packet.");
        _socket->Read(&buffer[0], sizeof(buffer));

        if (_stop) return;
        //logger_base.debug(" Read done. %ldms", sw.Time());

        if (_socket->GetLastIOReadSize() == 0)
        {
            _socket->WaitForRead(0, 50);
        }
        else
        {
            OSCPacket packet(buffer, sizeof(buffer), _listenerManager->GetScheduleManager()->GetOptions()->GetOSCOptions(), _frameMS);

            if (packet.IsSync())
            {
                std::string stepname = packet.GetStepName();
                std::string timingname = packet.GetTimingName();
                long ms = packet.GetMS(_frameMS);

                if (stepname != "")
                {
                    _frameMS = _listenerManager->Sync(stepname, ms, GetType());
                }
                else if (timingname != "")
                {
                    _frameMS = _listenerManager->Sync(timingname, ms, GetType());
                }
            }
            else if (packet.IsOk())
            {
                logger_base.debug("OSC Path: %s.", (const char*)packet.GetPath().c_str());
                _listenerManager->ProcessPacket(GetType(), packet.GetPath(), packet.GetP1(), packet.GetP2(), packet.GetP3());
            }
            else
            {
                logger_base.debug("Invalid OSC Packet.");
            }
        }
    }
}

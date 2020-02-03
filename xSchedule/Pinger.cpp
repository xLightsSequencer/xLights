#include "Pinger.h"
#include "../xLights/outputs/OutputManager.h"
#include "events/ListenerManager.h"
#include "../xLights/outputs/ControllerEthernet.h"
#include "../xLights/outputs/ControllerSerial.h"

#include <atomic>

#include <log4cpp/Category.hh>

class PingThread : public wxThread
{
    APinger* _pinger;
    std::atomic<bool> _stop;
    std::atomic<bool> _running;

public:

    PingThread(APinger* pinger) : wxThread(wxTHREAD_JOINABLE)
    {
        _pinger = pinger;
        _stop = false;
        _running = false;
    }

    virtual ~PingThread()
    {
        //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (_running && !_stop)
        {
            _stop = true;
        }
    }

    void Stop()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Asking pinging thread %s to stop", (const char *)_pinger->GetName().c_str());
        _stop = true;
    }
    
    // if ping fails try this many times before setting the result
#define PINGRETRIES 3

    virtual void* Entry() override
    {
        _running = true;

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Pinging thread %s started", (const char *)_pinger->GetName().c_str());

        while (!_stop)
        {

            auto res = Output::PINGSTATE::PING_UNKNOWN;

            // if it previously failed ... only try once
            int loops = PINGRETRIES;
            if (_pinger->GetFailCount() > 0) loops = 1;
            
            for (int i = 0; !_stop && i < loops; i++)
            {
                res = _pinger->Ping();
                if (res != Output::PINGSTATE::PING_ALLFAILED)
                {
                    break;
                }
            }
            _pinger->SetPingResult(res);

            int count = 0;

            while (!_stop && count < _pinger->GetPingInterval() * 100)
            {
                wxMilliSleep(10);
                count++;
            }
        }
        _running = false;

        return nullptr;
    }
};

APinger::APinger(ListenerManager* lm, Controller* controller)
{
    _listenerManager = lm;
    _controller = controller;
    _why = "Output";
    if (dynamic_cast<ControllerEthernet*>(controller) != nullptr)
    {
        _ip = dynamic_cast<ControllerEthernet*>(controller)->GetIP();
    }
    _lastResult = Output::PINGSTATE::PING_UNKNOWN;
    _failCount = 0;
    _pingThread = new PingThread(this);
    _pingThread->Create();
    _pingThread->Run();
}

APinger::APinger(ListenerManager* lm, const std::string ip, const std::string why)
{
    _listenerManager = lm;
    _controller = nullptr;
    _ip = ip;
    _why = why;
    _lastResult = Output::PINGSTATE::PING_UNKNOWN;
    _failCount = 0;
    _pingThread = new PingThread(this);
    _pingThread->Create();
    _pingThread->Run();
}

APinger::~APinger()
{
}

Output::PINGSTATE APinger::GetPingResult() const
{
    return _lastResult;
}

bool APinger::GetPingResult(Output::PINGSTATE state) const
{
    switch (state)
    {
    case Output::PINGSTATE::PING_ALLFAILED:
        return false;
    case Output::PINGSTATE::PING_OK:
    case Output::PINGSTATE::PING_WEBOK:
    case Output::PINGSTATE::PING_OPEN:
    case Output::PINGSTATE::PING_OPENED:
    case Output::PINGSTATE::PING_UNKNOWN:
    case Output::PINGSTATE::PING_UNAVAILABLE:
    default:
        return true;
    }
}

std::string APinger::GetPingResultName(Output::PINGSTATE state)
{
    switch(state)
    {
    case Output::PINGSTATE::PING_ALLFAILED:
        return "Failed";
    case Output::PINGSTATE::PING_UNAVAILABLE:
        return "Unavailable";
    case Output::PINGSTATE::PING_OK:
    case Output::PINGSTATE::PING_WEBOK:
    case Output::PINGSTATE::PING_OPEN:
    case Output::PINGSTATE::PING_OPENED:
        return "Ok";
    case Output::PINGSTATE::PING_UNKNOWN:
        return "Unknown";
    }

    return "Unknown";
}

Output::PINGSTATE APinger::Ping()
{
    if (_controller != nullptr)
    {
        return _controller->Ping();
    }
 
    return IPOutput::Ping(_ip, "");
}

void APinger::SetPingResult(Output::PINGSTATE result)
{
    _lastResult = result;

    if (result == Output::PINGSTATE::PING_ALLFAILED) _failCount++;
    if (result == Output::PINGSTATE::PING_OK || result == Output::PINGSTATE::PING_OPEN || result == Output::PINGSTATE::PING_OPENED || result == Output::PINGSTATE::PING_WEBOK) _failCount = 0;

    _listenerManager->ProcessPacket("Ping", GetPingResult(result), _ip);
}

std::string APinger::GetName() const
{
    if (_controller != nullptr)
    {
        return _controller->GetPingDescription();
    }
    return _ip + " " + _why;
}

void APinger::Stop()
{
    // tell it to stop ... but it may take a bit of time to stop
    if (_pingThread != nullptr)
    {
        _pingThread->Stop();
        _pingThread->Delete();
        delete _pingThread;
        _pingThread = nullptr;
    }
    _controller = nullptr;
    _ip = "";
}

std::string GetID(Controller* controller)
{
    if (dynamic_cast<ControllerEthernet*>(controller) != nullptr)
    {
        return dynamic_cast<ControllerEthernet*>(controller)->GetIP();
    }
    else if (dynamic_cast<ControllerSerial*>(controller) != nullptr)
    {
        return dynamic_cast<ControllerSerial*>(controller)->GetPort();
    }
    return "";
}

Pinger::Pinger(ListenerManager* listenerManager, OutputManager* outputManager)
{
    _listenerManager = listenerManager;
    std::list<std::string> created;

    auto controllers = outputManager->GetControllers();

    for (const auto& it : controllers)
    {
        if (it->CanPing() && it->IsEnabled())
        {
            // check if we have already seen it
            bool found = false;
            for (const auto& cit : created)
            {
                if (cit == GetID(it))
                {
                    // we have seen it
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                created.push_back(GetID(it));
                _pingers.push_back(new APinger(listenerManager, it));
            }
        }
    }
}

Pinger::~Pinger()
{
    // Tell them to stop first ... means when we come to delete them we may not need to wait as long
    for (auto it = _pingers.begin(); it != _pingers.end(); ++it)
    {
        (*it)->Stop();
    }

    // give the ping threads some CPU time
    wxMilliSleep(1);

    for (const auto& it : _pingers)
    {
        delete it;
    }
}

void Pinger::AddIP(const std::string ip, const std::string why)
{
    if (ip == "") return;
    if (ip == "255.255.255.255") return;

    for (const auto& it : _pingers)
    {
        if (it->GetIP() == ip) return;
    }

    _pingers.push_back(new APinger(_listenerManager, ip, why));
}

void Pinger::RemoveNonOutputIPs()
{
    std::list<APinger*> newPingers;
    for (const auto& it : _pingers)
    {
        if (!it->IsOutput())
        {
            it->Stop();
            delete it;
        }
        else
        {
            newPingers.push_back(it);
        }
    }
    _pingers = newPingers;
}

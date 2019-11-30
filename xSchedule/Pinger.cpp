#include "Pinger.h"
#include <log4cpp/Category.hh>
#include "../xLights/outputs/OutputManager.h"
#include "../xLights/outputs/IPOutput.h"
#include "events/ListenerManager.h"

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

            auto res = PINGSTATE::PING_UNKNOWN;

            // if it previously failed ... only try once
            int loops = PINGRETRIES;
            if (_pinger->GetFailCount() > 0) loops = 1;
            
            for (int i = 0; !_stop && i < loops; i++)
            {
                res = _pinger->Ping();
                if (res != ::PING_ALLFAILED)
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

APinger::APinger(ListenerManager* lm, Output* output)
{
    _listenerManager = lm;
    _output = output;
    _why = "Output";
    _ip = output->GetIP();
    _lastResult = PINGSTATE::PING_UNKNOWN;
    _failCount = 0;
    _pingThread = new PingThread(this);
    _pingThread->Create();
    _pingThread->Run();
}

APinger::APinger(ListenerManager* lm, const std::string ip, const std::string why)
{
    _listenerManager = lm;
    _output = nullptr;
    _ip = ip;
    _why = why;
    _lastResult = PINGSTATE::PING_UNKNOWN;
    _failCount = 0;
    _pingThread = new PingThread(this);
    _pingThread->Create();
    _pingThread->Run();
}

APinger::~APinger()
{
}

PINGSTATE APinger::GetPingResult() const
{
    return _lastResult;
}

bool APinger::GetPingResult(PINGSTATE state) const
{
    switch (state)
    {
    case PINGSTATE::PING_ALLFAILED:
        return false;
    case PINGSTATE::PING_OK:
    case PINGSTATE::PING_WEBOK:
    case PINGSTATE::PING_OPEN:
    case PINGSTATE::PING_OPENED:
    case PINGSTATE::PING_UNKNOWN:
    case PINGSTATE::PING_UNAVAILABLE:
    default:
        return true;
    }
}

std::string APinger::GetPingResultName(PINGSTATE state)
{
    switch(state)
    {
    case PINGSTATE::PING_ALLFAILED:
        return "Failed";
    case PINGSTATE::PING_UNAVAILABLE:
        return "Unavailable";
    case PINGSTATE::PING_OK:
    case PINGSTATE::PING_WEBOK:
    case PINGSTATE::PING_OPEN:
    case PINGSTATE::PING_OPENED:
        return "Ok";
    case PINGSTATE::PING_UNKNOWN:
        return "Unknown";
    }

    return "Unknown";
}

PINGSTATE APinger::Ping()
{
    if (_output != nullptr)
    {
        return _output->Ping();
    }
 
    return IPOutput::Ping(_ip, "");
}

void APinger::SetPingResult(PINGSTATE result)
{
    _lastResult = result;

    if (result == PINGSTATE::PING_ALLFAILED) _failCount++;
    if (result == PINGSTATE::PING_OK || result == PINGSTATE::PING_OPEN || result == PINGSTATE::PING_OPENED || result == PINGSTATE::PING_WEBOK) _failCount = 0;

    _listenerManager->ProcessPacket("Ping", GetPingResult(result), _ip);
}

std::string APinger::GetName() const
{
    if (_output != nullptr)
    {
        return _output->GetPingDescription();
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
    _output = nullptr;
    _ip = "";
}

std::string GetID(Output* output)
{
    std::string id = output->GetIP();

    if (id == "") id = output->GetCommPort();

    return id;
}

Pinger::Pinger(ListenerManager* listenerManager, OutputManager* outputManager)
{
    _listenerManager = listenerManager;
    std::list<std::string> created;

    auto outputs = outputManager->GetOutputs();

    for (const auto& it : outputs)
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

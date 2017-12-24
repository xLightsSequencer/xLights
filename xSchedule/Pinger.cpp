#include "Pinger.h"
#include <log4cpp/Category.hh>
#include "../xLights/outputs/OutputManager.h"

class PingThread : public wxThread
{
    APinger* _pinger;
    bool _stop;
    bool _running;

public:

    PingThread(APinger* pinger)
    {
        _pinger = pinger;
        _stop = false;
        _running = false;
    }

    virtual ~PingThread()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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

    virtual void* Entry() override
    {
        _running = true;

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug("Pinging thread %s started", (const char *)_pinger->GetName().c_str());

        while (!_stop)
        {
            _pinger->Ping();
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

APinger::APinger(Output* output)
{
    _output = output;
    _lastResult = PINGSTATE::PING_UNKNOWN;
    _pingThread = new PingThread(this);
    _pingThread->Create();
    _pingThread->Run();
}

APinger::~APinger()
{
}

PINGSTATE APinger::GetPingResult()
{
    std::unique_lock<std::mutex> mutLock(_lock);
    return _lastResult;
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

void APinger::Ping()
{
    SetPingResult(_output->Ping());
}

void APinger::SetPingResult(PINGSTATE result)
{
    std::unique_lock<std::mutex> mutLock(_lock);
    _lastResult = result;
}

std::string APinger::GetName() const
{
    if (_output != nullptr)
    {
        return _output->GetPingDescription();
    }

    return "";
}

void APinger::Stop()
{
    _output = nullptr;

    // tell it to stop ... but it may take a bit of time to stop
    if (_pingThread != nullptr)
    {
        _pingThread->Stop();
        _pingThread = nullptr;
    }
}

std::string GetID(Output* output)
{
    std::string id = output->GetIP();

    if (id == "") id = output->GetCommPort();

    return id;
}

Pinger::Pinger(OutputManager* outputManager)
{
    std::list<std::string> created;

    auto outputs = outputManager->GetOutputs();

    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if ((*it)->CanPing())
        {
            // check if we have already seen it
            bool found = false;
            for (auto cit = created.begin(); cit != created.end(); ++cit)
            {
                if (*cit == GetID(*it))
                {
                    // we have seen it
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                created.push_back(GetID(*it));
                _pingers.push_back(new APinger(*it));
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

    for (auto it  = _pingers.begin(); it != _pingers.end(); ++it)
    {
        delete *it;
    }
}

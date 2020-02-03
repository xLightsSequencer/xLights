#pragma once

#include <string>
#include <list>
#include <atomic>

#include "../xLights/outputs/Output.h"

#define PINGINTERVAL 60

class OutputManager;
class PingThread;
class ListenerManager;
class Controller;

class APinger
{
    PingThread* _pingThread = nullptr;
	Controller* _controller = nullptr;
	std::atomic<Output::PINGSTATE> _lastResult;
    std::string _ip;
    std::string _why;
    ListenerManager* _listenerManager = nullptr;
    std::atomic<int> _failCount;

    public:

    void SetPingResult(Output::PINGSTATE result);
    bool IsOutput() const { return _controller != nullptr; }
	APinger(ListenerManager* listenerManager, Controller* controller);
	APinger(ListenerManager* listenerManager, const std::string ip, const std::string why);
	virtual ~APinger();
	Output::PINGSTATE GetPingResult() const;
    bool GetPingResult(Output::PINGSTATE state) const;
    static std::string GetPingResultName(Output::PINGSTATE state);
    Output::PINGSTATE Ping();
	std::string GetName() const;
    int GetPingInterval() const { return PINGINTERVAL; }
    void Stop();
    std::string GetIP() const { return _ip; }
    int GetFailCount() const { return  _failCount; }
};

class Pinger
{
    ListenerManager* _listenerManager = nullptr;
    std::list<APinger*> _pingers;
	
	public:
		Pinger(ListenerManager* listenerManager, OutputManager* outputManager);
		virtual ~Pinger();
        std::list<APinger*> GetPingers() const { return _pingers; }
        void AddIP(const std::string ip, const std::string why);
        void RemoveNonOutputIPs();
};

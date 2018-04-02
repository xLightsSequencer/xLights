#ifndef PINGER_H
#define PINGER_H

#include <string>
#include <list>
#include "../xLights/outputs/Output.h"
#include <mutex>

#define PINGINTERVAL 60

class OutputManager;
class PingThread;
class ListenerManager;

class APinger
{
    PingThread* _pingThread;
	Output* _output;
	PINGSTATE _lastResult;
    std::mutex _lock;
    std::string _ip;
    std::string _why;
    ListenerManager* _listenerManager;

    void SetPingResult(PINGSTATE result);

    public:

    bool IsOutput() const { return _output != nullptr; }
	APinger(ListenerManager* listenerManager, Output* output);
	APinger(ListenerManager* listenerManager, const std::string ip, const std::string why);
	virtual ~APinger();
	PINGSTATE GetPingResult();
    bool GetPingResult(PINGSTATE state);
    static std::string GetPingResultName(PINGSTATE state);
    void Ping();
	std::string GetName() const;
    int GetPingInterval() const { return PINGINTERVAL; }
    void Stop();
    std::string GetIP() const { return _ip; }
};

class Pinger
{
    ListenerManager* _listenerManager;
    std::list<APinger*> _pingers;
	
	public:
		Pinger(ListenerManager* listenerManager, OutputManager* outputManager);
		virtual ~Pinger();
        std::list<APinger*> GetPingers() const { return _pingers; }
        void AddIP(const std::string ip, const std::string why);
        void RemoveNonOutputIPs();
};

#endif 
#ifndef PINGER_H
#define PINGER_H

#include <string>
#include <list>
#include "../xLights/outputs/Output.h"
#include <mutex>

#define PINGINTERVAL 60

class OutputManager;
class PingThread;

class APinger
{
    PingThread* _pingThread;
	Output* _output;
	PINGSTATE _lastResult;
    std::mutex _lock;
    std::string _ip;
    std::string _why;

    void SetPingResult(PINGSTATE result);

    public:

    bool IsOutput() const { return _output != nullptr; }
	APinger(Output* output);
	APinger(const std::string ip, const std::string why);
	virtual ~APinger();
	PINGSTATE GetPingResult();
	static std::string GetPingResultName(PINGSTATE state);
    void Ping();
	std::string GetName() const;
    int GetPingInterval() const { return PINGINTERVAL; }
    void Stop();
    std::string GetIP() const { return _ip; }
};

class Pinger
{
	std::list<APinger*> _pingers;
	
	public:
		Pinger(OutputManager* outputManager);
		virtual ~Pinger();
        std::list<APinger*> GetPingers() const { return _pingers; }
        void AddIP(const std::string ip, const std::string why);
        void RemoveNonOutputIPs();
};

#endif 
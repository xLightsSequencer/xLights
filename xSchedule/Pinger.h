#ifndef PINGER_H
#define PINGER_H

#include <string>
#include <list>
#include <wx/wx.h>
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

    void SetPingResult(PINGSTATE result);

    public:

	APinger(Output* output);
	virtual ~APinger();
	PINGSTATE GetPingResult();
	static std::string GetPingResultName(PINGSTATE state);
    void Ping();
	std::string GetName() const;
    int GetPingInterval() const { return PINGINTERVAL; }
    void Stop();
};

class Pinger
{
	std::list<APinger*> _pingers;
	
	public:
		Pinger(OutputManager* outputManager);
		virtual ~Pinger();
        std::list<APinger*> GetPingers() const { return _pingers; }
};

#endif 
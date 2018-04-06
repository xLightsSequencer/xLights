#ifndef EVENTPING_H
#define EVENTPING_H

#include "EventBase.h"

#include <map>

class EventPing: public EventBase
{
    int _failures;
    std::string _ip;
    std::map<std::string, int> _count;

	public:
        EventPing();
		EventPing(wxXmlNode* node);
		virtual ~EventPing() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "Ping"; }
        void SetFailures(int failures) { if (_failures != failures) { _failures = failures; _changeCount++; } }
        int GetFailures() const { return _failures; }
        void SetIP(std::string ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
        std::string GetIP() const { return _ip; }
        virtual void Process(bool success, const std::string& ip, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

#endif

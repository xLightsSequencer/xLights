#ifndef EVENTSTATE_H
#define EVENTSTATE_H

#include "EventBase.h"
#include <list>

class EventState: public EventBase
{
    std::string _state;

	public:
        EventState();
		EventState(wxXmlNode* node);
		virtual ~EventState() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "State"; }
        void SetState(std::string state) { if (_state != state) { _state = state; _changeCount++; } }
        std::string GetState() const { return _state; }
        void Process(const std::string&  state, ScheduleManager* scheduleManager);
        static std::list<std::string> GetStates();
        void ProcessState(const std::string&  state, ScheduleManager* scheduleManager);
        static std::string GetParmToolTip();
};

#endif

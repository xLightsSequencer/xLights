#ifndef EVENTFPP_H
#define EVENTFPP_H

#include "EventBase.h"

class EventFPP: public EventBase
{
    std::string _id;

	public:
        EventFPP();
		EventFPP(wxXmlNode* node);
		virtual ~EventFPP() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "FPP"; }
        void SetEventId(std::string id) { if (_id != id) { _id = id; _changeCount++; } }
        std::string GetEventId() const { return _id; }
        virtual void Process(const std::string& id, ScheduleManager* scheduleManager) override;
};

#endif

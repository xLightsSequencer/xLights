#ifndef EVENTLOR_H
#define EVENTLOR_H

#include "EventSerial.h"

class EventLor: public EventSerial
{
    std::string _unit_id;

	public:
		EventLor();
		EventLor(wxXmlNode* node);
		virtual ~EventLor() {}
		virtual wxXmlNode* Save() override;
        void SetUnitId(std::string unit_id) { if (_unit_id != unit_id) { _unit_id = unit_id; _changeCount++; } }
        std::string GetUnitId() const { return _unit_id; }
		virtual std::string GetType() const override { return "LOR"; }
        //virtual void Process(const std::string& commPort, wxByte* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

#endif

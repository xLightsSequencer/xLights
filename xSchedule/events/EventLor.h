#ifndef EVENTLOR_H
#define EVENTLOR_H

#include "EventSerial.h"

class EventLor: public EventSerial
{
    int _unit_id;
    std::string _unit_id_str;

	public:
		EventLor();
		EventLor(wxXmlNode* node);
		virtual ~EventLor() {}
		virtual wxXmlNode* Save() override;
        void SetUnitId(std::string unit_id);
        int GetUnitId() const { return _unit_id; }
        std::string GetUnitIdString() const { return _unit_id_str; }
		virtual std::string GetType() const override { return "LOR"; }
        virtual int GetSubType() const override { return _unit_id; }
        //virtual void Process(const std::string& commPort, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

#endif

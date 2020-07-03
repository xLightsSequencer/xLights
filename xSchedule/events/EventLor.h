#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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

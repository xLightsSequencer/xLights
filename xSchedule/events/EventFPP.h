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
        static std::string GetParmToolTip();
};

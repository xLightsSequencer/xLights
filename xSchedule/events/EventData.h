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

class EventData: public EventBase
{
    std::string _channel;

	public:
		EventData();
		EventData(wxXmlNode* node);
		virtual ~EventData() {}
        bool IsFrameProcess() const override { return true; }
        virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "Data"; }
        void SetChannel(std::string channel) { if (_channel != channel) { _channel = channel; _changeCount++; } }
        std::string GetChannel() const { return _channel; }
        virtual void Process(uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

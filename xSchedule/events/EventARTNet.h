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

class EventARTNet: public EventBase
{
    int _universe;
    int _channel;

	public:
        virtual bool IsUniverseProcess() const override { return true; }
        EventARTNet();
		EventARTNet(wxXmlNode* node);
		virtual ~EventARTNet() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "ARTNet"; }
        void SetUniverse(int universe) { if (_universe != universe) { _universe = universe; _changeCount++; } }
        void SetChannel(int channel) { if (_channel != channel) { _channel = channel; _changeCount++; } }
        int GetUniverse() const { return _universe; }
        int GetChannel() const { return _channel; }
        virtual void Process(int universe, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

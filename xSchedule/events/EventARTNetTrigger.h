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

class EventARTNetTrigger : public EventBase
{
	int _oem;
	uint8_t _key;
	uint8_t _subkey;

	public:
        EventARTNetTrigger();
		EventARTNetTrigger(wxXmlNode* node);
		virtual ~EventARTNetTrigger() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "ARTNetTrigger"; }
        void SetOEM(int oem) { if (_oem != oem) { _oem = oem; _changeCount++; } }
        void SetKey(int key) { if (_key != key) { _key = key; _changeCount++; } }
        void SetSubKey(int subkey) { if (_subkey != subkey) { _subkey = subkey; _changeCount++; } }
        int GetOEM() const { return _oem; }
        int GetKey() const { return _key; }
        int GetSubKey() const { return _subkey; }
        virtual void Process(int universe, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

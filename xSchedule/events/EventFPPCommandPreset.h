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

class EventFPPCommandPreset: public EventBase
{
    std::string _presetName;

	public:
        EventFPPCommandPreset();
		EventFPPCommandPreset(wxXmlNode* node);
		virtual ~EventFPPCommandPreset() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "FPPCommandPreset"; }
        void SetEventPresetName(std::string presetName) { if (_presetName != presetName) { _presetName = presetName; _changeCount++; } }
        std::string GetEventPresetName() const { return _presetName; }
        virtual void Process(const std::string& presetName, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};

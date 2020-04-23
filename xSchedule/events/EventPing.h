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

#include <map>

class EventPing: public EventBase
{
    int _failures;
    std::string _ip;
    std::map<std::string, int> _count;
    bool _onceOnly;

	public:
        EventPing();
		EventPing(wxXmlNode* node);
		virtual ~EventPing() {}
		virtual wxXmlNode* Save() override;
		virtual std::string GetType() const override { return "Ping"; }
        void SetFailures(int failures) { if (_failures != failures) { _failures = failures; _changeCount++; } }
        int GetFailures() const { return _failures; }
        void SetIP(std::string ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
        std::string GetIP() const { return _ip; }
        void SetOnceOnly(bool onceOnly) { if (_onceOnly != onceOnly) { _onceOnly = onceOnly; _changeCount++; } }
        bool GetOnceOnly() const { return _onceOnly; }
        virtual void Process(bool success, const std::string& ip, ScheduleManager* scheduleManager) override;
        static std::string GetParmToolTip();
};


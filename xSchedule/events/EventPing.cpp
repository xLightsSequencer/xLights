/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventPing.h"
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "../ScheduleManager.h"

EventPing::EventPing() : EventBase()
{
    _failures = 1;
    _ip = "All";
    _onceOnly = true;
}

EventPing::EventPing(wxXmlNode* node) : EventBase(node)
{
    _failures = wxAtoi(node->GetAttribute("Failures", "1"));
    _ip = node->GetAttribute("IP", "All");
    _onceOnly = (node->GetAttribute("OnceOnly", "False") == "True");
}

wxXmlNode* EventPing::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventPing");
    en->AddAttribute("Failures", wxString::Format("%d", _failures));
    en->AddAttribute("IP", _ip);
    if (_onceOnly)
    {
        en->AddAttribute("OnceOnly", "True");
    }
    EventBase::Save(en);
    return en;
}

void EventPing::Process(bool success, const std::string& ip, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_ip != "All" && !wxString(_ip).StartsWith(ip)) return;

    if (success)
    {
        _count[ip] = 0;
    }
    else
    {
        if (_count.find(ip) == _count.end()) _count[ip] = 0;
        _count[ip] = _count[ip] + 1;

        if (_count[ip] == _failures)
        {
            wxString pp1 = _parm1;
            wxString pp2 = _parm2;
            wxString pp3 = _parm3;
            pp1.Replace("%IP%", ip);
            pp2.Replace("%IP%", ip);
            pp3.Replace("%IP%", ip);

            std::string parameters = pp1.ToStdString();
            if (pp2 != "") parameters += "," + pp2.ToStdString();
            if (pp3 != "") parameters += "," + pp3.ToStdString();

            logger_base.debug("Event fired %s:%s:%s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetName().c_str(), (const char *)_ip.c_str(), (const char *)ip.c_str(),
                (const char *)_command.c_str(), (const char *)parameters.c_str());

            size_t rate = 0;
            wxString msg;
            scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
            logger_base.debug("    Event processed.");
        }

        if (!_onceOnly && _count[ip] >= _failures)
        {
            _count[ip] = 0;
        }
    }
}

std::string EventPing::GetParmToolTip()
{
    return "Available:\n\n   %IP% - IP Address that failed ping";
}

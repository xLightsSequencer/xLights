/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventMQTT.h"
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "../ScheduleManager.h"

EventMQTT::EventMQTT() : EventBase()
{
    _topic = "/xSchedule/Event";
    _ip = "127.0.0.1";
    _port = 1883;
    SetDefaultClientId();
}

EventMQTT::EventMQTT(wxXmlNode* node) : EventBase(node)
{
    _topic = node->GetAttribute("Topic", "/xSchedule/Event").ToStdString();
    _ip = node->GetAttribute("IP", "127.0.0.1").ToStdString();
    _username = node->GetAttribute("Username", "").ToStdString();
    _password = node->GetAttribute("Password", "").ToStdString();
    _clientId = node->GetAttribute("ClientId", "").ToStdString();
    _port = wxAtoi(node->GetAttribute("Port", "1883"));

    if (_clientId == "") SetDefaultClientId();
}

wxXmlNode* EventMQTT::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventMQTT");
    en->AddAttribute("Topic", _topic);
    en->AddAttribute("IP", _ip);
    en->AddAttribute("Username", _username);
    en->AddAttribute("Password", _password);
    en->AddAttribute("ClientId", _clientId);
    en->AddAttribute("Port", wxString::Format("%d", _port));
    EventBase::Save(en);
    return en;
}

void EventMQTT::Process(const std::string& topic, const std::string& data, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_topic != topic) return;

    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    auto split = wxSplit(data, ',');

    p1.Replace("%DATA%", data);
    if (split.size() > 0)
    {
        p1.Replace("%DATA1%", split[0]);
        if (split.size() > 1)
        {
            p1.Replace("%DATA2%", split[1]);
            if (split.size() > 2)
            {
                p1.Replace("%DATA3%", split[2]);
            }
        }
    }

    p2.Replace("%DATA%", data);
    if (split.size() > 0)
    {
        p2.Replace("%DATA1%", split[0]);
        if (split.size() > 1)
        {
            p2.Replace("%DATA2%", split[1]);
            if (split.size() > 2)
            {
                p2.Replace("%DATA3%", split[2]);
            }
        }
    }

    p3.Replace("%DATA%", data);
    if (split.size() > 0)
    {
        p3.Replace("%DATA1%", split[0]);
        if (split.size() > 1)
        {
            p3.Replace("%DATA2%", split[1]);
            if (split.size() > 2)
            {
                p3.Replace("%DATA3%", split[2]);
            }
        }
    }

    std::string parameters = p1.ToStdString();
    if (p2 != "") parameters += "," + p2.ToStdString();
    if (p3 != "") parameters += "," + p3.ToStdString();

    logger_base.debug("Event fired %s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetName().c_str(),
        (const char *)_command.c_str(), (const char *)parameters.c_str());

    size_t rate = 0;
    wxString msg;
    scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
    logger_base.debug("    Event processed.");
}

std::string EventMQTT::GetParmToolTip()
{
    return "Available:\n\n   %DATA% - entire payload\n   %DATA1% - payload first CSV value\n   %DATA2% - payload second CSV value\n   %DATA3% - payload third CSV value\n";
}

void EventMQTT::SetClientId(std::string clientId)
{
    if (clientId == "") {
        _changeCount++;
        SetDefaultClientId();
    }
    else
    {
        if (_clientId != clientId) {
            _clientId = clientId;
            _changeCount++;
        }
    }
}
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EventFPP.h"
#include <wx/xml/xml.h>
#include "./utils/spdlog_macros.h"
#include "../ScheduleManager.h"

EventFPP::EventFPP() : EventBase()
{
    _id = "";
}

EventFPP::EventFPP(wxXmlNode* node) : EventBase(node)
{
    _id = node->GetAttribute("Id", "");
}

wxXmlNode* EventFPP::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventFPP");
    en->AddAttribute("Id", _id);
    EventBase::Save(en);
    return en;
}

void EventFPP::Process(const std::string& id, ScheduleManager* scheduleManager)
{
    
    if (id != _id) return;

    auto parts = wxSplit(id, '_');
    
    wxString major = "";
    wxString minor = "";
    if (parts.size() == 2)
    {
        major = parts[0];
        minor = parts[1];
    }
    else if (parts.size() == 1)
    {
        major = parts[0];
    }

    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    p1.Replace("%MAJOR%", major);
    p2.Replace("%MAJOR%", major);
    p3.Replace("%MAJOR%", major);

    p1.Replace("%MINOR%", minor);
    p2.Replace("%MINOR%", minor);
    p3.Replace("%MINOR%", minor);

    p1.Replace("%ID%", _id);
    p2.Replace("%ID%", _id);
    p3.Replace("%ID%", _id);

    std::string parameters = p1.ToStdString();
    if (p2 != "") parameters += "," + p2.ToStdString();
    if (p3 != "") parameters += "," + p3.ToStdString();

    LOG_DEBUG("Event fired %s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetName().c_str(),
        (const char *)_command.c_str(), (const char *)parameters.c_str());

    size_t rate = 0;
    wxString msg;
    scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
    LOG_DEBUG("    Event processed.");
}

std::string EventFPP::GetParmToolTip()
{
    return "Available:\n\n   %MAJOR% - FPP major event id 01-24\n   %MINOR% - FPP minor event id 01-24\n   %ID% - FPP event id MAJOR_MINOR";
}

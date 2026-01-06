/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "EventState.h"
#include "../ScheduleManager.h"

#include "./utils/spdlog_macros.h"

EventState::EventState() : EventBase()
{
    _state = "Lights On";
}

EventState::EventState(wxXmlNode* node) : EventBase(node)
{
    _state = node->GetAttribute("State", "Lights On");
}

wxXmlNode* EventState::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventState");
    en->AddAttribute("State", _state);
    EventBase::Save(en);
    return en;
}

void EventState::Process(const std::string& state, ScheduleManager* scheduleManager)
{
    

    if (_state == state)
    {
		LOG_DEBUG("Event fired %s:%s", (const char *)GetType().c_str(), (const char *)GetState().c_str());
		ProcessState(state, scheduleManager);
		LOG_DEBUG("    Event processed.");
    }
}

void EventState::ProcessState(const std::string& state, ScheduleManager* scheduleManager)
{
    
    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    std::string parameters = p1.ToStdString();
    if (p2 != "") parameters += "," + p2.ToStdString();
    if (p3 != "") parameters += "," + p3.ToStdString();

    LOG_DEBUG("Event fired %s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetState().c_str(),
        (const char *)_command.c_str(), (const char *)parameters.c_str());

    size_t rate = 0;
    wxString msg;
    scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
    LOG_DEBUG("    Event processed.");
}

std::list<std::string> EventState::GetStates()
{
    std::list<std::string> res;

	res.push_back("Lights On");
	res.push_back("Lights Off");
	res.push_back("Output Open Error");
	res.push_back("Startup");
	//res.push_back("Shutdown");
	
    return res;
}

std::string EventState::GetParmToolTip()
{
    return "";
}

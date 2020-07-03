/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "EventState.h"
#include "../ScheduleManager.h"

#include <log4cpp/Category.hh>

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
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_state == state)
    {
		logger_base.debug("Event fired %s:%s", (const char *)GetType().c_str(), (const char *)GetState().c_str());
		ProcessState(state, scheduleManager);
		logger_base.debug("    Event processed.");
    }
}

void EventState::ProcessState(const std::string& state, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    std::string parameters = p1.ToStdString();
    if (p2 != "") parameters += "," + p2.ToStdString();
    if (p3 != "") parameters += "," + p3.ToStdString();

    logger_base.debug("Event fired %s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetState().c_str(),
        (const char *)_command.c_str(), (const char *)parameters.c_str());

    size_t rate = 0;
    wxString msg;
    scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
    logger_base.debug("    Event processed.");
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

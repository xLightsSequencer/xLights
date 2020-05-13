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
#include <wx/wx.h>
#include <wx/xml/xml.h>
#include "../ScheduleManager.h"
#include <log4cpp/Category.hh>

int EventBase::__nextId = 0;

EventBase::EventBase()
{
    _id = __nextId++;
    _command = "";
    _parm1 = "";
    _parm2 = "";
    _parm3 = "";
    _name = "";
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _condition = "Equals";
    _threshold = 255;
    _lastValue = 0;
    _first = true;
}

EventBase::EventBase(wxXmlNode* node)
{
    _lastValue = 0;
    _first = true;
    _id = __nextId++;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _command = node->GetAttribute("Command", "");
    _parm1 = node->GetAttribute("CommandParm1", "");
    _parm2 = node->GetAttribute("CommandParm2", "");
    _parm3 = node->GetAttribute("CommandParm3", "");
    _name = node->GetAttribute("Name", "");
    _condition = node->GetAttribute("Condition", "Equals").ToStdString();
    _threshold = wxAtoi(node->GetAttribute("Threshold", "255"));
}

void EventBase::Save(wxXmlNode* node)
{
    node->AddAttribute("Command", _command);
    node->AddAttribute("Name", _name);
    node->AddAttribute("CommandParm1", _parm1);
    node->AddAttribute("CommandParm2", _parm2);
    node->AddAttribute("CommandParm3", _parm3);
    node->AddAttribute("Condition", _condition);
    node->AddAttribute("Threshold", wxString::Format("%d", _threshold));
}

std::string EventBase::DefaultParmTooltip()
{
    return "Available:\n\n   %CHVALUE% - Channel value\n   %CHVALUE100% - Channel value scaled from 0-255 to 0-100\n   %CHVALUE255% - Channel value scaled from 0-100 to 0-255";
}

void EventBase::ProcessCommand(uint8_t value, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    int value100 = value * 100 / 255;
    int value255 = value * 255 / 100;

    p1.Replace("%CHVALUE%", wxString::Format("%d", (int)value));
    p2.Replace("%CHVALUE%", wxString::Format("%d", (int)value));
    p3.Replace("%CHVALUE%", wxString::Format("%d", (int)value));
    // Scale value from 0-255 to 0-100
    p1.Replace("%CHVALUE100%", wxString::Format("%d", value100));
    p2.Replace("%CHVALUE100%", wxString::Format("%d", value100));
    p3.Replace("%CHVALUE100%", wxString::Format("%d", value100));
    // Scale value from 0-100 to 0-255
    p1.Replace("%CHVALUE255%", wxString::Format("%d", value255));
    p2.Replace("%CHVALUE255%", wxString::Format("%d", value255));
    p3.Replace("%CHVALUE255%", wxString::Format("%d", value255));

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

bool EventBase::EvaluateCondition(uint8_t value)
{
    bool res = false;
    bool changed = _first || _lastValue != value;
    _first = false;

    if (_condition == "On Change")
    {
        res = changed;
    }
    else if (_condition == "Continuous")
    {
        res = true;
    }
    else if (_condition == "Equals")
    {
        res = changed && value == _threshold;
    }
    else if (_condition == "Less Than")
    {
        res = changed && value < _threshold&& _lastValue >= _threshold;
    }
    else if (_condition == "Less Than or Equals")
    {
        res = changed && value <= _threshold && _lastValue > _threshold;
    }
    else if (_condition == "Greater Than")
    {
        res = changed && value > _threshold && _lastValue <= _threshold;
    }
    else if (_condition == "Greater Than or Equals")
    {
        res = changed && value >= _threshold && _lastValue < _threshold;
    }
    else if (_condition == "Not Equals")
    {
        res = changed && value != _threshold;
    }

    _lastValue = value;

    return res;
}

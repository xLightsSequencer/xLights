/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventOSC.h"
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "../ScheduleManager.h"

EventOSC::EventOSC() : EventBase()
{
    _path = "/xSchedule/Event";
    _oscParm1Type = "None";
    _oscParm2Type = "None";
    _oscParm3Type = "None";
}

EventOSC::EventOSC(wxXmlNode* node) : EventBase(node)
{
    _path = node->GetAttribute("Path", "/xSchedule/Event").ToStdString();
    _oscParm1Type = node->GetAttribute("Parm1Type", "None").ToStdString();
    _oscParm2Type = node->GetAttribute("Parm2Type", "None").ToStdString();
    _oscParm3Type = node->GetAttribute("Parm3Type", "None").ToStdString();
}

wxXmlNode* EventOSC::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventOSC");
    en->AddAttribute("Path", _path);
    en->AddAttribute("Parm1Type", _oscParm1Type);
    en->AddAttribute("Parm2Type", _oscParm2Type);
    en->AddAttribute("Parm3Type", _oscParm3Type);
    EventBase::Save(en);
    return en;
}

void EventOSC::Process(const std::string& path, const std::string& p1, const std::string& p2, const std::string& p3, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_path != path) return;

    wxString pp1 = _parm1;
    wxString pp2 = _parm2;
    wxString pp3 = _parm3;

    pp1.Replace("%P1%", p1);
    pp2.Replace("%P1%", p1);
    pp3.Replace("%P1%", p1);
    pp1.Replace("%P2%", p2);
    pp2.Replace("%P2%", p2);
    pp3.Replace("%P2%", p2);
    pp1.Replace("%P3%", p3);
    pp2.Replace("%P3%", p3);
    pp3.Replace("%P3%", p3);

    int i = wxAtoi(p1);
    wxString pi100 = wxString::Format("%d", i * 100 / 255);
    pp1.Replace("%P1I100%", pi100);
    pp2.Replace("%P1I100%", pi100);
    pp3.Replace("%P1I100%", pi100);
    i = wxAtoi(p2);
    pi100 = wxString::Format("%d", i * 100 / 255);
    pp1.Replace("%P2I100%", pi100);
    pp2.Replace("%P2I100%", pi100);
    pp3.Replace("%P2I100%", pi100);
    i = wxAtoi(p3);
    pi100 = wxString::Format("%d", i * 100 / 255);
    pp1.Replace("%P3I100%", pi100);
    pp2.Replace("%P3I100%", pi100);
    pp3.Replace("%P3I100%", pi100);

    i = wxAtoi(p1);
    wxString pi255 = wxString::Format("%d", i * 255 / 100);
    pp1.Replace("%P1I255%", pi255);
    pp2.Replace("%P1I255%", pi255);
    pp3.Replace("%P1I255%", pi255);
    i = wxAtoi(p2);
    pi255 = wxString::Format("%d", i * 255 / 100);
    pp1.Replace("%P2I255%", pi255);
    pp2.Replace("%P2I255%", pi255);
    pp3.Replace("%P2I255%", pi255);
    i = wxAtoi(p3);
    pi100 = wxString::Format("%d", i * 255 / 100);
    pp1.Replace("%P3I255%", pi255);
    pp2.Replace("%P3I255%", pi255);
    pp3.Replace("%P3I255%", pi255);

    float f = wxAtof(p1);
    wxString pf100 = wxString::Format("%d", (int)(f * 100));
    pp1.Replace("%P1F100%", pf100);
    pp2.Replace("%P1F100%", pf100);
    pp3.Replace("%P1F100%", pf100);
    f = wxAtof(p2);
    pf100 = wxString::Format("%d", (int)(f * 100));
    pp1.Replace("%P2F100%", pf100);
    pp2.Replace("%P2F100%", pf100);
    pp3.Replace("%P2F100%", pf100);
    f = wxAtof(p3);
    pf100 = wxString::Format("%d", (int)(f * 100));
    pp1.Replace("%P3F100%", pf100);
    pp2.Replace("%P3F100%", pf100);
    pp3.Replace("%P3F100%", pf100);

    f = wxAtof(p1);
    wxString pf255 = wxString::Format("%d", (int)(f * 255));
    pp1.Replace("%P1F255%", pf255);
    pp2.Replace("%P1F255%", pf255);
    pp3.Replace("%P1F255%", pf255);
    f = wxAtof(p2);
    pf255 = wxString::Format("%d", (int)(f * 255));
    pp1.Replace("%P2F255%", pf255);
    pp2.Replace("%P2F255%", pf255);
    pp3.Replace("%P2F255%", pf255);
    f = wxAtof(p3);
    pf100 = wxString::Format("%d", (int)(f * 255));
    pp1.Replace("%P3F255%", pf255);
    pp2.Replace("%P3F255%", pf255);
    pp3.Replace("%P3F255%", pf255);

    auto paths = wxSplit(_path, '/');
    int j = 1;
    for (auto it = paths.begin(); it != paths.end(); ++it)
    {
        auto var = wxString::Format("%%PATH%d%%", j++);
        pp1.Replace(var, *it);
        pp2.Replace(var, *it);
        pp3.Replace(var, *it);
    }

    std::string parameters = pp1.ToStdString();
    if (pp2 != "") parameters += "," + pp2.ToStdString();
    if (pp3 != "") parameters += "," + pp3.ToStdString();

    logger_base.debug("Event fired %s:%s -> %s:%s", (const char *)GetType().c_str(), (const char *)GetName().c_str(),
        (const char *)_command.c_str(), (const char *)parameters.c_str());

    size_t rate = 0;
    wxString msg;
    scheduleManager->Action(_command, parameters, "", nullptr, nullptr, nullptr, rate, msg);
    logger_base.debug("    Event processed.");
}

std::string EventOSC::GetParmToolTip()
{
    return "Available:\n\n   %P1% - %P3% - Raw values of OSC parameters\n   %Pxy100% - OSC parameter x of type y (I - integer or F - float) scaled from I - 0-255 F 0.0-1.0 to 0-100. eg %P1I100%\n   %Pxy255% - OSC parameter x of type y (I - integer or F - float) scaled from I - 0-100/F - 0.0-1.0 to 0-255. eg %P2F255%\n   %PATHx% - components of the OSC path where x is 1 to the number of path components";
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventFPPCommandPreset.h"
#include <wx/xml/xml.h>
#include <log4cpp/Category.hh>
#include "../ScheduleManager.h"

EventFPPCommandPreset::EventFPPCommandPreset() :
    EventBase()
{
    _presetName = "";
}

EventFPPCommandPreset::EventFPPCommandPreset(wxXmlNode* node) :
    EventBase(node)
{
    _presetName = node->GetAttribute("PresetName", "");
}

wxXmlNode* EventFPPCommandPreset::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventFPPCommandPreset");
    en->AddAttribute("PresetName", _presetName);
    EventBase::Save(en);
    return en;
}

void EventFPPCommandPreset::Process(const std::string& presetName, ScheduleManager* scheduleManager)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (presetName != _presetName) return;

    wxString p1 = _parm1;
    wxString p2 = _parm2;
    wxString p3 = _parm3;

    p1.Replace("%PRESETNAME%", _presetName);
    p2.Replace("%PRESETNAME%", _presetName);
    p3.Replace("%PRESETNAME%", _presetName);

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

std::string EventFPPCommandPreset::GetParmToolTip()
{
    return "Available:\n\n   %PRESETNAME% - FPP Command Preset Name";
}

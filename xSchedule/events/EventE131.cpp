/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventE131.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>

EventE131::EventE131() : EventBase()
{
    _universe = 1;
    _channel = 1;
}

EventE131::EventE131(wxXmlNode* node) : EventBase(node)
{
    _universe = wxAtoi(node->GetAttribute("Universe", "1"));
    _channel = wxAtoi(node->GetAttribute("Channel", "1"));
}

wxXmlNode* EventE131::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventE131");
    en->AddAttribute("Universe", wxString::Format("%d", _universe));
    en->AddAttribute("Channel", wxString::Format("%d", _channel));
    EventBase::Save(en);
    return en;
}

void EventE131::Process(int universe, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager)
{
    if (universe != _universe) return;

    if (_channel >= 1 && _channel <= buffersize)
    {
        uint8_t value = *(buffer + _channel - 1);

        if (EvaluateCondition(value))
        {
            ProcessCommand(value, scheduleManager);
        }
    }
}

std::string EventE131::GetParmToolTip()
{
    return EventBase::DefaultParmTooltip();
}

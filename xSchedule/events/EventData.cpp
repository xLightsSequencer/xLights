/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EventData.h"
#include <wx/xml/xml.h>
#include "../ScheduleManager.h"
#include "../../xLights/outputs/OutputManager.h"

EventData::EventData() : EventBase()
{
    _channel = 1;
}

EventData::EventData(wxXmlNode* node) : EventBase(node)
{
    _channel = node->GetAttribute("Channel", "1");
}

wxXmlNode* EventData::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventData");
    en->AddAttribute("Channel", _channel);
    EventBase::Save(en);
    return en;
}

void EventData::Process(uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager)
{
    long sc = scheduleManager->GetOutputManager()->DecodeStartChannel(_channel);

    if (sc >= 1 && sc <= buffersize)
    {
        uint8_t value = *(buffer + sc - 1);

        if (EvaluateCondition(value))
        {
            ProcessCommand(value, scheduleManager);
        }
    }
}

std::string EventData::GetParmToolTip()
{
    return EventBase::DefaultParmTooltip();
}


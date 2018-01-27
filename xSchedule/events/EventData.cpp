#include "EventData.h"
#include <wx/wx.h>
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

void EventData::Process(wxByte* buffer, long buffersize, ScheduleManager* scheduleManager)
{
    long sc = scheduleManager->GetOutputManager()->DecodeStartChannel(_channel);

    if (sc >= 1 && sc <= buffersize)
    {
        wxByte value = *(buffer + sc - 1);

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


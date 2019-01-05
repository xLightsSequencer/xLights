#include "EventARTNet.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>

EventARTNet::EventARTNet() : EventBase()
{
    _universe = 1;
    _channel = 1;
    _condition = "Equals";
    _threshold = 255;
}

EventARTNet::EventARTNet(wxXmlNode* node) : EventBase(node)
{
    _universe = wxAtoi(node->GetAttribute("Universe", "1"));
    _channel = wxAtoi(node->GetAttribute("Channel", "1"));
}

wxXmlNode* EventARTNet::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventARTNet");
    en->AddAttribute("Universe", wxString::Format("%d", _universe));
    en->AddAttribute("Channel", wxString::Format("%d", _channel));
    EventBase::Save(en);
    return en;
}

void EventARTNet::Process(int universe, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager)
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

std::string EventARTNet::GetParmToolTip()
{
    return EventBase::DefaultParmTooltip();
}

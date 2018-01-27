#include "EventSerial.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>

EventSerial::EventSerial() : EventBase()
{
    _protocol = "DMX";
    _commPort = "COM1";
    _speed = 19200;
    _channel = 1;
    _condition = "Equals";
    _threshold = 255;
}

EventSerial::EventSerial(wxXmlNode* node) : EventBase(node)
{
    _protocol = node->GetAttribute("Protocol", "DMX").ToStdString();
    _commPort = node->GetAttribute("CommPort", "COM1").ToStdString();
    _speed = wxAtoi(node->GetAttribute("Speed", "19200"));
    _channel = wxAtoi(node->GetAttribute("Channel", "1"));
}

wxXmlNode* EventSerial::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventSerial");
    en->AddAttribute("Protocol", _protocol);
    en->AddAttribute("CommPort", _commPort);
    en->AddAttribute("Speed", wxString::Format("%d", _speed));
    en->AddAttribute("Channel", wxString::Format("%d", _channel));
    EventBase::Save(en);
    return en;
}

std::string EventSerial::GetSerialConfig() const
{
    if (_protocol == "OpenDMX")
    {
        return "8N2";
    }
    else
    {
        return "8N1";
    }
}

void EventSerial::Process(const std::string& commPort, wxByte* buffer, long buffersize, ScheduleManager* scheduleManager)
{
    if (commPort != _commPort) return;

    if (_channel >= 1 && _channel <= buffersize)
    {
        wxByte value = *(buffer + _channel - 1);

        if (EvaluateCondition(value))
        {
            ProcessCommand(value, scheduleManager);
        }
    }
}

std::string EventSerial::GetParmToolTip()
{
    return EventBase::DefaultParmTooltip();
}


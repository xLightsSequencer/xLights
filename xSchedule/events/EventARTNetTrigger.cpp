#include "EventARTNetTrigger.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>

EventARTNetTrigger::EventARTNetTrigger() : EventBase()
{
    _oem = 0xFFFF;
    _key = 1;
    _subkey = 1;
}

EventARTNetTrigger::EventARTNetTrigger(wxXmlNode* node) : EventBase(node)
{
    _oem = wxAtoi(node->GetAttribute("OEM", "65535"));
    _key = wxAtoi(node->GetAttribute("Key", "1"));
    _subkey = wxAtoi(node->GetAttribute("SubKey", "1"));
}

wxXmlNode* EventARTNetTrigger::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventARTNetTrigger");
    en->AddAttribute("OEM", wxString::Format("%d", _oem));
    en->AddAttribute("Key", wxString::Format("%d", _key));
    en->AddAttribute("SubKey", wxString::Format("%d", _subkey));
    EventBase::Save(en);
    return en;
}

void EventARTNetTrigger::Process(int oem, uint8_t* buffer, long buffersize, ScheduleManager* scheduleManager)
{
	if (buffersize < 2) return;
	int key = buffer[0];
	int subkey = buffer[1];
	
	if (oem == _oem && key == _key && subkey == _subkey)
	{
		ProcessCommand(subkey, scheduleManager);
	}
}

std::string EventARTNetTrigger::GetParmToolTip()
{
    return EventBase::DefaultParmTooltip();
}

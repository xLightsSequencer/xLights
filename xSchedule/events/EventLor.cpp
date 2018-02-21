#include "EventLor.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>

EventLor::EventLor() : EventSerial()
{
    _unit_id = "0x1";
}

EventLor::EventLor(wxXmlNode* node) : EventSerial(node)
{
    _unit_id = node->GetAttribute("UnitId", "0x01").ToStdString();
}

wxXmlNode* EventLor::Save()
{
    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "EventLor");
    en->AddAttribute("UnitId", _unit_id);
    en->AddAttribute("CommPort", _commPort);
    en->AddAttribute("Speed", wxString::Format("%d", _speed));
    en->AddAttribute("Channel", wxString::Format("%d", _channel));
    EventBase::Save(en);
    return en;
}

std::string EventLor::GetParmToolTip()
{
    return EventBase::DefaultParmTooltip();
}

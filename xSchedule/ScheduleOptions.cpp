#include "ScheduleOptions.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>

ScheduleOptions::ScheduleOptions(wxXmlNode* node)
{
    _dirty = false;
    _sync = node->GetAttribute("Sync", "FALSE") == "TRUE";
    _sendOffWhenNotRunning = node->GetAttribute("SendOffWhenNotRunning", "FALSE") == "TRUE";
}

ScheduleOptions::ScheduleOptions()
{
    _dirty = false;
    _sync = false;
    _sendOffWhenNotRunning = false;
}

ScheduleOptions::~ScheduleOptions()
{
}

wxXmlNode* ScheduleOptions::Save() const
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Options");

    if (IsSync())
    {
        res->AddAttribute("Sync", "TRUE");
    }
    if (IsSendOffWhenNotRunning())
    {
        res->AddAttribute("SendOffWhenNotRunning", "TRUE");
    }

    return res;
}

std::list<std::string> ScheduleOptions::GetProjectors() const
{
    std::list<std::string> res;

    return res;
}

std::string ScheduleOptions::GetProjectorIpAddress(const std::string& projector)
{
    return "";
}

std::string ScheduleOptions::GetProjectorPassword(const std::string& projector)
{
    return "";
}

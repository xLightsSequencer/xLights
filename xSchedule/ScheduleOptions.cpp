#include "ScheduleOptions.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>

ScheduleOptions::ScheduleOptions(wxXmlNode* node)
{
    _dirty = false;
    _sync = node->GetAttribute("Sync", "FALSE") == "TRUE";
    _sendOffWhenNotRunning = node->GetAttribute("SendOffWhenNotRunning", "FALSE") == "TRUE";
    _runOnMachineStartup = node->GetAttribute("RunOnMachineStartup", "FALSE") == "TRUE";

    for (auto n = node->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "Projector")
        {
            _projectorIPs[n->GetAttribute("Projector", "").ToStdString()] = n->GetAttribute("IP", "").ToStdString();
            _projectorPasswords[n->GetAttribute("Projector", "").ToStdString()] = n->GetAttribute("Password", "").ToStdString();
        }
        else if (n->GetName() == "Button")
        {
            _buttonCommands[n->GetAttribute("Label", "").ToStdString()] = n->GetAttribute("Command", "").ToStdString();
            _buttonParameters[n->GetAttribute("Label", "").ToStdString()] = n->GetAttribute("Parameters", "").ToStdString();
        }
    }

    if (_buttonCommands.size() == 0)
    {
        _buttonCommands["Play Selected"] = "Play selected playlist";
        _buttonParameters["Play Selected"] = "";
        _buttonCommands["Stop All"] = "Stop all now";
        _buttonParameters["Stop All"] = "";
    }
}

ScheduleOptions::ScheduleOptions()
{
    _dirty = false;
    _sync = false;
    _runOnMachineStartup = false;
    _sendOffWhenNotRunning = false;
}

ScheduleOptions::~ScheduleOptions()
{
}

wxXmlNode* ScheduleOptions::Save()
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
    if (IsRunOnMachineStartup())
    {
        res->AddAttribute("RunOnMachineStartup", "TRUE");
    }

    for (auto it = _projectorIPs.begin(); it != _projectorIPs.end(); ++it)
    {
        wxXmlNode* projector = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Projector");
        std::string indx = it->first;
        projector->AddAttribute("Projector", indx);
        projector->AddAttribute("IP", it->second);
        projector->AddAttribute("Password", GetProjectorPassword(indx));
        res->AddChild(projector);
    }

    for (auto it = _buttonCommands.begin(); it != _buttonCommands.end(); ++it)
    {
        wxXmlNode* button = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Button");
        std::string indx = it->first;
        button->AddAttribute("Label", indx);
        button->AddAttribute("Command", it->second);
        button->AddAttribute("Parameters", GetButtonParameter(indx));
        res->AddChild(button);
    }

    return res;
}

std::list<std::string> ScheduleOptions::GetProjectors() const
{
    std::list<std::string> res;

    for (auto it = _projectorIPs.begin(); it != _projectorIPs.end(); ++it)
    {
        res.push_back(it->first);
    }

    return res;
}

std::list<std::string> ScheduleOptions::GetButtons() const
{
    std::list<std::string> res;

    for (auto it = _buttonCommands.begin(); it != _buttonCommands.end(); ++it)
    {
        res.push_back(it->first);
    }

    return res;
}

std::string ScheduleOptions::GetProjectorIpAddress(const std::string& projector)
{
    if (_projectorIPs.find(projector) != _projectorIPs.end())
    {
        return _projectorIPs[projector];
    }

    return "";
}

std::string ScheduleOptions::GetProjectorPassword(const std::string& projector)
{
    if (_projectorPasswords.find(projector) != _projectorPasswords.end())
    {
        return _projectorPasswords[projector];
    }

    return "";
}

std::string ScheduleOptions::GetButtonCommand(const std::string& button)
{
    if (_buttonCommands.find(button) != _buttonCommands.end())
    {
        return _buttonCommands[button];
    }

    return "";
}

std::string ScheduleOptions::GetButtonParameter(const std::string& button)
{
    if (_buttonParameters.find(button) != _buttonParameters.end())
    {
        return _buttonParameters[button];
    }

    return "";
}

void ScheduleOptions::ClearProjectors()
{
    _projectorIPs.clear();
    _projectorPasswords.clear();
    _dirty = true;
}

void ScheduleOptions::ClearButtons()
{
    _buttonCommands.clear();
    _buttonParameters.clear();
    _dirty = true;
}

void ScheduleOptions::SetProjectorIPAddress(const std::string& projector, const std::string& ip)
{
    _projectorIPs[projector] = ip;
    _dirty = true;
}

void ScheduleOptions::SetProjectorPassword(const std::string& projector, const std::string& password)
{
    _projectorPasswords[projector] = password;
    _dirty = true;
}

void ScheduleOptions::SetButtonCommand(const std::string& button, const std::string& command)
{
    _buttonCommands[button] = command;
    _dirty = true;
}

void ScheduleOptions::SetButtonParameter(const std::string& button, const std::string& parameter)
{
    _buttonParameters[button] = parameter;
    _dirty = true;
}


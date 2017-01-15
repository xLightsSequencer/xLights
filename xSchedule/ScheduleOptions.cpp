#include "ScheduleOptions.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>

ScheduleOptions::ScheduleOptions(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _sync = node->GetAttribute("Sync", "FALSE") == "TRUE";
    _sendOffWhenNotRunning = node->GetAttribute("SendOffWhenNotRunning", "FALSE") == "TRUE";
    _port = wxAtoi(node->GetAttribute("WebServerPort", "80"));
    _wwwRoot = node->GetAttribute("WWWRoot", "xScheduleWeb");

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
            _buttonParameters[n->GetAttribute("Label", "").ToStdString()] = n->GetAttribute("Hotkey", "").ToStdString();
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
    _wwwRoot = "xScheduleWeb";
    _port = 80;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _sync = false;
    _sendOffWhenNotRunning = false;
}

ScheduleOptions::~ScheduleOptions()
{
}

wxXmlNode* ScheduleOptions::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Options");

    res->AddAttribute("WWWRoot", _wwwRoot);
    if (IsSync())
    {
        res->AddAttribute("Sync", "TRUE");
    }
    if (IsSendOffWhenNotRunning())
    {
        res->AddAttribute("SendOffWhenNotRunning", "TRUE");
    }
    res->AddAttribute("WebServerPort", wxString::Format(wxT("%i"), _port));

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
        button->AddAttribute("Hotkey", GetButtonHotkey(indx));
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

char ScheduleOptions::GetButtonHotkey(const std::string& button)
{
    if (_buttonHotkeys.find(button) != _buttonHotkeys.end())
    {
        return _buttonHotkeys[button];
    }

    return '\0';
}

void ScheduleOptions::ClearProjectors()
{
    if (_projectorIPs.size() > 0)
    {
        _projectorIPs.clear();
        _projectorPasswords.clear();
        _changeCount++;
    }
}

void ScheduleOptions::ClearButtons()
{
    if (_buttonCommands.size() > 0)
    {
        _buttonCommands.clear();
        _buttonParameters.clear();
        _changeCount++;
    }
}

void ScheduleOptions::SetProjectorIPAddress(const std::string& projector, const std::string& ip)
{
    if (_projectorIPs.find(projector) == _projectorIPs.end() || _projectorIPs[projector] != ip)
    {
        _projectorIPs[projector] = ip;
        _changeCount++;
    }
}

void ScheduleOptions::SetProjectorPassword(const std::string& projector, const std::string& password)
{
    if (_projectorPasswords.find(projector) == _projectorPasswords.end() || _projectorPasswords[projector] != password)
    {
        _projectorPasswords[projector] = password;
        _changeCount++;
    }
}

void ScheduleOptions::SetButtonCommand(const std::string& button, const std::string& command)
{
    if (_buttonCommands.find(button) == _buttonCommands.end() || _buttonCommands[button] != command)
    {
        _buttonCommands[button] = command;
        _changeCount++;
    }
}

void ScheduleOptions::SetButtonParameter(const std::string& button, const std::string& parameter)
{
    if (_buttonParameters.find(button) == _buttonParameters.end() || _buttonParameters[button] != parameter)
    {
        _buttonParameters[button] = parameter;
        _changeCount++;
    }
}

void ScheduleOptions::SetButtonHotkey(const std::string& button, char hotkey)
{
    if (_buttonHotkeys.find(button) == _buttonHotkeys.end() || _buttonHotkeys[button] != hotkey)
    {
        _buttonHotkeys[button] = hotkey;
        _changeCount++;
    }
}

std::string ScheduleOptions::GetButtonsJSON() const
{
    std::string res;
    bool first = true;
    res = "{\"buttons\":[";
    for (auto it = _buttonCommands.begin(); it != _buttonCommands.end(); ++it)
    {
        wxString c(it->second);

        if (!c.Contains("selected") && !c.Contains("Selected"))
        {
            if (!first)
            {
                res += ",";
            }
            first = false;
            res += "\"" + it->first + "\"";
        }
    }
    res += "]}";

    return res;
}

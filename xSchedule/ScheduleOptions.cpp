#include "ScheduleOptions.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>
#include <wx/stdpaths.h>
#include "UserButton.h"
#include "CommandManager.h"

ScheduleOptions::ScheduleOptions(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _sync = node->GetAttribute("Sync", "FALSE") == "TRUE";
    _advancedMode = node->GetAttribute("AdvancedMode", "FALSE") == "TRUE";
    _webAPIOnly = node->GetAttribute("APIOnly", "FALSE") == "TRUE";
    _sendOffWhenNotRunning = node->GetAttribute("SendOffWhenNotRunning", "FALSE") == "TRUE";
#ifdef __WXMSW__
    _port = wxAtoi(node->GetAttribute("WebServerPort", "80"));
#else
    _port = wxAtoi(node->GetAttribute("WebServerPort", "8080"));
#endif
    _passwordTimeout = wxAtoi(node->GetAttribute("PasswordTimeout", "30"));
    _wwwRoot = node->GetAttribute("WWWRoot", "xScheduleWeb");
    _password = node->GetAttribute("Password", "");

    for (auto n = node->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "Projector")
        {
            _projectorIPs[n->GetAttribute("Projector", "").ToStdString()] = n->GetAttribute("IP", "").ToStdString();
            _projectorPasswords[n->GetAttribute("Projector", "").ToStdString()] = n->GetAttribute("Password", "").ToStdString();
        }
        else if (n->GetName() == "Button")
        {
            _buttons.push_back(new UserButton(n));
        }
        else if (n->GetName() == "Matrix")
        {
            _matrices.push_back(new MatrixMapper(n));
        }
        else if (n->GetName() == "VMatrix")
        {
            _virtualMatrices.push_back(new VirtualMatrix(n));
        }
    }
}

void ScheduleOptions::AddButton(const std::string& label, const std::string& command, const std::string& parms, char hotkey)
{
    UserButton* b = new UserButton();
    b->SetLabel(label);
    b->SetCommand(command);
    b->SetParameters(parms);
    b->SetHotkey(hotkey);
    _buttons.push_back(b);
}

ScheduleOptions::ScheduleOptions()
{
    _password = "";
    _passwordTimeout = 30;
    _wwwRoot = "xScheduleWeb";
#ifdef __WXMSW__
    _port = 80;
#else
    _port = 8080;
#endif
    _webAPIOnly = false;
    _changeCount = 1;
    _lastSavedChangeCount = 0;
    _sync = false;
    _sendOffWhenNotRunning = false;
    _advancedMode = false;
}

ScheduleOptions::~ScheduleOptions()
{
    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        delete *it;
    }
    _buttons.clear();
}

wxXmlNode* ScheduleOptions::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Options");

    res->AddAttribute("WWWRoot", _wwwRoot);
    res->AddAttribute("Password", _password);
    if (IsSync())
    {
        res->AddAttribute("Sync", "TRUE");
    }

    if (_webAPIOnly)
    {
        res->AddAttribute("APIOnly", "TRUE");
    }

    if (_advancedMode)
    {
        res->AddAttribute("AdvancedMode", "TRUE");
    }

    if (IsSendOffWhenNotRunning())
    {
        res->AddAttribute("SendOffWhenNotRunning", "TRUE");
    }

    res->AddAttribute("WebServerPort", wxString::Format(wxT("%i"), _port));
    res->AddAttribute("PasswordTimeout", wxString::Format(wxT("%i"), _passwordTimeout));

    for (auto it = _projectorIPs.begin(); it != _projectorIPs.end(); ++it)
    {
        wxXmlNode* projector = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Projector");
        std::string indx = it->first;
        projector->AddAttribute("Projector", indx);
        projector->AddAttribute("IP", it->second);
        projector->AddAttribute("Password", GetProjectorPassword(indx));
        res->AddChild(projector);
    }

    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    for (auto it = _matrices.begin(); it != _matrices.end(); ++it)
    {
        res->AddChild((*it)->Save());
    }

    for (auto it = _virtualMatrices.begin(); it != _virtualMatrices.end(); ++it)
    {
        res->AddChild((*it)->Save());
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

std::vector<UserButton*> ScheduleOptions::GetButtons() const
{
    return _buttons;
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
    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        delete *it;
    }
    _buttons.clear();
    _changeCount++;
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

std::string ScheduleOptions::GetButtonsJSON(const CommandManager &cmdMgr) const
{
    std::string res;
    bool first = true;
    res = "{\"buttons\":[";
    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        if (wxString((*it)->GetLabel()).StartsWith("HIDE_"))
        {
            // dont return these
        }
        else
        {
            auto cmd = cmdMgr.GetCommand((*it)->GetCommand());
            if (!cmd->IsUIOnly())
            {
                if (!first)
                {
                    res += ",";
                }
                first = false;
                res += "\"" + (*it)->GetLabel() + "\"";
            }
        }
    }
    res += "]}";

    return res;
}

bool ScheduleOptions::IsDirty() const
{
    bool res = _lastSavedChangeCount != _changeCount;

    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

    for (auto it = _matrices.begin(); it != _matrices.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

    for (auto it = _virtualMatrices.begin(); it != _virtualMatrices.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

    return res;
}

void ScheduleOptions::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;

    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        (*it)->ClearDirty();
    }

    for (auto it = _matrices.begin(); it != _matrices.end(); ++it)
    {
        (*it)->ClearDirty();
    }

    for (auto it = _virtualMatrices.begin(); it != _virtualMatrices.end(); ++it)
    {
        (*it)->ClearDirty();
    }
}

UserButton* ScheduleOptions::GetButton(const std::string& label) const
{
    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        if (wxString((*it)->GetLabel()).Lower() == wxString(label).Lower())
        {
            return *it;
        }
    }

    return nullptr;
}


#include "ScheduleOptions.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include "UserButton.h"
#include "CommandManager.h"
#include "Projector.h"

ScheduleOptions::ScheduleOptions(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    _sync = node->GetAttribute("Sync", "FALSE") == "TRUE";
    _advancedMode = node->GetAttribute("AdvancedMode", "FALSE") == "TRUE";
    _webAPIOnly = node->GetAttribute("APIOnly", "FALSE") == "TRUE";
    _sendOffWhenNotRunning = node->GetAttribute("SendOffWhenNotRunning", "FALSE") == "TRUE";
    _sendBackgroundWhenNotRunning = node->GetAttribute("SendBackgroundWhenNotRunning", "FALSE") == "TRUE";
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
            _projectors.push_back(new Projector(n));
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
        else if (n->GetName() == "FPPRemote")
        {
            _fppRemotes.push_back(n->GetAttribute("IP").ToStdString());
        }
    }
}

void ScheduleOptions::AddProjector(const std::string& name, const std::string& ip, const std::string& password)
{
    Projector* p = new Projector();
    p->SetName(name);
    p->SetIP(ip);
    p->SetPassword(password);
    _projectors.push_back(p);
}

void ScheduleOptions::AddButton(const std::string& label, const std::string& command, const std::string& parms, char hotkey, const std::string& color)
{
    UserButton* b = new UserButton();
    b->SetLabel(label);
    b->SetCommand(command);
    b->SetParameters(parms);
    b->SetHotkey(hotkey);
    b->SetColor(color);
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
    _sendBackgroundWhenNotRunning = false;
    _advancedMode = false;
}

ScheduleOptions::~ScheduleOptions()
{
    for (auto it = _projectors.begin(); it != _projectors.end(); ++it)
    {
        delete *it;
    }
    _projectors.clear();

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

    if (IsSendBackgroundWhenNotRunning())
    {
        res->AddAttribute("SendBackgroundWhenNotRunning", "TRUE");
    }

    res->AddAttribute("WebServerPort", wxString::Format(wxT("%i"), _port));
    res->AddAttribute("PasswordTimeout", wxString::Format(wxT("%i"), _passwordTimeout));

    for (auto it = _projectors.begin(); it != _projectors.end(); ++it)
    {
        res->AddChild((*it)->Save());
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

    for (auto it = _fppRemotes.begin(); it != _fppRemotes.end(); ++it)
    {
        wxXmlNode* n = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "FPPRemote");
        n->AddAttribute("IP", wxString(*it));
        res->AddChild(n);
    }

    return res;
}

std::list<Projector*> ScheduleOptions::GetProjectors() const
{
    return _projectors;
}

std::vector<UserButton*> ScheduleOptions::GetButtons() const
{
    return _buttons;
}

Projector* ScheduleOptions::GetProjector(const std::string& projector)
{
    for (auto it = _projectors.begin(); it != _projectors.end(); ++it)
    {
        if ((*it)->GetName() == projector)
        {
            return *it;
        }
    }

    return nullptr;
}

void ScheduleOptions::ClearProjectors()
{
    for (auto it = _projectors.begin(); it != _projectors.end(); ++it)
    {
        delete *it;
    }
    _projectors.clear();
    _changeCount++;
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

std::string ScheduleOptions::GetButtonsJSON(const CommandManager &cmdMgr, const std::string& reference) const
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
                res += "{\"label\":\"" + 
                    (*it)->GetLabel() + "\",\"color\":\"" +
                    (*it)->GetColorName() + "\",\"id\":\"" +
                    wxString::Format("%i", (*it)->GetId()).ToStdString() + "\"}";
            }
        }
    }
    res += "],\"reference\":\""+reference+"\"}";

    return res;
}

bool ScheduleOptions::IsDirty() const
{
    bool res = _lastSavedChangeCount != _changeCount;

    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

    for (auto it = _projectors.begin(); it != _projectors.end(); ++it)
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

    for (auto it = _projectors.begin(); it != _projectors.end(); ++it)
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

UserButton* ScheduleOptions::GetButton(wxUint32 id) const
{
    for (auto it = _buttons.begin(); it != _buttons.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            return *it;
        }
    }

    return nullptr;
}

std::string ScheduleOptions::GetDefaultRoot() const
{
    wxString d;
#ifdef __WXMSW__
    d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
#elif __LINUX__
    d = wxStandardPaths::Get().GetDataDir();
    if (!wxDir::Exists(d)) {
        d = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
    }
#else
    d = wxStandardPaths::Get().GetResourcesDir();
#endif
    return d.ToStdString();
}

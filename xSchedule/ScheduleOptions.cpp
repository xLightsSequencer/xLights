#include "ScheduleOptions.h"
#include <wx/xml/xml.h>
#include <wx/wxcrt.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include "UserButton.h"
#include "CommandManager.h"
#include "Projector.h"
#include "../xLights/AudioManager.h"
#include <log4cpp/Category.hh>
#include "events/EventBase.h"
#include "events/EventARTNet.h"
#include "events/EventSerial.h"
#include "events/EventOSC.h"
#include "events/EventFPP.h"
#include "events/EventMIDI.h"
#include "events/EventE131.h"
#include "events/EventData.h"

ScheduleOptions::ScheduleOptions(OutputManager* outputManager, wxXmlNode* node)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _oscOptions = nullptr;
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
    _artNetTimeCodeFormat = wxAtoi(node->GetAttribute("ARTNetTimeCodeFormat", "1"));
    _audioDevice = node->GetAttribute("AudioDevice", "").ToStdString();
    AudioManager::SetAudioDevice(_audioDevice);
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
            _matrices.push_back(new MatrixMapper(outputManager, n));
        }
        else if (n->GetName() == "VMatrix")
        {
            _virtualMatrices.push_back(new VirtualMatrix(outputManager, n));
        }
        else if (n->GetName() == "Events")
        {
            for (auto n2 = n->GetChildren(); n2 != nullptr; n2 = n2->GetNext())
            {
                if (n2->GetName() == "EventE131")
                {
                    _events.push_back(new EventE131(n2));
                }
                else if (n2->GetName() == "EventData")
                {
                    _events.push_back(new EventData(n2));
                }
                else if (n2->GetName() == "EventOSC")
                {
                    _events.push_back(new EventOSC(n2));
                }
                else if (n2->GetName() == "EventFPP")
                {
                    _events.push_back(new EventFPP(n2));
                }
                else if (n2->GetName() == "EventMIDI")
                {
                    _events.push_back(new EventMIDI(n2));
                }
                else if (n2->GetName() == "EventSerial")
                {
                    _events.push_back(new EventSerial(n2));
                }
                else if (n2->GetName() == "EventARTNet")
                {
                    _events.push_back(new EventARTNet(n2));
                }
                else
                {
                    logger_base.warn("Unrecognised event type %s.", (const char *)n2->GetName().c_str());
                }
            }
        }
        else if (n->GetName() == "FPPRemote")
        {
            _fppRemotes.push_back(n->GetAttribute("IP").ToStdString());
        }
        else if (n->GetName() == "OSC")
        {
            _oscOptions = new OSCOptions(n);
        }
    }

    if (_oscOptions == nullptr) _oscOptions = new OSCOptions();
}

void ScheduleOptions::AddProjector(const std::string& name, const std::string& ip, const std::string& password)
{
    Projector* p = new Projector();
    p->SetName(name);
    p->SetIP(ip);
    p->SetPassword(password);
    _projectors.push_back(p);
}

void ScheduleOptions::SetAudioDevice(const std::string& audioDevice)
{
    if (_audioDevice != audioDevice) {
        _audioDevice = audioDevice;
        AudioManager::SetAudioDevice(_audioDevice);
        _changeCount++;
    }
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
    _artNetTimeCodeFormat = 1;
    _oscOptions = new OSCOptions();
    _password = "";
    _passwordTimeout = 30;
    _wwwRoot = "xScheduleWeb";
    _audioDevice = "";
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
    if (_oscOptions != nullptr) delete _oscOptions;
}

wxXmlNode* ScheduleOptions::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Options");

    res->AddAttribute("AudioDevice", _audioDevice);
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
    res->AddAttribute("ARTNetTimeCodeFormat", wxString::Format("%d", _artNetTimeCodeFormat));

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

    wxXmlNode* en = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Events");
    res->AddChild(en);
    for (auto it = _events.begin(); it != _events.end(); ++it)
    {
        en->AddChild((*it)->Save());
    }

    for (auto it = _fppRemotes.begin(); it != _fppRemotes.end(); ++it)
    {
        wxXmlNode* n = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "FPPRemote");
        n->AddAttribute("IP", wxString(*it));
        res->AddChild(n);
    }

    if (_oscOptions != nullptr) res->AddChild(_oscOptions->Save());

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

    for (auto it = _events.begin(); it != _events.end(); ++it)
    {
        res = res || (*it)->IsDirty();
    }

    if (_oscOptions != nullptr) res = res || _oscOptions->IsDirty();

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

    for (auto it = _events.begin(); it != _events.end(); ++it)
    {
        (*it)->ClearDirty();
    }

    if (_oscOptions != nullptr) _oscOptions->ClearDirty();
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

wxXmlNode* OSCOptions::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "OSC");

    res->AddAttribute("MasterPath", _masterPath);
    res->AddAttribute("RemotePath", _remotePath);
    res->AddAttribute("IP", _ipAddress);
    res->AddAttribute("Time", DecodeTime(_time));
    res->AddAttribute("Frame", DecodeFrame(_frame));
    res->AddAttribute("ServerPort", wxString::Format("%d", _serverport));
    res->AddAttribute("ClientPort", wxString::Format("%d", _clientport));
    res->AddAttribute("TimeBased", (IsTime() ? "True" : "False"));
    return res;
}

OSCFRAME OSCOptions::EncodeFrame(std::string frame) const
{
    if (frame == "Default (int)") return OSCFRAME::FRAME_DEFAULT;
    if (frame == "24 fps (int)") return OSCFRAME::FRAME_24;
    if (frame == "25 fps (int)") return OSCFRAME::FRAME_25;
    if (frame == "29.97 fps (int)") return OSCFRAME::FRAME_2997;
    if (frame == "30 fps (int)") return OSCFRAME::FRAME_30;
    if (frame == "60 fps (int)") return OSCFRAME::FRAME_60;
    if (frame == "Progress (float)") return OSCFRAME::FRAME_PROGRESS;

    wxASSERT(false);
    return OSCFRAME::FRAME_DEFAULT;
}

std::string OSCOptions::DecodeFrame(OSCFRAME frame)
{
    switch (frame)
    {
    case OSCFRAME::FRAME_DEFAULT:
        return "Default (int)";
    case OSCFRAME::FRAME_24:
        return "24 fps (int)";
    case OSCFRAME::FRAME_25:
        return "25 fps (int)";
    case OSCFRAME::FRAME_2997:
        return "29.97 fps (int)";
    case OSCFRAME::FRAME_30:
        return "30 fps (int)";
    case OSCFRAME::FRAME_60:
        return "60 fps (int)";
    case OSCFRAME::FRAME_PROGRESS:
        return "Progress (float)";
    }
    wxASSERT(false);
    return "Default (int)";
}

OSCTIME OSCOptions::EncodeTime(std::string time) const
{
    if (time == "Seconds (float)") return OSCTIME::TIME_SECONDS;
    if (time == "Milliseconds (int)") return OSCTIME::TIME_MILLISECONDS;

    wxASSERT(false);
    return OSCTIME::TIME_SECONDS;
}

std::string OSCOptions::DecodeTime(OSCTIME time)
{
    switch(time)
    {
    case OSCTIME::TIME_SECONDS: return "Seconds (float)";
    case OSCTIME::TIME_MILLISECONDS: return "Milliseconds (int)";
    }

    wxASSERT(false);
    return "Seconds (float)";
}

void OSCOptions::Load(wxXmlNode* node)
{
    _masterPath = node->GetAttribute("MasterPath", "/Timecode/%STEPNAME%").ToStdString();
    _remotePath = node->GetAttribute("RemotePath", "/Timecode/%STEPNAME%").ToStdString();
    _ipAddress = node->GetAttribute("IP", "255.255.255.255").ToStdString();
    _time = EncodeTime(node->GetAttribute("Time", "Seconds (float)").ToStdString());
    _frame = EncodeFrame(node->GetAttribute("Frame", "Default (int)").ToStdString());
    _serverport = wxAtoi(node->GetAttribute("ServerPort", "9000"));
    _clientport = wxAtoi(node->GetAttribute("ClientPort", "9000"));
    _time_not_frames = (node->GetAttribute("TimeBased", "True") == "True");
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

OSCOptions::OSCOptions(wxXmlNode* node)
{
    Load(node);
}

OSCOptions::OSCOptions()
{
    _masterPath = "/Timecode/%STEPNAME%";
    _remotePath = "/Timecode/%STEPNAME%";
    _ipAddress = "255.255.255.255";
    _time = EncodeTime("Seconds (float)");
    _frame = EncodeFrame("Default (int)");
    _serverport = 9000;
    _clientport = 9000;
    _time_not_frames = true;
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

#include "SMSDaemonOptions.h"

#include <wx/xml/xml.h>
#include <wx/file.h>
#include <wx/filename.h>
#include "../../xLights/UtilFunctions.h"
#include "MagicWord.h"

#include "spdlog/spdlog.h"
#include <wx/wxcrt.h>

SMSDaemonOptions::SMSDaemonOptions()
{
}

SMSDaemonOptions::~SMSDaemonOptions()
{
    while (_magicWords.size() > 0) {
        delete _magicWords.back();
        _magicWords.pop_back();
    }
}

void SMSDaemonOptions::Load(const std::string& showDir)
{
    wxString options = showDir;
    if (options != "" && !options.EndsWith("/") && !options.EndsWith("\\"))
    {
        options += wxFileName::GetPathSeparator();
    }
    options += "xSMSDaemon.xSMSDaemon";

    if (wxFile::Exists(options))
    {
        wxXmlDocument doc(options);

        if (doc.IsOk())
        {
            spdlog::debug("Options loaded from {}.", (const char*)options.c_str());
            auto n = doc.GetRoot();
            _textItem = n->GetAttribute("TextItem", "");
            _user = n->GetAttribute("User", "");
            _sid = n->GetAttribute("SID", "");
            _token = n->GetAttribute("Token", "");
            _phone = n->GetAttribute("Phone", "");
            _smsService = n->GetAttribute("SMSService", "Bandwidth");
            if (_smsService == "")
            {
                _smsService = "Bandwidth";
            }
            _defaultMessage = n->GetAttribute("DefaultMessage", "");
            _successMessage = n->GetAttribute("SuccessMessage", "");
            _rejectMessage = n->GetAttribute("RejectMessage", "");

            _retrieveInterval = wxAtoi(n->GetAttribute("RetrieveInterval", "10"));
            _displayDuration = wxAtoi(n->GetAttribute("DisplayDuration", "30"));
            _maximiumMessageLength = wxAtoi(n->GetAttribute("MaxMsgLen", "100"));
            _maximiumTimesToDisplay = wxAtoi(n->GetAttribute("MaxDisplays", "0"));
            _maxMsgAgeMinsForResponse = wxAtoi(n->GetAttribute("MaxMsgAgeMinsForResponse", "10"));
            _maximiumMessageAge = wxAtoi(n->GetAttribute("MaxMsgAge", "10"));
            _timezoneAdjust = wxAtoi(n->GetAttribute("TimezoneAdjust", "0"));

            _rejectProfanity = n->GetAttribute("RejectProfanity", "TRUE") == "TRUE";
            _usePurgoMalum = n->GetAttribute("UsePurgoMalum", "FALSE") == "TRUE";
            _usePhoneBlacklist = n->GetAttribute("UsePhoneBlacklist", "FALSE") == "TRUE";
            _useLocalBlacklist = n->GetAttribute("UseLocalBlacklist", "FALSE") == "TRUE";
            _useLocalWhitelist = n->GetAttribute("UseLocalWhitelist", "FALSE") == "TRUE";
            _acceptOneWordOnly = n->GetAttribute("AcceptOneWordOnly", "FALSE") == "TRUE";
            _manualModeration = n->GetAttribute("ManualModeration", "FALSE") == "TRUE";
            _ignoreOversizedMessages = n->GetAttribute("IgnoreOversizedMessages", "FALSE") == "TRUE";
            _upperCase = n->GetAttribute("UpperCase", "FALSE") == "TRUE";

            for (wxXmlNode* nn = n->GetChildren(); nn != nullptr; nn = nn->GetNext()) {
                if (nn->GetName() == "MagicWords") {
                    for (wxXmlNode* nnn = nn->GetChildren(); nnn != nullptr; nnn = nnn->GetNext()) {
                        if (nnn->GetName() == "MagicWord") {
                            _magicWords.push_back(new MagicWord(nnn));
                        }
                    }
                }
            }
        }
        else
        {
            spdlog::debug("Options xml file not valid XML {}.", (const char*)options.c_str());
        }
    }
    else
    {
        spdlog::debug("Options did not exist {}.", (const char*)options.c_str());
    }
}

void SMSDaemonOptions::Save(const std::string& showDir)
{
    wxString options = showDir;
    if (options != "" && !options.EndsWith("/") && !options.EndsWith("\\"))
    {
        options += wxFileName::GetPathSeparator();
    }
    options += "xSMSDaemon.xSMSDaemon";

    wxXmlDocument doc;

    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "SMSDaemon");

    node->AddAttribute("TextItem", _textItem);
    node->AddAttribute("User", _user);
    node->AddAttribute("SID", _sid);
    node->AddAttribute("Token", _token);
    node->AddAttribute("Phone", _phone);
    node->AddAttribute("SMSService", _smsService);
    wxString def = _defaultMessage;
    if (def.StartsWith('\b'))
    {
        def = def.substr(1);
    }
    node->AddAttribute("DefaultMessage", def);
    node->AddAttribute("SuccessMessage", _successMessage);
    node->AddAttribute("RejectMessage", _rejectMessage);

    node->AddAttribute("RetrieveInterval", wxString::Format("%d", _retrieveInterval));
    node->AddAttribute("DisplayDuration", wxString::Format("%d", _displayDuration));
    node->AddAttribute("MaxMsgLen", wxString::Format("%d", _maximiumMessageLength));
    node->AddAttribute("MaxMsgAge", wxString::Format("%d", _maximiumMessageAge));
    node->AddAttribute("TimezoneAdjust", wxString::Format("%d", _timezoneAdjust));
    node->AddAttribute("MaxDisplays", wxString::Format("%d", _maximiumTimesToDisplay));
    node->AddAttribute("MaxMsgAgeMinsForResponse", wxString::Format("%d", _maxMsgAgeMinsForResponse));

    if (!_rejectProfanity) node->AddAttribute("RejectProfanity", "FALSE");
    if (_usePurgoMalum) node->AddAttribute("UsePurgoMalum", "TRUE");
    if (_usePhoneBlacklist) node->AddAttribute("UsePhoneBlacklist", "TRUE");
    if (_useLocalBlacklist) node->AddAttribute("UseLocalBlacklist", "TRUE");
    if (_useLocalWhitelist) node->AddAttribute("UseLocalWhitelist", "TRUE");
    if (_acceptOneWordOnly) node->AddAttribute("AcceptOneWordOnly", "TRUE");
    if (_manualModeration) node->AddAttribute("ManualModeration", "TRUE");
    if (_ignoreOversizedMessages) node->AddAttribute("IgnoreOversizedMessages", "TRUE");
    if (_upperCase) node->AddAttribute("UpperCase", "TRUE");

    wxXmlNode* mws = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "MagicWords");
    node->AddChild(mws);
    for (const auto& it : _magicWords) {
        it->Save(mws);
    }

    doc.SetRoot(node);
    spdlog::debug("Options saved to {}.", (const char*)options.c_str());
    doc.Save(options);
    ClearDirty();
}

bool SMSDaemonOptions::IsValid() const
{
    if (_smsService == "Bandwidth")
    {
        if (_sid == "" || _token == "" || _textItem == "" || _user == "") return false;
    }
    else if (_smsService == "Voip.ms")
    {
        if (_sid == "" || _textItem == "" || _user == "") return false;
    }
    return true;
}

void SMSDaemonOptions::SetDefaultMessage(std::string defaultMessage) 
{ 
    wxString def = defaultMessage;
    if (def.StartsWith('\b'))
    {
        def = def.substr(1);
    }
    if (def != _defaultMessage) 
    { 
        _defaultMessage = def; _changeCount++; 
    } 
}

bool SMSDaemonOptions::IsDirty() const
{
    return _lastSavedChangeCount != _changeCount;
}

void SMSDaemonOptions::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;
}

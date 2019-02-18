#include "SMSDaemonOptions.h"

#include <wx/xml/xml.h>
#include <wx/file.h>
#include "../../xLights/UtilFunctions.h"

#include <log4cpp/Category.hh>
#include <wx/wxcrt.h>

SMSDaemonOptions::SMSDaemonOptions()
{
}

void SMSDaemonOptions::Load(const std::string& showDir)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString options = showDir;
    if (options != "" && !options.EndsWith("/") && !options.EndsWith("\\"))
    {
        options += "/";
    }
    options += "xSMSDaemon.xSMSDaemon";

    if (wxFile::Exists(options))
    {
        wxXmlDocument doc(options);

        if (doc.IsOk())
        {
            logger_base.debug("Options loaded from %s.", (const char *)options.c_str());
            auto n = doc.GetRoot();
            _xScheduleIP = n->GetAttribute("xScheduleIP", "127.0.0.1");
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
            _maximiumMessageAge = wxAtoi(n->GetAttribute("MaxMsgAge", "10"));
            _xSchedulePort = wxAtoi(n->GetAttribute("xSchedulePort", "80"));

            _rejectProfanity = n->GetAttribute("RejectProfanity", "TRUE") == "TRUE";
            _usePurgoMalum = n->GetAttribute("UsePurgoMalum", "FALSE") == "TRUE";
            _usePhoneBlacklist = n->GetAttribute("UsePhoneBlacklist", "FALSE") == "TRUE";
            _useLocalBlacklist = n->GetAttribute("UseLocalBlacklist", "FALSE") == "TRUE";
            _useLocalWhitelist = n->GetAttribute("UseLocalWhitelist", "FALSE") == "TRUE";
            _acceptOneWordOnly = n->GetAttribute("AcceptOneWordOnly", "FALSE") == "TRUE";
            _ignoreOversizedMessages = n->GetAttribute("IgnoreOversizedMessages", "FALSE") == "TRUE";
            _upperCase = n->GetAttribute("UpperCase", "FALSE") == "TRUE";
        }
        else
        {
            logger_base.debug("Options xml file not valid XML %s.", (const char *)options.c_str());
        }
    }
    else
    {
        logger_base.debug("Options did not exist %s.", (const char *)options.c_str());
    }
}

void SMSDaemonOptions::Save(const std::string& showDir)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString options = showDir;
    if (options != "" && !options.EndsWith("/") && !options.EndsWith("\\"))
    {
        options += "/";
    }
    options += "xSMSDaemon.xSMSDaemon";

    wxXmlDocument doc;

    wxXmlNode * node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "SMSDaemon");

    node->AddAttribute("xScheduleIP", _xScheduleIP);
    node->AddAttribute("TextItem", _textItem);
    node->AddAttribute("User", _user);
    node->AddAttribute("SID", _sid);
    node->AddAttribute("Token", _token);
    node->AddAttribute("Phone", _phone);
    node->AddAttribute("SMSService", _smsService);
    node->AddAttribute("DefaultMessage", _defaultMessage);
    node->AddAttribute("SuccessMessage", _successMessage);
    node->AddAttribute("RejectMessage", _rejectMessage);

    node->AddAttribute("RetrieveInterval", wxString::Format("%d", _retrieveInterval));
    node->AddAttribute("DisplayDuration", wxString::Format("%d", _displayDuration));
    node->AddAttribute("MaxMsgLen", wxString::Format("%d", _maximiumMessageLength));
    node->AddAttribute("MaxMsgAge", wxString::Format("%d", _maximiumMessageAge));
    node->AddAttribute("MaxDisplays", wxString::Format("%d", _maximiumTimesToDisplay));
    node->AddAttribute("xSchedulePort", wxString::Format("%d", _xSchedulePort));

    if (!_rejectProfanity) node->AddAttribute("RejectProfanity", "FALSE");
    if (_usePurgoMalum) node->AddAttribute("UsePurgoMalum", "TRUE");
    if (_usePhoneBlacklist) node->AddAttribute("UsePhoneBlacklist", "TRUE");
    if (_useLocalBlacklist) node->AddAttribute("UseLocalBlacklist", "TRUE");
    if (_useLocalWhitelist) node->AddAttribute("UseLocalWhitelist", "TRUE");
    if (_acceptOneWordOnly) node->AddAttribute("AcceptOneWordOnly", "TRUE");
    if (_ignoreOversizedMessages) node->AddAttribute("IgnoreOversizedMessages", "TRUE");
    if (_upperCase) node->AddAttribute("UpperCase", "TRUE");

    doc.SetRoot(node);
    logger_base.debug("Options saved to %s.", (const char *)options.c_str());
    doc.Save(options);
    ClearDirty();
}

bool SMSDaemonOptions::IsValid() const
{
    if (!IsIPValid(_xScheduleIP)) return false;

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

bool SMSDaemonOptions::IsDirty() const
{
    return _lastSavedChangeCount != _changeCount;
}

void SMSDaemonOptions::ClearDirty()
{
    _lastSavedChangeCount = _changeCount;
}
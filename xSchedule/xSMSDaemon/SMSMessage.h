#ifndef SMSMESSAGE_H
#define SMSMESSAGE_H

#include <locale>
#include <fstream>

#include "utils/Curl.h"

#include "spdlog/spdlog.h"

#include <wx/wx.h>
#include <wx/uri.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <set>
#include <string>

class SMSMessage
{
    static std::set<std::string> _phoneBlacklist;
    static std::set<std::string> _blacklist;
    static std::set<std::string> _whitelist;
    static int __nextId;

    public:
    int _id;
    wxDateTime _timestamp;
	std::string _rawMessage;
	std::string _message;
	std::wstring _wmessage;
	std::string _from;
    int _displayCount = 0;
    bool _displayed = false;
    bool _filtered = false;
    bool _moderatedOk = false;
    int _moderatedOkCount = 0;

    SMSMessage() {
        _id = __nextId++;
    }

    bool Censored() const {
        return _filtered || _rawMessage == "" || _message != "";
    }

    bool IsModeratedOk() const {
        return _moderatedOk;
    }

    void SetModeratedOk(bool moderatedOk) {
        _moderatedOk = moderatedOk;
        if (_moderatedOk) {
            _moderatedOkCount++;
        }
    }

    int IsFirstModeratedOk() const {
        return _moderatedOkCount == 1 && _moderatedOk;
    }

    void Censor(bool reject);

    wxString GetLog() const {
        return wxString::Format("%s: %s %s", _from, _timestamp.FromTimezone(wxDateTime::TZ::GMT0).FormatISOCombined(), _rawMessage);
    }

    std::string GetStatus() const {
        return wxString::Format("Age %d mins, Displayed %d", GetAgeMins(), _displayCount);
    }

    void Display() {
        _displayCount++;
        _displayed = true;
    }

    wxString GetUIMessage() const {
        if (!_wmessage.empty()) {
            return wxString(_from) + ": " + wxString(_wmessage);
        } else {
            return _from + ": " + _message;
        }
    }

    int GetId() const const {
        return _id;
    }

    int GetAgeMins() const;

    bool operator==(const int i) const {
        return _id == i;
    }

    bool operator==(const SMSMessage& other) const {
        return _timestamp == other._timestamp && _rawMessage == other._rawMessage;
    }

    bool operator<(const SMSMessage& other) const {
        if (_displayCount < other._displayCount) {
            return true;
        } else if (_displayCount == other._displayCount) {
            return (_timestamp < other._timestamp);
        }
        return false;
    }

    static void LoadList(std::set<std::string>& set, const wxFileName& file);

    static void LoadBlackList();

    static void LoadPhoneBlackList();

    static void LoadWhiteList();

    bool PassesBlacklist() const;

    bool PassesPhoneBlacklist() const;

    bool PassesWhitelist() const;
};
#endif

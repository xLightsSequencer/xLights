#ifndef SMSMESSAGE_H
#define SMSMESSAGE_H

#include <locale>
#include <fstream>

#include "utils/Curl.h"

#include <wx/wx.h>
#include <wx/uri.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include "../../xLights/UtilFunctions.h"

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

    SMSMessage()
    {
        _id = __nextId++;
    }

    bool Censored() const
    {
        return _filtered || _rawMessage == "" || _message != "";
    }

    bool IsModeratedOk() const { return _moderatedOk; }
    void SetModeratedOk(bool moderatedOk) 
    { 
        _moderatedOk = moderatedOk; 
        if (_moderatedOk)
        {
            _moderatedOkCount++;
        }
    }
    int IsFirstModeratedOk() const { return _moderatedOkCount == 1 && _moderatedOk; }
    void Censor(bool reject)
    {
        if (reject)
        {
            wxURI url("https://www.purgomalum.com/service/containsprofanity?text=" + _rawMessage);
            auto msg = Curl::HTTPSGet(url.BuildURI().ToStdString());
            if (msg == "false")
            {
                _message = _rawMessage;
            }
            else
            {
                _filtered = true;
            }
        }
        else
        {
            wxURI url("https://www.purgomalum.com/service/xml?text=" + _rawMessage);
            auto msg = Curl::HTTPSGet(url.BuildURI().ToStdString());

            wxRegEx regex("result>([^<]*)<\\/result");
            regex.Matches(msg);
            _message = regex.GetMatch(msg, 1);
        }
    }

    wxString GetLog() const
    {
        return wxString::Format("%s: %s %s", _from, _timestamp.FromTimezone(wxDateTime::TZ::GMT0).FormatISOCombined(), _rawMessage);
    }

    std::string GetStatus() const
    {
        return wxString::Format("Age %d mins, Displayed %d", GetAgeMins(), _displayCount);
    }

    void Display()
    {
        _displayCount++;
        _displayed = true;
    }

    wxString GetUIMessage() const
    {
        if (_wmessage != "")
        {
            return wxString(_from) + ": " + wxString(_wmessage);
        }
        else
        {
            return _from + ": " + _message;
        }
    }

    int GetId() const
    {
        return _id;
    }

    int GetAgeMins() const
    {
        wxTimeSpan age = wxDateTime::Now().MakeGMT() - _timestamp;
        return age.GetDays() * 24 * 60 + age.GetHours() * 60 + age.GetMinutes();
    }

    bool operator==(const int i) const
    {
        return _id == i;
    }

	bool operator==(const SMSMessage& other) const
	{
		return _timestamp == other._timestamp && _rawMessage == other._rawMessage;
	}

    bool operator<(const SMSMessage& other) const
    {
        if (_displayCount < other._displayCount)
        {
            return true;
        }
        else if (_displayCount == other._displayCount)
        {
            return (_timestamp < other._timestamp);
        }
        return false;
    }

    static void LoadList(std::set<std::string>& set, const wxFileName& file)
    {
        std::ifstream t;
        t.open(file.GetFullPath().ToStdString());
        std::string line;
        while (t) {
            std::getline(t, line);
            set.insert(Lower(line));
        }
        t.close();
    }

    static void LoadBlackList()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (_blacklist.size() > 0) return;

        wxFileName fn("Blacklist.txt");

        if (!fn.Exists())
        {
            logger_base.error("Blacklist file not found %s.", (const char *)fn.GetFullPath().c_str());
            return;
        }

        LoadList(_blacklist, fn);
    }

    static void LoadPhoneBlackList()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (_phoneBlacklist.size() > 0) return;

        wxFileName fn("PhoneBlacklist.txt");

        if (!fn.Exists())
        {
            logger_base.error("Phone Blacklist file not found %s.", (const char *)fn.GetFullPath().c_str());
            return;
        }

        LoadList(_phoneBlacklist, fn);
    }

    static void LoadWhiteList()
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        if (_whitelist.size() > 0) return;

        wxFileName fn("Whitelist.txt");

        if (!fn.Exists())
        {
            logger_base.error("Whitelist file not found %s.", (const char *)fn.GetFullPath().c_str());
            return;
        }

        LoadList(_whitelist, fn);
    }

    bool PassesBlacklist() const
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        LoadBlackList();

        wxStringTokenizer tkz(_rawMessage, wxT(" ,;:.)([]\\/<>-_*&^%$#~`\"?"));
        while (tkz.HasMoreTokens())
        {
            wxString token = tkz.GetNextToken().Lower();
            if (_blacklist.find(token.ToStdString()) != _blacklist.end())
            {
                logger_base.debug("Blacklist failed on '%s'", (const char*)token.c_str());
                return false;
            }
        }
        return true;
    }

    bool PassesPhoneBlacklist() const
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        LoadPhoneBlackList();

        if (_phoneBlacklist.find(_from) == _phoneBlacklist.end())
        {
            return true;
        }
        logger_base.debug("Phone blacklist failed on '%s'", (const char*)_from.c_str());
        return false;
    }

    bool PassesWhitelist() const
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        LoadWhiteList();

        wxStringTokenizer tkz(_rawMessage, wxT(" ,;:.!|)([]\\/<>-_*&^%$#@~`\"?"));
        while (tkz.HasMoreTokens())
        {
            wxString token = tkz.GetNextToken().Lower();
            if (_whitelist.find(token.ToStdString()) == _whitelist.end())
            {
                logger_base.debug("Whitelist failed on '%s'", (const char*)token.c_str());
                return false;
            }
        }
        return true;
    }
};
#endif

#ifndef TWILIO_H
#define TWILIO_H

#include <string>
#include <vector>

#include "Curl.h"

#include <wx/wx.h>
#include <wx/uri.h>
#include <wx/xml/xml.h>
#include "../../xLights/UtilFunctions.h"
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>
#include <fstream>
#include <iostream>
#include "../wxJSON/jsonreader.h"
#include <codecvt>
#include <locale>

//#define STANDALONE

struct SMSMessage
{
	wxDateTime _timestamp;
	std::string _rawMessage;
	std::string _message;
	std::wstring _wmessage;
	std::string _from;
    int _displayCount = 0;

    bool Censored() const
    {
        return _rawMessage == "" || _message != "";
    }

    void Censor()
    {
        wxURI url("https://www.purgomalum.com/service/xml?text=" + _rawMessage);

        auto msg = Curl::HTTPSGet(url.BuildURI().ToStdString());

        wxRegEx regex("result>([^<]*)<\\/result");
        regex.Matches(msg);
        _message = regex.GetMatch(msg, 1);
    }

    std::string GetStatus() const
    {
        return wxString::Format("Age %d mins, Displayed %d", GetAgeMins(), _displayCount);
    }

    int GetAgeMins() const
    {
        wxTimeSpan age = wxDateTime::Now().MakeGMT() - _timestamp;
        return age.GetDays() * 24 * 60 + age.GetHours() * 60 + age.GetMinutes();
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
};

class Bandwidth
{
	const std::string BANDWIDTH_API_URL = "https://api.catapult.inetwork.com/v1/users/{user}";
    std::string _user;
    std::string _sid;
	std::string _token;
	std::string _myNumber;
	std::vector<SMSMessage> _messages;
    std::set<std::string> _blacklist;
    std::set<std::string> _whitelist;

    public:

        Bandwidth() {}
        void SetUser(const std::string& user) { _user = user; }
        void SetSID(const std::string& sid) { _sid = sid; }
        void SetToken(const std::string& token) { _token = token; }
        void SetPhone(const std::string& phone) { _myNumber = phone; }
		bool SendSMS(const std::string& number, const std::string& message) const
		{
		    //std::vector<Curl::Var> vars;
			//vars.push_back(Curl::Var("To", number));
			//vars.push_back(Curl::Var("From", _myNumber));
			//vars.push_back(Curl::Var("Body", message));
            //std::string url = TWILIO_API_URL;
            //Replace(url, "{sid}", _sid);
            //Replace(url, "{user}", _user);
            //Replace(url, "{token}", _token);
		    //url += "/SMS/Messages";
			//std::string res = Curl::HTTPSPost(url, vars);
            //return res != "";
            return true;
		}

        void LoadList(std::set<std::string>& set, const wxFileName& file)
		{
            std::filebuf fb;
            if (fb.open(file.GetFullPath().ToStdString(), std::ios::in))
            {
                std::istream is(&fb);
                while (is)
                {
                    //set.emplace(is.get());
                }
                fb.close();
            }
		}

        void LoadBlackList()
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

        void LoadWhiteList()
		{
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            if (_blacklist.size() > 0) return;

            wxFileName fn("Whitelist.txt");

            if (!fn.Exists())
            {
                logger_base.error("Whitelist file not found %s.", (const char *)fn.GetFullPath().c_str());
                return;
            }

            LoadList(_whitelist, fn);
		}

        bool PassesBlacklist(const std::string& msg)
		{
            LoadBlackList();

            wxStringTokenizer tkz(msg, wxT(" ,;:.)([]\\/<>-_*&^%$#~`\"?"));
            while (tkz.HasMoreTokens())
            {
                wxString token = tkz.GetNextToken().Lower();
                if (_blacklist.find(token) != _blacklist.end())
                {
                    return false;
                }
            }
            return true;
		}

        bool PassesWhitelist(const std::string& msg)
		{
            LoadWhiteList();

            wxStringTokenizer tkz(msg, wxT(" ,;:.!|)([]\\/<>-_*&^%$#@~`\"?"));
            while (tkz.HasMoreTokens())
            {
                wxString token = tkz.GetNextToken().Lower();
                if (_whitelist.find(token) == _whitelist.end())
                {
                    return false;
                }
            }
            return true;
        }

        std::vector<SMSMessage>& GetMessages(int maxAgeMins)
		{
            // remove any messages that are too old
            _messages.erase(std::remove_if(_messages.begin(),
                _messages.end(),
                [maxAgeMins](SMSMessage& x) {return x.GetAgeMins() >= maxAgeMins && maxAgeMins != 0; }),
                _messages.end());

		    // put the next one to display at the front
            std::sort(_messages.begin(), _messages.end());

		    return _messages;
		}
		bool RetrieveMessages(int maxAgeMins, int maxMessageLen, bool ignoreOversize, bool useWhitelist, bool useBlacklist, bool oneWord, bool upperCase)
		{
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            bool added = false;
#ifdef STANDALONE
            for (int i = 0; i < 10; i++)
            {
                SMSMessage msg;
                msg._timestamp = wxDateTime::Now() - wxTimeSpan(0, rand01() * 60);
                msg._rawMessage = "Test " + msg._timestamp.FormatTime();

                if (!useBlacklist || PassesBlacklist(msg._rawMessage))
                {
                    if (!useWhitelist || PassesWhitelist(msg._rawMessage))
                    {
                        // Only add if not too old
                        if (msg.GetAgeMins() < maxAgeMins)
                        {
                            // Only add if not already there
                            bool found = false;
                            for (auto it : _messages)
                            {
                                if (it == msg)
                                {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                            {
                                msg.Censor();

                                if (maxMessageLen != 0 && msg._message.size() > maxMessageLen && !ignoreOversize)
                                {
                                    msg._message = msg._message.substr(0, maxMessageLen);
                                }

                                // messages have to be under the max
                                if (maxMessageLen == 0 || msg._message.size() <= maxMessageLen)
                                {
                                    if (!oneWord || msg._message.find(" ") == std::string::npos)
                                    {
                                        _messages.push_back(msg);
                                        added = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
#else
            // /2010-04-01/Accounts/{AccountSid}/SMS/Messages
            // /2010-04-01/Accounts/{AccountSid}/Calls.csv
			//std::string url = TWILIO_API_URL + "/"+ API_VERSION + "/Accounts/" + _sid + "/Calls.csv";
			//std::string res = Curl::HTTPSGet(url);

            std::string url = BANDWIDTH_API_URL;
            Replace(url, "{sid}", _sid);
            Replace(url, "{user}", _user);
            Replace(url, "{token}", _token);
            url += "/messages?page=0&size=100";
			std::string res = Curl::HTTPSGet(url, _sid, _token);
            logger_base.debug("%s", (const char*)url.c_str());
            logger_base.debug("%s", (const char*)res.c_str());

            // construct the JSON root object
            wxJSONValue  root;

            // construct a JSON parser
            wxJSONReader reader;

            // now read the JSON text and store it in the 'root' structure
            // check for errors before retreiving values...

            // strip out unicode characters as they cause a crash
            Replace(res, "\\u", "!!u!!");

            int numErrors = reader.Parse(res, &root);
            if (numErrors > 0) {
                logger_base.error("The JSON document is not well-formed: %s", (const char*)res.c_str());
            }
            else
            {
                if (root.IsArray())
                {
                    for (int i = 0; i < root.Size(); i++)
                    {
                        wxJSONValue &m = root[i];

                        wxJSONValue defaultValue = wxString("");
                        SMSMessage msg;
                        auto timestamp = m.Get("time", defaultValue).AsString();
                        msg._timestamp.ParseISOCombined(timestamp);
                        msg._from = m.Get("from", defaultValue).AsString().ToStdString();
                        msg._rawMessage = m.Get("text", defaultValue).AsString().ToStdString();

                        if (!useBlacklist || PassesBlacklist(msg._rawMessage))
                        {
                            if (!useWhitelist || PassesWhitelist(msg._rawMessage))
                            {
                                // Only add if not too old
                                if (maxAgeMins == 0 || msg.GetAgeMins() < maxAgeMins)
                                {
                                    // Only add if not already there
                                    bool found = false;
                                    for (auto it : _messages)
                                    {
                                        if (it == msg)
                                        {
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found)
                                    {
                                        msg.Censor();

                                        if (maxMessageLen != 0 && msg._message.size() > maxMessageLen && !ignoreOversize)
                                        {
                                            msg._message = msg._message.substr(0, maxMessageLen);
                                        }

                                        // messages have to be under the max
                                        if (maxMessageLen == 0 || msg._message.size() <= maxMessageLen)
                                        {
                                            if (!oneWord || msg._message.find(" ") == std::string::npos)
                                            {
                                                if (upperCase)
                                                {
                                                    msg._message = wxString(msg._message).Upper().ToStdString();
                                                }

                                                if (Contains(msg._message, "!!u!!") || Contains(msg._message, "!!U!!"))
                                                {
                                                    auto s = msg._message;
                                                    Replace(s, "!!u!!", "\\u");
                                                    Replace(s, "!!U!!", "\\u");
                                                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                                                    std::wstring wide = converter.from_bytes(s);
                                                    int a = 0;
                                                    std::wstring w;
                                                    std::string utf;
                                                    int state = 0;
                                                    for (auto c : s)
                                                    {
                                                        if (state == 1)
                                                        {
                                                            if (c == 'u')
                                                            {
                                                                state = 2;
                                                            }
                                                            else
                                                            {
                                                                w += '\\';
                                                                w += c;
                                                            }
                                                        }
                                                        else if (state == 2)
                                                        {
                                                            utf += c;
                                                            if (utf.size() == 4)
                                                            {
                                                                int u;
                                                                sscanf(utf.c_str(), "%x", &u);
                                                                w += wchar_t(u);
                                                                state = 0;
                                                                utf = "";
                                                            }
                                                        }
                                                        else if (state == 0)
                                                        {
                                                            if (c == '\\')
                                                            {
                                                                state = 1;
                                                                utf = "";
                                                            }
                                                            else
                                                            {
                                                                w += c;
                                                            }
                                                        }
                                                    }
                                                    msg._wmessage = w;
                                                }

                                                _messages.push_back(msg);
                                                added = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
#endif

            // remove any messages that are too old
            _messages.erase(std::remove_if(_messages.begin(),
                _messages.end(),
                [maxAgeMins](SMSMessage& x) {return x.GetAgeMins() >= maxAgeMins && maxAgeMins != 0; }),
                _messages.end());

            // put the next one to display at the front
            std::sort(_messages.begin(), _messages.end());

			return added;
		}
};

#endif

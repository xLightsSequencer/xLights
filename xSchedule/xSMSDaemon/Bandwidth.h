#ifndef BANDWIDTH_H
#define BANDWIDTH_H

#include <string>
#include <vector>
#include <codecvt>
#include <locale>

#include "Curl.h"

#include <wx/wx.h>

#include "../../xLights/UtilFunctions.h"
#include "../wxJSON/jsonreader.h"
#include "SMSMessage.h"
#include "SMSService.h"

class Bandwidth : public SMSService
{
	const std::string BANDWIDTH_API_URL = "https://api.catapult.inetwork.com/v1/users/{user}";
    const wxString body = "{\"from\":\"{phone}\",\"to\":\"{tophone}\",\"text\":\"{message}\"}";

    public:

        Bandwidth() : SMSService() {}

        virtual bool SendSMS(const std::string& number, const std::string& message) const override
		{
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            std::string url = BANDWIDTH_API_URL;
            Replace(url, "{sid}", _sid);
            Replace(url, "{user}", _user);
            Replace(url, "{token}", _token);
            url += "/messages";

            wxString b = body;
            b.Replace("{phone}", _myNumber);
            b.Replace("{tophone}", number);
            b.Replace("{message}", message);

            std::string res = Curl::HTTPSPost(url, b, _sid, _token);
            logger_base.debug("%s", (const char*)url.c_str());
            logger_base.debug("%s", (const char*)res.c_str());
            return true;
		}

        virtual std::string GetServiceName() const override { return "Bandwidth"; }
        virtual bool RetrieveMessages(int maxAgeMins, int maxMessageLen, bool ignoreOversize, bool useWhitelist, bool useBlacklist, bool usePhoneBlacklist, bool oneWord, bool upperCase, bool rejectCensored, const wxString& successMessage, const wxString& rejectMessage) override
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

            auto xx = Curl::HTTPSPost("127.0.0.1:80/xScheduleStash", "body", "", "");

            bool added = false;

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
                                if (!usePhoneBlacklist || msg.PassesPhoneBlacklist())
                                {
                                    if (!useBlacklist || msg.PassesBlacklist())
                                    {
                                        if (!useWhitelist || msg.PassesWhitelist())
                                        {
                                            msg.Censor(rejectCensored);

                                            if (msg._message != "")
                                            {
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
                                                        logger_base.info("Accepted Msg: %s", (const char*)msg.GetLog().c_str());
                                                        SendSuccessMessage(msg, successMessage);
                                                    }
                                                    else
                                                    {
                                                        logger_base.warn("Rejected Msg: Not one word : %s", (const char*)msg.GetLog().c_str());
                                                        SendRejectMessage(msg, rejectMessage);
                                                    }
                                                }
                                                else
                                                {
                                                    logger_base.warn("Rejected Msg: Too long : %s", (const char*)msg.GetLog().c_str());
                                                    SendRejectMessage(msg, rejectMessage);
                                                }
                                            }
                                            else
                                            {
                                                if (rejectCensored)
                                                {
                                                    logger_base.warn("Rejected Msg: Censored : %s", (const char*)msg.GetLog().c_str());
                                                    SendRejectMessage(msg, rejectMessage);
                                                }
                                            }
                                        }
                                        else
                                        {
                                            logger_base.warn("Rejected Msg: Whitelist : %s", (const char*)msg.GetLog().c_str());
                                            SendRejectMessage(msg, rejectMessage);
                                        }
                                    }
                                    else
                                    {
                                        logger_base.warn("Rejected Msg: Blacklist : %s", (const char*)msg.GetLog().c_str());
                                        SendRejectMessage(msg, rejectMessage);
                                    }
                                }
                                else
                                {
                                    logger_base.warn("Rejected Msg: Phone Blacklist : %s", (const char*)msg.GetLog().c_str());
                                    SendRejectMessage(msg, rejectMessage);
                                }
                            }
                            else
                            {
                                // we already have this message but dont log
                            }
                        }
                        else
                        {
                            // too old but dont log
                        }
                    }
                }
            }

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

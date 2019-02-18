#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <vector>
#include <codecvt>

#include <wx/uri.h>
#include "SMSMessage.h"
#include "SMSDaemonOptions.h"

class SMSService
{
    protected:

    std::string _user;
    std::string _sid;
	std::string _token;
	std::string _myNumber;
	std::vector<SMSMessage> _messages;

    public:

        SMSService() {}
        virtual ~SMSService() {}
        void SetUser(const std::string& user) { _user = user; }
        void SetSID(const std::string& sid) { _sid = sid; }
        void SetToken(const std::string& token) { _token = token; }
        void SetPhone(const std::string& phone) { _myNumber = phone; }
        void PrepareMessages(int maxAgeMins)
        {
            // remove any messages that are too old
            _messages.erase(std::remove_if(_messages.begin(),
                _messages.end(),
                [maxAgeMins](SMSMessage& x) {return maxAgeMins != 0 && x.GetAgeMins() >= maxAgeMins; }),
                _messages.end());

            // put the next one to display at the front
            std::sort(_messages.begin(), _messages.end());
        }
        std::vector<SMSMessage>& GetMessages()
		{
		    return _messages;
		}
        void Reset()
        {
            _messages.clear();
        }
        virtual std::string GetServiceName() const = 0;
        virtual bool SendSMS(const std::string& number, const std::string& message) const = 0;
        virtual bool RetrieveMessages(const SMSDaemonOptions& options) = 0;
        void SendSuccessMessage(const SMSMessage& msg, const wxString& successMessage) const
        {
            if (successMessage != "")
            {
                SendSMS(msg._from, successMessage);
            }
        }
        void SendRejectMessage(const SMSMessage& msg, const wxString& rejectMessage) const
        {
            if (rejectMessage != "")
            {
                SendSMS(msg._from, rejectMessage);
            }
        }
        bool AddMessage(SMSMessage& msg, const SMSDaemonOptions& options)
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            bool added = false;

            int maxAgeMins = options.GetMaxMessageAge();

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
                    if (!options.GetUsePhoneBlacklist() || msg.PassesPhoneBlacklist())
                    {
                        if (!options.GetUseLocalBlacklist() || msg.PassesBlacklist())
                        {
                            if (!options.GetUseLocalWhitelist() || msg.PassesWhitelist())
                            {
                                msg.Censor(options.GetRejectProfanity());

                                if (msg._message != "")
                                {
                                    int maxMessageLen = options.GetMaxMessageLength();
                                    if (maxMessageLen != 0 && msg._message.size() > maxMessageLen && !options.GetIgnoreOversizedMessages())
                                    {
                                        msg._message = msg._message.substr(0, maxMessageLen);
                                    }

                                    // messages have to be under the max
                                    if (maxMessageLen == 0 || msg._message.size() <= maxMessageLen)
                                    {
                                        if (!options.GetAcceptOneWordOnly() || msg._message.find(" ") == std::string::npos)
                                        {
                                            if (options.GetUpperCase())
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
                                            if (msg._from != _myNumber)
                                            {
                                                SendSuccessMessage(msg, options.GetSuccessMessage());
                                            }
                                        }
                                        else
                                        {
                                            logger_base.warn("Rejected Msg: Not one word : %s", (const char*)msg.GetLog().c_str());
                                            SendRejectMessage(msg, options.GetRejectMessage());
                                        }
                                    }
                                    else
                                    {
                                        logger_base.warn("Rejected Msg: Too long : %s", (const char*)msg.GetLog().c_str());
                                        SendRejectMessage(msg, options.GetRejectMessage());
                                    }
                                }
                                else
                                {
                                    if (options.GetRejectProfanity())
                                    {
                                        logger_base.warn("Rejected Msg: Censored : %s", (const char*)msg.GetLog().c_str());
                                        SendRejectMessage(msg, options.GetRejectMessage());
                                    }
                                }
                            }
                            else
                            {
                                logger_base.warn("Rejected Msg: Whitelist : %s", (const char*)msg.GetLog().c_str());
                                SendRejectMessage(msg, options.GetRejectMessage());
                            }
                        }
                        else
                        {
                            logger_base.warn("Rejected Msg: Blacklist : %s", (const char*)msg.GetLog().c_str());
                            SendRejectMessage(msg, options.GetRejectMessage());
                        }
                    }
                    else
                    {
                        logger_base.warn("Rejected Msg: Phone Blacklist : %s", (const char*)msg.GetLog().c_str());
                        SendRejectMessage(msg, options.GetRejectMessage());
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

            return added;
        }

        void ClearDisplayed()
        {
            for (auto& it : _messages)
            {
                it._displayed = false;
            }
        }

        void AddTestMessages(wxArrayString msgs, const SMSDaemonOptions& options)
        {
            for (auto m : msgs)
            {
                SMSMessage msg;
                msg._timestamp = wxDateTime::Now().MakeGMT();
                msg._from = "TEST";
                msg._rawMessage = m;
                AddMessage(msg, options);
            }
        }
};
#endif
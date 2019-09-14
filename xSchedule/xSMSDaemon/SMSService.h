#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <vector>
#include <atomic>
#include <mutex>
#include <codecvt>

#include <wx/uri.h>

#include "SMSMessage.h"
#include "SMSDaemonOptions.h"

class SMSService;

class RetrieveThread : public wxThread
{
    SMSService* _service;
    std::atomic_bool _stop = {false};
    std::atomic_bool _running = {false};
    uint32_t _interval = 30000;

public:
    RetrieveThread(SMSService* service, uint32_t interval);
    virtual ~RetrieveThread() {};
    bool IsOk() const { return _running; }
    virtual void* Entry() override;
    void Stop() { _stop = true; }
};

class SMSService
{
    std::recursive_mutex _threadLock;
    RetrieveThread* _thread = nullptr;
    std::vector<SMSMessage> _messages;
    std::vector<SMSMessage> _rejectedMessages;
    wxDateTime _lastRetrieved = wxInvalidDateTime;
    std::map<std::string, int> _msgsReceivedFromPhone;

    protected:

    SMSDaemonOptions _options;

    int GetMessagesReceivedFromPhone(const std::string& phone)
    {
        if (_msgsReceivedFromPhone.find(phone) == _msgsReceivedFromPhone.end())
        {
            return 0;
        }
        return _msgsReceivedFromPhone[phone];
    }

    bool MessagesBelowMaximumMessageCount(int max, const std::string& phone)
    {
        if (_msgsReceivedFromPhone.find(phone) == _msgsReceivedFromPhone.end())
        {
            _msgsReceivedFromPhone[phone] = 0;
            return true;
        }

        _msgsReceivedFromPhone[phone] = _msgsReceivedFromPhone[phone] + 1;

        return (max == 0 || _msgsReceivedFromPhone[phone] <= max);
    }

    public:

        SMSService(const SMSDaemonOptions& options) : _options(options)
        {
            StartThread(_options.GetRetrieveInterval() * 1000);
        }
        virtual ~SMSService()
        {
            StopThread();
        }
        std::string GetLastRetrieved()
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            if (_lastRetrieved == wxInvalidDateTime)
            {
                return "Never";
            }
            return _lastRetrieved.FormatTime().ToStdString();
        }
        void Retrieved()
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            _lastRetrieved = wxDateTime::Now();
        }
        void StartThread(uint32_t interval)
        {
            StopThread();
            _thread = new RetrieveThread(this, interval);
        }
        void StopThread()
        {
            if (_thread != nullptr)
            {
                _thread->Stop();
                _thread->Wait(wxTHREAD_WAIT_BLOCK);
                delete _thread;
                _thread = nullptr;
            }
        }
        std::string GetUser()
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            return _options.GetUser();
        }
        std::string GetSID()
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            return _options.GetSID();
        }
        std::string GetToken()
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            return _options.GetToken();
        }
        std::string GetPhone()
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            return _options.GetPhone();
        }
        void PrepareMessages(int maxAgeMins)
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);

            // remove any messages that are too old
            _messages.erase(std::remove_if(_messages.begin(),
                _messages.end(),
                [maxAgeMins](SMSMessage& x) {return maxAgeMins != 0 && x.GetAgeMins() >= maxAgeMins; }),
                _messages.end());

            // put the next one to display at the front
            std::sort(_messages.begin(), _messages.end());
        }
        std::vector<SMSMessage> GetMessages()
		{
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            // return a copy of the messages ... so it can use it in a thread safe manner
		    return _messages;
		}
        void Reset(const SMSDaemonOptions& options)
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            _options = options;
            _messages.clear();
        }
        void Display(const SMSMessage& msg)
        {
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            for (auto& it : _messages)
            {
                if (it == msg)
                {
                    it.Display();
                }
            }
        }
        virtual std::string GetServiceName() const = 0;
        virtual bool SendSMS(const std::string& number, const std::string& message) = 0;
        virtual bool RetrieveMessages() = 0;
        void SendSuccessMessage(const SMSMessage& msg, const wxString& successMessage)
        {
            if (successMessage != "" &&
                (_options.GetMaxMsgAgeMinsForResponse() == 0 || msg.GetAgeMins() < _options.GetMaxMsgAgeMinsForResponse()))
            {
                SendSMS(msg._from, successMessage);
            }
        }
        void SendRejectMessage(const SMSMessage& msg, const wxString& rejectMessage)
        {
            if (rejectMessage != "" &&
                (_options.GetMaxMsgAgeMinsForResponse() == 0 || msg.GetAgeMins() < _options.GetMaxMsgAgeMinsForResponse()))
            {
                SendSMS(msg._from, rejectMessage);
            }
        }
        bool AddMessage(SMSMessage& msg)
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            bool added = false;

            Replace(msg._rawMessage, ",", ""); // remove commas from messages as these cause issues

            int maxAgeMins = _options.GetMaxMessageAge();

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
                for (auto it : _rejectedMessages)
                {
                    if (it == msg)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    if (!_options.GetUsePhoneBlacklist() || msg.PassesPhoneBlacklist())
                    {
                        if (_options.GetMaximumMessagesPerPhone() == 0 || MessagesBelowMaximumMessageCount(_options.GetMaximumMessagesPerPhone(), msg._from))
                        {
                            if (!_options.GetUseLocalBlacklist() || msg.PassesBlacklist())
                            {
                                if (!_options.GetUseLocalWhitelist() || msg.PassesWhitelist())
                                {
                                    if (_options.GetUsePurgoMalum())
                                    {
                                        msg.Censor(_options.GetRejectProfanity());
                                    }
                                    else
                                    {
                                        msg._message = msg._rawMessage;
                                    }

                                    if (msg._message != "")
                                    {
                                        int maxMessageLen = _options.GetMaxMessageLength();
                                        if (maxMessageLen != 0 && msg._message.size() > maxMessageLen &&
                                            !_options.GetIgnoreOversizedMessages())
                                        {
                                            msg._message = msg._message.substr(0, maxMessageLen);
                                        }

                                        // messages have to be under the max
                                        if (maxMessageLen == 0 || msg._message.size() <= maxMessageLen)
                                        {
                                            if (!_options.GetAcceptOneWordOnly() || msg._message.find(" ") == std::string::npos)
                                            {
                                                if (_options.GetUpperCase())
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
                                                if (msg._from != GetPhone())
                                                {
                                                    SendSuccessMessage(msg, _options.GetSuccessMessage());
                                                }
                                            }
                                            else
                                            {
                                                logger_base.warn("Rejected Msg: Not one word : %s", (const char*)msg.GetLog().c_str());
                                                _rejectedMessages.push_back(msg);
                                                SendRejectMessage(msg, _options.GetRejectMessage());
                                            }
                                        }
                                        else
                                        {
                                            logger_base.warn("Rejected Msg: Too long : %s", (const char*)msg.GetLog().c_str());
                                            _rejectedMessages.push_back(msg);
                                            SendRejectMessage(msg, _options.GetRejectMessage());
                                        }
                                    }
                                    else
                                    {
                                        if (_options.GetRejectProfanity())
                                        {
                                            logger_base.warn("Rejected Msg: Censored : %s", (const char*)msg.GetLog().c_str());
                                            _rejectedMessages.push_back(msg);
                                            SendRejectMessage(msg, _options.GetRejectMessage());
                                        }
                                    }
                                }
                                else
                                {
                                    logger_base.warn("Rejected Msg: Whitelist : %s", (const char*)msg.GetLog().c_str());
                                    _rejectedMessages.push_back(msg);
                                    SendRejectMessage(msg, _options.GetRejectMessage());
                                }
                            }
                            else
                            {
                                logger_base.warn("Rejected Msg: Too many messages from number : %s : %d > %d", (const char*)msg.GetLog().c_str(), GetMessagesReceivedFromPhone(msg._from), _options.GetMaximumMessagesPerPhone());
                                _rejectedMessages.push_back(msg);
                                // SendRejectMessage(msg, _options.GetRejectMessage()); - we dont want to do this
                            }
                        }
                        else
                        {
                            logger_base.warn("Rejected Msg: Blacklist : %s", (const char*)msg.GetLog().c_str());
                            _rejectedMessages.push_back(msg);
                            SendRejectMessage(msg, _options.GetRejectMessage());
                        }
                    }
                    else
                    {
                        logger_base.warn("Rejected Msg: Phone Blacklist : %s", (const char*)msg.GetLog().c_str());
                        _rejectedMessages.push_back(msg);
                        SendRejectMessage(msg, _options.GetRejectMessage());
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
            std::lock_guard<std::recursive_mutex> lock(_threadLock);
            for (auto& it : _messages)
            {
                it._displayed = false;
            }
        }

        void AddTestMessages(wxArrayString msgs)
        {
            for (auto m : msgs)
            {
                SMSMessage msg;
                msg._timestamp = wxDateTime::Now().MakeGMT() + wxTimeSpan(0, _options.GetTimezoneAdjust());
                msg._from = "TEST";
                msg._rawMessage = m;
                AddMessage(msg);
            }
        }
};

inline RetrieveThread::RetrieveThread(SMSService* service, uint32_t interval) : wxThread(wxTHREAD_JOINABLE)
{
    _service = service;
    _interval = interval;
    Run();
}

inline void* RetrieveThread::Entry()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_service == nullptr)
    {
        logger_base.info("Retrieve thread started but service was null. Exiting.");
        return nullptr;
    }

    _running = true;
    logger_base.info("Retrieve thread for %s running.", (const char *)_service->GetServiceName().c_str());
    bool first = true;

    while (!_stop)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            wxDateTime next = wxDateTime::Now() + wxTimeSpan(0, 0, 0, _interval);
            logger_base.debug("Next message retrieval at %s.", (const char *)next.FormatTime().c_str());

            while (!_stop && wxDateTime::Now() < next)
            {
                wxMilliSleep(100);
            }
        }

        if (!_stop)
        {
            _service->RetrieveMessages();
        }
    }

    _running = false;
    return nullptr;
}

class TestService : public SMSService
{
public:

    TestService(const SMSDaemonOptions& options) : SMSService(options) {}
    virtual bool SendSMS(const std::string& number, const std::string& message) override { return true; }
    virtual std::string GetServiceName() const override { return "Test"; }
    virtual bool RetrieveMessages() override { return true; }
};
#endif
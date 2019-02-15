#ifndef SMSSERVICE_H
#define SMSSERVICE_H

#include <vector>

#include <wx/uri.h>
#include "SMSMessage.h"

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
        void Reset()
        {
            _messages.clear();
        }
        virtual std::string GetServiceName() const = 0;
        virtual bool SendSMS(const std::string& number, const std::string& message) const = 0;
        virtual bool RetrieveMessages(int maxAgeMins, int maxMessageLen, bool ignoreOversize, bool useWhitelist, 
            bool useBlacklist, bool usePhoneBlacklist, bool oneWord, bool upperCase, bool rejectCensored, 
            const wxString& successMessage, const wxString& rejectMessage) = 0;
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
};
#endif
#ifndef TWILIO_H
#define TWILIO_H

#include <string>
#include <vector>

#include "utils/Curl.h"

#include <wx/wx.h>

#include "../../xLights/UtilFunctions.h"
#include "../wxJSON/jsonreader.h"
#include "SMSMessage.h"
#include "SMSService.h"

class Twilio : public SMSService
{
	const std::string TWILIO_API_URL = "https://api.twilio.com/2010-04-01/Accounts/{user}/Messages.json";

    public:

        Twilio(SMSDaemonOptions* options) : SMSService(options) {}

        virtual bool SendSMS(const std::string& number, const std::string& message) override
		{
            if (number == "TEST") return false;

            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            std::string url = TWILIO_API_URL;
            auto sid = GetSID();
            auto token = GetToken();
            auto user = GetUser();
            Replace(url, "{sid}", sid);
            Replace(url, "{user}", user);
            Replace(url, "{token}", token);

            std::vector<Curl::Var> vars;
            vars.push_back(Curl::Var("To", number));
            vars.push_back(Curl::Var("From", GetPhone()));
            vars.push_back(Curl::Var("Body", message));

            logger_base.debug("Sending SMS to:'%s' from:'%s' body:'%s'.",
                              (const char*)number.c_str(),
                              (const char*)GetPhone().c_str(),
                              (const char*)message.c_str());
            std::string res = Curl::HTTPSPost(url, vars, user, token);
            //logger_base.debug("%s", (const char*)url.c_str());
            logger_base.debug("%s", (const char*)res.c_str());
            return true;
		}

        virtual std::string GetServiceName() const override { return "Twilio"; }
        virtual bool RetrieveMessages() override
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

            bool added = false;

            std::string url = TWILIO_API_URL;
            auto sid = GetSID();
            auto token = GetToken();
            auto user = GetUser();
            Replace(url, "{sid}", sid);
            Replace(url, "{user}", user);
            Replace(url, "{token}", token);

            logger_base.debug("Retrieving messages using basic auth {Account SID:Auth Token}.");
            std::string res = Curl::HTTPSGet(url, user, token);
            //logger_base.debug("%s", (const char*)url.c_str());
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
                Retrieved();

                wxJSONValue defaultValue = wxString("");
                wxJSONValue msgs = root.Get("messages", defaultValue);

                if (msgs.IsArray())
                {
                    for (int i = 0; i < msgs.Size(); i++)
                    {
                        wxJSONValue &m = msgs[i];

                        if (m.Get("direction", defaultValue).AsString() == "inbound") // received
                        {
                            SMSMessage msg;
                            wxString timestamp = m.Get("date_created", defaultValue).AsString();
                            wxString::const_iterator end;
                            //"Sat, 10 Jun 2017 14:03:59 +0000"
                            msg._timestamp.ParseFormat(timestamp, "%a, %d %b %Y %H:%M:%S ", &end);
                            msg._timestamp += wxTimeSpan(0, _options->GetTimezoneAdjust());
                            msg._from = m.Get("from", defaultValue).AsString().ToStdString();
                            msg._rawMessage = m.Get("body", defaultValue).AsString().ToStdString();

                            if (AddMessage(msg))
                            {
                                added = true;
                            }
                        }
                    }
                }
                else
                {
                    logger_base.error("No SMS messages found: %s", (const char*)res.c_str());
                }
            }
            return added;
        }
};
#endif

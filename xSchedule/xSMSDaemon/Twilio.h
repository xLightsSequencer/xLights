#ifndef TWILIO_H
#define TWILIO_H

#include <string>
#include <vector>
#include <locale>

#include "Curl.h"

#include <wx/wx.h>

#include "../../xLights/UtilFunctions.h"
#include "../wxJSON/jsonreader.h"
#include "SMSMessage.h"
#include "SMSService.h"

class Twilio : public SMSService
{
	const std::string TWILIO_API_URL = "https://api.twilio.com/2010-04-01/Accounts/{sid}/Messages.json";

    public:

        Twilio() : SMSService() {}

        virtual bool SendSMS(const std::string& number, const std::string& message) const override
		{
            if (number == "TEST") return false;

            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            std::string url = TWILIO_API_URL;
            Replace(url, "{sid}", _sid);
            Replace(url, "{user}", _user);
            Replace(url, "{token}", _token);

            std::vector<Curl::Var> vars;
            vars.push_back(Curl::Var("To", number));
            vars.push_back(Curl::Var("From", _myNumber));
            vars.push_back(Curl::Var("Body", message));

            std::string res = Curl::HTTPSPost(url, vars, _sid, _token);
            logger_base.debug("%s", (const char*)url.c_str());
            logger_base.debug("%s", (const char*)res.c_str());
            return true;
		}

        virtual std::string GetServiceName() const override { return "Bandwidth"; }
        virtual bool RetrieveMessages(const SMSDaemonOptions& options) override
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

            bool added = false;

            std::string url = TWILIO_API_URL;
            Replace(url, "{sid}", _sid);
            Replace(url, "{user}", _user);
            Replace(url, "{token}", _token);

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
                            msg._from = m.Get("from", defaultValue).AsString().ToStdString();
                            msg._rawMessage = m.Get("body", defaultValue).AsString().ToStdString();

                            if (AddMessage(msg, options))
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
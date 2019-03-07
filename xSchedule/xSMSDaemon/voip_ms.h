#ifndef VOIP_MS_H
#define VOIP_MS_H

#include <string>
#include <vector>
#include <locale>

#include "Curl.h"

#include <wx/wx.h>

#include "../../xLights/UtilFunctions.h"
#include "../wxJSON/jsonreader.h"
#include "SMSMessage.h"
#include "SMSService.h"

class Voip_ms : public SMSService
{
	const std::string VOIP_MS_API_URL = "https://voip.ms/api/v1/rest.php?api_username={user}&api_password={sid}&method=";

    public:

        Voip_ms() : SMSService() {}

        virtual bool SendSMS(const std::string& number, const std::string& message) const override
		{
            if (number == "TEST") return false;

            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            std::string url = VOIP_MS_API_URL;
            Replace(url, "{sid}", _sid);
            Replace(url, "{user}", _user);
            Replace(url, "{token}", _token);
            url += "sendSMS";

            std::vector<Curl::Var> vars;
            vars.push_back(Curl::Var("did", _myNumber));
            vars.push_back(Curl::Var("dst", number));
            vars.push_back(Curl::Var("message", message));

            std::string res = Curl::HTTPSPost(url, vars);
            logger_base.debug("%s", (const char*)url.c_str());
            logger_base.debug("%s", (const char*)res.c_str());
            return Contains(res, "status\":\"success");
		}

        virtual std::string GetServiceName() const override { return "Voip.ms"; }
        virtual bool RetrieveMessages(const SMSDaemonOptions& options) override
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

            bool added = false;

            std::string url = VOIP_MS_API_URL;
            Replace(url, "{sid}", _sid);
            Replace(url, "{user}", _user);
            Replace(url, "{token}", _token);
            url += "getSMS";

            std::vector<Curl::Var> vars;
            vars.push_back(Curl::Var("type", "1"));
            vars.push_back(Curl::Var("limit", "100"));
            vars.push_back(Curl::Var("timezone", "0"));

            std::string res = Curl::HTTPSPost(url, vars);
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
                if (root.Get("status", defaultValue).AsString() == "success")
                {
                    wxJSONValue msgs = root.Get("sms", defaultValue);

                    if (msgs.IsArray())
                    {
                        for (int i = 0; i < msgs.Size(); i++)
                        {
                            wxJSONValue &m = msgs[i];

                            if (m.Get("type", defaultValue).AsString() == "1") // received
                            {
                                SMSMessage msg;
                                wxString timestamp = m.Get("date", defaultValue).AsString();
                                wxString::const_iterator end;
                                msg._timestamp.ParseFormat(timestamp, "%Y-%m-%d %H:%M:%S", &end);
                                wxASSERT(end == timestamp.end());
                                msg._from = m.Get("contact", defaultValue).AsString().ToStdString();
                                msg._rawMessage = m.Get("message", defaultValue).AsString().ToStdString();

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
                else
                {
                    logger_base.error("Get SMS call failed: %s", (const char*)res.c_str());
                }
            }
            return added;
        }
};
#endif
#ifndef BANDWIDTH_H
#define BANDWIDTH_H

#include <string>
#include <vector>
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
            if (number == "TEST") return false;

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
        virtual bool RetrieveMessages(const SMSDaemonOptions& options) override
        {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

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

                        if (AddMessage(msg, options))
                        {
                            added = true;
                        }
                    }
                }
            }

            return added;
        }
};
#endif

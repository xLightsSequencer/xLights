#ifndef VOIP_MS_H
#define VOIP_MS_H

#include <string>
#include <vector>

#include "utils/Curl.h"

#include <wx/wx.h>

#include "../../xLights/UtilFunctions.h"
#include "../wxJSON/jsonreader.h"
#include "SMSMessage.h"
#include "SMSService.h"

class Voip_ms : public SMSService
{
    const std::string VOIP_MS_API_URL = "https://voip.ms/api/v1/rest.php?api_username={user}&api_password={sid}&method=";

public:
    Voip_ms(SMSDaemonOptions* options) :
        SMSService(options)
    {}

    virtual bool SendSMS(const std::string& number, const std::string& message) override
    {
        if (number == "TEST")
            return false;

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        std::string url = VOIP_MS_API_URL;
        Replace(url, "{sid}", GetSID());
        Replace(url, "{user}", GetUser());
        Replace(url, "{token}", GetToken());
        url += "sendSMS";

        std::vector<std::pair<std::string, std::string>> vars;
        vars.push_back({ "did", GetPhone() });
        vars.push_back({ "dst", number });
        vars.push_back({ "message", message });

        logger_base.debug("HTTPS Get %s", (const char*)url.c_str());
        for (const auto& it : vars) {
            logger_base.debug("    '%s' = '%s'", (const char*)it.first.c_str(), (const char*)it.second.c_str());
        }
        logger_base.debug("Sending SMS response did:'%s' dst:'%s' message:'%s'", (const char*)GetPhone().c_str(), (const char*)number.c_str(), (const char*)message.c_str());

        std::string res = Curl::HTTPSGet(url, "", "", 10, vars);
        logger_base.debug("'%s'", (const char*)res.c_str());
        return Contains(res, "status\":\"success");
    }

    virtual std::string GetServiceName() const override
    {
        return "Voip.ms";
    }
    virtual bool RetrieveMessages() override
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        bool added = false;

        std::string url = VOIP_MS_API_URL;
        Replace(url, "{sid}", GetSID());
        Replace(url, "{user}", GetUser());
        Replace(url, "{token}", GetToken());
        url += "getSMS";

        std::vector<std::pair<std::string, std::string>> vars;
        vars.push_back({ "type", "1" });
        vars.push_back({ "limit", "100" });
        vars.push_back({ "timezone", "0" });

        logger_base.debug("HTTPS Get %s", (const char*)url.c_str());
        for (const auto& it : vars) {
            logger_base.debug("    '%s' = '%s'", (const char*)it.first.c_str(), (const char*)it.second.c_str());
        }
        logger_base.debug("Retrieving messages: %s", (const char*)url.c_str());

        std::string res = Curl::HTTPSGet(url, "", "", 10, vars);
        logger_base.debug("Result '%s'", (const char*)res.c_str());

        // construct the JSON root object
        wxJSONValue root;

        // construct a JSON parser
        wxJSONReader reader;

        // now read the JSON text and store it in the 'root' structure
        // check for errors before retreiving values...

        // strip out unicode characters as they cause a crash
        Replace(res, "\\u", "!!u!!");

        int numErrors = reader.Parse(res, &root);
        if (numErrors > 0) {
            logger_base.error("The JSON document is not well-formed: %s", (const char*)res.c_str());
        } else {
            wxJSONValue defaultValue = wxString("");
            if (root.Get("status", defaultValue).AsString() == "success") {
                Retrieved();

                wxJSONValue msgs = root.Get("sms", defaultValue);

                if (msgs.IsArray()) {
                    for (int i = 0; i < msgs.Size(); i++) {
                        wxJSONValue& m = msgs[i];

                        if (m.Get("type", defaultValue).AsString() == "1") // received
                        {
                            SMSMessage msg;
                            wxString timestamp = m.Get("date", defaultValue).AsString();
                            wxString::const_iterator end;
                            msg._timestamp.ParseFormat(timestamp, "%Y-%m-%d %H:%M:%S", &end);
                            msg._timestamp += wxTimeSpan(0, _options->GetTimezoneAdjust());
                            wxASSERT(end == timestamp.end());
                            msg._from = m.Get("contact", defaultValue).AsString().ToStdString();
                            msg._rawMessage = m.Get("message", defaultValue).AsString().ToStdString();

                            if (AddMessage(msg)) {
                                added = true;
                            }
                        }
                    }
                } else {
                    logger_base.error("No SMS messages found: %s", (const char*)res.c_str());
                }
            } else {
                logger_base.error("Get SMS call failed: %s", (const char*)res.c_str());
            }
        }
        return added;
    }
};
#endif

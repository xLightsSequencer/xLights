#ifndef VOIP_MS_H
#define VOIP_MS_H

#include <string>
#include <vector>

#include "utils/Curl.h"

#include <wx/wx.h>

#include <Log.h>

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

        std::string url = VOIP_MS_API_URL;
        Replace(url, "{sid}", GetSID());
        Replace(url, "{user}", GetUser());
        Replace(url, "{token}", GetToken());
        url += "sendSMS";

        std::vector<std::pair<std::string, std::string>> vars;
        vars.push_back({ "did", GetPhone() });
        vars.push_back({ "dst", number });
        vars.push_back({ "message", message });

        spdlog::debug("HTTPS Get {}", url);
        for (const auto& it : vars) {
            spdlog::debug("    '{}' = '{}'", it.first, it.second);
        }
        spdlog::debug("Sending SMS response did:'{}' dst:'{}' message:'{}'", GetPhone(), number, message);
        std::string res = Curl::HTTPSGet(url, "", "", 10, vars);
        spdlog::debug("{}", res);
        return Contains(res, "status\":\"success");
    }

    virtual std::string GetServiceName() const override
    {
        return "Voip.ms";
    }
    virtual bool RetrieveMessages() override
    {
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

        spdlog::debug("HTTPS Get {}", url);
        for (const auto& it : vars) {
            spdlog::debug("    '{}' = '{}'", it.first, it.second);
        }
        spdlog::debug("Retrieving messages: {}", url);
        std::string res = Curl::HTTPSGet(url, "", "", 10, vars);
        spdlog::debug("Result '{}'", res);

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
            spdlog::error("The JSON document is not well-formed: {}", res);
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
                    spdlog::error("No SMS messages found: {}", res);
                }
            } else {
                spdlog::error("Get SMS call failed: {}", res);
            }
        }
        return added;
    }
};
#endif

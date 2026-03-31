#pragma once
#include <string>

#include "utils/CurlManager.h"

#include <log.h>

#include "../../xLights/utils/string_utils.h"

#include "SMSMessage.h"
#include "SMSService.h"

#include <nlohmann/json.hpp>

class Bandwidth : public SMSService
{
	const std::string BANDWIDTH_API_URL = "https://api.catapult.inetwork.com/v1/users/{user}";
    const std::string body = "{\"from\":\"{phone}\",\"to\":\"{tophone}\",\"text\":\"{message}\"}"; // why does someone like to hand gerenate JSON, instead of using a JSON library??

    public:

        Bandwidth(SMSDaemonOptions* options) : SMSService(options) {}

        virtual bool SendSMS(const std::string& number, const std::string& message) override
		{
            if (number == "TEST") return false;

            std::string url = BANDWIDTH_API_URL;
            auto sid = GetSID();
            auto token = GetToken();
            Replace(url, "{sid}", sid);
            Replace(url, "{user}", GetUser());
            Replace(url, "{token}", token);
            url += "/messages";

            std::string b = body;
            Replace(b, "{phone}", GetPhone());
            Replace(b, "{tophone}", number);
            Replace(b, "{message}", message);

            spdlog::debug("Sending SMS tophone:'{}' phone:'{}' message:'{}'.",
                          number,
                          GetPhone(),
                          message);
            std::string res = CurlManager::HTTPSPost(url, b, sid, token, "JSON");

            //logger_base.debug("%s", (const char*)url.c_str());
            spdlog::debug("{}", res);
            return true;
		}

        virtual std::string GetServiceName() const override { return "Bandwidth"; }
        virtual bool RetrieveMessages() override
        {
            bool added = false;

            std::string url = BANDWIDTH_API_URL;
            auto sid = GetSID();
            auto token = GetToken();
            Replace(url, "{sid}", sid);
            Replace(url, "{user}", GetUser());
            Replace(url, "{token}", token);
            url += "/messages?page=0&size=100";
            spdlog::debug("Retrieving messages.");
            std::string res = CurlManager::HTTPSGet(url, sid, token);
            spdlog::debug("{}", res);

            // strip out unicode characters as they cause a crash
            Replace(res, "\\u", "!!u!!");

            try
            {
                nlohmann::json const root = nlohmann::json::parse(res);
                Retrieved();

                if (root.is_array()) {
                    for (const auto& m : root) {
                        // only add inbound messages
                        if (m.contains("direction") && m["direction"].get<std::string>() == "in") {
                            SMSMessage msg;
                            
                            if (m.contains("time")) {
                                auto timestamp = wxString(m["time"].get<std::string>());
                                msg._timestamp.ParseISOCombined(timestamp);
                                msg._timestamp += wxTimeSpan(0, _options->GetTimezoneAdjust());
                            }
                            
                            if (m.contains("from")) {
                                msg._from = m["from"].get<std::string>();
                            }
                            
                            if (m.contains("text")) {
                                msg._rawMessage = m["text"].get<std::string>();
                            }

                            if (AddMessage(msg)) {
                                added = true;
                            }
                        }
                    }
                }
            }
            catch (const nlohmann::json::parse_error& e)
            {
                spdlog::error("The JSON document is not well-formed: {}. Error: {}", res, e.what());
                return false;
            }
            return added;
        }
};

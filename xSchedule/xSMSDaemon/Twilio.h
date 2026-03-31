#pragma once

#include <string>
#include <vector>

#include "utils/CurlManager.h"

#include <log.h>


#include <nlohmann/json.hpp>

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

            std::string url = TWILIO_API_URL;
            auto sid = GetSID();
            auto token = GetToken();
            auto user = GetUser();
            Replace(url, "{sid}", sid);
            Replace(url, "{user}", user);
            Replace(url, "{token}", token);

            std::vector<CurlManager::Var> vars;
            vars.push_back(CurlManager::Var("To", number));
            vars.push_back(CurlManager::Var("From", GetPhone()));
            vars.push_back(CurlManager::Var("Body", message));

            spdlog::debug("Sending SMS to:'{}' from:'{}' body:'{}'.",
                          number,
                          GetPhone(),
                          message);
            std::string res = CurlManager::HTTPSPost(url, vars, user, token);
            //logger_base.debug("%s", (const char*)url.c_str());
            spdlog::debug("{}", res);
            return true;
		}

        virtual std::string GetServiceName() const override { return "Twilio"; }
        virtual bool RetrieveMessages() override
        {
            bool added = false;

            std::string url = TWILIO_API_URL;
            auto sid = GetSID();
            auto token = GetToken();
            auto user = GetUser();
            Replace(url, "{sid}", sid);
            Replace(url, "{user}", user);
            Replace(url, "{token}", token);

            spdlog::debug("Retrieving messages using basic auth {Account SID:Auth Token}.");
            std::string res = CurlManager::HTTPSGet(url, user, token);
            //logger_base.debug("%s", (const char*)url.c_str());
            spdlog::debug("{}", res);

            // now read the JSON text and store it in the 'root' structure
            // check for errors before retreiving values...

            // strip out unicode characters as they cause a crash
            Replace(res, "\\u", "!!u!!");

            try {
                // construct the JSON root object
                nlohmann::json const root = nlohmann::json::parse(res);

                Retrieved();

                nlohmann::json const msgs = root.at("messages");

                if (msgs.is_array()) {
                    for (int i = 0; i < msgs.size(); i++) {
                        nlohmann::json const& m = msgs[i];
                        if (m.at("direction").get<std::string>() == "inbound") // received
                        {
                            SMSMessage msg;
                            std::string timestamp = m.value("date_created", std::string());
                            wxString::const_iterator end;
                            //"Sat, 10 Jun 2017 14:03:59 +0000"
                            msg._timestamp.ParseFormat(timestamp, "%a, %d %b %Y %H:%M:%S ", &end);
                            msg._timestamp += wxTimeSpan(0, _options->GetTimezoneAdjust());
                            msg._from = m.value("from", std::string());
                            msg._rawMessage = m.value("body", std::string());

                            if (AddMessage(msg)) {
                                added = true;
                            }
                        }
                    }
                } else {
                    spdlog::error("No SMS messages found: {}", res);
                }
            }
            catch (nlohmann::json::parse_error& e) {
                spdlog::error("The JSON document is not well-formed: {}", res);
                spdlog::error("Parse error at byte {}: {}", e.byte, e.what());
                return false;
            } catch (std::exception& e) {
                spdlog::error("Parse error: {}", e.what());
                return false;
            }
            return added;
        }
};

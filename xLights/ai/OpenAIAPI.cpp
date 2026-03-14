#include "ServiceManager.h"
#include "UtilFunctions.h"
#include "OpenAIAPI.h"
#include "utils/Curl.h"
#include <nlohmann/json.hpp>

#include "OpenAIImageGenerator.h"

#include <string>
#include <vector>

#include <log4cpp/Category.hh>

constexpr const char* completion_url = "/chat/completions";

std::pair<std::string, bool> OpenAIAPI::CallLLM(const std::string& prompt) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string bearerToken = token;

    if (bearerToken.empty()) {
        wxMessageBox("You must set a " + GetLLMName() + " Bearer Token in the Preferences on the Services Panel", "Error", wxICON_ERROR);
        return { GetLLMName() + ": Bearer Token is empty", false };
    }

    // remove all \t, \r and \n as chatGPT does not like it
    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));
    Replace(p, std::string("\n"), std::string("\\n"));

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] }";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + bearerToken }
    };

    logger_base.debug("%s: %s", (const char*)GetLLMName().c_str(), (const char*)request.c_str());

    int responseCode = 0;
    std::string response = Curl::HTTPSPost(base_url + completion_url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("%s Response %d: %s", (const char*)GetLLMName().c_str(), responseCode, (const char*)response.c_str());

    if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        logger_base.error("%s: Invalid JSON response: %s", (const char*)GetLLMName().c_str(), (const char*)response.c_str());
        return { GetLLMName() +  ": Invalid JSON response", false };
    }

    auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
        logger_base.error("%s: No choices in response", (const char*)GetLLMName().c_str());
        return { GetLLMName() + ": No choices in response", false };
    }

    auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
        logger_base.error("%s: No text in response", (const char*)GetLLMName().c_str());
        return { GetLLMName() + ": No text in response", false };
    }

    response = text.get<std::string>();
    logger_base.debug("%s: %s", GetLLMName().c_str(), response.c_str());

    return { response, true };
}

aiBase::AIColorPalette OpenAIAPI::GenerateColorPalette(const std::string& prompt) const {
    aiBase::AIColorPalette ret;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (token.empty()) {
        ret.error = "You must set a " + GetLLMName() + " Bearer Token in the Preferences on the Services Panel";
        return ret;
    }

    std::string fullprompt = "xlights color palettes are 8 unique colors. Can you create a color palette that would represent the moods and imagery " + prompt + ". Avoid dark, near black colors.";

    std::string schema = R"(
    "response_format" : {
        "type": "json_schema",
        "json_schema": {
            "name": "color_palette",
            "schema": {
                "type": "object",
                "properties":  {
                    "description": {
                        "type": "string"
                    },
                    "colors": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "hex_code": {
                                    "type": "string"
                                },
                                "name": {
                                    "type": "string"
                                },
                                "usage_notes": {
                                    "type": "string"
                                }
                            },
                            "required": [
                                "hex_code", "name", "usage_notes"
                            ]
                        }
                    }
                },
                "required": [
                    "description", "colors"
                ]
            }
        }
    }
    )";

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(fullprompt) + "\" } ]," + schema + "}";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + token }
    };

    logger_base.debug("%s: %s", (const char*)GetLLMName().c_str(), (const char*)request.c_str());

    int responseCode = 0;
    std::string response = Curl::HTTPSPost(base_url + completion_url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("%s Response %d: %s", (const char*)GetLLMName().c_str(), responseCode, (const char*)response.c_str());
    if (responseCode != 200) {
        ret.error = response;
    }
    logger_base.debug("%s Response: %s", (const char*)GetLLMName().c_str(), (const char*)response.c_str());
    try {
        nlohmann::json root = nlohmann::json::parse(response);

        if (root.contains("choices") && root["choices"].is_array() && root["choices"].size() > 0 && root["choices"][0].contains("message")) {
            auto const color_responce = root["choices"][0]["message"]["content"].get<std::string>();

            logger_base.debug("%s Content %s", (const char*)GetLLMName().c_str(), (const char*)color_responce.c_str());
            try {
                // Check if the response is valid JSON
                nlohmann::json const color_root = nlohmann::json::parse(color_responce);
                if (color_root.contains("colors") && color_root["colors"].is_array()) {
                    aiBase::AIColorPalette ret;
                    ret.description = prompt;
                    if (color_root.contains("description")) {
                        ret.description = color_root["description"].get<std::string>();
                    }
                    for (int x = 0; x < color_root["colors"].size(); x++) {
                        auto& color = color_root["colors"][x];
                        ret.colors.push_back(aiBase::AIColor());
                        ret.colors.back().hexValue = color["hex_code"].get<std::string>();
                        ret.colors.back().description = color["usage_notes"].get<std::string>();
                        ret.colors.back().name = color["name"].get<std::string>();
                    }
                    return ret;
                }
            } catch (const std::exception& ex) {
                logger_base.error(ex.what());
            }
            logger_base.error("Response does not contain 'colors' array or is not in expected format.");
            ret.error = "Response does not contain 'colors' array or is not in expected format.";
        } else {
            logger_base.error("Invalid response from %s API.", (const char*)GetLLMName().c_str());
            ret.error = "Invalid response from " + GetLLMName() + " API.";
        }
    } catch (const std::exception& e) {
        logger_base.error(e.what());
    }

    return ret;
}

aiBase::AIImageGenerator* OpenAIAPI::createAIImageGenerator() const {
    return new OpenAIImageGenerator(base_url, token, image_model);
}
#include "ServiceManager.h"
#include "UtilFunctions.h"
#include "OpenAIAPI.h"
#include "utils/Curl.h"
#include <nlohmann/json.hpp>

#include "OpenAIImageGenerator.h"

#include <string>
#include <vector>

#include "spdlog/spdlog.h"

constexpr const char* completion_url = "/chat/completions";

std::pair<std::string, bool> OpenAIAPI::CallLLM(const std::string& prompt) const {
    

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

    spdlog::debug("{}: {}", GetLLMName(), request);

    int responseCode = 0;
    std::string response = Curl::HTTPSPost(base_url + completion_url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    spdlog::debug("{} Response {}: {}", GetLLMName(), responseCode, response);

    if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        spdlog::error("{}: Invalid JSON response: {}", GetLLMName(), response);
        return { GetLLMName() +  ": Invalid JSON response", false };
    }

    auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
        spdlog::error("{}: No choices in response", GetLLMName());
        return { GetLLMName() + ": No choices in response", false };
    }

    auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
        spdlog::error("{}: No text in response", GetLLMName());
        return { GetLLMName() + ": No text in response", false };
    }

    response = text.get<std::string>();
    spdlog::debug("{}: {}", GetLLMName(), response);

    return { response, true };
}

aiBase::AIColorPalette OpenAIAPI::GenerateColorPalette(const std::string& prompt) const {
    aiBase::AIColorPalette ret;
    

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

    spdlog::debug("{}: {}", GetLLMName(), request);

    int responseCode = 0;
    std::string response = Curl::HTTPSPost(base_url + completion_url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    spdlog::debug("{} Response {}: {}", GetLLMName(), responseCode, response);
    if (responseCode != 200) {
        ret.error = response;
    }
    spdlog::debug("{} Response: {}", GetLLMName(), response);
    try {
        nlohmann::json root = nlohmann::json::parse(response);

        if (root.contains("choices") && root["choices"].is_array() && root["choices"].size() > 0 && root["choices"][0].contains("message")) {
            auto const color_responce = root["choices"][0]["message"]["content"].get<std::string>();

            spdlog::debug("{} Content {}", GetLLMName(), color_responce);
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
                spdlog::error("{}", ex.what());
            }
            spdlog::error("Response does not contain 'colors' array or is not in expected format.");
            ret.error = "Response does not contain 'colors' array or is not in expected format.";
        } else {
            spdlog::error("Invalid response from {} API.", GetLLMName());
            ret.error = "Invalid response from " + GetLLMName() + " API.";
        }
    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
    }

    return ret;
}

aiBase::AIImageGenerator* OpenAIAPI::createAIImageGenerator() const {
    return new OpenAIImageGenerator(base_url, token, image_model);
}
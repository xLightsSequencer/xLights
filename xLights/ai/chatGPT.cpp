#include "chatGPT.h"
#include <nlohmann/json.hpp>
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>

#include <vector>
#include <string>

#include <log4cpp/Category.hh>

bool chatGPT::IsAvailable() const {
    return !bearer_token.empty() && _enabled;
}

void chatGPT::SaveSettings() const {
	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	_sm->setServiceSetting("ChatGPTEnable", _enabled);    
	_sm->setServiceSetting("ChatGPTModel", model);
    _sm->setSecretServiceToken("ChatGPTBearerToken", bearer_token);
	logger_base.info("ChatGPT settings saved successfully");
}

void chatGPT::LoadSettings() {
    model = _sm->getServiceSetting("ChatGPTModel", model);
    _enabled = _sm->getServiceSetting("ChatGPTEnable", _enabled);
    bearer_token = _sm->getSecretServiceToken("ChatGPTBearerToken");
}

void chatGPT::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("ChatGPT"));
    auto p = page->Append(new wxBoolProperty("Enabled", "ChatGPT.Enabled", _enabled));
    p->SetEditor("CheckBox");
    page->Append(new wxStringProperty("Bearer Token", "ChatGPT.Token", bearer_token));
    page->Append(new wxStringProperty("Model", "ChatGPT.Model", model));
}

void chatGPT::SetSetting(const std::string& key, const wxVariant& value) {
	if (key == "ChatGPT.Enabled") {
		_enabled = value.GetBool();
	} else if (key == "ChatGPT.Token") {
		bearer_token = value.GetString();
	} else if (key == "ChatGPT.Model") {
		model = value.GetString();
	}
}

std::pair<std::string, bool> chatGPT::CallLLM(const std::string& prompt) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string bearerToken = bearer_token;

    if (bearerToken.empty()) {
        bearerToken = _sm->getSecretServiceToken("ChatGPTBearerToken");
	}

	if (bearer_token.empty() && bearerToken.empty()) {
		wxMessageBox("You must set a ChatGPT Bearer Token in the Preferences on the Services Panel", "Error", wxICON_ERROR);
        return { "ChatGPT: Bearer Token is empty", false };
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

    logger_base.debug("ChatGPT: %s", request.c_str());

	int responseCode = 0;	
	std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("ChatGPT Response %d: %s", responseCode, response.c_str());

	if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        logger_base.error("ChatGPT: Invalid JSON response: %s", response.c_str());
        return { "ChatGPT: Invalid JSON response", false };
    }

	auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
		logger_base.error("ChatGPT: No choices in response");
        return { "ChatGPT: No choices in response", false };
	}

	auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
		logger_base.error("ChatGPT: No text in response");
        return { "ChatGPT: No text in response", false };
	}

	response = text.get<std::string>();
	logger_base.debug("ChatGPT: %s", response.c_str());

    return { response, true };
}



aiBase::AIColorPalette chatGPT::GenerateColorPalette(const std::string &prompt) const {
    aiBase::AIColorPalette ret;
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    
    std::string bearerToken = bearer_token;
    
    if (bearerToken.empty()) {
        bearerToken = _sm->getSecretServiceToken("ChatGPTBearerToken");
    }
    
    if (bearer_token.empty() && bearerToken.empty()) {
        ret.error = "You must set a ChatGPT Bearer Token in the Preferences on the Services Panel";
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
    
    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(fullprompt) + "\" } ],"
        + schema +  "}";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + bearerToken }
    };

    logger_base.debug("ChatGPT: %s", request.c_str());

    int responseCode = 0;
    std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("ChatGPT Response %d: %s", responseCode, response.c_str());
    if (responseCode != 200) {
        ret.error = response;
    }
    logger_base.debug("ChatGPT Response: %s", response.c_str());
    try {
        nlohmann::json root = nlohmann::json::parse(response);

        if (root.contains("choices") && root["choices"].is_array() && root["choices"].size() > 0 && root["choices"][0].contains("message")) {
            auto const color_responce = root["choices"][0]["message"]["content"].get<std::string>();

            logger_base.debug("ChatGPT Content %s", (const char*)color_responce.c_str());
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
            logger_base.error("Invalid response from ChatGPT API.");
            ret.error = "Invalid response from ChatGPT API.";
        }
    } catch (const std::exception& e) {
        logger_base.error(e.what());
    }
    
    return ret;
}

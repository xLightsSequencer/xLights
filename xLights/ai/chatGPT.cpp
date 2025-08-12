#include "chatGPT.h"
#include <nlohmann/json.hpp>

#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>
#include <wx/colour.h>

#include <vector>
#include <string>

#include "./utils/spdlog_macros.h"

bool chatGPT::IsAvailable() const {
    return !bearer_token.empty() && _enabled;
}

void chatGPT::SaveSettings() const {
	
	_sm->setServiceSetting("ChatGPTEnable", _enabled);    
	_sm->setServiceSetting("ChatGPTModel", model);
    _sm->setSecretServiceToken("ChatGPTBearerToken", bearer_token);
	LOG_INFO("ChatGPT settings saved successfully");
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

	std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] , \"temperature\": " + std::to_string(temperature) + " }";

	std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + bearerToken }
    };

    LOG_DEBUG("ChatGPT: %s", request.c_str());

	int responseCode = 0;	
	std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    LOG_DEBUG("ChatGPT Response %d: %s", responseCode, response.c_str());

	if (responseCode != 200) {
        return { response, false };
    }

	nlohmann::json root;
	try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        spdlog::error("ChatGPT: Invalid JSON response: {}", response);
        return { "ChatGPT: Invalid JSON response", false };
    }

	auto choices = root["choices"];
	if (choices.is_null() || choices.size() == 0) {
		LOG_ERROR("ChatGPT: No choices in response");
        return { "ChatGPT: No choices in response", false };
	}

	auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
		LOG_ERROR("ChatGPT: No text in response");
        return { "ChatGPT: No text in response", false };
	}

	response = text.get<std::string>();
	LOG_DEBUG("ChatGPT: %s", response.c_str());

    return { response, true };
}

std::vector<wxColour> chatGPT::CallLLMForColors(const std::string& prompt) const {
    return std::vector<wxColour>();
}
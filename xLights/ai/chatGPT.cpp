#include "chatGPT.h"
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

	std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] , \"temperature\": " + std::to_string(temperature) + " }";

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

	wxJSONReader reader;
	wxJSONValue root;
    if (reader.Parse(response, &root) > 0)
	{
		logger_base.error("ChatGPT: Failed to parse response");
        return { "ChatGPT: Failed to parse response", false };
	}

	wxJSONValue choices = root["choices"];
	if (choices.IsNull() || choices.Size() == 0) {
		logger_base.error("ChatGPT: No choices in response");
        return { "ChatGPT: No choices in response", false };
	}

	wxJSONValue choice = choices[0];
	wxJSONValue text = choice["message"]["content"];
	if (text.IsNull()) {
		logger_base.error("ChatGPT: No text in response");
        return { "ChatGPT: No text in response", false };
	}

	response = text.AsString();
	logger_base.debug("ChatGPT: %s", response.c_str());

    return { response, true };
}
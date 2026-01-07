#include "gemini.h"
#include <nlohmann/json.hpp>
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>

#include <vector>
#include <string>

#include <log4cpp/Category.hh>

bool gemini::IsAvailable() const {
    return !api_key.empty() && _enabled;
}

void gemini::SaveSettings() const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _sm->setServiceSetting("GeminiModel", model);
    _sm->setServiceSetting("GeminiEnable", _enabled);
    _sm->setSecretServiceToken("GeminiApiKey", api_key);
    logger_base.info("Gemini settings saved successfully");
}

void gemini::LoadSettings() {
    model = _sm->getServiceSetting("GeminiModel", model);
    _enabled = _sm->getServiceSetting("GeminiEnable", _enabled);
    api_key = _sm->getSecretServiceToken("GeminiApiKey");
}

void gemini::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("Gemini"));
    auto p = page->Append(new wxBoolProperty("Enabled", "Gemini.Enabled", _enabled));
    p->SetEditor("CheckBox");
    page->Append(new wxStringProperty("API Key", "Gemini.Key", api_key));
    page->Append(new wxStringProperty("Model", "Gemini.Model", model));
}

void gemini::SetSetting(const std::string& key, const wxVariant& value) {
    if (key == "Gemini.Enabled") {
        _enabled = value.GetBool();
    } else if (key == "Gemini.Key") {
        api_key = value.GetString();
    } else if (key == "Gemini.Model") {
        model = value.GetString();
    }
}

std::pair<std::string, bool> gemini::CallLLM(const std::string& prompt) const {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    std::string apiKey = api_key;

    if (apiKey.empty()) {
        apiKey = _sm->getSecretServiceToken("GeminiApiKey");
    }

    if (api_key.empty() && apiKey.empty()) {
        wxMessageBox("You must set a Gemini API Key in the Preferences on the Services Panel", "Error", wxICON_ERROR);
        return { "Gemini: API Key is empty", false };
    }

    std::string p = prompt;
    Replace(p, std::string("\t"), std::string(" "));
    Replace(p, std::string("\r"), std::string(""));

    std::string const request = "{ \"model\": \"" + model + "\", \"messages\": [ { \"role\": \"user\",\"content\": \"" + JSONSafe(p) + "\" } ] }";

    std::vector<std::pair<std::string, std::string>> customHeaders = {
        { "Authorization", "Bearer " + apiKey }
    };

    logger_base.debug("Gemini: %s", request.c_str());

    int responseCode = 0;	
    std::string response = Curl::HTTPSPost(url, request, "", "", "JSON", 60, customHeaders, &responseCode);

    logger_base.debug("Gemini Response %d: %s", responseCode, response.c_str());

    if (responseCode != 200) {
        return { response, false };
    }

    nlohmann::json root;
    try {
        root = nlohmann::json::parse(response);
    } catch (const std::exception&) {
        logger_base.error("Gemini: Invalid JSON response: %s", response.c_str());
        return { "Gemini: Invalid JSON response", false };
    }

    auto choices = root["choices"];
    if (choices.is_null() || choices.size() == 0) {
        logger_base.error("Gemini: No choices in response");
        return { "Gemini: No choices in response", false };
    }

    auto choice = choices[0];
    auto text = choice["message"]["content"];
    if (text.is_null()) {
        logger_base.error("Gemini: No text in response");
        return { "Gemini: No text in response", false };
    }

    response = text.get<std::string>();
    logger_base.debug("Gemini: %s", response.c_str());

    return { response, true };
}

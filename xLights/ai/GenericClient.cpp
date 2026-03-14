#include "GenericClient.h"

#include "ServiceManager.h"

#include <wx/propgrid/propgrid.h>

#include <string>

constexpr const char* completions_api = "/completions";

bool GenericClient::IsAvailable() const {
    return !base_url.empty() && _enabled;
}

void GenericClient::SaveSettings() const {
    _sm->setServiceSetting("GenericClientEnable", _enabled);
    _sm->setServiceSetting("GenericClientBaseURL", base_url);
    _sm->setServiceSetting("GenericClientAPIKey", token);
    _sm->setServiceSetting("GenericClientModel", model);
    _sm->setServiceSetting("GenericClientImageModel", image_model);
}

void GenericClient::LoadSettings() {
    _enabled = _sm->getServiceSetting("GenericClientEnable", _enabled);
    base_url = _sm->getServiceSetting("GenericClientBaseURL", base_url);
    token = _sm->getServiceSetting("GenericClientAPIKey", token);
    model = _sm->getServiceSetting("GenericClientModel", model);
    image_model = _sm->getServiceSetting("GenericClientImageModel", image_model);
}

void GenericClient::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("Generic OpenAI Client"));
    auto p = page->Append(new wxBoolProperty("Enabled", "GenericClient.Enabled", _enabled));
    p->SetEditor("CheckBox");
    page->Append(new wxStringProperty("Base URL", "GenericClient.BaseURL", base_url));
    auto* apiKeyProp = page->Append(new wxStringProperty("API Key", "GenericClient.APIKey", token));
    apiKeyProp->SetAttribute(wxPG_STRING_PASSWORD, true);
    apiKeyProp->SetHelpString("Fake API key");
    page->Append(new wxStringProperty("Completion Model", "GenericClient.Model", model));
    page->Append(new wxStringProperty("Image Model", "GenericClient.ImageModel", image_model));
}

void GenericClient::SetSetting(const std::string& key, const wxVariant& value) {
	if (key == "GenericClient.Enabled") {
		_enabled = value.GetBool();
    } else if (key == "GenericClient.BaseURL") {
        base_url = value.GetString();
    } else if (key == "GenericClient.APIKey") {
        token = value.GetString();
    } else if (key == "GenericClient.Model") {
		model = value.GetString();
	} else if (key == "GenericClient.ImageModel") {
        image_model = value.GetString();
	}
}
#include "GenericClient.h"

#include "ServiceManager.h"

#include <wx/propgrid/propgrid.h>

#include <string>

constexpr const char* completions_api = "/completions";

bool GenericClient::IsAvailable() const {
    return !base_url.empty() && !_enabledTypes.empty();
}

void GenericClient::SaveSettings() const {
    _sm->setServiceSetting("GenericClientBaseURL", base_url);
    _sm->setSecretServiceToken("GenericClientAPIKey", token);
    _sm->setServiceSetting("GenericClientModel", model);
    _sm->setServiceSetting("GenericClientImageModel", image_model);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("GenericClientEnable_") + aiType::TypeSettingsSuffix(t), IsEnabledForType(t));
    }
}

void GenericClient::LoadSettings() {
    base_url = _sm->getServiceSetting("GenericClientBaseURL", base_url);
    token = _sm->getSecretServiceToken("GenericClientAPIKey");
    model = _sm->getServiceSetting("GenericClientModel", model);
    image_model = _sm->getServiceSetting("GenericClientImageModel", image_model);
    bool oldEnabled = _sm->getServiceSetting("GenericClientEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("GenericClientEnable_") + aiType::TypeSettingsSuffix(t), oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

void GenericClient::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("Generic OpenAI Client"));
    for (auto t : GetTypes()) {
        auto p = page->Append(new wxBoolProperty(wxString("Enable ") + aiType::TypeName(t),
                                                  wxString("GenericClient.Enable_") + aiType::TypeSettingsSuffix(t),
                                                  IsEnabledForType(t)));
        p->SetEditor("CheckBox");
    }
    page->Append(new wxStringProperty("Base URL", "GenericClient.BaseURL", base_url));
    auto* apiKeyProp = page->Append(new wxStringProperty("API Key", "GenericClient.APIKey", token));
    apiKeyProp->SetAttribute(wxPG_STRING_PASSWORD, true);
    apiKeyProp->SetHelpString("Fake API key");
    page->Append(new wxStringProperty("Completion Model", "GenericClient.Model", model));
    page->Append(new wxStringProperty("Image Model", "GenericClient.ImageModel", image_model));
}

void GenericClient::SetSetting(const std::string& key, const wxVariant& value) {
    for (auto t : GetTypes()) {
        if (key == std::string("GenericClient.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value.GetBool());
            return;
        }
    }
    if (key == "GenericClient.BaseURL") {
        base_url = value.GetString();
    } else if (key == "GenericClient.APIKey") {
        token = value.GetString();
    } else if (key == "GenericClient.Model") {
		model = value.GetString();
	} else if (key == "GenericClient.ImageModel") {
        image_model = value.GetString();
	}
}

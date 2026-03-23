#include "chatGPT.h"
#include <nlohmann/json.hpp>
#include "ServiceManager.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/propgrid/propgrid.h>

#include <vector>
#include <string>

#include "spdlog/spdlog.h"

bool chatGPT::IsAvailable() const {
    return !token.empty() && !_enabledTypes.empty();
}

void chatGPT::SaveSettings() const {
	
	_sm->setServiceSetting("ChatGPTModel", model);
    _sm->setServiceSetting("ChatGPTImageModel", image_model);
    _sm->setSecretServiceToken("ChatGPTBearerToken", token);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("ChatGPTEnable_") + aiType::TypeSettingsSuffix(t), IsEnabledForType(t));
    }
	spdlog::info("ChatGPT settings saved successfully");
}

void chatGPT::LoadSettings() {
    model = _sm->getServiceSetting("ChatGPTModel", model);
    image_model = _sm->getServiceSetting("ChatGPTImageModel", image_model);
    token = _sm->getSecretServiceToken("ChatGPTBearerToken");
    // Migrate from old single enable setting
    bool oldEnabled = _sm->getServiceSetting("ChatGPTEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("ChatGPTEnable_") + aiType::TypeSettingsSuffix(t), oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

void chatGPT::PopulateLLMSettings(wxPropertyGrid* page) {
    page->Append(new wxPropertyCategory("ChatGPT"));
    for (auto t : GetTypes()) {
        auto p = page->Append(new wxBoolProperty(wxString("Enable ") + aiType::TypeName(t),
                                                  wxString("ChatGPT.Enable_") + aiType::TypeSettingsSuffix(t),
                                                  IsEnabledForType(t)));
        p->SetEditor("CheckBox");
    }
    auto* tokenProp = page->Append(new wxStringProperty("Bearer Token", "ChatGPT.Token", token));
    tokenProp->SetAttribute(wxPG_STRING_PASSWORD, true);
    tokenProp->SetHelpString("Your ChatGPT Bearer Token (masked for security)");
    page->Append(new wxStringProperty("Model", "ChatGPT.Model", model));
    page->Append(new wxStringProperty("Image Model", "ChatGPT.ImgModel", image_model));
}

void chatGPT::SetSetting(const std::string& key, const wxVariant& value) {
    for (auto t : GetTypes()) {
        if (key == std::string("ChatGPT.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value.GetBool());
            return;
        }
    }
	if (key == "ChatGPT.Token") {
		token = value.GetString();
	} else if (key == "ChatGPT.Model") {
		model = value.GetString();
    } else if (key == "ChatGPT.ImgModel") {
        image_model = value.GetString();
    }
}

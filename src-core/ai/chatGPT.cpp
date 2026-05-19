#include "ai/chatGPT.h"
#include "ai/ServiceManager.h"

#include <log.h>

#include <string>
#include <vector>

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

std::vector<ServiceProperty> chatGPT::GetProperties() const {
    std::vector<ServiceProperty> props;
    props.push_back({ ServiceProperty::Kind::Category, {}, "ChatGPT", "ChatGPT", {}, {}, {} });
    for (auto t : GetTypes()) {
        props.push_back({
            ServiceProperty::Kind::Bool,
            std::string("ChatGPT.Enable_") + aiType::TypeSettingsSuffix(t),
            std::string("Enable ") + aiType::TypeName(t),
            "ChatGPT",
            {},
            {},
            IsEnabledForType(t)
        });
    }
    props.push_back({ ServiceProperty::Kind::Secret, "ChatGPT.Token", "Bearer Token", "ChatGPT",
                     "Your ChatGPT Bearer Token (masked for security)", {}, token });
    props.push_back({ ServiceProperty::Kind::String, "ChatGPT.Model", "Model", "ChatGPT", {}, {}, model });
    props.push_back({ ServiceProperty::Kind::String, "ChatGPT.ImgModel", "Image Model", "ChatGPT", {}, {}, image_model });
    return props;
}

void chatGPT::SetProperty(const std::string& id, bool value) {
    for (auto t : GetTypes()) {
        if (id == std::string("ChatGPT.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value);
            return;
        }
    }
}

void chatGPT::SetProperty(const std::string& id, const std::string& value) {
    if (id == "ChatGPT.Token") {
        token = value;
    } else if (id == "ChatGPT.Model") {
        model = value;
    } else if (id == "ChatGPT.ImgModel") {
        image_model = value;
    }
}

#include "ai/GenericClient.h"
#include "ai/ServiceManager.h"

#include <string>
#include <vector>

bool GenericClient::IsAvailable() const {
    return !base_url.empty() && !_enabledTypes.empty();
}

void GenericClient::SaveSettings() const {
    _sm->setServiceSetting("GenericClientBaseURL", base_url);
    _sm->setSecretServiceToken("GenericClientAPIKey", token);
    _sm->setServiceSetting("GenericClientModel", model);
    _sm->setServiceSetting("GenericClientImageModel", image_model);
    _sm->setServiceSetting("GenericClientTranscribeModel", transcribe_model);
    for (auto t : GetTypes()) {
        _sm->setServiceSetting(std::string("GenericClientEnable_") + aiType::TypeSettingsSuffix(t), IsEnabledForType(t));
    }
}

void GenericClient::LoadSettings() {
    base_url = _sm->getServiceSetting("GenericClientBaseURL", base_url);
    token = _sm->getSecretServiceToken("GenericClientAPIKey");
    model = _sm->getServiceSetting("GenericClientModel", model);
    image_model = _sm->getServiceSetting("GenericClientImageModel", image_model);
    transcribe_model = _sm->getServiceSetting("GenericClientTranscribeModel", transcribe_model);
    bool oldEnabled = _sm->getServiceSetting("GenericClientEnable", false);
    for (auto t : GetTypes()) {
        bool enabled = _sm->getServiceSetting(std::string("GenericClientEnable_") + aiType::TypeSettingsSuffix(t), oldEnabled);
        SetEnabledForType(t, enabled);
    }
}

std::vector<ServiceProperty> GenericClient::GetProperties() const {
    std::vector<ServiceProperty> props;
    props.push_back({ ServiceProperty::Kind::Category, {}, "Generic OpenAI Client", "GenericClient", {}, {}, {} });
    for (auto t : GetTypes()) {
        props.push_back({
            ServiceProperty::Kind::Bool,
            std::string("GenericClient.Enable_") + aiType::TypeSettingsSuffix(t),
            std::string("Enable ") + aiType::TypeName(t),
            "GenericClient",
            {},
            {},
            IsEnabledForType(t)
        });
    }
    props.push_back({ ServiceProperty::Kind::String, "GenericClient.BaseURL", "Base URL", "GenericClient", {}, {}, base_url });
    props.push_back({ ServiceProperty::Kind::Secret, "GenericClient.APIKey", "API Key", "GenericClient", "Fake API key", {}, token });
    props.push_back({ ServiceProperty::Kind::String, "GenericClient.Model", "Completion Model", "GenericClient", {}, {}, model });
    props.push_back({ ServiceProperty::Kind::String, "GenericClient.ImageModel", "Image Model", "GenericClient", {}, {}, image_model });
    props.push_back({ ServiceProperty::Kind::String, "GenericClient.TranscribeModel", "Transcribe Model", "GenericClient", {}, {}, transcribe_model });
    return props;
}

void GenericClient::SetProperty(const std::string& id, bool value) {
    for (auto t : GetTypes()) {
        if (id == std::string("GenericClient.Enable_") + aiType::TypeSettingsSuffix(t)) {
            SetEnabledForType(t, value);
            return;
        }
    }
}

void GenericClient::SetProperty(const std::string& id, const std::string& value) {
    if (id == "GenericClient.BaseURL") {
        base_url = value;
    } else if (id == "GenericClient.APIKey") {
        token = value;
    } else if (id == "GenericClient.Model") {
        model = value;
    } else if (id == "GenericClient.ImageModel") {
        image_model = value;
    } else if (id == "GenericClient.TranscribeModel") {
        transcribe_model = value;
    }
}

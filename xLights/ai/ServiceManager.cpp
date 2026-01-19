#include "ServiceManager.h"
#include "aiBase.h"
#include "chatGPT.h"
#include "ollama.h"

#ifdef __WXOSX__
#include "AppleIntelligence.h"
#endif

#include "xLightsMain.h"
#include "utils/Curl.h"
#include "UtilFunctions.h"

#include <wx/secretstore.h>

#include <memory>
#include <string>
#include <vector>

#include <log4cpp/Category.hh>

ServiceManager::ServiceManager(xLightsFrame* xl)
{
#if defined(__WXOSX__) && defined(__arm64__)
    if (wxCheckOsVersion(26, 0, 0)) {
        m_services.push_back(std::make_unique<AppleIntelligence>(this));
    }
#endif
    m_services.push_back(std::make_unique<chatGPT>(this));
    m_services.push_back(std::make_unique<ollama>(this));
    for (auto& service : m_services) {
        service->LoadSettings();
    }
}

ServiceManager::~ServiceManager() {
}

aiBase* ServiceManager::getService(std::string const& serviceName) {
    for (auto& service : m_services) {
        if (Lower(service->GetLLMName()) == Lower(serviceName)) {
            return service.get();
        }
    }
    return nullptr;
}

void ServiceManager::addService(std::unique_ptr<aiBase> service) {
    m_services.push_back(std::move(service));
}

aiBase* ServiceManager::findService(aiType::TYPE serviceType) {
    for (auto& service : m_services) {
        for (auto &t : service->GetTypes()) {
            if (t == serviceType && service->IsEnabled()) {
                return service.get();
            }
        }
    }
    return nullptr;
}

void ServiceManager::setServiceSetting(std::string const& key, int value) {
    wxConfigBase* config = wxConfigBase::Get();
    config->Write(wxString("xLightsServiceSettings" + key), value);
    config->Flush();
}

void ServiceManager::setServiceSetting(std::string const& key, bool value) {
    wxConfigBase* config = wxConfigBase::Get();
    config->Write(wxString("xLightsServiceSettings" + key), value);
    config->Flush();
}

void ServiceManager::setServiceSetting(std::string const& key, std::string const& value) {
    // It would be nice if we had a secret we  could encrypt these with
    wxConfigBase* config = wxConfigBase::Get();
    config->Write(wxString("xLightsServiceSettings" + key), wxString(value));
    config->Flush();
}

int ServiceManager::getServiceSetting(std::string const& key, int defaultValue) const {
    wxConfigBase* config = wxConfigBase::Get();
    return config->Read(wxString("xLightsServiceSettings" + key), defaultValue);
}

bool ServiceManager::getServiceSetting(std::string const& key, bool defaultValue) const {
    wxConfigBase* config = wxConfigBase::Get();
    return config->Read(wxString("xLightsServiceSettings" + key), defaultValue);
}

std::string ServiceManager::getServiceSetting(std::string const& key, std::string const& defaultValue) const {
    // It would be nice if we had a secret we  could encrypt these with
    wxConfigBase* config = wxConfigBase::Get();
    wxString const value = config->Read(wxString("xLightsServiceSettings" + key), wxString(defaultValue));
    return value.ToStdString();
}

#if wxUSE_SECRETSTORE
static wxSecretStore pwdStore = wxSecretStore::GetDefault();
std::string ServiceManager::getSecretServiceToken(std::string const& service) const {
    if (pwdStore.IsOk()) {
        wxSecretValue password;
        wxString usr;
        if (pwdStore.Load("xLightsServiceSettings" + service, usr, password)) {
            //user = usr;
            return password.GetAsString();
        }
    }
    return std::string();
}

void ServiceManager::setSecretServiceToken(std::string const& service, std::string const& token) {
    if (!pwdStore.IsOk()) {
       return;
    }

    wxSecretValue tt(token);
    if (!pwdStore.Save("xLightsServiceSettings" + service, "token", tt)) {
        printf("Failed to save %s\n", (const char *)service.c_str());
    }
}
#else
std::string ServiceManager::getSecretServiceToken(std::string const& service) const {
    return getServiceSetting(service + "_token", std::string());
}

void ServiceManager::setSecretServiceToken(std::string const& service, std::string const& token) {
    setServiceSetting(service + "_token", token);
}
#endif

/*
std::unique_ptr<aiBase> ServiceManager::GetLLM() {
    // we arrange these in priority order ... although in reality users are likely to only have one
    // maybe we need to give the user control over the order of use (although i am not sure when it would use anything other than the top one)

    auto gpt = std::make_unique<chatGPT>(chatGPT(this));
    if (gpt->IsAvailable()) {
        return gpt;
    }

    return nullptr;
}
*/

#include "ai/ServiceManager.h"
#include "ai/IServiceSettingsStore.h"
#include "ai/aiBase.h"
#include "ai/aiPlugin.h"
#include "ai/chatGPT.h"
#include "ai/claude.h"
#include "ai/gemini.h"
#include "ai/ollama.h"
#include "ai/GenericClient.h"
// AppleIntelligence (on-device FoundationModels palettes + ImagePlayground
// images + SFSpeechRecognizer lyrics) is gated to Apple Silicon — the
// FoundationModels API requires Apple Silicon, and SFSpeech's on-device
// path also wants it. Intel Macs and non-Apple platforms skip entirely.
#if defined(__APPLE__) && defined(__arm64__)
#include "ai/AppleIntelligence.h"
#endif
#ifdef HAVE_OPENVINO_GENAI
#include "ai/OpenVINOService.h"
#endif

#include "utils/string_utils.h"

#include <memory>
#include <string>
#include <vector>

#include <log.h>

// Settings keys are prefixed so services can pick short keys ("ClaudeModel")
// without worrying about collisions with the wider app config.
constexpr const char* kSettingsPrefix = "xLightsServiceSettings";

static std::string prefixKey(std::string_view key) {
    std::string s(kSettingsPrefix);
    s.append(key);
    return s;
}

// Helper: wrap a built-in service (deleted via operator delete)
static ServiceManager::ServicePtr makeBuiltin(aiBase* p) {
    return { p, [](aiBase* s) { delete s; } };
}

ServiceManager::ServiceManager(IServiceSettingsStore* store, const std::string& pluginDir)
    : m_store(store),
      m_pluginLoader({"AIPlugin", "xlCreateAIService", "xlDestroyAIService", "ServiceManager"})
{
#if defined(__APPLE__) && defined(__arm64__)
    // AppleIntelligence's enabled capabilities depend on the OS
    // version (FoundationModels on macOS 26 / iOS 26, ImagePlayground
    // on macOS 15.4 / iOS 18.4, SFSpeech everywhere). Skip
    // registration entirely if the running OS supports none of them
    // so the service doesn't show up as an empty entry in settings.
    {
        auto appleIntel = std::make_unique<AppleIntelligence>(this);
        if (!appleIntel->GetTypes().empty()) {
            m_services.push_back(makeBuiltin(appleIntel.release()));
        }
    }
#endif

    m_services.push_back(makeBuiltin(new chatGPT(this)));
    m_services.push_back(makeBuiltin(new claude(this)));
    m_services.push_back(makeBuiltin(new ollama(this)));
    m_services.push_back(makeBuiltin(new gemini(this)));
    m_services.push_back(makeBuiltin(new GenericClient(this)));
#ifdef HAVE_OPENVINO_GENAI
    m_services.push_back(makeBuiltin(new OpenVINOService(this)));
#endif
    
    if (!pluginDir.empty()) {
        loadPlugins(pluginDir);
    }

    for (auto& service : m_services) {
        service->LoadSettings();
    }
}

ServiceManager::~ServiceManager() {
    // m_services is destroyed before m_pluginLoader (reverse declaration order),
    // so all service destructors run while their DLLs are still mapped.
}

void ServiceManager::loadPlugins(const std::string& pluginDir) {
    m_pluginLoader.loadPlugins(
        pluginDir,
        [this] { return this; },
        [this](aiBase* service, PluginLoader<aiBase, ServiceManager*>::DestroyFn destroyFn, const std::string& path) {
            spdlog::info("ServiceManager: loaded AI plugin '{}' ({})", service->GetLLMName(), path);
            m_services.emplace_back(service, [destroyFn](aiBase* s) { destroyFn(s); });
        });
}

std::vector<aiBase*> ServiceManager::getServices() const {
    std::vector<aiBase*> ret;
    ret.reserve(m_services.size());
    for (auto const& s : m_services) {
        ret.push_back(s.get());
    }
    return ret;
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
    auto* raw = service.release();
    m_services.emplace_back(raw, [](aiBase* s) { delete s; });
    raw->LoadSettings();
}

void ServiceManager::addService(ServicePtr service) {
    auto* raw = service.get();
    m_services.push_back(std::move(service));
    raw->LoadSettings();
}

aiBase* ServiceManager::findService(aiType::TYPE serviceType) {
    for (auto& service : m_services) {
        for (auto& t : service->GetTypes()) {
            if (t == serviceType && service->IsEnabledForType(serviceType)) {
                return service.get();
            }
        }
    }
    return nullptr;
}

std::vector<aiBase*> ServiceManager::findServices(aiType::TYPE serviceType) {
    std::vector<aiBase*> ret;
    for (auto& service : m_services) {
        for (auto& t : service->GetTypes()) {
            if (t == serviceType && service->IsEnabledForType(serviceType)) {
                ret.push_back(service.get());
            }
        }
    }
    return ret;
}

void ServiceManager::setServiceSetting(std::string_view key, int value) {
    if (m_store) m_store->setInt(prefixKey(key), value);
}

void ServiceManager::setServiceSetting(std::string_view key, bool value) {
    if (m_store) m_store->setBool(prefixKey(key), value);
}

void ServiceManager::setServiceSetting(std::string_view key, std::string const& value) {
    if (m_store) m_store->setString(prefixKey(key), value);
}

int ServiceManager::getServiceSetting(std::string_view key, int defaultValue) const {
    return m_store ? m_store->getInt(prefixKey(key), defaultValue) : defaultValue;
}

bool ServiceManager::getServiceSetting(std::string_view key, bool defaultValue) const {
    return m_store ? m_store->getBool(prefixKey(key), defaultValue) : defaultValue;
}

std::string ServiceManager::getServiceSetting(std::string_view key, std::string const& defaultValue) const {
    return m_store ? m_store->getString(prefixKey(key), defaultValue) : defaultValue;
}

std::string ServiceManager::getSecretServiceToken(std::string_view serviceName) const {
    return m_store ? m_store->getSecret(prefixKey(serviceName)) : std::string();
}

void ServiceManager::setSecretServiceToken(std::string_view serviceName, std::string const& token) {
    if (m_store) m_store->setSecret(prefixKey(serviceName), token);
}

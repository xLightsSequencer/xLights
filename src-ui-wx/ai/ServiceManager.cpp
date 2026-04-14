#include "ServiceManager.h"
#include "aiBase.h"
#include "aiPlugin.h"
#include "chatGPT.h"
#include "claude.h"
#include "gemini.h"
#include "ollama.h"
#include "GenericClient.h"

#ifdef __WXOSX__
#include "AppleIntelligence.h"
#endif

#include "xLightsMain.h"
#include "utils/CurlManager.h"
#include "UtilFunctions.h"
#include "ui/shared/utils/wxUtilities.h"

#include <wx/secretstore.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <log.h>

// Helper: wrap a built-in service (deleted via operator delete)
static ServiceManager::ServicePtr makeBuiltin(aiBase* p) {
    return ServiceManager::ServicePtr(p, [](aiBase* s) { delete s; });
}

ServiceManager::ServiceManager(xLightsFrame* xl)
{
#if defined(__WXOSX__) && defined(__arm64__)
    {
        auto appleIntel = new AppleIntelligence(this);
        if (!appleIntel->GetTypes().empty()) {
            m_services.push_back(makeBuiltin(appleIntel));
        } else {
            delete appleIntel;
        }
    }
#endif
    m_services.push_back(makeBuiltin(new chatGPT(this)));
    m_services.push_back(makeBuiltin(new claude(this)));
    m_services.push_back(makeBuiltin(new ollama(this)));
    m_services.push_back(makeBuiltin(new gemini(this)));
    m_services.push_back(makeBuiltin(new GenericClient(this)));

    // Load plugin DLLs from <exe dir>/ai_plugins/
    wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
    std::string pluginDir = (exePath.GetPath() + wxFILE_SEP_PATH + "ai_plugins").ToStdString();
    loadPlugins(pluginDir);

    for (auto& service : m_services) {
        service->LoadSettings();
    }
}

ServiceManager::~ServiceManager() {
    // m_services is destroyed before m_pluginLibraries (reverse declaration order),
    // so all service destructors run while their DLLs are still mapped.
}

// PluginLibrary destructor: close the platform DLL handle
ServiceManager::PluginLibrary::~PluginLibrary() {
    if (handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
    }
}

void ServiceManager::loadPlugins(const std::string& pluginDir) {
    std::error_code ec;
    if (!std::filesystem::exists(pluginDir, ec) || ec) {
        return;
    }

#ifdef _WIN32
    constexpr const char* ext = ".dll";
#elif defined(__APPLE__)
    constexpr const char* ext = ".dylib";
#else
    constexpr const char* ext = ".so";
#endif

    for (auto const& entry : std::filesystem::directory_iterator(pluginDir, ec)) {
        if (ec) break;
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ext) continue;
        if (entry.path().string().find("AIPlugin") == std::string::npos)
            continue;

        std::string path = entry.path().string();

#ifdef _WIN32
        // LOAD_WITH_ALTERED_SEARCH_PATH makes Windows search the DLL's own
        // directory first, so whisper.dll / ggml.dll etc. can live alongside
        // the plugin without needing to be on the system PATH.
        void* handle = static_cast<void*>(
            LoadLibraryExA(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
#else
        void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
        // Helper: log to both spdlog and the VS Output window
#ifdef _WIN32
#define PLUGIN_WARN(msg) do { spdlog::warn(msg); OutputDebugStringA(("xLights plugin: " msg "\n")); } while(0)
#define PLUGIN_WARNF(ffmt, ...) do { auto _m = fmt::format("xLights plugin: " ffmt "\n", __VA_ARGS__); spdlog::warn(_m); OutputDebugStringA(_m.c_str()); } while(0)
#else
#define PLUGIN_WARN(msg)        spdlog::warn(msg)
#define PLUGIN_WARNF(ffmt, ...)  spdlog::warn(ffmt, __VA_ARGS__)
#endif

        if (!handle) {
#ifdef _WIN32
            PLUGIN_WARNF("failed to load '{}': error {}", path, GetLastError());
#else
            PLUGIN_WARNF("failed to load '{}': {}", path, dlerror());
#endif
            continue;
        }

#ifdef _WIN32
        auto createFn  = reinterpret_cast<xlCreateAIServiceFn> (GetProcAddress(static_cast<HMODULE>(handle), "xlCreateAIService"));
        auto destroyFn = reinterpret_cast<xlDestroyAIServiceFn>(GetProcAddress(static_cast<HMODULE>(handle), "xlDestroyAIService"));
#else
        auto createFn  = reinterpret_cast<xlCreateAIServiceFn> (dlsym(handle, "xlCreateAIService"));
        auto destroyFn = reinterpret_cast<xlDestroyAIServiceFn>(dlsym(handle, "xlDestroyAIService"));
#endif

        if (!createFn) {
            PLUGIN_WARNF("'{}' missing export xlCreateAIService", path);
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
            continue;
        }
        if (!destroyFn) {
            PLUGIN_WARNF("'{}' missing export xlDestroyAIService", path);
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
            continue;
        }

        aiBase* service = nullptr;
        try {
            service = createFn(this);
        } catch (const std::exception& ex) {
            PLUGIN_WARNF("'{}' xlCreateAIService threw: {}", path, ex.what());
        } catch (...) {
            PLUGIN_WARNF("'{}' xlCreateAIService threw unknown exception", path);
        }
        if (!service) {
            PLUGIN_WARNF("'{}' xlCreateAIService returned null", path);
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle));
#else
            dlclose(handle);
#endif
            continue;
        }

        spdlog::info("ServiceManager: loaded AI plugin '{}' ({})", service->GetLLMName(), path);
        m_pluginLibraries.emplace_back(handle);
        m_services.emplace_back(service, [destroyFn](aiBase* s) { destroyFn(s); });

#undef PLUGIN_WARN
#undef PLUGIN_WARNF
    }
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
    m_services.emplace_back(service.release(), [](aiBase* s) { delete s; });
}

void ServiceManager::addService(ServicePtr service) {
    m_services.push_back(std::move(service));
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

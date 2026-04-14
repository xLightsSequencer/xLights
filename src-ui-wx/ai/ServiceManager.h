#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#include "aiType.h"
#include "aiPlugin.h"

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

class xLightsFrame;
class aiBase;

class ServiceManager
{
public:
    // Custom-deleter unique_ptr: built-in services use `delete`, plugin services
    // use the DLL's xlDestroyAIService so memory is freed inside the correct module.
    using ServiceDeleter = std::function<void(aiBase*)>;
    using ServicePtr = std::unique_ptr<aiBase, ServiceDeleter>;

	explicit ServiceManager(xLightsFrame* xl);
	~ServiceManager();

	[[nodiscard]] aiBase* getService(std::string const& serviceName);

    // Add a built-in service (ownership transferred, deleted via operator delete).
    void addService(std::unique_ptr<aiBase> service);
    // Add a plugin service with an explicit destroy function (e.g. from a DLL).
    void addService(ServicePtr service);

    // Scan pluginDir for DLL/dylib/so files, load each one that exports the
    // xlCreateAIService / xlDestroyAIService entry points, and register the
    // resulting service.  Safe to call with a non-existent directory.
    void loadPlugins(const std::string& pluginDir);

    [[nodiscard]] aiBase* findService(aiType::TYPE serviceType);
    [[nodiscard]] std::vector<aiBase*> findServices(aiType::TYPE serviceType);

    void setServiceSetting(std::string const& key, int value);
    void setServiceSetting(std::string const& key, bool value);
	void setServiceSetting(std::string const& key, std::string const& value);

    [[nodiscard]] int getServiceSetting(std::string const& key, int defaultValue ) const;
    [[nodiscard]] bool getServiceSetting(std::string const& key, bool defaultValue) const;
    [[nodiscard]] std::string getServiceSetting(std::string const& key, std::string const& defaultValue) const;

    [[nodiscard]] std::string getSecretServiceToken(std::string const& service) const;
    void setSecretServiceToken(std::string const& service, std::string const& token);

    // Returns raw pointers to all registered services (built-in and plugin).
    [[nodiscard]] std::vector<aiBase*> getServices() const;

private:
    // RAII wrapper for a loaded plugin library handle.
    // Declared before m_services so it is destroyed AFTER m_services
    // (reverse construction order), guaranteeing the DLL stays mapped while
    // service destructors run.
    struct PluginLibrary {
        void* handle = nullptr;

        PluginLibrary() = default;
        explicit PluginLibrary(void* h) : handle(h) {}
        ~PluginLibrary();
        PluginLibrary(const PluginLibrary&) = delete;
        PluginLibrary& operator=(const PluginLibrary&) = delete;
        PluginLibrary(PluginLibrary&& o) noexcept : handle(o.handle) { o.handle = nullptr; }
        PluginLibrary& operator=(PluginLibrary&& o) noexcept {
            if (this != &o) { handle = o.handle; o.handle = nullptr; }
            return *this;
        }
    };

    std::vector<PluginLibrary> m_pluginLibraries; // must be declared before m_services
    std::vector<ServicePtr> m_services;
};

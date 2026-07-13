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

// Generic dlopen/LoadLibrary-based plugin loader, shared by ServiceManager
// (AI service plugins) and EffectManager (effect plugins). This class owns
// only the loaded library handles - it does NOT take ownership of the T*
// instances it creates. Callers receive each new item via `onLoaded` and are
// responsible for storing/destroying it (using the supplied destroy
// function). Because of this, embedders must declare their PluginLoader<T,
// Context> member BEFORE their own item-storage member, so C++'s reverse
// member-destruction order tears down items first and only then unloads the
// libraries they came from.

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include <log.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

template <typename T, typename Context>
class PluginLoader
{
public:
    using DestroyFn = void (*)(T*);
    // Invoked once per successfully loaded+created plugin item. The callback
    // takes ownership of `item` from this point on (wrap it with `destroy`
    // as the deleter). `path` is the plugin file that produced it, useful
    // for logging.
    using LoadedCallback = std::function<void(T* item, DestroyFn destroy, const std::string& path)>;
    // Invoked once per plugin file that failed to load/create, with a
    // human-readable reason. Optional - defaults to a no-op so existing
    // callers that only care about successes (logged separately via
    // spdlog::warn regardless) don't need to change.
    using FailedCallback = std::function<void(const std::string& path, const std::string& reason)>;

    struct Config {
        std::string fileNameFilter; // substring required in the filename, e.g. "AIPlugin"
        std::string createSymbol;   // exported factory function name, e.g. "xlCreateAIService"
        std::string destroySymbol;  // exported destroy function name, e.g. "xlDestroyAIService"
        std::string logPrefix;      // e.g. "ServiceManager", used in log lines
    };

    explicit PluginLoader(Config config) : m_config(std::move(config)) {}
    ~PluginLoader() = default;

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;
    PluginLoader(PluginLoader&&) = delete;
    PluginLoader& operator=(PluginLoader&&) = delete;

    // Scans pluginDir for DLL/dylib/so files whose name contains
    // fileNameFilter, loads each, and - if it exports both createSymbol and
    // destroySymbol - constructs a T via `contextProvider()` and invokes
    // onLoaded. contextProvider is called once per matched plugin file
    // (immediately before construction), so callers needing a distinct
    // context per plugin (e.g. a sequentially-assigned id) can do so, while
    // callers needing a fixed context (e.g. `this`) just return the same
    // value every time. Safe to call with a non-existent directory.
    void loadPlugins(const std::string& pluginDir, const std::function<Context()>& contextProvider, const LoadedCallback& onLoaded,
                      const FailedCallback& onFailed = {}) {
        using CreateFn = T* (*)(Context);

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
            if (entry.path().string().find(m_config.fileNameFilter) == std::string::npos)
                continue;

            std::string path = entry.path().string();

#ifdef _WIN32
            // LOAD_WITH_ALTERED_SEARCH_PATH makes Windows search the DLL's own
            // directory first, so dependent DLLs can live alongside the
            // plugin without needing to be on the system PATH.
            void* handle = static_cast<void*>(
                LoadLibraryExA(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
#else
            void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif

            if (!handle) {
                std::string reason;
#ifdef _WIN32
                reason = fmt::format("failed to load: error {}", GetLastError());
#else
                reason = fmt::format("failed to load: {}", dlerror());
#endif
                spdlog::warn("{}: '{}' {}", m_config.logPrefix, path, reason);
                if (onFailed) onFailed(path, reason);
                continue;
            }

#ifdef _WIN32
            auto createFn = reinterpret_cast<CreateFn>(
                GetProcAddress(static_cast<HMODULE>(handle), m_config.createSymbol.c_str()));
            auto destroyFn = reinterpret_cast<DestroyFn>(
                GetProcAddress(static_cast<HMODULE>(handle), m_config.destroySymbol.c_str()));
#else
            auto createFn  = reinterpret_cast<CreateFn>(dlsym(handle, m_config.createSymbol.c_str()));
            auto destroyFn = reinterpret_cast<DestroyFn>(dlsym(handle, m_config.destroySymbol.c_str()));
#endif

            if (!createFn) {
                std::string reason = fmt::format("missing export {}", m_config.createSymbol);
                spdlog::warn("{}: '{}' {}", m_config.logPrefix, path, reason);
                if (onFailed) onFailed(path, reason);
                closeLibrary(handle);
                continue;
            }
            if (!destroyFn) {
                std::string reason = fmt::format("missing export {}", m_config.destroySymbol);
                spdlog::warn("{}: '{}' {}", m_config.logPrefix, path, reason);
                if (onFailed) onFailed(path, reason);
                closeLibrary(handle);
                continue;
            }

            T* item = nullptr;
            std::string createFailReason;
            try {
                item = createFn(contextProvider());
            } catch (const std::exception& ex) {
                createFailReason = fmt::format("{} threw: {}", m_config.createSymbol, ex.what());
            } catch (...) {
                createFailReason = fmt::format("{} threw an unknown exception", m_config.createSymbol);
            }
            if (!item) {
                if (createFailReason.empty()) {
                    createFailReason = fmt::format("{} returned null", m_config.createSymbol);
                }
                spdlog::warn("{}: '{}' {}", m_config.logPrefix, path, createFailReason);
                if (onFailed) onFailed(path, createFailReason);
                closeLibrary(handle);
                continue;
            }

            m_libraries.emplace_back(handle);
            onLoaded(item, destroyFn, path);
        }
    }

private:
    // RAII wrapper for a loaded plugin library handle.
    struct PluginLibrary {
        void* handle = nullptr;

        explicit PluginLibrary(void* h) : handle(h) {}
        ~PluginLibrary() {
            if (handle) {
#ifdef _WIN32
                FreeLibrary(static_cast<HMODULE>(handle));
#else
                dlclose(handle);
#endif
            }
        }
        PluginLibrary(const PluginLibrary&) = delete;
        PluginLibrary& operator=(const PluginLibrary&) = delete;
        PluginLibrary(PluginLibrary&& o) noexcept : handle(o.handle) { o.handle = nullptr; }
        PluginLibrary& operator=(PluginLibrary&& o) noexcept {
            if (this != &o) { handle = o.handle; o.handle = nullptr; }
            return *this;
        }
    };

    static void closeLibrary(void* handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
    }

    Config m_config;
    std::vector<PluginLibrary> m_libraries; // must outlive every item handed to onLoaded
};

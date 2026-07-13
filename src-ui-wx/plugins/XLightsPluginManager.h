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

#include "xLightsPlugin.h"
#include "XLightsPluginHost.h"
#include "utils/PluginLoader.h"

#include <memory>
#include <string>
#include <vector>

// Owns the XLightsPluginHost (the ABI-facing IXLightsPluginHost
// implementation) and the loaded plugin instances. Must be constructed
// AFTER xLightsFrame::CreateSequencer() - unlike EffectManager's plugin
// loading (which must run BEFORE the Effects toolbox is built), UI plugins
// need the AUI manager and Tools menu to already exist.
class XLightsPluginManager {
public:
    explicit XLightsPluginManager(xLightsFrame* frame);
    ~XLightsPluginManager() = default;

    XLightsPluginManager(const XLightsPluginManager&) = delete;
    XLightsPluginManager& operator=(const XLightsPluginManager&) = delete;

    // Scans pluginDir for xLights Plugin DLLs (exporting xlCreateXLightsPlugin/
    // xlDestroyXLightsPlugin) and loads each one. Safe on a non-existent dir.
    void loadPlugins(const std::string& pluginDir);

    struct PluginInfo {
        std::string name;
        std::string path;
        bool loaded = false;
        std::string error;
    };
    [[nodiscard]] const std::vector<PluginInfo>& GetPluginInfo() const { return m_pluginInfo; }

private:
    // Declared before m_plugins so it (and the DLL handles it owns) is
    // destroyed AFTER m_plugins (reverse declaration order), guaranteeing the
    // DLLs stay mapped while plugin destructors run.
    PluginLoader<IXLightsPlugin, IXLightsPluginHost*> m_pluginLoader{
        {"XLightsPlugin", "xlCreateXLightsPlugin", "xlDestroyXLightsPlugin", "XLightsPluginManager"}
    };
    XLightsPluginHost m_host;
    std::vector<std::unique_ptr<IXLightsPlugin, xlDestroyXLightsPluginFn>> m_plugins;
    std::vector<PluginInfo> m_pluginInfo;
};

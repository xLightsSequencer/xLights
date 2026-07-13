/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "XLightsPluginManager.h"

#include <log.h>

XLightsPluginManager::XLightsPluginManager(xLightsFrame* frame) : m_host(frame) {}

void XLightsPluginManager::loadPlugins(const std::string& pluginDir) {
    m_pluginLoader.loadPlugins(
        pluginDir,
        [this] { return &m_host; },
        [this](IXLightsPlugin* plugin, PluginLoader<IXLightsPlugin, IXLightsPluginHost*>::DestroyFn destroyFn, const std::string& path) {
            spdlog::info("XLightsPluginManager: loaded plugin '{}' ({})", plugin->GetName(), path);
            m_pluginInfo.push_back({plugin->GetName(), path, true, ""});
            m_plugins.emplace_back(plugin, destroyFn);
        },
        [this](const std::string& path, const std::string& reason) {
            m_pluginInfo.push_back({"", path, false, reason});
        });
}

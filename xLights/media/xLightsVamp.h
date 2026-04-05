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

// VAMP audio analysis plugin integration.
// Desktop-only; not needed on iPad (timing analysis done on desktop).

#include <list>
#include <map>
#include <string>
#include <vector>

#include "vamp-hostsdk/PluginLoader.h"

class AudioManager;

class xLightsVamp {
    Vamp::HostExt::PluginLoader* _loader = nullptr;
    std::map<std::string, Vamp::Plugin*> _plugins;
    std::vector<Vamp::Plugin*> _loadedPlugins;
    std::map<std::string, Vamp::Plugin*> _allplugins;
    std::vector<Vamp::Plugin*> _allloadedPlugins;
    void LoadPlugins(AudioManager* paudio);

public:
    struct PluginDetails {
    public:
        std::string Name;
        std::string Description;
    };

    xLightsVamp();
    ~xLightsVamp();
    static void ProcessFeatures(Vamp::Plugin::FeatureList& feature, std::vector<int>& starts, std::vector<int>& ends, std::vector<std::string>& labels);
    std::list<std::string> GetAvailablePlugins(AudioManager* paudio);
    std::list<std::string> GetAllAvailablePlugins(AudioManager* paudio);
    Vamp::Plugin* GetPlugin(std::string name);
};

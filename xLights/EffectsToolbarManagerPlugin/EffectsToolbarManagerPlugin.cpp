/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// "xLights Plugin": builds xlights_plugins/EffectsToolbarManagerXLightsPlugin.dll,
// which XLightsPluginManager::loadPlugins() picks up at startup (after
// CreateSequencer(), so the AUI manager + Tools menu already exist).
//
// Lets the user reorder and show/hide effects in the always-visible Effects
// toolbar (a real need once enough effects/plugins are registered that the
// toolbar gets cluttered), persisting the choice per-user so it survives
// restarts. v1 scope is intentionally just this - not the other toolbars
// (Main/Play/Edit/Windows/AC/View), which are hand-built from heterogeneous,
// sometimes-stateful items rather than a uniform named list like
// EffectManager gives us for effects.
//
// Deliberately includes NO wxWidgets headers and links no wx library - all
// UI goes through the abstract IPlugin* facade in xLightsPlugin.h (including
// the reorder dialog itself, a host-side wrapper around wx's built-in
// wxRearrangeDialog). See that header's comment for why a plugin can't
// construct wx objects itself.

#include "plugins/xLightsPlugin.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>

namespace {
    constexpr const char* CONFIG_FILE_NAME = "effectstoolbarmanager.json";
}

class EffectsToolbarManagerPlugin : public IXLightsPlugin {
public:
    explicit EffectsToolbarManagerPlugin(IXLightsPluginHost* host) : IXLightsPlugin(host)
    {
        host_->AddPluginsMenuAction("Manage Effects Toolbar...", [this] { ShowManagerDialog(); });

        // Apply any previously-saved order/visibility immediately, so the
        // toolbar reflects it from the very first frame rather than only
        // after the user reopens the manager dialog. The toolbar has already
        // been built with the full default list by this point (UI plugins
        // load after CreateSequencer(), which is after the Effects toolbar
        // is first populated), so this is a rebuild, not the initial build.
        std::vector<std::string> names;
        std::vector<bool> visible;
        if (LoadConfig(names, visible)) {
            host_->SetEffectsToolbarEffects(VisibleNamesInOrder(names, visible));
        }
    }

    [[nodiscard]] std::string GetName() const override { return "Effects Toolbar Manager"; }

private:
    void ShowManagerDialog()
    {
        std::vector<std::string> allNames = host_->GetAllEffectNames();

        std::vector<std::string> items;
        std::vector<bool> checked;

        std::vector<std::string> savedNames;
        std::vector<bool> savedVisible;
        if (LoadConfig(savedNames, savedVisible)) {
            // Start from the saved order, dropping anything that no longer
            // exists (e.g. an effect plugin was removed since the config was
            // saved).
            for (size_t i = 0; i < savedNames.size(); ++i) {
                if (std::find(allNames.begin(), allNames.end(), savedNames[i]) != allNames.end()) {
                    items.push_back(savedNames[i]);
                    checked.push_back(savedVisible[i]);
                }
            }
        }
        // Append anything not already accounted for - covers first run and
        // any newly-added effect/plugin - visible by default so nothing new
        // silently disappears from the toolbar.
        for (const std::string& name : allNames) {
            if (std::find(items.begin(), items.end(), name) == items.end()) {
                items.push_back(name);
                checked.push_back(true);
            }
        }

        if (!host_->ShowReorderDialog("Manage Effects Toolbar",
                                      "Choose which effects appear in the toolbar, and their order:",
                                      items, checked)) {
            return;
        }

        SaveConfig(items, checked);
        host_->SetEffectsToolbarEffects(VisibleNamesInOrder(items, checked));
    }

    static std::vector<std::string> VisibleNamesInOrder(const std::vector<std::string>& names, const std::vector<bool>& visible)
    {
        std::vector<std::string> result;
        for (size_t i = 0; i < names.size() && i < visible.size(); ++i) {
            if (visible[i]) {
                result.push_back(names[i]);
            }
        }
        return result;
    }

    [[nodiscard]] std::string ConfigPath() const
    {
        return host_->GetUserDataDir() + "/" + CONFIG_FILE_NAME;
    }

    [[nodiscard]] bool LoadConfig(std::vector<std::string>& names, std::vector<bool>& visible) const
    {
        std::ifstream in(ConfigPath());
        if (!in.is_open()) {
            return false;
        }
        nlohmann::json j;
        try {
            in >> j;
        } catch (const nlohmann::json::exception&) {
            return false;
        }
        if (!j.is_array()) {
            return false;
        }
        for (const auto& entry : j) {
            if (!entry.is_object()) {
                continue;
            }
            names.push_back(entry.value("name", std::string()));
            visible.push_back(entry.value("visible", true));
        }
        return !names.empty();
    }

    void SaveConfig(const std::vector<std::string>& names, const std::vector<bool>& visible) const
    {
        nlohmann::json j = nlohmann::json::array();
        for (size_t i = 0; i < names.size(); ++i) {
            j.push_back({{"name", names[i]}, {"visible", visible[i]}});
        }
        std::ofstream out(ConfigPath());
        if (out.is_open()) {
            out << j.dump(2);
        }
    }
};

XL_XLIGHTS_PLUGIN_EXPORT(EffectsToolbarManagerPlugin)

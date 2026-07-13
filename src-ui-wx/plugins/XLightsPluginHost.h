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

#include <memory>
#include <vector>

class xLightsFrame;
class wxCommandEvent;
class wxMenu;
class SelectedEffectChangedEvent;

// Implements IXLightsPluginHost by forwarding to the real xLightsFrame APIs
// (EffectPresetManager, ApplyEffectsPreset, MainSequencer selection,
// EVT_SELECTED_EFFECT_CHANGED/EVT_UNSELECTED_EFFECT, the AUI manager + Tools
// menu), and by constructing real wx widgets - wrapped behind IPluginWidget
// implementations - entirely on this (the host's) side. frame must outlive
// this object.
class XLightsPluginHost : public IXLightsPluginHost {
public:
    explicit XLightsPluginHost(xLightsFrame* frame);
    ~XLightsPluginHost() override;

    XLightsPluginHost(const XLightsPluginHost&) = delete;
    XLightsPluginHost& operator=(const XLightsPluginHost&) = delete;

    [[nodiscard]] std::vector<std::string> GetPresetPaths() const override;
    bool ApplyPreset(const std::string& presetPath) override;

    [[nodiscard]] const Effect* GetSelectedEffect() const override;
    void OnSelectionChanged(SelectionChangedFn callback) override;

    IPluginPanel* CreateDockablePanel(const std::string& paneName, const std::string& caption,
                                      const std::string& toolsMenuLabel) override;
    void AddPluginsMenuAction(const std::string& label, std::function<void()> callback) override;

    [[nodiscard]] std::vector<std::string> GetAllEffectNames() const override;
    void SetEffectsToolbarEffects(const std::vector<std::string>& orderedVisibleNames) override;

    bool ShowReorderDialog(const std::string& title, const std::string& message,
                           std::vector<std::string>& items, std::vector<bool>& checked) override;

    [[nodiscard]] std::string GetUserDataDir() const override;

    // Takes ownership of a wx-backed IPluginWidget implementation and
    // returns the raw pointer handed back to the plugin. Public so the
    // wx-backed wrapper classes (defined in XLightsPluginHost.cpp) can use
    // it without needing friend access.
    IPluginWidget* AdoptWidget(std::unique_ptr<IPluginWidget> widget);

private:
    xLightsFrame* frame_;
    std::vector<SelectionChangedFn> selectionCallbacks_;
    bool boundSelectionEvents_ = false;
    // "Tools > Plugins" submenu, created lazily the first time any plugin
    // registers a dockable panel or a plain menu action.
    wxMenu* pluginsSubmenu_ = nullptr;
    // Owns every IPluginWidget (panels and everything added to them) handed
    // out to plugins - they live for the process, same as the panes/panels
    // that back them.
    std::vector<std::unique_ptr<IPluginWidget>> widgets_;

    wxMenu* EnsurePluginsSubmenu();
    void EnsureSelectionEventsBound();
    void OnEffectSelected(SelectedEffectChangedEvent& event);
    void OnEffectUnselected(wxCommandEvent& event);
};

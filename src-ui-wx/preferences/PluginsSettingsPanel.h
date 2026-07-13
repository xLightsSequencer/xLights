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

#include <wx/panel.h>

#include <utility>
#include <vector>

class wxListCtrl;
class xLightsFrame;

// Read-only view of loaded plugins, split into the two categories the
// PluginLoader<T,Context> framework currently supports: Effect Plugins
// (effects/EffectManager) and xLights Plugins (plugins/XLightsPluginManager -
// dockable panels / Tools-menu UI extensions). Hand-written (not wxSmith)
// since it's a simple, dynamically populated list rather than a fixed
// settings form.
class PluginsSettingsPanel : public wxPanel {
public:
    PluginsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY,
                          const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~PluginsSettingsPanel() = default;

private:
    xLightsFrame* frame;
    wxListCtrl* effectPluginsList = nullptr;
    wxListCtrl* xlightsPluginsList = nullptr;
    // Files copied into effect_plugins/ or xlights_plugins/ this session that
    // the respective manager hasn't loaded yet (both only scan once, at
    // startup) - (display name, path).
    std::vector<std::pair<wxString, wxString>> m_pendingAdds;
    std::vector<std::pair<wxString, wxString>> m_pendingXLightsPluginAdds;

    void PopulateEffectPluginsList();
    void OnAddEffectPluginClick(wxCommandEvent& event);
    void OnRemoveEffectPluginClick(wxCommandEvent& event);
    void PopulateXLightsPluginsList();
    void OnAddXLightsPluginClick(wxCommandEvent& event);
    void OnRemoveXLightsPluginClick(wxCommandEvent& event);

    // Shared by both Remove handlers - knownCount is how many rows at the
    // front of `list` come from the manager's own (already-loaded, from
    // startup) PluginInfo list, so a selected row past that is one of
    // `pendingAdds` instead. A pending (never-loaded-this-session) entry is
    // deleted outright; a known entry is disabled by renaming to "<path>.removed"
    // instead - DeleteFile fails on Windows while a DLL stays loaded/mapped,
    // but rename (the same trick self-updaters use) generally succeeds even
    // then, and it's reversible.
    void RemoveSelectedPlugin(wxListCtrl* list, size_t knownCount,
                              std::vector<std::pair<wxString, wxString>>& pendingAdds,
                              const wxString& kindLabel);
};

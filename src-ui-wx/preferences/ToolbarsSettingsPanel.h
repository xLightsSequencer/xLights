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

#include <string>
#include <utility>
#include <vector>

class wxBoxSizer;
class wxCommandEvent;
class wxRearrangeCtrl;
class xLightsFrame;

// "Toolbars" Preferences page. v1 only manages the always-visible Effects
// toolbar (see ToolbarLayout.h for the persisted shape) - a second toolbar
// could get its own section here later without touching the storage format.
// Hand-written (not wxSmith), same reasoning as PluginsSettingsPanel: a
// dynamically populated list, not a fixed settings form.
class ToolbarsSettingsPanel : public wxPanel {
public:
    ToolbarsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY,
                           const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~ToolbarsSettingsPanel() = default;

    virtual bool TransferDataFromWindow() override;

private:
    // wxRearrangeList exposes no API to reload its order/checked state in
    // place (GetCurrentOrder() is the only public read path), so "Reset to
    // Defaults" recreates the control rather than mutating it.
    void BuildRearrangeCtrl(const std::vector<std::pair<std::string, bool>>& layout);
    std::vector<std::pair<std::string, bool>> DefaultEffectsToolbarLayout() const;
    void OnResetToDefaults(wxCommandEvent& event);
    // Reorder (Up/Down) and check/uncheck are both consumed internally by
    // wxRearrangeCtrl/wxRearrangeList before reaching us, so this is bound
    // directly to the list's check event and the ctrl's Up/Down buttons
    // (event.Skip()'d through so their own internal state-tracking still
    // runs) rather than relying on one of those events bubbling up unheard.
    void OnRearrangeChanged(wxCommandEvent& event);

    xLightsFrame* frame;
    wxBoxSizer* _mainSizer = nullptr;
    wxRearrangeCtrl* _effectsToolbarRearrange = nullptr;
    // Items passed to the rearrange control, in their original (construction-
    // time) index order - GetCurrentOrder() returns indices into this array,
    // not names, so it's needed to decode the result back into names.
    std::vector<std::string> _effectsToolbarItemNames;
};

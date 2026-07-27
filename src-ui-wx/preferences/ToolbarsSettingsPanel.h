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
class wxButton;
class wxCommandEvent;
class wxDataViewEvent;
class wxDataViewListCtrl;
class xLightsFrame;

class ToolbarsSettingsPanel : public wxPanel {
public:
    ToolbarsSettingsPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id = wxID_ANY,
                           const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~ToolbarsSettingsPanel() = default;

    virtual bool TransferDataFromWindow() override;

private:
    void PopulateRows(const std::vector<std::pair<std::string, bool>>& layout);
    std::vector<std::pair<std::string, bool>> DefaultEffectsToolbarLayout() const;
    void OnResetToDefaults(wxCommandEvent& event);
    void OnToggleChanged(wxDataViewEvent& event);
    void OnSelectionChanged(wxDataViewEvent& event);
    void OnMoveUp(wxCommandEvent& event);
    void OnMoveDown(wxCommandEvent& event);
    void MoveSelectedRow(int direction);
    void UpdateMoveButtonState();
    std::string GetRowName(unsigned int row) const;

    xLightsFrame* frame;
    wxBoxSizer* _mainSizer = nullptr;
    wxDataViewListCtrl* _effectsToolbarList = nullptr;
    wxButton* _upButton = nullptr;
    wxButton* _downButton = nullptr;
    bool _suppressChangeEvents = false;
};

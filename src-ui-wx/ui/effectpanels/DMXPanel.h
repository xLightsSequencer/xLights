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

#include "JsonEffectPanel.h"

#include <list>

class Model;
class wxButton;
class wxNotebook;
class wxSizer;
class wxWindow;

class DMXPanel : public JsonEffectPanel {
public:
    DMXPanel(wxWindow* parent, const nlohmann::json& metadata);
    ~DMXPanel() override = default;

    void ValidateWindow() override;
    void SetDefaultParameters() override;
    void SetPanelStatus(Model* cls) override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    wxWindow* BuildChannelsNotebook(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildButtonsRow(wxWindow* parentWin, wxSizer* sizer);

    // Build one page of 16 channels starting at startChannel (1-based).
    void BuildChannelPage(wxWindow* pagePanel, int startChannel);

    // Handlers
    void OnRemapClick(wxCommandEvent& event);
    void OnRemapRClick(wxContextMenuEvent& event);
    void OnRemapBulkEdit(wxCommandEvent& event);
    void OnSaveAsStateClick(wxCommandEvent& event);
    void OnLoadFromStateClick(wxCommandEvent& event);

    std::list<Model*> GetActiveModels();

    wxNotebook* _notebook = nullptr;
    wxButton* _buttonRemap = nullptr;
    wxButton* _buttonSaveAsState = nullptr;
    wxButton* _buttonLoadState = nullptr;
};

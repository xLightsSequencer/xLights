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

#include "ui/effectpanels/JsonEffectPanel.h"
#include "ui/shared/controls/BulkEditControls.h"

#include <string>

class Model;
class SubBufferPanel;
class wxButton;
class wxCheckBox;
class wxSizer;

class BufferPanel : public JsonEffectPanel {
public:
    BufferPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize);
    ~BufferPanel() override = default;

    wxString GetBufferString();
    void SetDefaultControls(const Model* model, bool optionbased = false);
    void ValidateWindow() override;

    void UpdateBufferStyles(const Model* model);
    void UpdateCamera(const Model* model);

    // Used by import code to decide whether a camera tag is relevant.
    static bool CanRenderBufferUseCamera(const std::string& rb);

    // Exposed so the SubModelsDialog / other code can still use the subBuffer
    // widget if needed. Populated in BuildSubBufferRow.
    SubBufferPanel* subBufferPanel = nullptr;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    // Custom row builders
    wxWindow* BuildResetPanelRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildSubBufferRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildRotoZoomPresetRow(wxWindow* parentWin, wxSizer* sizer);

    // Handlers
    void OnResetBufferPanelClick(wxCommandEvent& event);
    void OnBufferStyleChoiceSelect(wxCommandEvent& event);
    void OnBufferTransformSelect(wxCommandEvent& event);
    void OnPresetSelect(wxCommandEvent& event);

    // Cached pointers
    wxCheckBox* _resetBufferPanelCheck = nullptr;
    BulkEditChoice* _rotoZoomPresetChoice = nullptr;

    std::string _defaultCamera = "2D";
    bool _mg = false;
};

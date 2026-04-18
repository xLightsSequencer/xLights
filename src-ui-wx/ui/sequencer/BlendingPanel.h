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
#include <vector>

class Model;
class wxButton;
class wxCheckBox;
class wxSizer;

// Layer-blending settings panel: layer-mix method / morph / canvas + fade-in /
// fade-out transitions. Historically named "TimingPanel" because the transitions
// live here, but the dominant content is how the layer blends with the layer
// below — the real timeline/timing UI is on the main sequencer, not in here.
// The on-disk setting-key prefix stays `T_` so existing .xsq files round-trip
// unchanged; only the user-visible label and the C++ class name are "Blending".
class BlendingPanel : public JsonEffectPanel {
public:
    BlendingPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize);
    ~BlendingPanel() override = default;

    // Layer-bracket info provided by xLightsFrame so the Layers button can
    // open the LayerSelectDialog with the right range.
    void SetLayersBelow(int start, int end, std::vector<int> effects, bool blending) {
        _startLayer = start;
        _endLayer = end;
        _layerWithEffect = effects;
        _modelBlending = blending;
    }
    void SetLayersSelected(std::string layersSelected) { _layersSelected = layersSelected; }

    // Custom serializer used by xLightsFrame::GetEffectTextFromWindows to
    // collect the T_-prefixed settings from this panel. Emits only non-
    // default values (the suppressIfDefault flag in Blending.json), replaces
    // the framework's default E_ prefix with T_, and appends the LayersSelected
    // blob when Canvas mode is enabled.
    wxString GetBlendingString();

    void SetDefaultControls(const Model* model, bool optionbased = false);
    void ValidateWindow() override;

    // Referenced from tabSequencer.cpp — keep as a public member pointer so
    // external check-then-enable-randomise logic keeps working.
    xlLockButton* BitmapButton_CheckBox_LayerMorph = nullptr;

    // Referenced from ShaderPanel.cpp and JsonEffectPanel.cpp (effect
    // canvasMode metadata hook) — keep as a public member pointer.
    BulkEditCheckBox* CheckBox_Canvas = nullptr;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    // Custom compound rows built by CreateCustomControl.
    wxWindow* BuildLayerMorphRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildLayerMethodRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildCanvasRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildTransitionHeader(wxWindow* parentWin, wxSizer* sizer, bool isIn);

    // Handlers
    void OnResetBlendingPanelClick(wxCommandEvent& event);
    void OnLayerMethodSelect(wxCommandEvent& event);
    void OnCanvasClick(wxCommandEvent& event);
    void OnLayersClick(wxCommandEvent& event);
    void OnAboutLayersClick(wxCommandEvent& event);
    void OnFadeinText(wxCommandEvent& event);
    void OnFadeoutText(wxCommandEvent& event);
    void OnFadeinDropdown(wxCommandEvent& event);
    void OnFadeoutDropdown(wxCommandEvent& event);
    void OnTransitionTypeSelect(wxCommandEvent& event);

    // Cached pointers (used by handlers / validate / serializer)
    wxCheckBox* _resetBlendingPanelCheck = nullptr;
    BulkEditCheckBox* _layerMorphCheck = nullptr;
    BulkEditSlider* _effectLayerMixSlider = nullptr;
    BulkEditTextCtrl* _effectLayerMixText = nullptr;
    xlLockButton* _layerMixLock = nullptr;
    BulkEditChoice* _layerMethodChoice = nullptr;
    wxButton* _aboutLayersButton = nullptr;
    wxButton* _layersButton = nullptr;

    BulkEditChoice* _inTypeChoice = nullptr;
    BulkEditComboBox* _fadeinCombo = nullptr;
    BulkEditChoice* _outTypeChoice = nullptr;
    BulkEditComboBox* _fadeoutCombo = nullptr;

    // Canvas / layer selection state
    std::string _layersSelected;
    int _startLayer = -1;
    int _endLayer = -1;
    bool _modelBlending = false;
    std::vector<int> _layerWithEffect;
};

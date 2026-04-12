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

class StateEffect;
class Model;
class wxRadioButton;
class wxChoice;

class StatePanel : public JsonEffectPanel {
public:
    StatePanel(wxWindow* parent, const nlohmann::json& metadata);
    ~StatePanel() override = default;

    void ValidateWindow() override;
    void SetDefaultParameters() override;
    void SetPanelStatus(Model* cls) override;
    void SetRenderableEffect(RenderableEffect* eff) override;

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    // Repopulate the State_State choice using the currently selected
    // State_StateDefinition and the current model.
    void PopulateStates();
    // Repopulate the State_TimingTrack choice from the sequence's timing tracks.
    void PopulateTimingTracks();
    // Enable/disable the state and timing-track choices based on which radio is selected.
    void UpdateStateSourceEnable();

    StateEffect* _effect = nullptr;
    Model* _model = nullptr;

    wxRadioButton* _radioState = nullptr;
    wxRadioButton* _radioTiming = nullptr;
    wxChoice* _stateChoice = nullptr;
    wxChoice* _timingTrackChoice = nullptr;
};

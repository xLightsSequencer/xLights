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

class StatePanel : public JsonEffectPanel {
public:
    StatePanel(wxWindow* parent, const nlohmann::json& metadata);
    ~StatePanel() override = default;

    void SetPanelStatus(Model* cls) override;
    void SetRenderableEffect(RenderableEffect* eff) override;

private:
    // Repopulate the State_State choice using the currently selected
    // State_StateDefinition and the current model.
    void PopulateStates();

    StateEffect* _effect = nullptr;
    Model* _model = nullptr;
};

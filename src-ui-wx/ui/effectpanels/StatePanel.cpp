/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "StatePanel.h"

#include <wx/choice.h>

#include "effects/StateEffect.h"
#include "models/Model.h"
#include "models/ModelGroup.h"

StatePanel::StatePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    // When the state-definition choice changes, repopulate the state-list
    // choice with the states inside the newly selected definition.
    auto* defChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_State_StateDefinition", this));
    if (defChoice) {
        defChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            PopulateStates();
            e.Skip();
        });
    }
}

void StatePanel::SetRenderableEffect(RenderableEffect* eff) {
    JsonEffectPanel::SetRenderableEffect(eff);
    _effect = dynamic_cast<StateEffect*>(eff);
}

void StatePanel::SetPanelStatus(Model* cls) {
    // Base class populates timing tracks and model-driven dynamicOptions
    // (including State_StateDefinition via dynamicOptions: "states").
    JsonEffectPanel::SetPanelStatus(cls);

    _model = cls;
    PopulateStates();
}

void StatePanel::PopulateStates() {
    auto* stateChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_State_State", this));
    auto* defChoice = dynamic_cast<wxChoice*>(
        wxWindow::FindWindowByName("ID_CHOICE_State_StateDefinition", this));
    if (stateChoice == nullptr) return;

    wxString selected = stateChoice->GetStringSelection();
    stateChoice->Clear();
    stateChoice->Append("<ALL>");

    if (_effect != nullptr && _model != nullptr && defChoice != nullptr) {
        std::string defName = defChoice->GetStringSelection().ToStdString();
        if (!defName.empty()) {
            std::list<std::string> states = _effect->GetStates(_model, defName);
            for (const auto& s : states) {
                stateChoice->Append(wxString(s));
            }
        }
    }

    // Restore the prior selection if it still exists; otherwise default to
    // <ALL> so users can see a sensible initial option.
    if (!selected.empty() && stateChoice->SetStringSelection(selected)) {
        // kept
    } else {
        stateChoice->SetSelection(0);
    }
}

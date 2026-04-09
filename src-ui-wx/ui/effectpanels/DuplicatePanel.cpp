/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DuplicatePanel.h"

#include <wx/choice.h>
#include <wx/spinctrl.h>

#include "effects/DuplicateEffect.h"
#include "render/SequenceElements.h"
#include "xLightsApp.h"
#include "xLightsMain.h"

DuplicatePanel::DuplicatePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    // When model choice changes, update layer spin range
    auto* modelCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Duplicate_Model", this));
    if (modelCtrl) {
        modelCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            auto* model = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Duplicate_Model", this));
            auto* spin = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Duplicate_Layer", this));
            if (model && spin && mSequenceElements != nullptr) {
                spin->SetRange(1, DuplicateEffect::GetLayersForModel(*mSequenceElements, model->GetStringSelection()));
            }
            e.Skip();
        });
    }
}

void DuplicatePanel::SetPanelStatus(Model* cls) {
    auto* modelCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Duplicate_Model", this));
    if (!modelCtrl) return;

    modelCtrl->Clear();
    if (cls == nullptr) return;

    xLightsFrame* xlights = xLightsApp::GetFrame();
    if (xlights == nullptr) return;

    auto& se = xlights->GetSequenceElements();
    for (const auto& it : se.GetAllElementNamesWithEffectsExtended()) {
        if (it != cls->GetFullName()) modelCtrl->AppendString(it);
    }
}

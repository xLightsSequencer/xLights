/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "WavePanel.h"

#include <wx/choice.h>

#include "ui/shared/controls/BulkEditControls.h"

WavePanel::WavePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    auto* typeCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Wave_Type", this));
    if (typeCtrl) typeCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    ValidateWindow();
}

void WavePanel::ValidateWindow() {
    auto* typeCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Wave_Type", this));
    if (!typeCtrl) return;

    bool isFractal = typeCtrl->GetStringSelection() == "Fractal/ivy";

    auto* wavesVC = dynamic_cast<BulkEditValueCurveButton*>(wxWindow::FindWindowByName("ID_VALUECURVE_Number_Waves", this));
    if (wavesVC) {
        if (isFractal) {
            wavesVC->GetValue()->SetActive(false);
            wavesVC->Disable();
        } else {
            wavesVC->Enable();
        }
    }
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SnowflakesPanel.h"
#include "EffectPanelUtils.h"

#include <wx/choice.h>

#include "ui/shared/controls/BulkEditControls.h"

SnowflakesPanel::SnowflakesPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    auto* fallingCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Falling", this));
    if (fallingCtrl) fallingCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    ValidateWindow();
}

void SnowflakesPanel::ValidateWindow() {
    auto* fallingCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Falling", this));
    if (!fallingCtrl) return;

    bool isDriving = fallingCtrl->GetStringSelection() == "Driving";

    // WarmupFrames: fully disabled in Driving mode
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Snowflakes_WarmupFrames", !isDriving);

    // Count and Speed: sliders stay enabled, but VCs are disabled in Driving mode
    auto* countVC = dynamic_cast<BulkEditValueCurveButton*>(wxWindow::FindWindowByName("ID_VALUECURVE_Snowflakes_Count", this));
    auto* speedVC = dynamic_cast<BulkEditValueCurveButton*>(wxWindow::FindWindowByName("ID_VALUECURVE_Snowflakes_Speed", this));

    if (isDriving) {
        if (countVC) { countVC->GetValue()->SetActive(false); countVC->Disable(); }
        if (speedVC) { speedVC->GetValue()->SetActive(false); speedVC->Disable(); }
        // Re-enable sliders/text that VC disable may have turned off
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Snowflakes_Count", true);
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Snowflakes_Speed", true);
    } else {
        if (countVC) countVC->Enable();
        if (speedVC) speedVC->Enable();
    }
}

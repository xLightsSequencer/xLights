/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "WarpPanel.h"
#include "EffectPanelUtils.h"

#include <wx/choice.h>

WarpPanel::WarpPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    // Connect choice change events to trigger ValidateWindow
    auto* typeCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Warp_Type", this));
    auto* treatmentCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Warp_Treatment_APPLYLAST", this));
    if (typeCtrl) typeCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    if (treatmentCtrl) treatmentCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    ValidateWindow();
}

void WarpPanel::ValidateWindow() {
    auto* typeCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Warp_Type", this));
    auto* treatmentCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Warp_Treatment_APPLYLAST", this));
    if (!typeCtrl || !treatmentCtrl) return;

    std::string warpType = typeCtrl->GetStringSelection().ToStdString();
    std::string warpTreatment = treatmentCtrl->GetStringSelection().ToStdString();

    // Force treatment to "constant" for types that only support constant
    bool constantOnly = warpType == "water drops" || warpType == "single water drop" ||
                        warpType == "wavy" || warpType == "sample on" ||
                        warpType == "copy" || warpType == "mirror" || warpType == "flip";
    if (constantOnly && !warpTreatment.empty() && warpTreatment != "constant") {
        treatmentCtrl->SetStringSelection("constant");
        warpTreatment = "constant";
        wxBell();
    }

    // X/Y: disable for dissolve, drop, wavy
    bool disableXY = warpType == "dissolve" || warpType == "drop" || warpType == "wavy";
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Warp_X", !disableXY);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Warp_Y", !disableXY);

    // Cycle Count: enabled for certain types with constant treatment
    bool supportsCycleCount = !(warpType == "water drops" || warpType == "wavy" ||
                                warpType == "sample on" || warpType == "mirror" ||
                                warpType == "transpose");
    if (warpTreatment != "constant") supportsCycleCount = false;
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Warp_Cycle_Count", supportsCycleCount);

    // Speed: only for certain types
    bool supportsSpeed = warpType == "water drops" || warpType == "ripple" ||
                         warpType == "circular swirl" || warpType == "wavy";
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Warp_Speed", supportsSpeed);

    // Frequency: only for banded swirl and ripple
    bool supportsFrequency = warpType == "banded swirl" || warpType == "ripple";
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Warp_Frequency", supportsFrequency);
}

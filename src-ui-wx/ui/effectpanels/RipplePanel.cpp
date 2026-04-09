/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RipplePanel.h"
#include "EffectPanelUtils.h"

#include <wx/choice.h>
#include <wx/filepicker.h>

RipplePanel::RipplePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Object_To_Draw", this));
    auto* styleCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Draw_Style", this));
    if (objectCtrl) objectCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    if (styleCtrl) styleCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    ValidateWindow();
}

void RipplePanel::ValidateWindow() {
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Object_To_Draw", this));
    auto* styleCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Draw_Style", this));
    if (!objectCtrl || !styleCtrl) return;

    std::string object = objectCtrl->GetStringSelection().ToStdString();
    std::string style = styleCtrl->GetStringSelection().ToStdString();

    // Points and Rotation enabled for Star, Snow Flake, Polygon
    bool hasPoints = (object == "Star" || object == "Snow Flake" || object == "Polygon");
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Points", hasPoints);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Rotation", hasPoints);

    // New features enabled when style is not "Old"
    bool newFeatures = (style != "Old");

    // Rotation also enabled for new draw styles
    if (newFeatures) {
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Rotation", true);
    }

    // Force SVG to Circle if not new features
    if (!newFeatures && object == "SVG") {
        objectCtrl->SetStringSelection("Circle");
        object = "Circle";
    }

    // SVG file picker only for SVG object in new mode
    auto* svgPicker = dynamic_cast<wxFilePickerCtrl*>(wxWindow::FindWindowByName("ID_FILEPICKERCTRL_Ripple_SVG", this));
    if (svgPicker) {
        bool enableSvg = newFeatures && object == "SVG";
        svgPicker->Enable(enableSvg);
        if (!enableSvg) svgPicker->SetFileName(wxFileName(""));
    }

    // New feature controls
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Scale", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Spacing", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Twist", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Direction", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Velocity", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Outline", newFeatures);
}

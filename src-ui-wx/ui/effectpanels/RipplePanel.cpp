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
#include <wx/sizer.h>
#include <wx/stattext.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/controls/MediaPickerCtrl.h"

RipplePanel::RipplePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, true) {
    // Defer build so CreateCustomControl virtual dispatch can reach this subclass.
    BuildFromJson(metadata);

    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Object_To_Draw", this));
    auto* styleCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Draw_Style", this));
    // Separate handlers so each choice can mediate the SVG/"Old" mutual
    // exclusion in the correct direction (the user changed THIS control,
    // so the OTHER one should yield).
    if (objectCtrl) {
        objectCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            OnObjectChanged();
            ValidateWindow();
            e.Skip();
        });
    }
    if (styleCtrl) {
        styleCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            OnStyleChanged();
            ValidateWindow();
            e.Skip();
        });
    }
    ValidateWindow();
}

void RipplePanel::OnObjectChanged() {
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Object_To_Draw", this));
    auto* styleCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Draw_Style", this));
    if (!objectCtrl || !styleCtrl) return;
    // SVG only renders under the new draw styles. If the user picks SVG
    // while Style is still "Old", bump Style to a sensible non-Old default
    // instead of silently snapping the object back to Circle.
    if (objectCtrl->GetStringSelection() == "SVG" &&
        styleCtrl->GetStringSelection() == "Old") {
        styleCtrl->SetStringSelection("Lines Outward");
    }
}

void RipplePanel::OnStyleChanged() {
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Object_To_Draw", this));
    auto* styleCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Draw_Style", this));
    if (!objectCtrl || !styleCtrl) return;
    // The reverse: Style "Old" doesn't support SVG, so revert Object to
    // Circle when the user picks Old while SVG is selected.
    if (styleCtrl->GetStringSelection() == "Old" &&
        objectCtrl->GetStringSelection() == "SVG") {
        objectCtrl->SetStringSelection("Circle");
    }
}

wxWindow* RipplePanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");

    if (id == "Ripple_SVG") {
        // Row: label | hidden file picker + MediaPickerCtrl
        auto* label = new wxStaticText(parentWin, wxID_ANY, "SVG File");
        sizer->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        // Hidden file picker holds the actual file path for serialization.
        auto* hiddenPicker = new BulkEditFilePickerCtrl(parentWin, wxNewId(), wxEmptyString, wxEmptyString,
                                                         "*.svg", wxDefaultPosition, wxDefaultSize,
                                                         wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                         wxDefaultValidator, "ID_FILEPICKERCTRL_Ripple_SVG");
        hiddenPicker->Hide();

        // Media picker integrates with the show's media library.
        _svgPicker = new MediaPickerCtrl(parentWin, wxID_ANY, MediaType::SVG);
        _svgPicker->SetLinkedPicker(hiddenPicker);
        sizer->Add(_svgPicker, 1, wxALL | wxEXPAND, 2);

        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);
        return _svgPicker;
    }
    return nullptr;
}

void RipplePanel::ValidateWindow() {
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Object_To_Draw", this));
    auto* styleCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Ripple_Draw_Style", this));
    if (!objectCtrl || !styleCtrl) return;

    std::string object = objectCtrl->GetStringSelection().ToStdString();
    std::string style = styleCtrl->GetStringSelection().ToStdString();

    // Points and Rotation enabled for Star, Snow Flake, Polygon
    bool hasPoints = (object == "Star" || object == "Snow Flake" || object == "Polygon");
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_RIPPLE_POINTS", hasPoints);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Rotation", hasPoints);

    // New features enabled when style is not "Old". The cross-mediation between
    // Object=SVG and Style=Old happens in OnObjectChanged()/OnStyleChanged()
    // so we can pick the right direction based on which control the user just
    // changed; here we only handle enable/disable.
    bool newFeatures = (style != "Old");

    // Rotation also enabled for new draw styles
    if (newFeatures) {
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Rotation", true);
    }

    // SVG media picker only for SVG object in new mode. The hidden file picker
    // (which holds the actual path for serialization) is enabled/disabled in
    // lockstep so that GetEffectStringFromWindow correctly excludes the SVG
    // setting when not in SVG mode.
    bool enableSvg = newFeatures && object == "SVG";
    if (_svgPicker) {
        _svgPicker->Enable(enableSvg);
        if (!enableSvg) _svgPicker->SetPath("");
    }
    auto* hiddenPicker = dynamic_cast<wxFilePickerCtrl*>(wxWindow::FindWindowByName("ID_FILEPICKERCTRL_Ripple_SVG", this));
    if (hiddenPicker) {
        hiddenPicker->Enable(enableSvg);
        if (!enableSvg) hiddenPicker->SetFileName(wxFileName(""));
    }

    // New feature controls
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Scale", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Spacing", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Twist", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Direction", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Velocity", newFeatures);
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Ripple_Outline", newFeatures);
}

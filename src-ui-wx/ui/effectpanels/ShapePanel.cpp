/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShapePanel.h"
#include "EffectPanelUtils.h"

#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/controls/MediaPickerCtrl.h"
#include "render/TextDrawingContext.h"
#include "ui/effects/CharMapDialog.h"

ShapePanel::ShapePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, true) {
    // Build now that virtual dispatch to CreateCustomControl will work
    BuildFromJson(metadata);

    // Bind change events for ValidateWindow
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Shape_ObjectToDraw", this));
    if (objectCtrl) objectCtrl->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });

    auto bindCheck = [this](const std::string& name) {
        auto* cb = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName(name, this));
        if (cb) cb->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });
    };
    bindCheck("ID_CHECKBOX_Shape_RandomLocation");
    bindCheck("ID_CHECKBOX_Shapes_RandomMovement");
    bindCheck("ID_CHECKBOX_Shape_UseMusic");
    bindCheck("ID_CHECKBOX_Shape_FireTiming");

    if (_fontPicker) _fontPicker->Bind(wxEVT_FONTPICKER_CHANGED, [this](wxFontPickerEvent& e) { ValidateWindow(); e.Skip(); });
    if (_charSpin) _charSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& e) { ValidateWindow(); e.Skip(); });
    if (_skinToneChoice) _skinToneChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { FireChangeEvent(); ValidateWindow(); e.Skip(); });

    ValidateWindow();
}

wxWindow* ShapePanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");

    if (id == "SVG") {
        // Row: label | hidden file picker + MediaPickerCtrl
        auto* label = new wxStaticText(parentWin, wxID_ANY, "SVG File");
        sizer->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        // Hidden file picker for serialization
        auto* hiddenPicker = new BulkEditFilePickerCtrl(parentWin, wxNewId(), wxEmptyString, wxEmptyString,
                                                         "*.svg", wxDefaultPosition, wxDefaultSize,
                                                         wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL,
                                                         wxDefaultValidator, "ID_FILEPICKERCTRL_SVG");
        hiddenPicker->Hide();

        // Media picker that links to the hidden file picker
        _svgPicker = new MediaPickerCtrl(parentWin, wxID_ANY, MediaType::SVG);
        _svgPicker->SetLinkedPicker(hiddenPicker);
        sizer->Add(_svgPicker, 1, wxALL | wxEXPAND, 2);

        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);
        return _svgPicker;
    }

    if (id == "Shape_Font") {
        // Row: label | font picker
        auto* label = new wxStaticText(parentWin, wxID_ANY, "Character");
        sizer->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        _fontPicker = new BulkEditFontPicker(parentWin, wxNewId(), wxNullFont,
                                              wxDefaultPosition, wxDefaultSize,
                                              wxFNTP_FONTDESC_AS_LABEL, wxDefaultValidator,
                                              "ID_FONTPICKER_Shape_Font");
        wxFont defaultFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, NATIVE_EMOJI_FONT);
        _fontPicker->SetSelectedFont(defaultFont);
        sizer->Add(_fontPicker, 1, wxALL | wxEXPAND, 2);
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);
        return _fontPicker;
    }

    if (id == "Shape_Char") {
        // Row: spacer | spin + emoji + browse button
        sizer->Add(-1, -1, 1, wxALL, 2);

        auto* rowSizer = new wxBoxSizer(wxHORIZONTAL);
        _charSpin = new BulkEditSpinCtrl(parentWin, wxNewId(), "127876",
                                          wxDefaultPosition, wxSize(65, -1),
                                          wxSP_ARROW_KEYS, 32, 917631, 127876,
                                          "ID_SPINCTRL_Shape_Char");
        rowSizer->Add(_charSpin, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

        _emojiDisplay = new wxStaticText(parentWin, wxID_ANY, " ", wxDefaultPosition, wxSize(30, -1));
        wxFont ef(20, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, NATIVE_EMOJI_FONT);
        _emojiDisplay->SetFont(ef);
        rowSizer->Add(_emojiDisplay, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

        auto* browseBtn = new wxButton(parentWin, wxNewId(), "Browse...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
        browseBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
            if (_fontPicker && _charSpin) {
                CharMapDialog dlg(this, _fontPicker->GetSelectedFont(), _charSpin->GetValue());
                dlg.ShowModal();
                _charSpin->SetValue(dlg.GetCharCode());
                wxSpinEvent sce;
                HandleSpinChange(sce);
                ValidateWindow();
            }
        });
        rowSizer->Add(browseBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

        sizer->Add(rowSizer, 1, wxALL | wxEXPAND, 0);
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);
        return _charSpin;
    }

    if (id == "Shape_SkinTone") {
        // Row: label | skin tone choice
        auto* label = new wxStaticText(parentWin, wxID_ANY, "Skin Tone");
        sizer->Add(label, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);

        _skinToneChoice = new BulkEditChoice(parentWin, wxNewId(), wxDefaultPosition, wxDefaultSize,
                                              0, nullptr, 0, wxDefaultValidator,
                                              "ID_CHOICE_Shape_SkinTone");
        _skinToneChoice->Append("Default");
        _skinToneChoice->Append("Light");
        _skinToneChoice->Append("Medium-Light");
        _skinToneChoice->Append("Medium");
        _skinToneChoice->Append("Medium-Dark");
        _skinToneChoice->Append("Dark");
        _skinToneChoice->SetSelection(0);
        sizer->Add(_skinToneChoice, 1, wxALL | wxEXPAND, 2);
        if (cols >= 3) sizer->Add(-1, -1, 1, wxALL, 1);
        if (cols >= 4) sizer->Add(-1, -1, 1, wxALL, 1);
        return _skinToneChoice;
    }

    return nullptr;
}

void ShapePanel::ValidateWindow() {
    auto* objectCtrl = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Shape_ObjectToDraw", this));
    if (!objectCtrl) return;
    std::string object = objectCtrl->GetStringSelection().ToStdString();

    // Points: enabled for Star, Ellipse, Polygon
    bool hasPoints = (object == "Star" || object == "Ellipse" || object == "Polygon");
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shape_Points", hasPoints);

    // Label changes: "Ratio" for Ellipse, "Points" for others
    auto* pointsLabel = dynamic_cast<wxStaticText*>(wxWindow::FindWindowByName("ID_STATICTEXT_Shape_Points", this));
    if (pointsLabel) pointsLabel->SetLabel(object == "Ellipse" ? "Ratio" : "Points");

    // SVG media picker
    if (_svgPicker) {
        _svgPicker->Enable(object == "SVG");
        if (object != "SVG") _svgPicker->SetPath("");
    }

    // Emoji controls
    bool isEmoji = (object == "Emoji");
    if (_fontPicker) _fontPicker->Enable(isEmoji);
    if (_charSpin) _charSpin->Enable(isEmoji);
    if (_skinToneChoice) {
        _skinToneChoice->Enable(isEmoji);
        if (!isEmoji) _skinToneChoice->SetSelection(0);
    }

    // Update emoji display
    if (_emojiDisplay) {
        if (isEmoji && _fontPicker && _charSpin) {
            wxFont f = _fontPicker->GetSelectedFont();
            wxString face = f.GetFaceName();
            if (face == WIN_NATIVE_EMOJI_FONT || face == OSX_NATIVE_EMOJI_FONT || face == LINUX_NATIVE_EMOJI_FONT) {
                f.SetFaceName(NATIVE_EMOJI_FONT);
            }
            f.SetPixelSize(wxSize(20, 20));
            _emojiDisplay->SetFont(f);
            wxString label = wxString(wxUniChar(_charSpin->GetValue()));
            if (_skinToneChoice && _skinToneChoice->GetSelection() > 0) {
                label.Append(wxString(wxUniChar(_skinToneChoice->GetSelection() + 0x1F3FA)));
            }
            _emojiDisplay->SetLabel(label);
        } else {
            _emojiDisplay->SetLabel(" ");
        }
    }

    // Thickness: disabled for Snowflake, Emoji
    bool noThickness = (object == "Snowflake" || object == "Emoji");
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shape_Thickness", !noThickness);

    // Hold Colour: disabled for SVG
    auto* holdColour = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Shape_HoldColour", this));
    if (holdColour) holdColour->Enable(object != "SVG");

    // Rotation: disabled for Emoji, Candy Cane, SVG, Random, Circle
    bool noRotation = (object == "Emoji" || object == "Candy Cane" || object == "SVG" ||
                       object == "Random" || object == "Circle");
    EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shape_Rotation", !noRotation);

    // Random Movement disables Direction/Velocity
    auto* randomMove = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Shapes_RandomMovement", this));
    if (randomMove) {
        bool isRandom = randomMove->IsChecked();
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shapes_Direction", !isRandom);
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shapes_Velocity", !isRandom);
    }

    // Random Location disables CentreX/Y
    auto* randomLoc = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Shape_RandomLocation", this));
    if (randomLoc) {
        bool isRandom = randomLoc->IsChecked();
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shape_CentreX", !isRandom);
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shape_CentreY", !isRandom);
    }

    // Use Music enables Sensitivity
    auto* useMusic = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Shape_UseMusic", this));
    if (useMusic) {
        EffectPanelUtils::enableControlsByName(this, "ID_SLIDER_Shape_Sensitivity", useMusic->IsChecked());
    }

    // Fire Timing enables TimingTrack, FilterLabel, FilterReg
    auto* fireTiming = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Shape_FireTiming", this));
    if (fireTiming) {
        bool ft = fireTiming->IsChecked();
        auto* timingTrack = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Shape_FireTimingTrack", this));
        if (timingTrack) timingTrack->Enable(ft);
        EffectPanelUtils::enableControlsByName(this, "ID_TEXTCTRL_Shape_FilterLabel", ft);
        auto* filterReg = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Shape_FilterReg", this));
        if (filterReg) filterReg->Enable(ft);
    }
}

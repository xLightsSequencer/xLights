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
#include <wx/menu.h>
#include <wx/fontenum.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/controls/MediaPickerCtrl.h"
#include "render/TextDrawingContext.h"
#include "ui/effects/CharMapDialog.h"

namespace {
class ShapePanelFontEnumerator : public wxFontEnumerator {
public:
    bool Exists(const std::string& font) const {
        return std::find(_fonts.begin(), _fonts.end(), font) != _fonts.end();
    }
    bool OnFacename(const wxString& facename) override {
        _fonts.push_back(facename.ToStdString());
        return true;
    }
private:
    std::list<std::string> _fonts;
};
} // namespace

ShapePanel::ShapePanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, true) {
    // Build now that virtual dispatch to CreateCustomControl will work
    BuildFromJson(metadata);

    // Populate the curated emoji shortcut list and wire up the right-click
    // context menu on the emoji display label.
    PopulateEmojiList();
    if (_emojiDisplay) {
        _emojiDisplay->Bind(wxEVT_CONTEXT_MENU, &ShapePanel::OnEmojiContextMenu, this);
    }

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

ShapePanel::~ShapePanel() {
    for (auto* e : _emojis) {
        delete e;
    }
    _emojis.clear();
}

void ShapePanel::PopulateEmojiList() {
    ShapePanelFontEnumerator fontEnumerator;
    fontEnumerator.EnumerateFacenames();

    if (fontEnumerator.Exists("Webdings")) {
        _emojis.push_back(new ShapePanelEmoji("Spider", "Webdings", 33));
        _emojis.push_back(new ShapePanelEmoji("Web", "Webdings", 34));
        _emojis.push_back(new ShapePanelEmoji("Explosion", "Webdings", 42));
        _emojis.push_back(new ShapePanelEmoji("Cause Ribbon", "Webdings", 45));
        _emojis.push_back(new ShapePanelEmoji("Heart", "Webdings", 89));
        _emojis.push_back(new ShapePanelEmoji("Roses", "Webdings", 90));
        _emojis.push_back(new ShapePanelEmoji("Badge", "Webdings", 100));
        _emojis.push_back(new ShapePanelEmoji("Present 1", "Webdings", 101));
        _emojis.push_back(new ShapePanelEmoji("Fire Truck", "Webdings", 102));
        _emojis.push_back(new ShapePanelEmoji("Police Car", "Webdings", 112));
        _emojis.push_back(new ShapePanelEmoji("Musical Note", "Webdings", 175));
    }

    if (fontEnumerator.Exists("Wingdings")) {
        _emojis.push_back(new ShapePanelEmoji("Bell", "Wingdings", 37));
        _emojis.push_back(new ShapePanelEmoji("Candle", "Wingdings", 39));
        _emojis.push_back(new ShapePanelEmoji("Smiley", "Wingdings", 74));
        _emojis.push_back(new ShapePanelEmoji("Snowflake 1", "Wingdings", 84));
        _emojis.push_back(new ShapePanelEmoji("Cross 1", "Wingdings", 85));
        _emojis.push_back(new ShapePanelEmoji("Cross 2", "Wingdings", 86));
        _emojis.push_back(new ShapePanelEmoji("Cross 3", "Wingdings", 87));
        _emojis.push_back(new ShapePanelEmoji("Star of David", "Wingdings", 89));
        _emojis.push_back(new ShapePanelEmoji("Star", "Wingdings", 171));
    }

    if (fontEnumerator.Exists(NATIVE_EMOJI_FONT)) {
        _emojis.push_back(new ShapePanelEmoji("Snowman 1", NATIVE_EMOJI_FONT, 9924));
        _emojis.push_back(new ShapePanelEmoji("Snowman 2", NATIVE_EMOJI_FONT, 9927));
        _emojis.push_back(new ShapePanelEmoji("Snowflake 2", NATIVE_EMOJI_FONT, 10052));
        _emojis.push_back(new ShapePanelEmoji("Snowflake 3", NATIVE_EMOJI_FONT, 10053));
        _emojis.push_back(new ShapePanelEmoji("Snowflake 4", NATIVE_EMOJI_FONT, 10054));

        _emojis.push_back(new ShapePanelEmoji("Christmas Tree", NATIVE_EMOJI_FONT, 0x1F384));
        _emojis.push_back(new ShapePanelEmoji("Gift", NATIVE_EMOJI_FONT, 0x1F381));
        _emojis.push_back(new ShapePanelEmoji("Mr. Claus", NATIVE_EMOJI_FONT, 0x1F385));
        _emojis.push_back(new ShapePanelEmoji("Mrs. Claus", NATIVE_EMOJI_FONT, 0x1F936));
    }

    if (fontEnumerator.Exists("XmasDings")) {
        _emojis.push_back(new ShapePanelEmoji("Presents ->", "XmasDings", 49));
        _emojis.push_back(new ShapePanelEmoji("Baubles ->", "XmasDings", 66));
        _emojis.push_back(new ShapePanelEmoji("Bells ->", "XmasDings", 80));
        _emojis.push_back(new ShapePanelEmoji("Angels ->", "XmasDings", 85));
        _emojis.push_back(new ShapePanelEmoji("Stockings ->", "XmasDings", 87));
        _emojis.push_back(new ShapePanelEmoji("Trees ->", "XmasDings", 97));
        _emojis.push_back(new ShapePanelEmoji("Wreaths ->", "XmasDings", 104));
        _emojis.push_back(new ShapePanelEmoji("Hollies ->", "XmasDings", 107));
        _emojis.push_back(new ShapePanelEmoji("Candles ->", "XmasDings", 109));
        _emojis.push_back(new ShapePanelEmoji("Snowmen ->", "XmasDings", 113));
        _emojis.push_back(new ShapePanelEmoji("Santas ->", "XmasDings", 116));
        _emojis.push_back(new ShapePanelEmoji("Candy Canes ->", "XmasDings", 120));
    }
}

void ShapePanel::OnEmojiContextMenu(wxContextMenuEvent& event) {
    if (_emojis.empty()) return;
    wxMenu mnuEmoji;
    for (const auto* e : _emojis) {
        mnuEmoji.Append(wxNewId(), wxString(e->GetName()));
    }
    mnuEmoji.Connect(wxEVT_COMMAND_MENU_SELECTED,
                     (wxObjectEventFunction)&ShapePanel::OnEmojiMenuItem,
                     nullptr, this);
    PopupMenu(&mnuEmoji);
}

void ShapePanel::OnEmojiMenuItem(wxCommandEvent& event) {
    auto* eo = static_cast<wxMenu*>(event.GetEventObject());
    wxString item = eo->GetLabelText(event.GetId());
    for (const auto* e : _emojis) {
        if (wxString(e->GetName()) == item) {
            if (_fontPicker) {
                wxFont f;
                f.Create(10, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                         false, wxString(e->GetFont()), wxFONTENCODING_DEFAULT);
                if (f.IsOk()) {
                    _fontPicker->SetSelectedFont(f);
                    wxFontPickerEvent fpe;
                    HandleFontChange(fpe);
                }
            }
            if (_charSpin) {
                _charSpin->SetValue(e->GetChar());
                wxSpinEvent sce;
                HandleSpinChange(sce);
            }
            ValidateWindow();
            return;
        }
    }
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
        _skinToneChoice->Append("Medium Light");
        _skinToneChoice->Append("Medium");
        _skinToneChoice->Append("Medium Dark");
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

    // SVG media picker. The hidden BulkEditFilePickerCtrl holds the actual
    // path for serialization, so it must be enabled/disabled in lockstep —
    // otherwise GetEffectStringFromWindow (which only skips *disabled* controls)
    // will leak E_FILEPICKERCTRL_SVG into the effect string when Object != SVG.
    bool isSvg = (object == "SVG");
    if (_svgPicker) {
        _svgPicker->Enable(isSvg);
        if (!isSvg) _svgPicker->SetPath("");
    }
    auto* hiddenSvgPicker = dynamic_cast<wxFilePickerCtrl*>(wxWindow::FindWindowByName("ID_FILEPICKERCTRL_SVG", this));
    if (hiddenSvgPicker) {
        hiddenSvgPicker->Enable(isSvg);
        if (!isSvg) hiddenSvgPicker->SetFileName(wxFileName(""));
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

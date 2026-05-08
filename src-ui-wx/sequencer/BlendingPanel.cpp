/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "sequencer/BlendingPanel.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tooltip.h>

#include <algorithm>
#include <vector>

#include "effectpanels/EffectPanelManager.h"
#include "effectpanels/EffectPanelUtils.h"
#include "sequencer/LayerSelectDialog.h"
#include "shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"
#include "models/Model.h"
#include "render/ValueCurveConsts.h"
#include "xLightsMain.h"
#include "UtilFunctions.h"

namespace {
// Long tooltip shown on the Layer Method choice and also used by the "?"
// About button as a quick reference popup. Kept verbatim from the legacy
// panel so users see the same text.
constexpr const char* LAYER_METHOD_TOOLTIP =
    "Layering defines how Effect 1 and Effect 2 will be mixed together.\n"
    "Here are the Choices\n"
    "* Normal: This is the same as 1 reveals 2.\n"
    "* Effect 1: Shows only Effect 1. Slide the slider to the right to blend in some Effect 2. \n"
    "* Effect 2: Shows only Effect 2. Slide the slider to the right to blend in some Effect 1.\n"
    "* 1 is Mask: (Shadow) Effect 1 will cast a shadow onto Effect 2 for every Effect 1 pixel that has a non-black value.\n"
    "* 2 is Mask: (Shadow) Effect 2 will cast a shadow onto Effect 1 for every Effect 2 pixel that has a non-black value.\n"
    "* 1 is Unmask: Unmask like but colours are revealed with no fade. Black becomes white.\n"
    "* 2 is Unmask: Unmask like but colours are revealed with no fade. Black becomes white.\n"
    "* 1 is True Unmask: (Mask) Only allow Effect 2 to show through when Effect 1 has a non-black pixel.\n"
    "* 2 is True Unmask: (Mask) Only allow Effect 1 to show through when Effect 2 has a non-black pixel.\n"
    "* Shadow 1 on 2: Take brightness and Saturation from 1, use hue from 2\n"
    "* Shadow 2 on 1: Take brightness and Saturation from 2, use hue from 1\n"
    "* 1 reveals 2: (Superimpose) Effect 1 reveals Effect 2\n"
    "* 2 reveals 1: (Superimpose) Effect 2 reveals Effect 1\n"
    "* Layered: Effect 1 only shows in black regions of Effect 2.\n"
    "* Average: Take value of Effect  and Add it to Value from Effect 2. Average the sum\n"
    "* Bottom-top: Effect 1 is put on bottom of model, Effect 2 is put on top in a split screen display\n"
    "* Left-Right: Effect goes 1 goes on the left side, Effect 2 on the right. Split screen goes down middle of model.\n"
    "* Highlight - Creates highlights by showcasing Effect 1's color where available, while using Effect 2's color where Effect 1 lacks color\n"
    "* Highlight Vibrant - Intensifies Effect 2's color where Effect 1 has content, without affectinng black or dark areas\n"
    "* Additive - Take value of Effect 1 and Add it to Value from Effect 2.\n"
    "* Subtractive - Take value of Effect 1 and Subtract it from the Value from Effect 2.\n"
    "* Brightness - Multiply each colour channel value of both layers and divide by 255.\n"
    "* Max - Take the maximum value for each channel from both effects\n"
    "* Min - Take the minimum value for each channel from both effects\n"
    "* Canvas - Blend the selected layers into this layer";

// Transitions that ignore the Adjustment slider — the slider/VC/text are
// disabled for these when selected. Matches the legacy panel layout.
const std::vector<wxString> TRANSITIONS_NO_ADJUST = {
    "Fade", "Square Explode", "Circle Explode", "Fold", "Dissolve",
    "Circular Swirl", "Zoom", "Doorway", "Swap", "Shatter"
};

// Transitions that ignore the Reverse checkbox.
const std::vector<wxString> TRANSITIONS_NO_REVERSE = {
    "Fade", "Slide Bars", "Blend", "Dissolve", "Circular Swirl", "Zoom",
    "Doorway", "Blobs", "Pinwheel", "Swap", "Shatter", "Circles"
};

// Layer method options in the legacy display order (the map order is
// alphabetical which isn't what we want).
const std::vector<wxString> LAYER_METHODS = {
    "Normal", "Effect 1", "Effect 2",
    "1 is Mask", "2 is Mask",
    "1 is Unmask", "2 is Unmask",
    "1 is True Unmask", "2 is True Unmask",
    "1 reveals 2", "2 reveals 1",
    "Shadow 1 on 2", "Shadow 2 on 1",
    "Layered", "Average", "Bottom-Top", "Left-Right",
    "Highlight", "Highlight Vibrant",
    "Additive", "Subtractive", "Brightness", "Max", "Min"
};

nlohmann::json LoadBlendingMetadata() {
    std::string metaDir = EffectPanelManager::GetMetadataDirectory();
    if (metaDir.empty()) return {};
    return JsonEffectPanel::LoadMetadata(metaDir + "/shared/Blending.json");
}
} // namespace

BlendingPanel::BlendingPanel(wxWindow* parent, wxWindowID /*id*/,
                         const wxPoint& /*pos*/, const wxSize& /*size*/) :
    JsonEffectPanel(parent, LoadBlendingMetadata(), /*deferBuild*/ true) {
    BuildFromJson(metadata_);

    // Cache pointers to the framework-built controls used by the validate
    // / serialize / default-reset logic. All guarded because lookup can
    // legitimately return nullptr if the JSON changes.
    if (auto* p = GetPropertyInfo("LayerMethodRow")) {
        // Custom row — _layerMethodChoice set directly in BuildLayerMethodRow.
        (void)p;
    }

    // Populate layer method choice in the legacy (non-alphabetical) order.
    // The JSON property is `LayerMethodRow` (custom), but the actual choice
    // lives in a subclass field set by BuildLayerMethodRow.
    if (_layerMethodChoice != nullptr) {
        for (const auto& m : LAYER_METHODS) {
            _layerMethodChoice->Append(m);
        }
        _layerMethodChoice->SetStringSelection("Normal");
        _layerMethodChoice->SetToolTip(wxString(LAYER_METHOD_TOOLTIP));
    }

    SetName("Blending");

    // Restore the 'Reset panel when changing effects' preference.
    auto* config = GetXLightsConfig();
    bool reset = true;
    if (config) config->Read("xLightsResetBlendingPanel", &reset, true);
    if (_resetBlendingPanelCheck) _resetBlendingPanelCheck->SetValue(reset);

    ValidateWindow();
    SetMinSize(wxSize(50, 50));
}

wxWindow* BlendingPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                            const nlohmann::json& prop, int /*cols*/) {
    std::string id = prop.value("id", "");
    if (id == "ResetPanelRow") {
        _resetBlendingPanelCheck = new wxCheckBox(parentWin, wxNewId(),
                                                 "Reset panel when changing effects",
                                                 wxDefaultPosition, wxDefaultSize, 0,
                                                 wxDefaultValidator,
                                                 _T("IDD_CHECKBOX_ResetBlendingPanel"));
        _resetBlendingPanelCheck->SetValue(true);
        sizer->Add(_resetBlendingPanelCheck, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
        _resetBlendingPanelCheck->Bind(wxEVT_CHECKBOX, &BlendingPanel::OnResetBlendingPanelClick, this);
        return _resetBlendingPanelCheck;
    }
    if (id == "LayerMorphRow") {
        return BuildLayerMorphRow(parentWin, sizer);
    }
    if (id == "LayerMethodRow") {
        return BuildLayerMethodRow(parentWin, sizer);
    }
    if (id == "CanvasRow") {
        return BuildCanvasRow(parentWin, sizer);
    }
    if (id == "In_Transition_Header") {
        return BuildTransitionHeader(parentWin, sizer, /*isIn*/ true);
    }
    if (id == "Out_Transition_Header") {
        return BuildTransitionHeader(parentWin, sizer, /*isIn*/ false);
    }
    return nullptr;
}

wxWindow* BlendingPanel::BuildLayerMorphRow(wxWindow* parentWin, wxSizer* sizer) {
    // 4-col layout: [Morph checkbox] [EffectLayerMix slider] [text] [lock]
    auto* row = new wxFlexGridSizer(0, 4, 0, 0);
    row->AddGrowableCol(1);

    _layerMorphCheck = new BulkEditCheckBox(parentWin, wxNewId(), "Morph",
                                             wxDefaultPosition, wxDefaultSize, 0,
                                             wxDefaultValidator,
                                             _T("ID_CHECKBOX_LayerMorph"));
    _layerMorphCheck->SetValue(false);
    _layerMorphCheck->SetToolTip("Gradual cross-fade from Effect1 to Effect2");
    row->Add(_layerMorphCheck, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    _effectLayerMixSlider = new BulkEditSlider(parentWin, wxNewId(), 0, 0, 100,
                                                wxDefaultPosition, wxDefaultSize, 0,
                                                wxDefaultValidator,
                                                _T("ID_SLIDER_EffectLayerMix"));
    _effectLayerMixSlider->SetMinSize(wxDLG_UNIT(parentWin, wxSize(30, -1)));
    row->Add(_effectLayerMixSlider, 1, wxALL | wxEXPAND, 1);

    _effectLayerMixText = new BulkEditTextCtrl(parentWin, wxNewId(), "0",
                                                wxDefaultPosition,
                                                wxDLG_UNIT(parentWin, wxSize(20, -1)), 0,
                                                wxDefaultValidator,
                                                _T("IDD_TEXTCTRL_EffectLayerMix"));
    _effectLayerMixText->SetMaxLength(3);
    row->Add(_effectLayerMixText, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 1);

    _layerMixLock = new xlLockButton(parentWin, wxNewId(), wxNullBitmap,
                                      wxDefaultPosition, wxSize(14, 14),
                                      wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                      _T("ID_BITMAPBUTTON_SLIDER_EffectLayerMix"));
    _layerMixLock->SetToolTip("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value.");
    _layerMixLock->Bind(wxEVT_BUTTON, &BlendingPanel::OnLockButtonClick, this);
    row->Add(_layerMixLock, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);

    // Also expose the Morph lock button (separate from the mix lock) since
    // external code at tabSequencer.cpp:2434 checks BitmapButton_CheckBox_LayerMorph.
    BitmapButton_CheckBox_LayerMorph = new xlLockButton(parentWin, wxNewId(), wxNullBitmap,
                                                        wxDefaultPosition, wxSize(14, 14),
                                                        wxBU_AUTODRAW | wxBORDER_NONE, wxDefaultValidator,
                                                        _T("ID_BITMAPBUTTON_CHECKBOX_LayerMorph"));
    BitmapButton_CheckBox_LayerMorph->Hide(); // legacy kept it on the row but it's rarely visible
    BitmapButton_CheckBox_LayerMorph->Bind(wxEVT_BUTTON, &BlendingPanel::OnLockButtonClick, this);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);
    return _layerMorphCheck;
}

wxWindow* BlendingPanel::BuildLayerMethodRow(wxWindow* parentWin, wxSizer* sizer) {
    // 2-col: [LayerMethod choice] [About "?" button]
    auto* row = new wxFlexGridSizer(0, 2, 0, 0);
    row->AddGrowableCol(0);

    _layerMethodChoice = new BulkEditChoice(parentWin, wxNewId(),
                                             wxDefaultPosition, wxDefaultSize, 0, nullptr,
                                             wxFULL_REPAINT_ON_RESIZE, wxDefaultValidator,
                                             _T("ID_CHOICE_LayerMethod"));
    row->Add(_layerMethodChoice, 1, wxALL | wxEXPAND, 2);
    _layerMethodChoice->Bind(wxEVT_CHOICE, &BlendingPanel::OnLayerMethodSelect, this);

    _aboutLayersButton = new wxButton(parentWin, wxNewId(), "?",
                                       wxDefaultPosition, wxSize(25, 23), 0,
                                       wxDefaultValidator, _T("IDD_BUTTON_ABOUT_LAYERS"));
    _aboutLayersButton->SetToolTip("About Layer Blending Types");
    row->Add(_aboutLayersButton, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    _aboutLayersButton->Bind(wxEVT_BUTTON, &BlendingPanel::OnAboutLayersClick, this);

    sizer->Add(row, 1, wxALL | wxEXPAND, 2);
    return _layerMethodChoice;
}

wxWindow* BlendingPanel::BuildTransitionHeader(wxWindow* parentWin, wxSizer* sizer, bool isIn) {
    // Compound single-line row: [transition type choice] + "Time (s)" label
    // + [fade combobox]. Matches the legacy layout and uses the same wx
    // control names so the framework's window-walk serializer picks them up.
    auto* row = new wxBoxSizer(wxHORIZONTAL);

    // Transition type choice
    wxString choiceName = isIn ? wxString("ID_CHOICE_In_Transition_Type")
                                 : wxString("ID_CHOICE_Out_Transition_Type");
    auto* typeChoice = new BulkEditChoice(parentWin, wxNewId(),
                                           wxDefaultPosition, wxDefaultSize,
                                           0, nullptr, 0,
                                           wxDefaultValidator, choiceName);
    static const wxString TRANSITION_TYPES[] = {
        "Blend", "Blinds", "Blobs", "Bow Tie", "Circle Explode", "Circles",
        "Circular Swirl", "Clock", "Dissolve", "Doorway", "Fade", "Fold",
        "From Middle", "Pinwheel", "Shatter", "Slide Bars", "Slide Checks",
        "Square Explode", "Star", "Swap", "Wipe", "Zoom"
    };
    for (const auto& t : TRANSITION_TYPES) {
        typeChoice->Append(t);
    }
    typeChoice->SetStringSelection("Fade");
    row->Add(typeChoice, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    typeChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) { ValidateWindow(); e.Skip(); });

    // "Time (s)" label
    auto* label = new wxStaticText(parentWin, wxID_ANY, "Time (s)");
    row->Add(label, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    // Fade combobox — construct empty, then AppendDefault + Append common values.
    wxString comboName = isIn ? wxString("ID_TEXTCTRL_Fadein")
                                : wxString("ID_TEXTCTRL_Fadeout");
    // No validator: wxFloatingPointValidator is locale-aware and rejects '.'
    // on locales that use ',' as the decimal separator, causing the value to
    // fall back to 0 on Windows. The RepairEmbeddedKey parser handles the
    // historic missing-comma corruption case defensively instead.
    auto* fadeCombo = new BulkEditComboBox(parentWin, wxNewId(), wxString("0.00"),
                                            wxDefaultPosition, wxSize(100, -1),
                                            0, nullptr, wxCB_SORT,
                                            wxDefaultValidator, comboName);
    for (const auto& v : { "0.00", "0.25", "0.50", "0.75", "1.00", "1.50", "2.00" }) {
        fadeCombo->AppendDefault(v);
        fadeCombo->Append(wxString(v));
    }
    fadeCombo->SetValue("0.00");
    row->Add(fadeCombo, 1, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    // Bind directly and explicitly start the save timer via FireChangeEvent.
    // AddListeners' Connect(id, wxEVT_TEXT) on the panel is supposed to
    // bubble-catch the combo's text events, but on GTK that path is flaky for
    // wxComboBox and the user's typed value can silently fail to reach the
    // save timer. Driving FireChangeEvent() here makes saves reliable on all
    // platforms. Skip so any other bubbling handlers still run.
    auto commit = [this](wxCommandEvent& e) {
        ValidateWindow();
        FireChangeEvent();
        e.Skip();
    };
    fadeCombo->Bind(wxEVT_TEXT, commit);
    fadeCombo->Bind(wxEVT_COMBOBOX, commit);
    // Safety net for GTK: Tab/focus-away always commits whatever the user typed.
    fadeCombo->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
        ValidateWindow();
        FireChangeEvent();
        e.Skip();
    });

    if (isIn) {
        _inTypeChoice = typeChoice;
        _fadeinCombo = fadeCombo;
    } else {
        _outTypeChoice = typeChoice;
        _fadeoutCombo = fadeCombo;
    }

    sizer->Add(row, 0, wxALL | wxEXPAND, 2);
    return typeChoice;
}

wxWindow* BlendingPanel::BuildCanvasRow(wxWindow* parentWin, wxSizer* sizer) {
    // Horizontal row: [Canvas checkbox][Layers ... button][spacer].
    // Checkbox and button sit right next to each other so the user can click
    // Layers immediately after enabling Canvas; the spacer absorbs the rest
    // of the width so the left cluster doesn't stretch.
    auto* row = new wxBoxSizer(wxHORIZONTAL);

    CheckBox_Canvas = new BulkEditCheckBox(parentWin, wxNewId(), "Canvas",
                                            wxDefaultPosition, wxDefaultSize, 0,
                                            wxDefaultValidator,
                                            _T("ID_CHECKBOX_Canvas"));
    CheckBox_Canvas->SetValue(false);
    row->Add(CheckBox_Canvas, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Canvas->Bind(wxEVT_CHECKBOX, &BlendingPanel::OnCanvasClick, this);

    _layersButton = new wxButton(parentWin, wxNewId(), "Layers ...",
                                  wxDefaultPosition, wxDefaultSize, 0,
                                  wxDefaultValidator, _T("IDD_BUTTON_Layers"));
    row->Add(_layersButton, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
    _layersButton->Bind(wxEVT_BUTTON, &BlendingPanel::OnLayersClick, this);

    row->AddStretchSpacer(1);

    sizer->Add(row, 0, wxALL | wxEXPAND, 2);
    return CheckBox_Canvas;
}

void BlendingPanel::OnResetBlendingPanelClick(wxCommandEvent& /*event*/) {
    auto* config = GetXLightsConfig();
    if (config && _resetBlendingPanelCheck) {
        config->Write("xLightsResetBlendingPanel", _resetBlendingPanelCheck->IsChecked());
    }
}

void BlendingPanel::OnLayerMethodSelect(wxCommandEvent& event) {
    if (_layerMethodChoice && _layerMethodChoice->GetStringSelection() == "Canvas") {
        wxCommandEvent eventUpdateEffect(EVT_UPDATE_EFFECT);
        wxPostEvent(GetParent(), eventUpdateEffect);
    }
    ValidateWindow();
    // Must Skip so the Connect()-wired HandleCommandChange on the parent
    // panel also runs and fires the save timer — otherwise the user's
    // choice is silently discarded.
    event.Skip();
}

void BlendingPanel::OnCanvasClick(wxCommandEvent& event) {
    ValidateWindow();
    event.Skip();
}

void BlendingPanel::OnLayersClick(wxCommandEvent& /*event*/) {
    wxASSERT(_startLayer <= _endLayer);
    LayerSelectDialog dlg(this, _startLayer, _endLayer, _modelBlending,
                          _layersSelected, _layerWithEffect);
    OptimiseDialogPosition(&dlg);
    if (dlg.ShowModal() == wxID_OK) {
        _layersSelected = dlg.GetSelectedLayers();
        FireChangeEvent();
    }
}

void BlendingPanel::OnAboutLayersClick(wxCommandEvent& /*event*/) {
    if (_layerMethodChoice && _layerMethodChoice->GetToolTip()) {
        ViewTempFile(_layerMethodChoice->GetToolTip()->GetTip(), "layerblendhelp");
    }
}

void BlendingPanel::OnFadeinText(wxCommandEvent& /*event*/) {
    ValidateWindow();
}

void BlendingPanel::OnFadeoutText(wxCommandEvent& /*event*/) {
    ValidateWindow();
}

void BlendingPanel::OnFadeinDropdown(wxCommandEvent& /*event*/) {
    if (auto* p = GetPropertyInfo("Fadein")) {
        if (auto* cb = dynamic_cast<BulkEditComboBox*>(p->comboBox)) {
            cb->PopulateComboBox();
        }
    }
}

void BlendingPanel::OnFadeoutDropdown(wxCommandEvent& /*event*/) {
    if (auto* p = GetPropertyInfo("Fadeout")) {
        if (auto* cb = dynamic_cast<BulkEditComboBox*>(p->comboBox)) {
            cb->PopulateComboBox();
        }
    }
}

void BlendingPanel::OnTransitionTypeSelect(wxCommandEvent& /*event*/) {
    ValidateWindow();
}

void BlendingPanel::SetDefaultControls(const Model* /*model*/, bool optionbased) {
    bool reset = !optionbased || (_resetBlendingPanelCheck && _resetBlendingPanelCheck->GetValue());
    if (!reset) {
        ValidateWindow();
        return;
    }

    _layersSelected = "";

    // Reset the framework-built rows to their JSON defaults.
    JsonEffectPanel::SetDefaultParameters();

    // Then the custom rows (not in properties_).
    if (_layerMorphCheck) _layerMorphCheck->SetValue(false);
    if (_layerMethodChoice) _layerMethodChoice->SetStringSelection("Normal");
    if (CheckBox_Canvas) CheckBox_Canvas->SetValue(false);
    if (_effectLayerMixText) _effectLayerMixText->SetValue("0");
    if (_effectLayerMixSlider) _effectLayerMixSlider->SetValue(0);
    if (_inTypeChoice) _inTypeChoice->SetStringSelection("Fade");
    if (_outTypeChoice) _outTypeChoice->SetStringSelection("Fade");
    if (_fadeinCombo) _fadeinCombo->SetValue("0.00");
    if (_fadeoutCombo) _fadeoutCombo->SetValue("0.00");

    // Value curve defaults for the transition adjustment sliders.
    auto resetVC = [this](const char* name) {
        auto* win = wxWindow::FindWindowByName(name, this);
        if (auto* vc = dynamic_cast<BulkEditValueCurveButton*>(win)) {
            vc->GetValue()->SetDefault(0.0f, 100.0f);
            vc->UpdateState();
        }
    };
    resetVC("ID_VALUECURVE_In_Transition_Adjust");
    resetVC("ID_VALUECURVE_Out_Transition_Adjust");

    ValidateWindow();
}

void BlendingPanel::ValidateWindow() {
    JsonEffectPanel::ValidateWindow();

    // Canvas mode requires a valid layer range before the Layers button
    // becomes usable.
    if (_layersButton) {
        if (CheckBox_Canvas && CheckBox_Canvas->GetValue() && _startLayer != -1) {
            _layersButton->Enable(true);
        } else {
            _layersSelected = "";
            _layersButton->Enable(false);
        }
    }

    // Clamp negative fade times (users can type anything into the combo box).
    // Use ToCDouble so '.' is the decimal separator regardless of the system
    // locale — otherwise wxAtof would return 0 for "0.50" on locales that
    // expect ',' as the separator and wrongly enable/disable the adjust rows.
    double fadeIn = 0.0;
    double fadeOut = 0.0;
    if (_fadeinCombo) {
        wxString v = _fadeinCombo->GetValue();
        bool ok = v.ToCDouble(&fadeIn);
        if (!ok || fadeIn < 0) { _fadeinCombo->SetValue("0.00"); fadeIn = 0; }
    }
    if (_fadeoutCombo) {
        wxString v = _fadeoutCombo->GetValue();
        bool ok = v.ToCDouble(&fadeOut);
        if (!ok || fadeOut < 0) { _fadeoutCombo->SetValue("0.00"); fadeOut = 0; }
    }

    // Compound enable/disable: transition adjust / reverse are only active
    // when fade time > 0 AND the selected type supports that parameter.
    auto* inAdjust = GetPropertyInfo("In_Transition_Adjust");
    auto* outAdjust = GetPropertyInfo("Out_Transition_Adjust");
    auto* inReverse = GetPropertyInfo("In_Transition_Reverse");
    auto* outReverse = GetPropertyInfo("Out_Transition_Reverse");

    auto enableRow = [this](JsonEffectPanel::PropertyInfo* p, bool enabled) {
        if (p == nullptr) return;
        if (p->slider) p->slider->Enable(enabled);
        if (p->buddyText) p->buddyText->Enable(enabled);
        if (p->textCtrl) p->textCtrl->Enable(enabled);
        if (p->valueCurveBtn) p->valueCurveBtn->Enable(enabled);
        if (p->checkBox) p->checkBox->Enable(enabled);
        // Also disable the sibling static label so the row dims as a unit.
        if (!p->id.empty()) {
            if (auto* label = wxWindow::FindWindowByName(
                    wxString::Format("ID_STATICTEXT_%s", p->id), this)) {
                label->Enable(enabled);
            }
        }
    };
    auto inList = [](const std::vector<wxString>& list, const wxString& val) {
        return std::find(list.cbegin(), list.cend(), val) != list.cend();
    };

    bool inEnable = fadeIn != 0.0;
    wxString inTypeSel = _inTypeChoice ? _inTypeChoice->GetStringSelection() : wxString("Fade");
    enableRow(inAdjust, inEnable && !inList(TRANSITIONS_NO_ADJUST, inTypeSel));
    enableRow(inReverse, inEnable && !inList(TRANSITIONS_NO_REVERSE, inTypeSel));

    bool outEnable = fadeOut != 0.0;
    wxString outTypeSel = _outTypeChoice ? _outTypeChoice->GetStringSelection() : wxString("Fade");
    enableRow(outAdjust, outEnable && !inList(TRANSITIONS_NO_ADJUST, outTypeSel));
    enableRow(outReverse, outEnable && !inList(TRANSITIONS_NO_REVERSE, outTypeSel));
}

wxString BlendingPanel::GetBlendingString() {
    // The framework's GetEffectString honours suppressIfDefault on every
    // JSON property, so we get a filtered settings string out of the box.
    // But the four transition header controls (Type choice + Fade combobox
    // for In and Out) are built as custom widgets, not JSON properties, so
    // they bypass the framework suppress logic. Strip them manually when at
    // default, then swap the E_ prefix for T_ and append the LayersSelected
    // blob when Canvas mode is active.
    wxString s = JsonEffectPanel::GetEffectString();

    auto stripSetting = [&s](const wxString& key) {
        // Match either leading "<key>=...," or ",<key>=...,".
        int start = 0;
        while ((start = s.find(key + "=")) != wxString::npos) {
            // Must start at beginning or immediately after a comma.
            if (start > 0 && s[start - 1] != ',') {
                start++;
                continue;
            }
            int end = s.find(",", start);
            if (end == wxString::npos) end = s.length();
            int removeFrom = start;
            int removeLen = end - start + 1; // include trailing comma
            // If we were at the very start, don't leave a leading comma.
            if (removeFrom + removeLen > (int)s.length()) removeLen = s.length() - removeFrom;
            s.erase(removeFrom, removeLen);
        }
    };
    // Fade "zero" is any of the equivalent string representations. Avoids
    // wxAtof, which is locale-aware on Windows — on a locale that uses ','
    // as the decimal separator wxAtof("0.50") returns 0 and would strip a
    // valid non-zero fade value. Also avoids stripping transiently while
    // the user is mid-edit: partial input like "0." is not in the list,
    // so the setting is preserved until they commit to an actual zero.
    auto isFadeZero = [](const wxString& v) {
        return v == "" || v == "0" || v == "0.0" || v == "0.00";
    };
    auto maybeStrip = [&](wxWindow* ctrl, const wxString& key, const wxString& defaultValue) {
        if (ctrl == nullptr) return;
        wxString value;
        if (auto* c = dynamic_cast<wxChoice*>(ctrl)) {
            value = c->GetStringSelection();
        } else if (auto* cb = dynamic_cast<wxComboBox*>(ctrl)) {
            value = cb->GetValue();
        }
        if (value == defaultValue) {
            stripSetting(key);
        }
    };
    maybeStrip(_inTypeChoice, "E_CHOICE_In_Transition_Type", "Fade");
    maybeStrip(_outTypeChoice, "E_CHOICE_Out_Transition_Type", "Fade");
    if (_fadeinCombo && isFadeZero(_fadeinCombo->GetValue())) {
        stripSetting("E_TEXTCTRL_Fadein");
    }
    if (_fadeoutCombo && isFadeZero(_fadeoutCombo->GetValue())) {
        stripSetting("E_TEXTCTRL_Fadeout");
    }

    // When the fade time is 0, the legacy panel also never writes the
    // adjust / reverse controls — the transition is inactive regardless of
    // what the slider / checkbox is set to. Mirror that behavior.
    bool fadeInZero = !_fadeinCombo || isFadeZero(_fadeinCombo->GetValue());
    bool fadeOutZero = !_fadeoutCombo || isFadeZero(_fadeoutCombo->GetValue());
    if (fadeInZero) {
        stripSetting("E_SLIDER_In_Transition_Adjust");
        stripSetting("E_VALUECURVE_In_Transition_Adjust");
        stripSetting("E_CHECKBOX_In_Transition_Reverse");
    }
    if (fadeOutZero) {
        stripSetting("E_SLIDER_Out_Transition_Adjust");
        stripSetting("E_VALUECURVE_Out_Transition_Adjust");
        stripSetting("E_CHECKBOX_Out_Transition_Reverse");
    }

    s.Replace(",E_", ",T_");
    if (s.StartsWith("E_")) {
        s = "T_" + s.Mid(2);
    }

    if (CheckBox_Canvas && CheckBox_Canvas->GetValue() && !_layersSelected.empty()) {
        if (!s.empty() && !s.EndsWith(",")) s += ",";
        s += "T_LayersSelected=";
        s += wxString(_layersSelected);
        s += ",";
    }
    return s;
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ServoPanel.h"
#include "EffectPanelUtils.h"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>

#include "ui/shared/controls/BulkEditControls.h"
#include "effects/ServoEffect.h"
#include "effects/EffectManager.h"
#include "render/Effect.h"
#include "xLightsApp.h"
#include "xLightsMain.h"

ServoPanel::ServoPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata, /*deferBuild*/ true) {
    BuildFromJson(metadata);

    // The Use Timing Track checkbox change requires us to refresh enable
    // states across the start/end controls (declarative visibility rules
    // would be too coarse — we have multiple interacting predicates).
    auto* tt = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Timing_Track", this));
    if (tt) {
        tt->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }
    auto* track = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Servo_TimingTrack", this));
    if (track) {
        track->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }

    ValidateWindow();
}

wxWindow* ServoPanel::CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                           const nlohmann::json& prop, int cols) {
    std::string id = prop.value("id", "");
    if (id == "Servo_StartEndRow") {
        return BuildStartEndRow(parentWin, sizer);
    }
    if (id == "Servo_ButtonRow") {
        return BuildButtonRow(parentWin, sizer);
    }
    return nullptr;
}

wxWindow* ServoPanel::BuildStartEndRow(wxWindow* parentWin, wxSizer* sizer) {
    // 5-column grid: linked-toggle | label | slider | VC button | text.
    // Two rows (Start, End). The End row reuses col 4 as a spacer because
    // only Start has a value-curve in the legacy panel.
    auto* grid = new wxFlexGridSizer(0, 5, 0, 0);
    grid->AddGrowableCol(2);

    // === Start row ===
    _startLinked = new LinkedToggleButton(parentWin, wxNewId(), wxEmptyString,
                                           wxDefaultPosition, wxDefaultSize,
                                           0, wxDefaultValidator,
                                           _T("ID_TOGGLEBUTTON_Start"));
    grid->Add(_startLinked, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    _startLabel = new wxStaticText(parentWin, wxID_ANY, _("Start:"),
                                    wxDefaultPosition, wxDefaultSize, 0,
                                    _T("ID_STATICTEXT_Servo"));
    grid->Add(_startLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

    _startSlider = new BulkEditSliderF1(parentWin, wxNewId(), 0, 0, 1000,
                                         wxDefaultPosition, wxDefaultSize,
                                         0, wxDefaultValidator,
                                         _T("IDD_SLIDER_Servo"));
    grid->Add(_startSlider, 1, wxALL | wxEXPAND, 2);

    _startVC = new BulkEditValueCurveButton(parentWin, wxNewId(),
                                             GetValueCurveNotSelectedBitmap(),
                                             wxDefaultPosition, wxDefaultSize,
                                             wxBU_AUTODRAW | wxBORDER_NONE,
                                             wxDefaultValidator,
                                             _T("ID_VALUECURVE_Servo"));
    _startVC->GetValue()->SetLimits(SERVO_MIN, SERVO_MAX);
    _startVC->GetValue()->SetDivisor(SERVO_DIVISOR);
    grid->Add(_startVC, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Connect(_startVC->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
            (wxObjectEventFunction)&ServoPanel::OnVCButtonClick);

    _startText = new BulkEditTextCtrlF1(parentWin, wxNewId(), _T("0"),
                                         wxDefaultPosition,
                                         wxDLG_UNIT(parentWin, wxSize(25, -1)),
                                         wxTE_PROCESS_ENTER, wxDefaultValidator,
                                         _T("ID_TEXTCTRL_Servo"));
    grid->Add(_startText, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    // === End row ===
    _endLinked = new LinkedToggleButton(parentWin, wxNewId(), wxEmptyString,
                                         wxDefaultPosition, wxDefaultSize,
                                         0, wxDefaultValidator,
                                         _T("ID_TOGGLEBUTTON_End"));
    grid->Add(_endLinked, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    _endLabel = new wxStaticText(parentWin, wxID_ANY, _("End:"),
                                  wxDefaultPosition, wxDefaultSize, 0,
                                  _T("ID_STATICTEXT1"));
    grid->Add(_endLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

    _endSlider = new BulkEditSliderF1(parentWin, wxNewId(), 0, 0, 1000,
                                       wxDefaultPosition, wxDefaultSize,
                                       0, wxDefaultValidator,
                                       _T("IDD_SLIDER_EndValue"));
    grid->Add(_endSlider, 1, wxALL | wxEXPAND, 2);

    grid->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    _endText = new BulkEditTextCtrlF1(parentWin, wxNewId(), _T("0"),
                                       wxDefaultPosition,
                                       wxDLG_UNIT(parentWin, wxSize(25, -1)),
                                       wxTE_PROCESS_ENTER, wxDefaultValidator,
                                       _T("ID_TEXTCTRL_EndValue"));
    grid->Add(_endText, 0, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

    sizer->Add(grid, 1, wxALL | wxEXPAND, 0);

    // Wire change events for sync + neighbor-effect propagation.
    _startSlider->Bind(wxEVT_SLIDER, &ServoPanel::OnStartValueUpdated, this);
    _startText->Bind(wxEVT_TEXT, &ServoPanel::OnStartValueUpdated, this);
    _endSlider->Bind(wxEVT_SLIDER, &ServoPanel::OnEndValueUpdated, this);
    _endText->Bind(wxEVT_TEXT, &ServoPanel::OnEndValueUpdated, this);

    return _startSlider;
}

wxWindow* ServoPanel::BuildButtonRow(wxWindow* parentWin, wxSizer* sizer) {
    auto* row = new wxFlexGridSizer(0, 4, 0, 0);
    row->AddGrowableCol(0);
    row->AddGrowableCol(1);

    _syncCheck = new wxCheckBox(parentWin, wxNewId(), _("Sync"),
                                 wxDefaultPosition, wxDefaultSize, 0,
                                 wxDefaultValidator, _T("IDD_CHECKBOX_Sync"));
    _syncCheck->SetValue(false);
    row->Add(_syncCheck, 1, wxALL | wxEXPAND, 5);

    row->Add(-1, -1, 1, wxALL | wxEXPAND, 5);

    _equalButton = new wxButton(parentWin, wxNewId(), _("Equal"),
                                 wxDefaultPosition, wxDefaultSize, 0,
                                 wxDefaultValidator, _T("ID_BUTTON1"));
    row->Add(_equalButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    _swapButton = new wxButton(parentWin, wxNewId(), _("Swap"),
                                wxDefaultPosition, wxDefaultSize, 0,
                                wxDefaultValidator, _T("IDD_SwapButton"));
    row->Add(_swapButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

    sizer->Add(row, 1, wxALL | wxEXPAND, 0);

    _syncCheck->Bind(wxEVT_CHECKBOX, &ServoPanel::OnSyncClicked, this);
    _equalButton->Bind(wxEVT_BUTTON, &ServoPanel::OnEqualClicked, this);
    _swapButton->Bind(wxEVT_BUTTON, &ServoPanel::OnSwapClicked, this);

    return _syncCheck;
}

void ServoPanel::OnSwapClicked(wxCommandEvent& event) {
    if (_startText && _endText) {
        wxString v1 = _startText->GetValue();
        wxString v2 = _endText->GetValue();
        _startText->SetValue(v2);
        _endText->SetValue(v1);
    }
    event.Skip();
}

void ServoPanel::OnEqualClicked(wxCommandEvent& event) {
    if (_startText && _endText) {
        _endText->SetValue(_startText->GetValue());
    }
    event.Skip();
}

void ServoPanel::OnSyncClicked(wxCommandEvent& event) {
    if (_syncCheck && _syncCheck->IsChecked() &&
        _startSlider && _endSlider && _startText && _endText) {
        _endSlider->SetValue(_startSlider->GetValue());
        _endText->SetValue(_startText->GetValue());
        FireChangeEvent();
    }
    event.Skip();
}

void ServoPanel::OnStartValueUpdated(wxCommandEvent& event) {
    // CRITICAL: Skip() must be called so that BulkEditSlider's own
    // SLIDER_UPDATED handler (which mirrors the slider value into the buddy
    // text control) still runs after this handler. Without Skip the slider
    // and the text box go out of sync.
    event.Skip();

    if (!_startSlider || !_startText || !_endSlider || !_endText) return;
    bool changed = false;

    if (_syncCheck && _syncCheck->IsChecked()) {
        _endSlider->SetValue(_startSlider->GetValue());
        _endText->ChangeValue(_startText->GetValue());
        changed = true;
    }
    if (_startLinked && _startLinked->IsEnabled() && _startLinked->GetValue()) {
        // Push the new start value into the END field of the previous Servo
        // effect on the same row, so consecutive servo effects chain smoothly.
        xLightsFrame::GetFrame()->CallOnEffectBeforeSelected([&changed, this](Effect* eff) {
            if (eff->GetEffectIndex() == EffectManager::eff_SERVO) {
                changed |= eff->SetSetting("E_TEXTCTRL_EndValue", _startText->GetValue());
            }
            return changed;
        });
    }
    if (changed) {
        FireChangeEvent();
    }
}

void ServoPanel::OnEndValueUpdated(wxCommandEvent& event) {
    // See OnStartValueUpdated — Skip() lets BulkEditSlider sync to its text.
    event.Skip();

    if (!_startSlider || !_startText || !_endSlider || !_endText) return;
    bool changed = false;

    if (_syncCheck && _syncCheck->IsChecked()) {
        _startSlider->SetValue(_endSlider->GetValue());
        _startText->ChangeValue(_endText->GetValue());
        changed = true;
    }
    if (_endLinked && _endLinked->IsEnabled() && _endLinked->GetValue()) {
        // Push the new end value into the START field of the next Servo effect.
        xLightsFrame::GetFrame()->CallOnEffectAfterSelected([&changed, this](Effect* eff) {
            if (eff->GetEffectIndex() == EffectManager::eff_SERVO) {
                changed |= eff->SetSetting("E_TEXTCTRL_Servo", _endText->GetValue());
            }
            return changed;
        });
    }
    if (changed) {
        FireChangeEvent();
    }
}

void ServoPanel::ValidateWindow() {
    // Skip JsonEffectPanel::ValidateWindow's visibility-rule pass — Servo
    // doesn't declare any rules, and the start/end interactions below need
    // direct enable/disable on widgets that aren't tracked in properties_.
    auto* tt = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowByName("ID_CHECKBOX_Timing_Track", this));
    auto* track = dynamic_cast<wxChoice*>(wxWindow::FindWindowByName("ID_CHOICE_Servo_TimingTrack", this));

    bool useTiming = tt && tt->IsChecked();
    bool hasTrack = track && !track->GetStringSelection().empty();
    bool vcActive = _startVC && _startVC->GetValue() && _startVC->GetValue()->IsActive();

    if (track) track->Enable(useTiming);

    if (useTiming && hasTrack) {
        // Phoneme-driven mode: every value control is irrelevant. Disable the
        // entire start/end+button area.
        if (_startSlider) _startSlider->Disable();
        if (_startText) _startText->Disable();
        if (_startVC) _startVC->Disable();
        if (_startLabel) _startLabel->Disable();
        if (_endSlider) _endSlider->Disable();
        if (_endText) _endText->Disable();
        if (_endLabel) _endLabel->Disable();
        if (_swapButton) _swapButton->Disable();
        if (_equalButton) _equalButton->Disable();
        if (_syncCheck) _syncCheck->Disable();
        if (_startLinked) _startLinked->Disable();
        if (_endLinked) _endLinked->Disable();
        return;
    }

    // Manual value mode: Start is always enabled.
    if (_startSlider) _startSlider->Enable();
    if (_startText) _startText->Enable();
    if (_startVC) _startVC->Enable();
    if (_startLabel) _startLabel->Enable();

    if (vcActive) {
        // Value curve drives the whole effect — there is no separate End,
        // just a single value over time. Repurpose the Start label and
        // disable everything that implies a start-vs-end split.
        if (_startLabel) _startLabel->SetLabelText(_("Value:"));
        if (_endSlider) _endSlider->Disable();
        if (_endText) _endText->Disable();
        if (_endLabel) _endLabel->Disable();
        if (_swapButton) _swapButton->Disable();
        if (_equalButton) _equalButton->Disable();
        if (_syncCheck) {
            _syncCheck->SetValue(false);
            _syncCheck->Disable();
        }
        if (_startLinked) _startLinked->Disable();
        if (_endLinked) _endLinked->Disable();
    } else {
        if (_startLabel) _startLabel->SetLabelText(_("Start:"));
        if (_endLabel) _endLabel->Enable();
        if (_swapButton) _swapButton->Enable();
        if (_equalButton) _equalButton->Enable();
        if (_syncCheck) {
            _syncCheck->Enable();
            // If the user has manually changed Start vs End apart, the panel
            // shouldn't claim they're still synced.
            if (_startText && _endText && _startText->GetValue() != _endText->GetValue()) {
                _syncCheck->SetValue(false);
            }
        }
        if (_endSlider) _endSlider->Enable();
        if (_endText) _endText->Enable();
        if (_startLinked) _startLinked->Enable();
        if (_endLinked) _endLinked->Enable();
    }
}

void ServoPanel::SetDefaultParameters() {
    JsonEffectPanel::SetDefaultParameters();
    if (_endLinked) _endLinked->SetValue(false);
    if (_startLinked) _startLinked->SetValue(false);
    if (_startVC) _startVC->SetActive(false);
    if (_startSlider) SetSliderValue(_startSlider, 0);
    if (_endSlider) SetSliderValue(_endSlider, 0);
    if (_syncCheck) _syncCheck->SetValue(false);
}

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VUMeterPanel.h"

#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/textctrl.h>

#include "ui/shared/controls/MediaPickerCtrl.h"
#include "effects/VUMeterEffect.h"
#include "render/SequenceElements.h"
#include "UtilFunctions.h"

namespace {
// Helper to look up a child control by name and dynamic_cast it. Returns
// nullptr if the control wasn't built (defensive — most lookups should hit).
template <typename T>
T* FindCtrl(wxWindow* parent, const wxString& name) {
    return dynamic_cast<T*>(wxWindow::FindWindowByName(name, parent));
}

bool IsType(const wxString& type, std::initializer_list<const char*> values) {
    for (const char* v : values) {
        if (type == v) return true;
    }
    return false;
}
} // namespace

VUMeterPanel::VUMeterPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    // Cache pointers to all the controls we need to enable/disable in
    // ValidateWindow. Doing this once avoids re-walking the window tree on
    // every event.
    _typeChoice        = FindCtrl<wxChoice>(this, "ID_CHOICE_VUMeter_Type");
    _shapeChoice       = FindCtrl<wxChoice>(this, "ID_CHOICE_VUMeter_Shape");
    _timingTrackChoice = FindCtrl<wxChoice>(this, "ID_CHOICE_VUMeter_TimingTrack");
    _logXAxisCheck     = FindCtrl<wxCheckBox>(this, "ID_CHECKBOX_VUMeter_LogarithmicX");
    _slowDownFalls     = FindCtrl<wxCheckBox>(this, "ID_CHECKBOX_VUMeter_SlowDownFalls");
    _regexCheck        = FindCtrl<wxCheckBox>(this, "ID_CHECKBOX_Regex");
    _filterText        = FindCtrl<wxTextCtrl>(this, "ID_TEXTCTRL_Filter");
    _barsSlider        = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_Bars");
    _barsText          = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_Bars");
    _gainSlider        = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_Gain");
    _gainText          = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_Gain");
    _gainVC            = wxWindow::FindWindowByName("ID_VALUECURVE_VUMeter_Gain", this);
    _sensSlider        = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_Sensitivity");
    _sensText          = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_Sensitivity");
    _startNoteSlider   = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_StartNote");
    _startNoteText     = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_StartNote");
    _endNoteSlider     = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_EndNote");
    _endNoteText       = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_EndNote");
    _xOffsetSlider     = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_XOffset");
    _xOffsetText       = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_XOffset");
    _yOffsetSlider     = FindCtrl<wxSlider>(this, "ID_SLIDER_VUMeter_YOffset");
    _yOffsetText       = FindCtrl<wxTextCtrl>(this, "IDD_TEXTCTRL_VUMeter_YOffset");
    _yOffsetVC         = wxWindow::FindWindowByName("ID_VALUECURVE_VUMeter_YOffset", this);

    // Replace the framework-built SVG file picker with a media-aware picker
    // (matching the legacy Sketch / Ripple / Shape pattern). The hidden file
    // picker still owns the path so the file is serialized as
    // E_FILEPICKERCTRL_SVGFile by GetEffectStringFromWindow.
    _hiddenSvgPicker = FindCtrl<wxFilePickerCtrl>(this, "ID_FILEPICKERCTRL_SVGFile");
    if (_hiddenSvgPicker != nullptr) {
        _hiddenSvgPicker->Hide();
        _svgMediaPicker = new MediaPickerCtrl(_hiddenSvgPicker->GetParent(), wxID_ANY, MediaType::SVG);
        _svgMediaPicker->SetLinkedPicker(_hiddenSvgPicker);
        if (auto* containing = _hiddenSvgPicker->GetContainingSizer()) {
            containing->Replace(_hiddenSvgPicker, _svgMediaPicker);
            containing->Layout();
        }
    }

    // Type / Shape changes need a re-validation. The framework only auto-binds
    // the visibility-rule conditions; here we have a fully custom ValidateWindow.
    if (_typeChoice) {
        _typeChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }
    if (_shapeChoice) {
        _shapeChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent& e) {
            ValidateWindow();
            e.Skip();
        });
    }

    // MIDI note cross-validation: when the user moves either Start or End
    // (via slider or by editing the text box and tabbing away), enforce
    // start <= end and refresh the slider tooltips with the note name.
    if (_startNoteSlider) {
        _startNoteSlider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& e) {
            EnforceNoteOrder(/*startChanged*/ true);
            RefreshNoteTooltips();
            e.Skip();
        });
    }
    if (_endNoteSlider) {
        _endNoteSlider->Bind(wxEVT_SLIDER, [this](wxCommandEvent& e) {
            EnforceNoteOrder(/*startChanged*/ false);
            RefreshNoteTooltips();
            e.Skip();
        });
    }
    if (_startNoteText) {
        _startNoteText->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
            EnforceNoteOrder(/*startChanged*/ true);
            RefreshNoteTooltips();
            e.Skip();
        });
    }
    if (_endNoteText) {
        _endNoteText->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& e) {
            EnforceNoteOrder(/*startChanged*/ false);
            RefreshNoteTooltips();
            e.Skip();
        });
    }

    if (_filterText) {
        _filterText->SetToolTip("Only trigger on timing events which contain this token in their text. Blank matches all. Multiple tokens can be ; separated in non-regex mode.");
    }

    RefreshNoteTooltips();
    ValidateWindow();
}

void VUMeterPanel::EnforceNoteOrder(bool startChanged) {
    if (!_startNoteSlider || !_endNoteSlider) return;
    int start = _startNoteSlider->GetValue();
    int end = _endNoteSlider->GetValue();
    if (end < start) {
        if (startChanged) {
            end = start;
            _endNoteSlider->SetValue(end);
            if (_endNoteText) _endNoteText->ChangeValue(wxString::Format("%d", end));
        } else {
            start = end;
            _startNoteSlider->SetValue(start);
            if (_startNoteText) _startNoteText->ChangeValue(wxString::Format("%d", start));
        }
    }
}

void VUMeterPanel::RefreshNoteTooltips() {
    if (_startNoteSlider) {
        _startNoteSlider->SetToolTip(wxString(DecodeMidi(_startNoteSlider->GetValue()).c_str()));
    }
    if (_endNoteSlider) {
        _endNoteSlider->SetToolTip(wxString(DecodeMidi(_endNoteSlider->GetValue()).c_str()));
    }
}

void VUMeterPanel::ValidateWindow() {
    if (!_typeChoice) return;
    wxString type = _typeChoice->GetStringSelection();

    // Helper to enable/disable a slider+text+VC trio.
    auto setRowEnabled = [](wxWindow* slider, wxWindow* text, wxWindow* vc, bool enabled) {
        if (slider) slider->Enable(enabled);
        if (text) text->Enable(enabled);
        if (vc) vc->Enable(enabled);
    };

    if (_logXAxisCheck) {
        _logXAxisCheck->Enable(IsType(type, {
            "Spectrogram", "Spectrogram Peak", "Spectrogram Circle Line", "Spectrogram Line"
        }));
    }

    setRowEnabled(_gainSlider, _gainText, _gainVC, IsType(type, {
        "Volume Bars", "Waveform", "Frame Waveform", "On", "Color On", "Intensity Wave",
        "Level Bar", "Level Random Bar", "Level Color", "Level Pulse", "Level Jump",
        "Level Jump 100", "Level Pulse Color", "Timing Event Jump", "Note On",
        "Note Level Bar", "Note Level Random Bar", "Note Level Pulse",
        "Note Level Jump", "Note Level Jump 100", "Spectrogram Circle Line", "Level Shape"
    }));

    bool timingEventType = IsType(type, {
        "Timing Event Spike", "Timing Event Sweep", "Timing Event Sweep 2",
        "Timing Event Timed Sweep", "Timing Event Timed Sweep 2",
        "Timing Event Alternate Timed Sweep", "Timing Event Alternate Timed Sweep 2",
        "Timing Event Timed Chase To Middle", "Timing Event Timed Chase From Middle",
        "Pulse", "Timing Event Color", "Timing Event Pulse", "Timing Event Bar",
        "Timing Event Bar Bounce", "Timing Event Random Bar", "Timing Event Bars",
        "Timing Event Jump 100", "Timing Event Pulse Color", "Timing Event Jump"
    });
    if (_timingTrackChoice) _timingTrackChoice->Enable(timingEventType);
    if (_regexCheck) _regexCheck->Enable(timingEventType);
    if (_filterText) _filterText->Enable(timingEventType);

    setRowEnabled(_sensSlider, _sensText, nullptr, IsType(type, {
        "Level Pulse", "Level Jump", "Level Jump 100", "Level Pulse Color", "Level Shape",
        "Level Bar", "Level Random Bar", "Level Color", "Note Level Bar",
        "Note Level Random Bar", "Spectrogram Peak", "Spectrogram Line",
        "Spectrogram Circle Line", "Note Level Jump", "Note Level Jump 100",
        "Note Level Pulse", "Timing Event Color", "Dominant Frequency Colour",
        "Dominant Frequency Colour Gradient"
    }));

    if (_slowDownFalls) {
        _slowDownFalls->Enable(IsType(type, {
            "Level Shape", "Spectrogram", "Spectrogram Line", "Spectrogram Circle Line",
            "Spectrogram Peak"
        }));
    }

    // Bars / Shape / SVG file: a compound rule that depends on both the
    // Type choice and (when Type is "Level Shape") the Shape choice. The
    // legacy ValidateWindow expressed this same logic.
    if (type == "Level Shape") {
        if (_shapeChoice) _shapeChoice->Enable(true);
        wxString shape = _shapeChoice ? _shapeChoice->GetStringSelection() : wxString();
        bool barsAsPoints = (shape == "Star" || shape == "Filled Star" || shape == "Snowflake");
        if (_barsSlider) _barsSlider->Enable(barsAsPoints);
        if (_barsText) _barsText->Enable(barsAsPoints);
        bool svgEnabled = (shape == "SVG");
        if (_hiddenSvgPicker) _hiddenSvgPicker->Enable(svgEnabled);
        if (_svgMediaPicker) _svgMediaPicker->Enable(svgEnabled);
    } else {
        if (_shapeChoice) _shapeChoice->Enable(false);
        if (_hiddenSvgPicker) _hiddenSvgPicker->Enable(false);
        if (_svgMediaPicker) _svgMediaPicker->Enable(false);
        // Bars enabled for everything except a small set of "single value" types
        bool barsEnabled = !IsType(type, {
            "On", "Color On", "Timing Event Color", "Level Color", "Note On"
        });
        if (_barsSlider) _barsSlider->Enable(barsEnabled);
        if (_barsText) _barsText->Enable(barsEnabled);
    }

    bool noteRange = IsType(type, {
        "Spectrogram", "Spectrogram Line", "Spectrogram Circle Line", "Spectrogram Peak",
        "Note On", "Note Level Bar", "Note Level Random Bar", "Note Level Jump",
        "Note Level Jump 100", "Note Level Pulse", "Dominant Frequency Colour",
        "Dominant Frequency Colour Gradient"
    });
    setRowEnabled(_startNoteSlider, _startNoteText, nullptr, noteRange);
    setRowEnabled(_endNoteSlider, _endNoteText, nullptr, noteRange);

    bool xOff = IsType(type, {
        "Spectrogram", "Spectrogram Line", "Spectrogram Circle Line", "Spectrogram Peak",
        "Level Shape"
    });
    setRowEnabled(_xOffsetSlider, _xOffsetText, nullptr, xOff);

    bool yOff = IsType(type, {
        "Level Shape", "Spectrogram Circle Line", "Frame Waveform", "Waveform"
    });
    setRowEnabled(_yOffsetSlider, _yOffsetText, _yOffsetVC, yOff);

    RefreshNoteTooltips();
}

void VUMeterPanel::SetPanelStatus(Model* /*cls*/) {
    // VUMeter wants ALL timing tracks (any layer count), unlike the framework's
    // "timingTracks" filter which only includes <= 1 layer tracks. Populate
    // manually rather than going through the framework's dynamicOptions.
    if (_timingTrackChoice == nullptr) return;

    wxString selection = _timingTrackChoice->GetStringSelection();
    _timingTrackChoice->Clear();
    if (mSequenceElements != nullptr) {
        for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++) {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                _timingTrackChoice->Append(e->GetName());
            }
        }
    }
    if (!selection.empty()) {
        _timingTrackChoice->SetStringSelection(selection);
    }
    if (_timingTrackChoice->GetSelection() == wxNOT_FOUND && _timingTrackChoice->GetCount() > 0) {
        _timingTrackChoice->SetSelection(0);
    }
    ValidateWindow();
}

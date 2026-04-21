/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PianoPanel.h"

#include <wx/spinctrl.h>
#include <wx/choice.h>

#include "UtilFunctions.h"

PianoPanel::PianoPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    // Cross-validate start/end MIDI on spin change
    auto* startSpin = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_StartMIDI", this));
    auto* endSpin = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_EndMIDI", this));

    if (startSpin) {
        startSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& e) {
            auto* ss = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_StartMIDI", this));
            auto* es = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_EndMIDI", this));
            if (ss && es && ss->GetValue() > es->GetValue()) {
                es->SetValue(ss->GetValue());
            }
            ValidateWindow();
            e.Skip();
        });
    }
    if (endSpin) {
        endSpin->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent& e) {
            auto* ss = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_StartMIDI", this));
            auto* es = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_EndMIDI", this));
            if (ss && es && es->GetValue() < ss->GetValue()) {
                ss->SetValue(es->GetValue());
            }
            ValidateWindow();
            e.Skip();
        });
    }
    ValidateWindow();
}

void PianoPanel::ValidateWindow() {
    // Enable/disable timing track based on whether tracks are available
    // (handled by base class EVT_SETTIMINGTRACKS handler)

    // Set MIDI note tooltips
    auto* startSpin = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_StartMIDI", this));
    auto* endSpin = dynamic_cast<wxSpinCtrl*>(wxWindow::FindWindowByName("ID_SPINCTRL_Piano_EndMIDI", this));
    if (startSpin) startSpin->SetToolTip(wxString(DecodeMidi(startSpin->GetValue())));
    if (endSpin) endSpin->SetToolTip(wxString(DecodeMidi(endSpin->GetValue())));
}

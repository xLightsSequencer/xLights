/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MusicPanel.h"
#include "EffectPanelUtils.h"

#include <wx/slider.h>
#include <wx/textctrl.h>

#include "UtilFunctions.h"

MusicPanel::MusicPanel(wxWindow* parent, const nlohmann::json& metadata)
    : JsonEffectPanel(parent, metadata) {
    // Cross-validate start/end notes on slider change
    auto* startSlider = dynamic_cast<wxSlider*>(wxWindow::FindWindowByName("ID_SLIDER_Music_StartNote", this));
    auto* endSlider = dynamic_cast<wxSlider*>(wxWindow::FindWindowByName("ID_SLIDER_Music_EndNote", this));

    auto syncNotes = [this](wxCommandEvent& e) {
        auto* ss = dynamic_cast<wxSlider*>(wxWindow::FindWindowByName("ID_SLIDER_Music_StartNote", this));
        auto* es = dynamic_cast<wxSlider*>(wxWindow::FindWindowByName("ID_SLIDER_Music_EndNote", this));
        auto* st = dynamic_cast<wxTextCtrl*>(wxWindow::FindWindowByName("IDD_TEXTCTRL_Music_StartNote", this));
        auto* et = dynamic_cast<wxTextCtrl*>(wxWindow::FindWindowByName("IDD_TEXTCTRL_Music_EndNote", this));
        if (!ss || !es) { e.Skip(); return; }

        int start = ss->GetValue();
        int end = es->GetValue();
        if (e.GetEventObject() == ss && end < start) {
            es->SetValue(start);
            if (et) et->SetValue(wxString::Format("%d", start));
        } else if (e.GetEventObject() == es && end < start) {
            ss->SetValue(end);
            if (st) st->SetValue(wxString::Format("%d", end));
        }
        ValidateWindow();
        e.Skip();
    };

    if (startSlider) startSlider->Bind(wxEVT_SLIDER, syncNotes);
    if (endSlider) endSlider->Bind(wxEVT_SLIDER, syncNotes);
    ValidateWindow();
}

void MusicPanel::ValidateWindow() {
    auto* startSlider = dynamic_cast<wxSlider*>(wxWindow::FindWindowByName("ID_SLIDER_Music_StartNote", this));
    auto* endSlider = dynamic_cast<wxSlider*>(wxWindow::FindWindowByName("ID_SLIDER_Music_EndNote", this));
    if (startSlider) startSlider->SetToolTip(wxString(DecodeMidi(startSlider->GetValue())));
    if (endSlider) endSlider->SetToolTip(wxString(DecodeMidi(endSlider->GetValue())));
}

#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/dialog.h>
#include <wx/bmpbuttn.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/spinctrl.h>
#include <vector>

#include "Color.h"

class xlColorCanvas;

// KiCad-inspired single-page color picker dialog.
// HSV disc + Value slider + spin controls + common/recent swatches + preview bar.
class xlColourPickerDialog : public wxDialog
{
public:
    xlColourPickerDialog(wxWindow* parent, const wxColour& initial,
                         wxWindowID id = wxID_ANY,
                         const wxString& title = "Color Picker");

    wxColour GetColour() const;

private:
    xlColorCanvas* m_disc        = nullptr;   // TYPE_WHEEL — full HSV disc
    xlColorCanvas* m_valueSlider = nullptr;   // TYPE_SLIDER MODE_BRIGHTNESS

    wxSpinCtrl* m_hueSpin = nullptr;   // 0-360
    wxSpinCtrl* m_satSpin = nullptr;   // 0-255
    wxSpinCtrl* m_valSpin = nullptr;   // 0-255
    wxSpinCtrl* m_redSpin = nullptr;
    wxSpinCtrl* m_grnSpin = nullptr;
    wxSpinCtrl* m_bluSpin = nullptr;

    // ---- preview / bottom bar ----
    wxPanel*    m_beforePanel = nullptr;
    wxPanel*    m_afterPanel  = nullptr;
    wxTextCtrl* m_hexField    = nullptr;

    // ---- swatches (plain wxPanel — no button chrome, no gaps) ----
    std::vector<wxWindow*> m_commonSwatches;
    std::vector<wxWindow*> m_recentSwatches;

    // ---- state ----
    xlColor  m_color;
    wxColour m_initialColor;
    bool     m_updating = false;

    // ---- helpers ----
    void SyncAllFromColor(bool syncDisc = true);
    void UpdateAfterPanel();
    void PushRecentColor(const xlColor& c);
    void LoadRecentSwatches();
    void SetColorAndSync(const xlColor& c);

    // ---- event handlers ----
    void OnDiscChanged(wxCommandEvent& event);
    void OnSliderChanged(wxCommandEvent& event);
    void OnHSVSpin(wxSpinEvent& event);
    void OnRGBSpin(wxSpinEvent& event);
    void OnHexText(wxCommandEvent& event);
    void OnRandomize(wxCommandEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnSystemPicker(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

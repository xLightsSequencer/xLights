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

#include <wx/panel.h>
#include <wx/bmpbuttn.h>
#include <wx/clrpicker.h>

// A colored-swatch button that opens xlColourPickerDialog (or the OS dialog when the
// custom-picker feature flag is off). Fires wxEVT_COLOURPICKER_CHANGED on OK so existing
// event-table handlers in effect panels work without change.
class xlColourPickerButton : public wxPanel
{
public:
    xlColourPickerButton(wxWindow* parent,
                         wxWindowID id,
                         const wxColour& initial = *wxBLACK,
                         const wxPoint& pos = wxDefaultPosition,
                         const wxSize& size = wxDefaultSize,
                         long style = 0,
                         const wxValidator& validator = wxDefaultValidator,
                         const wxString& name = wxPanelNameStr);

    wxColour GetColour() const { return m_colour; }
    void SetColour(const wxColour& c);

    // Compatibility shim: BulkEditColourPickerCtrl expects GetPickerCtrl() to return
    // the inner clickable widget so it can attach its own right-click handler.
    wxControl* GetPickerCtrl() { return m_button; }

private:
    wxBitmapButton* m_button = nullptr;
    wxColour m_colour;

    static wxBitmap MakeColorBitmap(const wxColour& c, const wxSize& sz);
    void OnClick(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

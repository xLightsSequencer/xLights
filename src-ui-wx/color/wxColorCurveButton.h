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

#include <string>

#include <wx/bmpbuttn.h>
#include <wx/event.h>
#include <wx/wx.h>

#include "render/ColorCurve.h"

wxDECLARE_EVENT(EVT_CC_CHANGED, wxCommandEvent);

class ColorCurveButton : public wxBitmapButton
{
    ColorCurve* _cc;
    std::string _color;
    void LeftClick(wxCommandEvent& event);
    void RightClick(wxContextMenuEvent& event);

public:
    ColorCurveButton(wxWindow* parent,
                     wxWindowID id,
                     const wxBitmap& bitmap,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxBU_AUTODRAW,
                     const wxValidator& validator = wxDefaultValidator,
                     const wxString& name = wxButtonNameStr);
    ~ColorCurveButton();
    virtual void SetValue(const wxString& value);
    ColorCurve* GetValue() const;
    void ToggleActive();
    void SetActive(bool active, bool notify = true);
    void UpdateState(bool notify = true);
    void UpdateBitmap();
    std::string GetColor() const { return _color; }
    void SetColor(std::string color, bool notify = true);
    void SetDefaultCC(const std::string& color);
    void NotifyChange(bool coloursPanelReload = false);
};

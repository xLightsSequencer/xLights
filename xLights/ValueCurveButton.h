#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/bmpbuttn.h>
#include "ValueCurve.h"

wxDECLARE_EVENT(EVT_VC_CHANGED, wxCommandEvent);

class ValueCurveButton :
    public wxBitmapButton
{
    ValueCurve* _vc;
    void RenderNewBitmap();

public:
    ValueCurveButton(wxWindow *parent,
        wxWindowID id,
        const wxBitmap& bitmap,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxBU_AUTODRAW,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxButtonNameStr);
    ~ValueCurveButton();
    virtual void SetValue(const wxString& value);
    void SetLimits(float min, float max);
    ValueCurve* GetValue();
    void ToggleActive();
    void SetActive(bool active);
    void UpdateState();
    void UpdateBitmap();
    void NotifyChange();

    static wxBitmap disabledBitmap;
};

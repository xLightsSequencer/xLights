#ifndef VALUECURVEBUTTON_H
#define VALUECURVEBUTTON_H

#include <wx/bmpbuttn.h>
#include "ValueCurve.h"
#include "../include/valuecurvenotselected.xpm"

wxDECLARE_EVENT(EVT_VC_CHANGED, wxCommandEvent);

class ValueCurveButton :
    public wxBitmapButton
{
    ValueCurve* _vc;
    void NotifyChange();
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

    
    static wxBitmap disabledBitmap;
};

#endif

#ifndef VALUECURVEBUTTON_H
#define VALUECURVEBUTTON_H

#include <wx/bmpbuttn.h>
#include "ValueCurve.h"
#include "../include/valuecurvenotselected.xpm"

wxDECLARE_EVENT(EVT_VC_CHANGED, wxCommandEvent);

class xLightsFrame;

class ValueCurveButton :
    public wxBitmapButton
{
    ValueCurve* _vc;
    xLightsFrame* _frame;
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
    xLightsFrame* GetFrame() const { wxASSERT(_frame != nullptr); return _frame; }
    void SetFrame(xLightsFrame* frame) { _frame = frame; }

    static wxBitmap disabledBitmap;
};

#endif

#include "ValueCurveButton.h"
#include "../include/valuecurveselected.xpm"
#include "../include/valuecurvenotselected.xpm"

wxDEFINE_EVENT(EVT_VC_CHANGED, wxCommandEvent);

ValueCurveButton::ValueCurveButton(wxWindow *parent,
    wxWindowID id,
    const wxBitmap& bitmap,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxValidator& validator,
    const wxString& name) : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    _vc = new ValueCurve(name.ToStdString());
}


ValueCurveButton::~ValueCurveButton()
{
    if (_vc != NULL)
    {
        delete _vc;
    }
}

void ValueCurveButton::SetActive(bool active)
{
    _vc->SetActive(active);
    UpdateState();
}

void ValueCurveButton::ToggleActive()
{
    _vc->ToggleActive();
    UpdateState();
}

void ValueCurveButton::UpdateState()
{
    if (GetValue()->IsActive())
    {
        wxBitmap bmp(valuecurveselected_24);
        SetBitmap(bmp);
    }
    else
    {
        wxBitmap bmp(valuecurvenotselected_24);
        SetBitmap(bmp);
    }
}

void ValueCurveButton::SetValue(const wxString& value)
{
    _vc->Deserialise(value.ToStdString());
    UpdateState();
    NotifyChange();
}

void ValueCurveButton::NotifyChange()
{
    wxCommandEvent eventVCChange(EVT_VC_CHANGED);
    eventVCChange.SetEventObject(this);
    wxPostEvent(GetParent(), eventVCChange);
}

void ValueCurveButton::SetLimits(float min, float max)
{
    _vc->SetLimits(min, max);
    UpdateState();
}

ValueCurve* ValueCurveButton::GetValue()
{
    return _vc;
}

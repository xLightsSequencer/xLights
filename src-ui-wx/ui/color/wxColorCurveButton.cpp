/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wxColorCurveButton.h"

#include <wx/dcmemory.h>

#include "ui/color/ColorCurveDialog.h"
#include "ui/color/wxColorCurveRenderer.h"
#include "ui/shared/utils/wxUtilities.h"
#include "ui/color/xlColourData.h"

wxDEFINE_EVENT(EVT_CC_CHANGED, wxCommandEvent);

ColorCurveButton::ColorCurveButton(wxWindow* parent,
                                   wxWindowID id,
                                   const wxBitmap& bitmap,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style,
                                   const wxValidator& validator,
                                   const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    _color = "#FFFFFF";
    _cc = new ColorCurve(name.ToStdString(), "Gradient", _color);
    Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ColorCurveButton::LeftClick);
    Connect(id, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&ColorCurveButton::RightClick);
}

void ColorCurveButton::LeftClick(wxCommandEvent& event)
{
    ColorCurveButton* w = static_cast<ColorCurveButton*>(event.GetEventObject());
    wxColour color = w->GetBackgroundColour();
    auto const& [res, ncolor] = xlColourData::INSTANCE.ShowColorDialog(this, color);
    if (res == wxID_OK) {
        _cc->SetActive(false);
        color = ncolor;
        _color = color.GetAsString();
        _cc->SetDefault(wxColourToXlColor(color));
        UpdateBitmap();
        NotifyChange(true);
    }
}

void ColorCurveButton::RightClick(wxContextMenuEvent& event)
{
    ColorCurveButton* w = static_cast<ColorCurveButton*>(event.GetEventObject());

    ColorCurveDialog ccd(this, w->GetValue());
    if (ccd.ShowModal() == wxID_OK) {
        w->SetActive(true);
        UpdateBitmap();
        NotifyChange(true);
    } else {
        if (ccd.DidExport()) {
            NotifyChange(true);
        }
    }
}

ColorCurveButton::~ColorCurveButton()
{
    if (_cc != nullptr) {
        delete _cc;
    }
}

void ColorCurveButton::SetActive(bool active, bool notify)
{
    _cc->SetActive(active);
    UpdateState(notify);
}

void ColorCurveButton::ToggleActive()
{
    _cc->ToggleActive();
    UpdateState();
}

void ColorCurveButton::SetDefaultCC(const std::string& color)
{
    _cc->SetDefault(xlColor(color));
}

void ColorCurveButton::SetColor(std::string color, bool notify)
{
    _cc->SetActive(false);
    _cc->SetDefault(xlColor(color));
    _color = color;
    UpdateBitmap();
    if (notify) {
        NotifyChange();
    }
}

void ColorCurveButton::UpdateBitmap()
{
    wxSize sz = GetSize();
    if (GetValue()->IsActive()) {
        SetBitmap(wxColorCurveRenderer::GetColorCurveImage(*_cc, sz.GetWidth(), sz.GetHeight(), false));
        UnsetToolTip();
    } else {
        wxColor color(_color);
        SetBackgroundColour(color);
        SetForegroundColour(color);
        wxImage image(sz.GetWidth(), sz.GetHeight());
        if (color.IsOk()) {
            image.SetRGB(wxRect(0, 0, sz.GetWidth(), sz.GetHeight()),
                         color.Red(), color.Green(), color.Blue());
        }
        wxBitmap bmp(image);
        SetBitmap(bmp);
        SetToolTip(wxString::Format("%s\n%d,%d,%d\n%s", _color, color.Red(), color.Green(), color.Blue(), GetColourName(wxColourToXlColor(color))));
    }
    Refresh();
}

void ColorCurveButton::UpdateState(bool notify)
{
    UpdateBitmap();
    if (notify) {
        NotifyChange();
    }
}

void ColorCurveButton::SetValue(const wxString& value)
{
    _cc->Deserialise(value.ToStdString());
    _cc->SetId(GetName().ToStdString());
    UpdateState();
}

void ColorCurveButton::NotifyChange(bool coloursPanelReload)
{
    wxCommandEvent eventCCChange(EVT_CC_CHANGED);
    eventCCChange.SetInt(coloursPanelReload);
    eventCCChange.SetEventObject(this);
    wxPostEvent(GetParent(), eventCCChange);
}

ColorCurve* ColorCurveButton::GetValue() const
{
    return _cc;
}

#include "MHRgbPickerPanel.h"
#include "Color.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

namespace
{
    const int BorderWidth = 5;
    const float box_size = 0.02f;
    const float snap_zone = 0.015f;
    const float gap_degrees = 45.0f;
    const float line_gap = gap_degrees / 360.0f;
    const float num_lines = 1.0f / line_gap;
}

BEGIN_EVENT_TABLE(MHRgbPickerPanel, wxPanel)
    EVT_PAINT(MHRgbPickerPanel::OnPaint)
    EVT_LEFT_DOWN(MHRgbPickerPanel::OnLeftDown)
    EVT_LEFT_UP(MHRgbPickerPanel::OnLeftUp)
    EVT_MOTION(MHRgbPickerPanel::OnMouseMove)
    EVT_ENTER_WINDOW(MHRgbPickerPanel::OnEntered)
END_EVENT_TABLE()

MHRgbPickerPanel::MHRgbPickerPanel(IMHRgbPickerPanelParent* rgbPickerParent, wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS),
    m_rgbPickerParent(rgbPickerParent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void MHRgbPickerPanel::OnPaint(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC pdc(this);
    wxSize sz(GetSize());

    xlColor c;
    HSVValue v;
    v.value = 1.0f;
    float center = (sz.y - 4.0f) / 2.0f;
    float radius = center - 2.0f;
    for( int y = 0; y < sz.y; ++y ) {
        for( int x = 0; x < sz.x; ++x ) {
            float xleg = (float)x - center;
            float yleg = (float)y - center;
            float hyp = sqrt(xleg * xleg + yleg * yleg);
            if( hyp <= center ) {  // inside circle
                float phi = atan2(xleg, yleg) * 180.0f / PI + 630.0f;
                phi = fmod(phi, 360.0f);
                v.saturation = hyp / radius;
                v.hue = phi / 360.0f;
                c.fromHSV(v);
                wxColor cc(c);
                pdc.SetPen(wxPen(cc));
                pdc.DrawPoint(x,y);
            }
        }
    }
}

void MHRgbPickerPanel::SetPosition(wxPoint2DDouble pos)
{
    m_mousePos = pos;
    Refresh();
}

void MHRgbPickerPanel::OnLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    m_rgbPickerParent->NotifyColorUpdated();
    Refresh();
    m_mouseDown = true;
}

void MHRgbPickerPanel::OnLeftUp(wxMouseEvent& event)
{
    m_mouseDown = false;
}

void MHRgbPickerPanel::OnMouseMove(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    if( m_mouseDown ) {
        m_mousePos = UItoNormalized(ptUI);
        m_rgbPickerParent->NotifyColorUpdated();
        Refresh();
    }
}

void MHRgbPickerPanel::OnEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble MHRgbPickerPanel::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble MHRgbPickerPanel::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y));
}

wxPoint MHRgbPickerPanel::NormalizedToUI2(const wxPoint2DDouble& pt) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}

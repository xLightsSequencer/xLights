#include "MHRgbPickerPanel.h"
#include "Color.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/rawbmp.h>

namespace
{
    const int BorderWidth = 5;
    const float box_size = 0.02f;
    const float snap_zone = 0.015f;
    const float gap_degrees = 45.0f;
    const float line_gap = gap_degrees / 360.0f;
    const float num_lines = 1.0f / line_gap;
    const int handleRadius = 10;
}

BEGIN_EVENT_TABLE(MHRgbPickerPanel, wxPanel)
    EVT_PAINT(MHRgbPickerPanel::OnPaint)
    EVT_KEY_DOWN(MHRgbPickerPanel::OnKeyDown)
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
    CreateHsvBitmap(wxSize(256, 256));
    CreateHsvBitmapMask();
}

MHRgbPickerPanel::~MHRgbPickerPanel()
{
    if( m_hsvBitmap != nullptr ) {
        delete m_hsvBitmap;
    }
}

void MHRgbPickerPanel::OnPaint(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC pdc(this);

    if ( !m_hsvBitmap->IsOk() )
     return;

    wxSize dcSize = pdc.GetSize();

    if ( m_hsvBitmap->GetSize() != dcSize ) {
        CreateHsvBitmap(dcSize);
        CreateHsvBitmapMask();
    }

    pdc.DrawBitmap(*m_hsvBitmap, 0, 0, true);

    // draw color handles
    for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
        wxPoint ptUI {NormalizedToUI2((*it).pt)};
        xlColor c {(*it).color};
        wxBrush b {wxColour(c)};
        pdc.SetBrush(b);
        pdc.SetPen(*wxBLACK_PEN);
        pdc.DrawCircle(ptUI, handleRadius-1);
        pdc.DrawCircle(ptUI, handleRadius);
        pdc.DrawCircle(ptUI, handleRadius+1);
    }
}

void MHRgbPickerPanel::OnKeyDown(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE) {
        if( active_handle >= 0 ) {
            m_handles.erase(m_handles.begin()+active_handle);
            selected_point = -1;
            active_handle = -1;
            m_rgbPickerParent->NotifyColorUpdated();
            Refresh();
        }
    }
}
void MHRgbPickerPanel::OnLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    m_mouseDown = true;
    if( m_handles.size() == 0 ) {
        if( insideColors(ptUI.m_x, ptUI.m_y) ) {
            wxColour color{GetPointColor(ptUI.m_x, ptUI.m_y)};
            m_handles.push_back(HandlePoint(m_mousePos, color));
            selected_point = 0;
            active_handle = 0;
        }
    } else if( insideColors(ptUI.m_x, ptUI.m_y) ) {
        m_handles[active_handle].pt.m_x = m_mousePos.m_x;
        m_handles[active_handle].pt.m_y = m_mousePos.m_y;
        xlColor c {GetPointColor(ptUI.m_x, ptUI.m_y)};
        m_handles[active_handle].color = wxColour(c);
        selected_point = active_handle;
    }
    m_rgbPickerParent->NotifyColorUpdated();
    Refresh();
}

void MHRgbPickerPanel::OnLeftUp(wxMouseEvent& event)
{
    m_mouseDown = false;
    selected_point = -1;
}

void MHRgbPickerPanel::OnMouseMove(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    if( m_mouseDown ) {
        m_mousePos = UItoNormalized(ptUI);
        if( selected_point != -1 ) {
            if( insideColors(ptUI.m_x, ptUI.m_y) ) {
                m_handles[selected_point].pt.m_x = m_mousePos.m_x;
                m_handles[selected_point].pt.m_y = m_mousePos.m_y;
                xlColor c {GetPointColor(ptUI.m_x, ptUI.m_y)};
                m_handles[selected_point].color = wxColour(c);
            }
        }
        m_rgbPickerParent->NotifyColorUpdated();
        Refresh();
    }
}

void MHRgbPickerPanel::OnEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

int MHRgbPickerPanel::HitTest( wxPoint2DDouble& ptUI )
{
    for (int i = 0; i < m_handles.size(); ++i ) {
        wxPoint2DDouble pt{NormalizedToUI(m_handles[i].pt)};
        if( (ptUI.m_x > pt.m_x - handleRadius) &&
        (ptUI.m_x < pt.m_x + handleRadius) &&
        (ptUI.m_y > pt.m_y - handleRadius) &&
        (ptUI.m_y < pt.m_y + handleRadius) ) {
            return i;
        }
    }
    return -1;
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

void MHRgbPickerPanel::CreateHsvBitmap(const wxSize& newSize)
{
    center = (float)newSize.GetWidth() / 2.0f;
    radius = center - 10.0f;

    if( m_hsvBitmap != nullptr ) {
        delete m_hsvBitmap;
    }

    m_hsvBitmap = new wxBitmap( newSize.GetWidth(), newSize.GetHeight() );

    wxNativePixelData data(*m_hsvBitmap);
    if ( !data )
    {
        // ... raw access to bitmap data unavailable
        return;
    }
    wxNativePixelData::Iterator p(data);
    for ( int y = 0; y < newSize.GetHeight(); ++y )
    {
        wxNativePixelData::Iterator rowStart = p;
        for ( int x = 0; x < newSize.GetWidth(); ++x, ++p )
        {
            xlColor c {GetPointColor(x, y)};
            p.Red() = c.red;
            p.Green() = c.green;
            p.Blue() = c.blue;
        }
        p = rowStart;
        p.OffsetY(data, 1);
    }
}

bool MHRgbPickerPanel::insideColors(int x, int y)
{
    float xleg = (float)x - center;
    float yleg = (float)y - center;
    float hyp = sqrt(xleg * xleg + yleg * yleg);
    if( hyp <= radius ) {  // inside circle
        return true;
    } else {
        return false;
    }
}

xlColor MHRgbPickerPanel::GetPointColor(int x, int y)
{
    HSVValue v;
    v.value = 1.0f;
    xlColor c;
    float xleg = (float)x - center;
    float yleg = (float)y - center;
    float hyp = sqrt(xleg * xleg + yleg * yleg);
    if( hyp <= radius ) {  // inside circle
        float phi = atan2(xleg, yleg) * 180.0f / PI + 630.0f;
        phi = fmod(phi, 360.0f);
        v.saturation = hyp / center;
        v.hue = phi / 360.0f;
        c.fromHSV(v);
    } else {
        c.red = 0;
        c.green = 0;
        c.blue = 0;
    }
    return c;
}

void MHRgbPickerPanel::CreateHsvBitmapMask()
{
    wxColour color(0,0,0);
    m_hsvMask = new wxMask(*m_hsvBitmap, color);
    m_hsvBitmap->SetMask(m_hsvMask);
}

wxColour MHRgbPickerPanel::GetColour()
{
    if( active_handle >= 0 ) {
        return m_handles[active_handle].color;
    } else {
        return wxColour(0,0,0);
    }
}

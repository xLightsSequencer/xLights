/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MHRgbPickerPanel.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/rawbmp.h>

#include "../../UtilFunctions.h"
namespace
{
    const int BorderWidth = 5;
    const int handleRadius = 10;
    const double v_size = 20;
}

BEGIN_EVENT_TABLE(MHRgbPickerPanel, wxPanel)
    EVT_PAINT(MHRgbPickerPanel::OnPaint)
    EVT_KEY_DOWN(MHRgbPickerPanel::OnKeyDown)
    EVT_KEY_UP(MHRgbPickerPanel::OnKeyUp)
    EVT_LEFT_DOWN(MHRgbPickerPanel::OnLeftDown)
    EVT_LEFT_UP(MHRgbPickerPanel::OnLeftUp)
    EVT_MOTION(MHRgbPickerPanel::OnMouseMove)
    EVT_ENTER_WINDOW(MHRgbPickerPanel::OnEntered)
    EVT_LEAVE_WINDOW(MHRgbPickerPanel::OnLeave)
    EVT_SIZE(MHRgbPickerPanel::OnSize)
END_EVENT_TABLE()


MHRgbPickerPanel::MHRgbPickerPanel(IMHRgbPickerPanelParent* rgbPickerParent, wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    MHColorPanel(parent, id, pos, size),
    m_rgbPickerParent(rgbPickerParent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    CreateHsvBitmap(wxSize(256, 256));
    CreateHsvBitmapMask();
}

MHRgbPickerPanel::~MHRgbPickerPanel()
{
    if (m_hsvBitmap != nullptr) {
        delete m_hsvBitmap;
    }
}

void MHRgbPickerPanel::OnSize(wxSizeEvent& event){
    wxSize old_sz = GetSize();
    if( old_sz.GetWidth() != old_sz.GetHeight() ) {
        if (old_sz.GetWidth() > 270) {
            wxSize new_size = old_sz;
            new_size.SetHeight(new_size.GetWidth() + 30);
            SetMinSize(new_size);
            SetSize(new_size);
        }
    }
    Refresh();
    //skip the event.
    event.Skip();
}

void MHRgbPickerPanel::OnPaint(wxPaintEvent& /*event*/) {
    wxAutoBufferedPaintDC pdc(this);

    if (!m_hsvBitmap->IsOk()) {
        return;
    }

    wxSize dcSize = pdc.GetSize();
    
// Windows leaves artifacts in the clear area of the mask without this clear but
// with the clear OSX doesn't look as nice because it paints a background
#ifdef __WXMSW__
    pdc.Clear();
#endif

    if ( m_hsvBitmap->GetSize() != dcSize ) {
        CreateHsvBitmap(dcSize);
        CreateHsvBitmapMask();
    }

    pdc.DrawBitmap(*m_hsvBitmap, 0, 0, true);

    // draw color handles
    int handle = 1;
    for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
        wxPoint ptUI {NormalizedToUI2((*it).pt)};
        xlColor c {(*it).color};
        wxBrush b {wxColour(c)};
        pdc.SetBrush(b);
        pdc.SetPen(*wxBLACK_PEN);
        pdc.DrawCircle(ptUI, handleRadius-1);
        pdc.DrawCircle(ptUI, handleRadius);
        pdc.DrawCircle(ptUI, handleRadius+1);
        if (c.asHSV().value > 0.5) {
            pdc.SetTextForeground(wxColour(xlBLACK));
        } else {
            pdc.SetTextForeground(wxColour(xlWHITE));
        }
        wxString text = wxString::Format("%d", handle);
        pdc.DrawText(text, ptUI.x-4, ptUI.y-8);
        handle++;
    }
    // draw color value bar
    if( active_handle >= 0 ) {
        wxRect rect(v_left, v_top, v_width, v_height);
        HSVValue hsv {m_handles[active_handle].color};
        double value {hsv.value};
        hsv.value = 1.0f;
        xlColor color{hsv};
        wxColour dest_color {(wxColour)color};
        hsv.value = 0.0f;
        color.fromHSV(hsv);
        wxColour init_color {(wxColour)color};
        double setpoint {v_width * value + v_left};
        pdc.GradientFillLinear(rect, init_color, dest_color);

        // Draw triangle setpoint
        wxPoint points[] = { wxPoint(setpoint,v_top+10), wxPoint(setpoint-10,v_top+20), wxPoint(setpoint+10,v_top+20) };
        int num_points {3};
        if( value > 0.4 ) {
            pdc.SetBrush(*wxBLACK_BRUSH);
        } else {
            pdc.SetBrush(*wxWHITE_BRUSH);
        }
        pdc.DrawPolygon(num_points, points, wxODDEVEN_RULE);
        pdc.SetTextForeground(wxColour(xlBLACK));
        wxString text = wxString::Format("H:%i,S:%i,V:%i",
                        int(hsv.hue * 360.0), int(hsv.saturation * 100.0), int(value * 100.0));
        pdc.DrawText(text, 0 , 0);
    }
}

void MHRgbPickerPanel::OnKeyDown(wxKeyEvent& event) {
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE) {
        if (active_handle >= 0) {
            m_handles.erase(m_handles.begin() + active_handle);
            if (!m_handles.empty()) {
                selected_point = std::max(active_handle - 1, 0);
                active_handle = std::max(active_handle - 1, 0);
            } else {
                selected_point = -1;
                active_handle = -1;
            }
            m_rgbPickerParent->NotifyColorUpdated();
            Refresh();
        }
    } else if (keycode == WXK_SHIFT) {
        m_shiftdown = true;
    }
}

void MHRgbPickerPanel::OnKeyUp(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_SHIFT) {
        m_shiftdown = false;
    }
}

void MHRgbPickerPanel::OnLeftDown(wxMouseEvent& event) {
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    m_mouseDown = true;
    if (m_handles.empty()) {
        if (insideColors(ptUI.m_x, ptUI.m_y)) {
            xlColor color{ GetPointColor(ptUI.m_x, ptUI.m_y) };
            m_handles.push_back(HandlePoint(m_mousePos, color));
            selected_point = 0;
            active_handle = 0;
            m_rgbPickerParent->NotifyColorUpdated();
            Refresh();
        }
    } else if (insideColors(ptUI.m_x, ptUI.m_y)) {
        selected_point = HitTest(ptUI);
        if (selected_point >= 0 && !m_shiftdown) {
            active_handle = selected_point;
            if (active_handle >= 0) {
                m_handles.erase(m_handles.begin() + active_handle);
                if (m_handles.size() > 0) {
                    selected_point = std::max(active_handle - 1, 0);
                    active_handle = std::max(active_handle - 1, 0);
                } else {
                    selected_point = -1;
                    active_handle = -1;
                }
                m_rgbPickerParent->NotifyColorUpdated();
            }
            Refresh();
        } else {
            if (m_handles.size() < 8 && !m_shiftdown) {
                active_handle = m_handles.size();
                xlColor color{ GetPointColor(ptUI.m_x, ptUI.m_y) };
                m_handles.push_back(HandlePoint(m_mousePos, color));
                selected_point = active_handle;
            } else {
                if (selected_point >= 0) {
                    active_handle = selected_point;
                    m_handles[active_handle].pt.m_x = m_mousePos.m_x;
                    m_handles[active_handle].pt.m_y = m_mousePos.m_y;
                    xlColor c{ GetPointColor(ptUI.m_x, ptUI.m_y) };
                    m_handles[active_handle].color = wxColour(c);
                }
            }
            m_rgbPickerParent->NotifyColorUpdated();
            Refresh();
        }
    } else {
        if (HitTestV(ptUI)) {
            m_mouseDown = false;
            m_mouseDownV = true;
        }
    }
}

void MHRgbPickerPanel::OnLeftUp(wxMouseEvent& event)
{
    m_mouseDown = false;
    m_mouseDownV = false;
    m_mouseDClick = false;
    selected_point = -1;
}

void MHRgbPickerPanel::OnMouseMove(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    if( m_mouseDown ) {
        if( selected_point != -1 ) {
            if( insideColors(ptUI.m_x, ptUI.m_y) ) {
                m_mousePos = UItoNormalized(ptUI);
                m_handles[selected_point].pt.m_x = m_mousePos.m_x;
                m_handles[selected_point].pt.m_y = m_mousePos.m_y;
                xlColor c {GetPointColor(ptUI.m_x, ptUI.m_y)};
                m_handles[selected_point].color = wxColour(c);
                m_rgbPickerParent->NotifyColorUpdated();
                Refresh();
            }
        }
    } else if( m_mouseDownV ) {
        HitTestV( ptUI );
    }
}

void MHRgbPickerPanel::OnEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

void MHRgbPickerPanel::OnLeave(wxMouseEvent& /*event*/) {
    m_mouseDown = false;
    m_mouseDownV = false;
    m_mouseDClick = false;
    selected_point = -1;
}

bool MHRgbPickerPanel::HitTestV( wxPoint2DDouble& ptUI )
{
    if( active_handle >= 0 ) {
        if( ptUI.m_x >= v_left && ptUI.m_x <= v_left + v_width &&
           ptUI.m_y >= v_top && ptUI.m_y <= v_top + v_height ) {
            HSVValue hsv {m_handles[active_handle].color};
            hsv.value = {(ptUI.m_x - v_left) / v_width};;
            m_handles[active_handle].color = hsv;
            m_rgbPickerParent->NotifyColorUpdated();
            Refresh();
            return true;
        }
    }
    return false;
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

    v_left = center - radius;
    v_top = radius * 2.0 + v_size;
    v_width = radius * 2.0;

    if( m_hsvBitmap != nullptr ) {
        delete m_hsvBitmap;
        m_hsvBitmap = nullptr;
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
    wxColour color(0, 0, 0);
    m_hsvMask = new wxMask(*m_hsvBitmap, color);
    m_hsvBitmap->SetMask(m_hsvMask);
}

std::string MHRgbPickerPanel::GetColour()
{
    if( m_handles.size() >= 0 ) {
        std::string text{"Color: "};
        bool add_comma = false;
        for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
            wxString color = wxString::Format("%f,%f,%f", (*it).color.hue, (*it).color.saturation, (*it).color.value);
            if( add_comma ) {
                text += ",";
            }
            text += color;
            add_comma = true;
        }
        return text;
    } else {
        return "Color: 0,0,0";
    }
}

void MHRgbPickerPanel::SetColours( const std::string& _colors )
{
    m_handles.clear();
    selected_point = -1;
    active_handle = -1;
    wxArrayString colors = wxSplit(_colors, ',');
    unsigned long num_colors = colors.size() / 3;
    for( int i = 0; i < num_colors; ++i ) {
        double hue { wxAtof(colors[i*3]) };
        double sat { wxAtof(colors[i*3+1]) };
        double val { wxAtof(colors[i*3+2]) };
        HSVValue v(hue, sat, val);
        double hyp {v.saturation * center};
        double phi {v.hue * 360.0f * PI / 180.0f};
        float x = cos(phi) * hyp + center;
        float y = sin(phi) * hyp + center;
        wxPoint2DDouble pt((int)x, (int)y);
        wxPoint2DDouble pt2(UItoNormalized(pt));
        pt2.m_y = 1.0 - pt2.m_y;
        m_handles.push_back(HandlePoint(pt2, v));
    }
    if( num_colors > 0 ) {
        active_handle = 0;
    }
    Refresh();
}

void MHRgbPickerPanel::ResetColours() {
    for (std::size_t i = 0; i < m_handles.size(); ++i) {
        m_handles.erase(m_handles.begin() + i);
    }
    m_rgbPickerParent->NotifyColorUpdated();
    Refresh();
    selected_point = -1;
    active_handle = -1;
}

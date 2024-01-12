/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MHColorWheelPanel.h"
#include "Color.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/rawbmp.h>

namespace
{
    const int BorderWidth = 5;
    //const int handleRadius = 10;
}

BEGIN_EVENT_TABLE(MHColorWheelPanel, wxPanel)
    EVT_PAINT(MHColorWheelPanel::OnPaint)
    EVT_LEFT_DOWN(MHColorWheelPanel::OnLeftDown)
    EVT_LEFT_UP(MHColorWheelPanel::OnLeftUp)
    EVT_MOTION(MHColorWheelPanel::OnMouseMove)
    EVT_ENTER_WINDOW(MHColorWheelPanel::OnEntered)
    EVT_SIZE(MHColorWheelPanel::OnSize)
END_EVENT_TABLE()


MHColorWheelPanel::MHColorWheelPanel(IMHColorWheelPanelParent* colorWheelParent, wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    MHColorPanel(parent, id, pos, size),
    m_colorWheelParent(colorWheelParent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    CreateHsvBitmap(wxSize(256, 256));
    CreateHsvBitmapMask();
}

MHColorWheelPanel::~MHColorWheelPanel()
{
    if( m_hsvBitmap != nullptr ) {
        delete m_hsvBitmap;
    }
}

void MHColorWheelPanel::OnSize(wxSizeEvent& event){
    wxSize old_sz = GetSize();
    if( old_sz.GetWidth() != old_sz.GetHeight() ) {
        if( old_sz.GetWidth() > 270 ) {
            wxSize new_size = old_sz;
            new_size.SetHeight(new_size.GetWidth());
            SetMinSize(new_size);
        }
    }
    Refresh();
    //skip the event.
    event.Skip();
}

void MHColorWheelPanel::OnPaint(wxPaintEvent& /*event*/)
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
  
}

void MHColorWheelPanel::OnLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    m_mouseDown = true;
}

void MHColorWheelPanel::OnLeftUp(wxMouseEvent& event)
{
    m_mouseDown = false;
    m_mouseDClick = false;
}

void MHColorWheelPanel::OnMouseMove(wxMouseEvent& event)
{
    //wxAffineMatrix2D m;
    //wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    if( m_mouseDown ) {
    }
}

void MHColorWheelPanel::OnEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

int MHColorWheelPanel::HitTest( wxPoint2DDouble& ptUI )
{
    /*for (int i = 0; i < m_handles.size(); ++i ) {
        wxPoint2DDouble pt{NormalizedToUI(m_handles[i].pt)};
        if( (ptUI.m_x > pt.m_x - handleRadius) &&
        (ptUI.m_x < pt.m_x + handleRadius) &&
        (ptUI.m_y > pt.m_y - handleRadius) &&
        (ptUI.m_y < pt.m_y + handleRadius) ) {
            return i;
        }
    }*/
    return -1;
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble MHColorWheelPanel::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble MHColorWheelPanel::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y));
}

wxPoint MHColorWheelPanel::NormalizedToUI2(const wxPoint2DDouble& pt) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}

void MHColorWheelPanel::CreateHsvBitmap(const wxSize& newSize)
{
    center = (float)newSize.GetWidth() / 2.0f;
    radius = center - 10.0f;

    if( m_hsvBitmap != nullptr ) {
        delete m_hsvBitmap;
    }

    m_hsvBitmap = new wxBitmap( newSize.GetWidth(), newSize.GetHeight() );

    wxMemoryDC memDC;
    memDC.SelectObject(*m_hsvBitmap);
    
    int num_colors = colors.size();
    float pie_size = 360.0f / num_colors;
    float slot_radius = PI / num_colors * radius / ((PI/num_colors) + 1.0f);
    float hyp = radius - slot_radius;
    slot_radius *= 0.8f;
    float start_angle = 0.0f;
    memDC.SetPen(*wxBLACK_PEN);
    memDC.SetBrush(*wxLIGHT_GREY_BRUSH);
    memDC.DrawCircle(center, center, radius);
    for (auto const& col : colors) {
        float x = sin(glm::radians(start_angle)) * hyp + center;
        float y = cos(glm::radians(start_angle)) * hyp + center;
        memDC.SetBrush(wxColour(col.red,col.green,col.blue));
        memDC.DrawCircle(x, y, slot_radius);
        start_angle += pie_size;
    }

    memDC.SelectObject(wxNullBitmap);
    

    /*
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
    }*/
}

bool MHColorWheelPanel::insideColors(int x, int y)
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

xlColor MHColorWheelPanel::GetPointColor(int x, int y)
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

void MHColorWheelPanel::CreateHsvBitmapMask()
{
    wxColour color(0,0,0);
    m_hsvMask = new wxMask(*m_hsvBitmap, color);
    m_hsvBitmap->SetMask(m_hsvMask);
}

std::string MHColorWheelPanel::GetColour()
{
/*    if( m_handles.size() >= 0 ) {
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
    } else {*/
        return "Wheel: 0,0,0";
    //}
}

void MHColorWheelPanel::SetColours( const std::string& _colors )
{
    wxArrayString colors = wxSplit(_colors, ',');
    unsigned long num_colors {colors.size() / 3};
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
    }
    Refresh();
}

void MHColorWheelPanel::DefineColours( xlColorVector& _colors )
{
    colors = _colors;
    CreateHsvBitmap(wxSize(256, 256));
    CreateHsvBitmapMask();
}

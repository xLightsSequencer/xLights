#include "MovingHeadCanvasPanel.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

namespace
{
    const int BorderWidth = 5;
}

BEGIN_EVENT_TABLE(MovingHeadCanvasPanel, wxPanel)
    EVT_PAINT(MovingHeadCanvasPanel::OnMovingHeadPaint)
    EVT_LEFT_DOWN(MovingHeadCanvasPanel::OnMovingHeadLeftDown)
    EVT_MOTION(MovingHeadCanvasPanel::OnMovingHeadMouseMove)
    EVT_ENTER_WINDOW(MovingHeadCanvasPanel::OnMovingHeadEntered)
END_EVENT_TABLE()


MovingHeadCanvasPanel::MovingHeadCanvasPanel(IMovingHeadCanvasParent* movingHeadCanvasParent, wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS),
    m_movingHeadCanvasParent(movingHeadCanvasParent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void MovingHeadCanvasPanel::OnMovingHeadPaint(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC pdc(this);
    wxSize sz(GetSize());
    wxRect borderRect(BorderWidth, BorderWidth, sz.GetWidth() - 2 * BorderWidth - 2, sz.GetHeight() - 2 * BorderWidth - 2);
    wxRect bgRect(wxRect(borderRect).Deflate(1, 1));

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(borderRect);
    
    for( int i = 0; i < 9; ++i ) {
        wxPoint2DDouble start_x(0.1 * (float)i + 0.1, 0);
        wxPoint2DDouble end_x(0.1 * (float)i + 0.1, 1);
        wxPoint2DDouble start_y(0, 0.1 * (float)i + 0.1);
        wxPoint2DDouble end_y(1, 0.1 * (float)i + 0.1);
        if( i == 4 ) {
            pdc.SetPen(*wxGREY_PEN);
        } else {
            pdc.SetPen(*wxLIGHT_GREY_PEN);
        }
        pdc.DrawLine(NormalizedToUI2(start_x), NormalizedToUI2(end_x));
        pdc.DrawLine(NormalizedToUI2(start_y), NormalizedToUI2(end_y));
    }
    
    pdc.SetPen(*wxRED_PEN);
    wxPoint2DDouble lstart_x(0, m_mousePos.m_y);
    wxPoint2DDouble lend_x(m_mousePos.m_x-2.0f, m_mousePos.m_y);
    wxPoint2DDouble rstart_x(m_mousePos.m_x+2.0f, m_mousePos.m_y);
    wxPoint2DDouble rend_x(1.0f, m_mousePos.m_y);
    pdc.DrawLine(NormalizedToUI2(lstart_x), NormalizedToUI2(lend_x));
    pdc.DrawLine(NormalizedToUI2(rstart_x), NormalizedToUI2(rend_x));
    wxPoint2DDouble tstart_y(m_mousePos.m_x, 0);
    wxPoint2DDouble tend_y(m_mousePos.m_x, m_mousePos.m_y-2.0f);
    wxPoint2DDouble bstart_y(m_mousePos.m_x, m_mousePos.m_y+2.0f);
    wxPoint2DDouble bend_y(m_mousePos.m_x, 1.0f);
    pdc.DrawLine(NormalizedToUI2(tstart_y), NormalizedToUI2(tend_y));
    pdc.DrawLine(NormalizedToUI2(bstart_y), NormalizedToUI2(bend_y));
    wxPoint2DDouble top_left_corner(m_mousePos.m_x-2.0f, m_mousePos.m_y-2.0f);
    wxPoint2DDouble top_right_corner(m_mousePos.m_x+2.0f, m_mousePos.m_y-2.0f);
    wxPoint2DDouble bot_left_corner(m_mousePos.m_x-2.0f, m_mousePos.m_y+2.0f);
    wxPoint2DDouble bot_right_corner(m_mousePos.m_x+2.0f, m_mousePos.m_y+2.0f);
    pdc.DrawLine(NormalizedToUI2(top_left_corner), NormalizedToUI2(top_right_corner));
    pdc.DrawLine(NormalizedToUI2(top_right_corner), NormalizedToUI2(bot_right_corner));
    pdc.DrawLine(NormalizedToUI2(bot_right_corner), NormalizedToUI2(bot_left_corner));
    pdc.DrawLine(NormalizedToUI2(bot_left_corner), NormalizedToUI2(top_left_corner));
}

void MovingHeadCanvasPanel::SetPosition(wxPoint2DDouble pos)
{

}

void MovingHeadCanvasPanel::OnMovingHeadLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    m_movingHeadCanvasParent->NotifyPositionUpdated();
}

void MovingHeadCanvasPanel::OnMovingHeadMouseMove(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    m_movingHeadCanvasParent->NotifyPositionUpdated();
    Refresh();
}

void MovingHeadCanvasPanel::OnMovingHeadEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble MovingHeadCanvasPanel::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble MovingHeadCanvasPanel::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y));
}

wxPoint MovingHeadCanvasPanel::NormalizedToUI2(const wxPoint2DDouble& pt) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}

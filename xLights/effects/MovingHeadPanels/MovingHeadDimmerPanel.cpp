/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MovingHeadDimmerPanel.h"
#include "../../sequencer/Element.h"

#include <wx/dcbuffer.h>
#include <wx/graphics.h>

namespace
{
    const int BorderWidth = 5;
    const int HandleWidth = 6;
    const int HalfHandleWidth = HandleWidth / 2;
    const float box_size = 0.02f;
    const float snap_zone = 0.015f;
    const float num_hlines = 10.0f;
    const float num_vlines = 8.0f;
    const float hline_gap = 1.0f / num_hlines;
    const float vline_gap = 1.0f / num_vlines;
    const float marker_length = 0.04f;
    const float pt_tol = 0.001f;  // can't let points be too close or Value Curve deletes them
}

BEGIN_EVENT_TABLE(MovingHeadDimmerPanel, wxPanel)
    EVT_PAINT(MovingHeadDimmerPanel::OnMovingHeadPaint)
    EVT_KEY_DOWN(MovingHeadDimmerPanel::OnKeyDown)
    EVT_LEFT_DOWN(MovingHeadDimmerPanel::OnMovingHeadLeftDown)
    EVT_LEFT_UP(MovingHeadDimmerPanel::OnMovingHeadLeftUp)
    EVT_MOTION(MovingHeadDimmerPanel::OnMovingHeadMouseMove)
    EVT_ENTER_WINDOW(MovingHeadDimmerPanel::OnMovingHeadEntered)
    EVT_SIZE(MovingHeadDimmerPanel::OnSize)
END_EVENT_TABLE()


MovingHeadDimmerPanel::MovingHeadDimmerPanel(IMovingHeadDimmerParent* MovingHeadDimmerParent, wxWindow* parent, wxWindowID id /*=wxID_ANY*/, const wxPoint& pos /*= wxDefaultPosition*/, const wxSize& size /*=wxDefaultSize*/) :
    wxPanel(parent, id, pos, size, wxNO_BORDER | wxWANTS_CHARS),
    m_MovingHeadDimmerParent(MovingHeadDimmerParent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    wxPoint2DDouble pt1(NormalizedToUI2(wxPoint2DDouble(0.0f,0.0f)));
    wxPoint2DDouble pt2(NormalizedToUI2(wxPoint2DDouble(1.0f,0.0f)));
    m_handles.push_back(UItoNormalized(pt1));
    m_handles.push_back(UItoNormalized(pt2));
}

void MovingHeadDimmerPanel::OnSize(wxSizeEvent& event){
    wxSize old_sz = GetSize();
    if( old_sz.GetHeight() != old_sz.GetWidth()/2 ) {
        old_sz.SetHeight(old_sz.GetWidth()/2);
        SetMinSize(old_sz);
        SetSize(old_sz);
    }
    Refresh();
    //skip the event.
    event.Skip();
}

void MovingHeadDimmerPanel::OnMovingHeadPaint(wxPaintEvent& /*event*/)
{
    wxAutoBufferedPaintDC pdc(this);
    wxSize sz(GetSize());

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxGREY_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);
    pdc.SetPen(*wxLIGHT_GREY_PEN);

   for( int i = 0; i <= num_vlines; ++i ) {
        wxPoint2DDouble start_x1(vline_gap * (float)i, 0);
        wxPoint2DDouble end_x1(vline_gap * (float)i, marker_length);
        wxPoint2DDouble start_x2(vline_gap * (float)i, 1 - marker_length);
        wxPoint2DDouble end_x2(vline_gap * (float)i, 1);
        pdc.DrawLine(NormalizedToUI2(start_x1), NormalizedToUI2(end_x1));
        pdc.DrawLine(NormalizedToUI2(start_x2), NormalizedToUI2(end_x2));
    }

    for( int i = 0; i <= num_hlines + 2; ++i ) {
        wxPoint2DDouble start_y1(0, hline_gap * (float)i);
        wxPoint2DDouble end_y1(marker_length, hline_gap * (float)i);
        wxPoint2DDouble start_y2(1 - marker_length, hline_gap * (float)i);
        wxPoint2DDouble end_y2(1, hline_gap * (float)i);
        pdc.DrawLine(NormalizedToUI2(start_y1), NormalizedToUI2(end_y1));
        pdc.DrawLine(NormalizedToUI2(start_y2), NormalizedToUI2(end_y2));
    }

    // draw outline
    pdc.DrawLine(NormalizedToUI2(wxPoint2DDouble(0.0f,0.0f)), NormalizedToUI2(wxPoint2DDouble(0.0f,1.0f)));
    pdc.DrawLine(NormalizedToUI2(wxPoint2DDouble(0.0f,1.0f)), NormalizedToUI2(wxPoint2DDouble(1.0f,1.0f)));
    pdc.DrawLine(NormalizedToUI2(wxPoint2DDouble(1.0f,1.0f)), NormalizedToUI2(wxPoint2DDouble(1.0f,0.0f)));
    pdc.DrawLine(NormalizedToUI2(wxPoint2DDouble(1.0f,0.0f)), NormalizedToUI2(wxPoint2DDouble(0.0f,0.0f)));

    if (timingTrack_ != nullptr) {
        pdc.SetPen(*wxBLUE_PEN);
        if (timingTrack_->GetEffectLayerCount() > 0 ) {
            EffectLayer* el = timingTrack_->GetEffectLayer(0);
            for (int i = 0; i < el->GetEffectCount(); i++) {
                Effect* e = el->GetEffect(i);
                if (e->GetStartTimeMS() >= startTimeMs_ && e->GetStartTimeMS() <= endTimeMs_) {
                    double xpos = (double)(e->GetStartTimeMS() - startTimeMs_) / (double)(endTimeMs_ - startTimeMs_);
                    wxPoint2DDouble start_x1(xpos, 0);
                    wxPoint2DDouble end_x1(xpos, 1);
                    pdc.DrawLine(NormalizedToUI2(start_x1), NormalizedToUI2(end_x1));
                }
                if (e->GetEndTimeMS() >= startTimeMs_ && e->GetEndTimeMS() <= endTimeMs_) {
                    double xpos = (double)(e->GetEndTimeMS() - startTimeMs_) / (double)(endTimeMs_ - startTimeMs_);
                    wxPoint2DDouble start_x1(xpos, 0);
                    wxPoint2DDouble end_x1(xpos, 1);
                    pdc.DrawLine(NormalizedToUI2(start_x1), NormalizedToUI2(end_x1));
                }
            }
        }
    }

    // draw curve
    wxPoint old_pt(-1,-1);
    pdc.SetPen(*wxGREEN_PEN);
    for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
        wxPoint ptUI {NormalizedToUI2(*it)};
        if (old_pt.x != -1) {
            pdc.DrawLine(old_pt.x, old_pt.y, ptUI.x, ptUI.y);
        }
        old_pt = ptUI;
    }

    // draw handles
    int handle = 0;
    for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
        if (handle == selected_handle) {
            pdc.SetPen(*wxBLUE_PEN);
            pdc.SetBrush(*wxBLUE_BRUSH);
        } else {
            pdc.SetPen(*wxRED_PEN);
            pdc.SetBrush(*wxRED_BRUSH);
        }
        wxPoint ptUI {NormalizedToUI2(*it)};
        pdc.DrawRectangle(ptUI.x - HalfHandleWidth, ptUI.y - HalfHandleWidth, HandleWidth, HandleWidth);
        handle++;
    }

    if (active_handle >= 0) {
        wxPoint2DDouble pos = m_handles[active_handle];
        pdc.SetPen(*wxRED_PEN);
        wxPoint2DDouble lstart_x(0, pos.m_y);
        wxPoint2DDouble lend_x(pos.m_x-box_size, pos.m_y);
        wxPoint2DDouble rstart_x(pos.m_x+box_size, pos.m_y);
        wxPoint2DDouble rend_x(1.0f, pos.m_y);
        pdc.DrawLine(NormalizedToUI2(lstart_x), NormalizedToUI2(lend_x));
        pdc.DrawLine(NormalizedToUI2(rstart_x), NormalizedToUI2(rend_x));
        wxPoint2DDouble tstart_y(pos.m_x, 0);
        wxPoint2DDouble tend_y(pos.m_x, pos.m_y-box_size);
        wxPoint2DDouble bstart_y(pos.m_x, pos.m_y+box_size);
        wxPoint2DDouble bend_y(pos.m_x, 1.0f);
        pdc.DrawLine(NormalizedToUI2(tstart_y), NormalizedToUI2(tend_y));
        pdc.DrawLine(NormalizedToUI2(bstart_y), NormalizedToUI2(bend_y));
        wxPoint2DDouble top_left_corner(pos.m_x-box_size/2.0f, pos.m_y-box_size);
        wxPoint2DDouble top_right_corner(pos.m_x+box_size/2.0f, pos.m_y-box_size);
        wxPoint2DDouble bot_left_corner(pos.m_x-box_size/2.0f, pos.m_y+box_size);
        wxPoint2DDouble bot_right_corner(pos.m_x+box_size/2.0f, pos.m_y+box_size);
        pdc.DrawLine(NormalizedToUI2(top_left_corner), NormalizedToUI2(top_right_corner));
        pdc.DrawLine(NormalizedToUI2(top_right_corner), NormalizedToUI2(bot_right_corner));
        pdc.DrawLine(NormalizedToUI2(bot_right_corner), NormalizedToUI2(bot_left_corner));
        pdc.DrawLine(NormalizedToUI2(bot_left_corner), NormalizedToUI2(top_left_corner));
    }
}

void MovingHeadDimmerPanel::OnKeyDown(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE) {
        if( selected_handle >= 0 ) {
            m_handles.erase(m_handles.begin()+selected_handle);
            selected_handle = -1;
            active_handle = -1;
            m_MovingHeadDimmerParent->NotifyDimmerUpdated();
            Refresh();
        }
    }
}

std::string MovingHeadDimmerPanel::GetDimmerCommands()
{
    if( m_handles.size() >= 0 ) {
        std::string text{"Dimmer: "};
        bool add_comma = false;
        for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
            wxString dimmer_pt = wxString::Format("%f,%f", (*it).m_x, (*it).m_y);
            if( add_comma ) {
                text += ",";
            }
            text += dimmer_pt;
            add_comma = true;
        }
        return text;
    } else {
        return "Dimmer: 0,1,0,1";
    }
}

void MovingHeadDimmerPanel::SetDimmerCommands( const std::string& _dimmerCmds )
{
    m_handles.clear();
    selected_handle = -1;
    active_handle = -1;
    wxArrayString points = wxSplit(_dimmerCmds, ',');
    size_t num_pts = points.size() / 2;
    for (size_t i = 0; i < num_pts; ++i) {
        double x { wxAtof(points[i*2]) };
        double y { wxAtof(points[i*2+1]) };
        wxPoint2DDouble pt(x, y);
        pt.m_y = pt.m_y;
        m_handles.push_back(pt);
    }
    Refresh();
}

void MovingHeadDimmerPanel::SnapToLines(wxPoint2DDouble& pos)
{
    float snap_x = round(pos.m_x / hline_gap) * hline_gap;
    float snap_y = round(pos.m_y / vline_gap) * vline_gap;
    if( abs(pos.m_x - snap_x) < snap_zone ) {
        pos.m_x = snap_x;
    }
    if( abs(pos.m_y - snap_y) < snap_zone ) {
        pos.m_y = snap_y;
    }
}

void MovingHeadDimmerPanel::OnMovingHeadLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    m_mousePos = UItoNormalized(ptUI);
    //SnapToLines(m_mousePos);
    m_mouseDown = true;
    if( m_handles.size() == 0 ) {
        m_handles.push_back(m_mousePos);
        active_handle = 0;
        m_MovingHeadDimmerParent->NotifyDimmerUpdated();
        Refresh();
    } else {
        int selected_point = HitTest(ptUI);
        if( selected_point >= 0 ) {
            active_handle = selected_point;
        } else {
            active_handle = m_handles.size();
            int new_pos = 0;
            for (auto it = m_handles.begin(); it != m_handles.end(); ++it) {
                if( (*it).m_x < m_mousePos.m_x) {
                    new_pos++;
                } else {
                    active_handle = new_pos;
                    m_handles.insert(m_handles.begin() + active_handle, m_mousePos);
                    break;
                }
            }
            m_MovingHeadDimmerParent->NotifyDimmerUpdated();
        }
        if (active_handle != 0 &&
            active_handle != m_handles.size()-1) {
            selected_handle = active_handle;
        } else {
            selected_handle = -1;
        }
        Refresh();
    }
}

void MovingHeadDimmerPanel::OnMovingHeadLeftUp(wxMouseEvent& event)
{
    active_handle = -1;
    m_mouseDown = false;
    Refresh();
}

void MovingHeadDimmerPanel::OnMovingHeadMouseMove(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    if( m_mouseDown ) {
        SetCursor(wxCURSOR_HAND);
        m_mousePos = UItoNormalized(ptUI);
        //SnapToLines(m_mousePos);
        if( active_handle != -1 ) {
            m_mousePos = UItoNormalized(ptUI);
            if (m_mousePos.m_x >= 0.0f &&
                m_mousePos.m_x <= 1.0f &&
                m_mousePos.m_y >= 0.0f &&
                m_mousePos.m_y <= 1.0f) {
                // check if inside surrounding handles
                if (active_handle != 0 &&
                    active_handle != m_handles.size()-1) {
                    if (m_mousePos.m_x < m_handles[active_handle-1].m_x + pt_tol) {
                        m_handles[active_handle].m_x = m_handles[active_handle-1].m_x + pt_tol;
                    }
                    else if (m_mousePos.m_x > m_handles[active_handle+1].m_x - pt_tol) {
                        m_handles[active_handle].m_x = m_handles[active_handle+1].m_x - pt_tol;
                    } else {
                        m_handles[active_handle].m_x = m_mousePos.m_x;
                    }
                }
                m_handles[active_handle].m_y = m_mousePos.m_y;
                m_MovingHeadDimmerParent->NotifyDimmerUpdated();
            }
        }
        Refresh();
    } else {
        if ( HitTest(ptUI) >= 0 ) {
            SetCursor(wxCURSOR_HAND);
        } else {
            SetCursor(wxCURSOR_ARROW);
        }
    }
}

void MovingHeadDimmerPanel::OnMovingHeadEntered(wxMouseEvent& /*event*/)
{
    SetFocus();
}

int MovingHeadDimmerPanel::HitTest(wxPoint2DDouble& ptUI)
{
    for (int i = 0; i < m_handles.size(); ++i ) {
        wxPoint2DDouble pt{NormalizedToUI(m_handles[i])};
        if( (ptUI.m_x > pt.m_x - HalfHandleWidth) &&
        (ptUI.m_x < pt.m_x + HalfHandleWidth) &&
        (ptUI.m_y > pt.m_y - HalfHandleWidth) &&
        (ptUI.m_y < pt.m_y + HalfHandleWidth) ) {
            return i;
        }
    }
    return -1;
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble MovingHeadDimmerPanel::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble MovingHeadDimmerPanel::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(GetSize() - wxSize(2 * BorderWidth + 2, 2 * BorderWidth + 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y));
}

wxPoint MovingHeadDimmerPanel::NormalizedToUI2(const wxPoint2DDouble& pt) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}

void MovingHeadDimmerPanel::SetTimingTrack( const Element* timing )
{
    timingTrack_ = timing;
}

void MovingHeadDimmerPanel::SetEffectTimeRange(int startTimeMs, int endTimeMs)
{
    startTimeMs_ = startTimeMs;
    endTimeMs_ = endTimeMs;
}

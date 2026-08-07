/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectWheelDialog.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <cmath>
#include <algorithm>
#include "effectpanels/EffectIconCache.h"
#include "effects/EffectManager.h"
#include "xLightsApp.h"
#include "xLightsMain.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

static int AngleToIndex(double dx, double dy, int numSectors) {
    double angle = atan2(dy, dx) * 180.0 / M_PI;
    if (angle < 0) angle += 360.0;
    double sectorSize = 360.0 / numSectors;
    angle = fmod(angle - 270.0 + (sectorSize / 2.0) + 360.0, 360.0);
    return (int)(angle / sectorSize) % numSectors;
}

BEGIN_EVENT_TABLE(EffectWheelDialog, wxDialog)
    EVT_PAINT(EffectWheelDialog::OnPaint)
    EVT_MOTION(EffectWheelDialog::OnMouseMove)
    EVT_LEFT_DOWN(EffectWheelDialog::OnLeftDown)
    EVT_LEFT_UP(EffectWheelDialog::OnLeftUp)
    EVT_SHOW(EffectWheelDialog::OnShow)
END_EVENT_TABLE()

EffectWheelDialog::EffectWheelDialog(wxWindow* parent, const std::vector<const KeyBinding*>& bindings)
    : wxDialog(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxSTAY_ON_TOP | wxFRAME_SHAPED)
    , m_bindings(bindings)
    , m_selectedBinding(nullptr)
    , m_hoveredSector(-1)
    , m_hoveredPage(-1)
    , m_currentPage(0)
    , m_centerHovered(false)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    int total = (int)m_bindings.size();
    int sizeVal = FromDIP(240);
    m_innerRadius = FromDIP(30);
    m_outerRadius = FromDIP(119);

    if (total > kPageSize) {
        sizeVal = FromDIP(332);
        m_innerRadius = FromDIP(56);
        m_outerRadius = FromDIP(165);
        m_exitRadius = FromDIP(28);
    } else {
        if (total > 8) {
            sizeVal = FromDIP(320);
            m_innerRadius = FromDIP(45);
            m_outerRadius = FromDIP(159);
        }
        m_exitRadius = m_innerRadius;
    }

    m_center = wxPoint(sizeVal / 2, sizeVal / 2);
    SetClientSize(wxSize(sizeVal, sizeVal));

    SetCircularShape(m_outerRadius);

    Connect(wxID_ANY, wxEVT_CHAR_HOOK, wxKeyEventHandler(EffectWheelDialog::OnKeyDown), nullptr, this);
}

int EffectWheelDialog::PageCount() const {
    int total = (int)m_bindings.size();
    if (total <= kPageSize) return 1;
    return (total + kPageSize - 1) / kPageSize;
}

std::vector<const KeyBinding*> EffectWheelDialog::CurrentPageBindings() const {
    size_t start = (size_t)m_currentPage * kPageSize;
    if (start >= m_bindings.size()) return {};
    size_t end = std::min(start + (size_t)kPageSize, m_bindings.size());
    return std::vector<const KeyBinding*>(m_bindings.begin() + start, m_bindings.begin() + end);
}

void EffectWheelDialog::PositionAtMouse(const wxPoint& mousePos) {
    Move(mousePos - m_center);
}

void EffectWheelDialog::SetCircularShape(int radius) {
    const int numPoints = 32;
    std::vector<wxPoint> points(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        double angle = i * 2.0 * M_PI / numPoints;
        points[i].x = m_center.x + radius * cos(angle);
        points[i].y = m_center.y + radius * sin(angle);
    }
    SetShape(wxRegion(points.size(), points.data()));
}

std::string EffectWheelDialog::GetEffectNameFromBinding(const KeyBinding* kb) {
    if (!kb) return "";
    
    // Details string is: EffectName[:SettingsString]
    std::string details = "";
    if (kb->GetEffectName() != "" && kb->GetEffectString() != "") {
        details = kb->GetEffectName() + ":" + kb->GetEffectString();
    } else if (kb->GetEffectString() != "") {
        details = kb->GetEffectString();
    } else if (kb->GetEffectName() != "") {
        details = kb->GetEffectName();
    }

    size_t colonPos = details.find(':');
    std::string effectName = (colonPos == std::string::npos) ? details : details.substr(0, colonPos);
    return effectName;
}

EffectWheelDialog::HitResult EffectWheelDialog::HitTest(const wxPoint& pos) const {
    double dx = pos.x - m_center.x;
    double dy = pos.y - m_center.y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist > m_outerRadius) return {HitZone::None, -1};

    if (dist >= m_innerRadius) {
        int numSectors = (int)CurrentPageBindings().size();
        if (numSectors == 0) return {HitZone::None, -1};
        return {HitZone::Spoke, AngleToIndex(dx, dy, numSectors)};
    }

    if (dist >= m_exitRadius) {
        int page = AngleToIndex(dx, dy, kMaxPages);
        return (page < PageCount()) ? HitResult{HitZone::PageLive, page} : HitResult{HitZone::PageDead, -1};
    }

    return {HitZone::Exit, -1};
}

void EffectWheelDialog::OnMouseMove(wxMouseEvent& event) {
    HitResult hit = HitTest(event.GetPosition());
    int newHoverSector = (hit.zone == HitZone::Spoke) ? hit.index : -1;
    int newHoverPage = (hit.zone == HitZone::PageLive) ? hit.index : -1;
    bool newCenterHovered = (hit.zone == HitZone::Exit);

    if (newHoverSector != m_hoveredSector || newHoverPage != m_hoveredPage || newCenterHovered != m_centerHovered) {
        m_hoveredSector = newHoverSector;
        m_hoveredPage = newHoverPage;
        m_centerHovered = newCenterHovered;
        Refresh();
    }
}

void EffectWheelDialog::OnPaint(wxPaintEvent& WXUNUSED(event)) {
    wxAutoBufferedPaintDC dc(this);
    
    // Clear background to a solid dark grey matching xLights grid dark theme
    dc.SetBackground(wxBrush(wxColour(30, 30, 30)));
    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (!gc) return;

    auto pageBindings = CurrentPageBindings();
    int numSectors = (int)pageBindings.size();
    if (numSectors == 0) {
        delete gc;
        return;
    }

    double sectorSizeRad = (2.0 * M_PI) / numSectors;

    // Translucent dark outer wheel circle
    gc->SetBrush(wxBrush(wxColour(30, 30, 30, 220)));
    gc->SetPen(wxPen(wxColour(255, 255, 255, 50), 2));
    gc->DrawEllipse(m_center.x - m_outerRadius, m_center.y - m_outerRadius, m_outerRadius * 2, m_outerRadius * 2);

    int iconSize = FromDIP(24);

    for (int i = 0; i < numSectors; ++i) {
        double angle = i * sectorSizeRad - M_PI_2 - (sectorSizeRad / 2.0);

        // Highlight hovered sector
        if (i == m_hoveredSector) {
            wxGraphicsPath path = gc->CreatePath();
            path.AddArc(m_center.x, m_center.y, m_outerRadius, angle, angle + sectorSizeRad, true);
            path.AddArc(m_center.x, m_center.y, m_innerRadius, angle + sectorSizeRad, angle, false);
            path.CloseSubpath();

            gc->SetBrush(wxBrush(wxColour(0, 120, 215, 120))); // Nice soft blue highlight
            gc->FillPath(path);
        }

        // Draw radial dividing line
        gc->SetPen(wxPen(wxColour(255, 255, 255, 30), 1));
        gc->StrokeLine(m_center.x + m_innerRadius * cos(angle), m_center.y + m_innerRadius * sin(angle),
                       m_center.x + m_outerRadius * cos(angle), m_center.y + m_outerRadius * sin(angle));

        // Draw icon and text labels
        double midAngle = angle + (sectorSizeRad / 2.0);

        // Put the icon near the outer edge to maximize circumference and avoid overlaps
        double r_icon = m_outerRadius - FromDIP(22.0);
        int ix = m_center.x + r_icon * cos(midAngle);
        int iy = m_center.y + r_icon * sin(midAngle);

        const KeyBinding* kb = pageBindings[i];
        std::string effectName = GetEffectNameFromBinding(kb);

        EffectManager& effMgr = xLightsApp::GetFrame()->GetEffectManager();
        RenderableEffect* re = effMgr.GetEffect(effectName);
        if (re) {
            const wxBitmapBundle& bundle = EffectIconCache::GetEffectIcon(re, 24);
            wxBitmap bmp = bundle.GetBitmap(wxSize(iconSize, iconSize));
            if (bmp.IsOk()) {
                // Draw icon centered at (ix, iy)
                gc->DrawBitmap(bmp, ix - iconSize / 2, iy - iconSize / 2, iconSize, iconSize);
            }
        }

        // Draw Effect Name, slanted to follow the slice
        // Put the text at a radius centered in the gap between the inner cutout and the icon
        double r_text = (m_innerRadius + r_icon - FromDIP(12.0)) / 2.0;
        int tx = m_center.x + r_text * cos(midAngle);
        int ty = m_center.y + r_text * sin(midAngle);

        gc->SetFont(wxFont(FromDIP(8), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL), wxColour(*wxWHITE));
        double tw, th;
        gc->GetTextExtent(effectName, &tw, &th);

        gc->PushState();
        gc->Translate(tx, ty);

        double textAngle = midAngle;
        if (cos(midAngle) < 0.0) {
            textAngle += M_PI; // Flip text on the left side of the circle so it's right-side up
        }
        gc->Rotate(textAngle);

        gc->DrawText(effectName, -(tw / 2.0), -(th / 2.0));
        gc->PopState();
    }

    // Paging ring: only present when there's more than one page.
    if (m_exitRadius < m_innerRadius) {
        int pageCount = PageCount();
        double pageSectorRad = (2.0 * M_PI) / kMaxPages;

        for (int p = 0; p < kMaxPages; ++p) {
            double angle = p * pageSectorRad - M_PI_2 - (pageSectorRad / 2.0);
            bool live = p < pageCount;
            bool active = live && (p == m_currentPage);
            bool hovered = live && !active && (p == m_hoveredPage);

            wxGraphicsPath path = gc->CreatePath();
            path.AddArc(m_center.x, m_center.y, m_innerRadius, angle, angle + pageSectorRad, true);
            path.AddArc(m_center.x, m_center.y, m_exitRadius, angle + pageSectorRad, angle, false);
            path.CloseSubpath();

            if (active) {
                gc->SetBrush(wxBrush(wxColour(0, 120, 215, 220)));
                gc->SetPen(wxPen(wxColour(255, 255, 255, 80), 1));
            } else if (hovered) {
                gc->SetBrush(wxBrush(wxColour(0, 120, 215, 120)));
                gc->SetPen(wxPen(wxColour(255, 255, 255, 60), 1));
            } else if (live) {
                gc->SetBrush(wxBrush(wxColour(50, 50, 50, 220)));
                gc->SetPen(wxPen(wxColour(255, 255, 255, 60), 1));
            } else {
                gc->SetBrush(wxBrush(wxColour(25, 25, 25, 160)));
                gc->SetPen(wxPen(wxColour(255, 255, 255, 20), 1));
            }
            gc->FillPath(path);
            gc->StrokePath(path);

            if (live) {
                double midAngle = angle + (pageSectorRad / 2.0);
                double r_page = (m_exitRadius + m_innerRadius) / 2.0;
                int px = m_center.x + r_page * cos(midAngle);
                int py = m_center.y + r_page * sin(midAngle);

                wxString pageLabel = wxString::Format("%d", p + 1);
                gc->SetFont(wxFont(FromDIP(10), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD), *wxWHITE);
                double pw, ph;
                gc->GetTextExtent(pageLabel, &pw, &ph);
                gc->DrawText(pageLabel, px - pw / 2.0, py - ph / 2.0);
            }
        }
    }

    // Draw central hollow cutout with hover highlight
    if (m_centerHovered) {
        gc->SetBrush(wxBrush(wxColour(180, 40, 40, 220)));
        gc->SetPen(wxPen(wxColour(255, 120, 120, 200), 2));
    } else {
        gc->SetBrush(wxBrush(wxColour(20, 20, 20, 255)));
        gc->SetPen(wxPen(wxColour(255, 255, 255, 80), 2));
    }
    gc->DrawEllipse(m_center.x - m_exitRadius, m_center.y - m_exitRadius, m_exitRadius * 2, m_exitRadius * 2);

    // Draw "Exit" hint in the center circle
    {
        wxColour textColor = m_centerHovered ? wxColour(255, 255, 255) : wxColour(170, 170, 170);
        bool large = (m_exitRadius > FromDIP(35));
        wxFont iconFont(FromDIP(large ? 11 : 9), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        wxFont labelFont(FromDIP(large ? 8 : 7), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

        double iw, ih, lw, lh;
        gc->SetFont(iconFont, textColor);
        gc->GetTextExtent(wxT("×"), &iw, &ih);
        gc->SetFont(labelFont, textColor);
        gc->GetTextExtent("Exit", &lw, &lh);

        double gap = FromDIP(1.0);
        double totalH = ih + gap + lh;
        double startY = m_center.y - totalH / 2.0;

        gc->SetFont(iconFont, textColor);
        gc->DrawText(wxT("×"), m_center.x - iw / 2.0, startY);
        gc->SetFont(labelFont, textColor);
        gc->DrawText("Exit", m_center.x - lw / 2.0, startY + ih + gap);
    }

    delete gc;
}

void EffectWheelDialog::OnLeftDown(wxMouseEvent& event) {
    HitResult hit = HitTest(event.GetPosition());

    switch (hit.zone) {
    case HitZone::Spoke: {
        auto pageBindings = CurrentPageBindings();
        m_selectedBinding = pageBindings[hit.index];
        if (HasCapture()) ReleaseMouse();
        EndModal(wxID_OK);
        break;
    }
    case HitZone::PageLive:
        if (hit.index != m_currentPage) {
            m_currentPage = hit.index;
            m_hoveredSector = -1;
            Refresh();
        }
        break;
    case HitZone::PageDead:
        break;
    case HitZone::Exit:
    case HitZone::None:
        if (HasCapture()) ReleaseMouse();
        EndModal(wxID_CANCEL);
        break;
    }
}

void EffectWheelDialog::OnLeftUp(wxMouseEvent& WXUNUSED(event)) {
}

void EffectWheelDialog::OnKeyDown(wxKeyEvent& event) {
    if (event.GetKeyCode() == WXK_ESCAPE) {
        if (HasCapture()) {
            ReleaseMouse();
        }
        EndModal(wxID_CANCEL);
    } else {
        event.Skip();
    }
}

void EffectWheelDialog::OnShow(wxShowEvent& event) {
    if (event.IsShown()) {
        if (!HasCapture()) {
            CaptureMouse();
        }
    } else {
        if (HasCapture()) {
            ReleaseMouse();
        }
    }
    event.Skip();
}

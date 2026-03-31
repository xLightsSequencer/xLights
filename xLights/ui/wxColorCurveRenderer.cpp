/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wxColorCurveRenderer.h"

#include <wx/dcmemory.h>
#include <wx/settings.h>

#include "ui/wxUtilities.h"

namespace wxColorCurveRenderer {

wxBitmap GetColorCurveImage(const ColorCurve& cc, int x, int y, bool bars)
{
    wxImage bmp(x, y);
    wxBitmap b(bmp);
    wxMemoryDC dc(b);

    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    dc.DrawRectangle(0, 0, x, y);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);

    for (int i = 0; i < x; i++) {
        wxColor c = xlColorToWxColour(cc.GetValueAt(static_cast<float>(i) / static_cast<float>(x)));
        dc.SetPen(wxPen(c, 1, wxPENSTYLE_SOLID));
        if (bars) {
            dc.DrawLine(wxPoint(i, 0), wxPoint(i, static_cast<float>(y) * 0.75));
        } else {
            dc.DrawLine(wxPoint(i, 0), wxPoint(i, y));
        }
    }

    if (!bars) {
        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
        dc.DrawRectangle(0, 0, x, y);
    }
    return b;
}

wxBitmap GetSolidColourImage(int x, int y, const wxColour& c)
{
    wxImage bmp(x, y);
    wxBitmap b(bmp);
    wxMemoryDC dc(b);
    dc.SetPen(wxPen(c));
    dc.SetBrush(wxBrush(c));
    dc.DrawRectangle(0, 0, x, y);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK)));
    dc.DrawRectangle(0, 0, x, y);
    return b;
}

} // namespace wxColorCurveRenderer

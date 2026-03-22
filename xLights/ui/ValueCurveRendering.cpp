/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ValueCurveRendering.h"
#include "render/ValueCurve.h"

#include <wx/dcmemory.h>

#include <algorithm>
#include <cmath>

wxBitmap GetValueCurveImage(ValueCurve& vc, int w, int h, double scaleFactor)
{
    if (scaleFactor < 1.0) {
        scaleFactor = 1.0;
    }
    float width = w * scaleFactor;
    float height = h * scaleFactor;

    wxBitmap bmp(width, height);

    wxMemoryDC dc(bmp);
    dc.SetBrush(*wxLIGHT_GREY_BRUSH);
    dc.DrawRectangle(0, 0, width, height);
    dc.SetPen(*wxBLACK_PEN);
    float lastY = height - 1 - (vc.GetValueAt(0, 0, 1)) * height;

    std::string type = vc.GetType();
    if (type == "Music" || type == "Inverted Music" || type == "Music Trigger Fade")
    {
        dc.DrawCircle(width / 4, height - height / 4, wxCoord(std::min(width / 5, height / 5)));
        dc.DrawLine(width / 4 + width / 5, height - height / 4, width / 4 + width / 5, height / 5);
        dc.DrawLine(width / 4 + width / 5, height / 5, width - width/10, height/ 4);
        dc.DrawLine(width / 4 + width / 5, height / 4, width - width/10, height/ 3);
        float min = (vc.GetParameter1() - vc.GetMin()) / (vc.GetMax() - vc.GetMin()) * height;
        float max = (vc.GetParameter2() - vc.GetMin()) / (vc.GetMax() - vc.GetMin()) * height;
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawLine(0, height - min, width, height - min);
        dc.SetPen(*wxRED_PEN);
        dc.DrawLine(0, height - max, width, height - max);
    }
    else if (type == "Timing Track Toggle" || type == "Timing Track Fade Fixed" || type == "Timing Track Fade Proportional") {
        dc.DrawLine(width / 4, height / 4, width - width / 4, height / 4);
        dc.DrawLine(width / 3, height - height / 4, width / 3, height / 4);
        dc.DrawLine(width - width / 3, height - height / 4, width - width / 3, height / 4);
        dc.SetPen(*wxGREEN_PEN);
        float min = (vc.GetParameter1() - vc.GetMin()) / (vc.GetMax() - vc.GetMin()) * height;
        float max = (vc.GetParameter2() - vc.GetMin()) / (vc.GetMax() - vc.GetMin()) * height;
        dc.DrawLine(0, height - min, width, height - min);
        dc.SetPen(*wxRED_PEN);
        dc.DrawLine(0, height - max, width, height - max);
    }
    else
    {
        for (int x = 1; x < width; x++) {
            float x1 = x;
            x1 /= (float)width;

            float y = (vc.GetValueAt(x1, 0, 1)) * (float)width;
            y = (float)height - 1.0f - y;
            dc.DrawLine(x - 1, lastY, x, std::round(y));
            lastY = y;
        }
    }

    if (scaleFactor > 1.0f) {
        wxImage img = bmp.ConvertToImage();
        return wxBitmap(img, 8, scaleFactor);
    }
    return bmp;
}

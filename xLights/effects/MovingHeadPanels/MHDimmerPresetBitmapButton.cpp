/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MHDimmerPresetBitmapButton.h"
#include "../SketchEffectDrawing.h"

#include "Color.h"

#include <wx/graphics.h>

MHDimmerPresetBitmapButton::MHDimmerPresetBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
}

MHDimmerPresetBitmapButton::~MHDimmerPresetBitmapButton()
{
}
void MHDimmerPresetBitmapButton::DoSetSizeHints(int minW, int minH,
    int maxW, int maxH,
    int incW, int incH)
{
    int offset = 0;
#ifdef LINUX
    offset = 12; //Linux puts a 6 pixel border around it
#endif // LINUX
    wxBitmapButton::DoSetSizeHints(minW + offset,
        minH + offset,
        maxW + offset,
        maxH + offset,
        incW, incH);
}

void MHDimmerPresetBitmapButton::SetPreset(const std::string& _settings)
{
    mSettings = _settings;
    RenderNewBitmap();
}

void MHDimmerPresetBitmapButton::RenderNewBitmap() {
    SetBitmap(CreateImage(48, 48, GetContentScaleFactor()));
}

wxBitmap MHDimmerPresetBitmapButton::CreateImage( int w, int h, double scaleFactor ) {
    if (scaleFactor < 1.0) {
        scaleFactor = 1.0;
    }
    float width = w * scaleFactor;
    float height = h * scaleFactor;

    wxBitmap bmp(width, height);

    wxMemoryDC dc(bmp);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawRectangle(0, 0, width, height);

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

        wxColour c(xlGREEN);
        wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(c));
        gc->SetPen(pen);

        wxGraphicsPath graphicsPath(gc->CreatePath());
        wxArrayString dimmers = wxSplit(mSettings, ',');
        wxPoint2DDouble pt(wxAtof(dimmers[0]), wxAtof(dimmers[1]));
        wxPoint2DDouble startPt(NormalizedToUI(pt, scaleFactor));
        graphicsPath.MoveToPoint(startPt);
        size_t num_pts = dimmers.size() / 2;
        for( size_t i = 1; i < num_pts; ++i ) {
            double x { wxAtof(dimmers[i*2]) };
            double y { wxAtof(dimmers[i*2+1]) };
            wxPoint2DDouble pt(x , y);
            wxPoint2DDouble endPt(NormalizedToUI(pt, scaleFactor));
            graphicsPath.AddLineToPoint(endPt);
        }
        gc->DrawPath(graphicsPath);

    }
    
    if (scaleFactor > 1.0f) {
        wxImage img = bmp.ConvertToImage();
        return wxBitmap(img, 8, scaleFactor);
    }
    return bmp;
}

void MHDimmerPresetBitmapButton::SetBitmap(const wxBitmapBundle& bpm)
{
    wxBitmapButton::SetBitmap(bpm);
}

wxPoint2DDouble MHDimmerPresetBitmapButton::NormalizedToUI(const wxPoint2DDouble& pt, double scaleFactor) const
{
    wxSize sz(GetSize());
    double x = pt.m_x * sz.GetWidth() * scaleFactor;
    double y = pt.m_y * sz.GetHeight() * scaleFactor;
    return wxPoint2DDouble(x, + ((sz.GetHeight() * scaleFactor) - y));
}

wxPoint MHDimmerPresetBitmapButton::NormalizedToUI2(const wxPoint2DDouble& pt, double scaleFactor) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt, scaleFactor);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}


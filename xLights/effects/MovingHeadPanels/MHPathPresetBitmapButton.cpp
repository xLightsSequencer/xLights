/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MHPathPresetBitmapButton.h"
#include "../SketchEffectDrawing.h"

#include "Color.h"

#include <wx/graphics.h>

MHPathPresetBitmapButton::MHPathPresetBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
}

MHPathPresetBitmapButton::~MHPathPresetBitmapButton()
{
}
void MHPathPresetBitmapButton::DoSetSizeHints(int minW, int minH,
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

void MHPathPresetBitmapButton::SetPreset(const std::string& _settings)
{
    mSettings = _settings;
    RenderNewBitmap();
}

void MHPathPresetBitmapButton::RenderNewBitmap() {
    SetBitmap(CreateImage(48, 48, GetContentScaleFactor()));
}

wxBitmap MHPathPresetBitmapButton::CreateImage( int w, int h, double scaleFactor ) {
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

        wxColour c(xlORANGE);
        wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(c));
        gc->SetPen(pen);

        SketchEffectSketch sketch = SketchEffectSketch::SketchFromString(mSettings);

        // Moving heads only have one path
        const auto& all_paths = sketch.paths();
        const auto& path = all_paths[0];

        wxGraphicsPath graphicsPath(gc->CreatePath());
        const auto& firstSegment(path->segments().front());
        wxPoint2DDouble startPt(NormalizedToUI(firstSegment->StartPoint(), scaleFactor));
        graphicsPath.MoveToPoint(startPt);
    
        for (const auto& segment : path->segments()) {
            std::shared_ptr<SketchQuadraticBezier> quadratic;
            std::shared_ptr<SketchCubicBezier> cubic;

            if (std::dynamic_pointer_cast<SketchLine>(segment) != nullptr) {
                wxPoint2DDouble endPt(NormalizedToUI(segment->EndPoint(), scaleFactor));
                graphicsPath.AddLineToPoint(endPt);
            } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segment)) != nullptr) {
                wxPoint2DDouble ctrlPt(NormalizedToUI(quadratic->ControlPoint(), scaleFactor));
                wxPoint2DDouble endPt(NormalizedToUI(quadratic->EndPoint(), scaleFactor));
                graphicsPath.AddQuadCurveToPoint(ctrlPt.m_x, ctrlPt.m_y, endPt.m_x, endPt.m_y);
            } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segment)) != nullptr) {
                wxPoint2DDouble ctrlPt1(NormalizedToUI(cubic->ControlPoint1(), scaleFactor));
                wxPoint2DDouble ctrlPt2(NormalizedToUI(cubic->ControlPoint2(), scaleFactor));
                wxPoint2DDouble endPt(NormalizedToUI(cubic->EndPoint(), scaleFactor));
                graphicsPath.AddCurveToPoint(ctrlPt1, ctrlPt2, endPt);
            }
        }
        if (path->isClosed())
            graphicsPath.CloseSubpath();

        gc->DrawPath(graphicsPath);

    }
    
    if (scaleFactor > 1.0f) {
        wxImage img = bmp.ConvertToImage();
        return wxBitmap(img, 8, scaleFactor);
    }
    return bmp;
}

void MHPathPresetBitmapButton::SetBitmap(const wxBitmapBundle& bpm)
{
    wxBitmapButton::SetBitmap(bpm);
}

wxPoint2DDouble MHPathPresetBitmapButton::NormalizedToUI(const wxPoint2DDouble& pt, double scaleFactor) const
{
    wxSize sz(GetSize());
    double x = pt.m_x * sz.GetWidth() * scaleFactor;
    double y = pt.m_y * sz.GetHeight() * scaleFactor;
    return wxPoint2DDouble(x, + ((sz.GetHeight() * scaleFactor) - y));
}

wxPoint MHPathPresetBitmapButton::NormalizedToUI2(const wxPoint2DDouble& pt, double scaleFactor) const
{
    wxPoint2DDouble pt1 = NormalizedToUI(pt, scaleFactor);
    return wxPoint((int)pt1.m_x, (int)pt1.m_y);
}


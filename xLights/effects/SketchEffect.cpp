#include "SketchEffect.h"

#include "BulkEditControls.h"
#include "RenderBuffer.h"
#include "SketchEffectDrawing.h"
#include "SketchPanel.h"
#include "assist/AssistPanel.h"
#include "assist/SketchAssistPanel.h"

#include "../../include/sketch-64.xpm"
#include "../../include/sketch-48.xpm"
#include "../../include/sketch-32.xpm"
#include "../../include/sketch-24.xpm"
#include "../../include/sketch-16.xpm"

#include <wx/image.h>

#include <algorithm>
#include <cstdint>

namespace
{
    struct LinearInterpolater {
        double operator()(double t) const
        {
            return t;
        }
    };

    template<class T>
    double interpolate(double x, double loIn, double loOut, double hiIn, double hiOut, const T& interpolater)
    {
        return (loIn != hiIn)
                   ? (loOut + (hiOut - loOut) * interpolater((x - loIn) / (hiIn - loIn)))
                   : ((loOut + hiOut) / 2);
    }

    double calcPercentage(double v, double s, double e)
    {
        return (v - s) / (e - s);
    }
}

SketchEffect::SketchEffect(int id) :
    RenderableEffect(id, "Sketch", sketch_16_xpm, sketch_24_xpm, sketch_32_xpm, sketch_48_xpm, sketch_64_xpm)
{
}

SketchEffect::~SketchEffect()
{

}

void SketchEffect::Render(Effect* /*effect*/, SettingsMap& settings, RenderBuffer& buffer )
{
    double progress = buffer.GetEffectTimeIntervalPosition(1.f);

    std::string sketchDef = settings.Get("TEXTCTRL_SketchDef", "");
    double drawPercentage = GetValueCurveDouble("DrawPercentage", SketchPanel::DrawPercentageDef, settings, progress,
                                                SketchPanel::DrawPercentageMin, SketchPanel::DrawPercentageMax,
                                                buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int thickness = GetValueCurveInt("Thickness", SketchPanel::ThicknessDef, settings, progress,
                                     SketchPanel::ThicknessMin, SketchPanel::ThicknessMax,
                                     buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool motionEnabled = settings.GetBool("CHECKBOX_MotionEnabled");
    int motionPercentage = GetValueCurveInt("MotionPercentage", SketchPanel::MotionPercentageDef, settings, progress,
                                            SketchPanel::MotionPercentageMin, SketchPanel::MotionPercentageMax,
                                            buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    xlColorVector colors(buffer.GetColorCount());
    for (size_t i = 0; i < buffer.GetColorCount(); ++i)
        colors[i] = buffer.palette.GetColor(i);
    
    if (sketchDef.empty())
        return;
    SketchEffectSketch sketch(SketchEffectSketch::SketchFromString(sketchDef));

    //
    // RenderBuffer --> wxImage
    //
    int bw = buffer.BufferWi;
    int bh = buffer.BufferHt;
    std::vector<uint8_t> rgb(bw * 3 * bh);
    std::vector<uint8_t> alpha(bw * bh);
    xlColor* px = buffer.GetPixels();
    int pxIndex = 0;
    int rgbIndex = 0;
    int alphaIndex = 0;
    for ( int y = 0; y < bh; ++y )
    {
        for ( int x = 0; x < bw; ++x, ++pxIndex )
        {
            rgb[rgbIndex++] = px[pxIndex].Red();
            rgb[rgbIndex++] = px[pxIndex].Green();
            rgb[rgbIndex++] = px[pxIndex].Green();
            alpha[alphaIndex++] = px[pxIndex].Alpha();
        }
    }
    wxImage img(bw, bh, rgb.data(), alpha.data(), true);

    //
    // rendering sketch via wxGraphicsContext
    //
    renderSketch(sketch, img, progress, 0.01*drawPercentage, thickness, motionEnabled, 0.01*motionPercentage, colors);

    //
    // wxImage --> RenderBuffer
    //
    for ( int y = 0; y < bh; ++y )
    {
        for (int x = 0; x < bw; ++x, ++px)
        {
            px->red = img.GetRed(x, y);
            px->green = img.GetGreen(x, y);
            px->blue = img.GetBlue(x, y);
            px->alpha = img.GetAlpha(x, y);
        }
    }
}

void SketchEffect::SetDefaultParameters()
{
    SketchPanel* p = (SketchPanel*)panel;

    SetTextValue(p->TextCtrl_SketchDef, SketchEffectSketch::DefaultSketchString());

    p->BitmapButton_DrawPercentage->SetActive(false);
    p->BitmapButton_Thickness->SetActive(false);
    p->BitmapButton_MotionPercentage->SetActive(false);

    SetCheckBoxValue(p->CheckBox_MotionEnabled, false);

    SetSliderValue(p->Slider_SketchBackgroundOpacity, 0x30);
    SetSliderValue(p->Slider_DrawPercentage, SketchPanel::DrawPercentageDef);
    SetSliderValue(p->Slider_Thickness, SketchPanel::ThicknessDef);
    SetSliderValue(p->Slider_MotionPercentage, SketchPanel::MotionPercentageDef);

    p->FilePicker_SketchBackground->SetFileName(wxFileName());

    p->ValidateWindow();
}

bool SketchEffect::needToAdjustSettings( const std::string& /*version*/ )
{
    return false;
}

void SketchEffect::adjustSettings( const std::string& version, Effect* effect, bool removeDefaults/*=true*/ )
{
    // give the base class a chance to adjust any settings
    if ( RenderableEffect::needToAdjustSettings( version ) )
    {
        RenderableEffect::adjustSettings( version, effect, removeDefaults );
    }
}

std::list<std::string> SketchEffect::CheckEffectSettings(const SettingsMap& /*settings*/, AudioManager* /*media*/, Model* /*model*/, Effect* /*eff*/, bool /*renderCache*/ )
{
    return std::list<std::string>();
}

AssistPanel* SketchEffect::GetAssistPanel(wxWindow* parent, xLightsFrame* /*xl_frame*/)
{
    if (m_panel == nullptr)
        return nullptr;
    auto lambda = [this](const std::string& sketchDef) {
        if (m_panel != nullptr) {
            m_panel->TextCtrl_SketchDef->SetValue(sketchDef);
        }
    };

    AssistPanel* assistPanel = new AssistPanel(parent);

    auto sketchAssistPanel = new SketchAssistPanel(assistPanel->GetCanvasParent());
    sketchAssistPanel->SetSketchDef(m_panel->TextCtrl_SketchDef->GetValue().ToStdString());
    sketchAssistPanel->SetSketchUpdateCallback(lambda);
    //sketchAssistPanel->SetxLightsFrame(xl_frame);
    assistPanel->AddPanel(sketchAssistPanel, wxALL | wxEXPAND);

    m_sketchAssistPanel = sketchAssistPanel;
    updateSketchAssistBackground();

    return assistPanel;
}

void SketchEffect::RemoveDefaults( const std::string& version, Effect* effect )
{

}

xlEffectPanel* SketchEffect::CreatePanel( wxWindow* parent )
{
    m_panel = new SketchPanel(parent);
    return m_panel;
}


void SketchEffect::renderSketch(const SketchEffectSketch& sketch, wxImage& img, double progress, double drawPercentage, int lineThickness, bool hasMotion, double motionPercentage, const xlColorVector& colors)
{
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(img));
    auto paths = sketch.paths();
    wxSize sz(img.GetSize());

    // In order for the animation to both "draw in" and "draw out" we adjust the [0,1] range...
    double maxProgress = hasMotion ? (1. + motionPercentage) : 1.;
    double adjustedProgress = interpolate(progress, 0., 0., 1., maxProgress, LinearInterpolater());

    // ... but we do a slightly different adjustment for the non-motion case
    if (!hasMotion)
        adjustedProgress = interpolate(progress, 0.0, 0.0, drawPercentage, 1.0, LinearInterpolater());
  
    double totalLength = 0.;
    for (const auto& path : paths)
        totalLength += path->Length();

    // Single closed path with motion is a special case for now... since the motion is supposed to
    // wrap to the beginning of the path, it's unclear when we would ever move on to the next path
    // unless a new setting was added
    if (hasMotion && paths.size() == 1 && paths.front()->isClosed()) {
        auto path = paths.front();
        wxColor color(colors[0].asWxColor());
        wxPen pen(color, lineThickness);
        gc->SetPen(pen);

        path->drawPartialPath(gc.get(), sz, progress, progress + motionPercentage);
        if (progress + motionPercentage > 1.)
            path->drawPartialPath(gc.get(), sz, 0., progress + motionPercentage - 1.);
        return;
    }
 
    double cumulativeLength = 0.;
    int i = 0;
    for (auto iter = paths.cbegin(); iter != paths.cend(); ++iter, ++i)
    {
        wxColor color(colors[i%colors.size()].asWxColor());
        wxPen pen(color, lineThickness);
        gc->SetPen(pen);
        double pathLength = (*iter)->Length();
        double percentageAtEndOfThisPath = (cumulativeLength + pathLength) / totalLength;
    
        if (!hasMotion && percentageAtEndOfThisPath <= adjustedProgress)
            (*iter)->drawEntirePath(gc.get(), sz);
        else {
            double percentageAtStartOfThisPath = cumulativeLength / totalLength;
            double percentageThroughThisPath = calcPercentage(adjustedProgress, percentageAtStartOfThisPath, percentageAtEndOfThisPath);
            if (!hasMotion)
                (*iter)->drawPartialPath(gc.get(), sz, std::nullopt, percentageThroughThisPath);
            else {
                double drawPercentageThroughThisPath = calcPercentage(adjustedProgress - motionPercentage, percentageAtStartOfThisPath, percentageAtEndOfThisPath);
                drawPercentageThroughThisPath = std::clamp(drawPercentageThroughThisPath, 0., 1.);

                (*iter)->drawPartialPath(gc.get(), sz, drawPercentageThroughThisPath, percentageThroughThisPath);
            }
        }
        cumulativeLength += pathLength;
    }
}

void SketchEffect::updateSketchAssistBackground() const
{
    if (m_panel == nullptr || m_sketchAssistPanel == nullptr)
        return;

    wxString path(m_panel->FilePicker_SketchBackground->GetFileName().GetFullPath());
    int opacity = m_panel->Slider_SketchBackgroundOpacity->GetValue();

    m_sketchAssistPanel->UpdateSketchBackground(path, opacity);
}

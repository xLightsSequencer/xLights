#include "SketchEffect.h"

#include "RenderBuffer.h"
#include "SketchEffectDrawing.h"
#include "SketchPanel.h"

#include "../../include/sketch-64.xpm"
#include "../../include/sketch-48.xpm"
#include "../../include/sketch-32.xpm"
#include "../../include/sketch-24.xpm"
#include "../../include/sketch-16.xpm"

#include <wx/image.h>

#include <cstdint>

SketchEffect::SketchEffect( int id ) :
    RenderableEffect( id, "Sketch", sketch_16_xpm, sketch_24_xpm, sketch_32_xpm, sketch_48_xpm, sketch_64_xpm ),
    m_sketch(SketchEffectSketch::DefaultSketch())
{
}

SketchEffect::~SketchEffect()
{

}

void SketchEffect::Render(Effect* /*effect*/, SettingsMap& /*settings*/, RenderBuffer& buffer )
{
    // This is a terrible effect... it currently just draws a hard-coded sketch!!
    double progress = buffer.GetEffectTimeIntervalPosition(1.f);

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
    renderSketch(img, progress);

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

void SketchEffect::RemoveDefaults( const std::string& version, Effect* effect )
{

}

xlEffectPanel* SketchEffect::CreatePanel( wxWindow* parent )
{
    return new SketchPanel( parent );
}

void SketchEffect::renderSketch(wxImage& img, double progress)
{
    std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(img));
    auto paths = m_sketch.paths();
    
    double totalLength = 0.;
    for (const auto& path : paths)
        totalLength += path->Length();
    
    double cumulativeLength = 0.;
    int i = 0;
    for (auto iter = paths.cbegin(); iter != paths.cend(); ++iter, ++i)
    {
        gc->SetPen((i % 2) ? *wxGREEN : *wxRED);
        double pathLength = (*iter)->Length();
        double percentageAtEndOfThisPath = (cumulativeLength + pathLength) / totalLength;
    
        if (percentageAtEndOfThisPath <= progress)
            (*iter)->drawEntirePath(gc.get());
        else
        {
            double percentageAtStartOfThisPath = cumulativeLength / totalLength;
            double percentageThroughThisPath = (progress - percentageAtStartOfThisPath) / (percentageAtEndOfThisPath - percentageAtStartOfThisPath);
            if (percentageThroughThisPath >= 0.)
                (*iter)->drawPartialPath(gc.get(), percentageThroughThisPath);
        }
        cumulativeLength += pathLength;
    }
}

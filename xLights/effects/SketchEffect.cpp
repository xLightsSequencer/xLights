#include "SketchEffect.h"

#include "RenderBuffer.h"
#include "SketchPanel.h"

#include "../../include/sketch-64.xpm"
#include "../../include/sketch-48.xpm"
#include "../../include/sketch-32.xpm"
#include "../../include/sketch-24.xpm"
#include "../../include/sketch-16.xpm"

#include <wx/graphics.h>
#include <wx/image.h>

#include <cstdint>
#include <memory>
#include <vector>

SketchEffect::SketchEffect( int id )
   : RenderableEffect( id, "Sketch", sketch_16_xpm, sketch_24_xpm, sketch_32_xpm, sketch_48_xpm, sketch_64_xpm )
{

}

SketchEffect::~SketchEffect()
{

}

void SketchEffect::Render(Effect* /*effect*/, SettingsMap& /*settings*/, RenderBuffer& buffer )
{
    // This is a terrible effect... it currently does almost nothing!!
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
    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(img));
        wxGraphicsPath path1 = gc->CreatePath();
        path1.MoveToPoint(0., 0.);
        path1.AddLineToPoint(0.5 * bw, 0.5 * bh);
        path1.AddCurveToPoint(bw, bh, 0.25 * bw, 0.25 * bh, 0.65 * bw, 0.3 * bh);
        path1.CloseSubpath();

        wxGraphicsPath path2 = gc->CreatePath();
        path2.MoveToPoint(0.8 * bw, 0.8 * bh);
        path2.AddLineToPoint(0.3 * bw, 0.7 * bh);
        path2.AddLineToPoint(0.05 * bw, 0.1 * bh);

        gc->SetPen(*wxRED);
        gc->StrokePath(path1);

        gc->SetPen(*wxGREEN);
        gc->StrokePath(path2);
    }

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

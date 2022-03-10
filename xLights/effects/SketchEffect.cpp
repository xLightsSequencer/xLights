#include "SketchEffect.h"

#include "RenderBuffer.h"
#include "SketchPanel.h"

#include "../../include/sketch-64.xpm"
#include "../../include/sketch-48.xpm"
#include "../../include/sketch-32.xpm"
#include "../../include/sketch-24.xpm"
#include "../../include/sketch-16.xpm"


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
    buffer.DrawVLine(0, 0, bh - 1, xlRED);
    buffer.DrawHLine(bh - 1, 0, bw - 1, xlGREEN);
    buffer.DrawLine(bw - 1, bh - 1, 0, 0, xlBLUE);
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

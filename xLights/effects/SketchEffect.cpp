#include "SketchEffect.h"
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

void SketchEffect::Render( Effect* effect, SettingsMap& settings, RenderBuffer& buffer )
{

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

}

std::list<std::string> SketchEffect::CheckEffectSettings( const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache )
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

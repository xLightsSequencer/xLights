#include "GalaxyEffect.h"
#include "GalaxyPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

GalaxyEffect::GalaxyEffect(int id) : RenderableEffect(id, "Galaxy")
{
    //ctor
}

GalaxyEffect::~GalaxyEffect()
{
    //dtor
}


wxPanel *GalaxyEffect::CreatePanel(wxWindow *parent) {
    return new GalaxyPanel(parent);
}


void GalaxyEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

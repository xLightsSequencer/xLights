#include "GarlandsEffect.h"
#include "GarlandsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

GarlandsEffect::GarlandsEffect(int id) : RenderableEffect(id, "Garlands")
{
    //ctor
}

GarlandsEffect::~GarlandsEffect()
{
    //dtor
}

wxPanel *GarlandsEffect::CreatePanel(wxWindow *parent) {
    return new GarlandsPanel(parent);
}

void GarlandsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

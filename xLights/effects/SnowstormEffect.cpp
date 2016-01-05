#include "SnowstormEffect.h"
#include "SnowstormPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SnowstormEffect::SnowstormEffect(int id) : RenderableEffect(id, "Snowstorm")
{
    //ctor
}

SnowstormEffect::~SnowstormEffect()
{
    //dtor
}

wxPanel *SnowstormEffect::CreatePanel(wxWindow *parent) {
    return new SnowstormPanel(parent);
}

void SnowstormEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

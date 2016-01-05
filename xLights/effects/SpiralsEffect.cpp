#include "SpiralsEffect.h"
#include "SpiralsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SpiralsEffect::SpiralsEffect(int id) : RenderableEffect(id, "Spirals")
{
    //ctor
}

SpiralsEffect::~SpiralsEffect()
{
    //dtor
}
wxPanel *SpiralsEffect::CreatePanel(wxWindow *parent) {
    return new SpiralsPanel(parent);
}

void SpiralsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

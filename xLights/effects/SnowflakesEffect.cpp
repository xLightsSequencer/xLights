#include "SnowflakesEffect.h"
#include "SnowflakesPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SnowflakesEffect::SnowflakesEffect(int id) : RenderableEffect(id, "Snowflakes")
{
    //ctor
}

SnowflakesEffect::~SnowflakesEffect()
{
    //dtor
}
wxPanel *SnowflakesEffect::CreatePanel(wxWindow *parent) {
    return new SnowflakesPanel(parent);
}

void SnowflakesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

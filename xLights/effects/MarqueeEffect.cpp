#include "MarqueeEffect.h"
#include "MarqueePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


MarqueeEffect::MarqueeEffect(int id) : RenderableEffect(id, "Marquee")
{
    //ctor
}

MarqueeEffect::~MarqueeEffect()
{
    //dtor
}


wxPanel *MarqueeEffect::CreatePanel(wxWindow *parent) {
    return new MarqueePanel(parent);
}


void MarqueeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

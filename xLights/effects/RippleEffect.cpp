#include "RippleEffect.h"
#include "RipplePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

RippleEffect::RippleEffect(int id) : RenderableEffect(id, "Ripple")
{
    //ctor
}

RippleEffect::~RippleEffect()
{
    //dtor
}
wxPanel *RippleEffect::CreatePanel(wxWindow *parent) {
    return new RipplePanel(parent);
}

void RippleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

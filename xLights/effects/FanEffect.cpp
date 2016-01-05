#include "FanEffect.h"
#include "FanPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

FanEffect::FanEffect(int id) : RenderableEffect(id, "Fan")
{
    //ctor
}

FanEffect::~FanEffect()
{
    //dtor
}

wxPanel *FanEffect::CreatePanel(wxWindow *parent) {
    return new FanPanel(parent);
}

void FanEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

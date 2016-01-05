#include "FireEffect.h"
#include "FirePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

FireEffect::FireEffect(int id) : RenderableEffect(id, "Fire")
{
    //ctor
}

FireEffect::~FireEffect()
{
    //dtor
}

wxPanel *FireEffect::CreatePanel(wxWindow *parent) {
    return new FirePanel(parent);
}

void FireEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

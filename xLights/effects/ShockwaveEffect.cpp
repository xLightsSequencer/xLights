#include "ShockwaveEffect.h"
#include "ShockwavePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

ShockwaveEffect::ShockwaveEffect(int id) : RenderableEffect(id, "Shockwave")
{
    //ctor
}

ShockwaveEffect::~ShockwaveEffect()
{
    //dtor
}

wxPanel *ShockwaveEffect::CreatePanel(wxWindow *parent) {
    return new ShockwavePanel(parent);
}

void ShockwaveEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

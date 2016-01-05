#include "MorphEffect.h"
#include "MorphPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


MorphEffect::MorphEffect(int id) : RenderableEffect(id, "Morph")
{
    //ctor
}

MorphEffect::~MorphEffect()
{
    //dtor
}

wxPanel *MorphEffect::CreatePanel(wxWindow *parent) {
    return new MorphPanel(parent);
}

void MorphEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

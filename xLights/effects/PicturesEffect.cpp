#include "PicturesEffect.h"
#include "PicturesPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


PicturesEffect::PicturesEffect(int id) : RenderableEffect(id, "Pictures")
{
    //ctor
}

PicturesEffect::~PicturesEffect()
{
    //dtor
}

wxPanel *PicturesEffect::CreatePanel(wxWindow *parent) {
    return new PicturesPanel(parent);
}

void PicturesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

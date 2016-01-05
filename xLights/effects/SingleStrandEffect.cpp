#include "SingleStrandEffect.h"
#include "SingleStrandPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

SingleStrandEffect::SingleStrandEffect(int id) : RenderableEffect(id, "Single Strand")
{
    //ctor
}

SingleStrandEffect::~SingleStrandEffect()
{
    //dtor
}

wxPanel *SingleStrandEffect::CreatePanel(wxWindow *parent) {
    return new SingleStrandPanel(parent);
}

void SingleStrandEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

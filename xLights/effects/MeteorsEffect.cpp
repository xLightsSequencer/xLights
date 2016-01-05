#include "MeteorsEffect.h"
#include "MeteorsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


MeteorsEffect::MeteorsEffect(int id) : RenderableEffect(id, "Meteors")
{
    //ctor
}

MeteorsEffect::~MeteorsEffect()
{
    //dtor
}
wxPanel *MeteorsEffect::CreatePanel(wxWindow *parent) {
    return new MeteorsPanel(parent);
}
void MeteorsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

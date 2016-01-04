#include "LightningEffect.h"
#include "LightningPanel.h"

LightningEffect::LightningEffect(int id) : RenderableEffect(id, "Lightning")
{
    //ctor
}

LightningEffect::~LightningEffect()
{
    //dtor
}

wxPanel *LightningEffect::CreatePanel(wxWindow *parent) {
    return new LightningPanel(parent);
}
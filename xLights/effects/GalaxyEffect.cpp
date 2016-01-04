#include "GalaxyEffect.h"
#include "GalaxyPanel.h"

GalaxyEffect::GalaxyEffect(int id) : RenderableEffect(id, "Galaxy")
{
    //ctor
}

GalaxyEffect::~GalaxyEffect()
{
    //dtor
}


wxPanel *GalaxyEffect::CreatePanel(wxWindow *parent) {
    return new GalaxyPanel(parent);
}
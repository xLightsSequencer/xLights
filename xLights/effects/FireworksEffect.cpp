#include "FireworksEffect.h"
#include "FireworksPanel.h"

FireworksEffect::FireworksEffect(int id) : RenderableEffect(id, "Fireworks")
{
    //ctor
}

FireworksEffect::~FireworksEffect()
{
    //dtor
}

wxPanel *FireworksEffect::CreatePanel(wxWindow *parent) {
    return new FireworksPanel(parent);
}
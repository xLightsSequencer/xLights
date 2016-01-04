#include "FireEffect.h"
#include "FirePanel.h"

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
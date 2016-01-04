#include "LifeEffect.h"
#include "LifePanel.h"
LifeEffect::LifeEffect(int id) : RenderableEffect(id, "Life")
{
    //ctor
}

LifeEffect::~LifeEffect()
{
    //dtor
}


wxPanel *LifeEffect::CreatePanel(wxWindow *parent) {
    return new LifePanel(parent);
}
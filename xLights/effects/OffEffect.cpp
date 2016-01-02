#include "OffEffect.h"
#include "OffPanel.h"

OffEffect::OffEffect(int i) : RenderableEffect(i, "Off")
{
    //ctor
}

OffEffect::~OffEffect()
{
    //dtor
}

wxPanel *OffEffect::CreatePanel(wxWindow *parent) {
    return new OffPanel(parent);
}

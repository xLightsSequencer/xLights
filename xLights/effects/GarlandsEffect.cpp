#include "GarlandsEffect.h"
#include "GarlandsPanel.h"

GarlandsEffect::GarlandsEffect(int id) : RenderableEffect(id, "Garlands")
{
    //ctor
}

GarlandsEffect::~GarlandsEffect()
{
    //dtor
}

wxPanel *GarlandsEffect::CreatePanel(wxWindow *parent) {
    return new GarlandsPanel(parent);
}
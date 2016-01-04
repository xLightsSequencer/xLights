#include "SnowstormEffect.h"
#include "SnowstormPanel.h"

SnowstormEffect::SnowstormEffect(int id) : RenderableEffect(id, "Snowstorm")
{
    //ctor
}

SnowstormEffect::~SnowstormEffect()
{
    //dtor
}

wxPanel *SnowstormEffect::CreatePanel(wxWindow *parent) {
    return new SnowstormPanel(parent);
}
#include "SpiralsEffect.h"
#include "SpiralsPanel.h"

SpiralsEffect::SpiralsEffect(int id) : RenderableEffect(id, "Spirals")
{
    //ctor
}

SpiralsEffect::~SpiralsEffect()
{
    //dtor
}
wxPanel *SpiralsEffect::CreatePanel(wxWindow *parent) {
    return new SpiralsPanel(parent);
}
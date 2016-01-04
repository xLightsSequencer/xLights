#include "CirclesEffect.h"
#include "CirclesPanel.h"

CirclesEffect::CirclesEffect(int i) : RenderableEffect(i, "Circles")
{
    //ctor
}

CirclesEffect::~CirclesEffect()
{
    //dtor
}
wxPanel *CirclesEffect::CreatePanel(wxWindow *parent) {
    return new CirclesPanel(parent);
}
#include "TwinkleEffect.h"
#include "TwinklePanel.h"

TwinkleEffect::TwinkleEffect(int id) : RenderableEffect(id, "Twinkle")
{
    //ctor
}

TwinkleEffect::~TwinkleEffect()
{
    //dtor
}
wxPanel *TwinkleEffect::CreatePanel(wxWindow *parent) {
    return new TwinklePanel(parent);
}
#include "ButterflyEffect.h"

#include "ButterflyPanel.h"

ButterflyEffect::ButterflyEffect(int i) : RenderableEffect(i, "Butterfly")
{
    //ctor
}

ButterflyEffect::~ButterflyEffect()
{
    //dtor
}


wxPanel *ButterflyEffect::CreatePanel(wxWindow *parent) {
    return new ButterflyPanel(parent);
}
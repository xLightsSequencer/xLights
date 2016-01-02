#include "BarsEffect.h"
#include "BarsPanel.h"

BarsEffect::BarsEffect(int i) : RenderableEffect(i, "Bars")
{
    //ctor
}

BarsEffect::~BarsEffect()
{
    //dtor
}

wxPanel *BarsEffect::CreatePanel(wxWindow *parent) {
    return new BarsPanel(parent);
}

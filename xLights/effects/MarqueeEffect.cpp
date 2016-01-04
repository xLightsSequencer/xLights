#include "MarqueeEffect.h"
#include "MarqueePanel.h"

MarqueeEffect::MarqueeEffect(int id) : RenderableEffect(id, "Marquee")
{
    //ctor
}

MarqueeEffect::~MarqueeEffect()
{
    //dtor
}


wxPanel *MarqueeEffect::CreatePanel(wxWindow *parent) {
    return new MarqueePanel(parent);
}
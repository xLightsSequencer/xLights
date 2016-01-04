#include "RippleEffect.h"
#include "RipplePanel.h"
RippleEffect::RippleEffect(int id) : RenderableEffect(id, "Ripple")
{
    //ctor
}

RippleEffect::~RippleEffect()
{
    //dtor
}
wxPanel *RippleEffect::CreatePanel(wxWindow *parent) {
    return new RipplePanel(parent);
}
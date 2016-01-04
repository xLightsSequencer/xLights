#include "PinwheelEffect.h"
#include "PinwheelPanel.h"

PinwheelEffect::PinwheelEffect(int id) : RenderableEffect(id, "Pinwheel")
{
    //ctor
}

PinwheelEffect::~PinwheelEffect()
{
    //dtor
}
wxPanel *PinwheelEffect::CreatePanel(wxWindow *parent) {
    return new PinwheelPanel(parent);
}
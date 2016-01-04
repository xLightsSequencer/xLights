#include "DMXEffect.h"
#include "DMXPanel.h"

DMXEffect::DMXEffect(int id) : RenderableEffect(id, "DMX")
{
    //ctor
}

DMXEffect::~DMXEffect()
{
    //dtor
}

wxPanel *DMXEffect::CreatePanel(wxWindow *parent) {
    return new DMXPanel(parent);
}
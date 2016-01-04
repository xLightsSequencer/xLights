#include "PlasmaEffect.h"
#include "PlasmaPanel.h"

PlasmaEffect::PlasmaEffect(int id) : RenderableEffect(id, "Plasma")
{
    //ctor
}

PlasmaEffect::~PlasmaEffect()
{
    //dtor
}
wxPanel *PlasmaEffect::CreatePanel(wxWindow *parent) {
    return new PlasmaPanel(parent);
}
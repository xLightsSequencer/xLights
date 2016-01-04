#include "PicturesEffect.h"
#include "PicturesPanel.h"
PicturesEffect::PicturesEffect(int id) : RenderableEffect(id, "Pictures")
{
    //ctor
}

PicturesEffect::~PicturesEffect()
{
    //dtor
}

wxPanel *PicturesEffect::CreatePanel(wxWindow *parent) {
    return new PicturesPanel(parent);
}
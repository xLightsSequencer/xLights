#include "MorphEffect.h"
#include "MorphPanel.h"

MorphEffect::MorphEffect(int id) : RenderableEffect(id, "Morph")
{
    //ctor
}

MorphEffect::~MorphEffect()
{
    //dtor
}

wxPanel *MorphEffect::CreatePanel(wxWindow *parent) {
    return new MorphPanel(parent);
}
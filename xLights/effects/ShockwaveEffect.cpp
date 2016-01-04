#include "ShockwaveEffect.h"
#include "ShockwavePanel.h"

ShockwaveEffect::ShockwaveEffect(int id) : RenderableEffect(id, "Shockwave")
{
    //ctor
}

ShockwaveEffect::~ShockwaveEffect()
{
    //dtor
}

wxPanel *ShockwaveEffect::CreatePanel(wxWindow *parent) {
    return new ShockwavePanel(parent);
}
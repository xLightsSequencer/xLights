#include "SingleStrandEffect.h"
#include "SingleStrandPanel.h"

SingleStrandEffect::SingleStrandEffect(int id) : RenderableEffect(id, "Single Strand")
{
    //ctor
}

SingleStrandEffect::~SingleStrandEffect()
{
    //dtor
}

wxPanel *SingleStrandEffect::CreatePanel(wxWindow *parent) {
    return new SingleStrandPanel(parent);
}
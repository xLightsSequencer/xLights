#include "FanEffect.h"
#include "FanPanel.h"

FanEffect::FanEffect(int id) : RenderableEffect(id, "Fan")
{
    //ctor
}

FanEffect::~FanEffect()
{
    //dtor
}

wxPanel *FanEffect::CreatePanel(wxWindow *parent) {
    return new FanPanel(parent);
}
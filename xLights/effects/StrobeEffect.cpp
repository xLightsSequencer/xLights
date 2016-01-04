#include "StrobeEffect.h"
#include "StrobePanel.h"

StrobeEffect::StrobeEffect(int id) : RenderableEffect(id, "Strobe")
{
    //ctor
}

StrobeEffect::~StrobeEffect()
{
    //dtor
}

wxPanel *StrobeEffect::CreatePanel(wxWindow *parent) {
    return new StrobePanel(parent);
}
#include "SpirographEffect.h"
#include "SpirographPanel.h"

SpirographEffect::SpirographEffect(int id) : RenderableEffect(id, "Spirograph")
{
    //ctor
}

SpirographEffect::~SpirographEffect()
{
    //dtor
}
wxPanel *SpirographEffect::CreatePanel(wxWindow *parent) {
    return new SpirographPanel(parent);
}
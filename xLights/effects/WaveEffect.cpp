#include "WaveEffect.h"
#include "WavePanel.h"

WaveEffect::WaveEffect(int id) : RenderableEffect(id, "Wave")
{
    //ctor
}

WaveEffect::~WaveEffect()
{
    //dtor
}
wxPanel *WaveEffect::CreatePanel(wxWindow *parent) {
    return new WavePanel(parent);
}
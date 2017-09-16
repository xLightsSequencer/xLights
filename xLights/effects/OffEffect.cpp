#include "OffEffect.h"
#include "OffPanel.h"

#include "../RenderBuffer.h"

#include "../../include/Off.xpm"

OffEffect::OffEffect(int i) : RenderableEffect(i, "Off", Off, Off, Off, Off, Off)
{
    //ctor
}

OffEffect::~OffEffect()
{
    //dtor
}

wxPanel *OffEffect::CreatePanel(wxWindow *parent) {
    return new OffPanel(parent);
}

void OffEffect::Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) {
    int x,y;
    //  Every Node, every frame set to BLACK
    for (x = 0; x < buffer.BufferWi; x++) {
        for (y = 0; y < buffer.BufferHt; y++) {
            buffer.SetPixel(x, y, xlBLACK);
        }
    }
}
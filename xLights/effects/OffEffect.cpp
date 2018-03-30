#include "OffEffect.h"
#include "OffPanel.h"
#include "../RenderBuffer.h"
#include <sstream>

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

void OffEffect::SetDefaultParameters(Model *cls) {
    OffPanel *p = (OffPanel*)panel;
    p->CheckBox_Transparent->SetValue(false);
}

void OffEffect::Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) {

    // dont change any pixels at all if we are transparent
    if (settings.GetBool("CHECKBOX_Off_Transparent", false)) return;

    //  Every Node, every frame set to BLACK
    for (int x = 0; x < buffer.BufferWi; x++) {
        for (int y = 0; y < buffer.BufferHt; y++) {
            buffer.SetPixel(x, y, xlBLACK);
        }
    }
}

std::string OffEffect::GetEffectString()
{
    OffPanel *p = (OffPanel*)panel;
    std::stringstream ret;
    if (p->CheckBox_Transparent->GetValue()) {
        ret << "E_CHECKBOX_Off_Transparent=1,";
    }
    return ret.str();
}

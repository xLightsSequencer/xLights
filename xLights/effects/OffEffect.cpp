/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <sstream>

#include "OffEffect.h"
#include "OffPanel.h"
#include "../RenderBuffer.h"
#include "UtilFunctions.h"
#include "models/Model.h"

#include "../../include/Off.xpm"

OffEffect::OffEffect(int i) : RenderableEffect(i, "Off", Off, Off, Off, Off, Off)
{
    //ctor
}

OffEffect::~OffEffect()
{
    //dtor
}

std::list<std::string> OffEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    // if persistent is on then canvas/off transparent cant be checked
    if (settings.Get("B_CHECKBOX_OverlayBkg", "0") == "0")
    {
        if (settings.Get("T_CHECKBOX_Canvas", "0") == "1" &&
            settings.Get("E_CHECKBOX_Off_Transparent", "0") == "0")
        {
            res.push_back(wxString::Format("    WARN: Canvas mode enabled on a off effect but effect is not transparent. This does nothing and slows down rendering. Effect: Off, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
        else if (settings.Get("T_CHECKBOX_Canvas", "0") == "0" &&
            settings.Get("E_CHECKBOX_Off_Transparent", "0") == "1")
        {
            res.push_back(wxString::Format("    WARN: Canvas mode not enabled on a off effect and effect is transparent. This does not do anything useful. Effect: Off, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    return res;
}

wxPanel *OffEffect::CreatePanel(wxWindow *parent) {
    return new OffPanel(parent);
}

void OffEffect::SetDefaultParameters() {
    OffPanel *p = (OffPanel*)panel;
    p->CheckBox_Transparent->SetValue(false);
}

void OffEffect::Render(Effect* effect, SettingsMap& settings, RenderBuffer& buffer)
{
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
    OffPanel* p = (OffPanel*)panel;
    std::stringstream ret;
    if (p->CheckBox_Transparent->GetValue()) {
        ret << "E_CHECKBOX_Off_Transparent=1,";
    }
    return ret.str();
}

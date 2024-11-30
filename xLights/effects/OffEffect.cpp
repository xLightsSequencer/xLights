/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
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
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    // if persistent is on then canvas/off transparent cant be checked
    if (settings.Get("B_CHECKBOX_OverlayBkg", "0") == "0") {
        if (settings.Get("T_CHECKBOX_Canvas", "0") == "1" &&
            settings.Get("E_CHOICE_Off_Style", "Black") == "Black") {
            res.push_back(wxString::Format("    WARN: Canvas mode enabled on a off effect but effect is not transparent. This does nothing and slows down rendering. Effect: Off, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        } else if (settings.Get("T_CHECKBOX_Canvas", "0") == "0" &&
            settings.Get("E_CHOICE_Off_Style", "Black") != "Black") {
            res.push_back(wxString::Format("    WARN: Canvas mode not enabled on a off effect and effect is transparent. This does not do anything useful. Effect: Off, Model: %s, Start %s", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    return res;
}

xlEffectPanel *OffEffect::CreatePanel(wxWindow *parent) {
    return new OffPanel(parent);
}

void OffEffect::SetDefaultParameters() {
    OffPanel *p = (OffPanel*)panel;
    p->OffStyleChoice->SetSelection(0);
}

bool OffEffect::needToAdjustSettings(const std::string& version) {
    if (IsVersionOlder("2024.11", version)) {
        return true;
    }
    return RenderableEffect::needToAdjustSettings(version);
}
void OffEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    RenderableEffect::adjustSettings(version, effect, removeDefaults);
    std::string i = effect->GetSettings().Get("E_CHECKBOX_Off_Transparent", "");
    if (i != "") {
        effect->GetSettings().erase("E_CHECKBOX_Off_Transparent");
        if (i == "1") {
            i = effect->GetSettings().Get("T_CHECKBOX_Canvas", "");
            if (i == "1") {
                // old canvas + transparent OFF effects would be black as canvas would not
                // have any alpha channels.
                effect->GetSettings()["E_CHOICE_Off_Style"] = "Transparent -> Black";
            } else {
                effect->GetSettings()["E_CHOICE_Off_Style"] = "Transparent";
            }
        }
    }
}


void OffEffect::Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
{
    std::string style = settings.Get("CHOICE_Off_Style", "Black");
    if (style == "Transparent") {
        // dont change any pixels at all if we are transparent
        return;
    } else if (style == "Black") {
        //  Every Node, every frame set to BLACK
        buffer.Fill(xlBLACK);
    } else if (style == "Black -> Transparent") {
        for (size_t x = 0; x < buffer.GetPixelCount(); ++x) {
            if (buffer.GetPixels()[x] == xlBLACK) {
                buffer.GetPixels()[x] = xlCLEAR;
            }
        }
    } else if (style == "Transparent -> Black") {
        for (size_t x = 0; x < buffer.GetPixelCount(); ++x) {
            if (buffer.GetPixels()[x] == xlCLEAR) {
                buffer.GetPixels()[x] = xlBLACK;
            }
        }
    }

}

wxString OffEffect::GetEffectString()
{
    OffPanel* p = (OffPanel*)panel;
    std::string style = p->OffStyleChoice->GetStringSelection().ToStdString();
    if (style != "Black") {
        return "E_CHOICE_Off_Style=" + style + ",";
    }
    return "";
}

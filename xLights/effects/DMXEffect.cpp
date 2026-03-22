/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DMXEffect.h"
#include "../ValueCurve.h"

#include <cstdlib>
#include <format>
#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../models/ModelGroup.h"

#include "../../include/dmx-16.xpm"
#include "../../include/dmx-24.xpm"
#include "../../include/dmx-32.xpm"
#include "../../include/dmx-48.xpm"
#include "../../include/dmx-64.xpm"
#include "UtilFunctions.h"

DMXEffect::DMXEffect(int id) : RenderableEffect(id, "DMX", dmx_16, dmx_24, dmx_32, dmx_48, dmx_64)
{
    //ctor
}

DMXEffect::~DMXEffect()
{
    //dtor
}

void DMXEffect::RemapSelectedDMXEffectValues(Effect* effect, const std::vector<std::tuple<int, int, float, int, wxString>>& dmxmappings) const {
    SettingsMap &settings = effect->GetSettings();
    SettingsMap const oldSettings = settings;
    for (auto const& [fromi, toi, scale, offset, inv] : dmxmappings) {
        auto const froms = std::format("{}", fromi);
        auto const tos = std::format("{}", toi);
        auto const slider = oldSettings.Get("E_SLIDER_DMX" + froms, "NOTTHERE");
        auto const vc = oldSettings.Get("E_VALUECURVE_DMX" + froms, "NOTTHERE");
        auto invert_chbx = oldSettings.Get("E_CHECKBOX_INVDMX" + froms, "NOTTHERE");

        if (slider != "NOTTHERE") {
            int const new_value = ((float)std::stoi(slider) * scale) + offset;
            settings["E_SLIDER_DMX" + tos] = std::to_string( new_value);
        } else {
            settings.erase("E_SLIDER_DMX" + tos);
        }

        if (vc != "NOTTHERE") {
            ValueCurve valc;
            valc.SetDivisor(1.0F);
            valc.SetLimits(DMX_MIN, DMX_MAX);
            valc.Deserialise(vc);
            valc.ScaleAndOffsetValues(scale, offset);
            settings["E_VALUECURVE_DMX" + tos] = valc.Serialise();
        } else {
            settings.erase("E_VALUECURVE_DMX" + tos);
        }

        if (inv == "Check") {
            invert_chbx = "1";
        } else if (inv == "Uncheck") {
            invert_chbx = "0";
        }

        if (invert_chbx != "NOTTHERE") {
            settings["E_CHECKBOX_INVDMX" + tos] = invert_chbx;
        } else {
            settings.erase("E_CHECKBOX_INVDMX" + tos);
        }
    }
}

static int GetPct(const std::string& val)
{
    int value = std::strtol(val.c_str(), nullptr, 10);
    return (value * 100) / 255;
}

void DMXEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    if (IsVersionOlder("2016.39", version))
    {
        if (settings.GetBool("E_CHECKBOX_Use_Dmx_Ramps")) {
            settings["E_VALUECURVE_DMX1"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX1|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX1"]), GetPct(settings["E_SLIDER_DMX1_Ramp"]));
            settings["E_VALUECURVE_DMX2"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX2|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX2"]), GetPct(settings["E_SLIDER_DMX2_Ramp"]));
            settings["E_VALUECURVE_DMX3"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX3|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX3"]), GetPct(settings["E_SLIDER_DMX3_Ramp"]));
            settings["E_VALUECURVE_DMX4"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX4|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX4"]), GetPct(settings["E_SLIDER_DMX4_Ramp"]));
            settings["E_VALUECURVE_DMX5"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX5|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX5"]), GetPct(settings["E_SLIDER_DMX5_Ramp"]));
            settings["E_VALUECURVE_DMX6"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX6|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX6"]), GetPct(settings["E_SLIDER_DMX6_Ramp"]));
            settings["E_VALUECURVE_DMX7"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX7|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX7"]), GetPct(settings["E_SLIDER_DMX7_Ramp"]));
            settings["E_VALUECURVE_DMX8"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX8|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX8"]), GetPct(settings["E_SLIDER_DMX8_Ramp"]));
            settings["E_VALUECURVE_DMX9"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX9|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX9"]), GetPct(settings["E_SLIDER_DMX9_Ramp"]));
            settings["E_VALUECURVE_DMX10"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX10|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX10"]), GetPct(settings["E_SLIDER_DMX10_Ramp"]));
            settings["E_VALUECURVE_DMX11"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX11|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX11"]), GetPct(settings["E_SLIDER_DMX11_Ramp"]));
            settings["E_VALUECURVE_DMX12"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX12|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX12"]), GetPct(settings["E_SLIDER_DMX12_Ramp"]));
            settings["E_VALUECURVE_DMX13"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX13|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX13"]), GetPct(settings["E_SLIDER_DMX13_Ramp"]));
            settings["E_VALUECURVE_DMX14"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX14|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX14"]), GetPct(settings["E_SLIDER_DMX14_Ramp"]));
            settings["E_VALUECURVE_DMX15"] = std::format("Active=TRUE|Id=ID_VALUECURVE_DMX15|Type=Ramp|Min=0.00|Max=255.00|P1={}|P2={}|RV=TRUE|", GetPct(settings["E_SLIDER_DMX15"]), GetPct(settings["E_SLIDER_DMX15_Ramp"]));
            settings["E_VALUECURVE_DMX16"] = "Active=TRUE|Id=ID_VALUECURVE_DMX16|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|RV=TRUE|";
            settings["E_VALUECURVE_DMX17"] = "Active=TRUE|Id=ID_VALUECURVE_DMX17|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|RV=TRUE|";
            settings["E_VALUECURVE_DMX18"] = "Active=TRUE|Id=ID_VALUECURVE_DMX18|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|RV=TRUE|";
            settings.erase("E_SLIDER_DMX1");
            settings.erase("E_SLIDER_DMX2");
            settings.erase("E_SLIDER_DMX3");
            settings.erase("E_SLIDER_DMX4");
            settings.erase("E_SLIDER_DMX5");
            settings.erase("E_SLIDER_DMX6");
            settings.erase("E_SLIDER_DMX7");
            settings.erase("E_SLIDER_DMX8");
            settings.erase("E_SLIDER_DMX9");
            settings.erase("E_SLIDER_DMX10");
            settings.erase("E_SLIDER_DMX11");
            settings.erase("E_SLIDER_DMX12");
            settings.erase("E_SLIDER_DMX13");
            settings.erase("E_SLIDER_DMX14");
            settings.erase("E_SLIDER_DMX15");
        } else {
            settings["E_SLIDER_DMX16"] = "0";
            settings["E_SLIDER_DMX17"] = "0";
            settings["E_SLIDER_DMX18"] = "0";
        }
        settings.erase("E_CHOICE_Num_Dmx_Channels");
        settings.erase("E_CHECKBOX_Use_Dmx_Ramps");
        settings.erase("E_SLIDER_DMX1_Ramp");
        settings.erase("E_SLIDER_DMX2_Ramp");
        settings.erase("E_SLIDER_DMX3_Ramp");
        settings.erase("E_SLIDER_DMX4_Ramp");
        settings.erase("E_SLIDER_DMX5_Ramp");
        settings.erase("E_SLIDER_DMX6_Ramp");
        settings.erase("E_SLIDER_DMX7_Ramp");
        settings.erase("E_SLIDER_DMX8_Ramp");
        settings.erase("E_SLIDER_DMX9_Ramp");
        settings.erase("E_SLIDER_DMX10_Ramp");
        settings.erase("E_SLIDER_DMX11_Ramp");
        settings.erase("E_SLIDER_DMX12_Ramp");
        settings.erase("E_SLIDER_DMX13_Ramp");
        settings.erase("E_SLIDER_DMX14_Ramp");
        settings.erase("E_SLIDER_DMX15_Ramp");
    }
}

bool DMXEffect::SetDMXSinglColorPixel(int chan, int num_channels, const SettingsMap &SettingsMap, double eff_pos, xlColor& color, RenderBuffer &buffer)
{
    if( num_channels >= chan ) {
        std::string const name = std::format("DMX{}", chan);
        int value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

        if (SettingsMap.GetBool("CHECKBOX_INV" + name, false))
        {
            value = 255 - value;
        }

        color.red = value;
        color.green = value;
        color.blue = value;
        buffer.SetPixel(chan-1, 0, color, false, false, true);
        return false;
    } 
    return true;
}

void DMXEffect::SetColorBasedOnStringType(int value, int slot, xlColor& color, const std::string& string_type)
{
    const char which_color = string_type[slot-1];
    if( which_color == 'R' ) {
        color.red = value;
    } else if( which_color == 'G' ) {
        color.green = value;
    } else if( which_color == 'B' ) {
        color.blue = value;
    }
}

bool DMXEffect::SetDMXRGBNode(int node, int num_channels, const SettingsMap &SettingsMap, double eff_pos, xlColor& color, RenderBuffer &buffer, const std::string& string_type)
{
    bool return_val = false;
    color = xlBLACK;
    int const base_chan = ((node - 1) * 3 + 1);
    if( num_channels >= base_chan || buffer.BufferWi < node) {
        std::string name = std::format("DMX{}", base_chan);
        int value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

        if (SettingsMap.GetBool("CHECKBOX_INV" + name, false)) {
            value = 255 - value;
        }

        SetColorBasedOnStringType(value, 1, color, string_type);
        if( num_channels >= base_chan + 1 ) {
            name = std::format("DMX{}", base_chan+1);
            value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

            if (SettingsMap.GetBool("CHECKBOX_INV" + name, false)) {
                value = 255 - value;
            }

            SetColorBasedOnStringType(value, 2, color, string_type);
            if( num_channels >= base_chan + 2 ) {
                name = std::format("DMX{}", base_chan + 2);
                value = GetValueCurveInt(name, 0, SettingsMap, eff_pos, DMX_MIN, DMX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

                if (SettingsMap.GetBool("CHECKBOX_INV" + name, false)) {
                    value = 255 - value;
                }

                SetColorBasedOnStringType(value, 3, color, string_type);
            } else {
                return_val = true;
            }
        } else {
            return_val = true;
        }
        buffer.SetPixel(node-1, 0, color, false, false, true);
    } else {
        return_val = true;
    }
    return return_val;
}

void DMXEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double const eff_pos = buffer.GetEffectTimeIntervalPosition();

    if (buffer.cur_model.empty()) {
        return;
    }
    const Model* model_info = buffer.GetModel();
    if (model_info == nullptr) {
        return;
    }

    int const num_channels = model_info->GetNumChannels();

    const std::string& string_type = model_info->GetStringType();

    xlColor color = xlBLACK;

    if (StartsWith(string_type, "Single Color")) {
        // handle channels for single color nodes
        for (uint32_t i = 1; i <= DMX_CHANNELS; ++i) {
            if (SetDMXSinglColorPixel(i, num_channels, SettingsMap, eff_pos, color, buffer))
                return;
        }
   } else {
        // handle channels for 3 color nodes
       for (uint32_t i = 1; i <= DMX_CHANNELS / 3; ++i) {
            if (SetDMXRGBNode(i, num_channels, SettingsMap, eff_pos, color, buffer, string_type))
                return;
        }
    }
}


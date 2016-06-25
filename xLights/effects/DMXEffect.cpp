#include "DMXEffect.h"
#include "DMXPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/dmx-16.xpm"
#include "../../include/dmx-24.xpm"
#include "../../include/dmx-32.xpm"
#include "../../include/dmx-48.xpm"
#include "../../include/dmx-64.xpm"


DMXEffect::DMXEffect(int id) : RenderableEffect(id, "DMX", dmx_16, dmx_24, dmx_32, dmx_48, dmx_64)
{
    //ctor
}

DMXEffect::~DMXEffect()
{
    //dtor
}

wxPanel *DMXEffect::CreatePanel(wxWindow *parent) {
    return new DMXPanel(parent);
}

static int GetPct(wxString val)
{
    int value = wxAtoi(val);
    return (value * 100) / 255;
}

void DMXEffect::adjustSettings(const std::string &version, Effect *effect)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect);
    }

    SettingsMap &settings = effect->GetSettings();

    if (IsVersionOlder("2016.39", version))
    {
        if (settings.GetBool("E_CHECKBOX_Use_Dmx_Ramps")) {
            settings["E_VALUECURVE_DMX1"] = wxString::Format("Id=ID_VALUECURVE_DMX1|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX1"]), GetPct(settings["E_SLIDER_DMX1_Ramp"]));
            settings["E_VALUECURVE_DMX2"] = wxString::Format("Id=ID_VALUECURVE_DMX2|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX2"]), GetPct(settings["E_SLIDER_DMX2_Ramp"]));
            settings["E_VALUECURVE_DMX3"] = wxString::Format("Id=ID_VALUECURVE_DMX3|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX3"]), GetPct(settings["E_SLIDER_DMX3_Ramp"]));
            settings["E_VALUECURVE_DMX4"] = wxString::Format("Id=ID_VALUECURVE_DMX4|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX4"]), GetPct(settings["E_SLIDER_DMX4_Ramp"]));
            settings["E_VALUECURVE_DMX5"] = wxString::Format("Id=ID_VALUECURVE_DMX5|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX5"]), GetPct(settings["E_SLIDER_DMX5_Ramp"]));
            settings["E_VALUECURVE_DMX6"] = wxString::Format("Id=ID_VALUECURVE_DMX6|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX6"]), GetPct(settings["E_SLIDER_DMX6_Ramp"]));
            settings["E_VALUECURVE_DMX7"] = wxString::Format("Id=ID_VALUECURVE_DMX7|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX7"]), GetPct(settings["E_SLIDER_DMX7_Ramp"]));
            settings["E_VALUECURVE_DMX8"] = wxString::Format("Id=ID_VALUECURVE_DMX8|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX8"]), GetPct(settings["E_SLIDER_DMX8_Ramp"]));
            settings["E_VALUECURVE_DMX9"] = wxString::Format("Id=ID_VALUECURVE_DMX9|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX9"]), GetPct(settings["E_SLIDER_DMX9_Ramp"]));
            settings["E_VALUECURVE_DMX10"] = wxString::Format("Id=ID_VALUECURVE_DMX10|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX10"]), GetPct(settings["E_SLIDER_DMX10_Ramp"]));
            settings["E_VALUECURVE_DMX11"] = wxString::Format("Id=ID_VALUECURVE_DMX11|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX11"]), GetPct(settings["E_SLIDER_DMX11_Ramp"]));
            settings["E_VALUECURVE_DMX12"] = wxString::Format("Id=ID_VALUECURVE_DMX12|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX12"]), GetPct(settings["E_SLIDER_DMX12_Ramp"]));
            settings["E_VALUECURVE_DMX13"] = wxString::Format("Id=ID_VALUECURVE_DMX13|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX13"]), GetPct(settings["E_SLIDER_DMX13_Ramp"]));
            settings["E_VALUECURVE_DMX14"] = wxString::Format("Id=ID_VALUECURVE_DMX14|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX14"]), GetPct(settings["E_SLIDER_DMX14_Ramp"]));
            settings["E_VALUECURVE_DMX15"] = wxString::Format("Id=ID_VALUECURVE_DMX15|Type=Ramp|Min=0.00|Max=255.00|P1=%d|P2=%d|", GetPct(settings["E_SLIDER_DMX15"]), GetPct(settings["E_SLIDER_DMX15_Ramp"]));
            settings["E_VALUECURVE_DMX16"] = "Id=ID_VALUECURVE_DMX16|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|";
            settings["E_VALUECURVE_DMX17"] = "Id=ID_VALUECURVE_DMX17|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|";
            settings["E_VALUECURVE_DMX18"] = "Id=ID_VALUECURVE_DMX18|Type=Ramp|Min=0.00|Max=255.00|P1=0|P2=0|";
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

void DMXEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    wxArrayString parts = wxSplit(SettingsMap.Get("CHOICE_Num_Dmx_Channels", "Use 1 Channel"), ' ');
    int channels = wxAtoi(parts[1]);

    xlColor color = xlBLACK;

    color.red = GetValueCurveInt("DMX1", 0, SettingsMap, eff_pos);
    if( channels >= 2 ) {
        color.green = GetValueCurveInt("DMX2", 0, SettingsMap, eff_pos);
    }
    if( channels >= 3 ) {
        color.blue = GetValueCurveInt("DMX3", 0, SettingsMap, eff_pos);
    }
    buffer.SetPixel(0, 0, color);

    if( channels < 4 || buffer.BufferWi < 2) return;
    color = xlBLACK;
    color.red = GetValueCurveInt("DMX4", 0, SettingsMap, eff_pos);
    if( channels >= 5 ) {
        color.green = GetValueCurveInt("DMX5", 0, SettingsMap, eff_pos);
    }
    if( channels >= 6 ) {
        color.blue = GetValueCurveInt("DMX6", 0, SettingsMap, eff_pos);
    }
    buffer.SetPixel(1, 0, color);

    if( channels < 7 || buffer.BufferWi < 3) return;
    color = xlBLACK;
    color.red = GetValueCurveInt("DMX7", 0, SettingsMap, eff_pos);
    if( channels >= 8 ) {
        color.green = GetValueCurveInt("DMX8", 0, SettingsMap, eff_pos);
    }
    if( channels >= 9 ) {
        color.blue = GetValueCurveInt("DMX9", 0, SettingsMap, eff_pos);
    }
    buffer.SetPixel(2, 0, color);

    if( channels < 10 || buffer.BufferWi < 4) return;
    color = xlBLACK;
    color.red = GetValueCurveInt("DMX10", 0, SettingsMap, eff_pos);
    if( channels >= 11 ) {
        color.green = GetValueCurveInt("DMX11", 0, SettingsMap, eff_pos);
    }
    if( channels >= 12 ) {
        color.blue = GetValueCurveInt("DMX12", 0, SettingsMap, eff_pos);
    }
    buffer.SetPixel(3, 0, color);

    if( channels < 13 || buffer.BufferWi < 5) return;
    color = xlBLACK;
    color.red = GetValueCurveInt("DMX13", 0, SettingsMap, eff_pos);
    if( channels >= 14 ) {
        color.green = GetValueCurveInt("DMX14", 0, SettingsMap, eff_pos);
    }
    if( channels >= 15 ) {
        color.blue = GetValueCurveInt("DMX15", 0, SettingsMap, eff_pos);
    }
    buffer.SetPixel(4, 0, color);

    if( channels < 16 || buffer.BufferWi < 6) return;
    color = xlBLACK;
    color.red = GetValueCurveInt("DMX16", 0, SettingsMap, eff_pos);
    if( channels >= 17 ) {
        color.green = GetValueCurveInt("DMX17", 0, SettingsMap, eff_pos);
    }
    if( channels >= 18 ) {
        color.blue = GetValueCurveInt("DMX18", 0, SettingsMap, eff_pos);
    }
    buffer.SetPixel(5, 0, color);
}

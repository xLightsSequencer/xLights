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


static int GetRampedValue(int start_val, int end_val, double eff_pos, bool use_ramps)
{
    if( !use_ramps || (start_val == end_val) ) return start_val;
    if( end_val > start_val )
    {
        return start_val + (int)((double)(end_val-start_val)*eff_pos);
    }
    else
    {
        return start_val - (int)((double)(start_val-end_val)*eff_pos);
    }
}


void DMXEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    bool use_ramps = SettingsMap.GetBool("CHECKBOX_Use_Dmx_Ramps");
    wxArrayString parts = wxSplit(SettingsMap.Get("CHOICE_Num_Dmx_Channels", "Use 1 Channel"), ' ');
    int channels = wxAtoi(parts[1]);
    
    xlColor color = xlBLACK;
    
    
    color.red = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX1", 0), SettingsMap.GetInt("SLIDER_DMX1_Ramp", 0), eff_pos, use_ramps);
    if( channels >= 2 ) {
        color.green = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX2", 0), SettingsMap.GetInt("SLIDER_DMX2_Ramp", 0), eff_pos, use_ramps);
    }
    if( channels >= 3 ) {
        color.blue = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX3", 0), SettingsMap.GetInt("SLIDER_DMX3_Ramp", 0), eff_pos, use_ramps);
    }
    buffer.SetPixel(0, 0, color);
    
    if( channels < 4 || buffer.BufferWi < 2) return;
    color = xlBLACK;
    color.red = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX4", 0), SettingsMap.GetInt("SLIDER_DMX4_Ramp", 0), eff_pos, use_ramps);
    if( channels >= 5 ) {
        color.green = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX5", 0), SettingsMap.GetInt("SLIDER_DMX5_Ramp", 0), eff_pos, use_ramps);
    }
    if( channels >= 6 ) {
        color.blue = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX6", 0), SettingsMap.GetInt("SLIDER_DMX6_Ramp", 0), eff_pos, use_ramps);
    }
    buffer.SetPixel(1, 0, color);
    
    if( channels < 7 || buffer.BufferWi < 3) return;
    color = xlBLACK;
    color.red = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX7", 0), SettingsMap.GetInt("SLIDER_DMX7_Ramp", 0), eff_pos, use_ramps);
    if( channels >= 8 ) {
        color.green = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX8", 0), SettingsMap.GetInt("SLIDER_DMX8_Ramp", 0), eff_pos, use_ramps);
    }
    if( channels >= 9 ) {
        color.blue = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX9", 0), SettingsMap.GetInt("SLIDER_DMX9_Ramp", 0), eff_pos, use_ramps);
    }
    buffer.SetPixel(2, 0, color);
    
    if( channels < 10 || buffer.BufferWi < 4) return;
    color = xlBLACK;
    color.red = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX10", 0), SettingsMap.GetInt("SLIDER_DMX10_Ramp", 0), eff_pos, use_ramps);
    if( channels >= 11 ) {
        color.green = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX11", 0), SettingsMap.GetInt("SLIDER_DMX11_Ramp", 0), eff_pos, use_ramps);
    }
    if( channels >= 12 ) {
        color.blue = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX12", 0), SettingsMap.GetInt("SLIDER_DMX12_Ramp", 0), eff_pos, use_ramps);
    }
    buffer.SetPixel(3, 0, color);
    
    if( channels < 13 || buffer.BufferWi < 5) return;
    color = xlBLACK;
    color.red = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX13", 0), SettingsMap.GetInt("SLIDER_DMX13_Ramp", 0), eff_pos, use_ramps);
    if( channels >= 14 ) {
        color.green = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX14", 0), SettingsMap.GetInt("SLIDER_DMX14_Ramp", 0), eff_pos, use_ramps);
    }
    if( channels >= 15 ) {
        color.blue = GetRampedValue(SettingsMap.GetInt("SLIDER_DMX15", 0), SettingsMap.GetInt("SLIDER_DMX15_Ramp", 0), eff_pos, use_ramps);
    }
    buffer.SetPixel(4, 0, color);
}

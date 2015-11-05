/***************************************************************
 * Name:      RenderDmx.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2015-11-02
 * Copyright: 2015 by Gil Jones
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#include "RgbEffects.h"

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

void RgbEffects::RenderDMX(int ch1, int ch2, int ch3, int ch4, int ch5, int ch6, int ch7, int ch8, int ch9, int ch10, int ch11, int ch12, int ch13, int ch14, int ch15,
                           int ch1_ramp, int ch2_ramp, int ch3_ramp, int ch4_ramp, int ch5_ramp, int ch6_ramp,
                           int ch7_ramp, int ch8_ramp, int ch9_ramp, int ch10_ramp, int ch11_ramp, int ch12_ramp,
                           int ch13_ramp, int ch14_ramp, int ch15_ramp,
                           bool use_ramps, const wxString& num_channels)
{
    double eff_pos = GetEffectTimeIntervalPosition();
    wxArrayString parts = wxSplit( num_channels, ' ');
    int channels = wxAtoi(parts[1]);

    xlColor color = xlBLACK;

    color.red = GetRampedValue(ch1, ch1_ramp, eff_pos, use_ramps);
    if( channels >= 2 ) {
        color.green = GetRampedValue(ch2, ch2_ramp, eff_pos, use_ramps);
    }
    if( channels >= 3 ) {
        color.blue = GetRampedValue(ch3, ch3_ramp, eff_pos, use_ramps);
    }
    SetPixel(0, 0, color);

    if( channels < 4 || BufferWi < 2) return;
    color = xlBLACK;
    color.red = GetRampedValue(ch4, ch4_ramp, eff_pos, use_ramps);
    if( channels >= 5 ) {
        color.green = GetRampedValue(ch5, ch5_ramp, eff_pos, use_ramps);
    }
    if( channels >= 6 ) {
        color.blue = GetRampedValue(ch6, ch6_ramp, eff_pos, use_ramps);
    }
    SetPixel(1, 0, color);

    if( channels < 7 || BufferWi < 3) return;
    color = xlBLACK;
    color.red = GetRampedValue(ch7, ch7_ramp, eff_pos, use_ramps);
    if( channels >= 8 ) {
        color.green = GetRampedValue(ch8, ch8_ramp, eff_pos, use_ramps);
    }
    if( channels >= 9 ) {
        color.blue = GetRampedValue(ch9, ch9_ramp, eff_pos, use_ramps);
    }
    SetPixel(2, 0, color);

    if( channels < 10 || BufferWi < 4) return;
    color = xlBLACK;
    color.red = GetRampedValue(ch10, ch10_ramp, eff_pos, use_ramps);
    if( channels >= 11 ) {
        color.green = GetRampedValue(ch11, ch11_ramp, eff_pos, use_ramps);
    }
    if( channels >= 12 ) {
        color.blue = GetRampedValue(ch12, ch12_ramp, eff_pos, use_ramps);
    }
    SetPixel(3, 0, color);

    if( channels < 13 || BufferWi < 5) return;
    color = xlBLACK;
    color.red = GetRampedValue(ch13, ch13_ramp, eff_pos, use_ramps);
    if( channels >= 14 ) {
        color.green = GetRampedValue(ch14, ch14_ramp, eff_pos, use_ramps);
    }
    if( channels >= 15 ) {
        color.blue = GetRampedValue(ch15, ch15_ramp, eff_pos, use_ramps);
    }
    SetPixel(4, 0, color);
}


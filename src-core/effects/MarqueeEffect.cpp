/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MarqueeEffect.h"

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/marquee-16.xpm"
#include "../../include/marquee-24.xpm"
#include "../../include/marquee-32.xpm"
#include "../../include/marquee-48.xpm"
#include "../../include/marquee-64.xpm"

int MarqueeEffect::sBandSizeDefault = 3;
int MarqueeEffect::sBandSizeMin = 1;
int MarqueeEffect::sBandSizeMax = 100;
int MarqueeEffect::sSkipSizeDefault = 0;
int MarqueeEffect::sSkipSizeMin = 0;
int MarqueeEffect::sSkipSizeMax = 100;
int MarqueeEffect::sThicknessDefault = 1;
int MarqueeEffect::sThicknessMin = 1;
int MarqueeEffect::sThicknessMax = 100;
int MarqueeEffect::sStaggerDefault = 0;
int MarqueeEffect::sStaggerMin = 0;
int MarqueeEffect::sStaggerMax = 50;
int MarqueeEffect::sSpeedDefault = 3;
int MarqueeEffect::sSpeedMin = 0;
int MarqueeEffect::sSpeedMax = 50;
int MarqueeEffect::sStartDefault = 0;
int MarqueeEffect::sStartMin = 0;
int MarqueeEffect::sStartMax = 100;
bool MarqueeEffect::sReverseDefault = false;
int MarqueeEffect::sScaleXDefault = 100;
int MarqueeEffect::sScaleXMin = 1;
int MarqueeEffect::sScaleXMax = 100;
int MarqueeEffect::sScaleYDefault = 100;
int MarqueeEffect::sScaleYMin = 1;
int MarqueeEffect::sScaleYMax = 100;
bool MarqueeEffect::sPixelOffsetsDefault = false;
int MarqueeEffect::sXCDefault = 0;
int MarqueeEffect::sXCMin = -100;
int MarqueeEffect::sXCMax = 100;
bool MarqueeEffect::sWrapXDefault = false;
int MarqueeEffect::sYCDefault = 0;
int MarqueeEffect::sYCMin = -100;
int MarqueeEffect::sYCMax = 100;
bool MarqueeEffect::sWrapYDefault = false;

MarqueeEffect::MarqueeEffect(int id) : RenderableEffect(id, "Marquee", marquee_16, marquee_24, marquee_32, marquee_48, marquee_64)
{
    //ctor
}

MarqueeEffect::~MarqueeEffect()
{
    //dtor
}

void MarqueeEffect::OnMetadataLoaded()
{
    sBandSizeDefault = GetIntDefault("Marquee_Band_Size", sBandSizeDefault);
    sBandSizeMin = (int)GetMinFromMetadata("Marquee_Band_Size", sBandSizeMin);
    sBandSizeMax = (int)GetMaxFromMetadata("Marquee_Band_Size", sBandSizeMax);
    sSkipSizeDefault = GetIntDefault("Marquee_Skip_Size", sSkipSizeDefault);
    sSkipSizeMin = (int)GetMinFromMetadata("Marquee_Skip_Size", sSkipSizeMin);
    sSkipSizeMax = (int)GetMaxFromMetadata("Marquee_Skip_Size", sSkipSizeMax);
    sThicknessDefault = GetIntDefault("Marquee_Thickness", sThicknessDefault);
    sThicknessMin = (int)GetMinFromMetadata("Marquee_Thickness", sThicknessMin);
    sThicknessMax = (int)GetMaxFromMetadata("Marquee_Thickness", sThicknessMax);
    sStaggerDefault = GetIntDefault("Marquee_Stagger", sStaggerDefault);
    sStaggerMin = (int)GetMinFromMetadata("Marquee_Stagger", sStaggerMin);
    sStaggerMax = (int)GetMaxFromMetadata("Marquee_Stagger", sStaggerMax);
    sSpeedDefault = GetIntDefault("Marquee_Speed", sSpeedDefault);
    sSpeedMin = (int)GetMinFromMetadata("Marquee_Speed", sSpeedMin);
    sSpeedMax = (int)GetMaxFromMetadata("Marquee_Speed", sSpeedMax);
    sStartDefault = GetIntDefault("Marquee_Start", sStartDefault);
    sStartMin = (int)GetMinFromMetadata("Marquee_Start", sStartMin);
    sStartMax = (int)GetMaxFromMetadata("Marquee_Start", sStartMax);
    sReverseDefault = GetBoolDefault("Marquee_Reverse", sReverseDefault);
    sScaleXDefault = GetIntDefault("Marquee_ScaleX", sScaleXDefault);
    sScaleXMin = (int)GetMinFromMetadata("Marquee_ScaleX", sScaleXMin);
    sScaleXMax = (int)GetMaxFromMetadata("Marquee_ScaleX", sScaleXMax);
    sScaleYDefault = GetIntDefault("Marquee_ScaleY", sScaleYDefault);
    sScaleYMin = (int)GetMinFromMetadata("Marquee_ScaleY", sScaleYMin);
    sScaleYMax = (int)GetMaxFromMetadata("Marquee_ScaleY", sScaleYMax);
    sPixelOffsetsDefault = GetBoolDefault("Marquee_PixelOffsets", sPixelOffsetsDefault);
    sXCDefault = GetIntDefault("MarqueeXC", sXCDefault);
    sXCMin = (int)GetMinFromMetadata("MarqueeXC", sXCMin);
    sXCMax = (int)GetMaxFromMetadata("MarqueeXC", sXCMax);
    sWrapXDefault = GetBoolDefault("Marquee_WrapX", sWrapXDefault);
    sYCDefault = GetIntDefault("MarqueeYC", sYCDefault);
    sYCMin = (int)GetMinFromMetadata("MarqueeYC", sYCMin);
    sYCMax = (int)GetMaxFromMetadata("MarqueeYC", sYCMax);
    sWrapYDefault = GetBoolDefault("Marquee_WrapY", sWrapYDefault);
}

static void UpdateMarqueeColor(int &position, int &band_color, int colorcnt, int color_size, int shift)
{
    if (shift == 0) return;
    if (shift > 0) {
        int index = 0;
        while (index < shift) {
            position++;
            if (position >= color_size) {
                band_color++;
                band_color %= colorcnt;
                position = 0;
            }
            index++;
        }
    } else {
        int index = 0;
        while (index > shift) {
            position--;
            if (position < 0) {
                band_color++;
                band_color %= colorcnt;
                position = color_size-1;
            }
            index--;
        }
    }
}

void MarqueeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();

    int BandSize = GetValueCurveInt("Marquee_Band_Size", sBandSizeDefault, SettingsMap, oset, sBandSizeMin, sBandSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int SkipSize = GetValueCurveInt("Marquee_Skip_Size", sSkipSizeDefault, SettingsMap, oset, sSkipSizeMin, sSkipSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Thickness = GetValueCurveInt("Marquee_Thickness", sThicknessDefault, SettingsMap, oset, sThicknessMin, sThicknessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int stagger = GetValueCurveInt("Marquee_Stagger", sStaggerDefault, SettingsMap, oset, sStaggerMin, sStaggerMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int mSpeed = GetValueCurveInt("Marquee_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int mStart = GetValueCurveInt("Marquee_Start", sStartDefault, SettingsMap, oset, sStartMin, sStartMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int x_scale = GetValueCurveInt("Marquee_ScaleX", sScaleXDefault, SettingsMap, oset, sScaleXMin, sScaleXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int y_scale = GetValueCurveInt("Marquee_ScaleY", sScaleYDefault, SettingsMap, oset, sScaleYMin, sScaleYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xc_adj = GetValueCurveInt("MarqueeXC", sXCDefault, SettingsMap, oset, sXCMin, sXCMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yc_adj = GetValueCurveInt("MarqueeYC", sYCDefault, SettingsMap, oset, sYCMin, sYCMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Marquee_Reverse", sReverseDefault);
    bool pixelOffsets = SettingsMap.GetBool("CHECKBOX_Marquee_PixelOffsets", sPixelOffsetsDefault);
    bool wrap_x = SettingsMap.GetBool("CHECKBOX_Marquee_WrapX", sWrapXDefault);
    bool wrap_y = SettingsMap.GetBool("CHECKBOX_Marquee_WrapY", sWrapYDefault);

    size_t colorcnt = buffer.GetColorCount();
    int color_size = BandSize +  SkipSize;
    int repeat_size = color_size * colorcnt;
    int eff_pos = buffer.curPeriod - buffer.curEffStartPer;

    int x = (mSpeed * eff_pos) / 5;
    int corner_x1 = 0;
    int corner_y1 = 0;
    int corner_x2 = std::round(((double)(buffer.BufferWi * x_scale) / 100.0) - 1.0);
    int corner_y2 = std::round(((double)(buffer.BufferHt * y_scale) / 100.0) - 1.0);
    int sign = 1;
    if (reverse_dir) {
        sign = -1;
    }

    int xoffset_adj = xc_adj;
    int yoffset_adj = yc_adj;
    if (!pixelOffsets) {
        xoffset_adj = (xoffset_adj*buffer.BufferWi)/100.0; // xc_adj is from -100 to 100
        yoffset_adj = (yoffset_adj*buffer.BufferHt)/100.0; // yc_adj is from -100 to 100
    }

    for (int thick = 0; thick < Thickness; thick++) {
        int current_color = ((x + mStart) % repeat_size) / color_size;
        int current_pos = (((x + mStart) % repeat_size) % color_size);
        if (sign < 0) {
            current_color = colorcnt - current_color - 1;
        }

        if (corner_y2 != corner_y1) {
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*(stagger + 1) * sign);
            for (int x_pos = corner_x1; x_pos <= corner_x2; x_pos++) {
                xlColor color = xlCLEAR;
                if (current_pos < BandSize) {
                    buffer.palette.GetColor(current_color, color);
                }
                buffer.ProcessPixel(x_pos + xoffset_adj, corner_y2 + yoffset_adj, color, wrap_x, wrap_y);
                UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1 * sign);
            }
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick * 2 * sign);
            for (int y_pos = corner_y2; y_pos >= corner_y1; y_pos--) {
                xlColor color = xlCLEAR;
                if (current_pos < BandSize) {
                    buffer.palette.GetColor(current_color, color);
                }
                buffer.ProcessPixel(corner_x2 + xoffset_adj, y_pos + yoffset_adj, color, wrap_x, wrap_y);
                UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1 * sign);
            }
        }
        UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick*2*sign);
        for (int x_pos = corner_x2; x_pos >= corner_x1; x_pos--) {
            xlColor color = xlCLEAR;
            if (current_pos < BandSize) {
                buffer.palette.GetColor(current_color, color);
            }
            buffer.ProcessPixel(x_pos + xoffset_adj, corner_y1 + yoffset_adj, color, wrap_x, wrap_y);
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1*sign);
        }
        if (corner_y2 != corner_y1) {
            UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, thick * 2 * sign);
            for (int y_pos = corner_y1; y_pos <= corner_y2 - 1; y_pos++) {
                xlColor color = xlCLEAR;
                if (current_pos < BandSize) {
                    buffer.palette.GetColor(current_color, color);
                }
                buffer.ProcessPixel(corner_x1 + xoffset_adj, y_pos + yoffset_adj, color, wrap_x, wrap_y);
                UpdateMarqueeColor(current_pos, current_color, colorcnt, color_size, 1 * sign);
            }
        }
        corner_x1++;
        corner_y1++;
        corner_x2--;
        corner_y2--;
    }
}

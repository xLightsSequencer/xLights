/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MarqueeEffect.h"
#include "MarqueePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/marquee-16.xpm"
#include "../../include/marquee-24.xpm"
#include "../../include/marquee-32.xpm"
#include "../../include/marquee-48.xpm"
#include "../../include/marquee-64.xpm"

MarqueeEffect::MarqueeEffect(int id) : RenderableEffect(id, "Marquee", marquee_16, marquee_24, marquee_32, marquee_48, marquee_64)
{
    //ctor
}

MarqueeEffect::~MarqueeEffect()
{
    //dtor
}

xlEffectPanel* MarqueeEffect::CreatePanel(wxWindow* parent)
{
    return new MarqueePanel(parent);
}

void MarqueeEffect::SetDefaultParameters() {
    MarqueePanel *mp = (MarqueePanel*)panel;
    if (mp == nullptr) {
        return;
    }

    SetSliderValue(mp->Slider_MarqueeXC, 0);
    SetSliderValue(mp->Slider_MarqueeYC, 0);
    SetSliderValue(mp->Slider_Marquee_Band_Size, 3);
    SetSliderValue(mp->Slider_Marquee_ScaleX, 100);
    SetSliderValue(mp->Slider_Marquee_ScaleY, 100);
    SetSliderValue(mp->Slider_Marquee_Skip_Size, 0);
    SetSliderValue(mp->Slider_Marquee_Speed, 3);
    SetSliderValue(mp->Slider_Marquee_Stagger, 0);
    SetSliderValue(mp->Slider_Marquee_Start, 0);
    SetSliderValue(mp->Slider_Marquee_Thickness, 1);

    SetCheckBoxValue(mp->CheckBox_Marquee_Reverse, false);
    SetCheckBoxValue(mp->CheckBox_Marquee_PixelOffsets, false);
    SetCheckBoxValue(mp->CheckBox_Marquee_WrapX, false);

    mp->BitmapButton_MarqueeXCVC->SetActive(false);
    mp->BitmapButton_MarqueeYCVC->SetActive(false);
    mp->BitmapButton_Marquee_Band_SizeVC->SetActive(false);
    mp->BitmapButton_Marquee_ScaleXVC->SetActive(false);
    mp->BitmapButton_Marquee_ScaleYVC->SetActive(false);
    mp->BitmapButton_Marquee_Skip_SizeVC->SetActive(false);
    mp->BitmapButton_Marquee_SpeedVC->SetActive(false);
    mp->BitmapButton_Marquee_StaggerVC->SetActive(false);
    mp->BitmapButton_Marquee_StartVC->SetActive(false);
    mp->BitmapButton_Marquee_ThicknessVC->SetActive(false);
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

    int BandSize = GetValueCurveInt("Marquee_Band_Size", 3, SettingsMap, oset, MARQUEE_BAND_SIZE_MIN, MARQUEE_BAND_SIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int SkipSize = GetValueCurveInt("Marquee_Skip_Size", 0, SettingsMap, oset, MARQUEE_SKIP_SIZE_MIN, MARQUEE_SKIP_SIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Thickness = GetValueCurveInt("Marquee_Thickness", 1, SettingsMap, oset, MARQUEE_THICKNESS_MIN, MARQUEE_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int stagger = GetValueCurveInt("Marquee_Stagger", 0, SettingsMap, oset, MARQUEE_STAGGER_MIN, MARQUEE_STAGGER_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int mSpeed = GetValueCurveInt("Marquee_Speed", 3, SettingsMap, oset, MARQUEE_SPEED_MIN, MARQUEE_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int mStart = GetValueCurveInt("Marquee_Start", 0, SettingsMap, oset, MARQUEE_START_MIN, MARQUEE_START_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int x_scale = GetValueCurveInt("Marquee_ScaleX", 100, SettingsMap, oset, MARQUEE_SCALEX_MIN, MARQUEE_SCALEX_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int y_scale = GetValueCurveInt("Marquee_ScaleY", 100, SettingsMap, oset, MARQUEE_SCALEY_MIN, MARQUEE_SCALEY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xc_adj = GetValueCurveInt("MarqueeXC", 0, SettingsMap, oset, MARQUEE_XC_MIN, MARQUEE_XC_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yc_adj = GetValueCurveInt("MarqueeYC", 0, SettingsMap, oset, MARQUEE_YC_MIN, MARQUEE_YC_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool reverse_dir = SettingsMap.GetBool("CHECKBOX_Marquee_Reverse");
    bool pixelOffsets = SettingsMap.GetBool("CHECKBOX_Marquee_PixelOffsets");
    bool wrap_x = SettingsMap.GetBool("CHECKBOX_Marquee_WrapX");
    bool wrap_y = SettingsMap.GetBool("CHECKBOX_Marquee_WrapY");

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
       // wxLogDebug(wxString::Format("Color: %d,  Pos: %d", current_color, current_pos));
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

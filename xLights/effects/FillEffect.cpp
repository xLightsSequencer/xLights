#include "FillEffect.h"
#include "FillPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../SequenceCheck.h"

#include "../../include/fill-16.xpm"
#include "../../include/fill-64.xpm"

FillEffect::FillEffect(int i) : RenderableEffect(i, "Fill", fill_16, fill_64, fill_64, fill_64, fill_64)
{
    //ctor
}

FillEffect::~FillEffect()
{
    //dtor
}

void FillEffect::SetDefaultParameters(Model *cls) {
    FillPanel *fp = (FillPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->BitmapButton_Fill_Band_Size->SetActive(false);
    fp->BitmapButton_Fill_Offset->SetActive(false);
    fp->BitmapButton_Fill_Position->SetActive(false);
    fp->BitmapButton_Fill_Skip_Size->SetActive(false);

    SetSliderValue(fp->Slider_Fill_Band_Size, 0);
    SetSliderValue(fp->Slider_Fill_Offset, 0);
    SetSliderValue(fp->Slider_Fill_Position, 100);
    SetSliderValue(fp->Slider_Fill_Skip_Size, 0);

    SetChoiceValue(fp->Choice_Fill_Direction, "Up");

    SetCheckBoxValue(fp->CheckBox_Fill_Color_Time, false);
    SetCheckBoxValue(fp->CheckBox_Fill_Offset_In_Pixels, true);
    SetCheckBoxValue(fp->CheckBox_Fill_Wrap, true);
}

std::list<std::string> FillEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    if (settings.Get("E_VALUECURVE_Fill_Position", "").find("Active=FALSE") != std::string::npos)
    {
        res.push_back(wxString::Format("    WARN: Fill effect without a position value curve. Was that intentional? Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *FillEffect::CreatePanel(wxWindow *parent) {
    return new FillPanel(parent);
}

bool FillEffect::needToAdjustSettings(const std::string &version)
{
    return true;
}

void FillEffect::adjustSettings(const std::string &version, Effect *effect)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect);
    }

    SettingsMap &settings = effect->GetSettings();

    if (IsVersionOlder("2016.41", version))
    {
        settings["E_CHECKBOX_Fill_Color_Time"] = "1";
    }

    if (IsVersionOlder("2016.53", version))
    {
        settings["E_CHECKBOX_Fill_Wrap"] = "1";
    }
}

static void UpdateFillColor(int &position, int &band_color, int colorcnt, int color_size, int shift)
{
    if( shift == 0 ) return;
    if( shift > 0 )
    {
        int index = 0;
        while( index < shift )
        {
            position++;
            if( position >= color_size )
            {
                band_color++;
                band_color %= colorcnt;
                position = 0;
            }
            index++;
        }
    }
    else
    {
        int index = 0;
        while( index > shift )
        {
            position--;
            if( position < 0 )
            {
                band_color++;
                band_color %= colorcnt;
                position = color_size-1;
            }
            index--;
        }
    }
}

void GetColorFromPosition(double pos, xlColor& color, size_t colorcnt, RenderBuffer &buffer)
{
    double color_val = pos * (colorcnt-1);
    int color_int = (int)color_val;
    double color_pct = color_val - (double)color_int;
    int color2 = std::min(color_int+1, (int)colorcnt-1);
    if( color_int < color2 )
    {
        buffer.Get2ColorBlend(color_int, color2, std::min( color_pct, 1.0), color);
    }
    else
    {
        buffer.palette.GetColor(color2, color);
    }
}

static inline int GetDirection(const std::string & DirectionString) {
    if ("Up" == DirectionString) {
        return 0;
    } else if ("Down" == DirectionString) {
        return 1;
    } else if ("Left" == DirectionString) {
        return 2;
    } else if ("Right" == DirectionString) {
        return 3;
    }
    return 0;
}

void FillEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    double eff_pos = buffer.GetEffectTimeIntervalPosition();
    int position = GetValueCurveInt("Fill_Position", 100, SettingsMap, eff_pos, 0, 100);
    double pos_pct = (double)position / 100.0;
    int Direction = GetDirection(SettingsMap["CHOICE_Fill_Direction"]);
    int BandSize = GetValueCurveInt("Fill_Band_Size", 0, SettingsMap, eff_pos, 0, 250);
    int SkipSize = GetValueCurveInt("Fill_Skip_Size", 0, SettingsMap, eff_pos, 0, 250);
    int offset = GetValueCurveInt("Fill_Offset", 0, SettingsMap, eff_pos, 0, 100);
    int offset_in_pixels = SettingsMap.GetBool("CHECKBOX_Fill_Offset_In_Pixels", true);
    int color_by_time = SettingsMap.GetBool("CHECKBOX_Fill_Color_Time", false);
    int wrap = SettingsMap.GetBool("CHECKBOX_Fill_Wrap", true);

    switch (Direction)
    {
        default:
        case 0:  // Up
        case 1:  // Down
            if( !offset_in_pixels ) {
                offset = ((buffer.BufferHt-1) * offset) / 100;
            } else {
                offset %= buffer.BufferHt;
            }
            break;
        case 2:  // Left
        case 3:  // Right
            if( !offset_in_pixels ) {
                offset = ((buffer.BufferWi-1) * offset) / 100;
            } else {
                offset %= buffer.BufferWi;
            }
            break;

    }


    int x,y;
    xlColor color;
    size_t colorcnt = buffer.GetColorCount();
    int color_size = BandSize +  SkipSize;
    int current_color = 0;
    int current_pos = 0;
    int target = 0;

    if( BandSize == 0 ) {
        GetColorFromPosition(eff_pos, color, colorcnt, buffer);
    }

    switch (Direction)
    {
        default:
        case 0:  // Up
            offset %= buffer.BufferHt;
            if( wrap ) {
                target = buffer.BufferHt*pos_pct+offset;
            } else {
                target = offset + (buffer.BufferHt-offset)*pos_pct;
            }
            for( y=offset; y<target; y++)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int y_pos = y;
                if( y_pos >= buffer.BufferHt ) y_pos -= buffer.BufferHt;
                if( !color_by_time ) {
                    double pos = (double)y / (double)(buffer.BufferHt+offset-1);
                    GetColorFromPosition(pos, color, colorcnt, buffer);
                }
                for (x=0; x<buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y_pos, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;
        case 1:  // Down
            offset %= buffer.BufferHt;
            if( wrap ) {
                target = buffer.BufferHt*(1.0-pos_pct)-offset;
            } else {
                target = (buffer.BufferHt-offset)*(1.0-pos_pct);
            }
            for( y=buffer.BufferHt-1-offset; y>=target; y--)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int y_pos = y;
                if( y_pos < 0 ) y_pos += buffer.BufferHt;
                if( !color_by_time ) {
                    double pos = 1.0 - (double)y / (double)(buffer.BufferHt+offset-1);
                    GetColorFromPosition(pos, color, colorcnt, buffer);
                }
                for (x=0; x<buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y_pos, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;
        case 2:  // Left
            offset %= buffer.BufferWi;
            if( wrap ) {
                target = buffer.BufferWi*(1.0-pos_pct)-offset;
            } else {
                target = (buffer.BufferWi-offset)*(1.0-pos_pct);
            }
            for (x=buffer.BufferWi-1-offset; x>=target; x--)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int x_pos = x;
                if( x_pos < 0 ) x_pos += buffer.BufferWi;
                if( !color_by_time ) {
                    double pos = 1.0 - (double)x / (double)(buffer.BufferWi+offset-1);
                    GetColorFromPosition(pos, color, colorcnt, buffer);
                }
                for( y=0; y<buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x_pos, y, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;
        case 3:  // Right
            offset %= buffer.BufferWi;
            if( wrap ) {
                target = buffer.BufferWi*pos_pct+offset;
            } else {
                target = offset + (buffer.BufferWi-offset)*pos_pct;
            }
            for (x=offset; x<target; x++)
            {
                if( BandSize > 0 ) {
                    color = xlBLACK;
                    if( current_pos < BandSize )
                    {
                        buffer.palette.GetColor(current_color, color);
                    }
                }
                int x_pos = x;
                if( x_pos >= buffer.BufferWi ) x_pos -= buffer.BufferWi;
                if( !color_by_time ) {
                    double pos = (double)x / (double)(buffer.BufferWi+offset-1);
                    GetColorFromPosition(pos, color, colorcnt, buffer);
                }
                for( y=0; y<buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x_pos, y, color);
                }
                if( BandSize > 0 ) {
                    UpdateFillColor(current_pos, current_color, colorcnt, color_size, 1);
                }
            }
            break;

    }
 }

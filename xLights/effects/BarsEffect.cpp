/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BarsEffect.h"
#include "BarsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/bars-16.xpm"
#include "../../include/bars-24.xpm"
#include "../../include/bars-32.xpm"
#include "../../include/bars-48.xpm"
#include "../../include/bars-64.xpm"

BarsEffect::BarsEffect(int i) : RenderableEffect(i, "Bars", bars_16, bars_24, bars_32, bars_48, bars_64)
{
    //ctor
}

BarsEffect::~BarsEffect()
{
    //dtor
}

xlEffectPanel *BarsEffect::CreatePanel(wxWindow *parent) {
    return new BarsPanel(parent);
}

static inline int GetDirection(const std::string & DirectionString) {
    if ("up" == DirectionString) {
        return 0;
    } else if ("down" == DirectionString) {
        return 1;
    } else if ("expand" == DirectionString) {
        return 2;
    } else if ("compress" == DirectionString) {
        return 3;
    } else if ("Left" == DirectionString) {
        return 4;
    } else if ("Right" == DirectionString) {
        return 5;
    } else if ("H-expand" == DirectionString) {
        return 6;
    } else if ("H-compress" == DirectionString) {
        return 7;
    } else if ("Alternate Up" == DirectionString) {
        return 8;
    } else if ("Alternate Down" == DirectionString) {
        return 9;
    } else if ("Alternate Left" == DirectionString) {
        return 10;
    } else if ("Alternate Right" == DirectionString) {
        return 11;
    } else if ("Custom Horz" == DirectionString) {
        return 12;
    } else if ("Custom Vert" == DirectionString) {
        return 13;
    }
    return 0;
}

void BarsEffect::SetDefaultParameters() {
    BarsPanel *bp = (BarsPanel*)panel;
    if (bp == nullptr) {
        return;
    }

    bp->BitmapButton_Bars_BarCount->SetActive(false);
    bp->BitmapButton_Bars_Cycles->SetActive(false);
    bp->BitmapButton_Bars_Center->SetActive(false);

    SetSliderValue(bp->Slider_Bars_BarCount, 1);
    SetSliderValue(bp->Slider_Bars_Cycles, 10);
    SetSliderValue(bp->Slider_Bars_Center, 0);

    SetChoiceValue(bp->Choice_Bars_Direction, "up");

    SetCheckBoxValue(bp->CheckBox_Bars_Highlight, false);
    SetCheckBoxValue(bp->CheckBox_Bars_3D, false);
    SetCheckBoxValue(bp->CheckBox_Bars_Gradient, false);
}

void BarsEffect::GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer &buffer, bool gradient, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index)
{
    if (buffer.palette.IsSpatial(colorIndex))
    {
        buffer.palette.GetSpatialColor(colorIndex, x, y, color);
        xlColor color2;
        buffer.palette.GetSpatialColor(color2Index, x, y, color2);

        if (buffer.allowAlpha) {
            if (gradient) buffer.Get2ColorBlend(color, color2, pct);
            if (highlight && n % BarHt == 0) color = xlWHITE;
            if (show3d) color.alpha = 255.0 * double(BarHt - n%BarHt - 1) / BarHt;
        }
        else {
            if (gradient) buffer.Get2ColorBlend(color, color2, pct);
            HSVValue hsv = color.asHSV();
            if (highlight && n % BarHt == 0) hsv.saturation = 0.0;
            if (show3d) hsv.value *= double(BarHt - n%BarHt - 1) / BarHt;
            color = hsv;
        }
    }
}

void BarsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float offset = buffer.GetEffectTimeIntervalPosition();
    int PaletteRepeat = GetValueCurveInt("Bars_BarCount", 1, SettingsMap, offset, BARCOUNT_MIN, BARCOUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double cycles = GetValueCurveDouble("Bars_Cycles", 1.0, SettingsMap, offset, BARCYCLES_MIN, BARCYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), 10);
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    double Center = GetValueCurveDouble("Bars_Center", 0, SettingsMap, position, BARCENTER_MIN, BARCENTER_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Direction = GetDirection(SettingsMap["CHOICE_Bars_Direction"]);
    bool Highlight = SettingsMap.GetBool("CHECKBOX_Bars_Highlight", false);
    bool Show3D = SettingsMap.GetBool("CHECKBOX_Bars_3D", false);
    bool Gradient = SettingsMap.GetBool("CHECKBOX_Bars_Gradient", false);

    int x,y,n,ColorIdx;
    size_t colorcnt = buffer.GetColorCount();
    if (colorcnt == 0) {
        colorcnt = 1;
    }
    int BarCount = PaletteRepeat * colorcnt;

    if (BarCount < 1) {
        BarCount=1;
    }

    xlColor color;

    if (Direction < 4 || Direction == 8 || Direction == 9)
    {
        int BarHt = (int)std::ceil((float)buffer.BufferHt / (float)BarCount);
        if(BarHt<1) BarHt=1;
        int NewCenter = buffer.BufferHt * (100 + Center) / 200;
        int BlockHt=colorcnt * BarHt;
        if(BlockHt<1) BlockHt=1;

        int f_offset = position*BlockHt;
        if (Direction == 8 || Direction == 9) {
            f_offset = floor(position*BarCount) * BarHt;
        }
        Direction = Direction > 4?Direction-8:Direction;

        for (y=-2*buffer.BufferHt; y<2*buffer.BufferHt; y++)
        {
            n=buffer.BufferHt+y+f_offset;
            ColorIdx=std::abs(n % BlockHt) / BarHt;
            int color2 = (ColorIdx+1)%colorcnt;
            double pct = (double)std::abs(n % BarHt) / (double)BarHt;

            if (buffer.allowAlpha) {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                if (Highlight && n % BarHt == 0) color = xlWHITE;
                if (Show3D) {
                    int numerator = BarHt - std::abs(n % BarHt) - 1;
                    color.alpha = 255.0 * double(numerator) / double(BarHt);
                }
            } else {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                HSVValue hsv = color.asHSV();
                if (Highlight && n % BarHt == 0) hsv.saturation=0.0;
                if (Show3D) {
                    int numerator = BarHt - std::abs(n % BarHt) - 1;
                    hsv.value *= double(numerator) / double(BarHt);
                }
                color = hsv;
            }
            switch (Direction)
            {
                case 1:
                    // down
                    for (x=0; x<buffer.BufferWi; x++)
                    {
                        GetSpatialColor(color, ColorIdx, (float)x / (float)buffer.BufferWi, (float)(n % BarHt) / (float)BarHt, buffer, Gradient, Highlight, Show3D, BarHt, n, pct, color2);
                        buffer.SetPixel(x, y, color);
                    }
                    break;
                case 2:
                    // expand
                    if (y <= NewCenter) {
                        for (x=0; x<buffer.BufferWi; x++)
                        {
                            GetSpatialColor(color, ColorIdx, (float)x / (float)buffer.BufferWi, (float)(n % BarHt) / (float)BarHt, buffer, Gradient, Highlight, Show3D, BarHt, n, pct, color2);
                            buffer.SetPixel(x, y, color);
                            buffer.SetPixel(x, NewCenter+(NewCenter-y), color);
                        }
                    }
                    break;
                case 3:
                    // compress
                    if (y >= NewCenter) {
                        for (x=0; x<buffer.BufferWi; x++)
                        {
                            GetSpatialColor(color, ColorIdx, (float)x / (float)buffer.BufferWi, (float)(n % BarHt) / (float)BarHt, buffer, Gradient, Highlight, Show3D, BarHt, n, pct, color2);
                            buffer.SetPixel(x, y, color);
                            buffer.SetPixel(x, NewCenter + (NewCenter - y), color);
                        }
                    }
                    break;
                default:
                    // up
                    for (x=0; x<buffer.BufferWi; x++)
                    {
                        GetSpatialColor(color, ColorIdx, (float)x / (float)buffer.BufferWi, 1.0 - (float)(n % BarHt) / (float)BarHt, buffer, Gradient, Highlight, Show3D, BarHt, n, pct, color2);
                        buffer.SetPixel(x, buffer.BufferHt-y-1, color);
                    }
                    break;
            }
        }
    }
    else if (Direction == 12 || Direction == 13)
    {
        int width = buffer.BufferWi;
        int height = buffer.BufferHt;
        if( Direction == 13 ) {
            std::swap(width, height);
        }
        int BarWi = (int)std::ceil((float)width / (float)BarCount);
        if(BarWi<1) BarWi=1;
        int NewCenter = (width * (100.0 + Center) / 200.0 - width/2);
        int BlockWi=colorcnt * BarWi;
        if(BlockWi<1) BlockWi=1;

        for (x=-2*width; x<2*width; x++)
        {
            n=width+x;
            ColorIdx=(n % BlockWi) / BarWi;
            int color2 = (ColorIdx+1)%colorcnt;
            double pct = (double)(n % BarWi) / (double)BarWi;
            if (buffer.allowAlpha) {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                if (Highlight && n % BarWi == 0) color = xlWHITE;
                if (Show3D) color.alpha = 255.0 * double(BarWi - n%BarWi - 1) / BarWi;

            } else {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                HSVValue hsv = color.asHSV();
                if (Highlight && n % BarWi == 0) hsv.saturation=0.0;
                if (Show3D) hsv.value *= double(BarWi - n%BarWi - 1) / BarWi;
                color = hsv;
            }

            int position_x = width-x-1 + NewCenter;
            for (y=0; y<height; y++)
            {
                GetSpatialColor(color, ColorIdx, 1.0 - pct, (float)y / (float)height, buffer, Gradient, Highlight, Show3D, BarWi, n, pct, color2);
                if( Direction == 12 ) {
                    buffer.SetPixel(position_x, y, color);
                } else {
                    buffer.SetPixel(y, position_x, color);
                }
            }
        }
    }
    else
    {
        int BarWi = (int)std::ceil((float)buffer.BufferWi / (float)BarCount);
        if(BarWi<1) BarWi=1;
        int NewCenter = buffer.BufferWi * (100 + Center) / 200;
        int BlockWi=colorcnt * BarWi;
        if(BlockWi<1) BlockWi=1;
        int f_offset = position*BlockWi;
        if (Direction > 9) {
            f_offset = floor(position*BarCount) * BarWi;
        }

        Direction = Direction > 9?Direction-6:Direction;
        for (x=-2*buffer.BufferWi; x<2*buffer.BufferWi; x++)
        {
            n=buffer.BufferWi+x+f_offset;
            ColorIdx=(n % BlockWi) / BarWi;
            int color2 = (ColorIdx+1)%colorcnt;
            double pct = (double)(n % BarWi) / (double)BarWi;
            if (buffer.allowAlpha) {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                if (Highlight && n % BarWi == 0) color = xlWHITE;
                if (Show3D) color.alpha = 255.0 * double(BarWi - n % BarWi - 1) / (double)BarWi;

            } else {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                HSVValue hsv = color.asHSV();
                if (Highlight && n % BarWi == 0) hsv.saturation=0.0;
                if (Show3D) hsv.value *= double(BarWi - n%BarWi - 1) / BarWi;
                color = hsv;
            }
            switch (Direction)
            {
                case 5:
                    // right
                    for (y=0; y<buffer.BufferHt; y++)
                    {
                        GetSpatialColor(color, ColorIdx, 1.0 - pct, (double)y / (double)buffer.BufferHt, buffer, Gradient, Highlight, Show3D, BarWi, n, pct, color2);
                        buffer.SetPixel(buffer.BufferWi-x-1, y, color);
                    }
                    break;
                case 6:
                    // H-expand
                    if (x <= NewCenter) {
                        for (y=0; y<buffer.BufferHt; y++)
                        {
                            GetSpatialColor(color, ColorIdx, pct, (double)y / (double)buffer.BufferHt, buffer, Gradient, Highlight, Show3D, BarWi, n, pct, color2);
                            buffer.SetPixel(x, y, color);
                            buffer.SetPixel(NewCenter + (NewCenter - x), y, color);
                        }
                    }
                    break;
                case 7:
                    // H-compress
                    if (x >= NewCenter) {
                        for (y=0; y<buffer.BufferHt; y++)
                        {
                            GetSpatialColor(color, ColorIdx, pct, (double)y / (double)buffer.BufferHt, buffer, Gradient, Highlight, Show3D, BarWi, n, pct, color2);
                            buffer.SetPixel(x, y, color);
                            buffer.SetPixel(NewCenter + (NewCenter - x), y, color);
                        }
                    }
                    break;
                default:
                    // left
                    for (y=0; y<buffer.BufferHt; y++)
                    {
                        GetSpatialColor(color, ColorIdx, pct, (double)y / (double)buffer.BufferHt, buffer, Gradient, Highlight, Show3D, BarWi, n, pct, color2);
                        buffer.SetPixel(x, y, color);
                    }
                    break;
            }
        }
    }
}

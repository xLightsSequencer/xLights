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

wxPanel *BarsEffect::CreatePanel(wxWindow *parent) {
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
    }
    return 0;
}

void BarsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float offset = buffer.GetEffectTimeIntervalPosition();
    int PaletteRepeat = GetValueCurveInt("Bars_BarCount", 1, SettingsMap, offset);
    double cycles = GetValueCurveDouble("Bars_Cycles", 1.0, SettingsMap, offset);

    int Center = SettingsMap.GetInt("SLIDER_Bars_Center", 0);
    int Direction = GetDirection(SettingsMap["CHOICE_Bars_Direction"]);
    bool Highlight = SettingsMap.GetBool("CHECKBOX_Bars_Highlight");
    bool Show3D = SettingsMap.GetBool("CHECKBOX_Bars_3D");
    bool Gradient = SettingsMap.GetBool("CHECKBOX_Bars_Gradient");

    int x,y,n,ColorIdx;
    HSVValue hsv;
    size_t colorcnt = buffer.GetColorCount();
    int BarCount = PaletteRepeat * colorcnt;
    double position = buffer.GetEffectTimeIntervalPosition(cycles);

    if (BarCount<1) BarCount=1;

    xlColor color;

    if (Direction < 4 || Direction == 8 || Direction == 9)
    {
        int BarHt = buffer.BufferHt/BarCount+1;
        if(BarHt<1) BarHt=1;
        int HalfHt = buffer.BufferHt/2;
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
            ColorIdx=(n % BlockHt) / BarHt;
            int color2 = (ColorIdx+1)%colorcnt;
            double pct = (double)(n % BarHt) / (double)BarHt;

            if (buffer.allowAlpha) {
                buffer.palette.GetColor(ColorIdx, color);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                if (Highlight && n % BarHt == 0) color = xlWHITE;
                if (Show3D) color.alpha = 255.0 * double(BarHt - n%BarHt - 1) / BarHt;
            } else {
                buffer.palette.GetHSV(ColorIdx, hsv);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
                if (Highlight && n % BarHt == 0) hsv.saturation=0.0;
                if (Show3D) hsv.value *= double(BarHt - n%BarHt - 1) / BarHt;
                color = hsv;
            }
            switch (Direction)
            {
                case 1:
                    // down
                    for (x=0; x<buffer.BufferWi; x++)
                    {
                        buffer.SetPixel(x, y, color);
                    }
                    break;
                case 2:
                    // expand
                    if (y <= NewCenter) {
                        for (x=0; x<buffer.BufferWi; x++)
                        {
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
                            buffer.SetPixel(x, y, color);
                            buffer.SetPixel(x, NewCenter + (NewCenter - y), color);
                        }
                    }
                    break;
                default:
                    // up
                    for (x=0; x<buffer.BufferWi; x++)
                    {
                        buffer.SetPixel(x, buffer.BufferHt-y-1, color);
                    }
                    break;
            }
        }
    }
    else
    {
        int BarWi = buffer.BufferWi/BarCount+1;
        if(BarWi<1) BarWi=1;
        int HalfWi = buffer.BufferWi/2;
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
                if (Show3D) color.alpha = 255.0 * double(BarWi - n%BarWi - 1) / BarWi;

            } else {
                buffer.palette.GetHSV(ColorIdx, hsv);
                if (Gradient) buffer.Get2ColorBlend(ColorIdx, color2, pct, color);
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
                        buffer.SetPixel(buffer.BufferWi-x-1, y, color);
                    }
                    break;
                case 6:
                    // H-expand
                    if (x <= NewCenter) {
                        for (y=0; y<buffer.BufferHt; y++)
                        {
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
                            buffer.SetPixel(x, y, color);
                            buffer.SetPixel(NewCenter + (NewCenter - x), y, color);
                        }
                    }
                    break;
                default:
                    // left
                    for (y=0; y<buffer.BufferHt; y++)
                    {
                        buffer.SetPixel(x, y, color);
                    }
                    break;
            }
        }
    }
}

#include "FillEffect.h"
#include "FillPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


#include "../../include/fill-16.xpm"
#include "../../include/fill-24.xpm"
#include "../../include/fill-32.xpm"
#include "../../include/fill-48.xpm"
#include "../../include/fill-64.xpm"

FillEffect::FillEffect(int i) : RenderableEffect(i, "Fill", fill_16, fill_24, fill_32, fill_48, fill_64)
{
    //ctor
}

FillEffect::~FillEffect()
{
    //dtor
}

wxPanel *FillEffect::CreatePanel(wxWindow *parent) {
    return new FillPanel(parent);
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
    int position = GetValueCurveInt("Fill_Position", 100, SettingsMap, eff_pos);
    double pos_pct = (double)position / 100.0;
    int Direction = GetDirection(SettingsMap["CHOICE_Fill_Direction"]);

    int x,y;
    xlColor color;
    size_t colorcnt = buffer.GetColorCount();
    double color_val = eff_pos * (colorcnt-1);
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

    switch (Direction)
    {
        default:
        case 0:  // Up
            for (x=0; x<buffer.BufferWi; x++)
            {
                for( y=0; y<buffer.BufferHt*pos_pct; y++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }
            break;
        case 1:  // Down
            for (x=0; x<buffer.BufferWi; x++)
            {
                for( y=buffer.BufferHt-1; y>=buffer.BufferHt*(1.0-pos_pct); y--)
                {
                    buffer.SetPixel(x, y, color);
                }
            }
            break;
        case 2:  // Left
            for (x=buffer.BufferWi-1; x>=buffer.BufferWi*(1.0-pos_pct); x--)
            {
                for( y=0; y<buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }
            break;
        case 3:  // Right
            for (x=0; x<buffer.BufferWi*pos_pct; x++)
            {
                for( y=0; y<buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }
            break;

    }
 }

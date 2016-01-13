#include "SpiralsEffect.h"
#include "SpiralsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/spirals-16.xpm"
#include "../../include/spirals-24.xpm"
#include "../../include/spirals-32.xpm"
#include "../../include/spirals-48.xpm"
#include "../../include/spirals-64.xpm"


SpiralsEffect::SpiralsEffect(int id) : RenderableEffect(id, "Spirals", spirals_16, spirals_24, spirals_32, spirals_48, spirals_64)
{
    //ctor
}

SpiralsEffect::~SpiralsEffect()
{
    //dtor
}
wxPanel *SpiralsEffect::CreatePanel(wxWindow *parent) {
    return new SpiralsPanel(parent);
}

void SpiralsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int PaletteRepeat = SettingsMap.GetInt("SLIDER_Spirals_Count", 0);
    float Movement = SettingsMap.GetDouble("TEXTCTRL_Spirals_Movement", 1.0);
    int Rotation = SettingsMap.GetInt("SLIDER_Spirals_Rotation", 0);
    int Thickness = SettingsMap.GetInt("SLIDER_Spirals_Thickness", 0);
    bool Blend = SettingsMap.GetBool("CHECKBOX_Spirals_Blend");
    bool Show3D = SettingsMap.GetBool("CHECKBOX_Spirals_3D");
    bool grow = SettingsMap.GetBool("CHECKBOX_Spirals_Grow");
    bool shrink = SettingsMap.GetBool("CHECKBOX_Spirals_Shrink");

    int strand_base,strand,thick,x,y,ColorIdx;
    if (PaletteRepeat == 0) {
        PaletteRepeat = 1;
    }
    size_t colorcnt=buffer.GetColorCount();
    int SpiralCount=colorcnt * PaletteRepeat;
    int deltaStrands=buffer.BufferWi / SpiralCount;
    int SpiralThickness=(deltaStrands * Thickness / 100) + 1;
    int spiralGap = deltaStrands - SpiralThickness;
    long SpiralState;
    long ThicknessState = 0;
    HSVValue hsv;
    xlColour color;
    
    int Direction = Movement > 0.001 ? 1 : (Movement < -0.001 ? -1 : 0);
    double position = buffer.GetEffectTimeIntervalPosition(std::abs(Movement));
    if (grow && shrink)
    {
        ThicknessState = position <= 0.5?spiralGap*(position*2):spiralGap*((1-position) * 2);
    }
    else if (grow)
    {
        ThicknessState = spiralGap * position;
    }
    else if (shrink)
    {
        ThicknessState = spiralGap * (1.0-position);
    }
    SpiralState = position*buffer.BufferWi*10*Direction;
    
    spiralGap += (spiralGap==0);
    SpiralThickness += ThicknessState;
    
    for(int ns=0; ns < SpiralCount; ns++)
    {
        strand_base=ns * deltaStrands;
        ColorIdx=ns % colorcnt;
        buffer.palette.GetColor(ColorIdx,color);
        for(thick=0; thick < SpiralThickness; thick++)
        {
            strand = (strand_base + thick) % buffer.BufferWi;
            for(y=0; y < buffer.BufferHt; y++)
            {
                x=(strand + SpiralState/10 + y*Rotation/buffer.BufferHt) % buffer.BufferWi;
                if (x < 0) x += buffer.BufferWi;
                if (Blend)
                {
                    buffer.GetMultiColorBlend(double(buffer.BufferHt-y-1)/double(buffer.BufferHt), false, color);
                }
                if (Show3D)
                {
                    double f = 1.0;
                    
                    if (Rotation < 0)
                    {
                        f=double(thick+1)/SpiralThickness;
                    }
                    else
                    {
                        f=double(SpiralThickness-thick)/SpiralThickness;
                    }
                    if (buffer.allowAlpha) {
                        xlColor c(color);
                        c.alpha = 255.0 * f;
                        buffer.SetPixel(x,y,c);
                    } else {
                        buffer.Color2HSV(color,hsv);
                        hsv.value *= f;
                        buffer.SetPixel(x,y,hsv);
                    }
                }
                else
                {
                    buffer.SetPixel(x,y,color);
                }
            }
        }
    }
}

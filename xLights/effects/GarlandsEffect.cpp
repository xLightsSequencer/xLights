#include "GarlandsEffect.h"
#include "GarlandsPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/garlands-16.xpm"
#include "../../include/garlands-24.xpm"
#include "../../include/garlands-32.xpm"
#include "../../include/garlands-48.xpm"
#include "../../include/garlands-64.xpm"

GarlandsEffect::GarlandsEffect(int id) : RenderableEffect(id, "Garlands", garlands_16, garlands_24, garlands_32, garlands_48, garlands_64)
{
    //ctor
}

GarlandsEffect::~GarlandsEffect()
{
    //dtor
}

wxPanel *GarlandsEffect::CreatePanel(wxWindow *parent) {
    return new GarlandsPanel(parent);
}

int GetDirection(const std::string &direction) {
    if ("Up" == direction) {
        return 0;
    } else if ("Down" == direction) {
        return 1;
    } else if ("Left" == direction) {
        return 2;
    } else if ("Right" == direction) {
        return 3;
    } else if ("Up then Down" == direction) {
        return 4;
    } else if ("Down then Up" == direction) {
        return 5;
    } else if ("Left then Right" == direction) {
        return 6;
    } else if ("Right then Left" == direction) {
        return 7;
    }
    return 0;
}

void GarlandsEffect::SetDefaultParameters(Model *cls) {
    GarlandsPanel *fp = (GarlandsPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    fp->BitmapButton_Garlands_CyclesVC->SetActive(false);
    fp->BitmapButton_Garlands_SpacingVC->SetActive(false);

    SetSliderValue(fp->Slider_Garlands_Type, 0);
    SetSliderValue(fp->Slider_Garlands_Spacing, 10);
    SetSliderValue(fp->Slider_Garlands_Cycles, 10);

    SetChoiceValue(fp->Choice_Garlands_Direction, "Up");
}

void GarlandsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int GarlandType = SettingsMap.GetInt("SLIDER_Garlands_Type", 0);
    int Spacing = GetValueCurveInt("Garlands_Spacing", 10, SettingsMap, oset, GARLANDS_SPACING_MIN, GARLANDS_SPACING_MAX);
    float cycles = GetValueCurveDouble("Garlands_Cycles", 1.0f, SettingsMap, oset, GARLANDS_CYCLES_MIN, GARLANDS_CYCLES_MAX);

    if (Spacing < 1) {
        Spacing = 1;
    }
    int x,y,yadj,ylimit,ring;
    double ratio;
    xlColor color;
    int dir = GetDirection(SettingsMap.Get("CHOICE_Garlands_Direction", "Up"));
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    if (dir > 3) {
        dir -= 4;
        if (position > 0.5) {
            position = (1.0 - position) * 2.0;
        } else {
            position *= 2.0;
        }
    }
    int buffMax = buffer.BufferHt;
    int garlandWid = buffer.BufferWi;
    if (dir > 1) {
        buffMax = buffer.BufferWi;
        garlandWid = buffer.BufferHt;
    }
    double PixelSpacing=Spacing*buffMax/100.0;
    if (PixelSpacing < 2.0) PixelSpacing=2.0;
    
    
    double total = buffMax * PixelSpacing - buffMax + 1;
    double positionOffset = total * position;
    
    for (ring = 0; ring < buffMax; ring++)
    {
        ratio=double(buffMax-ring-1)/double(buffMax);
        buffer.GetMultiColorBlend(ratio, false, color);
        
        y = 1.0 + ring*PixelSpacing - positionOffset;
        
        
        ylimit=ring;
        for (x=0; x<garlandWid; x++)
        {
            yadj=y;
            switch (GarlandType)
            {
                case 1:
                    switch (x%5)
                {
                    case 2:
                        yadj-=2;
                        break;
                    case 1:
                    case 3:
                        yadj-=1;
                        break;
                }
                    break;
                case 2:
                    switch (x%5)
                {
                    case 2:
                        yadj-=4;
                        break;
                    case 1:
                    case 3:
                        yadj-=2;
                        break;
                }
                    break;
                case 3:
                    switch (x%6)
                {
                    case 3:
                        yadj-=6;
                        break;
                    case 2:
                    case 4:
                        yadj-=4;
                        break;
                    case 1:
                    case 5:
                        yadj-=2;
                        break;
                }
                    break;
                case 4:
                    switch (x%5)
                {
                    case 1:
                    case 3:
                        yadj-=2;
                        break;
                }
                    break;
            }
            if (yadj < ylimit) yadj=ylimit;
            if (yadj < buffMax) {
                if (dir == 1 || dir == 2) {
                    yadj = buffMax - yadj - 1;
                }
                if (dir > 1) {
                    buffer.SetPixel(yadj,x,color);
                } else {
                    buffer.SetPixel(x,yadj,color);
                }
            }
        }
    }
}

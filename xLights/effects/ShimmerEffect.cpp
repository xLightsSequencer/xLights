#include "ShimmerEffect.h"
#include "ShimmerPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/shimmer.xpm"

ShimmerEffect::ShimmerEffect(int id) : RenderableEffect(id, "Shimmer", shimmer, shimmer, shimmer, shimmer, shimmer)
{
    //ctor
}

ShimmerEffect::~ShimmerEffect()
{
    //dtor
}

wxPanel *ShimmerEffect::CreatePanel(wxWindow *parent) {
    return new ShimmerPanel(parent);
}

void ShimmerEffect::SetDefaultParameters(Model *cls)
{
    ShimmerPanel *sp = (ShimmerPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Shimmer_CyclesVC->SetActive(false);
    sp->BitmapButton_Shimmer_Duty_FactorVC->SetActive(false);

    SetSliderValue(sp->Slider_Shimmer_Duty_Factor, 50);
    SetSliderValue(sp->Slider_Shimmer_Cycles, 10);

    SetCheckBoxValue(sp->CheckBox_Shimmer_Use_All_Colors, false);
}

void ShimmerEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    int Duty_Factor = GetValueCurveInt("Shimmer_Duty_Factor", 50, SettingsMap, oset);
    bool Use_All_Colors = SettingsMap.GetBool("CHECKBOX_Shimmer_Use_All_Colors", false);
    float cycles = GetValueCurveDouble("Shimmer_Cycles", 1.0, SettingsMap, oset);
    int colorcnt=buffer.GetColorCount();
    
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    
    double ColorIdx = round(position * 0.999 * (double)colorcnt);
    
    double pos2 = position * colorcnt;
    while (pos2 > 1.0) {
        pos2 -= 1.0;
    }
    if (pos2 * 100 > Duty_Factor) {
        return;
    }
    
    xlColor color;
    buffer.palette.GetColor(ColorIdx, color);
    for (int y=0; y<buffer.BufferHt; y++) {
        for (int x=0; x<buffer.BufferWi; x++) {
            if(Use_All_Colors) { // Should we randomly assign colors from palette or cycle thru sequentially?
                ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                buffer.palette.GetColor(ColorIdx, color); // Now go and get the hsv value for this ColorIdx
            }
            
            buffer.SetPixel(x,y,color); // Turn pixel
        }
    }
}

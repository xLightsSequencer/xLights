#include "ColorWashEffect.h"

#include "ColorWashPanel.h"
#include <wx/checkbox.h>
#include <wx/notebook.h>

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../../include/ColorWash.xpm"

#include <sstream>

static const std::string CHECKBOX_ColorWash_HFade("CHECKBOX_ColorWash_HFade");
static const std::string CHECKBOX_ColorWash_VFade("CHECKBOX_ColorWash_VFade");
static const std::string TEXTCTRL_ColorWash_Cycles("TEXTCTRL_ColorWash_Cycles");
static const std::string CHECKBOX_ColorWash_EntireModel("CHECKBOX_ColorWash_EntireModel");
static const std::string SLIDER_ColorWash_X1("SLIDER_ColorWash_X1");
static const std::string SLIDER_ColorWash_Y1("SLIDER_ColorWash_Y1");
static const std::string SLIDER_ColorWash_X2("SLIDER_ColorWash_X2");
static const std::string SLIDER_ColorWash_Y2("SLIDER_ColorWash_Y2");
static const std::string CHECKBOX_ColorWash_Shimmer("CHECKBOX_ColorWash_Shimmer");
static const std::string CHECKBOX_ColorWash_CircularPalette("CHECKBOX_ColorWash_CircularPalette");


ColorWashEffect::ColorWashEffect(int i) : RenderableEffect(i, "Color Wash", ColorWash, ColorWash, ColorWash, ColorWash, ColorWash)
{
    //ctor
}

ColorWashEffect::~ColorWashEffect()
{
    //dtor
}


int ColorWashEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                          DrawGLUtils::xlVertexColorAccumulator &bg) {
    if (e->HasBackgroundDisplayList()) {
        DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList(), bg);
        return e->GetBackgroundDisplayList().iconSize;
    }
    DrawGLUtils::DrawHBlendedRectangle(e->GetPalette(), x1, y1, x2, y2);
    return 2;
}

void ColorWashEffect::SetDefaultParameters(Model *cls) {
    ColorWashPanel *p = (ColorWashPanel*)panel;
    if (p == nullptr) {
        return;
    }
    p->CyclesTextCtrl->SetValue("1.0");
    SetCheckBoxValue(p->EntireModelCheckbox, true);
    SetCheckBoxValue(p->HFadeCheckBox, false);
    SetCheckBoxValue(p->VFadeCheckBox, false);
    SetCheckBoxValue(p->ShimmerCheckBox, false);
    SetCheckBoxValue(p->CircularPaletteCheckBox, false);
    SetSliderValue(p->C1SliderX, 0);
    SetSliderValue(p->C1SliderY, 0);
    SetSliderValue(p->C2SliderX, 100);
    SetSliderValue(p->C2SliderY, 100);
    p->CornersNotebook->Enable(false);
}

std::string ColorWashEffect::GetEffectString() {
    ColorWashPanel *p = (ColorWashPanel*)panel;
    std::stringstream ret;
    if (10 != p->SliderCycles->GetValue()) {
        ret << "E_TEXTCTRL_ColorWash_Cycles=";
        ret << p->CyclesTextCtrl->GetValue();
        ret << ",";
    }
    if (p->VFadeCheckBox->GetValue()) {
        ret << "E_CHECKBOX_ColorWash_VFade=1,";
    }
    if (p->HFadeCheckBox->GetValue()) {
        ret << "E_CHECKBOX_ColorWash_HFade=1,";
    }
    if (p->ShimmerCheckBox->GetValue()) {
        ret << "E_CHECKBOX_ColorWash_Shimmer=1,";
    }
    if (p->CircularPaletteCheckBox->GetValue()) {
        ret << "E_CHECKBOX_ColorWash_CircularPalette=1,";
    }
    if (!p->EntireModelCheckbox->GetValue()) {
        ret << "E_CHECKBOX_ColorWash_EntireModel=0,";
        if (p->C1SliderX->GetValue() != 0) {
            ret << "E_SLIDER_ColorWash_X1=";
            ret << p->C1SliderX->GetValue();
            ret << ",";
        }
        if (p->C1SliderY->GetValue() != 0) {
            ret << "E_SLIDER_ColorWash_Y1=";
            ret << p->C1SliderY->GetValue();
            ret << ",";
        }
        if (p->C2SliderX->GetValue() != 0) {
            ret << "E_SLIDER_ColorWash_X2=";
            ret << p->C2SliderX->GetValue();
            ret << ",";
        }
        if (p->C2SliderY->GetValue() != 0) {
            ret << "E_SLIDER_ColorWash_Y2=";
            ret << p->C2SliderY->GetValue();
            ret << ",";
        }
    }
    return ret.str();
}



wxPanel *ColorWashEffect::CreatePanel(wxWindow *parent) {
    return new ColorWashPanel(parent);
}


void ColorWashEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    bool HorizFade = SettingsMap.GetBool(CHECKBOX_ColorWash_HFade);
    bool VertFade = SettingsMap.GetBool(CHECKBOX_ColorWash_VFade);
    float cycles = SettingsMap.GetDouble(TEXTCTRL_ColorWash_Cycles, 1.0);
    bool EntireModel = SettingsMap.GetBool(CHECKBOX_ColorWash_EntireModel, true);
    int x1 = SettingsMap.GetInt(SLIDER_ColorWash_X1, 0);
    int y1 = SettingsMap.GetInt(SLIDER_ColorWash_Y1, 0);
    int x2 = SettingsMap.GetInt(SLIDER_ColorWash_X2, 100);
    int y2 = SettingsMap.GetInt(SLIDER_ColorWash_Y2, 100);
    bool shimmer = SettingsMap.GetBool(CHECKBOX_ColorWash_Shimmer);
    bool circularPalette = SettingsMap.GetBool(CHECKBOX_ColorWash_CircularPalette);
    
    
    int x,y;
    xlColor color, orig;
    
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    buffer.GetMultiColorBlend(position, circularPalette, color);
    
    int startX = 0;
    int startY = 0;
    int endX = buffer.BufferWi - 1;
    int endY = buffer.BufferHt - 1;
    
    if (!EntireModel) {
        startX = std::min(x1, x2);
        endX = std::max(x1, x2);
        startY = std::min(y1, y2);
        endY = std::max(y1, y2);
        startX = std::round(double(buffer.BufferWi - 0.5) * (double)startX / 100.0);
        endX = std::round(double(buffer.BufferWi - 0.5) * (double)endX / 100.0);
        startY = std::round(double(buffer.BufferHt - 0.5) * (double)startY / 100.0);
        endY = std::round(double(buffer.BufferHt - 0.5) * (double)endY / 100.0);
        startX = std::max(startX, 0);
        endX = std::min(endX, buffer.BufferWi - 1);
        startY = std::max(startY, 0);
        endY = std::min(endY, buffer.BufferHt - 1);
    }
    int tot = buffer.curPeriod - buffer.curEffStartPer;
    if (!shimmer || (tot % 2) == 0) {
        double HalfHt=double(endY - startY)/2.0;
        double HalfWi=double(endX - startX)/2.0;
        
        orig = color;
        HSVValue hsvOrig = color.asHSV();
        xlColor color2 = color;
        for (x=startX; x <= endX; x++)
        {
            HSVValue hsv = hsvOrig;
            if (HorizFade) {
                if (buffer.allowAlpha) {
                    color.alpha = (double)orig.alpha*(1.0-std::abs(HalfWi-x-startX)/HalfWi);
                } else {
                    hsv.value*=1.0-std::abs(HalfWi-x-startX)/HalfWi;
                    color = hsv;
                }
            }
            color2.alpha = color.alpha;
            for (y=startY; y<=endY; y++) {
                if (VertFade) {
                    if (buffer.allowAlpha) {
                        color.alpha = (double)color2.alpha*(1.0-std::abs(HalfHt-(y-startY))/HalfHt);
                    } else {
                        HSVValue hsv2 = hsv;
                        hsv2.value*=1.0-std::abs(HalfHt-(y-startY))/HalfHt;
                        color = hsv2;
                    }
                }
                buffer.SetPixel(x, y, color);
            }
        }
    } else {
        orig = xlBLACK;
    }
    std::unique_lock<std::recursive_mutex> lock(effect->GetBackgroundDisplayList().lock);
    if (VertFade || HorizFade) {
        effect->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6 * 2);
        int total = buffer.curEffEndPer - buffer.curEffStartPer + 1;
        double x1 = double(buffer.curPeriod - buffer.curEffStartPer) / double(total);
        double x2 = (buffer.curPeriod - buffer.curEffStartPer + 1.0) / double(total);
        int idx = (buffer.curPeriod - buffer.curEffStartPer) * 12;
        buffer.SetDisplayListVRect(effect, idx, x1, 0.0, x2, 0.5,
                                   xlBLACK, orig);
        buffer.SetDisplayListVRect(effect, idx, x1, 0.5, x2, 1.0,
                                   orig, xlBLACK);
    } else {
        effect->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6);
        int midX = (startX + endX) / 2;
        int midY = (startY + endY) / 2;
        buffer.CopyPixelsToDisplayListX(effect, midY, midX, midX);
    }

}

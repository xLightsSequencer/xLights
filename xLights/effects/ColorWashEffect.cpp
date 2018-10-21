#include <wx/checkbox.h>
#include <wx/notebook.h>

#include "ColorWashEffect.h"
#include "ColorWashPanel.h"
#include "../sequencer/Effect.h"
#include "../sequencer/EffectLayer.h"
#include "../sequencer/Element.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../../include/ColorWash.xpm"
#include "../models/DmxModel.h"

#include <sstream>

static const std::string CHECKBOX_ColorWash_HFade("CHECKBOX_ColorWash_HFade");
static const std::string CHECKBOX_ColorWash_VFade("CHECKBOX_ColorWash_VFade");
static const std::string TEXTCTRL_ColorWash_Cycles("TEXTCTRL_ColorWash_Cycles");
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
                                          DrawGLUtils::xlAccumulator &bg, xlColor* colorMask, bool ramps) {
    if (e->HasBackgroundDisplayList()) {
        DrawGLUtils::DrawDisplayList(x1, y1, x2-x1, y2-y1, e->GetBackgroundDisplayList(), bg);
        return e->GetBackgroundDisplayList().iconSize;
    }
    if (e->GetSettings().GetBool("E_CHECKBOX_ColorWash_CircularPalette")) {
        xlColorVector map(e->GetPalette());
        map.push_back(map[0]);
        bg.AddHBlendedRectangle(map, x1, y1, x2, y2, colorMask);
    } else {
        bg.AddHBlendedRectangle(e->GetPalette(), x1, y1, x2, y2, colorMask);
    }
    return 2;
}

void ColorWashEffect::SetDefaultParameters() {
    ColorWashPanel *p = (ColorWashPanel*)panel;
    if (p == nullptr) {
        return;
    }
    p->CyclesTextCtrl->SetValue("1.0");
    SetCheckBoxValue(p->HFadeCheckBox, false);
    SetCheckBoxValue(p->VFadeCheckBox, false);
    SetCheckBoxValue(p->ShimmerCheckBox, false);
    SetCheckBoxValue(p->CircularPaletteCheckBox, false);
    p->BitmapButton_ColorWash_CyclesVC->SetActive(false);
}

std::string ColorWashEffect::GetEffectString() {
    ColorWashPanel *p = (ColorWashPanel*)panel;
    std::stringstream ret;

    // You must always include value curves or they wont update correctly
    if (p->BitmapButton_ColorWash_CyclesVC->GetValue()->IsActive()) {
        ret << "E_VALUECURVE_ColorWash_Cycles=";
        ret << p->BitmapButton_ColorWash_CyclesVC->GetValue()->Serialise();
        ret << ",";
    }

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
    return ret.str();
}



wxPanel *ColorWashEffect::CreatePanel(wxWindow *parent) {
    return new ColorWashPanel(parent);
}

bool ColorWashEffect::needToAdjustSettings(const std::string &version) {
    return IsVersionOlder("2016.34", version) || RenderableEffect::needToAdjustSettings(version);
}
void ColorWashEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults) {
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    if (!effect->GetSettings().GetBool("E_CHECKBOX_ColorWash_EntireModel", true) ) {
        float x1 = effect->GetSettings().GetInt("E_SLIDER_ColorWash_X1", 0);
        float y1 = effect->GetSettings().GetInt("E_SLIDER_ColorWash_Y1", 0);
        float x2 = effect->GetSettings().GetInt("E_SLIDER_ColorWash_X2", 100);
        float y2 = effect->GetSettings().GetInt("E_SLIDER_ColorWash_Y2", 100);
        if (std::abs(x1) > 0.001f
            || std::abs(y1) > 0.001f
            || std::abs(100.0f - x2) > 0.001f
            || std::abs(100.0f - y2) > 0.001f) {
            std::string val = wxString::Format("%.2fx%.2fx%.2fx%.2f", x1, y1, x2, y2).ToStdString();
            effect->GetSettings()["B_CUSTOM_SubBuffer"] = val;
        }
    }
    effect->GetSettings().erase("E_CHECKBOX_ColorWash_EntireModel");
    effect->GetSettings().erase("E_SLIDER_ColorWash_X1");
    effect->GetSettings().erase("E_SLIDER_ColorWash_X2");
    effect->GetSettings().erase("E_SLIDER_ColorWash_Y1");
    effect->GetSettings().erase("E_SLIDER_ColorWash_Y2");
}
void ColorWashEffect::RemoveDefaults(const std::string &version, Effect *effect) {
    SettingsMap &settingsMap = effect->GetSettings();
    if (settingsMap.Get("E_CHECKBOX_ColorWash_HFade", "") == "0") {
        settingsMap.erase("E_CHECKBOX_ColorWash_HFade");
    }
    if (settingsMap.Get("E_CHECKBOX_ColorWash_VFade", "") == "0") {
        settingsMap.erase("E_CHECKBOX_ColorWash_VFade");
    }
    if (settingsMap.Get("E_CHECKBOX_ColorWash_Shimmer", "") == "0") {
        settingsMap.erase("E_CHECKBOX_ColorWash_Shimmer");
    }
    if (settingsMap.Get("E_CHECKBOX_ColorWash_CircularPalette", "") == "0") {
        settingsMap.erase("E_CHECKBOX_ColorWash_CircularPalette");
    }
    if (settingsMap.GetFloat("E_TEXTCTRL_ColorWash_Cycles", 0.0f) == 1.0f) {
        settingsMap.erase("E_TEXTCTRL_ColorWash_Cycles");
    }
    RenderableEffect::RemoveDefaults(version, effect);
}

void ColorWashEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    float cycles = GetValueCurveDouble("ColorWash_Cycles", 1.0, SettingsMap, oset, COLOURWASH_CYCLES_MIN, COLOURWASH_CYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool HorizFade = SettingsMap.GetBool(CHECKBOX_ColorWash_HFade);
    bool VertFade = SettingsMap.GetBool(CHECKBOX_ColorWash_VFade);
    bool shimmer = SettingsMap.GetBool(CHECKBOX_ColorWash_Shimmer);
    bool circularPalette = SettingsMap.GetBool(CHECKBOX_ColorWash_CircularPalette);

    int y;
    xlColor color, orig;

    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    buffer.GetMultiColorBlend(position, circularPalette, color);

    int startX = 0;
    int startY = 0;
    int endX = buffer.BufferWi - 1;
    int endY = buffer.BufferHt - 1;

    ///////////////////////// DMX Support ////////////////////////
    // if the model is a DMX model this will write the color into
    // the proper red, green, and blue channels.
    //////////////////////////////////////////////////////////////
    if (buffer.cur_model != "") {
        Model* model_info = buffer.GetModel();
        if (model_info != nullptr) {
            if( model_info->GetDisplayAs() == "DMX" ) {
                xlColor c;
                DmxModel* dmx = (DmxModel*)model_info;

                int white_channel = dmx->GetWhiteChannel();
                if (white_channel > 0 && color.red == color.green && color.red == color.blue)
                {
                    c.red = color.red;
                    c.green = color.red;
                    c.blue = color.red;
                    buffer.SetPixel(white_channel - 1, 0, c);
                }
                else
                {
                    int red_channel = dmx->GetRedChannel();
                    int grn_channel = dmx->GetGreenChannel();
                    int blu_channel = dmx->GetBlueChannel();
                    if (red_channel != 0) {
                        c.red = color.red;
                        c.green = color.red;
                        c.blue = color.red;
                        buffer.SetPixel(red_channel - 1, 0, c);
                    }
                    if (grn_channel != 0) {
                        c.red = color.green;
                        c.green = color.green;
                        c.blue = color.green;
                        buffer.SetPixel(grn_channel - 1, 0, c);
                    }
                    if (blu_channel != 0) {
                        c.red = color.blue;
                        c.green = color.blue;
                        c.blue = color.blue;
                        buffer.SetPixel(blu_channel - 1, 0, c);
                    }
                }
                return;
            }
        }
    }
    //////////////////////////////////////////////////////////////
    ///////////////////// End DMX Support ////////////////////////
    //////////////////////////////////////////////////////////////

    int tot = buffer.curPeriod - buffer.curEffStartPer;
    if (!shimmer || (tot % 2) == 0) {
        double HalfHt=double(endY - startY)/2.0;
        double HalfWi=double(endX - startX)/2.0;

        orig = color;
        HSVValue hsvOrig = color.asHSV();
        xlColor color2 = color;
        for (int x = startX; x <= endX; x++)
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
            else
            {
                color.alpha = orig.alpha;
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
        buffer.SetDisplayListVRect(effect, idx + 6, x1, 0.5, x2, 1.0,
                                   orig, xlBLACK);
    } else {
        effect->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6);
        int midX = (startX + endX) / 2;
        int midY = (startY + endY) / 2;
        buffer.CopyPixelsToDisplayListX(effect, midY, midX, midX);
    }
}

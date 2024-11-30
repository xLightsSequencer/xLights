/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <sstream>

#include "../../include/On.xpm"

#include "OnEffect.h"
#include "OnPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../Parallel.h"
#include <log4cpp/Category.hh>

static const std::string TEXTCTRL_Eff_On_Start("TEXTCTRL_Eff_On_Start");
static const std::string TEXTCTRL_Eff_On_End("TEXTCTRL_Eff_On_End");
static const std::string CHECKBOX_On_Shimmer("CHECKBOX_On_Shimmer");
static const std::string TEXTCTRL_On_Cycles("TEXTCTRL_On_Cycles");

OnEffect::OnEffect(int i) : RenderableEffect(i, "On", On, On, On, On, On)
{
    //ctor
}

OnEffect::~OnEffect()
{
    //dtor
}

xlEffectPanel *OnEffect::CreatePanel(wxWindow *parent) {
    return new OnPanel(parent);
}

void OnEffect::SetDefaultParameters() {
    OnPanel *p = (OnPanel*)panel;
    p->CheckBoxShimmer->SetValue(false);
    p->TextCtrlStart->SetValue("100");
    p->TextCtrlEnd->SetValue("100");
    p->TextCtrlCycles->SetValue("1.0");
    p->BitmapButton_On_Transparency->SetActive(false);
    SetSliderValue(p->Slider_On_Transparency, 0);
}

wxString OnEffect::GetEffectString() {
    OnPanel *p = (OnPanel*)panel;
    std::stringstream ret;
    if (100 != p->SliderStart->GetValue()) {
        ret << "E_TEXTCTRL_Eff_On_Start=";
        ret << p->TextCtrlStart->GetValue().ToStdString();
        ret << ",";
    }
    if (100 != p->SliderEnd->GetValue()) {
        ret << "E_TEXTCTRL_Eff_On_End=";
        ret << p->TextCtrlEnd->GetValue().ToStdString();
        ret << ",";
    }
    if (10 != p->SliderCycles->GetValue()) {
        ret << "E_TEXTCTRL_On_Cycles=";
        ret << p->TextCtrlCycles->GetValue().ToStdString();
        ret << ",";
    }
    if (p->CheckBoxShimmer->GetValue()) {
        ret << "E_CHECKBOX_On_Shimmer=1,";
    }
    if (p->BitmapButton_On_Transparency->GetValue()->IsActive()) {
        ret << "E_VALUECURVE_On_Transparency=";
        ret << p->BitmapButton_On_Transparency->GetValue()->Serialise();
        ret << ",";
    } else if (p->Slider_On_Transparency->GetValue() > 0) {
        ret << "E_TEXTCTRL_On_Transparency=";
        ret << p->TextCtrlOnTransparency->GetValue();
        ret << ",";
    }
    return ret.str();
}

void GetOnEffectColors(const Effect *e, xlColor &start, xlColor &end) {
    int starti = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_Start", 100);
    int endi = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_End", 100);
    xlColor newcolor = e->GetPalette()[0];
    if (starti == 100 && endi == 100) {
        start = end = newcolor;
    } else {
        HSVValue hsv = newcolor.asHSV();
        hsv.value = (hsv.value * starti) / 100;
        start = hsv;
        hsv = newcolor.asHSV();
        hsv.value = (hsv.value * endi) / 100;
        end = hsv;
    }
}

int OnEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                   xlVertexColorAccumulator &bg, xlColor* colorMask, bool ramp) 
{
    if (ramp)
    {
        bool shimmer = e->GetSettings().GetInt("E_CHECKBOX_On_Shimmer", 0) > 0;
        int starti = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_Start", 100);
        int endi = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_End", 100);
        xlColor color = e->GetPalette()[0];
        color.ApplyMask(colorMask);
        int height = y2 - y1;

        float starty = y2 - starti * height / 100.0f;
        float endy = y2 - endi * height / 100.0f;
        float m = float(endy - starty) / float(x2 - x1);

        bg.AddVertex(x1, starty, color);
        bg.AddVertex(x1, y2, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x2, endy, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x1, starty, color);

        if (shimmer)
        {
            const int gap = 3;
            for (int x = x1 + (gap*3); x < x2; x += (gap*3)) {
                float newY = m * (x - x1) + starty;
                float newY2 = m * (x + (gap) - x1) + starty;
                bg.AddVertex(x, y2, xlBLACK);
                bg.AddVertex(x, newY, xlBLACK);
                bg.AddVertex(x + (gap), newY2, xlBLACK);

                bg.AddVertex(x, y2, xlBLACK);
                bg.AddVertex(x + (gap), y2, xlBLACK);
                bg.AddVertex(x + (gap), newY2, xlBLACK);
            }
        }

        return 2;
    }
    else
    {
        if (e->HasBackgroundDisplayList()) {
            e->GetBackgroundDisplayList().addToAccumulator(x1, y1, x2 - x1, y2 - y1, bg);
            return e->GetBackgroundDisplayList().iconSize;
        }
        xlColor start;
        xlColor end;
        GetOnEffectColors(e, start, end);

        start.ApplyMask(colorMask);
        end.ApplyMask(colorMask);

        bg.AddVertex(x1, y1, start);
        bg.AddVertex(x1, y2, start);
        bg.AddVertex(x2, y2, end);

        bg.AddVertex(x2, y2, end);
        bg.AddVertex(x2, y1, end);
        bg.AddVertex(x1, y1, start);
        return 2;
    }
}

void OnEffect::RemoveDefaults(const std::string &version, Effect *effect) {
    SettingsMap &settingsMap = effect->GetSettings();
    if (settingsMap.Get("E_TEXTCTRL_Eff_On_Start", "") == "100") {
        settingsMap.erase("E_TEXTCTRL_Eff_On_Start");
    }
    if (settingsMap.Get("E_TEXTCTRL_Eff_On_End", "") == "100") {
        settingsMap.erase("E_TEXTCTRL_Eff_On_End");
    }
    if (settingsMap.Get("E_CHECKBOX_On_Shimmer", "") == "0") {
        settingsMap.erase("E_CHECKBOX_On_Shimmer");
    }
    if (settingsMap.Get("E_TEXTCTRL_On_Cycles", "") == "1.0") {
        settingsMap.erase("E_TEXTCTRL_On_Cycles");
    }
    RenderableEffect::RemoveDefaults(version, effect);
}

void OnEffect::Render(Effect *eff, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    
    int start = SettingsMap.GetInt(TEXTCTRL_Eff_On_Start, 100);
    int end = SettingsMap.GetInt(TEXTCTRL_Eff_On_End, 100);
    bool shimmer = SettingsMap.GetInt(CHECKBOX_On_Shimmer, 0) > 0;
    float cycles = SettingsMap.GetDouble(TEXTCTRL_On_Cycles, 1.0);
    
    int cidx = 0;
    if (shimmer) {
        int tot = buffer.curPeriod - buffer.curEffStartPer;
        if (tot % 2) {
            if (buffer.palette.Size() <= 1) {
                return;
            }
            cidx = 1;
        }
    }

    bool spatialcolour = buffer.palette.IsSpatial(cidx);
    double adjust = buffer.GetEffectTimeIntervalPosition(cycles);

    xlColor color;
    if (start == 100 && end == 100) {
        buffer.palette.GetColor(cidx, color);
    } else {
        HSVValue hsv;
        buffer.palette.GetHSV(cidx,hsv);
        double d = adjust;
        d = start + (end - start) * d;
        d = d / 100.0;
        hsv.value = hsv.value * d;
        color = hsv;
    }
    
    int transparency = GetValueCurveInt("On_Transparency", 0, SettingsMap, adjust, ON_TRANSPARENCY_MIN, ON_TRANSPARENCY_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    if (transparency) {
        transparency *= 255;
        transparency /= 100;
        color.alpha = 255 - transparency;
    }

    //Every Node set to selected color
    if (spatialcolour || buffer.dmx_buffer) {

        // doing x in parallel for large buffers
        parallel_for(0, buffer.BufferWi, [&buffer, cidx, start, end, adjust, transparency, color](int x) {
            auto col = color;
            for (int y = 0; y < buffer.BufferHt; ++y) {
                if (buffer.palette.IsRadial(cidx)) {
                    float zeroedx = (x - (buffer.BufferWi / 2));
                    float zeroedy = (y - (buffer.BufferHt / 2));
                    float round = 0;
                    if (zeroedx != 0 || zeroedy != 0) {
                        float signx = zeroedx > 0.0 ? 1.0 : (zeroedx < 0.0 ? -1.0 : 0.0);
                        float signy = zeroedy > 0.0 ? 1.0 : (zeroedy < 0.0 ? -1.0 : 0.0);
                        float signxy = zeroedx * zeroedy > 0.0 ? 1.0 : (zeroedx * zeroedy < 0.0 ? -1.0 : 0.0);
                        float signsignysigny = signy * signy > 0.0 ? 1.0 : (signy * signy < 0.0 ? -1.0 : 0.0);
                        round = (PI - PI / 2.0 * (1.0 + signx) * (1.0 - signsignysigny) - PI / 4.0 * (2.0 + signx) * signy - signxy * std::asin((std::abs(zeroedx) - std::abs(zeroedy)) / std::sqrt(2.0 * zeroedx * zeroedx + 2.0 * zeroedy * zeroedy))) / (2.0 * PI);
                    }
                    buffer.palette.GetSpatialColor(cidx, buffer.BufferWi / 2, buffer.BufferHt / 2, x, y, round, std::max(buffer.BufferWi, buffer.BufferHt) / 2, col);
                } else {
                    buffer.palette.GetSpatialColor(cidx, (float)x / (float)buffer.BufferWi, (float)y / (float)buffer.BufferHt, col);
                }
                if (start == 100 && end == 100) {
                } else {
                    HSVValue hsv = col.asHSV();
                    double d = adjust;
                    d = start + (end - start) * d;
                    d = d / 100.0;
                    hsv.value = hsv.value * d;
                    col = hsv;
                }
                if (transparency) {
                    col.alpha = 255 - transparency;
                }
                buffer.SetPixel(x, y, col);
            }
        });
    } else {
        buffer.Fill(color);
    }    

    if (shimmer || cycles != 1.0) {
        if (eff->IsBackgroundDisplayListEnabled()) {
            std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
            eff->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6);
            buffer.CopyPixelsToDisplayListX(eff, 0, 0, 0);
        }
    } else if (buffer.needToInit) {
        if (eff->IsBackgroundDisplayListEnabled()) {            
            std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
            eff->GetBackgroundDisplayList().resize(6);
            if (start == 100 && end == 100) {
                buffer.palette.GetColor(0, color);
                buffer.SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, color);
            } else {
                HSVValue hsv;
                buffer.palette.GetHSV(cidx,hsv);
                hsv.value = hsv.value * start / 100.0;
                color = hsv;
                
                buffer.palette.GetHSV(cidx,hsv);
                hsv.value = hsv.value * end / 100.0;
                buffer.SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, xlColor(hsv));
            }
        }
        buffer.needToInit = false;
    }
}

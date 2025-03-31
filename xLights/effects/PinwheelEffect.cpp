/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PinwheelEffect.h"
#include "PinwheelPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/pinwheel-16.xpm"
#include "../../include/pinwheel-24.xpm"
#include "../../include/pinwheel-32.xpm"
#include "../../include/pinwheel-48.xpm"
#include "../../include/pinwheel-64.xpm"
#include <log4cpp/Category.hh>

#include "../Parallel.h"

#include "ispc/PinwheelFunctions.ispc.h"

PinwheelEffect::PinwheelEffect(int id) : RenderableEffect(id, "Pinwheel", pinwheel_16, pinwheel_24, pinwheel_32, pinwheel_48, pinwheel_64)
{
    //ctor
}

PinwheelEffect::~PinwheelEffect()
{
    //dtor
}
xlEffectPanel *PinwheelEffect::CreatePanel(wxWindow *parent) {
    return new PinwheelPanel(parent);
}

void PinwheelEffect::SetDefaultParameters() {
    PinwheelPanel* pp = (PinwheelPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_PinwheelXCVC->SetActive(false);
    pp->BitmapButton_PinwheelYCVC->SetActive(false);
    pp->BitmapButton_Pinwheel_ArmSizeVC->SetActive(false);
    pp->BitmapButton_Pinwheel_SpeedVC->SetActive(false);
    pp->BitmapButton_Pinwheel_ThicknessVC->SetActive(false);
    pp->BitmapButton_Pinwheel_TwistVC->SetActive(false);
    pp->BitmapButton_Pinwheel_OffsetVC->SetActive(false);

    SetChoiceValue(pp->Choice_Pinwheel_3D, "none");
    SetChoiceValue(pp->Choice_Pinwheel_Style, "New Render Method");

    SetSliderValue(pp->Slider_PinwheelXC, 0);
    SetSliderValue(pp->Slider_PinwheelYC, 0);
    SetSliderValue(pp->Slider_Pinwheel_Arms, 3);
    SetSliderValue(pp->Slider_Pinwheel_ArmSize, 100);
    SetSliderValue(pp->Slider_Pinwheel_Thickness, 0);
    SetSliderValue(pp->Slider_Pinwheel_Twist, 0);
    SetSliderValue(pp->Slider_Pinwheel_Speed, 10);
    SetSliderValue(pp->Slider_Pinwheel_Offset, 0);

    SetCheckBoxValue(pp->CheckBox_Pinwheel_Rotation, true);
}

bool PinwheelEffect::needToAdjustSettings(const std::string &version) {
    // give the base class a chance to adjust any settings
    return RenderableEffect::needToAdjustSettings(version) || IsVersionOlder("2017.5", version);
}

void PinwheelEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults) {
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
    SettingsMap& settings = effect->GetSettings();
    if (settings.Contains("E_TEXTCTRL_Pinwheel_Speed")) {
        std::string val = settings["E_TEXTCTRL_Pinwheel_Speed"];
        settings.erase("E_TEXTCTRL_Pinwheel_Speed");
        settings["E_SLIDER_Pinwheel_Speed"] = val;
    }
}

PinwheelEffect::Pinwheel3DType PinwheelEffect::to3dType(const std::string& pinwheel_3d) {
    if (pinwheel_3d == "3D") {
        return PW_3D;
    } else if (pinwheel_3d == "3D Inverted") {
        return PW_3D_Inverted;
    } else if (pinwheel_3d == "Sweep") {
        return PW_SWEEP;
    }
    return PW_3D_NONE;
}

void PinwheelEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    const std::string& pinwheel_style = SettingsMap["CHOICE_Pinwheel_Style"];
    if (pinwheel_style == "New Render Method") {
        RenderNewMethod(effect, SettingsMap, buffer);
    } else {
        RenderOldMethod(effect, SettingsMap, buffer);
    }
}
void PinwheelEffect::RenderNewMethod(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    
    int pinwheel_arms = SettingsMap.GetInt("SLIDER_Pinwheel_Arms", 3);
    PinwheelData data(pinwheel_arms);
    
    data.pinwheel_twist = GetValueCurveInt("Pinwheel_Twist", 0, SettingsMap, oset, PINWHEEL_TWIST_MIN, PINWHEEL_TWIST_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pinwheel_thickness = GetValueCurveInt("Pinwheel_Thickness", 0, SettingsMap, oset, PINWHEEL_THICKNESS_MIN, PINWHEEL_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    data.pinwheel_rotation = SettingsMap.GetBool("CHECKBOX_Pinwheel_Rotation");
    const std::string& pinwheel_3d = SettingsMap["CHOICE_Pinwheel_3D"];
    data.xc_adj = GetValueCurveInt("PinwheelXC", 0, SettingsMap, oset, PINWHEEL_X_MIN, PINWHEEL_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    data.yc_adj = GetValueCurveInt("PinwheelYC", 0, SettingsMap, oset, PINWHEEL_Y_MIN, PINWHEEL_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pinwheel_armsize = GetValueCurveInt("Pinwheel_ArmSize", 100, SettingsMap, oset, PINWHEEL_ARMSIZE_MIN, PINWHEEL_ARMSIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pspeed = GetValueCurveInt("Pinwheel_Speed", 10, SettingsMap, oset, PINWHEEL_SPEED_MIN, PINWHEEL_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    data.poffset = GetValueCurveInt("Pinwheel_Offset", 0, SettingsMap, oset, PINWHEEL_OFFSET_MIN, PINWHEEL_OFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    
    data.pos = (float)((buffer.curPeriod - buffer.curEffStartPer) * pspeed * buffer.frameTimeInMs) / (float)PINWHEEL_SPEED_MAX;
    data.degrees_per_arm = 1;
    if (pinwheel_arms > 0) data.degrees_per_arm = 360 / pinwheel_arms;
    float armsize = (pinwheel_armsize / 100.0);
    
    data.pw3dType = to3dType(pinwheel_3d);
    
    HSVValue hsv;
    for (int i = 0; i < data.pinwheel_arms; i++) {
        data.colorarray[i] = (i + 1) % buffer.GetColorCount();
        data.colorIsSpacial[i] = buffer.palette.IsSpatial(data.colorarray[i]);
        if (data.colorIsSpacial[i]) {
            data.hasSpacial = true;
        }
        buffer.palette.GetColor(data.colorarray[i], data.colorsAsColor[i]);
        buffer.palette.GetHSV(data.colorarray[i], data.colorsAsHSV[i]);
    }
    
    int xc = (int)(ceil(std::hypot(buffer.BufferWi, buffer.BufferHt) / 2));
    data.xc_adj = (data.xc_adj * buffer.BufferWi) / 200;
    data.yc_adj = (data.yc_adj * buffer.BufferHt) / 200;
    
    data.max_radius = xc * armsize;
    if (pinwheel_thickness == 0) pinwheel_thickness = 1;
    data.tmax = (pinwheel_thickness / 100.0) * data.degrees_per_arm;
    
    // Force single visible line in case width is narrower than visible
    float pi_180 = (float)M_PI / 180.0f;
    for (int a = 0; a < pinwheel_arms; a++) {
        int ColorIdx = a;
        bool isSpacial = data.colorIsSpacial[ColorIdx];
        hsv = data.colorsAsHSV[ColorIdx];
        xlColor color = xlColor(hsv);
        
        int angle = (a * data.degrees_per_arm);
        if (data.pinwheel_rotation == 1) { // do we have CW rotation
            angle = (270 - angle) + data.pos + data.poffset;
        } else {
            angle = angle - 90 - data.pos - data.poffset;
        }
        
        if (data.max_radius != 0) {
            int t2 = (int)angle % data.degrees_per_arm;
            float round = (float)t2 / data.tmax;
            for (float r = 0; r <= data.max_radius; r += 0.5) {
                int degrees_twist = (r / data.max_radius) * data.pinwheel_twist;
                int x = floor((int)(r * buffer.cos((angle + degrees_twist) * pi_180)) + data.xc_adj + buffer.BufferWi / 2);
                int y = floor((int)(r * buffer.sin((angle + degrees_twist) * pi_180)) + data.yc_adj + buffer.BufferHt / 2);
                if (isSpacial) {
                    buffer.palette.GetSpatialColor(data.colorarray[ColorIdx], data.xc_adj + buffer.BufferWi / 2, data.yc_adj + buffer.BufferHt / 2,
                                                   x, y, round, data.max_radius, color);
                }
                buffer.SetPixel(x, y, color);
            }
        }
    }
    
    // Draw actual pinwheel arms
    if (data.max_radius != 0) {
        RenderNewArms(buffer, data);
    }
}
void PinwheelEffect::RenderNewArms(RenderBuffer& buffer, PinwheelData &data) {
    ispc::PinwheelData rdata;
    rdata.width = buffer.BufferWi;
    rdata.height = buffer.BufferHt;
    rdata.pinwheel_arms = data.pinwheel_arms;
    rdata.xc_adj = data.xc_adj;
    rdata.yc_adj = data.yc_adj;
    rdata.degrees_per_arm = data.degrees_per_arm;
    rdata.pinwheel_twist = data.pinwheel_twist;
    rdata.max_radius= data.max_radius;
    rdata.poffset = data.poffset;
    rdata.pw3dType = data.pw3dType;
    rdata.pinwheel_rotation = data.pinwheel_rotation;
    rdata.tmax = data.tmax;
    rdata.pos = data.pos;
    rdata.allowAlpha = buffer.allowAlpha;
    rdata.numColors = data.colorsAsColor.size();
    rdata.colorarray = &data.colorarray[0];
    
    std::vector<ispc::float3> colorsAsHSV(rdata.numColors);
    std::vector<uint8_t> colorIsSpacial(rdata.numColors);
    for (int x = 0; x < rdata.numColors; x++) {
        colorsAsHSV[x] = {(float)data.colorsAsHSV[x].hue, (float)data.colorsAsHSV[x].saturation, (float)data.colorsAsHSV[x].value};
        colorIsSpacial[x] = data.colorIsSpacial[x] ? 1 : 0;
    }
    rdata.colorsAsColor = (ispc::uint8_t4*)&data.colorsAsColor[0];
    rdata.colorsAsHSV = &colorsAsHSV[0];
    rdata.colorIsSpacial = &colorIsSpacial[0];
    rdata.bufferData = (void*)&buffer;
    
    int max = buffer.BufferHt * buffer.BufferWi;
    constexpr int bfBlockSize = 4096;
    int blocks = max / bfBlockSize + 1;
    parallel_for(0, blocks, [&rdata, &buffer, max](int y) {
        int start = y * bfBlockSize;
        int end = start + bfBlockSize;
        if (end > max) {
            end = max;
        }
        PinwheelEffectStyle0(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
    });
}
void PinwheelEffect::RenderOldMethod(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    
    int pinwheel_arms = SettingsMap.GetInt("SLIDER_Pinwheel_Arms", 3);
    int pinwheel_twist = GetValueCurveInt("Pinwheel_Twist", 0, SettingsMap, oset, PINWHEEL_TWIST_MIN, PINWHEEL_TWIST_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pinwheel_thickness = GetValueCurveInt("Pinwheel_Thickness", 0, SettingsMap, oset, PINWHEEL_THICKNESS_MIN, PINWHEEL_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pinwheel_rotation = SettingsMap.GetBool("CHECKBOX_Pinwheel_Rotation");
    const std::string& pinwheel_3d = SettingsMap["CHOICE_Pinwheel_3D"];
    int xc_adj = GetValueCurveInt("PinwheelXC", 0, SettingsMap, oset, PINWHEEL_X_MIN, PINWHEEL_X_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yc_adj = GetValueCurveInt("PinwheelYC", 0, SettingsMap, oset, PINWHEEL_Y_MIN, PINWHEEL_Y_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pinwheel_armsize = GetValueCurveInt("Pinwheel_ArmSize", 100, SettingsMap, oset, PINWHEEL_ARMSIZE_MIN, PINWHEEL_ARMSIZE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int pspeed = GetValueCurveInt("Pinwheel_Speed", 10, SettingsMap, oset, PINWHEEL_SPEED_MIN, PINWHEEL_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int poffset = GetValueCurveInt("Pinwheel_Offset", 0, SettingsMap, oset, PINWHEEL_OFFSET_MIN, PINWHEEL_OFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    
    double pos = (double)((buffer.curPeriod - buffer.curEffStartPer) * pspeed * buffer.frameTimeInMs) / (double)PINWHEEL_SPEED_MAX;
    int degrees_per_arm = 1;
    if (pinwheel_arms > 0) degrees_per_arm = 360 / pinwheel_arms;
    float armsize = (pinwheel_armsize / 100.0);
    
    Pinwheel3DType pw3dType = to3dType(pinwheel_3d);
    
    // Old Render Method
    size_t colorcnt = buffer.GetColorCount();

    int xc = (int)(std::max(buffer.BufferWi, buffer.BufferHt) / 2);

    for (int a = 1; a <= pinwheel_arms; a++) {
        int ColorIdx = a % colorcnt;

        int base_degrees;
        if (pinwheel_rotation == 1) {
            base_degrees = (a - 1) * degrees_per_arm + pos + poffset; // yes
        } else {
            base_degrees = (a - 1) * degrees_per_arm - pos + poffset; // no, we are CCW
        }

        float tmax = (pinwheel_thickness / 100.0) * degrees_per_arm / 2.0;
        for (float t = base_degrees - tmax; t <= base_degrees + tmax; t++) {
            Draw_arm(buffer, t, xc * armsize, pinwheel_twist, xc_adj, yc_adj, ColorIdx, pw3dType, (t - base_degrees + tmax) / (2 * tmax + 1));
        }
    }
}

void PinwheelEffect::adjustColor(PinwheelEffect::Pinwheel3DType pw3dType, xlColor& color, HSVValue& hsv, bool allowAlpha, float round) {
    switch (pw3dType) {
    case PW_3D:
        if (allowAlpha) {
            color.alpha = 255.0 - 255.0 * std::abs(round - 0.5) / 0.5;
        } else {
            hsv.value = 1.0 - hsv.value * std::abs(round - 0.5) / 0.5;
            color = hsv;
        }
        break;
    case PW_3D_Inverted:
        if (allowAlpha) {
            color.alpha = 255.0 * std::abs(round - 0.5) / 0.5;
        } else {
            hsv.value = hsv.value * std::abs(round - 0.5) / 0.5;
            color = hsv;
        }
        break;
    case PW_SWEEP:
        if (allowAlpha) {
            color.alpha = (int)(255.0 * round);
        } else {
            hsv.value = (float)hsv.value * round;
            color = hsv;
        }
        break;
    default:
        break;
    }
}

void PinwheelEffect::Draw_arm(RenderBuffer& buffer,
    int base_degrees, int max_radius, int pinwheel_twist,
    int xc_adj, int yc_adj, int colorIdx, PinwheelEffect::Pinwheel3DType pw3dType, float round)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    float pi_180 = (float)M_PI / 180.0f;

    int xc = buffer.BufferWi / 2;
    int yc = buffer.BufferHt / 2;
    xc = xc + ((xc_adj * xc) / 100); // xc_adj is from -100 to 100
    yc = yc + ((yc_adj * yc) / 100);

    bool isSpatial = buffer.palette.IsSpatial(colorIdx);
    xlColor color;
    HSVValue hsv;
    if (!isSpatial) {
        buffer.palette.GetColor(colorIdx, color);
        hsv = color.asHSV();
        adjustColor(pw3dType, color, hsv, buffer.allowAlpha, round);
    }

    if (max_radius != 0) {
        for (float r = 0.0f; r <= max_radius; r += 0.5f) {
            int degrees_twist = (r / max_radius) * pinwheel_twist;
            int degrees = base_degrees + degrees_twist;
            float phi = degrees * pi_180;
            int x = r * buffer.cos(phi) + xc;
            int y = r * buffer.sin(phi) + yc;

            if (isSpatial) {
                buffer.palette.GetSpatialColor(colorIdx, xc, yc, x, y, round, max_radius, color);
                hsv = color.asHSV();
                adjustColor(pw3dType, color, hsv, buffer.allowAlpha, round);
            }
            buffer.SetPixel(x, y, color);
        }
    }
}

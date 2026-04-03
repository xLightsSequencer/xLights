/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ValueCurveConsts.h"

#include <cassert>

namespace ValueCurveConsts {

// --- Buffer ---

double GetBufferSettingVCMin(const std::string& name) {
    if (name == "B_VALUECURVE_Blur")
        return BLUR_MIN;
    if (name == "B_VALUECURVE_Rotation")
        return RZ_ROTATION_MIN;
    if (name == "B_VALUECURVE_Zoom")
        return RZ_ZOOM_MIN;
    if (name == "B_VALUECURVE_Rotations")
        return RZ_ROTATIONS_MIN;
    if (name == "B_VALUECURVE_PivotPointX")
        return RZ_PIVOTX_MIN;
    if (name == "B_VALUECURVE_PivotPointY")
        return RZ_PIVOTY_MIN;
    if (name == "B_VALUECURVE_XRotation")
        return RZ_XROTATION_MIN;
    if (name == "B_VALUECURVE_YRotation")
        return RZ_YROTATION_MIN;
    if (name == "B_VALUECURVE_XPivot")
        return RZ_XPIVOT_MIN;
    if (name == "B_VALUECURVE_YPivot")
        return RZ_YPIVOT_MIN;
    assert(false);
    return 0;
}

double GetBufferSettingVCMax(const std::string& name) {
    if (name == "B_VALUECURVE_Blur")
        return BLUR_MAX;
    if (name == "B_VALUECURVE_Rotation")
        return RZ_ROTATION_MAX;
    if (name == "B_VALUECURVE_Zoom")
        return RZ_ZOOM_MAX;
    if (name == "B_VALUECURVE_Rotations")
        return RZ_ROTATIONS_MAX;
    if (name == "B_VALUECURVE_PivotPointX")
        return RZ_PIVOTX_MAX;
    if (name == "B_VALUECURVE_PivotPointY")
        return RZ_PIVOTY_MAX;
    if (name == "B_VALUECURVE_XRotation")
        return RZ_XROTATION_MAX;
    if (name == "B_VALUECURVE_YRotation")
        return RZ_YROTATION_MAX;
    if (name == "B_VALUECURVE_XPivot")
        return RZ_XPIVOT_MAX;
    if (name == "B_VALUECURVE_YPivot")
        return RZ_YPIVOT_MAX;
    assert(false);
    return 100;
}

int GetBufferSettingVCDivisor(const std::string& name) {
    if (name == "B_VALUECURVE_Zoom")
        return RZ_ZOOM_DIVISOR;
    if (name == "B_VALUECURVE_Rotations")
        return RZ_ROTATIONS_DIVISOR;
    return 1;
}

// --- Color ---

double GetColorSettingVCMin(const std::string& name) {
    if (name == "C_VALUECURVE_Brightness")
        return COLORPANEL_BRIGHTNESS_MIN;
    if (name == "C_VALUECURVE_SparkleFrequency")
        return COLORPANEL_SPARKLE_MIN;
    if (name == "C_VALUECURVE_Color_ValueAdjust")
        return COLORPANEL_VALUE_MIN;
    if (name == "C_VALUECURVE_Color_HueAdjust")
        return COLORPANEL_HUE_MIN;
    if (name == "C_VALUECURVE_Color_SaturationAdjust")
        return COLORPANEL_SATURATION_MIN;
    assert(false);
    return 0;
}

double GetColorSettingVCMax(const std::string& name) {
    if (name == "C_VALUECURVE_Brightness")
        return COLORPANEL_BRIGHTNESS_MAX;
    if (name == "C_VALUECURVE_SparkleFrequency")
        return COLORPANEL_SPARKLE_MAX;
    if (name == "C_VALUECURVE_Color_ValueAdjust")
        return COLORPANEL_VALUE_MAX;
    if (name == "C_VALUECURVE_Color_HueAdjust")
        return COLORPANEL_HUE_MAX;
    if (name == "C_VALUECURVE_Color_SaturationAdjust")
        return COLORPANEL_SATURATION_MAX;
    assert(false);
    return 100;
}

int GetColorSettingVCDivisor(const std::string& name) {
    return 1;
}

// --- Timing ---

double GetTimingSettingVCMin(const std::string& name) {
    if (name == "T_VALUECURVE_In_Transition_Adjust")
        return IN_TRANSITION_MIN;
    if (name == "T_VALUECURVE_Out_Transition_Adjust")
        return OUT_TRANSITION_MIN;
    assert(false);
    return 0;
}

double GetTimingSettingVCMax(const std::string& name) {
    if (name == "T_VALUECURVE_In_Transition_Adjust")
        return IN_TRANSITION_MAX;
    if (name == "T_VALUECURVE_Out_Transition_Adjust")
        return OUT_TRANSITION_MAX;
    assert(false);
    return 100;
}

int GetTimingSettingVCDivisor(const std::string& name) {
    return 1;
}

} // namespace ValueCurveConsts
